#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#include "wl_internal.h"
#include "wl_ioctl.h"
#include "wl_debug.h"

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif

#define DHD_IOCTL_MAGIC				(0x00444944)
#define DHD_GET_VAR					2
#define DHD_SET_VAR					3

#define WL_BSSCFG_PREX_STR			"bsscfg:"

#define WL_EVENT_LIST_SIZE			(5)
/* Timeout in second for event from driver */
#define WL_EVENT_TIMEOUT 			10
#define WL_EVENT_BUF_SIZE 			2048
#define WL_REG_EVENT_ONCE_MAX		(16)

#define THREAD_CLOSE_EVENT			"CLOSE"

/* Broadcom OUI (Organizationally Unique Identifier): Used in the proprietary(221) IE (Information Element) in all Broadcom devices */
#define BRCM_OUI            		"\x00\x10\x18"
/* Length in bytes of 802.11 OUI*/
#define DOT11_OUI_LEN 				3

#define JOIN_ASSOCIATED             (uint32_t)(1 << 0)
#define JOIN_AUTHENTICATED          (uint32_t)(1 << 1)
#define JOIN_LINK_READY             (uint32_t)(1 << 2)
#define JOIN_SECURITY_COMPLETE      (uint32_t)(1 << 3)
#define JOIN_SSID_SET               (uint32_t)(1 << 4)
#define JOIN_NO_NETWORKS            (uint32_t)(1 << 5)
#define JOIN_EAPOL_KEY_M1_TIMEOUT   (uint32_t)(1 << 6)
#define JOIN_EAPOL_KEY_M3_TIMEOUT   (uint32_t)(1 << 7)
#define JOIN_EAPOL_KEY_G1_TIMEOUT   (uint32_t)(1 << 8)
#define JOIN_EAPOL_KEY_FAILURE      (uint32_t)(1 << 9)
#define JOIN_COMPLETED 				(JOIN_AUTHENTICATED | JOIN_LINK_READY | JOIN_SSID_SET | JOIN_SECURITY_COMPLETE)

/* Linux network driver ioctl encoding */
typedef struct
{
	uint32_t cmd;		/**< common ioctl definition */
	void *buf;			/**< pointer to user buffer */
	uint32_t len;		/**< length of user buffer */
	uint8_t set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32_t used;		/**< bytes read or written (optional) */
	uint32_t needed;	/**< bytes needed (optional) */
	uint32_t driver;	/**< to identify target driver */
} wl_dhd_ioctl_t;

typedef struct
{
	uint8_t event_mask[WL_EVENTING_MASK_LEN];
#ifdef WL_CONFIG_CHECK_EVENT
	uint8_t reg_event_mask[WL_EVENTING_MASK_LEN];
	uint32_t is_event_set;
#endif /* WL_CONFIG_CHECK_EVENT */
	wl_event_cb_t callback;
	void* user_data;
} wl_event_item_t;

typedef enum
{
	WL_CONNECT_STATUS_DISCONNECTED = 0,
	WL_CONNECT_STATUS_DISCONNECTING,
	WL_CONNECT_STATUS_CONNECTED,
	WL_CONNECT_STATUS_CONNECTING,
} wl_connect_status_t;

static int wl_ioctl_init(const char* ifname);
static int wl_ioctl_deinit(void);
static int wl_event_init(void);
static int wl_event_deinit(void);
static void* wl_event_thread_func(void *args);
static void wl_connect_event_handler(wl_event_msg_t* msg, void* user_data);

pthread_mutex_t wl_dbg_mutex;

static int wl_ioctl_sockfd = -1;
static struct ifreq wl_ifr;
static bool wl_is_init;
static sem_t wl_connect_sem;
static pthread_mutex_t wl_mutex;
static pthread_t wl_event_thread;
static int wl_event_pipe[2];
static wl_event_item_t wl_event_list[WL_EVENT_LIST_SIZE];
static uint32_t wl_event_num;
static const uint32_t wl_connect_status_event[] = {
						WLC_E_SET_SSID,
                        WLC_E_JOIN,
                        WLC_E_LINK,
						WLC_E_AUTH,
						WLC_E_DEAUTH_IND,
						WLC_E_DISASSOC_IND,
						WLC_E_PSK_SUP
					};
static uint32_t wl_connect_status;
static uint32_t wl_join_status;

static wl_connect_status_cb_t wl_connect_status_callback;
static void *wl_connect_status_userdata;

static int wl_mkiovar(
				const char* iovar_name,
				void* param,
				int32_t paramlen,
				int8_t* iovar_buf,
				int32_t buflen)
{
    int32_t len;

    len = strlen(iovar_name) + 1;
    if ((len + paramlen) > buflen)
    {
        return WL_E_BUFTOOSHORT;
    }

    strncpy((char*)iovar_buf, iovar_name, buflen);

	if ((param != NULL) && (paramlen > 0))
	{
	    /* append data onto the end of the name string */
	    memcpy(&iovar_buf[len], param, paramlen);
	    len += paramlen;
	}

    return len;
}

static int wl_mkiovar_bsscfg(
				const char* iovar_name,
				void* param,
				int32_t paramlen,
				int8_t* iovar_buf,
				int32_t buflen,
				int32_t bssidx)
{
	const char *prefix = WL_BSSCFG_PREX_STR;
	int8_t *p;
	uint32_t prefixlen;
	uint32_t namelen;
	uint32_t iolen;

	if ((iovar_buf == NULL) || (buflen == 0))
	{
		return WL_E_BADARG;
	}

	memset(iovar_buf, 0, buflen);

	if (bssidx == 0)
	{
		return wl_mkiovar(iovar_name, param, paramlen, iovar_buf, buflen);
	}

	prefixlen = strlen(prefix);
	namelen = strlen(iovar_name) + 1;
	iolen = prefixlen + namelen + sizeof(uint32_t) + paramlen;

	if (buflen < 0 || iolen > (uint32_t)buflen)
	{
		return WL_E_BADARG;
	}

	p = iovar_buf;

	/* copy prefix, no null */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	memcpy(p, iovar_name, namelen);
	p += namelen;

	/* bss config index as first param */
	memcpy(p, &bssidx, sizeof(uint32_t));
	p += sizeof(uint32_t);

	/* parameter buffer follows */
	if (paramlen)
	{
		memcpy(p, param, paramlen);
	}

	return iolen;
}

int wl_ioctl(uint32_t cmd, void* buf, uint32_t len, bool set)
{
    int ret = 0;
    wl_ioctl_t ioc;

#if WL_LOG_LEVEL >= WL_LOG_DEBUG
	if (cmd == WLC_SET_VAR)
	{
        WL_DBG("SET IOVAR %s\n", (char*)buf);
	}
	else if (cmd == WLC_GET_VAR)
	{
        WL_DBG("GET IOVAR %s\n", (char*)buf);
	}
	else
	{
		WL_DBG("cmd = %d\n", cmd);
	}
#endif /* WL_LOG_LEVEL >= WL_LOG_DEBUG */

	WL_DUMP(buf, len);

	if (wl_ioctl_sockfd < 0)
	{
		WL_ERR("wl is not init\n");
		return WL_E_IOCTL_ERROR;
	}

    ioc.cmd = cmd;
    ioc.buf = buf;
    ioc.len = len;
    ioc.set = set;
    wl_ifr.ifr_data = (caddr_t)&ioc;

	ret = ioctl(wl_ioctl_sockfd, SIOCDEVPRIVATE, &wl_ifr);

    if (ret < 0) {
#if WL_LOG_LEVEL >= WL_LOG_DEBUG
        if ((cmd != WLC_GET_VAR) || (strncmp(buf, "bcmerrorstr", strlen("bcmerrorstr") != 0)))
        {
    		char buf[128];

    		memset(buf, 0, sizeof(buf));


    		wl_iovar_get("bcmerrorstr", buf, sizeof(buf));

            WL_ERR("ioctl err = %d, bcmerror = %s\n", ret, buf);
        }
#else /* WL_LOG_LEVEL >= WL_LOG_DEBUG */
        WL_ERR("ioctl err = %d\n", ret);
#endif /* WL_LOG_LEVEL >= WL_LOG_DEBUG */

		return WL_E_IOCTL_ERROR;
    }

	return WL_E_OK;
}

int wl_iovar_getbuf(
		const char* iovar_name,
		void* param,
		int32_t paramlen,
		int8_t* iovar_buf,
		int32_t buflen)
{
    int ret;

    ret = wl_mkiovar(
				iovar_name,
		        param,
		        paramlen,
		        iovar_buf,
		        buflen);

	if (ret > 0)
	{
		ret = wl_ioctl(WLC_GET_VAR, iovar_buf, buflen, false);
	}

    return ret;
}

int wl_iovar_setbuf(
		const char* iovar_name,
		void* param,
		int32_t paramlen,
		int8_t* iovar_buf,
		int32_t buflen)
{
    int ret;

    ret = wl_mkiovar(
				iovar_name,
		        param,
		        paramlen,
		        iovar_buf,
		        buflen);

	if (ret > 0)
	{
		ret = wl_ioctl(WLC_SET_VAR, iovar_buf, ret, true);
	}

    return ret;
}

int wl_iovar_get(const char* iovar_name, void* outbuf, int32_t len)
{
	int ret;
	int8_t smbuf[WLC_IOCTL_SMLEN];

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > WLC_IOCTL_SMLEN)
	{
		ret = wl_iovar_getbuf(iovar_name, NULL, 0, outbuf, len);
	}
	else
	{
		memset(smbuf, 0, sizeof(smbuf));

		ret = wl_iovar_getbuf(
					iovar_name,
					NULL,
					0,
					smbuf,
					sizeof(smbuf));

		if (ret == 0)
		{
			memcpy(outbuf, smbuf, len);
		}
	}

	return ret;
}

int wl_iovar_set(const char* iovar_name, void* param, int32_t paramlen)
{
	int8_t smbuf[WLC_IOCTL_SMLEN * 2];

	memset(smbuf, 0, sizeof(smbuf));

	return wl_iovar_setbuf(
				iovar_name,
				param,
				paramlen,
				smbuf,
				sizeof(smbuf));
}

int wl_iovar_getint(const char* iovar_name, int32_t* pval)
{
	return wl_iovar_get(iovar_name, pval, sizeof(int32_t));
}

int wl_iovar_setint(const char* iovar_name, int32_t val)
{
	return wl_iovar_set(iovar_name, &val, sizeof(int32_t));
}

int wl_iovar_bsscfg_getbuf(
	    const char* iovar_name,
	    void* param,
	    int32_t paramlen,
	    void* iovar_buf,
	    int32_t buflen,
	    int32_t bsscfg_idx)
{
	int ret = 0;

	wl_mkiovar_bsscfg(iovar_name, param, paramlen, iovar_buf, buflen, bsscfg_idx);

	ret = wl_ioctl(WLC_GET_VAR, iovar_buf, buflen, false);

	return ret;
}

int wl_iovar_bsscfg_setbuf(
        const char*iovar_name,
	    void* param,
	    int32_t paramlen,
	    void* iovar_buf,
	    int32_t buflen,
	    int32_t bsscfg_idx)
{
	int ret = 0;
	int32_t iovar_len;

	iovar_len = wl_mkiovar_bsscfg(iovar_name, param, paramlen, iovar_buf, buflen, bsscfg_idx);

    if (iovar_len > 0)
		ret = wl_ioctl(WLC_SET_VAR, iovar_buf, iovar_len, true);
	else {
		ret = WL_E_BUFTOOSHORT;
	}

	return ret;
}

int wl_iovar_bsscfg_get(
		const char* iovar_name,
		void* outbuf,
		int32_t len,
		int32_t bsscfg_idx)
{
	int ret;
	int8_t smbuf[WLC_IOCTL_SMLEN];

	/* use the return buffer if it is bigger than what we have on the stack */
	if (len > WLC_IOCTL_SMLEN)
	{
		ret = wl_iovar_bsscfg_getbuf(iovar_name, NULL, 0, outbuf, len, bsscfg_idx);
	}
	else
	{
		memset(smbuf, 0, sizeof(smbuf));

		ret = wl_iovar_bsscfg_getbuf(
					iovar_name,
					NULL,
					0,
					smbuf,
					sizeof(smbuf),
					bsscfg_idx);

		if (ret == 0)
		{
			memcpy(outbuf, smbuf, len);
		}
	}

	return ret;
}

int wl_iovar_bsscfg_set(
		const char* iovar_name,
		void* param,
		int32_t paramlen,
		int32_t bsscfg_idx)
{
	int8_t smbuf[WLC_IOCTL_SMLEN * 2];

	memset(smbuf, 0, sizeof(smbuf));

	return wl_iovar_bsscfg_setbuf(
				iovar_name,
				param,
				paramlen,
				smbuf,
				sizeof(smbuf),
				bsscfg_idx);
}

int wl_iovar_bsscfg_getint(
		const char*iovar_name,
		int32_t* pval,
		int32_t bsscfg_idx)
{
	return wl_iovar_bsscfg_get(iovar_name, pval, sizeof(int32_t), bsscfg_idx);
}

int wl_iovar_bsscfg_setint(
	    const char* iovar_name,
	    int32_t val,
	    int32_t bsscfg_idx)
{
	return wl_iovar_bsscfg_set(iovar_name, &val, sizeof(int32_t), bsscfg_idx);
}

int wl_dhd_iovar_setint(const char* iovar_name, int32_t val)
{
    int ret = 0;
    wl_dhd_ioctl_t ioc;
	int8_t iovar_buf[WLC_IOCTL_SMLEN];
    int32_t len;

	if (wl_ioctl_sockfd < 0)
	{
		WL_ERR("wl is not init\n");
		return WL_E_IOCTL_ERROR;
	}

    WL_DBG("IOVAR %s\n", (char*)iovar_name);

	memset(iovar_buf, 0, WLC_IOCTL_SMLEN);

    len = strlen(iovar_name) + 1;

	if ((len + sizeof(int32_t)) > WLC_IOCTL_SMLEN)
	{
		return WL_E_BUFTOOSHORT;
	}

    strncpy((char*)iovar_buf, iovar_name, WLC_IOCTL_SMLEN);
    memcpy(&iovar_buf[len], &val, sizeof(int32_t));
    len += sizeof(int32_t);

    ioc.cmd = DHD_SET_VAR;
    ioc.buf = iovar_buf;
    ioc.len = len;
    ioc.set = true;
	ioc.driver = DHD_IOCTL_MAGIC;
    wl_ifr.ifr_data = (caddr_t)&ioc;

	ret = ioctl(wl_ioctl_sockfd, SIOCDEVPRIVATE, &wl_ifr);

    if (ret < 0) {
		return WL_E_IOCTL_ERROR;
    }

	return WL_E_OK;
}

int wl_init(const char* ifname)
{
	int ret;
	wl_ether_addr_t bssid;

	pthread_mutex_init(&wl_dbg_mutex, NULL);

	WL_DBG("ifname = %s\n", ifname);

	if (wl_is_init)
	{
		WL_INFO("already init\n");
		return WL_E_OK;
	}

	ret = wl_ioctl_init(ifname);

	if (ret != WL_E_OK)
	{
		return ret;
	}

	ret = wl_get_bssid(&bssid);

	if (ret == 0)
	{
	#ifdef WL_FIX_CONNECT_STATUS_OPTIONS
		uint32_t wakeind = 0;
	
		ret = wl_wowl_get_wakeind(&wakeind);

		if ((ret == WL_E_OK) && (wakeind & WL_WOWL_DIS))
		{
			wl_connect_status = WL_CONNECT_STATUS_DISCONNECTED;
		}
		else
		{
			wl_connect_status = WL_CONNECT_STATUS_CONNECTED;
            wl_join_status = JOIN_COMPLETED;
		}
	#else /* WL_FIX_CONNECT_STATUS_OPTIONS */
		wl_connect_status = WL_CONNECT_STATUS_CONNECTED;
        wl_join_status = JOIN_COMPLETED;
	#endif /* WL_FIX_CONNECT_STATUS_OPTIONS */	
	}
	else
	{
		wl_connect_status = WL_CONNECT_STATUS_DISCONNECTED;
	}

	ret = wl_event_init();

	if (ret != WL_E_OK)
	{
		wl_ioctl_deinit();
	}

	pthread_mutex_init(&wl_mutex, NULL);
	sem_init(&wl_connect_sem, 0, 0);

	wl_is_init = true;

    return ret;
}

int wl_deinit(void)
{
	WL_DBG("enter\n");

	wl_event_deinit();
	wl_ioctl_deinit();
	pthread_mutex_destroy(&wl_mutex);
	sem_destroy(&wl_connect_sem);
	pthread_mutex_destroy(&wl_dbg_mutex);
	wl_is_init = false;

	return 0;
}

bool wl_is_associated(void)
{
	return (wl_connect_status == WL_CONNECT_STATUS_CONNECTED);
}

int wl_error(void)
{
	int ret;
	int32_t error;

	ret = wl_iovar_getint("bcmerror", &error);

	if (ret == 0)
	{
		return error;
	}
	else
	{
		return WL_E_OK;
	}
}

static int wl_ioctl_init(const char* ifname)
{
	int ret;
	struct ifreq ifr;

	WL_DBG("ifname = %s\n", ifname);

	ret = socket(AF_INET, SOCK_DGRAM, 0);

    if (ret < 0)
	{
		WL_ERR("create socket error = %d\n", ret);
		return WL_E_IOCTL_ERROR;
    }

	wl_ioctl_sockfd = ret;

    memset(&wl_ifr, 0, sizeof(struct ifreq));
    strncpy(wl_ifr.ifr_name, ifname, IFNAMSIZ);

	memcpy(&ifr, &wl_ifr, sizeof(struct ifreq));

	ret = ioctl(wl_ioctl_sockfd, SIOCGIFINDEX, &ifr);

    if (ret >= 0)
	{
		if (ifr.ifr_ifindex <= 0)
		{
			WL_ERR("invalid device %s\n", ifname);
			ret = WL_E_NODEVICE;
		}
    }

	if (ret < 0)
	{
		WL_ERR("ioctl error = %d\n", ret);
		close(wl_ioctl_sockfd);
		wl_ioctl_sockfd = -1;
		return WL_E_IOCTL_ERROR;
	}

    return WL_E_OK;
}

static int wl_ioctl_deinit(void)
{
	WL_DBG("enter\n");

	if (wl_ioctl_sockfd >= 0)
	{
		close(wl_ioctl_sockfd);
		wl_ioctl_sockfd = -1;
	}

	return WL_E_OK;
}

static int wl_event_init(void)
{
	int ret;

	WL_DBG("enter\n");

	ret = pthread_create(
				&wl_event_thread,
				NULL,
				wl_event_thread_func,
				NULL);

	if (ret < 0)
	{
		WL_ERR("create thread failed.\n");
		return WL_E_ERROR;
	}

	ret = wl_event_register(
				wl_connect_status_event, 
				sizeof(wl_connect_status_event) / sizeof(uint32_t),
				wl_connect_event_handler,
				NULL);

	return ret;
}

static int wl_event_deinit(void)
{
	WL_DBG("enter\n");

	wl_event_deregister(wl_connect_event_handler, NULL);

	WL_DBG("send close event to pipe\n");
   	write(wl_event_pipe[1], THREAD_CLOSE_EVENT, sizeof(THREAD_CLOSE_EVENT));

	WL_DBG("waiting event thread close\n");
    pthread_join(wl_event_thread, NULL);

	WL_DBG("exit\n");

	return WL_E_OK;
}

static void *wl_event_thread_func(void *args)
{
	int ret;
	int sockfd;
//	struct sockaddr_ll sll;
	char *buf = NULL;
	fd_set rfds;
	int fdsize;

	WL_DBG("enter\n");

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_BRCM));
	if (sockfd < 0)
	{
		WL_ERR("create socket err = %d\n", sockfd);
		return NULL;
	}

	FD_ZERO(&rfds);

	ret = pipe(wl_event_pipe);

	if (ret < 0)
	{
		WL_ERR("create pipe err = %d\n", ret);
		goto exit;
	}

	fdsize = MAX(sockfd, wl_event_pipe[0]);

#if (0)
	/* bind the socket first before starting escan so we won't miss any event */
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETHER_TYPE_BRCM);
	sll.sll_ifindex = wl_ifr.ifr_ifindex;

	ret = bind(fd, (struct sockaddr *)&sll, sizeof(sll));

	if (ret < 0) {
		WL_ERR("bind socket error = %d\n", ret);
		goto exit;
	}
#endif

	buf = (char*)malloc(WL_EVENT_BUF_SIZE);

	if (buf == NULL)
	{
		WL_ERR("alloc event buf failed.\n");
		goto pipeexit;
	}

	while (1)
	{
		int size;
		bcm_event_t *msg;

		WL_DBG("waiting event\n");

		FD_SET(sockfd, &rfds);
		FD_SET(wl_event_pipe[0], &rfds);

		ret = select(fdsize + 1, &rfds, NULL, NULL, NULL);

		if (ret < 0)
		{
			WL_ERR("select err = %d\n", ret);
			break;
		}

		if (FD_ISSET(wl_event_pipe[0], &rfds))
		{
			WL_ERR("Receive thread close event from pipe\n");
			break;
		}

		if (!FD_ISSET(sockfd, &rfds))
		{
			WL_ERR("unknown fd\n");
			break;
		}

		size = recv(sockfd, buf, WL_EVENT_BUF_SIZE, 0);

		WL_DBG("recv buf size = %d\n", size);

		if (size >= sizeof(bcm_event_t))
		{
			int i;

			msg = (bcm_event_t*)buf;

            if (memcmp(BRCM_OUI, &msg->bcm_hdr.oui[0], DOT11_OUI_LEN) != 0)
            {
				WL_INFO("Event OUI mismatch\n");
                continue;
            }

            msg->event.flags = NTOH16(msg->event.flags);
		    msg->event.event_type = NTOH32(msg->event.event_type);
            msg->event.status = NTOH32(msg->event.status);
            msg->event.reason = NTOH32(msg->event.reason);
            msg->event.auth_type = NTOH32(msg->event.auth_type);
            msg->event.datalen = NTOH32(msg->event.datalen);

			WL_DBG("recv event = %d\n", msg->event.event_type);

			if (msg->event.event_type > WLC_E_LAST)
			{
				WL_INFO("Invalid event\n");
				continue;
			}

			for (i = 0; i < wl_event_num; i++)
			{
				wl_event_item_t *item = &wl_event_list[i];

				if (ISSET(item->event_mask, msg->event.event_type))
				{
					item->callback(&msg->event, item->user_data);
				}
			}
		}
	}

pipeexit:
	close(wl_event_pipe[0]);
	close(wl_event_pipe[1]);

exit:
	if (buf != NULL)
	{
		free(buf);
	}

	close(sockfd);

	pthread_exit(NULL);

	return NULL;
}

static int wl_connect_status_update(bool is_connected)
{
    WL_DBG("is_connected = %d\n", is_connected);

    if (wl_connect_status_callback != NULL)
    {
        wl_connect_status_callback(is_connected, wl_connect_status_userdata);
    }
}

static void wl_connecting_ap_handler(wl_event_msg_t* msg, void* user_data)
{
	bool is_join_completed = false;

	WL_DBG("Before event handler, join state = 0x%08x\n", wl_join_status);

	switch(msg->event_type)
	{
        case WLC_E_PSK_SUP:
			WL_DBG("WLC_E_PSK_SUP, status = %d\n", msg->status);
            /* Ignore WLC_E_PSK_SUP event if link is not up */
            if (wl_join_status & JOIN_LINK_READY)
            {
                if (msg->status == WLC_SUP_KEYED)
                {
                    /* Successful WPA key exchange */
                    wl_join_status |= JOIN_SECURITY_COMPLETE;
                }
                else
                {
                    /* join has completed with an error */
                    is_join_completed = true;
                    if ((msg->status == WLC_SUP_KEYXCHANGE_WAIT_M1) && (msg->reason == WLC_E_SUP_WPA_PSK_TMO))
                    {
                        /* A timeout waiting for M1 may occur at the edge of the cell or if the AP is particularly slow. */
                        WL_DBG("Supplicant M1 timeout event\n");
                        wl_join_status |= JOIN_EAPOL_KEY_M1_TIMEOUT;
                    }
                    else if ((msg->status == WLC_SUP_KEYXCHANGE_WAIT_M3) && ( msg->reason == WLC_E_SUP_WPA_PSK_TMO))
                    {
                        /* A timeout waiting for M3 is an indicator that the passphrase may be incorrect. */
                        WL_DBG("Supplicant M3 timeout event\n");
                        wl_join_status |= JOIN_EAPOL_KEY_M3_TIMEOUT;
                    }
                    else if ((msg->status == WLC_SUP_KEYXCHANGE_WAIT_G1) && (msg->reason == WLC_E_SUP_WPA_PSK_TMO))
                    {
                        /* A timeout waiting for G1 (group key) may occur at the edge of the cell. */
                        WL_DBG("Supplicant G1 timeout event\n");
                        wl_join_status |= JOIN_EAPOL_KEY_G1_TIMEOUT;
                    }
                    else
                    {
                        WL_DBG("Unsuccessful supplicant event; status=0x%x\n", msg->status);
                        /* Unknown failure during EAPOL key handshake */
                        wl_join_status |= JOIN_EAPOL_KEY_FAILURE;
                    }
                }
            }
            break;

        case WLC_E_JOIN:
        case WLC_E_SET_SSID:
			WL_DBG("WLC_E_SET_SSID, status = %d\n", msg->status);
            if (msg->status == WLC_E_STATUS_SUCCESS)
            {
                /* SSID has been successfully set. */
                wl_join_status |= JOIN_SSID_SET;
            }
            else if ((msg->status & 0xFF) == WLC_E_STATUS_NO_NETWORKS)
            {
                wl_join_status |= JOIN_NO_NETWORKS;
                is_join_completed = true;
            }
            else
            {
                is_join_completed = true;
            }
            break;

        case WLC_E_LINK:
			WL_DBG("WLC_E_LINK, flags = 0x%08x\n", msg->flags);
			if (msg->flags & WLC_EVENT_MSG_LINK)
            {
                wl_join_status |= JOIN_LINK_READY;
            }
            else
            {
                wl_join_status &= ~JOIN_LINK_READY;
            }
            break;

        case WLC_E_DEAUTH_IND:
        case WLC_E_DISASSOC_IND:
			WL_DBG("Disconnect\n");
            wl_join_status &= ~(JOIN_AUTHENTICATED | JOIN_LINK_READY);
            break;

        case WLC_E_AUTH:
			WL_DBG("WLC_E_AUTH, status = %d\n", msg->status);
            if (msg->status == WLC_E_STATUS_SUCCESS)
            {
                wl_join_status |= JOIN_AUTHENTICATED;
            }
            else if (msg->status == WLC_E_STATUS_UNSOLICITED)
            {
                WL_DBG("Ignore UNSOLICITED pkt event\n");
            }
            else
            {
                /* We cannot authenticate. Perhaps we're blocked or at the edge of a cell. */
                is_join_completed = true;
            }
            break;

		default:
            break;
	}

	WL_DBG("After event handler, join state = 0x%08x, connect status = %d\n", wl_join_status, wl_connect_status);

    if (wl_join_status == JOIN_COMPLETED)
    {
        is_join_completed = true;
    }

    WL_DBG("is_join_completed = %d\n", is_join_completed);

    if (is_join_completed)
    {
        if (wl_join_status == JOIN_COMPLETED)
        {
            wl_connect_status = WL_CONNECT_STATUS_CONNECTED;
        }
        else
        {
            wl_connect_status = WL_CONNECT_STATUS_DISCONNECTED;
        }

		wl_join_wake();

        if (wl_connect_status == WL_CONNECT_STATUS_CONNECTED)
        {
            wl_connect_status_update(true);
        }
    }
}

static void wl_connect_status_change_handler(wl_event_msg_t* msg, void* user_data)
{
	bool is_join_completed = false;
    uint32_t old_status = wl_connect_status;

	WL_DBG("connect status = %d\n", wl_connect_status);

	switch(msg->event_type)
	{
        case WLC_E_LINK:
			WL_DBG("WLC_E_LINK, flags = 0x%08x\n", msg->flags);
			if (msg->flags & WLC_EVENT_MSG_LINK)
            {
                wl_connect_status = WL_CONNECT_STATUS_CONNECTED;                
            }
            else
            {
                wl_connect_status = WL_CONNECT_STATUS_DISCONNECTED;                
            }
            break;

        case WLC_E_DEAUTH_IND:
        case WLC_E_DISASSOC_IND:
			WL_DBG("Disconnect\n");
            wl_connect_status = WL_CONNECT_STATUS_DISCONNECTED;                
            break;

		default:
            break;
	}

    if (old_status != wl_connect_status)
    {
        bool is_connected = (wl_connect_status == WL_CONNECT_STATUS_CONNECTED);
    
        wl_connect_status_update(is_connected);
    }
}

static void wl_connect_event_handler(wl_event_msg_t* msg, void* user_data)
{
	if (wl_connect_status == WL_CONNECT_STATUS_CONNECTING)
    {
        wl_connecting_ap_handler(msg, user_data);
    }
    else
    {
        wl_connect_status_change_handler(msg, user_data);
    }
}

int wl_join_init(uint32_t security)
{
	WL_DBG("security = 0x%08x\n", security);

	if ((security == WL_SECURITY_OPEN) ||
		(security == WL_SECURITY_WPS_OPEN) ||
		(security == WL_SECURITY_WPS_SECURE))
	{
		wl_join_status = JOIN_SECURITY_COMPLETE;
	}
	else
	{
		wl_join_status = 0;
	}

	wl_connect_status = WL_CONNECT_STATUS_CONNECTING;

	return 0;
}

int wl_join_wait(uint32_t timeout)
{
	WL_DBG("timeout = %d\n", timeout);
	return wl_wait(&wl_connect_sem, timeout);
}

int wl_join_wake(void)
{
	WL_DBG("enter\n");

	sem_post(&wl_connect_sem);

	return 0;
}

int wl_join_error(void)
{
	if (wl_join_status == JOIN_COMPLETED)
	{
		return WL_E_OK;
	}
	else if (wl_join_status & JOIN_NO_NETWORKS)
	{
		return WL_E_NO_NETWORK;
	}
	else
	{
		return WL_E_ERROR;
	}
}

int wl_wait(sem_t* sem, uint32_t timeout)
{
	struct timespec ts;
	int ret;

	clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000;
    ts.tv_nsec += (timeout % 1000) * 1000;

	ret = sem_timedwait(sem, &ts);

	if (ret == -ETIMEDOUT)
	{
		WL_DBG("semaphore timeout\n");
		return WL_E_TIMEOUT;
	}
	else
	{
		return WL_E_OK;
	}
}

int wl_wake(sem_t* sem)
{
	return sem_post(sem);
}

int wl_event_register(
		const uint32_t *event_type,
		uint32_t event_num,
		wl_event_cb_t callback,
		void* user_data)
{
	int ret = WL_E_OK;
#ifdef WL_CONFIG_CHECK_EVENT
	uint8_t event_mask[WL_EVENTING_MASK_LEN];
	uint32_t count = 0;
#endif /* WL_CONFIG_CHECK_EVENT */
	wl_event_item_t *item;
	int i;

	WL_DBG("callback = 0x%08x, user_data = 0x%08x, wl_event_num = %d\n",
		(int)callback, (int)user_data, wl_event_num);

	pthread_mutex_lock(&wl_mutex);

	if (wl_event_num == WL_EVENT_LIST_SIZE)
	{
		return -1;
	}

#ifdef WL_CONFIG_CHECK_EVENT
	memset(event_mask, 0, WL_EVENTING_MASK_LEN);

	ret = wl_iovar_get("event_msgs", event_mask, WL_EVENTING_MASK_LEN);
	if (ret < 0)
	{
		return ret;
	}
#endif /* WL_CONFIG_CHECK_EVENT */

	item = &wl_event_list[wl_event_num];

	for (i = 0; i < event_num; i++)
	{
		if (event_type[i] > WL_EVENT_TYPE_MAX)
		{
			return -1;
		}
	
#ifdef WL_CONFIG_CHECK_EVENT
		if (!ISSET(event_mask, event_type[i]))
		{
			SETBIT(event_mask, event_type[i]);
			SETBIT(item->reg_event_mask, event_type[i]);			
			count++;
		}
#endif /* WL_CONFIG_CHECK_EVENT */

		SETBIT(item->event_mask, event_type[i]);	
	}

#ifdef WL_CONFIG_CHECK_EVENT
	if (count > 0)
	{
		item->is_event_set = true;

		ret = wl_iovar_set("event_msgs", &event_mask, WL_EVENTING_MASK_LEN);

		if (ret < 0)
		{
			return ret;
		}
	}
#endif /* WL_CONFIG_CHECK_EVENT */

	item->callback = callback;
	item->user_data = user_data;

	wl_event_num++;

	pthread_mutex_unlock(&wl_mutex);

	return ret;
}

int wl_event_deregister(wl_event_cb_t callback, void* user_data)
{
	int i;
	wl_event_item_t *item = NULL;

	WL_DBG("callback = 0x%08x, user_data = 0x%08x, wl_event_num = %d\n",
		(int)callback, (int)user_data, wl_event_num);

	pthread_mutex_lock(&wl_mutex);

	for (i = 0; i < wl_event_num; i++)
	{
		if ((wl_event_list[i].callback == callback) &&
			(wl_event_list[i].user_data == user_data))
		{
			item = &wl_event_list[i];
			break;
		}
	}

	if (item == NULL)
	{
		return 0;
	}

#ifdef WL_CONFIG_CHECK_EVENT
	if (item->is_event_set)
	{
		int ret;
		uint8_t event_mask[WL_EVENTING_MASK_LEN];

		ret = wl_iovar_get("event_msgs", event_mask, WL_EVENTING_MASK_LEN);
		if (ret == 0)
		{
			for (int j = 0; j < WL_EVENTING_MASK_LEN; i++)
			{
				event_mask[j] &= (~item->reg_event_mask[j]);
			}

			wl_iovar_set("event_msgs", &event_mask, WL_EVENTING_MASK_LEN);
		}
	}
#endif /* WL_CONFIG_CHECK_EVENT */

	if (i < (wl_event_num - 1))
	{
		memcpy(
			item,
			&wl_event_list[i + 1],
			sizeof(wl_event_item_t) * (wl_event_num - i - 1));
	}

	wl_event_num--;

	memset(&wl_event_list[wl_event_num], 0, sizeof(wl_event_item_t));

	pthread_mutex_unlock(&wl_mutex);

	return 0;
}

int wl_connect_status_register(wl_connect_status_cb_t callback, void* user_data)
{
    wl_connect_status_callback = callback;
    wl_connect_status_userdata = user_data;
}

int wl_connect_status_deregister(wl_connect_status_cb_t callback)
{
    wl_connect_status_callback = NULL;
    wl_connect_status_userdata = NULL;
}

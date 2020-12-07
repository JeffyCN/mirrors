#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "wl_internal.h"
#include "wl_ioctl.h"
#include "wl_debug.h"

#define CONNECT_TIMEOUT_MS			(200 * 1000)
#define CONNECT_CHECK_TIMES			(50)

#define FAKEARP_TCPKA_SESSION_ID    (1)
#define DEFAULT_TCPKA_SESSION_ID    (2)
#define FAKEARP_TCP_SRC_PORT    	(1)
#define WL_JOIN_TIMEOUT				(7000)
#define WL_SCAN_PROBE_INTERVAL		(20)

#define BCNTRIM_SKIP_BEACON			(8)

int wl_get_bssid(wl_ether_addr_t *bssid)
{
	return wl_ioctl(WLC_GET_BSSID, bssid, sizeof(wl_ether_addr_t), false);
}

int wl_get_channel(uint32_t *channel)
{
	int ret;
	wl_channel_info_t info;

	memset(&info, 0, sizeof(wl_channel_info_t));

	ret = wl_ioctl(WLC_GET_CHANNEL, &info, sizeof(wl_channel_info_t), false);

	if (ret == 0)
	{
	    *channel = (uint32_t)info.hw_channel;
	}
	
	return ret;
}

int wl_get_rssi(int32_t *rssi)
{
    return wl_ioctl(WLC_GET_RSSI, rssi, sizeof(uint32_t), false);
}

static int wl_reset_pre_pmk(void)
{
    uint8_t tmp_pmk[32] = { 0 };

	/* Check Parameters */

	return wl_iovar_set("sup_wpa_pre_passhash_pmk", tmp_pmk, 32);
}

static int wl_set_security(uint32_t security)
{
    int ret;
	uint32_t wsec;
	uint32_t sup_wpa = 0;
	uint32_t auth;
	uint32_t wpa_auth;

	wl_reset_pre_pmk();

	wsec = ((security & 0xFF) & ~WPS_ENABLED);

	WL_DBG("wsec = 0x%0x\n", wsec);

    /* Set Wireless Security Type */
	ret = wl_ioctl(WLC_SET_WSEC, &wsec, sizeof(uint32_t), true);
	if (ret < 0)
	{
		return ret;
	}

	if ((security & (WPA_SECURITY | WPA2_SECURITY)) != 0)
	{
		sup_wpa = 1;
	}

	WL_DBG("set sup_wpa\n");
	ret = wl_iovar_setint_bsscfg("sup_wpa", sup_wpa, 0);
	if (ret < 0)
	{
		return ret;
	}

    /* Set authentication type */
    if (security == WL_SECURITY_WEP_SHARED)
    {
        auth = 1; /* 1 = Shared Key authentication */
    }
    else
    {
        auth = 0; /*  0 = Open System authentication */
    }

	WL_DBG("auth = %d\n", auth);

	ret = wl_ioctl(WLC_SET_AUTH, &auth, sizeof(uint32_t), true);
	if (ret < 0)
	{
		return ret;
	}

    switch (security)
    {
        case WL_SECURITY_OPEN:
        case WL_SECURITY_WPS_OPEN:
        case WL_SECURITY_WPS_SECURE:
            wpa_auth = WL_WPA_AUTH_DISABLED;
            break;

        case WL_SECURITY_WPA_TKIP_PSK:
        case WL_SECURITY_WPA_AES_PSK:
        case WL_SECURITY_WPA_MIXED_PSK:
            wpa_auth = WL_WPA_AUTH_PSK;
            break;
        case WL_SECURITY_WPA2_AES_PSK:
        case WL_SECURITY_WPA2_TKIP_PSK:
        case WL_SECURITY_WPA2_MIXED_PSK:
            wpa_auth = WL_WPA2_AUTH_PSK;
            break;

        case WL_SECURITY_WPA_TKIP_ENT:
        case WL_SECURITY_WPA_AES_ENT:
        case WL_SECURITY_WPA_MIXED_ENT:
            wpa_auth = WL_WPA_AUTH_UNSPECIFIED;
            break;

        case WL_SECURITY_WPA2_TKIP_ENT:
        case WL_SECURITY_WPA2_AES_ENT:
        case WL_SECURITY_WPA2_MIXED_ENT:
            wpa_auth = WL_WPA2_AUTH_UNSPECIFIED;
            break;

        case WL_SECURITY_WEP_PSK:
        case WL_SECURITY_WEP_SHARED:
            wpa_auth = WL_WPA_AUTH_DISABLED;
            break;

        default:
            wpa_auth = WL_WPA_AUTH_DISABLED;
            break;
    }

	WL_DBG("wpa_auth = 0x%x\n", wpa_auth);

	ret = wl_ioctl(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(uint32_t), true);

	return ret;
}

static int wl_set_passphrase(const int8_t* security_key, uint8_t key_length)
{
    wl_wsec_pmk_t psk;

	WL_DBG("enter\n");

    memset(&psk, 0, sizeof(wl_wsec_pmk_t));
    memcpy(psk.key, security_key, key_length);
    psk.key_len = key_length;
    psk.flags = WSEC_PASSPHRASE;

    return wl_ioctl(WLC_SET_WSEC_PMK, &psk, sizeof(wl_wsec_pmk_t), true);
}

static int wl_set_wep_key(const int8_t* security_key, uint8_t key_length)
{
    int ret = WL_E_OK;
	int32_t i;

	WL_DBG("enter\n");

	for (i = 0; i < key_length; i = (uint16_t)(i + 2 + security_key[1]))
	{
    	const wl_wep_key_t* in_key = (const wl_wep_key_t*)&security_key[i];
    	wl_wsec_key_t out_key;

	    memset(&out_key, 0, sizeof(wl_wsec_key_t));

	    out_key.index = in_key->index;
    	out_key.len = in_key->length;
    	memcpy(out_key.data, in_key->data, in_key->length);

		switch (in_key->length)
    	{
        	case 5:
            	out_key.algo = CRYPTO_ALGO_WEP1;
            	break;
        	case 13:
            	out_key.algo = CRYPTO_ALGO_WEP128;
            	break;
        	case 16:
            	/* default to AES-CCM */
            	out_key.algo = CRYPTO_ALGO_AES_CCM;
            	break;
	        case 32:
	            out_key.algo = CRYPTO_ALGO_TKIP;
	            break;
	        default:
	            return WL_E_BADARG;
   		}

	    /* Set the first entry as primary key by default */
	    if (i == 0)
	    {
	        out_key.flags |= WL_PRIMARY_KEY;
	    }

		ret = wl_ioctl(WLC_SET_KEY, &out_key, sizeof(wl_wep_key_t), true);

		if (ret != WL_E_OK)
		{
			break;
		}
	}

	return ret;
}

int wl_set_security_key(
		uint32_t security,
		const int8_t* security_key,
		uint8_t key_length)
{
    int ret;

	WL_DBG("enter\n");

    switch (security)
    {
        case WL_SECURITY_OPEN:
        case WL_SECURITY_IBSS_OPEN:
        case WL_SECURITY_WPS_OPEN:
        case WL_SECURITY_WPS_SECURE:
        case WL_SECURITY_WPA_TKIP_ENT:
        case WL_SECURITY_WPA_AES_ENT:
        case WL_SECURITY_WPA_MIXED_ENT:
        case WL_SECURITY_WPA2_TKIP_ENT:
        case WL_SECURITY_WPA2_AES_ENT:
        case WL_SECURITY_WPA2_MIXED_ENT:
			ret = WL_E_OK;
            break;

        case WL_SECURITY_WPA_TKIP_PSK:
        case WL_SECURITY_WPA_AES_PSK:
        case WL_SECURITY_WPA_MIXED_PSK:
        case WL_SECURITY_WPA2_AES_PSK:
        case WL_SECURITY_WPA2_TKIP_PSK:
        case WL_SECURITY_WPA2_MIXED_PSK:
            ret = wl_set_passphrase(security_key, key_length);
            break;

        case WL_SECURITY_WEP_PSK:
        case WL_SECURITY_WEP_SHARED:
			ret = wl_set_wep_key(security_key, key_length);            
            break;

        default:
			ret = WL_E_ERROR;
            break;
    }

	return ret;
}

int wl_join_ap(
		wl_ssid_t* ssid,
		uint32_t security,
		const int8_t* security_key,
		uint8_t key_length)
{
    int ret;

	WL_DBG("ssid = %s, security = 0x%08x, key = %s\n", ssid->value, security, security_key);

	if (ssid->len > WL_MAX_SSID_LEN)
	{
		return WL_E_BADARG;
	}

	ret = wl_set_security(security);
	if (ret < 0)
	{
		return ret;
	}

	if ((security_key != NULL) && (key_length > 0))
	{
		ret = wl_set_security_key(security, security_key, key_length);
	}

	wl_join_init(security);

    if (ret == 0)
    {
		WL_DBG("WLC_SET_SSID\n");

		ret = wl_ioctl(WLC_SET_SSID, ssid, sizeof(wl_ssid_t), true);

		ret = wl_join_wait(WL_JOIN_TIMEOUT);

		if (ret == WL_E_OK)
		{
			ret = wl_join_error();
		}
    }

	return ret;
}

int wl_disassoc(void)
{
	return wl_ioctl(WLC_DISASSOC, NULL, 0, true);
}

int wl_set_pm_mode(uint32_t mode)
{
	if (mode == WL_PM_FAST)
	{
		wl_iovar_setint("pm2_sleep_ret", WL_CONFIG_PM2_SLEEP_RET);
	}

	return wl_ioctl(WLC_SET_PM, &mode, sizeof(uint32_t), true);
}

int wl_set_powersave(bool enable)
{
	uint32_t pm = enable ? WL_CONFIG_PM_MODE : WL_PM_OFF;

	return wl_set_pm_mode(pm);
}

typedef struct
{
	pthread_mutex_t mutex;
	bool is_completed;
	int status;
	int fetch;
	int total;
	wl_ap_info_t ap_info[WL_SCAN_AP_MAX];
} wl_scan_cntx_t;

static const uint32_t wl_escan_event[] = {
						WLC_E_ESCAN_RESULT
					};
static wl_scan_cntx_t *wl_scan_cntx;

static void wl_escan_event_handler(wl_event_msg_t *event, void* userdata)
{
	wl_scan_cntx_t* cntx = (wl_scan_cntx_t*)userdata;

	WL_DBG("enter\n");

	if (event->event_type != WLC_E_ESCAN_RESULT)
	{
		return;
	}

	if ((cntx != wl_scan_cntx) ||
		(cntx->is_completed == true) ||
		(cntx->status != WL_E_OK))
	{
		WL_DBG("scan is already done\n");
		return;
	}

	WL_DBG("status = %d\n", event->status);

	pthread_mutex_lock(&cntx->mutex);

	if (event->status == WLC_E_STATUS_PARTIAL)
	{
		wl_escan_result_t *escan_data = (wl_escan_result_t*)(&event[1]);
		wl_bss_info_t *bi = &escan_data->bss_info[0];
		wl_ap_info_t *tmp;
		int i;

		if (cntx->total == WL_SCAN_AP_MAX)
		{
			WL_DBG("Buffer is full\n");
			goto exit;
		}

		for (i = 0; i < cntx->total; i++)
		{
			wl_ap_info_t *tmp = &cntx->ap_info[i];
		
			if ((tmp->ssid.len == bi->SSID_len) &&
				(memcmp(tmp->ssid.value, bi->SSID, bi->SSID_len) == 0) &&
				(memcmp(&tmp->bssid, &bi->BSSID, sizeof(wl_ether_addr_t)) == 0))
			{
				WL_DBG("Same SSID: %s\n", bi->SSID);
				goto exit;
			}
		}

		tmp = &cntx->ap_info[cntx->total];
		wl_convert_bss_info(tmp, bi);

		cntx->total++;

		WL_DBG("total: %d\n", cntx->total);
	}
	else if (event->status == WLC_E_STATUS_SUCCESS)
	{
		WL_DBG("scan is done\n");
		cntx->is_completed = true;
	}
	else if (event->status != WL_SCAN_RESULTS_PENDING)
	{
		cntx->status = WL_E_ERROR;
	}

exit:
	pthread_mutex_unlock(&cntx->mutex);
	WL_DBG("exit\n");
}

static wl_scan_cntx_t* wl_scan_init(void)
{
	int ret;
	wl_scan_cntx_t* cntx;

	WL_DBG("enter\n");

	cntx = (wl_scan_cntx_t*)malloc(sizeof(wl_scan_cntx_t));

	if (cntx == NULL)
	{
		WL_ERR("alloc buffer failed\n");
		return NULL;
	}

	memset(cntx, 0, sizeof(wl_scan_cntx_t));

	pthread_mutex_init(&cntx->mutex, NULL);

	ret = wl_event_register(
				wl_escan_event,
				sizeof(wl_escan_event) / sizeof(uint32_t),
				wl_escan_event_handler,
				cntx);

	if (ret != WL_E_OK)
	{
		pthread_mutex_destroy(&cntx->mutex);
		free(cntx);
		cntx = NULL;
	}
	
	return cntx;
}

static void wl_scan_deinit(wl_scan_cntx_t* cntx)
{
	WL_DBG("enter\n");

	if (cntx != NULL)
	{
		wl_event_deregister(wl_escan_event_handler, cntx);
		pthread_mutex_destroy(&cntx->mutex);
		free(cntx);
	}
}

int wl_scan(
		const wl_ssid_t* ssid,
		const wl_ether_addr_t* bssid,
		const uint16_t* channel_list,
		int32_t channel_num,
		int32_t time_per_channel)
{
    int ret;
	wl_escan_params_t params;
	int32_t paramlen = WL_ESCAN_PARAMS_FIXED_SIZE;

	WL_DBG("enter\n");

	if (wl_scan_cntx != NULL)
	{
		if ((wl_scan_cntx->is_completed == true) ||
			(wl_scan_cntx->status != WL_E_OK))
		{
			wl_scan_deinit(wl_scan_cntx);
		}
		else
		{
			/* Already have a scan is running. */
			WL_DBG("scan is running\n");
			return WL_E_BUSY;
		}
	}

	wl_scan_cntx = wl_scan_init();

	if (wl_scan_cntx == NULL)
	{
		return WL_E_NOMEM;
	}

	memset(&params, 0, sizeof(wl_escan_params_t));

	params.version = WL_ESCAN_REQ_VERSION;
	params.action = WL_SCAN_ACTION_START;
	params.sync_id = WL_ESCAN_SYNC_ID;
	params.params.bss_type = DOT11_BSSTYPE_ANY;
	params.params.scan_type = 0;
	params.params.active_time = time_per_channel;
	if (time_per_channel > WL_SCAN_PROBE_INTERVAL)
	{
		params.params.nprobes = time_per_channel / WL_SCAN_PROBE_INTERVAL;
	}
	else
	{
		params.params.nprobes = -1;
	}
	params.params.passive_time = -1;
	params.params.home_time = -1;

    if (ssid != NULL)
    {
        memcpy(&params.params.ssid, ssid, sizeof(wl_ssid_t));
    }

    if (bssid != NULL)
    {
         memcpy(&params.params.bssid, bssid, sizeof(wl_ether_addr_t));
    }
    else
    {
		memset(&params.params.bssid, 0xFF, sizeof(wl_ether_addr_t));
    }

	if ((channel_list != NULL) && (channel_num > 0))
	{
        int32_t i;

		if (channel_num > WL_NUMCHANNELS)
		{
			channel_num = WL_NUMCHANNELS;
		}

        for (i = 0; i < channel_num; i++)
        {
        	params.params.channel_list[i] = CH20MHZ_CHSPEC(channel_list[i]);			
        }

		params.params.channel_num = channel_num;
		paramlen += sizeof(uint16_t) * channel_num;
	}
	else
	{
		params.params.channel_num = 0;
	}

	ret = wl_iovar_set("escan", &params, paramlen);

	if (ret != WL_E_OK)
	{
		wl_scan_deinit(wl_scan_cntx);
		wl_scan_cntx = NULL;
	}

	return ret;
}

int wl_get_scan_results(
		wl_ap_info_t* ap_info,
		uint32_t* num,
		bool* is_completed)
{
	wl_scan_cntx_t *cntx = wl_scan_cntx;
	int ret = WL_E_OK;
	int size = *num;
	int remain;

	WL_DBG("enter\n");

	if (cntx == NULL)
	{
		WL_INFO("Scan is done\n");
		*num = 0;
		*is_completed = true;
		return WL_E_OK;
	}

	pthread_mutex_lock(&cntx->mutex);

	WL_DBG("fetch = %d, total = %d\n", cntx->fetch, cntx->total);

	remain = cntx->total - cntx->fetch;

	if ((remain == 0) && 
		(cntx->status == WL_E_OK) &&
		(cntx->is_completed == false))
	{
		WL_INFO("Waiting scan result\n");
		pthread_mutex_unlock(&cntx->mutex);
		usleep(100000);
		pthread_mutex_lock(&cntx->mutex);
	}

	ret = cntx->status;

	if (ret != WL_E_OK)
	{
		*num = 0;
		*is_completed = true;
		goto exit;
	}

	if (remain < size)
	{
		size = remain;
	}

	if (size > 0)
	{
		memcpy(ap_info, &cntx->ap_info[cntx->fetch], sizeof(wl_ap_info_t) * size);
		cntx->fetch += size;
	}

	*num = size;

	WL_DBG("size = %d, is_completed = %d\n", size, cntx->is_completed);

	if (cntx->fetch < cntx->total)
	{
		*is_completed = false;
	}
	else
	{
		*is_completed = cntx->is_completed;
	}

exit:
	if ((ret != WL_E_OK) || cntx->is_completed)
	{
		printf("#########################\n");
		wl_scan_deinit(cntx);
		wl_scan_cntx = NULL;
	}
	pthread_mutex_unlock(&cntx->mutex);
	printf("exit\n");
	WL_DBG("exit\n");

	return ret;
}

int wl_set_hostsleep(bool sleep)
{
	int ret;

	ret = wl_iovar_setint("hostsleep", sleep);

	usleep(10000);

	if (ret == WL_E_OK)
	{
		printf("wl_dhd_iovar_setint set 1 ...\n");
		ret = wl_dhd_iovar_setint("sleep", 1);
	}

	usleep(100000);

	return ret;
}

int wl_set_deepsleep(bool deepsleep)
{
#ifdef WL_CONFIG_DEEPSLEEP
	return wl_iovar_setint("deepsleep", deepsleep);
#else /* WL_CONFIG_DEEPSLEEP */
	int skip = deepsleep ? BCNTRIM_SKIP_BEACON : 0;

	return wl_iovar_setint("bcntrim", skip);
#endif /* WL_CONFIG_DEEPSLEEP */
}

static int wl_wowl_pattern_add(
				uint32_t match_offset,
				const int8_t* pattern,
				uint32_t pattern_size)
{
    int ret;
    wl_wowl_pattern_t *wowl_pattern;
	int8_t smbuf[WLC_IOCTL_SMLEN * 2];
    int8_t* data = smbuf;
    uint32_t datalen;
	uint32_t mask_size = (pattern_size + 7) / 8;
	int32_t i;

	datalen = sizeof("wowl_pattern");
    strncpy((char*)data, "wowl_pattern", datalen);
	data += datalen;

    memcpy(data, "add", sizeof("add"));
    data += sizeof("add");
	datalen += sizeof("add");

    wowl_pattern = (wl_wowl_pattern_t*)data;
    wowl_pattern->masksize = mask_size;
    wowl_pattern->offset = match_offset;
    wowl_pattern->patternoffset = mask_size + sizeof(wl_wowl_pattern_t);
    wowl_pattern->patternsize = pattern_size;

    data += sizeof(wl_wowl_pattern_t);
	datalen += sizeof(wl_wowl_pattern_t);

	/* Set mask */
	for (i = 0; i < pattern_size; i++) {
		data[i / 8] |= 1 << (i % 8);		
	}
    data += mask_size;
	datalen += mask_size;

    memcpy(data, pattern, pattern_size);
	datalen += pattern_size;

	ret = wl_ioctl(WLC_SET_VAR, smbuf, datalen, true);

	return ret;
}

int wl_wowl_enable(
		uint32_t caps,
		uint32_t match_offset,
		const int8_t* pattern,
		uint32_t pattern_size)
{
    int ret;

	ret = wl_wowl_pattern_add(match_offset, pattern, pattern_size);
	if (ret < 0)
	{
		return 0;
	}

	ret = wl_iovar_setint("wowl", caps);
	if (ret < 0)
	{
		return 0;
	}

	ret = wl_iovar_setint("wowl_activate", 1);

	return ret;
}

int wl_wowl_disable(void)
{
    int ret;

	ret = wl_iovar_setint("wowl", 0);
	if (ret < 0)
	{
		return ret;
	}

	ret = wl_iovar_set("wowl_pattern", "clr",  sizeof("clr") + sizeof(wl_wowl_pattern_t));
	if (ret < 0)
	{
		return ret;
	}

	ret = wl_iovar_setint( "wowl_clear", 1);

	return ret;
}

int wl_wowl_get_wakeind(uint32_t* wakeind)
{
    int ret;
    wl_wowl_wakeind_t wowl_wakeind;

    ret = wl_iovar_get(
				"wowl_wakeind",
				&wowl_wakeind,
				sizeof(wl_wowl_wakeind_t));

	if (ret == 0)
	{
	    *wakeind = wowl_wakeind.pci_wakeind;
	}

    return ret;
}

int wl_wowl_clear_wakeind(void)
{
	return wl_iovar_set("wowl_wakeind", "clear", sizeof(wl_wowl_wakeind_t));
}

static int wl_tcpka_conn_add(wl_tcpka_param_t *tcpka_param)
{
	int ret;
	int8_t smbuf[WLC_IOCTL_SMLEN];
    wl_tcpka_conn_add_multi_t add_param;
    wl_tcpka_conn_enable_t enable_param;
	uint32_t sess_id;

	WL_DBG("enter\n");

	memset(&add_param, 0, sizeof(wl_tcpka_conn_add_multi_t));
	memset(&enable_param, 0, sizeof(wl_tcpka_conn_enable_t));

    add_param.sess_id = 0;
    memcpy(&add_param.dst_mac, &tcpka_param->dst_mac, sizeof(wl_ether_addr_t));
    memcpy(&add_param.src_ip, &tcpka_param->src_ip, sizeof(wl_ipv4_addr_t));
    memcpy(&add_param.dst_ip, &tcpka_param->dst_ip, sizeof(wl_ipv4_addr_t));

    add_param.ipid = tcpka_param->ipid;
    add_param.srcport = tcpka_param->srcport;
    add_param.dstport = tcpka_param->dstport;
    add_param.seq = tcpka_param->seq;
    add_param.ack = tcpka_param->ack;
    add_param.tcpwin = tcpka_param->tcpwin;
    add_param.tsval = tcpka_param->tsval;
    add_param.tsecr = tcpka_param->tsecr;

    if (tcpka_param->payload_len > WL_TCPKA_PAYLOAD_SIZE)
    {
        add_param.data_len = WL_TCPKA_PAYLOAD_SIZE;
    }
    else
    {
        add_param.data_len = tcpka_param->payload_len;
    }
    memcpy(add_param.data, tcpka_param->payload, add_param.data_len);

	ret = wl_iovar_getbuf(
				"tcpka_conn_add",
				&add_param,
				sizeof(wl_tcpka_conn_add_multi_t),
				smbuf,
				WLC_IOCTL_SMLEN);
	if (ret < 0)
	{
		return ret;
	}

	sess_id = *(uint32_t*)smbuf;

	WL_DBG("sess_id = %d\n", sess_id);

    enable_param.sess_id = sess_id;
    enable_param.flag = 1;
    enable_param.interval = tcpka_param->interval;
    enable_param.retry_interval = tcpka_param->retry_interval;
    enable_param.retry_count = tcpka_param->retry_count;

	ret = wl_iovar_set("tcpka_conn_enable", &enable_param, sizeof(wl_tcpka_conn_enable_t));

    return ret;
}

static int wl_tcpka_conn_del(uint32_t sess_id)
{
    int ret;
    wl_tcpka_conn_enable_t param;

	memset(&param, 0, sizeof(wl_tcpka_conn_enable_t));
    param.sess_id = sess_id;
    param.flag = 0;

	ret = wl_iovar_set("tcpka_conn_enable", &param, sizeof(wl_tcpka_conn_enable_t));

	if (ret == WL_E_OK)
	{
		ret = wl_iovar_setint("tcpka_conn_del", sess_id);
	}

    if (ret < 0)
    {
		int error;

		error = wl_error();
	
	    /* If the session is not existed, consider this operation is success. */
		if (error == WL_E_NOTFOUND)
		{
	        ret = WL_E_OK;
		}
    }

    return ret;
}

int wl_tcpka_conn_enable(wl_tcpka_param_t *tcpka_param)
{
	int ret;
#ifdef WL_CONFIG_FAKEARP
    uint16_t srcport = tcpka_param->srcport;
#endif /* WL_CONFIG_FAKEARP */

	/* Set TCPKA Mode as Multi-Mode */
	ret = wl_iovar_setint("tcpka_conn_mode", 1);

#ifdef WL_CONFIG_FAKEARP
	/* Add Fake ARP */
	tcpka_param->srcport = FAKEARP_TCP_SRC_PORT;
    ret = wl_tcpka_conn_add(tcpka_param);
	if (ret < 0)
	{
		return ret;
	}

	/* Add TCPKA */
	tcpka_param->srcport = srcport;
#endif /* WL_CONFIG_FAKEARP */

	ret = wl_tcpka_conn_add(tcpka_param);

    return ret;
}

int wl_tcpka_conn_disable(void)
{
	int ret;

	/* Set TCPKA Mode as Multi-Mode */
	ret = wl_iovar_setint("tcpka_conn_mode", 1);

	ret = wl_tcpka_conn_del(FAKEARP_TCPKA_SESSION_ID);

#ifdef WL_CONFIG_FAKEARP
	if (ret < 0)
	{
		return ret;
	}
	
    ret = wl_tcpka_conn_del(DEFAULT_TCPKA_SESSION_ID);
#endif /* WL_CONFIG_FAKEARP */

	return ret;
}

int wl_set_listen_interval(uint32_t listen_interval)
{
	int ret;

	ret = wl_iovar_setint("assoc_listen", listen_interval);
	if (ret < 0)
	{
		return ret;
	}

	if (wl_is_associated())
	{
		uint32_t bcn_li_dtim;
	    uint32_t beacon_period;
	    uint32_t dtim;

		ret = wl_ioctl(WLC_GET_BCNPRD, &beacon_period, sizeof(uint32_t), false);
		if (ret < 0)
		{
			return ret;
		}

		ret = wl_ioctl(WLC_GET_DTIMPRD, &dtim, sizeof(uint32_t), false);
		if (ret < 0)
		{
			return ret;
		}

		if ((beacon_period == 0) || (dtim == 0))
		{
			return WL_E_NOTASSOCIATED;
		}

		bcn_li_dtim = listen_interval / (beacon_period * dtim);

		if (bcn_li_dtim == 0)
		{
			bcn_li_dtim = 1;
		}

		ret = wl_iovar_setint("bcn_li_dtim", bcn_li_dtim);
	}

	return ret;
}

int wl_reset_listen_interval(void)
{
	return wl_iovar_setint("bcn_li_dtim", 0);
}

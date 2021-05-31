#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <sys/time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "wl.h"
#include "wifi.h"

#if 0
#define DEFAULT_IFNAME				"wlan0"
#define JOIN_AP_RETRIES				1
#define MAX_SAME_AP_NUM				5
#define MAX_SCAN_NUM				10
#define TCP_CONNECT_RETRIES			3

#define TCPKA_INTERVAL 				60
#define TCPKA_RETRY_INTERVAL		4
#define TCPKA_RETRY_COUNT 			15
#define ETHERNET_HEADER_LEN			14
#define IPV4_HEADER_FIXED_LEN		20
#define TCP_HEADER_FIXED_LEN		20
#define TCP_OPTIONS_LEN				12

#ifdef TLS_KEEP_ALIVE
#define WOWL_PATTERN_MATCH_OFFSET   0
#else /* TLS_KEEP_ALIVE */
#define WOWL_PATTERN_MATCH_OFFSET	\
			(ETHERNET_HEADER_LEN + IPV4_HEADER_FIXED_LEN + TCP_HEADER_FIXED_LEN + TCP_OPTIONS_LEN)
#endif /* TLS_KEEP_ALIVE */

#define CUSTOM_LISTEN_INTERVAL		1000
#define WAKEUP_BY_SSID_SCAN_INTERVAL	(60)
#define SOL_TCP			6
#define TCP_EXT_INFO	37
#endif
#define WAKEUP_BY_SSID_SCAN_INTERVAL	(60)

#define PRINT_HEX(buf, len) \
			do { \
				int i; \
				uint8_t *tmp = (uint8_t*)buf; \
				printf("%s, " #buf "[%d] =", __FUNCTION__, (int)len); \
				for (i = 0; i < len; i++) \
				{ \
					if ((i % 16) == 0) \
					{ \
						printf("\n"); \
					} \
					printf("%02x", tmp[i]); \
				} \
				printf("\n"); \
			} while(0)
/*
struct tcp_ext_info {
	uint16_t ip_id;
	uint32_t snd_nxt;
	uint32_t rcv_nxt;
	uint32_t window;
	uint32_t tsval;
	uint32_t tsecr;
};
*/

const char user_payload[] = "UserPayload";
//const char tcpka_payload[] = "HeartBeats";
//const char wowl_pattern[] = "Wakeup";

static void usage(void)
{
	printf("usage:\n"
	       "	keepalive [-i <ifname>] -s <SSID> [-k <PASSWORD>] -a <Server IP ADDRESS> -p <Server TCP Port>\n");
}

static int network_up(char* ifname)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

	sprintf(cmd, "ifconfig %s up", ifname);
	printf("%s, CMD: %s\n", cmd, __FUNCTION__);

	ret = system(cmd);

	printf("%s, ret = %d\n", __FUNCTION__, ret);

	sleep(1);

	return ret;
}

static int connect_ap(char* ssid_name, char* password)
{
	int ret;
	wl_ssid_t ssid;
	uint32_t i;
	uint32_t j;
	wl_ap_info_t join_list[MAX_SAME_AP_NUM];
	uint32_t ap_count = 0;

	memset(&ssid, 0, sizeof(ssid));
	ssid.len = strlen(ssid_name);
	memcpy(ssid.value, ssid_name, ssid.len);

	printf("%s, enter\n", __FUNCTION__);

	if (password == NULL)
	{
		printf("Open security type\n");
		join_list[0].security = WL_SECURITY_OPEN;
		join_list[0].channel = 0;
		ap_count = 1;
	}
	else
	{
		printf("Scan for the security type\n");
		
		ret = wl_scan(&ssid, NULL, NULL, 0, WL_SCAN_PER_CH_TIME_DEFAULT);

		printf("wl_scan result = %d\n", ret);
		if (ret < 0)
		{
			return ret;
		}

		while (1)
		{
			wl_ap_info_t ap_info[MAX_SCAN_NUM];
			uint32_t count = MAX_SCAN_NUM;
			bool is_completed = false;

			ret = wl_get_scan_results(ap_info, &count, &is_completed);

			if (ret < 0)
			{
				printf("wl_get_scan_results failed, result = %d\n", ret);
				break;
			}
			else
			{
				for (i = 0; i < count; i++)
				{
					if (ap_count < MAX_SAME_AP_NUM)
					{
						printf("Get AP[%d] security = 0x%08x, channel = %d\n", 
							ap_count, ap_info[i].security, ap_info[i].channel);

						memcpy(&join_list[ap_count], &ap_info[i], sizeof(wl_ap_info_t));
						ap_count++;
					}
				}

				if (is_completed)
				{
					printf("Scan Done\n");
					break;
				}
			}
		}
	}

	if (ap_count > 0)
	{
		/* Try to join each of the found AP until successfully */
		for (i = 0; i < ap_count; i++)
		{
			uint32_t security = join_list[i].security;
		    struct timeval start_time;
			struct timeval end_time;
			double join_time;

			printf("Starting to join AP[%d] : %s, password: %s\n", i, ssid_name, password);

			/* Optional: Using higher security */
			switch (security)
			{
				case WL_SECURITY_WPA_MIXED_PSK:
					security = WL_SECURITY_WPA_AES_PSK;
					break;

				case WL_SECURITY_WPA2_MIXED_PSK:
					security = WL_SECURITY_WPA2_AES_PSK;
					break;

				default:
					break;
			}				

			for (j = 0; j < JOIN_AP_RETRIES; j++)
			{
				uint8_t key_length = 0;

				if (password != NULL)
				{
					key_length = strlen(password);
				}

				gettimeofday(&start_time, NULL);

				ret = wl_join_ap_specific(
							&ssid,
							security,
							(int8_t*)password,
							key_length,
							NULL,
							join_list[i].channel);

				if (ret < 0)
				{
					printf("Try to join AP %d %d times  failed, ret = %d\n", i, j, ret);
				}
				else
				{
					gettimeofday(&end_time, NULL);
					join_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
				 	printf("Joined AP in time (ms) = %lf\n", join_time / 1000.0);
					break;
				}
			}

			if (ret == WL_E_OK)
			{
				break;
			}
		}

		if (ret == WL_E_OK)
		{
			printf("Joined AP succcessfully\n");
		}
	}
	else
	{
		printf("Can't find AP: %s\n", ssid_name);
		return WL_E_NO_NETWORK;
	}

	return ret;
}

int get_local_ip(const char * ifname, char *local_ip)
{
	int ret;
 	int sock;
	struct ifreq ifr;

	printf("%s, enter\n", __FUNCTION__);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
	{
		printf("%s, create socket err = %d\n", __FUNCTION__, sock);
		return sock;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	ret = ioctl(sock, SIOCGIFADDR, &ifr);

    if (ret == 0)
	{
		sprintf(local_ip, "%s", inet_ntoa(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr));
    }

	if (sock > 0)
	{
		close(sock);
	}

    return ret;
}

#ifdef CONFIG_PLATFORM_INGENIC
static int obtain_ip(char* ifname)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

	memset(cmd, 0, sizeof(cmd));
	ret = get_local_ip(ifname, cmd);

	if (ret < 0)
	{
		int i = 0;

		sprintf(cmd, "udhcpc -i %s", ifname);

		printf("CMD: %s\n", cmd);

		ret = system(cmd);

		printf("%s, ret = %d\n", __FUNCTION__, ret);

		while (i < 50)
		{
			memset(cmd, 0, sizeof(cmd));
			ret = get_local_ip(ifname, cmd);

			if (ret < 0)
			{
				sleep(1);
			}
			else
			{
				printf("%s, ip = %s\n", __FUNCTION__, cmd);
				break;
			}
		}
	}

	return ret;
}

#else /* CONFIG_PLATFORM_INGENIC */

static int obtain_ip(char* ifname)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

	memset(cmd, 0, sizeof(cmd));
	ret = get_local_ip(ifname, cmd);

	if (ret < 0)
	{
		int i = 0;

		sprintf(cmd, "udhcpc -i %s", ifname);

		printf("CMD: %s\n", cmd);

		ret = system(cmd);

		printf("%s, ret = %d\n", __FUNCTION__, ret);

		while (i < 50)
		{
			memset(cmd, 0, sizeof(cmd));
			ret = get_local_ip(ifname, cmd);

			if (ret < 0)
			{
				sleep(1);
			}
			else
			{
				printf("%s, ip = %s\n", __FUNCTION__, cmd);
				break;
			}
		}
	}

	return ret;
}
#endif /* CONFIG_PLATFORM_INGENIC */

static int ping_server(char* ip)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

	sprintf(cmd, "ping %s -c 5", ip);

	printf("%s, CMD: %s\n", __FUNCTION__, cmd);

	ret = system(cmd);

	return ret;
}

#ifdef CONFIG_PLATFORM_INGENIC
#define INGENIC_TOOL_PATH "/tmp/mnt/sdcard/T31_CY"	

static int system_sleep(void)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

	sprintf(cmd, INGENIC_TOOL_PATH "/probe --iic_mode --mcu_int_flag");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

	sprintf(cmd, INGENIC_TOOL_PATH "/probe --iic_mode --poweroff_pir");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

	sprintf(cmd, INGENIC_TOOL_PATH "/probe --iic_mode --clear_int_flag");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

	sprintf(cmd, INGENIC_TOOL_PATH "/probe --iic_mode --poweroff_master");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

	sleep(1);

	return ret;
}

#else /* CONFIG_PLATFORM_INGENIC */

static int system_sleep(void)
{
	int ret;
	char cmd[256];

	printf("%s, enter\n", __FUNCTION__);

//	sprintf(cmd, "echo mem > /sys/power/state");

    sprintf(cmd, "linux-serial-test -p /dev/ttyS5 -b 115200 &");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

    sleep(2);

    sprintf(cmd, "killall linux-serial-test");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

    sleep(2);

    sprintf(cmd, "echo -e -n \"\\x7b\\x05\\x71\\x00\\x0f\" > /dev/ttyS5");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

    sleep(2);

    sprintf(cmd, "echo -e -n \"\\x7b\\x04\\x21\\x5e\" > /dev/ttyS5");
	printf("%s, CMD: %s\n", __FUNCTION__, cmd);
	ret = system(cmd);

	return ret;
}
#endif /* CONFIG_PLATFORM_INGENIC */

static int connect_to_tcp_srv(char *ip, int port)
{
    int sock;
	int ret;
	int i;
    struct sockaddr_in addr = { 0 };

	printf("%s, enter\n", __FUNCTION__);

	sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("%s, create sock err = %d\n", __FUNCTION__, sock);
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

	for (i = 0; i < TCP_CONNECT_RETRIES; i++)
	{
		ret = connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	
	    if (ret < 0)
		{
	        printf("%s, connect server err = %d\n", __FUNCTION__, ret);
	    }
		else
		{
			break;
		}
	}

	if (ret < 0)
	{
		close(sock);
		return ret;
	}
	else
	{
		return sock;
	}
}


#ifdef TLS_KEEP_ALIVE
#define TLS1_2_VERSION			    0x0303
#define TLS1_2_VERSION_MAJOR		0x03
#define TLS1_2_VERSION_MINOR		0x03

typedef struct {
    int sock;
    SSL_CTX *ssl_ctx;
    SSL *ssl;
} tls_conn_t;

static SSL_CTX* tls_ctx_create(void)
{
    SSL_CTX *ssl_ctx = NULL;
	int ret;

	printf("%s, enter\n", __FUNCTION__);

    SSL_library_init();

    OpenSSL_add_all_algorithms();

    SSL_load_error_strings();

    ssl_ctx = SSL_CTX_new(TLSv1_2_client_method());

	/* Wi-Fi Firmware Only Support AES256-SHA */
	ret = SSL_CTX_set_cipher_list(ssl_ctx, "AES256-SHA");
	printf("%s, SSL_CTX_set_cipher_list, ret = %d\n", __FUNCTION__, ret);

	printf("%s, Exit\n", __FUNCTION__);

    return ssl_ctx;
}

static int tls_conn_open(
                tls_conn_t *tls_conn,
                char *ip,
                int port)
{
    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;
    int sock = -1;

	printf("%s, enter\n", __FUNCTION__);

    ssl_ctx = tls_ctx_create();
    if (ssl_ctx == NULL)
    {
		printf("%s, create TLS Context failed\n", __FUNCTION__);
        return -1;
    }

    sock = connect_to_tcp_srv(ip, port);
	if (sock < 0)
	{
		printf("%s, connect to TCP server failed\n", __FUNCTION__);
        goto fail;
	}

    ssl = SSL_new(ssl_ctx);
    if (ssl == NULL)
    {
		printf("%s, New TLS failed\n", __FUNCTION__);
        goto fail;
    }

    SSL_set_fd(ssl, sock);            

    /* perform the connection */
    if (SSL_connect(ssl) < 0)
    {
        printf("SSL_connect failed\n");
        goto fail;
    }

    tls_conn->sock = sock;
    tls_conn->ssl_ctx = ssl_ctx;
    tls_conn->ssl = ssl;

	printf("%s, Exit\n", __FUNCTION__);

    return 0;

fail:
    if (sock >= 0)
    {
        close(sock);
    }

    if (ssl != NULL)
    {
        SSL_free(ssl);
    }

    if (ssl_ctx != NULL)
    {
        SSL_CTX_free(ssl_ctx);
    }

	printf("%s, Exit failed\n", __FUNCTION__);

    return -1;
}

static int tls_conn_close(tls_conn_t *tls_conn)
{
	printf("%s, enter\n", __FUNCTION__);

    if (tls_conn == NULL)
    {
        return 0;
    }

    if (tls_conn->sock >= 0)
    {
        close(tls_conn->sock);                  
    }

    if (tls_conn->ssl != NULL)
    {
        SSL_free(tls_conn->ssl);
    }

    if (tls_conn->ssl_ctx != NULL)
    {
        SSL_CTX_free(tls_conn->ssl_ctx);
    }

	printf("%s, Exit\n", __FUNCTION__);

    return 0;
}

static int build_tls_param(
				wl_tls_param_t* param,
				char* ifname,
				tls_conn_t *tls_conn)
{
	int ret;
	struct sockaddr_in localaddr;
	struct sockaddr_in peeraddr;
    struct arpreq req;  
	struct tcp_ext_info tcp_info;
	socklen_t len;
    wl_ether_addr_t mac_addr;
    int sock = tls_conn->sock;
    SSL *ssl = tls_conn->ssl;

	printf("%s, enter\n", __FUNCTION__);

	memset(param, 0, sizeof(wl_tls_param_t));
	memset(&tcp_info, 0, sizeof(struct tcp_ext_info));

    param->version.major = TLS1_2_VERSION_MAJOR;
    param->version.minor = TLS1_2_VERSION_MINOR;
    param->compression_algorithm = WL_TLS_COMPRESSIONMETHOD_NULL;
    param->cipher_algorithm = WL_TLS_CIPHERALGORITHM_AES;
    param->cipher_type = WL_TLS_CIPHER_TYPE_BLOCK;
    param->mac_algorithm = WL_TLS_MAC_ALGORITHM_SHA1;

	param->keepalive_interval = TCPKA_INTERVAL;

    if (ssl->s3->read_key_length > WL_TLS_MAX_KEY_LENGTH)
    {
		printf("%s, read master key length exceeded\n", __FUNCTION__);
        return -1;
    }
    param->read_master_key_len = ssl->s3->read_key_length;
    memcpy(param->read_master_key, ssl->s3->read_key, param->read_master_key_len);

	PRINT_HEX(param->read_master_key, param->read_master_key_len);
	printf("%s, param->read_master_key_len = %d\n", __FUNCTION__, param->read_master_key_len);

	if (ssl->enc_read_ctx->cipher->iv_len > WL_TLS_MAX_IV_LENGTH)
	{
		printf("%s, read iv length exceeded\n", __FUNCTION__);
		return -1;
	}
    param->read_iv_len = ssl->enc_read_ctx->cipher->iv_len;
    memcpy(param->read_iv, ssl->enc_read_ctx->iv, param->read_iv_len);

	PRINT_HEX(param->read_iv, param->read_iv_len);
	printf("%s, param->read_iv_len = %d\n", __FUNCTION__, param->read_iv_len);

    if (ssl->s3->read_mac_secret_size > WL_TLS_MAX_MAC_KEY_LENGTH)
    {
		printf("%s, read mac secret size exceeded\n", __FUNCTION__);
        return -1;
    }
    param->read_mac_key_len = ssl->s3->read_mac_secret_size;
   	memcpy(param->read_mac_key, ssl->s3->read_mac_secret, param->read_mac_key_len);

	PRINT_HEX(param->read_mac_key, param->read_mac_key_len);
	printf("%s, param->read_mac_key_len = %d\n", __FUNCTION__, param->read_mac_key_len);

    memcpy(param->read_sequence, ssl->s3->read_sequence, WL_TLS_MAX_SEQUENCE_LENGTH);
    param->read_sequence_len = WL_TLS_MAX_SEQUENCE_LENGTH;

	PRINT_HEX(param->read_sequence, param->read_sequence_len);
	printf("%s, param->read_sequence_len = %d\n", __FUNCTION__, param->read_sequence_len);

    if (ssl->s3->write_key_length > WL_TLS_MAX_KEY_LENGTH)
    {
		printf("%s, write master key length exceeded\n", __FUNCTION__);
        return -1;
    }
    param->write_master_key_len = ssl->s3->write_key_length;
    memcpy(param->write_master_key, ssl->s3->write_key, param->write_master_key_len);

	PRINT_HEX(param->write_master_key, param->write_master_key_len);
	printf("%s, param->write_master_key_len = %d\n", __FUNCTION__, param->write_master_key_len);

	if (ssl->enc_write_ctx->cipher->iv_len > WL_TLS_MAX_IV_LENGTH)
	{
		printf("%s, write iv length exceeded\n", __FUNCTION__);
		return -1;
	}
    param->write_iv_len = ssl->enc_write_ctx->cipher->iv_len;
    memcpy(param->write_iv, ssl->enc_write_ctx->iv, param->write_iv_len);

	PRINT_HEX(param->write_iv, param->write_iv_len);
	printf("%s, param->write_iv_len = %d\n", __FUNCTION__, param->write_iv_len);

    if (ssl->s3->write_mac_secret_size > WL_TLS_MAX_MAC_KEY_LENGTH)
    {
		printf("%s, write mac secret size exceeded\n", __FUNCTION__);
        return -1;
    }
    param->write_mac_key_len = ssl->s3->write_mac_secret_size;
    memcpy(param->write_mac_key, ssl->s3->write_mac_secret, param->write_mac_key_len);

	PRINT_HEX(param->write_mac_key, param->write_mac_key_len);
	printf("%s, param->write_mac_key_len = %d\n", __FUNCTION__, param->write_mac_key_len);

    memcpy(param->write_sequence, ssl->s3->write_sequence, WL_TLS_MAX_SEQUENCE_LENGTH);
    param->write_sequence_len = WL_TLS_MAX_SEQUENCE_LENGTH;

	PRINT_HEX(param->write_sequence, param->read_sequence_len);
	printf("%s, param->write_sequence_len = %d\n", __FUNCTION__, param->write_sequence_len);

	len = sizeof(struct sockaddr_in);
	ret = getsockname(sock, (struct sockaddr*)&localaddr, &len);
	if (ret < 0)
	{
        printf("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->local_ip, &localaddr.sin_addr, WL_IPV4_ADDR_LEN);
    param->local_port = localaddr.sin_port;

	printf("%s, param->local_ip	: %d.%d.%d.%d\n", __FUNCTION__, 
		param->local_ip[0], 
		param->local_ip[1],
		param->local_ip[2],
		param->local_ip[3]);
	printf("%s, param->local_port	: 0x%04x\n", __FUNCTION__, param->local_port);

	len = sizeof(struct sockaddr_in);
	ret = getpeername(sock, (struct sockaddr*)&peeraddr, &len);
	if (ret < 0)
	{
        printf("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->remote_ip, &peeraddr.sin_addr, WL_IPV4_ADDR_LEN);
    param->remote_port = peeraddr.sin_port;

	printf("%s, param->remote_ip : %d.%d.%d.%d\n", __FUNCTION__, 
		param->remote_ip[0], 
		param->remote_ip[1],
		param->remote_ip[2],
		param->remote_ip[3]);
	printf("%s, param->remote_port : 0x%04x\n", __FUNCTION__, param->remote_port);

    ret = wl_get_mac_addr(&mac_addr);
    if (ret < 0)
    {
        return -1;
    }

    memcpy(&param->local_mac_addr, mac_addr.octet, WL_ETHER_ADDR_LEN);

    memset(&req, 0, sizeof(struct arpreq));  
    memcpy(&req.arp_pa, &peeraddr, sizeof(peeraddr));  
    strcpy(req.arp_dev, ifname);  
    req.arp_pa.sa_family = AF_INET;  
    req.arp_ha.sa_family = AF_UNSPEC;  
	ret = ioctl(sock, SIOCGARP, &req);
	if (ret < 0)
	{
        printf("%s, ioctl, SIOCGARP err = %d\n", __FUNCTION__, ret);
		return ret;
	}
	memcpy(&param->remote_mac_addr, req.arp_ha.sa_data, WL_ETHER_ADDR_LEN);

	printf("%s, param->remote_mac_addr : %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, 
		param->remote_mac_addr[0], 
		param->remote_mac_addr[1], 
		param->remote_mac_addr[2], 
		param->remote_mac_addr[3], 
		param->remote_mac_addr[4], 
		param->remote_mac_addr[5]);

	len = sizeof(tcp_info);
	ret = getsockopt(sock, SOL_TCP, TCP_EXT_INFO, &tcp_info, &len);
	if (ret < 0)
	{
        printf("%s, getsockopt err = %d\n", __FUNCTION__, ret);
		return ret;
	}

    param->app_syncid = tcp_info.ip_id;
    param->tcp_ack_num = ntohl(tcp_info.rcv_nxt);
    param->tcp_seq_num = ntohl(tcp_info.snd_nxt);
    param->tls_mode = WL_TLS_MODE_KEEPALIVE;

	printf("%s, param->app_syncid	: 0x%04x\n", __FUNCTION__, param->app_syncid);
	printf("%s, param->tcp_ack_num	: 0x%04x\n", __FUNCTION__, param->tcp_ack_num);
	printf("%s, param->tcp_seq_num	: 0x%04x\n", __FUNCTION__, param->tcp_seq_num);

	param->payload_len = strlen(tcpka_payload);
    if (param->payload_len > WL_TLS_MAX_PAYLOAD_LEN)
    {
        param->payload_len = WL_TLS_MAX_PAYLOAD_LEN;
    }
	memcpy(param->payload, tcpka_payload, param->payload_len);

	printf("%s, Exit\n", __FUNCTION__);

	return ret;
}

#else /* TLS_KEEP_ALIVE */

static int build_tcpka_param(
				wl_tcpka_param_t* param,
				char* ifname,
				int sock)
{
	int ret;
	struct sockaddr_in localaddr;
	struct sockaddr_in peeraddr;
    struct arpreq req;  
	struct tcp_ext_info tcp_info;
	socklen_t len;

	printf("%s, enter\n", __FUNCTION__);

	memset(param, 0, sizeof(wl_tcpka_param_t));
	memset(&tcp_info, 0, sizeof(struct tcp_ext_info));

	param->interval = TCPKA_INTERVAL;
	param->retry_interval = TCPKA_RETRY_INTERVAL;
	param->retry_count = TCPKA_RETRY_COUNT;

	printf("%s, interval		: %d\n", __FUNCTION__, param->interval);
	printf("%s, retry_interval	: %d\n", __FUNCTION__, param->retry_interval);
	printf("%s, retry_count		: %d\n", __FUNCTION__, param->retry_count);

	len = sizeof(struct sockaddr_in);
	ret = getsockname(sock, (struct sockaddr*)&localaddr, &len);
	if (ret < 0)
	{
        printf("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->src_ip, &localaddr.sin_addr, sizeof(wl_ipv4_addr_t));
    param->srcport = ntohs(localaddr.sin_port);

	printf("%s, src_ip	: %d.%d.%d.%d\n", __FUNCTION__, 
		param->src_ip.addr[0], 
		param->src_ip.addr[1],
		param->src_ip.addr[2],
		param->src_ip.addr[3]);
	printf("%s, srcport	: 0x%04x\n", __FUNCTION__, param->srcport);

	len = sizeof(struct sockaddr_in);
	ret = getpeername(sock, (struct sockaddr*)&peeraddr, &len);
	if (ret < 0)
	{
        printf("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->dst_ip, &peeraddr.sin_addr, sizeof(wl_ipv4_addr_t));
    param->dstport = ntohs(peeraddr.sin_port);

	printf("%s, dst_ip	: %d.%d.%d.%d\n", __FUNCTION__, 
		param->dst_ip.addr[0], 
		param->dst_ip.addr[1],
		param->dst_ip.addr[2],
		param->dst_ip.addr[3]);
	printf("%s, dstport	: 0x%04x\n", __FUNCTION__, param->dstport);

    memset(&req, 0, sizeof(struct arpreq));  
    memcpy(&req.arp_pa, &peeraddr, sizeof(peeraddr));  
    strcpy(req.arp_dev, ifname);  
    req.arp_pa.sa_family = AF_INET;  
    req.arp_ha.sa_family = AF_UNSPEC;  
	ret = ioctl(sock, SIOCGARP, &req);
	if (ret < 0)
	{
        printf("%s, ioctl, SIOCGARP err = %d\n", __FUNCTION__, ret);
		return ret;
	}
	memcpy(&param->dst_mac, req.arp_ha.sa_data, sizeof(wl_ether_addr_t));

	printf("%s, dst_mac	: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, 
		param->dst_mac.octet[0], 
		param->dst_mac.octet[1], 
		param->dst_mac.octet[2], 
		param->dst_mac.octet[3], 
		param->dst_mac.octet[4], 
		param->dst_mac.octet[5]);

	len = sizeof(tcp_info);
	ret = getsockopt(sock, SOL_TCP, TCP_EXT_INFO, &tcp_info, &len);
	if (ret < 0)
	{
        printf("%s, getsockopt err = %d\n", __FUNCTION__, ret);
		return ret;
	}

    param->ipid = tcp_info.ip_id;
    param->seq = tcp_info.snd_nxt;
    param->ack = tcp_info.rcv_nxt;
    param->tcpwin = tcp_info.window;
    param->tsval = tcp_info.tsval;
    param->tsecr = tcp_info.tsecr;

	printf("%s, ipid	: 0x%04x\n", __FUNCTION__, param->ipid);
	printf("%s, seq		: 0x%08x\n", __FUNCTION__, param->seq);
	printf("%s, ack		: 0x%08x\n", __FUNCTION__, param->ack);
	printf("%s, tcpwin	: 0x%08x\n", __FUNCTION__, param->tcpwin);
	printf("%s, tsval	: 0x%08x\n", __FUNCTION__, param->tsval);
	printf("%s, tsecr	: 0x%08x\n", __FUNCTION__, param->tsecr);

	param->payload_len = strlen(tcpka_payload);
	memcpy(param->payload, tcpka_payload, param->payload_len);

	return ret;
}

#endif /* TLS_KEEP_ALIVE */

static bool is_wakeup(void)
{
	int ret;
	uint32_t wakeind = 0;

	printf("%s, enter\n", __FUNCTION__);

	ret = wl_wowl_get_wakeind(&wakeind);
	if (ret < 0)
	{
		printf("%s, wl_wowl_get_wakeind, ret = %d\n", __FUNCTION__, ret);
	}
	else
	{
		if (wakeind & WL_WOWL_NET)
		{
			printf("%s, wakeup by APP\n", __FUNCTION__);
		}

		if (wakeind & WL_WOWL_DIS)
		{
			printf("%s, wakeup by disassocation\n", __FUNCTION__);			
		}

		if (wakeind & WL_WOWL_BCN)
		{
			printf("%s, wakeup by network lost\n", __FUNCTION__);			
		}

		if (wakeind & WL_WOWL_TCPKEEP_TIME)
		{
			printf("%s, wakeup by tcp connection lost\n", __FUNCTION__);			
		}

		if (wakeind & WL_WOWL_NFYSCAN_WAKE)
		{
			printf("%s, wakeup by scan SSID\n", __FUNCTION__);			
		}
	}

	return (wakeind != 0);
}

static int resume_enter(void)
{
	int ret;
	uint32_t wakeind = 0;

	printf("%s, enter\n", __FUNCTION__);

	ret = wl_set_hostsleep(false);
	if (ret < 0)
	{
		printf("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_set_deepsleep(false);
	if (ret < 0)
	{
		printf("%s, wl_set_deepsleep, err = %d\n", __FUNCTION__, ret);
	}

	ret = wl_set_powersave(false);
	if (ret < 0)
	{
		printf("%s, wl_set_powersave, err = %d\n", __FUNCTION__, ret);	
	}

	ret = wl_reset_listen_interval();
    if (ret < 0)
	{
        printf("%s, wl_reset_listen_interval, err = %d\n", __FUNCTION__, ret);
		return ret;
    }

	ret = wl_wowl_get_wakeind(&wakeind);
	if (ret < 0)
	{
        printf("%s, wl_wowl_get_wakeind, err = %d\n", __FUNCTION__, ret);
		return ret;
    }

	printf("%s, wakeind = 0x%08x\n", __FUNCTION__, wakeind);

	if (wakeind & WL_WOWL_NFYSCAN_WAKE)
	{
        printf("%s, wakeup by SSID\n", __FUNCTION__);
		wl_wakeup_by_ssid_stop();
	}
	else
	{
		ret = wl_wowl_clear_wakeind();
		if (ret < 0)
		{
			printf("%s, wl_wowl_clear_wakeind, err = %d\n", __FUNCTION__, ret);	
			return ret;		
		}

        if (wakeind & WL_WOWL_NET)
        {        
            wl_wowl_tcp_rst();
        }

		ret = wl_wowl_disable();
		if (ret < 0)
		{
			printf("%s, wl_wowl_disable, err = %d\n", __FUNCTION__, ret);	
			return ret;		
		}

		ret = wl_tcpka_conn_disable();
		if (ret < 0)
		{
			printf("%s, wl_tcpka_conn_disable, err = %d\n", __FUNCTION__, ret);	
		}
	}

	return ret;
}

static int suspend_enter(char* ifname, void *conn_info, bool is_connected)
{
	int ret;
#ifdef TLS_KEEP_ALIVE
    wl_tls_param_t tls_param;
    tls_conn_t *tls_conn = (tls_conn_t *)conn_info;
#else /* TLS_KEEP_ALIVE */
	wl_tcpka_param_t tcpka_param;
    int sock = (int)conn_info;
#endif /* TLS_KEEP_ALIVE */
    uint32_t wowl_caps;

	printf("%s, enter\n", __FUNCTION__);

	if (is_connected)
	{
#ifdef TLS_KEEP_ALIVE
		ret = build_tls_param(&tls_param, ifname, tls_conn);
	    if (ret < 0)
		{
	        printf("%s, build_tcpka_param, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		wowl_caps = WL_WOWL_NET | WL_WOWL_DIS | WL_WOWL_BCN | WL_WOWL_TCPKEEP_TIME | WL_WOWL_SECURE;

		ret = wl_wowl_secure_enable(
					wowl_caps,
					WOWL_PATTERN_MATCH_OFFSET,
					(const int8_t*)wowl_pattern,
					strlen(wowl_pattern),
					&tls_param);
	    if (ret < 0)
		{
	        printf("%s, wl_wowl_enable, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

#else /* TLS_KEEP_ALIVE */
		ret = build_tcpka_param(&tcpka_param, ifname, sock);
	    if (ret < 0)
		{
	        printf("%s, build_tcpka_param, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		ret = wl_tcpka_conn_enable(&tcpka_param);
	    if (ret < 0)
		{
	        printf("%s, wl_tcpka_conn_enable, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		wowl_caps = WL_WOWL_NET | WL_WOWL_DIS | WL_WOWL_BCN | WL_WOWL_TCPKEEP_TIME;

		ret = wl_wowl_enable(
					wowl_caps,
					WOWL_PATTERN_MATCH_OFFSET,
					(const int8_t*)wowl_pattern,
					strlen(wowl_pattern));
	    if (ret < 0)
		{
	        printf("%s, wl_wowl_enable, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

#endif /* TLS_KEEP_ALIVE */

		ret = wl_set_listen_interval(CUSTOM_LISTEN_INTERVAL);
	    if (ret < 0)
		{
	        printf("%s, wl_set_listen_interval, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		ret = wl_set_deepsleep(true);
		if (ret < 0)
		{
			printf("%s, wl_set_deepsleep, err = %d\n", __FUNCTION__, ret);
		}

		ret = wl_set_powersave(true);
		if (ret < 0)
		{
			printf("%s, wl_set_powersave, err = %d\n", __FUNCTION__, ret);	
			return ret;
		}
	}

	ret = wl_set_hostsleep(true);
	if (ret < 0)
	{
		printf("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);	
	}

	return ret;	
}

static void connect_status_change_handler(bool is_connected, void* userdata)
{
    printf("%s, is_connected = %d\n", __FUNCTION__, is_connected);

    if (is_connected)
    {
        /* Add code to handle connection */    
    }
    else
    {
        /* Add code to handle disconnection */    
    }
}

int main(int argc, char **argv)
{
	int ret;
	char *ifname = DEFAULT_IFNAME; 
	char *ssid = NULL; 
	char *password = NULL; 
	char *ip = NULL;
	int port = 0;
#ifdef TLS_KEEP_ALIVE
    tls_conn_t tls_conn;
#else /* TLS_KEEP_ALIVE */
    int sock = -1;
#endif /* TLS_KEEP_ALIVE */
	bool is_connected = true;

    if (argc < 2)
	{
		usage();
    }

	while (*argv)
	{
		if (!strcmp(*argv, "-h"))
		{
			usage();
		}
		else if (!strcmp(*argv, "-i"))
		{
			argv++;
			ifname = *argv;
		}
		else if (!strcmp(*argv, "-s"))
		{
			argv++;
			ssid = *argv;
		}
		else if (!strcmp(*argv, "-k"))
		{
			argv++;
			password = *argv;
		}
		else if (!strcmp(*argv, "-a"))
		{
			argv++;
			ip = *argv;
		}
		else if (!strcmp(*argv, "-p"))
		{
			argv++;
			port = atoi(*argv);
		}

		argv++;
	}

	if ((ssid == NULL) || (ip == NULL) || (port == 0))
	{
		usage();
		return 0;
	}

#ifdef TLS_KEEP_ALIVE
    memset(&tls_conn, 0, sizeof(tls_conn_t));
#endif /* TLS_KEEP_ALIVE */

	ret = network_up(ifname);

	if (ret < 0)
	{
		printf("network_up, err = %d\n", ret);	
		return 0;
	}

	ret = wl_init(ifname);
	if (ret < 0)
	{
		printf("wl_init failed.\n");	
		return 0;
	}

	ret = resume_enter();
	if (ret)
	{
		printf("resume_enter, err = %d\n", ret);	
		goto exit;
	}

	if (wl_is_associated())
	{
		printf("Already joined AP.\n");
	}
	else
	{
		ret = connect_ap(ssid, password);
	}

	if (ret == WL_E_OK)
	{
        wl_connect_status_register(connect_status_change_handler, NULL);
    
		ret = obtain_ip(ifname);
		if (ret)
		{
			printf("obtain_ip, err = %d\n", ret);	
			goto exit;
		}

		ret = ping_server(ip);
		if (ret)
		{
			printf("ping_server, err = %d\n", ret);	
			goto exit;
		}	

#ifdef TLS_KEEP_ALIVE
        ret = tls_conn_open(&tls_conn, ip, port);
        if (ret < 0)
        {
            goto exit;
        }

        ret = SSL_write(tls_conn.ssl, user_payload, strlen(user_payload));
	    if (ret <= 0)
		{
	        printf("SSL_write err = %d\n", ret);
			goto exit;
	    }
#else /* TLS_KEEP_ALIVE */
		sock = connect_to_tcp_srv(ip, port);
		if (sock < 0)
		{
			goto exit;		
		}

		ret = send(sock, user_payload, strlen(user_payload), 0);
	    if (ret < 0)
		{
	        printf("send err = %d\n", ret);
			goto exit;
	    }
#endif /* TLS_KEEP_ALIVE */

		sleep(2);
	}
	else if (ret == WL_E_NO_NETWORK)
	{
		wl_ssid_t wlssid;

		is_connected = false;

		printf("Can't find network, set wakeup by SSID\n");

		wlssid.len = strlen(ssid);

		if (wlssid.len > WL_MAX_SSID_LEN)
		{
			goto exit;
		}

		strncpy((char*)wlssid.value, ssid, WL_MAX_SSID_LEN);

		ret = wl_wakeup_by_ssid_start(&wlssid, WAKEUP_BY_SSID_SCAN_INTERVAL);

		if (ret < 0)
		{
			printf("wl_wakeup_by_ssid_start, err = %d\n", ret);
			goto exit;
		}
	}

#ifdef TLS_KEEP_ALIVE
	ret = suspend_enter(ifname, (void*)&tls_conn, is_connected);
#else /* TLS_KEEP_ALIVE */
	ret = suspend_enter(ifname, (void*)sock, is_connected);
#endif /* TLS_KEEP_ALIVE */

	system_sleep();

#if (0)
	while (1)
	{
		sleep(10);

		printf("Check whether the host is wake by Wi-Fi chip....n", ret);

		if (is_wakeup())
		{
			break;
		}
	}
#endif

exit:
#ifdef TLS_KEEP_ALIVE
    if (tls_conn.ssl_ctx != NULL)
    {
        tls_conn_close(&tls_conn);
    }
#else /* TLS_KEEP_ALIVE */
	if (sock >= 0)
	{
		close(sock);
	}
#endif /* TLS_KEEP_ALIVE */

    wl_connect_status_deregister(connect_status_change_handler);

	wl_deinit();
	
	return 0;
}

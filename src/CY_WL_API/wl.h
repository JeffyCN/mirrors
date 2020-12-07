#ifndef __WL_H__
#define __WL_H__

#include <stdint.h>
#include <stdbool.h>

#define WL_MAX_SSID_LEN				(32)
#define	WL_ETHER_ADDR_LEN			(6)
#define WL_IPV4_ADDR_LEN       		(4) 
#define WL_TCPKA_PAYLOAD_SIZE  		(100)
#define WL_NUMCHANNELS				(64)

#define WL_SCAN_AP_MAX				(64)
#define WL_SCAN_PER_CH_TIME_DEFAULT	(-1)

#define WL_WOWL_MAGIC       	(1 << 0)
#define WL_WOWL_NET         	(1 << 1)
#define WL_WOWL_DIS         	(1 << 2)
#define WL_WOWL_RETR        	(1 << 3)
#define WL_WOWL_BCN         	(1 << 4)
#define WL_WOWL_TST         	(1 << 5)
#define WL_WOWL_TRAFFIC     	(1 << 12)
#define WL_WOWL_BCAST       	(1 << 15)
#define WL_WOWL_TCPKEEP_TIME 	(1 << 17)  /*Wakeup on tcpkeep alive timeout */

typedef enum {
	WL_E_OK                     = 0,    /* Success */
	WL_E_ERROR                  = -1,   /* Error generic */
	WL_E_BADARG                 = -2,   /* Bad Argument */
    WL_E_BADOPTION	            = -3,   /* Bad option */
    WL_E_NOTUP                  = -4,   /* Not up */
    WL_E_NOTDOWN                = -5,   /* Not down */
    WL_E_NOTAP                  = -6,   /* Not AP */
    WL_E_NOTSTA                 = -7,   /* Not STA  */
    WL_E_BADKEYIDX			    = -8,   /* BAD Key Index */
    WL_E_RADIOOFF 			    = -9,   /* Radio Off */
    WL_E_NOTBANDLOCKED		    = -10,  /* Not  band locked */
    WL_E_NOCLK			        = -11,  /* No Clock */
    WL_E_BADRATESET             = -12,  /* BAD Rate valueset */
    WL_E_BADBAND                = -13,  /* BAD Band */
    WL_E_BUFTOOSHORT            = -14,	/* Buffer too short */
    WL_E_BUFTOOLONG             = -15,  /* Buffer too long */
    WL_E_BUSY                   = -16,	/* Busy */
    WL_E_NOTASSOCIATED          = -17,  /* Not Associated */
    WL_E_BADSSIDLEN             = -18,  /* Bad SSID len */
    WL_E_OUTOFRANGECHAN         = -19,  /* Out of Range Channel */
    WL_E_BADCHAN                = -20,  /* Bad Channel */
    WL_E_BADADDR                = -21,  /* Bad Address */
    WL_E_NORESOURCE             = -22,  /* Not Enough Resources */
    WL_E_UNSUPPORTED            = -23,	/* Unsupported */
    WL_E_BADLEN                 = -24,	/* Bad length */
    WL_E_NOTREADY               = -25,	/* Not Ready */
    WL_E_EPERM                  = -26,	/* Not Permitted */
    WL_E_NOMEM                  = -27,	/* No Memory */
    WL_E_ASSOCIATED             = -28,	/* Associated */
    WL_E_RANGE                  = -29,	/* Not In Range */
    WL_E_NOTFOUND               = -30,	/* Not Found */
    WL_E_WME_NOT_ENABLED        = -31,	/* WME Not Enabled */
    WL_E_TSPEC_NOTFOUND         = -32,	/* TSPEC Not Found */
    WL_E_ACM_NOTSUPPORTED       = -33,	/* ACM Not Supported */
    WL_E_NOT_WME_ASSOCIATION    = -34,  /* Not WME Association */
    WL_E_SDIO_ERROR			    = -35,	/* SDIO Bus Error */
    WL_E_DONGLE_DOWN		    = -36,	/* Dongle Not Accessible */
    WL_E_VERSION			    = -37, 	/* Incorrect version */
    WL_E_TXFAIL                 = -38, 	/* TX failure */
    WL_E_RXFAIL                 = -39,	/* RX failure */
    WL_E_NODEVICE               = -40,  /* Device not present */
    WL_E_NMODE_DISABLED         = -41,  /* NMODE disabled */
    WL_E_NONRESIDENT            = -42,  /* access to nonresident overlay */
    WL_E_SCANREJECT             = -43,  /* reject scan request */
    WL_E_USAGE_ERROR            = -44,  /* WLCMD usage error */
    WL_E_IOCTL_ERROR            = -45,  /* WLCMD ioctl error */
    WL_E_SERIAL_PORT_ERR        = -46,  /* RWL serial port error */
    WL_E_DISABLED               = -47,  /* Disabled in this build */
    WL_E_DECERR                 = -48,  /* Decrypt error */
    WL_E_ENCERR                 = -49,  /* Encrypt error */
    WL_E_MICERR                 = -50,  /* Integrity/MIC error */
    WL_E_REPLAY                 = -51,  /* Replay */
    WL_E_IE_NOTFOUND            = -52,  /* IE not found */
    WL_E_DATA_NOTFOUND          = -53,  /* Complete data not found in buffer */
    WL_E_TIMEOUT				= -110	/* Timeout */
} wl_err_t;

#define WEP_ENABLED            	0x0001
#define TKIP_ENABLED           	0x0002
#define AES_ENABLED            	0x0004
#define SHARED_ENABLED     		0x00008000
#define WPA_SECURITY       		0x00200000
#define WPA2_SECURITY      		0x00400000
#define ENTERPRISE_ENABLED 		0x02000000
#define WPS_ENABLED        		0x10000000
#define IBSS_ENABLED       		0x20000000

typedef enum
{
	/* Open security */
    WL_SECURITY_OPEN           	= 0,
    /* WEP PSK Security with open authentication */
    WL_SECURITY_WEP_PSK        	= WEP_ENABLED,
    /* WEP PSK Security with shared authentication */
    WL_SECURITY_WEP_SHARED     	= (WEP_ENABLED | SHARED_ENABLED),
    /* WPA PSK Security with TKIP */
    WL_SECURITY_WPA_TKIP_PSK   	= (WPA_SECURITY | TKIP_ENABLED),
    /* WPA PSK Security with AES */
    WL_SECURITY_WPA_AES_PSK    	= (WPA_SECURITY | AES_ENABLED),
    /* WPA PSK Security with AES & TKIP */
    WL_SECURITY_WPA_MIXED_PSK  	= (WPA_SECURITY | AES_ENABLED | TKIP_ENABLED),
    /* WPA2 PSK Security with AES */
    WL_SECURITY_WPA2_AES_PSK   	= (WPA2_SECURITY | AES_ENABLED),
    /* WPA2 PSK Security with TKIP */
    WL_SECURITY_WPA2_TKIP_PSK  	= (WPA2_SECURITY | TKIP_ENABLED),
    /* WPA2 PSK Security with AES & TKIP */
    WL_SECURITY_WPA2_MIXED_PSK 	= (WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),

    /* WPA Enterprise Security with TKIP */
    WL_SECURITY_WPA_TKIP_ENT   	= (ENTERPRISE_ENABLED | WPA_SECURITY | TKIP_ENABLED),
    /* WPA Enterprise Security with AES */
    WL_SECURITY_WPA_AES_ENT    	= (ENTERPRISE_ENABLED | WPA_SECURITY | AES_ENABLED ),
    /* WPA Enterprise Security with AES & TKIP */
    WL_SECURITY_WPA_MIXED_ENT  	= (ENTERPRISE_ENABLED | WPA_SECURITY | AES_ENABLED | TKIP_ENABLED),
    /* WPA2 Enterprise Security with TKIP */
    WL_SECURITY_WPA2_TKIP_ENT  	= (ENTERPRISE_ENABLED | WPA2_SECURITY | TKIP_ENABLED),
    /* WPA2 Enterprise Security with AES */
    WL_SECURITY_WPA2_AES_ENT   	= (ENTERPRISE_ENABLED | WPA2_SECURITY | AES_ENABLED),
    /* WPA2 Enterprise Security with AES & TKIP */
    WL_SECURITY_WPA2_MIXED_ENT 	= (ENTERPRISE_ENABLED | WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED),
    /* Open security on IBSS ad-hoc network */
    WL_SECURITY_IBSS_OPEN      	= (IBSS_ENABLED),
    /* WPS with open security */
    WL_SECURITY_WPS_OPEN       	= (WPS_ENABLED),
    /* WPS with AES security */
    WL_SECURITY_WPS_SECURE     	= (WPS_ENABLED | AES_ENABLED),
    WL_SECURITY_UNKNOWN		 	= -1
} wl_security_t;

typedef struct
{
	uint8_t octet[WL_ETHER_ADDR_LEN];
} wl_ether_addr_t;

typedef struct {
    uint8_t addr[WL_IPV4_ADDR_LEN];
} wl_ipv4_addr_t;

typedef struct
{
    uint32_t len;
    uint8_t  value[WL_MAX_SSID_LEN];
} wl_ssid_t;

typedef struct
{
    wl_ssid_t ssid;             /**< Service Set Identification (i.e. Name of Access Point)                    */
    wl_ether_addr_t bssid;   	/**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
    uint32_t security;      	/**< Security type                                                             */
    uint16_t channel;        	/**< Radio channel that the AP beacon was received on                          */
    int16_t rssi;  				/**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
} wl_ap_info_t;

typedef struct {
    /* The intervalof send tcp packet for keepalive (keep tcp session), a decimal value, in seconds. */
    uint16_t interval;
    /* The interval of re-send tcp packet, if not receive ACK from server, a decimal value, in seconds. */
    uint16_t retry_interval;
    /* Retry count if not receive ACK from server */
    uint16_t retry_count;
    /* Destination MAC address */
    wl_ether_addr_t dst_mac;
    /* Source (Local) IP address  */
    wl_ipv4_addr_t src_ip;
    /* Destination (Remote) IP address */
    wl_ipv4_addr_t dst_ip;
    /* The starting IP frame identification number */
    uint16_t ipid;
    /* Source port */
    uint16_t srcport;
    /* Destination port */
    uint16_t dstport;
    /* Sequence number of the TCP frame */
    uint32_t seq;
    /* Acknowledge number of the TCP frame */
    uint32_t ack;
    /* TCP Window */
    uint32_t tcpwin;
	/* TCP Option Timestamp  Value */
    uint32_t tsval;
	/* TCP Option Timestamp Echo Replay */
    uint32_t tsecr;
    /* The length of the TCP Payload */
    uint16_t payload_len;
    /* TCP Payload data */
    uint8_t payload[WL_TCPKA_PAYLOAD_SIZE];
} wl_tcpka_param_t;

extern int wl_init(const char* ifname);
extern int wl_deinit(void);
extern bool wl_is_associated(void);
extern int wl_get_bssid(wl_ether_addr_t *bssid);
extern int wl_get_channel(uint32_t* channel);
extern int wl_get_rssi(int32_t* rssi);
extern int wl_join_ap(
					wl_ssid_t* ssid,
					uint32_t security,
					const int8_t* security_key,
					uint8_t key_length);
extern int wl_scan(
				const wl_ssid_t* ssid,
				const wl_ether_addr_t* bssid,
				const uint16_t* channel_list,
				int32_t channel_num,
				int32_t time_per_channel);
extern int wl_get_scan_results(
					wl_ap_info_t* ap_info,
					uint32_t* num,
					bool* is_completed);
extern int wl_set_powersave(bool enable);
extern int wl_set_hostsleep(bool sleep);
extern int wl_set_deepsleep(bool deepsleep);
extern int wl_wowl_enable(
				uint32_t caps,
				uint32_t match_offset,
				const int8_t* pattern,
				uint32_t pattern_size);
extern int wl_wowl_disable(void);
extern int wl_wowl_get_wakeind(uint32_t *wakeind);
extern int wl_wowl_clear_wakeind(void);
extern int wl_tcpka_conn_enable(wl_tcpka_param_t *tcpka_param);
extern int wl_tcpka_conn_disable(void);
extern int wl_set_listen_interval(uint32_t listen_interval);
extern int wl_reset_listen_interval(void);

#endif /* __WL_H__ */

#ifndef __WL_IOCTL_H_
#define	__WL_IOCTL_H_

#include "wl.h"

/* Small length ioctl buffer required */
#define	WLC_IOCTL_SMLEN			    256

/* Max length ioctl buffer required */
#define	WLC_IOCTL_MAXLEN		    8192	

#define WL_SCAN_PARAMS_SSID_MAX 	10
#define WL_EVENTING_MASK_LEN		16
#define WL_EVENT_TYPE_MAX			255

#define	ETHER_TYPE_BRCM				0x886c		
#define WL_ESCAN_REQ_VERSION 		1
#define WL_ESCAN_SYNC_ID			(0x574C)

#define	SETBIT(array, i)		(((uint8_t*)array)[(i) / 8] |= 1 << ((i) % 8))
#define	CLRBIT(array, i)		(((uint8_t*)array)[(i) / 8] &= ~(1 << ((i) % 8)))
#define	ISSET(array, i)			(((const uint8_t*)array)[(i) / 8] & (1 << ((i) % 8)))

/* common ioctl definitions */
#define WLC_GET_MAGIC				0
#define WLC_GET_VERSION				1
#define WLC_UP						2
#define WLC_DOWN					3
#define WLC_SET_INFRA       		20
#define WLC_GET_AUTH				21
#define WLC_SET_AUTH				22
#define WLC_GET_BSSID				23
#define WLC_SET_BSSID				24
#define WLC_SET_SSID				26
#define WLC_GET_CHANNEL				29
#define WLC_SET_CHANNEL				30
#define WLC_GET_KEY					44
#define WLC_SET_KEY					45
#define WLC_DISASSOC				52
#define WLC_GET_BCNPRD				75
#define WLC_GET_DTIMPRD				77
#define WLC_GET_PM					85
#define WLC_SET_PM					86
#define WLC_GET_RSSI				127
#define WLC_GET_WSEC				133
#define WLC_SET_WSEC				134
#define WLC_GET_BSS_INFO			136
#define WLC_SET_WPA_AUTH			165
#define WLC_GET_VAR					262	/* get value of named variable */
#define WLC_SET_VAR					263	/* set named variable to value */
#define WLC_SET_WSEC_PMK			268

#define WLC_E_SET_SSID		0	/* indicates status of set SSID */
#define WLC_E_JOIN		    1	/* differentiates join IBSS from found (WLC_E_START) IBSS */
#define WLC_E_AUTH			3	/* 802.11 AUTH request */
#define WLC_E_AUTH_IND		4	/* 802.11 AUTH indication */
#define WLC_E_DEAUTH		5	/* 802.11 DEAUTH request */
#define WLC_E_DEAUTH_IND	6	/* 802.11 DEAUTH indication */
#define WLC_E_ASSOC			7	/* 802.11 ASSOC request */
#define WLC_E_ASSOC_IND		8	/* 802.11 ASSOC indication */
#define WLC_E_REASSOC		9	/* 802.11 REASSOC request */
#define WLC_E_REASSOC_IND	10	/* 802.11 REASSOC indication */
#define WLC_E_DISASSOC		11	/* 802.11 DISASSOC request */
#define WLC_E_DISASSOC_IND	12	/* 802.11 DISASSOC indication */
#define WLC_E_LINK			16	/* generic link indication */
#define WLC_E_PSK_SUP		46	/* WPA Handshake fail */
#define WLC_E_ESCAN_RESULT 	69	/* escan result event */
#define WLC_E_NFYSCAN_IND   145
#define WLC_E_LAST			163	/* highest val + 1 for range checking */

#define	OFFSETOF(type, member)		((uint32_t)&(((type *)0)->member))

/* Values for PM Mode */
typedef enum
{
	WL_PM_OFF	= 0,
	WL_PM_MAX	= 1,
	WL_PM_FAST 	= 2
} wl_pm_mode_t;

typedef enum
{
	WL_WEP_ENABLED 	= 0x0001,
	WL_TKIP_ENABLED = 0x0002,
	WL_AES_ENABLED	= 0x0004
} wl_wsec_t;

typedef enum
{
	WL_AUTH_OPEN_SYSTEM		= 0,	/* d11 open authentication */
	WL_AUTH_SHARED_KEY		= 1,	/* d11 shared authentication */
	WL_AUTH_OPEN_SHARED		= 2		/* try open, then shared if open failed w/rc 13 */
} wl_auth_t;

typedef enum
{
    WL_CIPHER_GROUP      			= 0,   /**< Use group cipher suite                                        */
    WL_CIPHER_WEP_40                = 1,   /**< WEP-40                                                        */
    WL_CIPHER_TKIP                  = 2,   /**< TKIP                                                          */
    WL_CIPHER_RESERVED              = 3,   /**< Reserved                                                      */
    WL_CIPHER_CCMP_128              = 4,   /**< CCMP-128 - default pairwise and group cipher suite in an RSNA */
    WL_CIPHER_WEP_104               = 5,   /**< WEP-104 - also known as WEP-128                               */
    WL_CIPHER_BIP_CMAC_128          = 6,   /**< BIP-CMAC-128 - default management frame cipher suite          */
    WL_CIPHER_GROUP_DISALLOWED      = 7,   /**< Group address traffic not allowed                             */
    WL_CIPHER_GCMP_128              = 8,   /**< GCMP-128 - default for 60 GHz STAs                            */
    WL_CIPHER_GCMP_256              = 9,   /**< GCMP-256 - introduced for Suite B                             */
    WL_CIPHER_CCMP_256              = 10,  /**< CCMP-256 - introduced for suite B                             */
    WL_CIPHER_BIP_GMAC_128          = 11,  /**< BIP-GMAC-128 - introduced for suite B                         */
    WL_CIPHER_BIP_GMAC_256          = 12,  /**< BIP-GMAC-256 - introduced for suite B                         */
    WL_CIPHER_BIP_CMAC_256          = 13,  /**< BIP-CMAC-256 - introduced for suite B                         */
} wl_80211_cipher_t;

/**
 * Enumeration of AKM (authentication and key management) suites. Table 8-140 802.11mc D3.0.
 */
typedef enum
{
    WL_AKM_RESERVED                    = 0,
    WL_AKM_8021X                       = 1,    /**< WPA2 enterprise                 */
    WL_AKM_PSK                         = 2,    /**< WPA2 PSK                        */
    WL_AKM_FT_8021X                    = 3,    /**< 802.11r Fast Roaming enterprise */
    WL_AKM_FT_PSK                      = 4,    /**< 802.11r Fast Roaming PSK        */
    WL_AKM_8021X_SHA256                = 5,
    WL_AKM_PSK_SHA256                  = 6,
    WL_AKM_TDLS                        = 7,    /**< Tunneled Direct Link Setup      */
    WL_AKM_SAE_SHA256                  = 8,
    WL_AKM_FT_SAE_SHA256               = 9,
    WL_AKM_AP_PEER_KEY_SHA256          = 10,
    WL_AKM_SUITEB_8021X_HMAC_SHA256    = 11,
    WL_AKM_SUITEB_8021X_HMAC_SHA384    = 12,
    WL_AKM_SUITEB_FT_8021X_HMAC_SHA384 = 13,
} wl_akm_suite_t;

typedef enum
{
	WL_WPA_AUTH_DISABLED 		= 0x0000,	/* Legacy (i.e., non-WPA) */
	WL_WPA_AUTH_NONE			= 0x0001,	/* none (IBSS) */
	WL_WPA_AUTH_UNSPECIFIED		= 0x0002,	/* over 802.1x */
	WL_WPA_AUTH_PSK				= 0x0004,	/* Pre-shared key */
	WL_WPA2_AUTH_UNSPECIFIED	= 0x0040,	/* over 802.1x */
	WL_WPA2_AUTH_PSK			= 0x0080	/* Pre-shared key */
} wl_wpa_auth_t;

typedef enum {
	/* Basic supplicant authentication states */
	WLC_SUP_DISCONNECTED = 0,
	WLC_SUP_CONNECTING,
	WLC_SUP_IDREQUIRED,
	WLC_SUP_AUTHENTICATING,
	WLC_SUP_AUTHENTICATED,
	WLC_SUP_KEYXCHANGE,
	WLC_SUP_KEYED,
	WLC_SUP_TIMEOUT,
	WLC_SUP_LAST_BASIC_STATE,

	/* Extended supplicant authentication states */
	/* Waiting to receive handshake msg M1 */
	WLC_SUP_KEYXCHANGE_WAIT_M1 = WLC_SUP_AUTHENTICATED,
	/* Preparing to send handshake msg M2 */
	WLC_SUP_KEYXCHANGE_PREP_M2 = WLC_SUP_KEYXCHANGE,
	/* Waiting to receive handshake msg M3 */
	WLC_SUP_KEYXCHANGE_WAIT_M3 = WLC_SUP_LAST_BASIC_STATE,
	WLC_SUP_KEYXCHANGE_PREP_M4,	/**< Preparing to send handshake msg M4 */
	WLC_SUP_KEYXCHANGE_WAIT_G1,	/**< Waiting to receive handshake msg G1 */
	WLC_SUP_KEYXCHANGE_PREP_G2	/**< Preparing to send handshake msg G2 */
} wl_sup_auth_status_t;


/* WPA failure reason codes carried in the WLC_E_PSK_SUP event */
#define WLC_E_SUP_OTHER			0	/* Other reason */
#define WLC_E_SUP_DECRYPT_KEY_DATA	1	/* Decryption of key data failed */
#define WLC_E_SUP_BAD_UCAST_WEP128	2	/* Illegal use of ucast WEP128 */
#define WLC_E_SUP_BAD_UCAST_WEP40	3	/* Illegal use of ucast WEP40 */
#define WLC_E_SUP_UNSUP_KEY_LEN		4	/* Unsupported key length */
#define WLC_E_SUP_PW_KEY_CIPHER		5	/* Unicast cipher mismatch in pairwise key */
#define WLC_E_SUP_MSG3_TOO_MANY_IE	6	/* WPA IE contains > 1 RSN IE in key msg 3 */
#define WLC_E_SUP_MSG3_IE_MISMATCH	7	/* WPA IE mismatch in key message 3 */
#define WLC_E_SUP_NO_INSTALL_FLAG	8	/* INSTALL flag unset in 4-way msg */
#define WLC_E_SUP_MSG3_NO_GTK		9	/* encapsulated GTK missing from msg 3 */
#define WLC_E_SUP_GRP_KEY_CIPHER	10	/* Multicast cipher mismatch in group key */
#define WLC_E_SUP_GRP_MSG1_NO_GTK	11	/* encapsulated GTK missing from group msg 1 */
#define WLC_E_SUP_GTK_DECRYPT_FAIL	12	/* GTK decrypt failure */
#define WLC_E_SUP_SEND_FAIL		13	/* message send failure */
#define WLC_E_SUP_DEAUTH		14	/* received FC_DEAUTH */
#define WLC_E_SUP_WPA_PSK_TMO		15	/* WPA PSK 4-way handshake timeout */
#define WLC_E_SUP_WPA_PSK_M1_TMO	16	/* WPA PSK 4-way handshake M1 timeout */
#define WLC_E_SUP_WPA_PSK_M3_TMO	17	/* WPA PSK 4-way handshake M3 timeout */


/* MLME Enumerations */
#define DOT11_BSSTYPE_INFRASTRUCTURE    0	/* d11 infrastructure */
#define DOT11_BSSTYPE_INDEPENDENT		1	/* d11 independent */
#define DOT11_BSSTYPE_ANY			    2	/* d11 any BSS type */
#define DOT11_BSSTYPE_MESH			    3	/* d11 Mesh */

#define DOT11_SCANTYPE_ACTIVE			0	/* d11 scan active */
#define DOT11_SCANTYPE_PASSIVE			1	/* d11 scan passive */

#define MCSSET_LEN						(16)
#define MAX_MCS_NUM						(128)	

typedef uint16_t wl_chanspec_t;

#define CH_MAX_2G_CHANNEL       	14      /* Max channel in 2G band */

#define WL_CHANSPEC_CHAN_MASK		0x00ff
#define WL_CHANSPEC_CHAN_SHIFT		0
#define WL_CHANSPEC_CHAN1_MASK		0x000f
#define WL_CHANSPEC_CHAN1_SHIFT		0
#define WL_CHANSPEC_CHAN2_MASK		0x00f0
#define WL_CHANSPEC_CHAN2_SHIFT		4

#define WL_CHANSPEC_CTL_SB_MASK		0x0700
#define WL_CHANSPEC_CTL_SB_SHIFT	8
#define WL_CHANSPEC_CTL_SB_LLL		0x0000
#define WL_CHANSPEC_CTL_SB_LLU		0x0100
#define WL_CHANSPEC_CTL_SB_LUL		0x0200
#define WL_CHANSPEC_CTL_SB_LUU		0x0300
#define WL_CHANSPEC_CTL_SB_ULL		0x0400
#define WL_CHANSPEC_CTL_SB_ULU		0x0500
#define WL_CHANSPEC_CTL_SB_UUL		0x0600
#define WL_CHANSPEC_CTL_SB_UUU		0x0700
#define WL_CHANSPEC_CTL_SB_LL		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_LU		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_UL		WL_CHANSPEC_CTL_SB_LUL
#define WL_CHANSPEC_CTL_SB_UU		WL_CHANSPEC_CTL_SB_LUU
#define WL_CHANSPEC_CTL_SB_L		WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_U		WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_LOWER	WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_UPPER	WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_NONE		WL_CHANSPEC_CTL_SB_LLL

#define WL_CHANSPEC_BW_MASK			0x3800
#define WL_CHANSPEC_BW_SHIFT		11
#define WL_CHANSPEC_BW_5			0x0000
#define WL_CHANSPEC_BW_10			0x0800
#define WL_CHANSPEC_BW_20			0x1000
#define WL_CHANSPEC_BW_40			0x1800
#define WL_CHANSPEC_BW_80			0x2000
#define WL_CHANSPEC_BW_160			0x2800
#define WL_CHANSPEC_BW_8080			0x3000
#define WL_CHANSPEC_BW_2P5			0x3800

#define WL_CHANSPEC_BAND_MASK		0xc000
#define WL_CHANSPEC_BAND_SHIFT		14
#define WL_CHANSPEC_BAND_2G			0x0000
#define WL_CHANSPEC_BAND_3G			0x4000
#define WL_CHANSPEC_BAND_4G			0x8000
#define WL_CHANSPEC_BAND_5G			0xc000
#define INVCHANSPEC					255
#define MAX_CHANSPEC				0xFFFF

#define CHSPEC_CHANNEL(chspec) 		((uint8_t)((chspec) & WL_CHANSPEC_CHAN_MASK))
#define CH20MHZ_CHSPEC(channel)    	(wl_chanspec_t)((wl_chanspec_t)(channel) | WL_CHANSPEC_BW_20 | \
                                            		 WL_CHANSPEC_CTL_SB_NONE | (((channel) <= CH_MAX_2G_CHANNEL) ? \
                                            		 WL_CHANSPEC_BAND_2G : WL_CHANSPEC_BAND_5G))

/* Linux network driver ioctl encoding */
typedef struct
{
	uint32_t cmd;		/**< common ioctl definition */
	void *buf;			/**< pointer to user buffer */
	uint32_t len;		/**< length of user buffer */
	uint8_t set;		/**< 1=set IOCTL; 0=query IOCTL */
	uint32_t used;		/**< bytes read or written (optional) */
	uint32_t needed;	/**< bytes needed (optional) */
} wl_ioctl_t;

/* Used to get specific STA parameters */
typedef struct
{
	uint32_t	val;
	wl_ether_addr_t ea;
} wl_scb_val_t;

/* channel encoding */
typedef struct wl_channel_info
{
	int32_t hw_channel;
	int32_t target_channel;
	int32_t scan_channel;
} wl_channel_info_t;

typedef struct
{
    uint32_t pci_wakeind;
    uint32_t ucode_wakeind;
} wl_wowl_wakeind_t;

typedef struct
{
	wl_ssid_t ssid;		    	/**< default: {0, ""} */
	wl_ether_addr_t bssid;	    /**< default: bcast */
	int8_t bss_type;			/**< default: any,
					             * DOT11_BSSTYPE_ANY/INFRASTRUCTURE/INDEPENDENT
					        	 */
	uint8_t scan_type;		    /**< flags, 0 use default */
	int32_t nprobes;			/**< -1 use default, number of probes per channel */
	int32_t active_time;		/**< -1 use default, dwell time per channel for
					         	 * active scanning
					         	 */
	int32_t passive_time;		/**< -1 use default, dwell time per channel
					 			 * for passive scanning
					 			 */
	int32_t home_time;			/**< -1 use default, dwell time for the home channel
					 			 * between channel scans
					 			 */
	int32_t channel_num;		/**< count of channels and ssids that follow
								 *
								 * low half is count of channels in channel_list, 0
								 * means default (use all available channels)
								 *
								 * high half is entries in wlc_ssid_t array that
								 * follows channel_list, aligned for int32 (4 bytes)
								 * meaning an odd channel count implies a 2-byte pad
								 * between end of channel_list and first ssid
								 *
								 * if ssid count is zero, single ssid in the fixed
								 * parameter portion is assumed, otherwise ssid in
								 * the fixed portion is ignored
								 */
	uint16_t channel_list[WL_NUMCHANNELS];		/**< list of chanspecs */
} wl_scan_params_t;

#define ISCAN_REQ_VERSION           1

#define WL_SCAN_ACTION_START        1
#define WL_SCAN_ACTION_CONTINUE     2
#define WL_SCAN_ACTION_ABORT        3

/* incremental scan struct */
typedef struct wl_iscan_params
{
	uint32_t version;
	uint16_t action;
	uint16_t scan_duration;
	wl_scan_params_t params;
} wl_iscan_params_t;

typedef struct wl_bss_info
{
	uint32_t version;			/**< version field */
	uint32_t length;				/**< byte length of data in this record,
						 		 	 * starting at version and including IEs
						 		 	 */
	wl_ether_addr_t BSSID;
	uint16_t beacon_period;			/**< units are Kusec */
	uint16_t capability;			/**< Capability information */
	uint8_t SSID_len;
	uint8_t SSID[32];
	struct
	{
		uint32_t count;				/**< # rates in this set */
		uint8_t	rates[16];			/**< rates in 500kbps units w/hi bit set if basic */
	} rateset;						/**< supported rates */
	wl_chanspec_t chanspec;			/**< chanspec for bss */
	uint16_t atim_window;			/**< units are Kusec */
	uint8_t dtim_period;			/**< DTIM period */
	int16_t RSSI;					/**< receive signal strength (in dBm) */
	int8_t phy_noise;				/**< noise (in dBm) */

	uint8_t n_cap;					/**< BSS is 802.11N Capable */
	uint32_t nbss_cap;				/**< 802.11N+AC BSS Capabilities */
	uint8_t ctl_ch;					/**< 802.11N BSS control channel number */
	uint8_t padding1[3];			/**< explicit struct alignment padding */
	uint16_t vht_rxmcsmap;			/**< VHT rx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint16_t vht_txmcsmap;			/**< VHT tx mcs map (802.11ac IE, VHT_CAP_MCS_MAP_*) */
	uint8_t flags;					/**< flags */
	uint8_t vht_cap;				/**< BSS is vht capable */
	uint8_t reserved[2];			/**< Reserved for expansion of BSS properties */
	uint8_t basic_mcs[MCSSET_LEN];	/**< 802.11N BSS required MCS set */

	uint16_t ie_offset;				/**< offset at which IEs start, from beginning */
	uint32_t ie_length;				/**< byte length of Information Elements */
	int16_t SNR;					/**< average SNR of during frame reception */

	/* Add new fields here */
	/* variable length Information Elements */
} wl_bss_info_t;

#define	LEGACY_WL_BSS_INFO_VERSION	107		/**< older version of wl_bss_info struct */
#define	LEGACY2_WL_BSS_INFO_VERSION	108		/**< old version of wl_bss_info struct */
#define	WL_BSS_INFO_VERSION			109		/**< current version of wl_bss_info struct */

typedef struct wl_scan_results {
	uint32_t buflen;
	uint32_t version;
	uint32_t count;
	wl_bss_info_t bss_info[1];
} wl_scan_results_t;

#define WLC_E_STATUS_SUCCESS		0	
#define WLC_E_STATUS_FAIL			1	
#define WLC_E_STATUS_TIMEOUT		2	
#define WLC_E_STATUS_NO_NETWORKS	3	
#define WLC_E_STATUS_ABORT			4	
#define WLC_E_STATUS_NO_ACK			5	
#define WLC_E_STATUS_UNSOLICITED	6	
#define WLC_E_STATUS_ATTEMPT		7	
#define WLC_E_STATUS_PARTIAL		8	

#define WLC_EVENT_MSG_LINK			0x01	/* link is up */

/* wl_iscan_results status values */
#define WL_SCAN_RESULTS_SUCCESS		0
#define WL_SCAN_RESULTS_PARTIAL		1
#define WL_SCAN_RESULTS_PENDING		2
#define WL_SCAN_RESULTS_ABORTED		3
#define WL_SCAN_RESULTS_NO_MEM  	4

#define WL_SCAN_RESULTS_FIXED_SIZE 	(sizeof(wl_scan_results_t) - sizeof(wl_bss_info_t))

/* size of wl_iscan_results not including variable length array */
#define WL_ISCAN_RESULTS_FIXED_SIZE \
			(WL_SCAN_RESULTS_FIXED_SIZE + OFFSETOF(wl_iscan_results_t, results))

#define WL_SCAN_PARAMS_FIXED_SIZE 	(OFFSETOF(wl_scan_params_t, channel_list))
#define WL_ISCAN_PARAMS_FIXED_SIZE \
			(OFFSETOF(wl_iscan_params_t, params) + OFFSETOF(wl_scan_params_t, channel_list))


#define WL_ESCAN_PARAMS_FIXED_SIZE \
			(OFFSETOF(wl_escan_params_t, params) + OFFSETOF(wl_scan_params_t, channel_list))

/* incremental scan results struct */
typedef struct
{
	uint32_t status;
	wl_scan_results_t results;
} wl_iscan_results_t;

typedef struct {
	uint32_t version;
	uint16_t action;
	uint16_t sync_id;
	wl_scan_params_t params;
} wl_escan_params_t;

typedef struct wl_escan_result {
	uint32_t buflen;
	uint32_t version;
	uint16_t sync_id;
	uint16_t bss_count;
	wl_bss_info_t bss_info[1];
} wl_escan_result_t;

typedef struct {
	uint8_t ether_dhost[WL_ETHER_ADDR_LEN];
	uint8_t	ether_shost[WL_ETHER_ADDR_LEN];
	uint16_t ether_type;
} wl_ether_header_t;

typedef struct bcmeth_hdr
{
	uint16_t subtype;	
	uint16_t length;
	uint8_t	version;	
	uint8_t	oui[3];		
	uint16_t usr_subtype;
} wl_bcmeth_hdr_t;

#define WL_IFNAME_MAX		(16)	

typedef struct
{
	uint16_t version;
	uint16_t flags;
	uint32_t event_type;
	uint32_t status;
	uint32_t reason;
	uint32_t auth_type;
	uint32_t datalen;
	wl_ether_addr_t	addr;
	char ifname[WL_IFNAME_MAX];
	uint8_t	ifidx;
	uint8_t bsscfgidx;
} wl_event_msg_t;

typedef struct bcm_event {
	wl_ether_header_t eth;
	wl_bcmeth_hdr_t	bcm_hdr;
	wl_event_msg_t event;
} bcm_event_t;

typedef enum
{
	wowl_pattern_type_bitmap = 0,
	wowl_pattern_type_arp,
	wowl_pattern_type_na
} wl_wowl_pattern_type_t;

typedef struct
{
    uint32_t masksize;
    uint32_t offset;
    uint32_t patternoffset;
    uint32_t patternsize;
	uint32_t id;							/* id */
	uint32_t reasonsize;					/* Size of the wakeup reason code */
	wl_wowl_pattern_type_t type;		/* Type of pattern */
} wl_wowl_pattern_t;

#define WSEC_MIN_PSK_LEN    	(8)
#define WSEC_MAX_PSK_LEN    	(64)
#define WSEC_PASSPHRASE        	(1 << 0)

typedef struct {
	uint16_t key_len;        		/* octets in key material */
	uint16_t flags;          		/* key handling qualification */
	uint8_t key[WSEC_MAX_PSK_LEN];  /* PMK material */
} wl_wsec_pmk_t;

/**
 * Structure for storing a WEP key
 */
typedef struct
{
    uint8_t index;    /**< WEP key index [0/1/2/3]                                             */
    uint8_t length;   /**< WEP key length. Either 5 bytes (40-bits) or 13-bytes (104-bits) */
    uint8_t data[32]; /**< WEP key as values NOT chars                                     */
} wl_wep_key_t;

#define CRYPTO_ALGO_OFF 			0
#define CRYPTO_ALGO_WEP1        	1
#define CRYPTO_ALGO_TKIP        	2
#define CRYPTO_ALGO_WEP128        	3
#define CRYPTO_ALGO_AES_CCM        	4
#define CRYPTO_ALGO_AES_OCB_MSDU    5
#define CRYPTO_ALGO_AES_OCB_MPDU    6
#define CRYPTO_ALGO_NALG        	7
#define WSEC_GEN_MIC_ERROR    		0x0001
#define WSEC_GEN_REPLAY        		0x0002
#define WSEC_GEN_ICV_ERROR    		0x0004
#define WL_SOFT_KEY    				(1 << 0)
#define WL_PRIMARY_KEY    			(1 << 1)
#define WL_KF_RES_4    				(1 << 4)
#define WL_KF_RES_5    				(1 << 5)
#define WL_IBSS_PEER_GROUP_KEY    	(1 << 6)
#define DOT11_MAX_KEY_SIZE    		32

typedef struct
{
    uint32_t index;
    uint32_t len;
    uint8_t data[DOT11_MAX_KEY_SIZE];
    uint32_t pad_1[18];
    uint32_t algo;
    uint32_t flags;
    uint32_t pad_2[2];
    int32_t pad_3;
    int32_t iv_initialized;
    int32_t pad_4;
    struct
    {
        uint32_t hi;
        uint16_t lo;
    } rxiv;
    uint32_t pad_5[2];
    wl_ether_addr_t ea;
} wl_wsec_key_t;

typedef struct {
    uint32_t sess_id;
    wl_ether_addr_t dst_mac;
    wl_ipv4_addr_t src_ip;
    wl_ipv4_addr_t dst_ip;
    uint16_t ipid;
    uint16_t srcport;
    uint16_t dstport;
    uint32_t seq;
    uint32_t ack;
    uint16_t tcpwin;
    uint32_t tsval;
    uint32_t tsecr;
    uint32_t len;
    uint16_t data_len;
    uint8_t data[WL_TCPKA_PAYLOAD_SIZE];
} wl_tcpka_conn_add_multi_t;

typedef struct  {
    uint32_t sess_id;
    uint32_t flag;
    uint16_t interval;
    uint16_t retry_interval;
    uint16_t retry_count;
} wl_tcpka_conn_enable_t;

typedef struct
{
    wl_ether_addr_t bssid;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
    uint16_t bssid_cnt;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
    uint32_t chanspec_num;
    wl_chanspec_t  chanspec_list[1];
} wl_assoc_params_t;

typedef struct
{
    wl_ssid_t ssid;
    wl_assoc_params_t params;
} wl_join_params_t;

/* scan to notify host params */
typedef struct wl_nfyscan_params {
	wl_ssid_t ssid;
    uint16_t interval;
	uint16_t action;
} wl_nfyscan_params_t;

#endif /* __WL_IOCTL_H_ */

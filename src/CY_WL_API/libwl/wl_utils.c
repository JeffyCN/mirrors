#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include "wl_internal.h"
#include "wl_debug.h"
#include "crypto/sha1.h"

#define RSN_IE_MINIMUM_LENGTH 				(8)
#define WPA_IE_MINIMUM_LENGTH 				(12)

#define DOT11_IE_ID_RSN						(48)
#define DOT11_IE_ID_VENDOR_SPECIFIC     	(221)

#define VENDOR_SPECIFIC_IE_MINIMUM_LENGTH 	(4)
/* WPA OUI */
#define WPA_OUI_TYPE1                     	"\x00\x50\xF2\x01"
/* Privacy subfield - indicates data confidentiality is required for all data frames exchanged */
#define DOT11_CAP_PRIVACY                 	(0x0010)

#define DOT11_IE_FIX_LEN					(2)

typedef struct
{
    uint8_t id;
    uint8_t len;
	uint8_t data[1];
} dot11_ie_t;

typedef struct
{
    uint8_t id;
    uint8_t len;
    uint8_t oui[4];
} vendor_ie_header_t;

/* Robust Secure Network */
typedef struct
{
    uint8_t id;
    uint8_t len;
    uint16_t version;
    uint8_t group_key_suite[4];          /* See wiced_80211_cipher_t for values */
    uint16_t pairwise_suite_count;
    uint8_t pairwise_suite_list[1][4];
} rsn_ie_fixed_portion_t;

/* WPA IE */
typedef struct
{
    vendor_ie_header_t vendor_header;
    uint16_t version;
    uint8_t multicast_suite[4];
    uint16_t unicast_suite_count;
    uint8_t unicast_suite_list[1][4];
} wpa_ie_fixed_portion_t;

typedef struct
{
    uint16_t akm_suite_count;
    uint8_t akm_suite_list[1][4];
} akm_suite_portion_t;

static dot11_ie_t* wl_find_ie(uint8_t* buf, uint32_t buflen, uint8_t id)
{
    while (buflen != 0)
    {
        uint8_t cur_id = buf[0];
        uint16_t cur_len = buf[1] + DOT11_IE_FIX_LEN;

        /* Check if we've overrun the buffer */
        if (cur_len > buflen)
        {
            return NULL;
        }

        /* Check if we've found the type we are looking for */
        if (cur_id == id)
        {
            return (dot11_ie_t*)buf;
        }

        /* Skip current TLV */
        buf += cur_len;
        buflen -= cur_len;
    }

    return NULL;
}

static wpa_ie_fixed_portion_t* wl_find_wpa_ie(uint8_t* buf, uint32_t buflen)
{
    vendor_ie_header_t* vender_ie = NULL;
    uint8_t* cur_buf = buf;
    uint32_t cur_buflen = buflen;

    while (1)
    {	
		vender_ie = (vendor_ie_header_t*)wl_find_ie(cur_buf, cur_buflen, DOT11_IE_ID_VENDOR_SPECIFIC);

		if (vender_ie == NULL)
		{
			return NULL;
		}

	    /* If the contents match the WPA_OUI and type=1 */
	    if ((vender_ie->len >= (uint8_t)VENDOR_SPECIFIC_IE_MINIMUM_LENGTH) &&
	        (memcmp(vender_ie->oui, WPA_OUI_TYPE1, sizeof(vender_ie->oui)) == 0))
	    {
	    	return (wpa_ie_fixed_portion_t*)vender_ie;
	    }

		cur_buf = (uint8_t*)(((uint8_t*)vender_ie) + DOT11_IE_FIX_LEN + vender_ie->len);
		cur_buflen = buflen - (cur_buf - buf);
    }

	return NULL;
}

static int wl_parse_security(uint32_t* security, wl_bss_info_t *bss_info)
{
    uint8_t* ie_buf;
    uint32_t ie_buflen;
    rsn_ie_fixed_portion_t* rsnie;
    wpa_ie_fixed_portion_t* wpaie = NULL;

    ie_buf = (uint8_t*)(((uint8_t*)bss_info ) + bss_info->ie_offset);
    ie_buflen = bss_info->ie_length;

    /* Find an RSN IE (Robust-Security-Network Information-Element) */
    rsnie = (rsn_ie_fixed_portion_t*)wl_find_ie(ie_buf, ie_buflen, DOT11_IE_ID_RSN);

    /* Find a WPA IE */
    if (rsnie == NULL)
    {
		wpaie = wl_find_wpa_ie(ie_buf, ie_buflen);
    }

    /* Check if AP is configured for WPA2 */
    if ((rsnie != NULL ) &&
        (rsnie->len >= (RSN_IE_MINIMUM_LENGTH + rsnie->pairwise_suite_count * sizeof(uint32_t))))
    {
        uint16_t i;

        *security = WPA2_SECURITY;

        /* Check the RSN contents to see if there are any references to TKIP cipher (2) in the group key or pairwise keys. If so it must be mixed mode. */
        if (rsnie->group_key_suite[3] == WL_CIPHER_TKIP)
        {
            *security |= TKIP_ENABLED;
        }
		else if (rsnie->group_key_suite[3] == WL_CIPHER_CCMP_128)
        {
            *security |= AES_ENABLED;
        }

        for (i = 0; i < rsnie->pairwise_suite_count; i++)
        {
            if (rsnie->pairwise_suite_list[i][3] == WL_CIPHER_TKIP)
            {
                *security |= TKIP_ENABLED;
            }
			else if (rsnie->pairwise_suite_list[i][3] == WL_CIPHER_CCMP_128)
            {
                *security |= AES_ENABLED;
            }
        }

        if (*security == WPA2_SECURITY)
        {
            *security = WL_SECURITY_UNKNOWN;
        }
        else
        {
            akm_suite_portion_t* akm_suites;

            akm_suites = (akm_suite_portion_t*) ((uint8_t*)rsnie->pairwise_suite_list + rsnie->pairwise_suite_count * 4);

            for (i = 0; i < akm_suites->akm_suite_count; i++)
            {
                if (akm_suites->akm_suite_list[i][3] == WL_AKM_8021X)
                {
                    *security |= ENTERPRISE_ENABLED;
                }
            }
        }
    }
    /* Check if AP is configured for WPA */
    else if ((wpaie != NULL) &&
             (wpaie->vendor_header.len >= (WPA_IE_MINIMUM_LENGTH + wpaie->unicast_suite_count * sizeof(uint32_t))))
    {
        uint16_t i;
        akm_suite_portion_t* akm_suites;

        *security =  WPA_SECURITY;

        if (wpaie->multicast_suite[3] == WL_CIPHER_TKIP)
        {
            *security |= TKIP_ENABLED;
        }
        if (wpaie->multicast_suite[3] == WL_CIPHER_CCMP_128)
        {
            *security |= AES_ENABLED;
        }

		akm_suites = (akm_suite_portion_t*) ((uint8_t*)wpaie->unicast_suite_list + wpaie->unicast_suite_count * 4);
        for (i = 0; i < akm_suites->akm_suite_count; i++ )
        {
            if (akm_suites->akm_suite_list[i][3] == WL_AKM_8021X)
            {
                *security |= ENTERPRISE_ENABLED;
            }
        }

        for (i = 0; i < wpaie->unicast_suite_count; i++)
        {
            if (wpaie->unicast_suite_list[i][3] == WL_CIPHER_CCMP_128)
            {
                *security |= AES_ENABLED;
            }
        }
    }
    /* Check if AP is configured for WEP, that is, if the capabilities field indicates privacy, then security supports WEP */
    else if ((bss_info->capability & DOT11_CAP_PRIVACY) != 0)
    {
        *security = WL_SECURITY_WEP_PSK;
    }
    else
    {
        /* Otherwise no security */
        *security = WL_SECURITY_OPEN;
    }

	return WL_E_OK;
}

static inline char wl_itoa(uint8_t i)
{
    if (i < 10) {
        return (char)('0' + i);
    } else {
        return (char)('A' + i - 10);
    }
}

int wl_convert_bss_info(wl_ap_info_t* ap_info, wl_bss_info_t *bss_info)
{
    /*
     * check the SSID length
     */
    if (bss_info->SSID_len > sizeof(bss_info->SSID))
    {
		return WL_E_ERROR;
    }

	memcpy(ap_info->ssid.value, bss_info->SSID, WL_MAX_SSID_LEN);
	ap_info->ssid.len = bss_info->SSID_len;

	memcpy(&ap_info->bssid, &bss_info->BSSID, sizeof(wl_ether_addr_t));

	ap_info->rssi = bss_info->RSSI;

	WL_DBG("chanspec = 0x%04x, ctl_ch = %d\n", bss_info->chanspec, bss_info->ctl_ch);

    if (bss_info->n_cap)
    {
        ap_info->channel = bss_info->ctl_ch;
	}
	else
	{
		ap_info->channel = bss_info->chanspec & WL_CHANSPEC_CHAN_MASK;
	}

	return wl_parse_security(&ap_info->security, bss_info);
}

#ifdef WL_CONFIG_HOST_CALC_PSK
int wl_calc_psk(
		uint8_t psk[32],
		const int8_t* passphrase,
		const wl_ssid_t* ssid)
{
	return pbkdf2_sha1((char*)passphrase, (uint8_t*)ssid->value, ssid->len, 4096, psk, 32);
}
#endif /* WL_CONFIG_HOST_CALC_PSK */

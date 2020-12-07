#ifndef __WL_COMMOM_H__
#define __WL_COMMOM_H__

#include <semaphore.h>
#include "wl.h"
#include "wl_ioctl.h"

/* Compile Options */
#define WIFI_CHIP_CYW43438	(1)
#define WIFI_CHIP_CYW43012	(2)

#ifndef WL_CONFIG_WIFI_CHIP
#define WL_CONFIG_WIFI_CHIP		WIFI_CHIP_CYW43438
#endif
#define WL_FIX_CONNECT_STATUS_OPTIONS

#ifndef WL_CONFIG_FAKEARP
#if (WL_CONFIG_WIFI_CHIP == WIFI_CHIP_CYW43438)
#define WL_CONFIG_FAKEARP
#endif
#endif

#ifndef WL_CONFIG_DEEPSLEEP
#if (WL_CONFIG_WIFI_CHIP == WIFI_CHIP_CYW43438)
#define WL_CONFIG_DEEPSLEEP
#endif
#endif

#ifndef WL_CONFIG_PM_MODE
#define WL_CONFIG_PM_MODE		(WL_PM_FAST)
#endif

#ifndef WL_CONFIG_PM2_SLEEP_RET
#define WL_CONFIG_PM2_SLEEP_RET	(30)
#endif

#undef WL_CONFIG_CHECK_EVENT

/* Reverse the bytes in a 16-bit value */
#define BCMSWAP16(val) \
    ((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) | \
          (((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))

/* Reverse the bytes in a 32-bit value */
#define BCMSWAP32(val) \
    ((uint32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) | \
          (((uint32_t)(val) & (uint32_t)0x0000ff00U) <<  8) | \
          (((uint32_t)(val) & (uint32_t)0x00ff0000U) >>  8) | \
          (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

#define HTON16(i) BCMSWAP16(i)
#define HTON32(i) BCMSWAP32(i)
#define NTOH16(i) BCMSWAP16(i)
#define NTOH32(i) BCMSWAP32(i)

typedef void (*wl_event_cb_t)(wl_event_msg_t* msg, void* user_data);

extern int wl_ioctl(uint32_t cmd, void* buf, uint32_t len, bool set);
extern int wl_iovar_getbuf(
				const char* iovar_name,
				void* param,
				int32_t paramlen,
				int8_t* iovar_buf,
				int32_t buflen);
extern int wl_iovar_setbuf(
				const char* iovar_name,
				void* param,
				int32_t paramlen,
				int8_t* iovar_buf,
				int32_t buflen);
extern int wl_iovar_get(const char* iovar_name, void* outbuf, int32_t len);
extern int wl_iovar_set(const char* iovar_name, void* param, int32_t paramlen);
extern int wl_iovar_getint(const char* iovar_name, int32_t* pval);
extern int wl_iovar_setint(const char* iovar_name, int val);
extern int wl_iovar_getbuf_bsscfg(
			    const char* iovar_name,
			    void* param,
			    int32_t paramlen,
			    void* iovar_buf,
			    int32_t buflen,
			    int32_t bsscfg_idx);
extern int wl_iovar_setbuf_bsscfg(
		        const char*iovar_name,
			    void* param,
			    int32_t paramlen,
			    void* iovar_buf,
			    int32_t buflen,
			    int32_t bsscfg_idx);
extern int wl_iovar_setint_bsscfg(
			    const char* iovar_name,
			    int32_t val,
			    int32_t bssidx);
extern int wl_iovar_getint_bsscfg(
				const char*iovar_name,
				int32_t* pval,
				int32_t bssidx);
extern int wl_dhd_iovar_setint(const char* iovar_name, int32_t val);
extern int wl_error(void);
extern int wl_join_init(uint32_t security);
extern int wl_join_error(void);
extern int wl_join_wait(uint32_t timeout);
extern int wl_join_wake(void);
extern int wl_wait(sem_t* sem, uint32_t timeout);
extern int wl_wake(sem_t* sem);
extern int wl_event_register(
				const uint32_t *event_type,
				uint32_t event_num,
				wl_event_cb_t callback,
				void* user_data);
extern int wl_event_deregister(wl_event_cb_t callback, void* user_data);
extern int wl_convert_bss_info(wl_ap_info_t* ap_info, wl_bss_info_t *bss_info);

#endif /* __WL_COMMOM_H__ */

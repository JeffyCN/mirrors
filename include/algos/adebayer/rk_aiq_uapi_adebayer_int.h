#ifndef _RK_AIQ_UAPI_ADEBAYER_INT_H_
#define _RK_AIQ_UAPI_ADEBAYER_INT_H_

#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_comm.h"

typedef enum rk_aiq_debayer_op_mode_s {
    RK_AIQ_DEBAYER_MODE_INVALID                     = 0,        /**< initialization value */
    RK_AIQ_DEBAYER_MODE_MANUAL                      = 1,        /**< run manual lens shading correction */
    RK_AIQ_DEBAYER_MODE_AUTO                        = 2,        /**< run auto lens shading correction */
    RK_AIQ_DEBAYER_MODE_MAX
} rk_aiq_debayer_op_mode_t;


typedef struct adebayer_attrib_auto_s {
    uint8_t     sharp_strength[9];
    uint8_t     low_freq_thresh;
    uint8_t     high_freq_thresh;
} adebayer_attrib_auto_t;

typedef struct adebayer_attrib_manual_s {
    int8_t      filter1[5];
    int8_t      filter2[5];
    uint8_t     gain_offset;
    uint8_t     sharp_strength;
    uint8_t     hf_offset;
    uint8_t     offset;
    uint8_t     clip_en;
    uint8_t     filter_g_en;
    uint8_t     filter_c_en;
    uint8_t     thed0;
    uint8_t     thed1;
    uint8_t     dist_scale;
    uint8_t     cnr_strength;
    uint8_t     shift_num;
} adebayer_attrib_manual_t;

typedef struct adebayer_attrib_s {
    rk_aiq_uapi_sync_t          sync;

    uint8_t                     enable;
    rk_aiq_debayer_op_mode_t    mode;
    adebayer_attrib_manual_t    stManual;
    adebayer_attrib_auto_t      stAuto;
} adebayer_attrib_t;

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAdebayerHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.

XCamReturn
rk_aiq_uapi_adebayer_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_attrib_t attr,
    bool need_sync
);

XCamReturn
rk_aiq_uapi_adebayer_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    adebayer_attrib_t* attr
);

#endif//_RK_AIQ_UAPI_ADEBAYER_INT_H_

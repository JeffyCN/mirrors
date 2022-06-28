#ifndef __RKAIQ_TYPES_ALGO_ADEBAYER_PRVT_H__
#define __RKAIQ_TYPES_ALGO_ADEBAYER_PRVT_H__

#include <atomic>
#include "rk_aiq_types_algo_adebayer_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

typedef enum AdebayerState_e {
    ADEBAYER_STATE_INVALID           = 0,
    ADEBAYER_STATE_INITIALIZED       = 1,
    ADEBAYER_STATE_STOPPED           = 2,
    ADEBAYER_STATE_RUNNING           = 3,
    ADEBAYER_STATE_LOCKED            = 4,
    ADEBAYER_STATE_MAX
} AdebayerState_t;

typedef struct AdebayerFullParam_s {
    unsigned char enable;
    int iso[9];
    signed char filter1[5];
    signed char filter2[5];
    unsigned char gain_offset;
    unsigned char sharp_strength[9];
    unsigned char offset;
    unsigned short hf_offset[9];
    unsigned char clip_en;
    unsigned char filter_g_en;
    unsigned char filter_c_en;
    unsigned char thed0;
    unsigned char thed1;
    unsigned char dist_scale;
    unsigned char cnr_strength;
    unsigned char shift_num;
    std::atomic<bool> updated;
} AdebayerFullParam_t;

typedef struct AdebayerContext_s {
    AdebayerConfig_t config;
    AdebayerState_t state;
    CamCalibDbContext_t* pCalibDb;
    CamCalibDbV2Context_t *pCalibDbV2;
    AdebayerFullParam_t full_param;
    adebayer_attrib_manual_t manualAttrib;
    rk_aiq_debayer_op_mode_t mode;
    int iso;
} AdebayerContext_t;

typedef struct _RkAiqAlgoContext {
    AdebayerContext_t adebayerCtx;
} RkAiqAlgoContext;

#endif//__RKAIQ_TYPES_ALGO_ADEBAYER_PRVT_H__

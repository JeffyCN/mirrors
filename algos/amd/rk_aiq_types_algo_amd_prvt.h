#ifndef __RKAIQ_TYPES_ALGO_AMD_PRVT_H__
#define __RKAIQ_TYPES_ALGO_AMD_PRVT_H__

#include "base/xcam_common.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "amd/rk_aiq_types_amd_algo.h"
#include "amd/rk_aiq_types_algo_amd_int.h"
#include "media_buffer/media_buffer_pool.h"
#include "mfnr_head_v1.h"

#define  AMD_RATIO_BUF_NUM      (4)
typedef struct AmdContext_s {
    CamCalibDbContext_t* calib;
    CamCalibDbV2Context_t *calibv2;
    int imgWidth;
    int imgHeight;
    int imgAlignedW;
    int imgAlignedH;
    CalibDbV2_MFNR_TuningPara_Motion_t motion;
    bool enable;
    rk_aiq_amd_params_t params;

    uint8_t* ratio_out[AMD_RATIO_BUF_NUM];
    int ratio_size;
    //int cur_index;

    XCamVideoBuffer* spImage_prev;
    XCamVideoBuffer* ispGain_prev;
    XCamVideoBuffer* spImage;
    XCamVideoBuffer* ispGain;

    uint8_t *pPreAlpha;
    short *pTmpBuf;
    uint8_t *static_ratio;

    MediaBufPoolConfig_t BufPoolConfig;
    MediaBufPoolMemory_t  BufPoolMem;
    MediaBufPool_t BufPool;
} AmdContext_t;

typedef struct _RkAiqAlgoContext {
    AmdContext_t amdCtx;
} RkAiqAlgoContext;

typedef struct Mt_Params_Select_t
{
    int enable;
  	float sigmaHScale;
	float sigmaLScale;
	float light_clp;
	float uv_weight;
    float mfnr_sigma_scale;
    float yuvnr_gain_scale[3];
    float frame_limit_y;
    float frame_limit_uv;
    float gain_scale_l_y;
    float gain_scale_h_y;
    float gain_scale_l_uv;
    float gain_scale_h_uv;
} Mt_Params_Select_t;

#endif//__RKAIQ_TYPES_ALGO_AMD_PRVT_H__

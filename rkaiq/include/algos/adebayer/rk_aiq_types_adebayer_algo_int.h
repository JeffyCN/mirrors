
#ifndef __RKAIQ_TYPES_ADEBAYER_ALGO_INT_H__
#define __RKAIQ_TYPES_ADEBAYER_ALGO_INT_H__

#include "rk_aiq_types_adebayer_algo.h"
#include "debayer_head.h"

typedef enum rk_aiq_debayer_op_mode_e {
    RK_AIQ_DEBAYER_MODE_INVALID                     = 0,        /**< initialization value */
    RK_AIQ_DEBAYER_MODE_MANUAL                      = 1,        /**< run manual lens shading correction */
    RK_AIQ_DEBAYER_MODE_AUTO                        = 2,        /**< run auto lens shading correction */
    RK_AIQ_DEBAYER_MODE_MAX
} rk_aiq_debayer_op_mode_t;

typedef enum AdebayerHwVersion_e
{
    ADEBAYER_HARDWARE_V1 = 0,  // rk1126/1109/356X/3588
    ADEBAYER_HARDWARE_V2 = 1,  //rk1106
    ADEBAYER_HARDWARE_V2_LITE = 2,  //rk3562
    ADEBAYER_HARDWARE_V3 = 3,   //rk3576
    ADEBAYER_HARDWARE_MAX,
} AdebayerHwVersion_t;

typedef struct AdebayerSeletedParamV1_s {
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
} AdebayerSeletedParamV1_t;

typedef struct AdebayerSeletedParamV2_s {
    bool debayer_en;

    //filter coef
    int lowfreq_filter1[4];
    int highfreq_filter2[4];
    int c_alpha_gaus_coe[3];
    int c_guid_gaus_coe[3];
    int c_ce_gaus_coe[3];

    //g_interp
    unsigned char debayer_clip_en;
    unsigned short debayer_gain_offset;
    unsigned char debayer_max_ratio;

    //g_drctwgt
    unsigned short debayer_hf_offset;
    unsigned char debayer_thed0;
    unsigned char debayer_thed1;
    unsigned char debayer_dist_scale;
    unsigned char debayer_select_thed;

    //g_filter
    unsigned char debayer_gfilter_en;
    unsigned short debayer_gfilter_offset;

    //c_filter
    unsigned char debayer_cfilter_en;

    unsigned short debayer_loggd_offset;

    float debayer_cfilter_str;
    float debayer_wet_clip;
    float debayer_wet_ghost;
    float debayer_wgtslope;

    float debayer_bf_sgm;
    unsigned char  debayer_bf_clip;
    unsigned char  debayer_bf_curwgt;

    unsigned short debayer_loghf_offset;

    unsigned short debayer_alpha_offset;
    float debayer_alpha_scale;
    unsigned short debayer_edge_offset;
    float debayer_edge_scale;

} AdebayerSeletedParamV2_t;

typedef struct AdebayerSeletedParamV2Lite_s {
    bool debayer_en;

    //filter coef
    int lowfreq_filter1[4];
    int highfreq_filter2[4];

    //g_interp
    unsigned char debayer_clip_en;
    unsigned short debayer_gain_offset;
    unsigned char debayer_max_ratio;

    //g_drctwgt
    unsigned short debayer_hf_offset;
    unsigned char debayer_thed0;
    unsigned char debayer_thed1;
    unsigned char debayer_dist_scale;
    unsigned char debayer_select_thed;

    //g_filter
    unsigned char debayer_gfilter_en;
    unsigned short debayer_gfilter_offset;

} AdebayerSeletedParamV2Lite_t;

typedef struct AdebayerSeletedParamV3_s
{
    // debayer params
    bool    hw_dmT_en;
    char    hw_dmT_loDrctFlt_coeff[4];
    char    hw_dmT_hiDrctFlt_coeff[4];

    int     sw_dmT_luma_val[RK_DEBAYER_V31_LUMA_POINT_NUM];
    int     sw_dmT_drct_offset[RK_DEBAYER_V31_LUMA_POINT_NUM];

    float   sw_dmT_gradLoFlt_alpha;

    int     hw_dmT_hiDrct_thred;
    int     hw_dmT_loDrct_thred;
    int     hw_dmT_hiTexture_thred;
    int     hw_dmT_drctMethod_thred;

    float   sw_dmT_gInterpWgtFlt_alpha;

    int     hw_dmT_gInterpSharpStrg_offset;
    int     hw_dmT_gInterpSharpStrg_maxLimit;
    int     hw_dmT_gInterpClip_en;

    int     hw_dmT_gOutlsFlt_en;
    int     hw_dmT_gOutlsFlt_mode;
    int     hw_dmT_gOutlsFltRange_offset;
    int     sw_dmT_gOutlsFlt_vsigma[RK_DEBAYER_V31_LUMA_POINT_NUM];
    float   sw_dmT_gOutlsFltRsigma_en;
    float   sw_dmT_gOutlsFlt_rsigma;
    float   sw_dmT_gOutlsFlt_coeff[3];
    float   sw_dmT_gOutlsFlt_ratio;

    // chroma filter
    int     hw_dmT_cnrFlt_en;
    int     hw_dmT_cnrAlphaLpf_coeff[3];
    int     hw_dmT_cnrLoGuideLpf_coeff[3];
    int     hw_dmT_cnrPreFlt_coeff[3];

    float   hw_dmT_cnrMoireAlpha_offset;
    float   sw_dmT_cnrMoireAlpha_scale;
    float   hw_dmT_cnrEdgeAlpha_offset;
    float   sw_dmT_cnrEdgeAlpha_scale;
    int     hw_dmT_logTrans_en;
    int     hw_dmT_cnrLogGrad_offset;
    int     hw_dmT_cnrLogGuide_offset;
    float   sw_dmT_cnrLoFlt_vsigma;
    float   sw_dmT_cnrLoFltWgt_maxLimit;
    float   sw_dmT_cnrLoFltWgt_minThred;
    float   sw_dmT_cnrLoFltWgt_slope;

    float   sw_dmT_cnrHiFlt_vsigma;
    float   hw_dmT_cnrHiFltWgt_minLimit;
    float   hw_dmT_cnrHiFltCur_wgt;

} AdebayerSeletedParamV3_t;

typedef struct AdebayerProcResultV1_s {
    AdebayerHwConfigV1_t* config;
} AdebayerProcResultV1_t;

typedef struct AdebayerProcResultV2_s {
    AdebayerHwConfigV2_t* config;
} AdebayerProcResultV2_t;

typedef struct AdebayerProcResultV3_s {
    AdebayerHwConfigV3_t* config;
} AdebayerProcResultV3_t;

#endif//__RKAIQ_TYPES_ADEBAYER_ALGO_INT_H__

#ifndef _RK_AIQ_ISP32_MODULES_H_
#define _RK_AIQ_ISP32_MODULES_H_

#include <stdint.h>

#include "include/common/rk-camera-module.h"
#include "include/common/rk_aiq_comm.h"
#include "include/common/rkisp32-config.h"
#include "include/common/rk-isp33-config.h"
#include "include/common/rk-isp35-config.h"
#include "rk_aiq_isp_blc30.h"
#if RKAIQ_HAVE_DEHAZE_V14
#include "rk_aiq_isp_histeq23.h"
#elif RKAIQ_HAVE_HISTEQ_V10
#include "rk_aiq_isp_histeq30.h"
#endif
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
#include "rk_aiq_isp_texEst40.h"
#endif
#include "rk_aiq_isp_gain20.h"
#include "rk_aiq_types.h"

typedef struct RKAiqAecExpInfo_s RKAiqAecExpInfo_t;

typedef struct blc_res_cvt_s {
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPreTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of optical black correction before TNR.))  */
    blc_obcPreTnr_dyn_t obcPreTnr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPostTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dynamic params of optical black correction after TNR.))  */
    blc_obcPostTnr_dyn_t obcPostTnr;
} blc_res_cvt_t;

typedef struct otplsc_res_cvt_s {
    __u32 flag;
    __u16 width;
    __u16 height;
    __u16 x_size_tbl[17];
    __u16 y_size_tbl[17];
    __u16 lsc_r[RKMODULE_LSCDATA_LEN];
    __u16 lsc_b[RKMODULE_LSCDATA_LEN];
    __u16 lsc_gr[RKMODULE_LSCDATA_LEN];
    __u16 lsc_gb[RKMODULE_LSCDATA_LEN];
} otplsc_res_cvt_t;

typedef struct {
    uint32_t frameId;
    uint16_t rawWidth;
    uint16_t rawHeight;
    bool isGrayMode;
    bool isFirstFrame;
    uint8_t frameNum;
    float preDGain;
    float preDGain_preFrm;
    float preDGain_drc;
    int frameIso[3];
    float frameEt[3];
    float frameDGain[3];
    struct rkmodule_awb_inf otp_awb;
    struct rkmodule_lsc_inf otp_lsc;
    struct rkmodule_af_inf *otp_af;
    struct rkmodule_pdaf_inf *otp_pdaf;
#if RKAIQ_HAVE_AIBNR
    RkAiqAibnrModelInfo_t aibnrModelInfo;
#endif
#if RKAIQ_HAVE_AIYNR
    RkAiqAiynrModelInfo_t aiynrModelInfo;
#endif
    RKAiqAecExpInfo_t *ae_exp;
    blc_res_cvt_t blc_res;
    bool use_aiisp;
    int warning_signal;
    int warning_count;
    bool cnr_path_en;
    float L2S_Ratio;
#ifdef USE_NEWSTRUCT
    float ynr_sigma[HIST_SIGMA_LUT_NUM];
#endif
    bool cmps_on;
    bool cmps_is15bit;
    uint8_t cmps_offsetbit;
    bool dehaze_en;
    bool histeq_en;
    bool btnr_en;
    bool btnr_init_en;
    int btnrCfg_pixDomain_mode;
    bool btnrT_predgainWkArd_en;
    int btnrT_predgain_mode;
    float btnrT_predgain_curve;
    int btnrCfg_trans_mode;
    int btnrCfg_trans_offset;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    texEst_param_t texEst_param;
    int sw_btnrT_outFrmBase_mode;
#endif
    int drc_warning_count;
    int blc_warning_count;
    int btnr_warning_count;
    void* pBlcInfo;
    float offset2Blc1;
    int blc0_diff;
    uint16_t isp_ob_offset;
    uint16_t isp_ob_offset1;
    bool ieEn;
    bool pre_ccm_asym_adj_en;
    uint16_t ccm_pre_Y2Alpha_fac0[ISP32_CCM_CURVE_NUM*2];
    uint8_t bayer_fmt;
    otplsc_res_cvt_t otplsc_res;
    bool _airms_en;
    bool _aiynr_en;
    struct rkmodule_mge_oeWgt* mge_oe_wgt;
} common_cvt_info_t;

typedef struct {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* isp_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* isp_cfg;
#endif
} isp_params_t;

RKAIQ_BEGIN_DECLARE

void rk_aiq_dm21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
void rk_aiq_btnr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, bool bypass);
void rk_aiq_ynr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo);
void rk_aiq_sharp32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_dehaze22_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
// void rk_aiq_gamma21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);

RKAIQ_END_DECLARE
#endif


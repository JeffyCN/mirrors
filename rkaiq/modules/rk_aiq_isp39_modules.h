#ifndef _RK_AIQ_ISP39_MODULES_H_
#define _RK_AIQ_ISP39_MODULES_H_

#include "rk_aiq_isp32_modules.h"
#include "common/rk-isp39-config.h"

typedef struct {
    int bayertnr_logprecision;
    int bayertnr_logfixbit;
    int bayertnr_logtblbit;
    int bayertnr_logscalebit;
    int bayertnr_logfixmul;
    int bayertnr_logtblmul;
    int bayertnr_logtablef[64];
    int bayertnr_logtablei[64];
    int bayertnr_tnr_sigma_curve_double_pos;

    float bayertnr_wgt_stat;
    float bayertnr_pk_stat;

    bool bayertnr_auto_sig_count_en;
    int  bayertnr_auto_sig_count_filt_wgt;
    int bayertnr_auto_sig_count_valid;
    uint32_t bayertnr_auto_sig_count_max;


    uint16_t tnr_luma_sigma_y[20];
    int bayertnr_iso_pre;
} btnr_trans_params_v30_t;

typedef struct {
    bool bypass;
    bool mBtnrStatsValid;
    struct isp39_bay3d_stat mBtnrStats;
    btnr_trans_params_v30_t mBtnrTransParams;
} btnr_cvt_info_t;

RKAIQ_BEGIN_DECLARE

void rk_aiq_btnr40_update_sq(struct isp39_isp_params_cfg* isp_cfg, btnr_cvt_info_t *pBtnrInfo, uint32_t frameId);
void rk_aiq_btnr40_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo, btnr_cvt_info_t *pBtnrInfo);

void rk_aiq_sharp34_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_ynr34_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo);
void rk_aiq_cnr34_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo);
void rk_aiq_drc40_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_blc30_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_dpcc21_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg);

RKAIQ_END_DECLARE

#endif


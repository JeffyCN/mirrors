#ifndef _RK_AIQ_MODULE_BTNR_COMMON_H_
#define _RK_AIQ_MODULE_BTNR_COMMON_H_

#include "rk_aiq_module_common.h"

#define trans_mode2str(mode) \
    (mode) == 0 ? "btnr_pixInBw20b_mode" : \
    (mode) == 1 ? "btnr_pixInBw15b_mode" : \
    "INVALID MODE"

typedef struct {
    uint8_t isFirstFrame;
    uint8_t isHdrMode;
    uint8_t isTransfBypass;

    uint8_t transf_mode;
    uint8_t transf_mode_scale;
    uint16_t transf_mode_offset;
    uint16_t itransf_mode_offset;
    uint32_t transf_data_max_limit;

    uint8_t bayertnr_logprecision;
    uint8_t bayertnr_logfixbit;
    uint8_t bayertnr_logtblbit;
    uint8_t bayertnr_logscalebit;
    uint16_t bayertnr_logfixmul;
    uint16_t bayertnr_logtblmul;
    uint16_t bayertnr_logtablef[65];
    uint16_t bayertnr_logtablei[65];

    float bayertnr_wgt_stat;
    float bayertnr_pk_stat;
    uint16_t bayertnr_lo_wgt_clip_min_limit;
    uint16_t bayertnr_lo_wgt_clip_max_limit;

    bool bayertnr_auto_sig_count_en;
    bool bayertnr_auto_sig_count_valid;
    int  bayertnr_auto_sig_count_filt_wgt;
    uint32_t bayertnr_auto_sig_count_max;


    int bayertnr_iso_pre;
    uint16_t tnr_luma_sigma_x[20];
    uint16_t tnr_luma_sigma_y[20];
    bool bayertnr_tnr_sigma_curve_double_en;
    uint8_t bayertnr_tnr_sigma_curve_double_pos;

} btnr_trans_params_t;

typedef struct {
    uint32_t id;
    uint32_t sigma_num;
    uint16_t sigma_y[20];
} btnr_stats_t;

typedef struct {
    bool isValid;
    uint32_t id;
    uint16_t noise_curve[17];
} sharp_stats_t;

#define RK_AIQ_BTNR_STATS_CNT 5
#define BAYERTNR_STATS_DELAY 2

typedef struct {
    btnr_stats_t mBtnrStats[RK_AIQ_BTNR_STATS_CNT];
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    sharp_stats_t mSharpStats[RK_AIQ_BTNR_STATS_CNT];
    uint16_t sharp_noise_curve_pre[17];
#endif
    uint8_t gic_noise_auto_mode;
    btnr_trans_params_t mBtnrTransParams;
    int hw_btnrCfg_pixDomain_mode;
    uint16_t btnr_stats_miss_cnt;
    uint16_t sharp_stats_miss_cnt;
    uint16_t stats_buffer_cnt;
    uint16_t stats_delay_cnt;

    void *btnr_attrib;
} btnr_cvt_info_t;

void bayertnr_logtrans_init(int bayertnr_trans_mode,  int bayertnr_trans_mode_scale, btnr_trans_params_t *pTransPrarms);
int bayertnr_logtrans(uint32_t tmpfix, btnr_trans_params_t* pTransPrarms);
int bayertnr_logiitrans(int tmpData, btnr_trans_params_t* pTransPrarms);
int rk_autoblc_gen_tbl(unsigned int* bayertnr_itransf_tbl, uint16_t bayertnr_pixlog_max, btnr_trans_params_t* pTransPrarms);
int bayertnr_update_sq(btnr_trans_params_t* pTransPrarms);
void bayertnr_save_stats(void *stats_buffer, btnr_cvt_info_t *pBtnrInfo);
int bayertnr_autosigma_config(btnr_stats_t *pStats, btnr_trans_params_t *pTransPrarms, blc_res_cvt_t* pBlc);
int bayertnr_tnr_noise_curve(int data, int isHdrShort, btnr_trans_params_t *pTransPrarms);
void bayertnr_sigmaY_interpolate(u16 *pSigX, u16 *pSigY, u16 *pLowX, u16 *pHighX, u16 *pLowY, u16 *pHighY,  int sigbins, float iso_ratio);
uint16_t btnr_hdr_sigmaY_interpolate(int xData, uint16_t* sigcur_idx, uint16_t*sigcur_val, int sigbins);
float btnr_hdr_mergeWgt_interpolate(int xData, int* luma_idx, float* luma2wgt_val);
void btnr_hdr_sigma_calc(uint16_t* pSigmaX, uint16_t* pSigmaY, uint16_t* pCalibX, uint16_t*pShortY, uint16_t* pLongY, mergeLuma2Wgt_t* pMerge_luma2wgt, btnr_trans_params_t *pTransParams, int sigbins, float hdr_ratio);


#endif

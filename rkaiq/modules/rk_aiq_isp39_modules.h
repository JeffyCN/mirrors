#ifndef _RK_AIQ_ISP39_MODULES_H_
#define _RK_AIQ_ISP39_MODULES_H_

#include "rk_aiq_isp32_modules.h"
#include "common/rk-isp39-config.h"
#include "common/rk-isp33-config.h"
#include "rk_aiq_module_btnr_common.h"
#include "rk_aiq_module_common.h"
#include "blc_algo/aiq_blcLoader.h"
#include "include/algos/awb/rk_aiq_types_awb_algo.h"
#if ISP_HW_V39
typedef enum LutBufferState_e {
    kInitial   = 0,
    kWait2Chip = 1,
    kChipInUse = 2,
} LutBufferState;

typedef struct LutBufferConfig_s {
    bool IsBigMode;
    uint32_t Width;
    uint32_t Height;
    uint32_t LutHCount;
    uint32_t LutVCount;
    uint16_t ScaleFactor;
    uint16_t PsfCfgCount;
} LutBufferConfig;

typedef struct LutBuffer_s {
    LutBufferState State;
    LutBufferConfig Config;
    int Fd;
    int Size;
    void* Addr;
} LutBuffer;

typedef struct isp_drv_share_mem_ops_s isp_drv_share_mem_ops_t;

typedef struct LutBufferManager_s {
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    LutBufferConfig config_;
} LutBufferManager;

typedef struct {
    char iqpath[255];
    uint16_t rawWidth;
    uint16_t rawHeight;
    isp_drv_share_mem_ops_t* mem_ops;
    bool is_multi_sensor;
    bool is_multi_isp;
    uint8_t multi_isp_extended_pixel;
    int isp_unite_mode;
    LutBufferManager* lut_manger_;
    LutBuffer* current_lut_[2];
    uint16_t current_lut_size;
    float hdr_ratio;
} cac_cvt_info_t;

void LutBufferManagerDeinit(cac_cvt_info_t *cacInfo, LutBufferManager* man);
#endif

typedef struct {
    int ds_fd;
    int iir_fd;
    int gain_fd;
    int ds_size;
    void* ds_address;
    void* tnr_ds_buf;
    struct blcLibrary lib_Blc_;
    bool init_success;
    rk_autoblc_param_t blc1_param;
    unsigned int bayertnr_itransf_tbl[4096];
    int blc_adjust[66];
    float cur_adjust[2];
    int autoblc_count;
    float sigma_ratio;
    float blc_adjust_tbl[26];
    int pre_adjust[2];
    uint16_t pre_ob_offset;
} blc_cvt_info_t;


RKAIQ_BEGIN_DECLARE

void rk_aiq_btnr40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, btnr_cvt_info_t *pBtnrInfo, mergeLuma2Wgt_t* pMergeLumaWgt);
void rk_aiq_sharp34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_ynr34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_cnr34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_drc40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, bool drc_en);
void rk_aiq_blc30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo,
                             blc_cvt_info_t* pBlcInfo, rk_aiq_wb_gain_v32_t* awb_gain_final);
void rk_aiq_dpcc21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gamma21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gic21_params_cvt(void* attr, struct isp39_gic_cfg* gic_cfg);
void rk_aiq_dehaze23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_histeq23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_dehazeHisteq23_sigma_params_cvt(isp_params_t* isp_params,
        common_cvt_info_t* cvtinfo);
void rk_aiq_yme10_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
#if ISP_HW_V39
void rk_aiq_cac21_params_cvt(void* attr, isp_params_t* isp_params,
                             isp_params_t* isp_cfg_right, cac_cvt_info_t *cacInfo, common_cvt_info_t* cvtinfo);
#endif
void rk_aiq_ldch22_params_cvt(const common_cvt_info_t* cvtinfo, void* attr,
                              isp_params_t* isp_params, isp_params_t* isp_cfg_right,
                              bool is_multi_isp);
void rk_aiq_ldcv22_params_cvt(const common_cvt_info_t* cvtinfo, void* attr,
                              isp_params_t* isp_params, isp_params_t* isp_cfg_right,
                              bool is_multi_isp);
void rk_aiq_csm21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_dm23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_merge22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, mergeLuma2Wgt_t* pMerge_luma2wgt);
void rk_aiq_lsc21_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_ae25_stats_cfg_cvt(void* meas, isp_params_t* isp_params);

void rk_aiq_rgbir10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_cgc10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_cp10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gain20_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
void rk_aiq_lut3d20_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_ccm22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);

RKAIQ_END_DECLARE

#endif


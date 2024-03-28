#ifndef _RK_AIQ_ISP32_MODULES_H_
#define _RK_AIQ_ISP32_MODULES_H_

typedef struct {
    uint32_t frameId;
    uint16_t rawWidth;
    uint16_t rawHeight;
    bool isGrayMode;
    bool isFirstFrame;
    uint8_t frameNum;
    float preDGain;
    int frameIso[3];
    float frameEt[3];
    float frameDGain[3];
} common_cvt_info_t;

RKAIQ_BEGIN_DECLARE

void rk_aiq_dm21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
void rk_aiq_btnr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, bool bypass);
void rk_aiq_ynr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo);
void rk_aiq_sharp32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_dehaze22_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
void rk_aiq_gamma21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);

RKAIQ_END_DECLARE
#endif


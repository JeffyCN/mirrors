/*
 *  Copyright (c) 2021 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rk_aiq_user_ae_thread_v25_itf.h"
#include "c_base/aiq_base.h"
#ifndef USE_IMPLEMENT_C
#include "RkAiqCoreConfig.h"
#endif

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

RKAIQ_BEGIN_DECLARE

#define RKISP_ALGO_AE_DEMO_VERSION     "v0.0.1"
#define RKISP_ALGO_AE_DEMO_VENDOR      "Rockchip"
#define RKISP_ALGO_AE_DEMO_DESCRIPTION "Rockchip Ae algo_thread"

#define RK_AE

/* instance was created by AIQ framework when rk_aiq_uapi_sysctl_register3Aalgo called */
typedef struct _RkAiqAlgoContext {
    rk_aiq_pfnAe_t*    cbs; // set by register
    bool               isGrpMode;
    union {
        rk_aiq_sys_ctx_t* aiq_ctx;  // set by register
        rk_aiq_camgroup_ctx_t* group_ctx;  // set by register
    };
    rk_aiq_ae_algo_config_t aeCfg; // ae config of AIQ framework
    rk_aiq_ae_info_t   aeInfo;
    rk_aiq_ae_result_t aeResult;
} RkAiqAlgoContext;

/******************************************************************************
 * aeCISFeature()
 *****************************************************************************/
static void aeCISFeature
(
    rk_aiq_ae_algo_config_t* pConfig,
    RKAiqAecExpInfo_t* rkAeExp
) {
    int dcg_mode;
    float gain;

    if(pConfig->isHdr) {
        dcg_mode = rkAeExp->HdrExp[pConfig->hdrFrmNum - 1].exp_real_params.dcg_mode;
        gain = rkAeExp->HdrExp[pConfig->hdrFrmNum - 1].exp_real_params.analog_gain;
    } else {
        dcg_mode = rkAeExp->LinearExp.exp_real_params.dcg_mode;
        gain = rkAeExp->LinearExp.exp_real_params.analog_gain;
    }

    if(pConfig->stNRswitch.valid == true) {

        float up_thres = (float)pConfig->stNRswitch.up_thres / (float)pConfig->stNRswitch.div_coeff;
        float down_thres = (float)pConfig->stNRswitch.down_thres / (float)pConfig->stNRswitch.div_coeff;

        if(gain >= up_thres)
            rkAeExp->CISFeature.SNR = (pConfig->stNRswitch.direct == 0) ? 1 : 0;
        if(gain < down_thres)
            rkAeExp->CISFeature.SNR = (pConfig->stNRswitch.direct == 0) ? 0 : 1;

    } else {
        //LCG/HCG => SNR
        rkAeExp->CISFeature.SNR = dcg_mode > 0 ? 1 : 0;
    }
}

/******************************************************************************
 * aeReal2RegConv()
 *****************************************************************************/
static XCamReturn aeReal2RegConv
(
    rk_aiq_ae_algo_config_t* pConfig,
    float SplitIntegrationTime,
    float SplitGain,
    unsigned int *regIntegrationTime,
    unsigned int *regGain,
    int pDcgMode
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    float dcg_ratio = 0.0f;
    float CISTimeRegOdevity[2] = {0};
    unsigned int CISTimeRegMin = 0;

    if(pConfig->isHdr) {
        dcg_ratio = pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_ratio;
        CISTimeRegMin = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->hdrFrmNum - 2].CISTimeRegMin.Coeff[0];
        CISTimeRegOdevity[0] = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->hdrFrmNum - 2].CISTimeRegOdevity.fCoeff[0];
        CISTimeRegOdevity[1] = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->hdrFrmNum - 2].CISTimeRegOdevity.fCoeff[1];
    } else {
        dcg_ratio = pConfig->stSensorInfo.CISDcgSet.Linear.dcg_ratio;
        CISTimeRegMin = pConfig->stSensorInfo.CISTimeSet.Linear.CISTimeRegMin;
        CISTimeRegOdevity[0] = pConfig->stSensorInfo.CISTimeSet.Linear.CISTimeRegOdevity.fCoeff[0];
        CISTimeRegOdevity[1] = pConfig->stSensorInfo.CISTimeSet.Linear.CISTimeRegOdevity.fCoeff[1];
    }


    //gain convertion
    float ag = SplitGain / ((pDcgMode >= 1) ? dcg_ratio : 1.0f);

    if(pConfig->stSensorInfo.Gain2Reg.GainMode == EXPGAIN_MODE_LINEAR) {

        float C1 = 0.0f, C0 = 0.0f, M0 = 0.0f, minReg = 0.0f, maxReg = 0.0f;

        for (int i = 0; i < pConfig->stSensorInfo.Gain2Reg.GainRange_len; i += 7) {
            if (ag >= pConfig->stSensorInfo.Gain2Reg.GainRange[i] && ag <= pConfig->stSensorInfo.Gain2Reg.GainRange[i + 1]) {
                C1 = pConfig->stSensorInfo.Gain2Reg.GainRange[i + 2];
                C0 = pConfig->stSensorInfo.Gain2Reg.GainRange[i + 3];
                M0 = pConfig->stSensorInfo.Gain2Reg.GainRange[i + 4];
                minReg = pConfig->stSensorInfo.Gain2Reg.GainRange[i + 5];
                maxReg = pConfig->stSensorInfo.Gain2Reg.GainRange[i + 6];
                break;
            }
        }

        if (C1 == 0.0f) {
            LOGE_AEC_SUBM(0xff, "GAIN OUT OF RANGE: lasttime-gain: %f-%f", SplitIntegrationTime, SplitGain);
            C1 = 16;
            C0 = 0;
            M0 = 1;
            minReg = 16;
            maxReg = 255;
        }

        LOGV_AEC_SUBM(0xff, "ag: %2.2f, C1: %2.2f  C0: %2.2f M0: %2.2f, minReg: %2.2f maxReg: %2.2f",
                      ag, C1, C0, M0, minReg, maxReg);

        *regGain = (int)(C1 * pow(ag, M0) - C0 + 0.5f);
        if (*regGain < minReg)
            *regGain = minReg;
        if (*regGain > maxReg)
            *regGain = maxReg;

    } else if(pConfig->stSensorInfo.Gain2Reg.GainMode == EXPGAIN_MODE_NONLINEAR_DB) {
        *regGain = (int)(20.0f * log10(ag) * 10.0f / 3.0f + 0.5f);
    }

    //time convertion
    float timeC0 = pConfig->stSensorInfo.Time2Reg.fCoeff[0];
    float timeC1 = pConfig->stSensorInfo.Time2Reg.fCoeff[1];
    float timeC2 = pConfig->stSensorInfo.Time2Reg.fCoeff[2];
    float timeC3 = pConfig->stSensorInfo.Time2Reg.fCoeff[3];
    LOGV_AEC_SUBM(0xff, "time coefficient: %f-%f-%f-%f", timeC0, timeC1, timeC2, timeC3);

    float pclk = pConfig->PixelClockFreqMHZ;
    float hts = pConfig->PixelPeriodsPerLine;
    float vts = pConfig->LinePeriodsPerField;

    *regIntegrationTime = (int)(timeC0 * vts + timeC1 + timeC2 * ((SplitIntegrationTime * pclk * 1000000 / hts) + timeC3));

    int Index = (*regIntegrationTime - CISTimeRegOdevity[1]) / (CISTimeRegOdevity[0]);
    *regIntegrationTime = CISTimeRegOdevity[0] * Index + CISTimeRegOdevity[1];
    *regIntegrationTime = MAX(*regIntegrationTime, CISTimeRegMin);
    return (ret);
}

/******************************************************************************
 * initAeStatsCfg()
 *****************************************************************************/
static XCamReturn initAeStatsCfg(rk_aiq_ae_algo_config_t* pConfig)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    pConfig->aeStatsCfg.sw_aeCfg_stats_en = true;
    pConfig->aeStatsCfg.hw_aeCfg_entityGroup_mode = aeStats_entity03_indWk_mode;

    if(pConfig->isBuildInHdr) {
        pConfig->aeStatsCfg.entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode = aeStats_btnrOutLow_mode;
        pConfig->aeStatsCfg.entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode = aeStats_btnrOutHigh_mode;
    } else {
        if(pConfig->isHdr) {
            pConfig->aeStatsCfg.entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode = aeStats_entity0_chl0DpcOut_mode;
            pConfig->aeStatsCfg.entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode = aeStats_entity3_chl1DpcOut_mode;
        } else {
            pConfig->aeStatsCfg.entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode = aeStats_entity0_chl0DpcOut_mode;
            pConfig->aeStatsCfg.entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode = aeStats_dmIn_mode;
        }
    }

    //rawae0
    pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_winZone_mode = aeStats_winZone_15x15_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width = pConfig->rawWidth;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height = pConfig->rawHeight;

    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_x = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_y = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_x = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_y = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_x = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_y = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_height = 100;

    //rawhist0
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_winZone_mode = aeStats_winZone_15x15_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_win_width = pConfig->rawWidth;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_win_height = pConfig->rawHeight;

    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_pix_mode = aeStats_histPixY_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_pixIn_shift = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_pixVal_thred = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_pixSkip_step = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[0] = 0x4d;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[1] = 0x4b;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[2] = 0x1d;
    pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[3] = 0x00;

    memset(pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt, 0x01, RAWHISTBIG_WIN_NUM * sizeof(uint8_t));

    //rawae3
    pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_winZone_mode = aeStats_winZone_15x15_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width = pConfig->rawWidth;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height = pConfig->rawHeight;

    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[0].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_x = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_y = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[1].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[2].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_x = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_y = 2;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_height = 100;

    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[3].hw_aeCfg_subWin_en = false;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_x = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_y = 150;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_width = 100;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_height = 100;


    //rawhist3
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_winZone_mode = aeStats_winZone_15x15_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_win_x = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_win_y = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_win_width = pConfig->rawWidth;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_win_height = pConfig->rawHeight;

    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_pix_mode = aeStats_histPixY_mode;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_pixIn_shift = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_pixVal_thred = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_pixSkip_step = 0;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[0] = 0x4d;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[1] = 0x4b;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[2] = 0x1d;
    pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[3] = 0x00;

    memset(pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt, 0x01, RAWHISTBIG_WIN_NUM * sizeof(uint8_t));

    //coWkEntity03
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.mainWin =  pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin;
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.subWin[0] =  pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[0];
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.subWin[1] =  pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[1];
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.subWin[2] =  pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[2];
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.subWin[3] =  pConfig->aeStatsCfg.entityGroup.entities.entity0.subWin[3];
    pConfig->aeStatsCfg.entityGroup.coWkEntity03.hist =  pConfig->aeStatsCfg.entityGroup.entities.entity0.hist;

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

/******************************************************************************
 * initAeResult()
 *****************************************************************************/
static void initAeResult(rk_aiq_ae_result_t* pAeResult, rk_aiq_ae_algo_config_t* pConfig)
{
    pAeResult->pfn_result.frmLengthLines = pConfig->PixelPeriodsPerLine;
    pAeResult->pfn_result.isLongFrmMode = false;
    pAeResult->pfn_result.isRkExpValid = true;

    // 0.) exposure params
    if(pConfig->isHdr) {
        //hdr_exp 0:sframe 1:mframe 2:lframe, set exp according to hdrframe
        pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.integration_time = 0.003f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.analog_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.digital_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.isp_dgain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.iso = pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.integration_time = 0.01f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.analog_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.digital_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.isp_dgain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.iso = pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.integration_time = 0.02f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.analog_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.digital_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.isp_dgain = 1.0f;
        pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.iso = pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        //dcg calc by custom
        if(!pConfig->stSensorInfo.CISDcgSet.Hdr.support_en) {
            pAeResult->pfn_result.cisRkExp.hdr_exp[0].exp_real_params.dcg_mode = -1;
            pAeResult->pfn_result.cisRkExp.hdr_exp[1].exp_real_params.dcg_mode = -1;
            pAeResult->pfn_result.cisRkExp.hdr_exp[2].exp_real_params.dcg_mode = -1;
        }

        for(int i = 0; i < pConfig->hdrFrmNum; i++) {

            aeReal2RegConv(pConfig, pAeResult->pfn_result.cisRkExp.hdr_exp[i].exp_real_params.integration_time,
                           pAeResult->pfn_result.cisRkExp.hdr_exp[i].exp_real_params.analog_gain,
                           &pAeResult->pfn_result.cisRkExp.hdr_exp[i].exp_sensor_params.coarse_integration_time,
                           &pAeResult->pfn_result.cisRkExp.hdr_exp[i].exp_sensor_params.analog_gain_code_global,
                           pAeResult->pfn_result.cisRkExp.hdr_exp[i].exp_real_params.dcg_mode);

        }

    } else {
        //linear_exp
        pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.integration_time = 0.003f;
        pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.analog_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.digital_gain = 1.0f;
        pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.isp_dgain = 1.0f;
        pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.iso = pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        if(!pConfig->stSensorInfo.CISDcgSet.Linear.support_en)
            pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.dcg_mode = -1;

        aeReal2RegConv(pConfig, pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.integration_time,
                       pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.analog_gain,
                       &pAeResult->pfn_result.cisRkExp.linear_exp.exp_sensor_params.coarse_integration_time,
                       &pAeResult->pfn_result.cisRkExp.linear_exp.exp_sensor_params.analog_gain_code_global,
                       pAeResult->pfn_result.cisRkExp.linear_exp.exp_real_params.dcg_mode);

    }

    //1.) stats cfg
    pAeResult->pfn_result.statsCfg.entValidBit = AE_BIT(AE_HwEnt0_Bit) | AE_BIT(AE_HwEnt3_Bit);

    pAeResult->pfn_result.statsCfg.hwEnt0.mainWin.hw_aeCfg_win_x = pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x;
    pAeResult->pfn_result.statsCfg.hwEnt0.mainWin.hw_aeCfg_win_y = pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y;
    pAeResult->pfn_result.statsCfg.hwEnt0.mainWin.hw_aeCfg_win_width = pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width;
    pAeResult->pfn_result.statsCfg.hwEnt0.mainWin.hw_aeCfg_win_height = pConfig->aeStatsCfg.entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height;
    memcpy(pAeResult->pfn_result.statsCfg.hwEnt0.hist.hw_aeCfg_zone_wgt, pConfig->aeStatsCfg.entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));

    pAeResult->pfn_result.statsCfg.hwEnt3.mainWin.hw_aeCfg_win_x = pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x;
    pAeResult->pfn_result.statsCfg.hwEnt3.mainWin.hw_aeCfg_win_y = pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y;
    pAeResult->pfn_result.statsCfg.hwEnt3.mainWin.hw_aeCfg_win_width = pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width;
    pAeResult->pfn_result.statsCfg.hwEnt3.mainWin.hw_aeCfg_win_height = pConfig->aeStatsCfg.entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height;
    memcpy(pAeResult->pfn_result.statsCfg.hwEnt3.hist.hw_aeCfg_zone_wgt, pConfig->aeStatsCfg.entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));

    pAeResult->pfn_result.statsCfg.pSwCoWkEnt03 = &pAeResult->pfn_result.statsCfg.hwEnt0;
}

static
void pfnAePreRes2AePreRes(RkAiqAlgoContext* algo_ctx, RKAiqAecExpInfo_t exp_params, aeStats_entitiesStats_t stats, float* glb_env) {

    if(algo_ctx->aeCfg.isHdr) {

        float exp[2] = {0.0f, 0.0f};
        float luma[2] = {0.0f, 0.0f};

        exp[0] = exp_params.HdrExp[0].exp_real_params.integration_time * 1000 * exp_params.HdrExp[0].exp_real_params.analog_gain \
                 * exp_params.HdrExp[0].exp_real_params.isp_dgain * exp_params.HdrExp[0].exp_real_params.digital_gain;

        exp[1] = exp_params.HdrExp[1].exp_real_params.integration_time * 1000 * exp_params.HdrExp[1].exp_real_params.analog_gain \
                 * exp_params.HdrExp[1].exp_real_params.isp_dgain * exp_params.HdrExp[1].exp_real_params.digital_gain;

        for(int i = 0; i < AESTATS_ZONE_15x15_NUM; i++) {
            luma[0] += (float)(stats.entity0.mainWin.hw_ae_meanBayerGrGb_val[i] >> 4); //channelg is 12bit, channelr/channelb is 10bit
            luma[1] += (float)(stats.entity3.mainWin.hw_ae_meanBayerGrGb_val[i] >> 4); //channelg is 12bit, channelr/channelb is 10bit
        }
        luma[0] /= AESTATS_ZONE_15x15_NUM;
        luma[1] /= AESTATS_ZONE_15x15_NUM;

        glb_env[0] = luma[0] / exp[0];
        glb_env[1] = luma[1] / exp[1];

    } else {
        float exp = exp_params.LinearExp.exp_real_params.integration_time * 1000 * exp_params.LinearExp.exp_real_params.analog_gain \
                    * exp_params.LinearExp.exp_real_params.isp_dgain * exp_params.LinearExp.exp_real_params.digital_gain;

        float luma = 0.0f;
        for(int i = 0; i < AESTATS_ZONE_15x15_NUM; i++)
            luma += (float)(stats.entity0.mainWin.hw_ae_meanBayerGrGb_val[i] >> 4); //channelg is 12bit, channelr/channelb is 10bit
        luma /= AESTATS_ZONE_15x15_NUM;

        glb_env[0] = luma / exp;
    }


}

static
void pfnAeRes2AeRes(rk_aiq_ae_algo_config_t* pConfig, ae_pfnAe_results_t* pfnAeResult, RkAiqAlgoProcResAe* AeProcRes)
{

    //AE new sensor exposure
    if(pConfig->isHdr) {
        //Hdr

        if(pfnAeResult->isRkExpValid) {
            for(int i = 0; i < pConfig->hdrFrmNum; i++) {

                aeReal2RegConv(pConfig, pfnAeResult->cisRkExp.hdr_exp[i].exp_real_params.integration_time,
                               pfnAeResult->cisRkExp.hdr_exp[i].exp_real_params.analog_gain,
                               &pfnAeResult->cisRkExp.hdr_exp[i].exp_sensor_params.coarse_integration_time,
                               &pfnAeResult->cisRkExp.hdr_exp[i].exp_sensor_params.analog_gain_code_global,
                               pfnAeResult->cisRkExp.hdr_exp[i].exp_real_params.dcg_mode);

            }
            AeProcRes->exp_i2c_params->bValid = false;
            AeProcRes->exp_i2c_params->nNumRegs = 0;
        } else {
            AeProcRes->exp_i2c_params->bValid = true;
            AeProcRes->exp_i2c_params->nNumRegs = pfnAeResult->cisI2cExp.nNumRegs;
        }
        AeProcRes->new_ae_exp->HdrExp[0] = pfnAeResult->cisRkExp.hdr_exp[0];
        AeProcRes->new_ae_exp->HdrExp[1] = pfnAeResult->cisRkExp.hdr_exp[1];
        AeProcRes->new_ae_exp->HdrExp[2] = pfnAeResult->cisRkExp.hdr_exp[2];

    } else {
        //Linear
        if(pfnAeResult->isRkExpValid) {
            aeReal2RegConv(pConfig, pfnAeResult->cisRkExp.linear_exp.exp_real_params.integration_time,
                           pfnAeResult->cisRkExp.linear_exp.exp_real_params.analog_gain,
                           &pfnAeResult->cisRkExp.linear_exp.exp_sensor_params.coarse_integration_time,
                           &pfnAeResult->cisRkExp.linear_exp.exp_sensor_params.analog_gain_code_global,
                           pfnAeResult->cisRkExp.linear_exp.exp_real_params.dcg_mode);
            AeProcRes->exp_i2c_params->bValid = false;
            AeProcRes->exp_i2c_params->nNumRegs = 0;
        } else {

            AeProcRes->exp_i2c_params->bValid = true;
            AeProcRes->exp_i2c_params->nNumRegs = pfnAeResult->cisI2cExp.nNumRegs;
        }

        AeProcRes->new_ae_exp->LinearExp = pfnAeResult->cisRkExp.linear_exp;
    }

    if(!pfnAeResult->isRkExpValid) {
        if(pfnAeResult->cisI2cExp.nNumRegs <= MAX_I2CDATA_LEN) {
            for(uint32_t i = 0; i < pfnAeResult->cisI2cExp.nNumRegs; i++) {
                AeProcRes->exp_i2c_params->DelayFrames[i] = pfnAeResult->cisI2cExp.pDelayFrames[i];
                AeProcRes->exp_i2c_params->RegAddr[i] = pfnAeResult->cisI2cExp.pRegAddr[i];
                AeProcRes->exp_i2c_params->AddrByteNum[i] = pfnAeResult->cisI2cExp.pAddrByteNum[i];
                AeProcRes->exp_i2c_params->RegValue[i] = pfnAeResult->cisI2cExp.pRegValue[i];
                AeProcRes->exp_i2c_params->ValueByteNum[i] = pfnAeResult->cisI2cExp.pValueByteNum[i];
            }
        } else {
            LOGE("too many i2c data to set!!");
        }
    }

    AeProcRes->new_ae_exp->frame_length_lines = pfnAeResult->frmLengthLines;
    AeProcRes->new_ae_exp->Iris = pfnAeResult->iris;

    AeProcRes->ae_proc_res_rk->exp_set_cnt = 1;
    AeProcRes->ae_proc_res_rk->exp_set_tbl[0] = *AeProcRes->new_ae_exp;
    AeProcRes->ae_proc_res_rk->LongFrmMode = pfnAeResult->isLongFrmMode;
    AeProcRes->ae_proc_res_rk->IsConverged = pfnAeResult->isConverged;

    //RK: CIS feature for NR
    aeCISFeature(pConfig, &AeProcRes->ae_proc_res_rk->exp_set_tbl[0]);

    //AE new HW config
    AeProcRes->ae_stats_cfg->sw_aeCfg_update = true;
    if(pfnAeResult->statsCfg.entValidBit & AE_BIT(AE_SwCoWkEnt03_Bit)) {
        AeProcRes->ae_stats_cfg->hw_aeCfg_entityGroup_mode = aeStats_entity03_coWk_mode;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_height;

        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.hist.hw_aeCfg_win_x = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.hist.hw_aeCfg_win_y = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.hist.hw_aeCfg_win_width = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.hist.hw_aeCfg_win_height = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_height;

        memcpy(AeProcRes->ae_stats_cfg->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt, \
               pfnAeResult->statsCfg.pSwCoWkEnt03->hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));
    } else {
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_height;

        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_height;

        memcpy(AeProcRes->ae_stats_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt, \
               pfnAeResult->statsCfg.hwEnt0.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));

        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_height;

        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_x;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_y;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_width;
        AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_height;

        memcpy(AeProcRes->ae_stats_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt, \
               pfnAeResult->statsCfg.hwEnt3.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));
    }

}

static
void pfnGrpAeRes2GrpAeRes(rk_aiq_ae_algo_config_t* pConfig, rk_aiq_singlecam_3a_result_t** rk_aiq_singlecam_3a_result,
                          ae_pfnAe_results_t* pfnAeResult)
{
    ae_pfnAe_single_results_t* pfnAeRes = pfnAeResult->next;

    for(int i = 0; i < pConfig->camIdArrayLen; i++) {

        *rk_aiq_singlecam_3a_result[i]->aec.exp_tbl_size = 1;

        //copy single exposure
        if(i > 0 && pfnAeRes != NULL) {
            if(pfnAeResult->isRkExpValid) {
                if(pConfig->isHdr) {
                    for(int j = 0; j < pConfig->hdrFrmNum; j++) {

                        aeReal2RegConv(pConfig, pfnAeRes->cisRkExp.hdr_exp[j].exp_real_params.integration_time,
                                       pfnAeRes->cisRkExp.hdr_exp[j].exp_real_params.analog_gain,
                                       &pfnAeRes->cisRkExp.hdr_exp[j].exp_sensor_params.coarse_integration_time,
                                       &pfnAeRes->cisRkExp.hdr_exp[j].exp_sensor_params.analog_gain_code_global,
                                       pfnAeRes->cisRkExp.hdr_exp[j].exp_real_params.dcg_mode);

                        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].HdrExp[j] = pfnAeRes->cisRkExp.hdr_exp[j];
                        rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = false;
                        rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = 0;
                    }
                } else {
                    aeReal2RegConv(pConfig, pfnAeRes->cisRkExp.linear_exp.exp_real_params.integration_time,
                                   pfnAeRes->cisRkExp.linear_exp.exp_real_params.analog_gain,
                                   &pfnAeRes->cisRkExp.linear_exp.exp_sensor_params.coarse_integration_time,
                                   &pfnAeRes->cisRkExp.linear_exp.exp_sensor_params.analog_gain_code_global,
                                   pfnAeRes->cisRkExp.linear_exp.exp_real_params.dcg_mode);

                    rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].LinearExp = pfnAeRes->cisRkExp.linear_exp;
                    rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = false;
                    rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = 0;
                }
            } else {


                rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].HdrExp[0] = pfnAeRes->cisRkExp.hdr_exp[0]; //also need float exposure
                rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].HdrExp[1] = pfnAeRes->cisRkExp.hdr_exp[1]; //also need float exposure
                rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].HdrExp[2] = pfnAeRes->cisRkExp.hdr_exp[2]; //also need float exposure
                rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].LinearExp = pfnAeRes->cisRkExp.linear_exp; //also need float exposure

                rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = true;
                rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = pfnAeRes->cisI2cExp.nNumRegs;

                if(!pfnAeResult->isRkExpValid) {
                    if(pfnAeRes->cisI2cExp.nNumRegs <= MAX_I2CDATA_LEN) {
                        for(uint32_t i = 0; i < pfnAeRes->cisI2cExp.nNumRegs; i++) {
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->DelayFrames[i] = pfnAeRes->cisI2cExp.pDelayFrames[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->RegAddr[i] = pfnAeRes->cisI2cExp.pRegAddr[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->AddrByteNum[i] = pfnAeRes->cisI2cExp.pAddrByteNum[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->RegValue[i] = pfnAeRes->cisI2cExp.pRegValue[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->ValueByteNum[i] = pfnAeRes->cisI2cExp.pValueByteNum[i];
                        }
                    } else {
                        LOGE("too many i2c data to set!!");
                    }
                }
            }

            pfnAeRes = pfnAeRes->next;
        } else {
            if(pfnAeResult->isRkExpValid) {
                if(pConfig->isHdr) {
                    for(int j = 0; j < pConfig->hdrFrmNum; j++) {

                        aeReal2RegConv(pConfig, pfnAeResult->cisRkExp.hdr_exp[j].exp_real_params.integration_time,
                                       pfnAeResult->cisRkExp.hdr_exp[j].exp_real_params.analog_gain,
                                       &pfnAeResult->cisRkExp.hdr_exp[j].exp_sensor_params.coarse_integration_time,
                                       &pfnAeResult->cisRkExp.hdr_exp[j].exp_sensor_params.analog_gain_code_global,
                                       pfnAeResult->cisRkExp.hdr_exp[j].exp_real_params.dcg_mode);

                        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].HdrExp[j] = pfnAeResult->cisRkExp.hdr_exp[j];
                        rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = false;
                        rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = 0;
                    }
                } else {
                    aeReal2RegConv(pConfig, pfnAeResult->cisRkExp.linear_exp.exp_real_params.integration_time,
                                   pfnAeResult->cisRkExp.linear_exp.exp_real_params.analog_gain,
                                   &pfnAeResult->cisRkExp.linear_exp.exp_sensor_params.coarse_integration_time,
                                   &pfnAeResult->cisRkExp.linear_exp.exp_sensor_params.analog_gain_code_global,
                                   pfnAeResult->cisRkExp.linear_exp.exp_real_params.dcg_mode);

                    rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].LinearExp = pfnAeResult->cisRkExp.linear_exp;
                    rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = false;
                    rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = 0;
                }
            } else {
                rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->bValid = true;
                rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->nNumRegs = pfnAeResult->cisI2cExp.nNumRegs;

                if(!pfnAeResult->isRkExpValid) {
                    if(pfnAeResult->cisI2cExp.nNumRegs <= MAX_I2CDATA_LEN) {
                        for(uint32_t i = 0; i < pfnAeResult->cisI2cExp.nNumRegs; i++) {
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->DelayFrames[i] = pfnAeResult->cisI2cExp.pDelayFrames[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->RegAddr[i] = pfnAeResult->cisI2cExp.pRegAddr[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->AddrByteNum[i] = pfnAeResult->cisI2cExp.pAddrByteNum[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->RegValue[i] = pfnAeResult->cisI2cExp.pRegValue[i];
                            rk_aiq_singlecam_3a_result[i]->aec.exp_i2c_params->ValueByteNum[i] = pfnAeResult->cisI2cExp.pValueByteNum[i];
                        }
                    } else {
                        LOGE("too many i2c data to set!!");
                    }
                }
            }
        }

        //copy common result
        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].frame_length_lines =  pfnAeResult->frmLengthLines;
        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].Iris =  pfnAeResult->iris;

        aeCISFeature(pConfig, &rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0]);

        //copy common RK result
        rk_aiq_singlecam_3a_result[i]->aec._aeProcRes.LongFrmMode = pfnAeResult->isLongFrmMode;
        rk_aiq_singlecam_3a_result[i]->aec._aeProcRes.IsConverged = pfnAeResult->isConverged;

        //copy hw result
        rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->sw_aeCfg_update = true;
        if(pfnAeResult->statsCfg.entValidBit & AE_BIT(AE_SwCoWkEnt03_Bit)) {
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->hw_aeCfg_entityGroup_mode = aeStats_entity03_coWk_mode;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_x;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_y;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_width;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.pSwCoWkEnt03->mainWin.hw_aeCfg_win_height;

            memcpy(rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt, \
                   pfnAeResult->statsCfg.pSwCoWkEnt03->hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));
        } else {
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_x;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_y;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_width;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt0.mainWin.hw_aeCfg_win_height;

            memcpy(rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt, \
                   pfnAeResult->statsCfg.hwEnt0.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));

            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_x;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_y;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_width;
            rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height = pfnAeResult->statsCfg.hwEnt3.mainWin.hw_aeCfg_win_height;

            memcpy(rk_aiq_singlecam_3a_result[i]->aec._aeNewCfg->entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt, \
                   pfnAeResult->statsCfg.hwEnt3.hist.hw_aeCfg_zone_wgt, AESTATS_ZONE_15x15_NUM * sizeof(uint8_t));
        }
    }

}

static XCamReturn AeDemoCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);

    RESULT ret = RK_AIQ_RET_SUCCESS;

    if(cfg->cbs) {

        RkAiqAlgoContext *ctx = (RkAiqAlgoContext*)aiq_mallocz(sizeof(RkAiqAlgoContext));
        if (ctx == NULL) {
            printf( "%s: create ae context fail!\n", __FUNCTION__);
            return XCAM_RETURN_ERROR_MEM;
        }
        memset(ctx, 0, sizeof(*ctx));

        ctx->aeCfg.fps_request = -1;
        ctx->aeCfg.update_fps = false;

        CalibDb_Sensor_ParaV2_t* calibv2_sensor_calib =
            (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, sensor_calib));

        ctx->aeCfg.stSensorInfo = *calibv2_sensor_calib;

        if (cfg->isGroupMode) {
            AlgoCtxInstanceCfgCamGroup* grpCfg = (AlgoCtxInstanceCfgCamGroup*)cfg;
            memcpy(ctx->aeCfg.camIdArray, grpCfg->camIdArray, sizeof(ctx->aeCfg.camIdArray));
            ctx->aeCfg.camIdArrayLen = grpCfg->camIdArrayLen;
            ctx->aeCfg.isGroupMode = true;
        } else {
            ctx->aeCfg.camIdArrayLen = 0;
            ctx->aeCfg.isGroupMode = false;
        }
        if(ctx->cbs) free(ctx->cbs);
        ctx->cbs = (rk_aiq_pfnAe_t*)aiq_mallocz(1 * sizeof(rk_aiq_pfnAe_t));
        memcpy(ctx->cbs, (rk_aiq_pfnAe_t*)cfg->cbs, sizeof(rk_aiq_pfnAe_t));

        *context = ctx;
    } else {
        if (cfg->isGroupMode) {
#ifdef RKAIQ_ENABLE_CAMGROUP
            g_RkIspAlgoDescCamgroupAe.common.create_context(context, cfg);
            (*context)->isGrpMode = true;
#endif
        } else {
            g_RkIspAlgoDescAe.common.create_context(context, cfg);
        }
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AeDemoDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);

    if(context == NULL) {
        return XCAM_RETURN_NO_ERROR;
    }

    if(context->cbs) {

        if (context->cbs->pfn_ae_exit) {
            context->cbs->pfn_ae_exit(context->aiq_ctx);
            context->aeCfg.isInit = false;
        }

        free(context->cbs);
        free(context);
        context = NULL;
    } else {
        if(context->isGrpMode) {
#ifdef RKAIQ_ENABLE_CAMGROUP
            g_RkIspAlgoDescCamgroupAe.common.destroy_context(context);
#endif
        } else {
            g_RkIspAlgoDescAe.common.destroy_context(context);
        }
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}


static XCamReturn AeDemoPrepare(RkAiqAlgoCom* params)
{
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoContext* algo_ctx = params->ctx;

    if(algo_ctx->cbs) {

        if (!algo_ctx->aeCfg.isInit) {
            algo_ctx->cbs->pfn_ae_init(algo_ctx->aiq_ctx);
            algo_ctx->aeCfg.isInit = true;
        }

        // 0.) working mode (normal / hdr 2 / hdr 3)
        algo_ctx->aeCfg.workMode = params->u.prepare.working_mode;

        if(algo_ctx->aeCfg.workMode <= RK_AIQ_WORKING_MODE_NORMAL) {
            algo_ctx->aeCfg.isHdr = false;
        } else {
            algo_ctx->aeCfg.isHdr = true;
            if(algo_ctx->aeCfg.workMode < RK_AIQ_WORKING_MODE_ISP_HDR3)
                algo_ctx->aeCfg.hdrFrmNum = 2;
            else
                algo_ctx->aeCfg.hdrFrmNum = 3;
        }

        // 1.) resolution
        algo_ctx->aeCfg.rawWidth = params->u.prepare.sns_op_width;
        algo_ctx->aeCfg.rawHeight = params->u.prepare.sns_op_height;

        if(algo_ctx->aeCfg.isGroupMode) {

            RkAiqAlgoCamGroupPrepare* AeCfgParam = (RkAiqAlgoCamGroupPrepare*)params;

            //read info from sensor (hts vts pclk)
            algo_ctx->aeCfg.LinePeriodsPerField = AeCfgParam->aec.LinePeriodsPerField;
            algo_ctx->aeCfg.PixelClockFreqMHZ = AeCfgParam->aec.PixelClockFreqMHZ;
            algo_ctx->aeCfg.PixelPeriodsPerLine = AeCfgParam->aec.PixelPeriodsPerLine;
            algo_ctx->aeCfg.stNRswitch = AeCfgParam->aec.nr_switch;

        } else {

            RkAiqAlgoConfigAe* AeCfgParam = (RkAiqAlgoConfigAe*)params;

            if(AeCfgParam->compr_bit > 0)
                algo_ctx->aeCfg.isBuildInHdr = true;
            else
                algo_ctx->aeCfg.isBuildInHdr = false;

            //read info from sensor (hts vts pclk)
            algo_ctx->aeCfg.LinePeriodsPerField = AeCfgParam->LinePeriodsPerField;
            algo_ctx->aeCfg.PixelClockFreqMHZ = AeCfgParam->PixelClockFreqMHZ;
            algo_ctx->aeCfg.PixelPeriodsPerLine = AeCfgParam->PixelPeriodsPerLine;
            algo_ctx->aeCfg.stNRswitch = AeCfgParam->nr_switch;

        }

        algo_ctx->aeCfg.init_fps = algo_ctx->aeCfg.PixelClockFreqMHZ * 1000000
                                   / (algo_ctx->aeCfg.LinePeriodsPerField * algo_ctx->aeCfg.PixelPeriodsPerLine);
        algo_ctx->aeCfg.last_fps = algo_ctx->aeCfg.init_fps;

        if (algo_ctx->aeCfg.fps_request == -1) {
            algo_ctx->aeCfg.last_vts = algo_ctx->aeCfg.LinePeriodsPerField;
            algo_ctx->aeCfg.vts_request = algo_ctx->aeCfg.last_vts;
        }

        // 3.) set initial hw config & initial ae result, initial value can be modified by user in processing

        if(!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
            //initial hw
            initAeStatsCfg(&algo_ctx->aeCfg); //for group ae, initial value can be the same

            //initial ae result (exposure, hw res)
            initAeResult(&algo_ctx->aeResult, &algo_ctx->aeCfg); //for group, only init full customRes

            algo_ctx->aeCfg.updateCalib = false;
        } else {
            //update calib
            algo_ctx->aeCfg.updateCalib = true;
        }
    } else {
        if(algo_ctx->isGrpMode) {
            LOGE("GROUP PREPARE");
#ifdef RKAIQ_ENABLE_CAMGROUP
            ret = g_RkIspAlgoDescCamgroupAe.prepare(params);
#endif
        } else {
            ret = g_RkIspAlgoDescAe.prepare(params);
        }

    }
    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

static XCamReturn AeDemoPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);

    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if(algo_ctx->cbs) {

        if(algo_ctx->aeCfg.isGroupMode) {
            RkAiqAlgoCamGroupProcOut* AeProcResParams = (RkAiqAlgoCamGroupProcOut*)outparams;
            if(!inparams->u.proc.init) {

                for(int j = 0; j < AeProcResParams->arraySize; j++) {

#ifdef USE_IMPLEMENT_C
                    AlgoRstShared_t* aePreRes_c = AeProcResParams->camgroupParmasArray[j]->aec._aePreRes_c;
                    RkAiqAlgoPreResAe* aePreRes = (RkAiqAlgoPreResAe*)aePreRes_c->_data;
#else
                    XCamVideoBuffer* XaePreRes = AeProcResParams->camgroupParmasArray[j]->aec._aePreRes;
                    RkAiqAlgoPreResAe* aePreRes = (RkAiqAlgoPreResAe*)XaePreRes->map(XaePreRes);
#endif

                    pfnAePreRes2AePreRes(algo_ctx, AeProcResParams->camgroupParmasArray[j]->aec.aec_stats_v25->ae_exp, \
                                         AeProcResParams->camgroupParmasArray[j]->aec.aec_stats_v25->ae_data.entityGroup.entities, \
                                         aePreRes->ae_pre_res_rk.GlobalEnvLv);

                }

            }
        } else {

            RkAiqAlgoPreResAe* AePreResParams = (RkAiqAlgoPreResAe*)outparams;

            if(!inparams->u.proc.init) {
                RkAiqAlgoPreAe* AePreParams = (RkAiqAlgoPreAe*)inparams;

                RKAiqAecStatsV25_t* xAecStats = AePreParams->aecStatsV25Buf;

                pfnAePreRes2AePreRes(algo_ctx, xAecStats->ae_exp, xAecStats->ae_data.entityGroup.entities, AePreResParams->ae_pre_res_rk.GlobalEnvLv);

            }
        }

    } else {
        if(algo_ctx->isGrpMode == false)
            ret = g_RkIspAlgoDescAe.pre_process(inparams, outparams);
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn AeDemoProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAe* AeProcParams = (RkAiqAlgoProcAe*)inparams;
    RkAiqAlgoProcResAe* AeProcResParams = (RkAiqAlgoProcResAe*)outparams;
    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if(algo_ctx->cbs) {
        if (algo_ctx->aeCfg.isGroupMode) {
            LOGE_AEC("wrong aec mode");
            return ret;
        }

        if(!inparams->u.proc.init) { // init=ture, stats=null

            RKAiqAecStatsV25_t* xAecStats = AeProcParams->aecStatsV25Buf;
            if (!xAecStats) {
                LOGE_AEC("aec stats is null");
                return(XCAM_RETURN_ERROR_FAILED);
            }
            //info: stats
            algo_ctx->aeInfo.pfn_info.entValidBit = algo_ctx->aeResult.pfn_result.statsCfg.entValidBit;
            algo_ctx->aeInfo.pfn_info.pHwEnt0 = &xAecStats->ae_data.entityGroup.entities.entity0;
            //algo_ctx->aeInfo.pfn_info.pHwEnt1 = &xAecStats->ae_data.entityGroup.entities.entity1;
            //algo_ctx->aeInfo.pfn_info.pHwEnt2 = &xAecStats->ae_data.entityGroup.entities.entity2;
            algo_ctx->aeInfo.pfn_info.pHwEnt3 = &xAecStats->ae_data.entityGroup.entities.entity3;
            algo_ctx->aeInfo.pfn_info.pSwCoWkEnt03 = &xAecStats->ae_data.entityGroup.coWkEnt03;

            //info: exp in rk mode
            if(algo_ctx->aeCfg.isHdr) {
                algo_ctx->aeInfo.pfn_info.cisRkExp.hdr_exp[0] = xAecStats->ae_exp.HdrExp[0];
                algo_ctx->aeInfo.pfn_info.cisRkExp.hdr_exp[1] = xAecStats->ae_exp.HdrExp[1];
                algo_ctx->aeInfo.pfn_info.cisRkExp.hdr_exp[2] = xAecStats->ae_exp.HdrExp[2];
            } else {
                algo_ctx->aeInfo.pfn_info.cisRkExp.linear_exp = xAecStats->ae_exp.LinearExp;
            }


            if (algo_ctx->cbs->pfn_ae_run)
                algo_ctx->cbs->pfn_ae_run(algo_ctx->aiq_ctx,
                                          &algo_ctx->aeInfo.pfn_info,
                                          &algo_ctx->aeResult.pfn_result
                                         );
        } else {
            if (algo_ctx->aeCfg.updateCalib) {
                LOGD_AEC_SUBM(0xff, "updateCalib, no need re-init");
                return ret;
            }

            if (algo_ctx->cbs->pfn_ae_run)
                algo_ctx->cbs->pfn_ae_run(algo_ctx->aiq_ctx,
                                          NULL,
                                          &algo_ctx->aeResult.pfn_result
                                         );
        }

        // gen patrt of hw results from initAecHwConfig
        memcpy(AeProcResParams->ae_stats_cfg, &algo_ctx->aeCfg.aeStatsCfg, sizeof(aeStats_cfg_t));

        // copy custom result to rk result
        pfnAeRes2AeRes(&algo_ctx->aeCfg, &algo_ctx->aeResult.pfn_result, AeProcResParams);

        if (algo_ctx->aeCfg.updateCalib) {
            algo_ctx->aeCfg.updateCalib = false;
        }
    } else {
        ret = g_RkIspAlgoDescAe.processing(inparams, outparams);
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AeDemoGroupProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoCamGroupProcIn* AeProcParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* AeProcResParams = (RkAiqAlgoCamGroupProcOut*)outparams;
    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if(algo_ctx->cbs) {
        if (!algo_ctx->aeCfg.isGroupMode) {
            LOGD_AEC("wrong aec mode for group");
            return ret;
        }

        if(!inparams->u.proc.init) { // init=ture, stats=null

            ae_pfnAe_info_t* pfnAe_info = &algo_ctx->aeInfo.pfn_info;
            ae_pfnAe_info_t* next_pfnAe_info = NULL;
            ae_pfnAe_results_t* pfnAe_res = &algo_ctx->aeResult.pfn_result;
            ae_pfnAe_single_results_t* next_pfnAe_res = NULL;

            RKAiqAecStatsV25_t*     aec_stats_v25 = NULL;

            // 0) copy stats & exp into pfn_info
            for(int i = 0; i < algo_ctx->aeCfg.camIdArrayLen; i++) {
                aec_stats_v25 = AeProcParams->camgroupParmasArray[i]->aec.aec_stats_v25;
                if (!aec_stats_v25) {
                    LOGE_GAEC("aec stats is null for %dth camera", i);
                    return(XCAM_RETURN_ERROR_FAILED);
                }

                if(i > 0) {
                    if(pfnAe_info->next == NULL)
                        pfnAe_info->next = (ae_pfnAe_info_t*)aiq_mallocz(1 * sizeof(ae_pfnAe_info_t));

                    next_pfnAe_info = pfnAe_info->next;
                    pfnAe_info = next_pfnAe_info;
                }

                //info: stats
                pfnAe_info->entValidBit = algo_ctx->aeResult.pfn_result.statsCfg.entValidBit;
                pfnAe_info->pHwEnt0 = &aec_stats_v25->ae_data.entityGroup.entities.entity0;
                //pfnAe_info->pHwEnt1 = &aec_stats_v25->ae_data.entityGroup.entities.entity1;
                //pfnAe_info->pHwEnt2 = &aec_stats_v25->ae_data.entityGroup.entities.entity2;
                pfnAe_info->pHwEnt3 = &aec_stats_v25->ae_data.entityGroup.entities.entity3;
                pfnAe_info->pSwCoWkEnt03 = &aec_stats_v25->ae_data.entityGroup.coWkEnt03;

                //info: exp in rk mode
                if(algo_ctx->aeCfg.isHdr) {
                    pfnAe_info->cisRkExp.hdr_exp[0] = aec_stats_v25->ae_exp.HdrExp[0];
                    pfnAe_info->cisRkExp.hdr_exp[1] = aec_stats_v25->ae_exp.HdrExp[1];
                    pfnAe_info->cisRkExp.hdr_exp[2] = aec_stats_v25->ae_exp.HdrExp[2];
                } else {
                    pfnAe_info->cisRkExp.linear_exp = aec_stats_v25->ae_exp.LinearExp;
                }
            }

            // 1) pfn_run
            if (algo_ctx->cbs->pfn_ae_run)
                algo_ctx->cbs->pfn_ae_run(algo_ctx->group_ctx,
                                          &algo_ctx->aeInfo.pfn_info,
                                          &algo_ctx->aeResult.pfn_result
                                         );
            // 2) release pfn_info->next
            while(pfnAe_res->next != NULL) {
                next_pfnAe_res = pfnAe_res->next->next;
                free(pfnAe_res->next);
                pfnAe_res->next = next_pfnAe_res;
            }

        } else {

            if (algo_ctx->aeCfg.updateCalib) {
                LOGD_AEC_SUBM(0xff, "updateCalib, no need re-init");
                return ret;
            }

            if (algo_ctx->cbs->pfn_ae_run)
                algo_ctx->cbs->pfn_ae_run(algo_ctx->group_ctx,
                                          NULL,
                                          &algo_ctx->aeResult.pfn_result
                                         );
        }

        // gen patrt of hw results from initAecHwConfig
        for(int i = 0; i < AeProcResParams->arraySize; i++)
            memcpy(AeProcResParams->camgroupParmasArray[i]->aec._aeNewCfg, &algo_ctx->aeCfg.aeStatsCfg, sizeof(aeStats_cfg_t));
        // copy custom result to rk result
        pfnGrpAeRes2GrpAeRes(&algo_ctx->aeCfg, AeProcResParams->camgroupParmasArray, &algo_ctx->aeResult.pfn_result);
    } else {
#ifdef RKAIQ_ENABLE_CAMGROUP
        ret = g_RkIspAlgoDescCamgroupAe.processing(inparams, outparams);
#endif
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;

}

static XCamReturn AeDemoPostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RESULT ret = RK_AIQ_RET_SUCCESS;

    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if(algo_ctx->cbs == NULL) {
        if(algo_ctx->isGrpMode == false)
            ret = g_RkIspAlgoDescAe.post_process(inparams, outparams);
    }

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static XCamReturn AeDemoDump(const RkAiqAlgoCom* inparams, st_string* result)
{
    RESULT ret = RK_AIQ_RET_SUCCESS;

    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if(algo_ctx->cbs == NULL) {

        if(algo_ctx->isGrpMode) {
#ifdef RKAIQ_ENABLE_CAMGROUP
            ret = g_RkIspAlgoDescCamgroupAe.dump(inparams, result);
#endif
        } else {
            ret = g_RkIspAlgoDescAe.dump(inparams, result);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif
//static std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*> g_customAe_desc_map;

XCamReturn
rk_aiq_uapi2_ae_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pfnAe_t* cbs)
{

    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const rk_aiq_camgroup_ctx_t* group_ctx = NULL;
    rk_aiq_sys_ctx_t* cast_ctx             = NULL;

    int algoType = 0;
    int algoId   = 0;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGI_AEC_SUBM(0xff, "group ae");
        group_ctx = (const rk_aiq_camgroup_ctx_t*)ctx;
#ifdef RKAIQ_ENABLE_CAMGROUP
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (!cast_ctx && group_ctx->cam_ctxs_array[i]) {
                cast_ctx = group_ctx->cam_ctxs_array[i];
                break;
            }
        }
#endif
        algoType   = RK_AIQ_ALGO_TYPE_AE;
        algoId     = 0;
    } else {
        LOGI_AEC_SUBM(0xff, "single ae");
        cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);
        algoType = RK_AIQ_ALGO_TYPE_AE;
        algoId   = 0;
    }

    bool isAeRgst = rk_aiq_uapi2_sysctl_getAxlibStatus(ctx,
                    algoType,
                    algoId);
    if (cbs == NULL && isAeRgst) {
        return XCAM_RETURN_BYPASS;
    } else if (isAeRgst) {
        // unregister rk ae to register custom ae
        rk_aiq_uapi2_ae_unRegister(ctx);
        LOGK_AEC("unRegister rk ae");
    }

    RkAiqAlgoDescription* desc = NULL;

#if 0
    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAe_desc_map.find(cast_ctx);

    if (it == g_customAe_desc_map.end()) {
        desc = new RkAiqAlgoDescription();
        g_customAe_desc_map[cast_ctx] = desc;
    } else {
        desc = it->second;
    }
#else
    if(cast_ctx->ae_desc == NULL)
        cast_ctx->ae_desc = (RkAiqAlgoDescription*)aiq_mallocz(1 * sizeof(RkAiqAlgoDescription));
    desc = cast_ctx->ae_desc;
#endif

    desc->common.version = RKISP_ALGO_AE_DEMO_VERSION;
    desc->common.vendor  = RKISP_ALGO_AE_DEMO_VENDOR;
    desc->common.description = RKISP_ALGO_AE_DEMO_DESCRIPTION;
    desc->common.type    = RK_AIQ_ALGO_TYPE_AE;
    desc->common.id      = 0;
    desc->common.create_context  = AeDemoCreateCtx;
    desc->common.destroy_context = AeDemoDestroyCtx;
    desc->prepare = AeDemoPrepare;
    desc->pre_process = AeDemoPreProcess;
    desc->processing = AeDemoProcessing;
    desc->post_process = AeDemoPostProcess;
#if RKAIQ_HAVE_DUMPSYS
    desc->dump = AeDemoDump;
#endif

    static RkAiqGrpCondition_t aeGrpCondV3x[] = {
        [0] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
    };
    RkAiqGrpConditions_t aeGrpCondsV3x = {grp_conds_array_info(aeGrpCondV3x)};

    struct RkAiqAlgoDesCommExt algoDes[] = {
#if defined(ISP_HW_V39)
        { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  5,  0, aeGrpCondsV3x},
#endif
#if defined(ISP_HW_V33)
        { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  6,  0, aeGrpCondsV3x},
#endif
#if defined(ISP_HW_V35)
        { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  7,  0, aeGrpCondsV3x},
#endif
        { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0,  0,  0, {0, 0} },
    };

    if (group_ctx) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (group_ctx->cam_ctxs_array[i]) {
                isAeRgst = rk_aiq_uapi2_sysctl_getAxlibStatus(group_ctx->cam_ctxs_array[i],
                           algoType, algoId);
                if (isAeRgst) {
                    continue;
                }

                ret = rk_aiq_uapi2_sysctl_register3Aalgo(group_ctx->cam_ctxs_array[i], &algoDes, NULL);
                if (ret == XCAM_RETURN_ERROR_ANALYZER) {
                    LOGE_AEC_SUBM(0xff, "no current aiq core status, please stop aiq before register custome ae!");
                    return ret;
                } else if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_AEC_SUBM(0xff, "ae register error, ret %d", ret);
                    return ret;
                }
            }
        }
#endif
    } else {
        ret = rk_aiq_uapi2_sysctl_register3Aalgo(ctx, &algoDes, cbs);
        if (ret == XCAM_RETURN_ERROR_ANALYZER) {
            LOGE_AEC_SUBM(0xff, "no current aiq core status, please stop aiq before register custome ae!");
            return ret;
        } else if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AEC_SUBM(0xff, "ae register error, ret %d", ret);
            return ret;
        }
    }

    if (group_ctx) {
        cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);
#if 0
        it = g_customAe_desc_map.find(cast_ctx);

        if (it == g_customAe_desc_map.end()) {
            desc = new RkAiqAlgoDescription();
            g_customAe_desc_map[cast_ctx] = desc;
        } else {
            desc = it->second;
        }
#else
        if(cast_ctx->ae_desc == NULL)
            cast_ctx->ae_desc = (RkAiqAlgoDescription*)aiq_mallocz(1 * sizeof(RkAiqAlgoDescription));
        desc = cast_ctx->ae_desc;
#endif

        desc->common.version = RKISP_ALGO_AE_DEMO_VERSION;
        desc->common.vendor  = RKISP_ALGO_AE_DEMO_VENDOR;
        desc->common.description = RKISP_ALGO_AE_DEMO_DESCRIPTION;
        desc->common.type    = RK_AIQ_ALGO_TYPE_AE;
        desc->common.id      = 0;
        desc->common.create_context  = AeDemoCreateCtx;
        desc->common.destroy_context = AeDemoDestroyCtx;
        desc->prepare = AeDemoPrepare;
        desc->pre_process = AeDemoPreProcess;
        desc->processing = AeDemoGroupProcessing;
        desc->post_process = AeDemoPostProcess;
#if RKAIQ_HAVE_DUMPSYS
        desc->dump = AeDemoDump;
#endif

        struct RkAiqAlgoDesCommExt algoDes_group[] = {
#if defined(ISP_HW_V39)
            { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  5,  0,  {0, 0}},
#endif
#if defined(ISP_HW_V33)
            { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  6,  0,  {0, 0}},
#endif
#if defined(ISP_HW_V35)
            { &desc->common, RK_AIQ_CORE_ANALYZE_AE, 0,  6,  0,  {0, 0}},
#endif
            { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0,  0,  0, {0, 0} },
        };

        ret = rk_aiq_uapi2_sysctl_register3Aalgo(ctx, &algoDes_group, cbs);
        if (ret == XCAM_RETURN_ERROR_ANALYZER) {
            LOGE_AEC_SUBM(0xff, "no current aiq core status, please stop aiq before register custome ae!");
            return ret;
        } else if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AEC_SUBM(0xff, "ae register error, ret %d", ret);
            return ret;
        }
    }

    RkAiqAlgoContext* algoCtx = rk_aiq_uapi2_sysctl_getAxlibCtx(ctx,
                                desc->common.type,
                                desc->common.id);

    if (algoCtx == NULL) {
        LOGE_AEC_SUBM(0xff, "can't get ae algo %d ctx!", desc->common.id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if(!cbs) {
        LOGD_AEC_SUBM(0xff, "RK AE");
    } else {
        LOGD_AEC_SUBM(0xff, "CUSTOM AE");
        algoCtx->aiq_ctx = (rk_aiq_sys_ctx_t*)(ctx);
        LOGD_AEC_SUBM(0xff, "register custom ae algo sucess for sys_ctx %p, lib_id %d !",
                      ctx,
                      desc->common.id);
    }

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

XCamReturn
rk_aiq_uapi2_ae_enable(const rk_aiq_sys_ctx_t* ctx, bool enable)
{

    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_ae_unRegister(const rk_aiq_sys_ctx_t* ctx)
{

    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = (rk_aiq_sys_ctx_t*)(ctx);

#if 0
    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAe_desc_map.find(cast_ctx);

    if (it == g_customAe_desc_map.end()) {
        LOGE_AEC_SUBM(0xff, "can't find custom ae algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }
#else
    if(!cast_ctx->ae_desc) {
        LOGE_AEC_SUBM(0xff, "can't find custom ae algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    }
    desc = cast_ctx->ae_desc;
#endif

    rk_aiq_uapi2_sysctl_unRegister3Aalgo(ctx,
                                         desc->common.type);

    LOGD_AEC_SUBM(0xff, "unregister custom ae algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);

#if 0
    delete it->second;
    g_customAe_desc_map.erase(it);
#else
    if(cast_ctx->ae_desc)
        free(cast_ctx->ae_desc);
    cast_ctx->ae_desc = NULL;
#endif

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

void rk_aiq_uapi2_ae_setFrameRate(RkAiqAlgoContext* context, float fps)
{
    if (fps > 0) {
        context->aeCfg.fps_request = fps;
        context->aeCfg.update_fps = true;
    } else {
        LOGE("%s: wrong set fps value: %f\n", __func__, fps);
    }

}

float rk_aiq_uapi2_ae_getFrameRate(RkAiqAlgoContext* context)
{
    return context->aeCfg.last_fps;
}


RKAIQ_END_DECLARE

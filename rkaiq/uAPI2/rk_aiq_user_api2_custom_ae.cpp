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

#include "rk_aiq_user_api2_custom_ae.h"
#include "RkAiqCalibDbV2Helper.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

RKAIQ_BEGIN_DECLARE

#define RKISP_ALGO_AE_DEMO_VERSION     "v0.0.1"
#define RKISP_ALGO_AE_DEMO_VENDOR      "Rockchip"
#define RKISP_ALGO_AE_DEMO_DESCRIPTION "Rockchip Custom Ae"

typedef struct rk_aiq_rkAe_config_s {
    int Working_mode;//values look up in rk_aiq_working_mode_t definiton
    int RawWidth;
    int RawHeight;
    CalibDb_Sensor_ParaV2_t stSensorInfo;
    RkAiqAecHwConfig_t    aeHwConfig;

    rk_aiq_sensor_nr_switch_t stNRswitch; //get from sensor
    float        LinePeriodsPerField; //get from sensor
    float        PixelClockFreqMHZ; //get from sensor
    float        PixelPeriodsPerLine; //get from sensor

    float last_fps;
    float init_fps;
    float last_vts;

    float vts_request;
    float fps_request;
    bool update_fps;

    uint8_t HdrFrmNum;
    bool IsHdr;
} rk_aiq_rkAe_config_t;

/* instance was created by AIQ framework when rk_aiq_uapi_sysctl_regLib called */
typedef struct _RkAiqAlgoContext {
    rk_aiq_customeAe_cbs_t cbs; // set by register
    union {
        rk_aiq_sys_ctx_t* aiq_ctx;  // set by register
        rk_aiq_camgroup_ctx_t* group_ctx;  // set by register
    };
    rk_aiq_rkAe_config_t rkCfg; // ae config of AIQ framework
    rk_aiq_customeAe_results_t customRes; // result of pfn_ae_run
    bool cutomAeInit;
    bool updateCalib;
    int camIdArray[6];
    int camIdArrayLen;
    bool isGroupMode;
} RkAiqAlgoContext;

/******************************************************************************
 * AeReg2RealConv()
 *****************************************************************************/
static XCamReturn AeReg2RealConv
(
    rk_aiq_rkAe_config_t* pConfig,
    int sensorGain,
    int sensorInttime,
    int sensorDcgmode,
    float& realGain,
    float& realInttime
) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //gain
    if(pConfig->stSensorInfo.Gain2Reg.GainMode == EXPGAIN_MODE_LINEAR) {
        float gainRange[] = {1, 2, 128, 0, 1, 128, 255,
                             2, 4, 64, -248, 1, 376, 504,
                             4, 8, 32, -756, 1, 884, 1012,
                             8, 16, 16, -1784, 1, 1912, 2040
                            };

        float* pgainrange = NULL;
        uint32_t size = 0;

        if (pConfig->stSensorInfo.Gain2Reg.GainRange_len <= 0) {
            pgainrange = gainRange;
            size = sizeof(gainRange) / sizeof(float);
        } else {
            pgainrange = pConfig->stSensorInfo.Gain2Reg.GainRange;
            size = pConfig->stSensorInfo.Gain2Reg.GainRange_len;
        }

        int *revert_gain_array = (int *)malloc((size / 7 * 2) * sizeof(int));
        if(revert_gain_array == NULL) {
            LOGE_AEC("%s: malloc fail", __func__);
            return  XCAM_RETURN_ERROR_MEM;
        }

        for(uint32_t i = 0; i < (size / 7); i++) {
            revert_gain_array[i * 2 + 0] = (int)(pgainrange[i * 7 + 2] * pow(pgainrange[i * 7 + 0], pgainrange[i * 7 + 4]) - pgainrange[i * 7 + 3] + 0.5f);
            revert_gain_array[i * 2 + 1] = (int)(pgainrange[i * 7 + 2] * pow(pgainrange[i * 7 + 1], pgainrange[i * 7 + 4]) - pgainrange[i * 7 + 3] + 0.5f);
        }

        // AG = (C1 * (analog gain^M0) - C0) + 0.5f
        float C1 = 0.0f, C0 = 0.0f, M0 = 0.0f, minReg = 0.0f, maxReg = 0.0f, minrealGain = 0.0f, maxrealGain = 0.0f;
        float ag = sensorGain;
        uint32_t i = 0;
        for(i = 0; i < (size / 7); i++) {
            if (ag >= revert_gain_array[i * 2 + 0] && ag <= revert_gain_array[i * 2 + 1]) {
                C1 = pgainrange[i * 7 + 2];
                C0 = pgainrange[i * 7 + 3];
                M0 = pgainrange[i * 7 + 4];
                minReg = pgainrange[i * 7 + 5];
                maxReg = pgainrange[i * 7 + 6];
                break;
            }
        }

        if(i > (size / 7)) {
            LOGE_AEC_SUBM(0xff, "GAIN OUT OF RANGE: lasttime-gain: %d-%d", sensorInttime, sensorGain);
            C1 = 16;
            C0 = 0;
            M0 = 1;
            minReg = 16;
            maxReg = 255;
        }

        realGain = pow(10, log10(((float)sensorGain + C0) / C1) / M0);
        minrealGain = pow(10, log10(((float)minReg + C0) / C1) / M0);
        maxrealGain = pow(10, log10(((float)maxReg + C0) / C1) / M0);

        if (realGain < minrealGain)
            realGain = minrealGain;
        if (realGain > maxrealGain)
            realGain = maxrealGain;

        if(revert_gain_array != NULL) {
            free(revert_gain_array);
            revert_gain_array = NULL;
        }
    } else if(pConfig->stSensorInfo.Gain2Reg.GainMode == EXPGAIN_MODE_NONLINEAR_DB) {
        realGain = pow(10, (float)sensorGain * 3 / (10.0f * 20.0f));
    }

    float dcg_ratio = (sensorDcgmode >= 1 ? pConfig->stSensorInfo.CISDcgSet.Linear.dcg_ratio : 1.0f);

    realGain *= dcg_ratio;

    //time
    float timeC0 = pConfig->stSensorInfo.Time2Reg.fCoeff[0];
    float timeC1 = pConfig->stSensorInfo.Time2Reg.fCoeff[1];
    float timeC2 = pConfig->stSensorInfo.Time2Reg.fCoeff[2];
    // float timeC3 = pConfig->stSensorInfo.Time2Reg.fCoeff[3];

    realInttime = ((sensorInttime - timeC0 * pConfig->LinePeriodsPerField - timeC1) / timeC2 /*- timeC3*/) *
                  pConfig->PixelPeriodsPerLine / (pConfig->PixelClockFreqMHZ * 1000000);

    return (ret);
}

/******************************************************************************
 * AeReal2RegConv()
 *****************************************************************************/
static XCamReturn AeReal2RegConv
(
    rk_aiq_rkAe_config_t* pConfig,
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

    if(pConfig->IsHdr) {
        dcg_ratio = pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_ratio;
        CISTimeRegMin = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->HdrFrmNum - 2].CISTimeRegMin;
        CISTimeRegOdevity[0] = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->HdrFrmNum - 2].CISTimeRegOdevity.fCoeff[0];
        CISTimeRegOdevity[1] = pConfig->stSensorInfo.CISTimeSet.Hdr[pConfig->HdrFrmNum - 2].CISTimeRegOdevity.fCoeff[1];
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
 * AeDcgConv
 *****************************************************************************/
static XCamReturn AeDcgConv
(
    rk_aiq_rkAe_config_t*  pConfig,
    float                  Gain,
    int*                   pDcgMode
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AEC_SUBM(0xff, "%s:(enter)\n", __FUNCTION__);

    //pointer check
    if (pConfig == NULL) {
        LOGE_AEC_SUBM(0xff, "%s: pConfig NULL pointer! \n", __FUNCTION__);
        return (XCAM_RETURN_ERROR_FAILED);
    }

    if(!pConfig->stSensorInfo.CISDcgSet.Linear.support_en) {
        *pDcgMode = -1;
        return XCAM_RETURN_NO_ERROR;
    }

    if(pConfig->stSensorInfo.CISDcgSet.Linear.dcg_optype <= RK_AIQ_OP_MODE_AUTO) {

        if(Gain >= pConfig->stSensorInfo.CISDcgSet.Linear.lcg2hcg_gain_th) {
            *pDcgMode = 1;
        } else if(Gain < pConfig->stSensorInfo.CISDcgSet.Linear.hcg2lcg_gain_th) {
            *pDcgMode = 0;
        }

        LOG1_AEC_SUBM(0xff, "gain=%f,dcg_mode=[%d]", Gain, *pDcgMode);
    } else {
        *pDcgMode = pConfig->stSensorInfo.CISDcgSet.Linear.dcg_mode.Coeff[0];
    }

    LOG1_AEC_SUBM(0xff, "%s: (exit)\n", __FUNCTION__);
    return (ret);
}

/******************************************************************************
 * AeHdrDcgConv
 *****************************************************************************/
static XCamReturn AeHdrDcgConv
(
    rk_aiq_rkAe_config_t*  pConfig,
    RkAiqExpParamComb_t*   pHdrExp
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AEC_SUBM(0xff, "%s:(enter)\n", __FUNCTION__);

    //pointer check
    if (pConfig == NULL) {
        LOGE_AEC_SUBM(0xff, "%s: pConfig NULL pointer! \n", __FUNCTION__);
        return (XCAM_RETURN_ERROR_FAILED);
    }

    if(!pConfig->stSensorInfo.CISDcgSet.Linear.support_en) {
        pHdrExp[0].exp_real_params.dcg_mode = -1;
        pHdrExp[1].exp_real_params.dcg_mode = -1;
        pHdrExp[2].exp_real_params.dcg_mode = -1;
        return XCAM_RETURN_NO_ERROR;
    }

    if(pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_optype <= RK_AIQ_OP_MODE_AUTO) {

        if(pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.analog_gain >= pConfig->stSensorInfo.CISDcgSet.Hdr.lcg2hcg_gain_th)
            pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.dcg_mode = 1;
        else if(pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.analog_gain < pConfig->stSensorInfo.CISDcgSet.Hdr.hcg2lcg_gain_th)
            pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.dcg_mode = 0;

        if(pConfig->stSensorInfo.CISDcgSet.Hdr.sync_switch) {
            if(pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.dcg_mode == 1 && \
                    pHdrExp[0].exp_real_params.analog_gain < pConfig->stSensorInfo.CISGainSet.CISExtraAgainRange.Min) {

                if(pHdrExp[0].exp_real_params.analog_gain >= pConfig->stSensorInfo.CISDcgSet.Hdr.lcg2hcg_gain_th)
                    pHdrExp[0].exp_real_params.dcg_mode = 1;
                else if(pHdrExp[0].exp_real_params.analog_gain < pConfig->stSensorInfo.CISDcgSet.Hdr.hcg2lcg_gain_th)
                    pHdrExp[0].exp_real_params.dcg_mode = 0;

                pHdrExp[1].exp_real_params.dcg_mode = pHdrExp[0].exp_real_params.dcg_mode;
                pHdrExp[2].exp_real_params.dcg_mode = pHdrExp[0].exp_real_params.dcg_mode;

            } else {
                for(int i = 0; i < pConfig->HdrFrmNum - 1; i++)
                    pHdrExp[i].exp_real_params.dcg_mode = pHdrExp[pConfig->HdrFrmNum - 1].exp_real_params.dcg_mode;

            }
        } else {
            for(int i = 0; i < pConfig->HdrFrmNum - 1; i++) {
                if(pHdrExp[i].exp_real_params.analog_gain >= pConfig->stSensorInfo.CISDcgSet.Hdr.lcg2hcg_gain_th)
                    pHdrExp[i].exp_real_params.dcg_mode = 1;
                else if(pHdrExp[i].exp_real_params.analog_gain < pConfig->stSensorInfo.CISDcgSet.Hdr.hcg2lcg_gain_th)
                    pHdrExp[i].exp_real_params.dcg_mode = 0;
            }
        }

    } else {
        pHdrExp[0].exp_real_params.dcg_mode = pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_mode.Coeff[0];
        pHdrExp[1].exp_real_params.dcg_mode = pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_mode.Coeff[1];
        pHdrExp[2].exp_real_params.dcg_mode = pConfig->stSensorInfo.CISDcgSet.Hdr.dcg_mode.Coeff[2];
    }
    LOG1_AEC_SUBM(0xff, "%s: (exit)\n", __FUNCTION__);
    return (ret);
}

/******************************************************************************
* AeGridWeight15x15to5x5()
*****************************************************************************/
static void AeGridWeight15x15to5x5
(
    unsigned char*  inWeights,
    unsigned char*  outWeights
)
{

    uint8_t line_5x5, col_5x5;
    uint8_t line_15x15, col_15x15;

    int SumWeight[RAWAELITE_WIN_NUM] = {0};

    for(int i = 0; i < 225; i++) {
        line_15x15 = i / 15;
        col_15x15 = i % 15;

        line_5x5 = line_15x15 / 3;
        col_5x5 = col_15x15 / 3;

        SumWeight[line_5x5 * 5 + col_5x5] += inWeights[line_15x15 * 15 + col_15x15];
    }

    for(int i = 0; i < 25; i++)
        outWeights[i] = SumWeight[i] / 9;

    /*for (int i = 0; i<5; i++){
           printf("%2d %2d %2d %2d %2d\n",
             outWeights[i * 5 + 0], outWeights[i * 5 + 1], outWeights[i * 5 + 2], outWeights[i * 5 + 3], outWeights[i * 5 + 4]);
    }*/

}

static void AeCISFeature
(
    rk_aiq_rkAe_config_t* pConfig,
    RKAiqAecExpInfo_t* rkAeExp
) {
    int dcg_mode;
    float gain;

    if(pConfig->IsHdr) {
        dcg_mode = rkAeExp->HdrExp[pConfig->HdrFrmNum - 1].exp_real_params.dcg_mode;
        gain = rkAeExp->HdrExp[pConfig->HdrFrmNum - 1].exp_real_params.analog_gain;
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

static XCamReturn AeDemoCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);

    RESULT ret = RK_AIQ_RET_SUCCESS;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        printf( "%s: create ae context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(ctx, 0, sizeof(*ctx));

    ctx->rkCfg.fps_request = -1;
    ctx->rkCfg.update_fps = false;

    CalibDb_Sensor_ParaV2_t* calibv2_sensor_calib =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, sensor_calib));

    ctx->rkCfg.stSensorInfo = *calibv2_sensor_calib;

    if (cfg->isGroupMode) {
        AlgoCtxInstanceCfgCamGroup* grpCfg = (AlgoCtxInstanceCfgCamGroup*)cfg;
        memcpy(ctx->camIdArray, grpCfg->camIdArray, sizeof(ctx->camIdArray));
        ctx->camIdArrayLen = grpCfg->camIdArrayLen;
        ctx->isGroupMode = true;
    } else {
        ctx->camIdArrayLen = 0;
        ctx->isGroupMode = false;
    }
    *context = ctx;
    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AeDemoDestroyCtx(RkAiqAlgoContext *context)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);

    if(context == NULL)
        return XCAM_RETURN_NO_ERROR;

    if (context->cbs.pfn_ae_exit) {
        context->cbs.pfn_ae_exit(context->aiq_ctx);
        context->cutomAeInit = false;
    }
    delete context;
    context = NULL;

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn initAecHwConfig(rk_aiq_rkAe_config_t* pConfig)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    /**************** set rawae_sel to choose BIG/LITE mode ******************/
    /*      rawae0-2.rawae_sel should be the same, defining BIG/LITE mode of chn[0-2]    */
    /*    rawae3.rawae_sel is different from rawae0-2, defining BIG/LITE mode of debayer  */
    /*****************************************************************/

#if defined(ISP_HW_V30)
    if(pConfig->HdrFrmNum < 3) {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 2;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 2;
        pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 2;
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 3;
    } else {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 3;
    }
#endif

#if defined(ISP_HW_V21)
    if(pConfig->IsHdr) {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 1;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 1; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 1; //raw.chn[0] = BIG, make all channel = BIG
    } else {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 1;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 1; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 3;
    }
#endif

#if defined(ISP_HW_V32)
    if(pConfig->IsHdr) {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 0;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 0;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 0; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 0; //raw.chn[0] = BIG, make all channel = BIG

    } else {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 1;
        pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 1;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 1; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 3; //raw.chn[0] = BIG, make all channel = BIG
    }

#endif

#if defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39)
    if(pConfig->IsHdr) {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 0;
        //pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 0;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 0; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 1; //raw.chn[0] = BIG, make all channel = BIG

    } else {
        pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel = 0;
        //pConfig->aeHwConfig.ae_meas.rawae1.rawae_sel = 0;
        //pConfig->aeHwConfig.ae_meas.rawae2.rawae_sel = 0; //rawae2 no effective
        pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel = 0; //raw.chn[0] = BIG, make all channel = BIG
    }
#endif


    pConfig->aeHwConfig.hist_meas.ae_swap = pConfig->aeHwConfig.ae_meas.rawae0.rawae_sel;
    pConfig->aeHwConfig.hist_meas.ae_sel = pConfig->aeHwConfig.ae_meas.rawae3.rawae_sel;

#if defined(ISP_HW_V39)
    /*****rawae0, BIG mode****/
    pConfig->aeHwConfig.ae_meas.rawae0.wnd_num = 2;
    pConfig->aeHwConfig.ae_meas.rawae0.win.h_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae0.win.v_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae0.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.ae_meas.rawae0.win.v_size = pConfig->RawHeight;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[0].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[0].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[0].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[0].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[1].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[1].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[1].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[1].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[2].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[2].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[2].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[2].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[3].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[3].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[3].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin[3].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae0.subwin_en[0] = 1;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin_en[1] = 1;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin_en[2] = 1;
    pConfig->aeHwConfig.ae_meas.rawae0.subwin_en[3] = 1;
#else
    /*****rawae0, LITE mode****/
    pConfig->aeHwConfig.ae_meas.rawae0.wnd_num = 1;
    pConfig->aeHwConfig.ae_meas.rawae0.win.h_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae0.win.v_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae0.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.ae_meas.rawae0.win.v_size = pConfig->RawHeight;
#endif


#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
    /*****rawae1, BIG mode****/
    pConfig->aeHwConfig.ae_meas.rawae1.wnd_num = 2;
    pConfig->aeHwConfig.ae_meas.rawae1.win.h_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae1.win.v_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae1.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.ae_meas.rawae1.win.v_size = pConfig->RawHeight;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[0].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[0].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[0].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[0].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[1].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[1].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[1].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[1].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[2].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[2].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[2].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[2].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[3].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[3].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[3].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin[3].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae1.subwin_en[0] = 1;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin_en[1] = 1;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin_en[2] = 1;
    pConfig->aeHwConfig.ae_meas.rawae1.subwin_en[3] = 1;
#endif

#if ISP_HW_V20 || ISP_HW_V30
    pConfig->aeHwConfig.ae_meas.rawae2.wnd_num = 2;
    pConfig->aeHwConfig.ae_meas.rawae2.win.h_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae2.win.v_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae2.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.ae_meas.rawae2.win.v_size = pConfig->RawHeight;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[0].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[0].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[0].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[0].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[1].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[1].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[1].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[1].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[2].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[2].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[2].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[2].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[3].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[3].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[3].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin[3].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae2.subwin_en[0] = 1;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin_en[1] = 1;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin_en[2] = 1;
    pConfig->aeHwConfig.ae_meas.rawae2.subwin_en[3] = 1;
#endif

    pConfig->aeHwConfig.ae_meas.rawae3.wnd_num = 2;
    pConfig->aeHwConfig.ae_meas.rawae3.win.h_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae3.win.v_offs = 0;
    pConfig->aeHwConfig.ae_meas.rawae3.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.ae_meas.rawae3.win.v_size = pConfig->RawHeight;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[0].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[0].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[0].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[0].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[1].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[1].v_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[1].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[1].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[2].h_offs = 2;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[2].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[2].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[2].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[3].h_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[3].v_offs = 150;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[3].h_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin[3].v_size = 100;  // must even number
    pConfig->aeHwConfig.ae_meas.rawae3.subwin_en[0] = 1;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin_en[1] = 1;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin_en[2] = 1;
    pConfig->aeHwConfig.ae_meas.rawae3.subwin_en[3] = 1;

#if defined(ISP_HW_V39)
    /****rawhist0, BIG mode****/
    pConfig->aeHwConfig.hist_meas.rawhist0.data_sel = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.waterline = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.mode = 5;
    pConfig->aeHwConfig.hist_meas.rawhist0.wnd_num = 2;
    pConfig->aeHwConfig.hist_meas.rawhist0.stepsize = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.h_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.v_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.v_size = pConfig->RawHeight;
    memset(pConfig->aeHwConfig.hist_meas.rawhist0.weight, 0x20, RAWHISTBIG_WIN_NUM * sizeof(unsigned char));
    pConfig->aeHwConfig.hist_meas.rawhist0.rcc = 0x4d;
    pConfig->aeHwConfig.hist_meas.rawhist0.gcc = 0x4b;
    pConfig->aeHwConfig.hist_meas.rawhist0.bcc = 0x1d;
    pConfig->aeHwConfig.hist_meas.rawhist0.off = 0x00;
#else
    /****rawhist0, LITE mode****/
    pConfig->aeHwConfig.hist_meas.rawhist0.data_sel = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.waterline = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.mode = 5;
    pConfig->aeHwConfig.hist_meas.rawhist0.stepsize = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.h_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.v_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.hist_meas.rawhist0.win.v_size = pConfig->RawHeight;
    memset(pConfig->aeHwConfig.hist_meas.rawhist0.weight, 0x20, RAWHISTBIG_WIN_NUM * sizeof(unsigned char));
    pConfig->aeHwConfig.hist_meas.rawhist0.rcc = 0x4d;
    pConfig->aeHwConfig.hist_meas.rawhist0.gcc = 0x4b;
    pConfig->aeHwConfig.hist_meas.rawhist0.bcc = 0x1d;
    pConfig->aeHwConfig.hist_meas.rawhist0.off = 0x00;
#endif

#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
    /****rawhist1-3, BIG mode****/
    pConfig->aeHwConfig.hist_meas.rawhist1.data_sel = 0;
    pConfig->aeHwConfig.hist_meas.rawhist1.waterline = 0;
    pConfig->aeHwConfig.hist_meas.rawhist1.mode = 5;
    pConfig->aeHwConfig.hist_meas.rawhist1.wnd_num = 2;
    pConfig->aeHwConfig.hist_meas.rawhist1.stepsize = 0;
    pConfig->aeHwConfig.hist_meas.rawhist1.win.h_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist1.win.v_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist1.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.hist_meas.rawhist1.win.v_size = pConfig->RawHeight;
    memset(pConfig->aeHwConfig.hist_meas.rawhist1.weight, 0x20, RAWHISTBIG_WIN_NUM * sizeof(unsigned char));
    pConfig->aeHwConfig.hist_meas.rawhist1.rcc = 0x4d;
    pConfig->aeHwConfig.hist_meas.rawhist1.gcc = 0x4b;
    pConfig->aeHwConfig.hist_meas.rawhist1.bcc = 0x1d;
    pConfig->aeHwConfig.hist_meas.rawhist1.off = 0x00;
#endif

#if ISP_HW_V20 || ISP_HW_V30
    pConfig->aeHwConfig.hist_meas.rawhist2.data_sel = 0;
    pConfig->aeHwConfig.hist_meas.rawhist2.waterline = 0;
    pConfig->aeHwConfig.hist_meas.rawhist2.mode = 5;
    pConfig->aeHwConfig.hist_meas.rawhist2.wnd_num = 2;
    pConfig->aeHwConfig.hist_meas.rawhist2.stepsize = 0;
    pConfig->aeHwConfig.hist_meas.rawhist2.win.h_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist2.win.v_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist2.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.hist_meas.rawhist2.win.v_size = pConfig->RawHeight;
    memset(pConfig->aeHwConfig.hist_meas.rawhist2.weight, 0x20, RAWHISTBIG_WIN_NUM * sizeof(unsigned char));
    pConfig->aeHwConfig.hist_meas.rawhist2.rcc = 0x4d;
    pConfig->aeHwConfig.hist_meas.rawhist2.gcc = 0x4b;
    pConfig->aeHwConfig.hist_meas.rawhist2.bcc = 0x1d;
    pConfig->aeHwConfig.hist_meas.rawhist2.off = 0x00;
#endif

    pConfig->aeHwConfig.hist_meas.rawhist3.data_sel = 0;
    pConfig->aeHwConfig.hist_meas.rawhist3.waterline = 0;
    pConfig->aeHwConfig.hist_meas.rawhist3.mode = 5;
    pConfig->aeHwConfig.hist_meas.rawhist3.wnd_num = 2;
    pConfig->aeHwConfig.hist_meas.rawhist3.stepsize = 0;
    pConfig->aeHwConfig.hist_meas.rawhist3.win.h_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist3.win.v_offs = 0;
    pConfig->aeHwConfig.hist_meas.rawhist3.win.h_size = pConfig->RawWidth;
    pConfig->aeHwConfig.hist_meas.rawhist3.win.v_size = pConfig->RawHeight;
    memset(pConfig->aeHwConfig.hist_meas.rawhist3.weight, 0x20, RAWHISTBIG_WIN_NUM * sizeof(unsigned char));
    pConfig->aeHwConfig.hist_meas.rawhist3.rcc = 0x4d;
    pConfig->aeHwConfig.hist_meas.rawhist3.gcc = 0x4b;
    pConfig->aeHwConfig.hist_meas.rawhist3.bcc = 0x1d;
    pConfig->aeHwConfig.hist_meas.rawhist3.off = 0x00;

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn updateAecHwConfig(RkAiqAlgoProcResAe* rkAeProcRes, rk_aiq_rkAe_config_t* rkAe)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    *rkAeProcRes->ae_meas = rkAe->aeHwConfig.ae_meas;
    *rkAeProcRes->hist_meas = rkAe->aeHwConfig.hist_meas;
    rkAeProcRes->ae_meas->ae_meas_update = true;
    rkAeProcRes->hist_meas->hist_meas_update = true;

    return ret;
}
static XCamReturn updateGrpAecHwConfig(rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result, int camnum, rk_aiq_rkAe_config_t* rkAe)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for(int i = 0; i < camnum; i++) {
        memcpy(rk_aiq_singlecam_3a_result[i]->aec._aeMeasParams, &rkAe->aeHwConfig.ae_meas, sizeof(rk_aiq_ae_meas_params_t));
        memcpy(rk_aiq_singlecam_3a_result[i]->aec._aeHistMeasParams, &rkAe->aeHwConfig.hist_meas, sizeof(rk_aiq_hist_meas_params_t));
        rk_aiq_singlecam_3a_result[i]->aec._aeMeasParams->ae_meas_update = true;
        rk_aiq_singlecam_3a_result[i]->aec._aeHistMeasParams->hist_meas_update = true;
    }

    return ret;
}

// call after initAecHwConfig
static void initCustomAeRes(rk_aiq_customeAe_results_t* customAe, rk_aiq_rkAe_config_t* pConfig)
{
    // 0.) exposure params
    if(pConfig->IsHdr) {
        //hdr_exp 0:sframe 1:mframe 2:lframe, set exp according to hdrframe
        customAe->hdr_exp[0].exp_real_params.integration_time = 0.003f;
        customAe->hdr_exp[0].exp_real_params.analog_gain = 1.0f;
        customAe->hdr_exp[0].exp_real_params.digital_gain = 1.0f;
        customAe->hdr_exp[0].exp_real_params.isp_dgain = 1.0f;
        customAe->hdr_exp[0].exp_real_params.iso = customAe->hdr_exp[0].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        customAe->hdr_exp[1].exp_real_params.integration_time = 0.01f;
        customAe->hdr_exp[1].exp_real_params.analog_gain = 1.0f;
        customAe->hdr_exp[1].exp_real_params.digital_gain = 1.0f;
        customAe->hdr_exp[1].exp_real_params.isp_dgain = 1.0f;
        customAe->hdr_exp[1].exp_real_params.iso = customAe->hdr_exp[1].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        customAe->hdr_exp[2].exp_real_params.integration_time = 0.02f;
        customAe->hdr_exp[2].exp_real_params.analog_gain = 1.0f;
        customAe->hdr_exp[2].exp_real_params.digital_gain = 1.0f;
        customAe->hdr_exp[2].exp_real_params.isp_dgain = 1.0f;
        customAe->hdr_exp[2].exp_real_params.iso = customAe->hdr_exp[2].exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        AeHdrDcgConv(pConfig, customAe->hdr_exp);

        for(int i = 0; i < pConfig->HdrFrmNum; i++) {

            AeReal2RegConv(pConfig, customAe->hdr_exp[i].exp_real_params.integration_time,
                           customAe->hdr_exp[i].exp_real_params.analog_gain,
                           &customAe->hdr_exp[i].exp_sensor_params.coarse_integration_time,
                           &customAe->hdr_exp[i].exp_sensor_params.analog_gain_code_global,
                           customAe->hdr_exp[i].exp_real_params.dcg_mode);

            customAe->exp_i2c_params.bValid = false;
        }
    } else {
        //linear_exp
        customAe->linear_exp.exp_real_params.integration_time = 0.003f;
        customAe->linear_exp.exp_real_params.analog_gain = 1.0f;
        customAe->linear_exp.exp_real_params.digital_gain = 1.0f;
        customAe->linear_exp.exp_real_params.isp_dgain = 1.0f;
        customAe->linear_exp.exp_real_params.iso = customAe->linear_exp.exp_real_params.analog_gain * 50; //RK: ISO = Gain*50

        AeDcgConv(pConfig, customAe->linear_exp.exp_real_params.analog_gain, &customAe->linear_exp.exp_real_params.dcg_mode);

        AeReal2RegConv(pConfig, customAe->linear_exp.exp_real_params.integration_time,
                       customAe->linear_exp.exp_real_params.analog_gain,
                       &customAe->linear_exp.exp_sensor_params.coarse_integration_time,
                       &customAe->linear_exp.exp_sensor_params.analog_gain_code_global,
                       customAe->linear_exp.exp_real_params.dcg_mode);

        customAe->exp_i2c_params.bValid = false;
    }

    customAe->frame_length_lines = pConfig->PixelPeriodsPerLine;
    customAe->is_longfrm_mode = false;

    //1.) hw params
    customAe->meas_win = pConfig->aeHwConfig.ae_meas.rawae3.win;
    memcpy(&customAe->meas_weight, &pConfig->aeHwConfig.hist_meas.rawhist3.weight, sizeof(customAe->meas_weight));

}

static XCamReturn AeDemoPrepare(RkAiqAlgoCom* params)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoContext* algo_ctx = params->ctx;

    if (!algo_ctx->cutomAeInit) {
        algo_ctx->cbs.pfn_ae_init(algo_ctx->aiq_ctx);
        algo_ctx->cutomAeInit = true;
    }

    // 0.) working mode (normal / hdr 2 / hdr 3)
    algo_ctx->rkCfg.Working_mode = params->u.prepare.working_mode;

    if(algo_ctx->rkCfg.Working_mode <= RK_AIQ_WORKING_MODE_NORMAL) {
        algo_ctx->rkCfg.IsHdr = false;
    } else {
        algo_ctx->rkCfg.IsHdr = true;
        if(algo_ctx->rkCfg.Working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)
            algo_ctx->rkCfg.HdrFrmNum = 2;
        else
            algo_ctx->rkCfg.HdrFrmNum = 3;
    }

    // 1.) resolution
    algo_ctx->rkCfg.RawWidth = params->u.prepare.sns_op_width;
    algo_ctx->rkCfg.RawHeight = params->u.prepare.sns_op_height;

    if(algo_ctx->isGroupMode) {

        RkAiqAlgoCamGroupPrepare* AeCfgParam = (RkAiqAlgoCamGroupPrepare*)params;

        //read info from sensor (hts vts pclk)
        algo_ctx->rkCfg.LinePeriodsPerField = AeCfgParam->aec.LinePeriodsPerField;
        algo_ctx->rkCfg.PixelClockFreqMHZ = AeCfgParam->aec.PixelClockFreqMHZ;
        algo_ctx->rkCfg.PixelPeriodsPerLine = AeCfgParam->aec.PixelPeriodsPerLine;
        algo_ctx->rkCfg.stNRswitch = AeCfgParam->aec.nr_switch;

    } else {

        RkAiqAlgoConfigAe* AeCfgParam = (RkAiqAlgoConfigAe*)params;

        //read info from sensor (hts vts pclk)
        algo_ctx->rkCfg.LinePeriodsPerField = AeCfgParam->LinePeriodsPerField;
        algo_ctx->rkCfg.PixelClockFreqMHZ = AeCfgParam->PixelClockFreqMHZ;
        algo_ctx->rkCfg.PixelPeriodsPerLine = AeCfgParam->PixelPeriodsPerLine;
        algo_ctx->rkCfg.stNRswitch = AeCfgParam->nr_switch;

    }

    algo_ctx->rkCfg.init_fps = algo_ctx->rkCfg.PixelClockFreqMHZ * 1000000
                               / (algo_ctx->rkCfg.LinePeriodsPerField * algo_ctx->rkCfg.PixelPeriodsPerLine);
    algo_ctx->rkCfg.last_fps = algo_ctx->rkCfg.init_fps;

    if (algo_ctx->rkCfg.fps_request == -1) {
        algo_ctx->rkCfg.last_vts = algo_ctx->rkCfg.LinePeriodsPerField;
        algo_ctx->rkCfg.vts_request = algo_ctx->rkCfg.last_vts;
    }

    // 3.) set initial hw config & initial ae result, initial value can be modified by user in processing

    if(!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        //initial hw
        initAecHwConfig(&algo_ctx->rkCfg); //for group ae, initial value can be the same

        //initial ae result (exposure, hw res)
        initCustomAeRes(&algo_ctx->customRes, &algo_ctx->rkCfg); //for group, only init full customRes

        algo_ctx->updateCalib = false;
    } else {
        //update calib
        algo_ctx->updateCalib = true;
    }

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

static
void _rkAeStats2CustomAeStats(rk_aiq_rkAe_config_t * pConfig,
                              rk_aiq_customAe_stats_t* customAe,
                              RKAiqAecStats_t* rkAe)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);

    // 1.0) ae hw stats
    customAe->rawae_stat[0] = rkAe->ae_data.chn[0];
    customAe->rawae_stat[1] = rkAe->ae_data.chn[1];
    customAe->rawae_stat[2] = rkAe->ae_data.chn[2];

    customAe->extra.rawae_big = rkAe->ae_data.extra.rawae_big;
    customAe->extra.rawhist_big = rkAe->ae_data.extra.rawhist_big;

    // 2.0) ae exposure stats
    if(pConfig->IsHdr) {
        customAe->hdr_exp[0] = rkAe->ae_exp.HdrExp[0];
        customAe->hdr_exp[1] = rkAe->ae_exp.HdrExp[1];
        customAe->hdr_exp[2] = rkAe->ae_exp.HdrExp[2];
    } else {
        customAe->linear_exp = rkAe->ae_exp.LinearExp;
    }


    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
}

static
XCamReturn _rkGrpAeStats2CustomGrpAeStats(rk_aiq_rkAe_config_t * pConfig,
        int camnum,
        rk_aiq_customAe_stats_t* customAe,
        rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result)
{
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCamVideoBuffer* aecStatsBuf = nullptr;
    RkAiqAecStats* xAecStats = nullptr;

    rk_aiq_customAe_stats_t* customAeStats = customAe;
    rk_aiq_customAe_stats_t* next_customAeStats = nullptr;

    for(int i = 0; i < camnum; i++) {

        aecStatsBuf = rk_aiq_singlecam_3a_result[i]->aec._aecStats;
        if (aecStatsBuf) {
            xAecStats = (RkAiqAecStats*)aecStatsBuf->map(aecStatsBuf);
            if (!xAecStats) {
                LOGE_GAEC("aec stats is null for %dth camera", i);
                return(XCAM_RETURN_ERROR_FAILED);
            }
        } else {
            LOGE_GAEC("aec stats is null for %dth camera", i);
            return(XCAM_RETURN_ERROR_FAILED);
        }

        if(i > 0) {
            if(customAeStats->next == nullptr)
                customAeStats->next = (rk_aiq_customAe_stats_t*)calloc(1, sizeof(rk_aiq_customAe_stats_t));

            next_customAeStats = customAeStats->next;
            customAeStats = next_customAeStats;
        }

        _rkAeStats2CustomAeStats(pConfig, customAeStats, &xAecStats->aec_stats);

    }

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);

    return (ret);
}

static void _customGrpAeStatsRelease(rk_aiq_customAe_stats_t *customAe)
{
    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    rk_aiq_customAe_stats_t* customAeStats = customAe->next;
    rk_aiq_customAe_stats_t* next_customAeStats = nullptr;

    while(customAeStats != nullptr) {

        next_customAeStats = customAeStats->next;
        free(customAeStats);
        customAeStats = next_customAeStats;
    }

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
}

static
void _customAeRes2rkAeRes(rk_aiq_rkAe_config_t* pConfig, RkAiqAlgoProcResAe* rkAeProcRes,
                          rk_aiq_customeAe_results_t* customAeProcRes)
{

    //AE new sensor exposure
    if(pConfig->IsHdr) {
        //Hdr
        if(!(customAeProcRes->exp_i2c_params.bValid))
            AeHdrDcgConv(pConfig, customAeProcRes->hdr_exp);

        for(int i = 0; i < pConfig->HdrFrmNum; i++) {

            if(!(customAeProcRes->exp_i2c_params.bValid)) {
                AeReal2RegConv(pConfig, customAeProcRes->hdr_exp[i].exp_real_params.integration_time,
                               customAeProcRes->hdr_exp[i].exp_real_params.analog_gain,
                               &customAeProcRes->hdr_exp[i].exp_sensor_params.coarse_integration_time,
                               &customAeProcRes->hdr_exp[i].exp_sensor_params.analog_gain_code_global,
                               customAeProcRes->hdr_exp[i].exp_real_params.dcg_mode);
            }
            rkAeProcRes->new_ae_exp->HdrExp[i] = customAeProcRes->hdr_exp[i];
        }

    } else {
        //Linear
        if(!customAeProcRes->exp_i2c_params.bValid) {
            AeDcgConv(pConfig, customAeProcRes->linear_exp.exp_real_params.analog_gain, &customAeProcRes->linear_exp.exp_real_params.dcg_mode);
            AeReal2RegConv(pConfig, customAeProcRes->linear_exp.exp_real_params.integration_time,
                           customAeProcRes->linear_exp.exp_real_params.analog_gain,
                           &customAeProcRes->linear_exp.exp_sensor_params.coarse_integration_time,
                           &customAeProcRes->linear_exp.exp_sensor_params.analog_gain_code_global,
                           customAeProcRes->linear_exp.exp_real_params.dcg_mode);

        }

        rkAeProcRes->new_ae_exp->LinearExp = customAeProcRes->linear_exp;
    }

    rkAeProcRes->exp_i2c_params->bValid = customAeProcRes->exp_i2c_params.bValid;
    rkAeProcRes->exp_i2c_params->nNumRegs = customAeProcRes->exp_i2c_params.nNumRegs;

    if(customAeProcRes->exp_i2c_params.bValid) {
        if(customAeProcRes->exp_i2c_params.nNumRegs <= MAX_I2CDATA_LEN) {
            for(uint32_t i = 0; i < customAeProcRes->exp_i2c_params.nNumRegs; i++) {
                rkAeProcRes->exp_i2c_params->DelayFrames[i] = customAeProcRes->exp_i2c_params.pDelayFrames[i];
                rkAeProcRes->exp_i2c_params->RegAddr[i] = customAeProcRes->exp_i2c_params.pRegAddr[i];
                rkAeProcRes->exp_i2c_params->AddrByteNum[i] = customAeProcRes->exp_i2c_params.pAddrByteNum[i];
                rkAeProcRes->exp_i2c_params->RegValue[i] = customAeProcRes->exp_i2c_params.pRegValue[i];
                rkAeProcRes->exp_i2c_params->ValueByteNum[i] = customAeProcRes->exp_i2c_params.pValueByteNum[i];
            }
        } else {
            LOGE("too many i2c data to set!!");
        }
    }

    rkAeProcRes->new_ae_exp->frame_length_lines = customAeProcRes->frame_length_lines;
    rkAeProcRes->new_ae_exp->Iris = customAeProcRes->Iris;

    rkAeProcRes->ae_proc_res_rk->exp_set_cnt = 1;
    rkAeProcRes->ae_proc_res_rk->exp_set_tbl[0] = *rkAeProcRes->new_ae_exp;
    rkAeProcRes->ae_proc_res_rk->LongFrmMode = customAeProcRes->is_longfrm_mode;

    //RK: CIS feature for NR
    AeCISFeature(pConfig, &rkAeProcRes->ae_proc_res_rk->exp_set_tbl[0]);

    //AE new HW config
    if (customAeProcRes->meas_win.h_size > 0 && customAeProcRes->meas_win.v_size > 0) {
        rkAeProcRes->ae_meas->rawae0.win = customAeProcRes->meas_win;
        rkAeProcRes->hist_meas->rawhist0.win = customAeProcRes->meas_win;

#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
        rkAeProcRes->ae_meas->rawae1.win = customAeProcRes->meas_win;
        rkAeProcRes->hist_meas->rawhist1.win = customAeProcRes->meas_win;
#endif

#if ISP_HW_V20 || ISP_HW_V30
        rkAeProcRes->ae_meas->rawae2.win = customAeProcRes->meas_win;
        rkAeProcRes->hist_meas->rawhist2.win = customAeProcRes->meas_win;
#endif

        rkAeProcRes->ae_meas->rawae3.win = customAeProcRes->meas_win;
        rkAeProcRes->hist_meas->rawhist3.win = customAeProcRes->meas_win;
    }

    AeGridWeight15x15to5x5(customAeProcRes->meas_weight, rkAeProcRes->hist_meas->rawhist0.weight);

#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
    memcpy(rkAeProcRes->hist_meas->rawhist1.weight, customAeProcRes->meas_weight, 15 * 15 * sizeof(unsigned char));
#endif

#if ISP_HW_V20 || ISP_HW_V30
    memcpy(rkAeProcRes->hist_meas->rawhist2.weight, customAeProcRes->meas_weight, 15 * 15 * sizeof(unsigned char));
#endif

    memcpy(rkAeProcRes->hist_meas->rawhist3.weight, customAeProcRes->meas_weight, 15 * 15 * sizeof(unsigned char));

}

static
void _customGrpAeSingleResSet(rk_aiq_rkAe_config_t* pConfig, rk_aiq_singlecam_3a_result_t* rk_aiq_singlecam_3a_result,
                              rk_aiq_customeAe_results_single_t customAeRes)
{
    // 0.) copy exposure params
    *rk_aiq_singlecam_3a_result->aec.exp_tbl_size = 1;

    if(pConfig->IsHdr) {
        //Hdr

        if(!customAeRes.exp_i2c_params.bValid)
            AeHdrDcgConv(pConfig, customAeRes.hdr_exp);

        for(int i = 0; i < pConfig->HdrFrmNum; i++) {

            if(!customAeRes.exp_i2c_params.bValid) {
                AeReal2RegConv(pConfig, customAeRes.hdr_exp[i].exp_real_params.integration_time,
                               customAeRes.hdr_exp[i].exp_real_params.analog_gain,
                               &customAeRes.hdr_exp[i].exp_sensor_params.coarse_integration_time,
                               &customAeRes.hdr_exp[i].exp_sensor_params.analog_gain_code_global,
                               customAeRes.hdr_exp[i].exp_real_params.dcg_mode);
            }

            rk_aiq_singlecam_3a_result->aec.exp_tbl[0].HdrExp[i] = customAeRes.hdr_exp[i];
        }

    } else {
        //Linear

        if(!customAeRes.exp_i2c_params.bValid) {
            AeDcgConv(pConfig, customAeRes.linear_exp.exp_real_params.analog_gain, &customAeRes.linear_exp.exp_real_params.dcg_mode);
            AeReal2RegConv(pConfig, customAeRes.linear_exp.exp_real_params.integration_time,
                           customAeRes.linear_exp.exp_real_params.analog_gain,
                           &customAeRes.linear_exp.exp_sensor_params.coarse_integration_time,
                           &customAeRes.linear_exp.exp_sensor_params.analog_gain_code_global,
                           customAeRes.linear_exp.exp_real_params.dcg_mode);
        }

        rk_aiq_singlecam_3a_result->aec.exp_tbl[0].LinearExp = customAeRes.linear_exp;
    }

    rk_aiq_singlecam_3a_result->aec.exp_i2c_params->bValid = customAeRes.exp_i2c_params.bValid;
    rk_aiq_singlecam_3a_result->aec.exp_i2c_params->nNumRegs = customAeRes.exp_i2c_params.nNumRegs;

    if(customAeRes.exp_i2c_params.bValid) {
        if(customAeRes.exp_i2c_params.nNumRegs <= MAX_I2CDATA_LEN) {

            for(uint32_t i = 0; i < customAeRes.exp_i2c_params.nNumRegs; i++) {
                rk_aiq_singlecam_3a_result->aec.exp_i2c_params->DelayFrames[i] = customAeRes.exp_i2c_params.pDelayFrames[i];
                rk_aiq_singlecam_3a_result->aec.exp_i2c_params->RegAddr[i] = customAeRes.exp_i2c_params.pRegAddr[i];
                rk_aiq_singlecam_3a_result->aec.exp_i2c_params->AddrByteNum[i] = customAeRes.exp_i2c_params.pAddrByteNum[i];
                rk_aiq_singlecam_3a_result->aec.exp_i2c_params->RegValue[i] = customAeRes.exp_i2c_params.pRegValue[i];
                rk_aiq_singlecam_3a_result->aec.exp_i2c_params->ValueByteNum[i] = customAeRes.exp_i2c_params.pValueByteNum[i];
            }
        } else {
            LOGE("too many i2c data to set!!");
        }
    }

    // 1.) copy hw params

    //RK: CIS feature for NR
    AeCISFeature(pConfig, rk_aiq_singlecam_3a_result->aec.exp_tbl);

    //AE new HW config
    if (customAeRes.meas_win.h_size > 0 &&
            customAeRes.meas_win.v_size > 0) {
        rk_aiq_singlecam_3a_result->aec._aeMeasParams->rawae0.win = customAeRes.meas_win;
        rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist0.win = customAeRes.meas_win;
#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
        rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist1.win = customAeRes.meas_win;
        rk_aiq_singlecam_3a_result->aec._aeMeasParams->rawae1.win = customAeRes.meas_win;
#endif
#if ISP_HW_V20 || ISP_HW_V30
        rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist2.win = customAeRes.meas_win;
        rk_aiq_singlecam_3a_result->aec._aeMeasParams->rawae2.win = customAeRes.meas_win;
#endif
        rk_aiq_singlecam_3a_result->aec._aeMeasParams->rawae3.win = customAeRes.meas_win;
        rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist3.win = customAeRes.meas_win;
    }

    AeGridWeight15x15to5x5(customAeRes.meas_weight, rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist0.weight);

#if ISP_HW_V20 || ISP_HW_V21 || ISP_HW_V30 || ISP_HW_V32
    memcpy(rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist1.weight, customAeRes.meas_weight, 15 * 15 * sizeof(unsigned char));
#endif

#if ISP_HW_V20 || ISP_HW_V30
    memcpy(rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist2.weight, customAeRes.meas_weight, 15 * 15 * sizeof(unsigned char));
#endif

    memcpy(rk_aiq_singlecam_3a_result->aec._aeHistMeasParams->rawhist3.weight, customAeRes.meas_weight, 15 * 15 * sizeof(unsigned char));

}

static
void _customGrpAeRes2rkGrpAeRes(rk_aiq_rkAe_config_t* pConfig, rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result, int camnum,
                                rk_aiq_customeAe_results_t* customAeProcRes)
{
    rk_aiq_customeAe_results_single_t* customAeRes = customAeProcRes->next;
    rk_aiq_customeAe_results_single_t tmp_customAeRes;

    for(int i = 0; i < camnum; i++) {

        //copy exposure & hw params
        if(i > 0 && customAeRes != nullptr) {
            tmp_customAeRes.hdr_exp[0] = customAeRes->hdr_exp[0];
            tmp_customAeRes.hdr_exp[1] = customAeRes->hdr_exp[1];
            tmp_customAeRes.hdr_exp[2] = customAeRes->hdr_exp[2];
            tmp_customAeRes.linear_exp = customAeRes->linear_exp;
            tmp_customAeRes.exp_i2c_params = customAeRes->exp_i2c_params;
            tmp_customAeRes.meas_win = customAeRes->meas_win;
            memcpy(tmp_customAeRes.meas_weight, customAeRes->meas_weight, 15 * 15 * sizeof(unsigned char));

            customAeRes = customAeRes->next;
        } else {
            tmp_customAeRes.hdr_exp[0] = customAeProcRes->hdr_exp[0];
            tmp_customAeRes.hdr_exp[1] = customAeProcRes->hdr_exp[1];
            tmp_customAeRes.hdr_exp[2] = customAeProcRes->hdr_exp[2];
            tmp_customAeRes.linear_exp = customAeProcRes->linear_exp;
            tmp_customAeRes.exp_i2c_params = customAeProcRes->exp_i2c_params;
            tmp_customAeRes.meas_win = customAeProcRes->meas_win;
            memcpy(tmp_customAeRes.meas_weight, customAeProcRes->meas_weight, 15 * 15 * sizeof(unsigned char));
        }

        _customGrpAeSingleResSet(pConfig, rk_aiq_singlecam_3a_result[i], tmp_customAeRes);

        //copy common result
        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].frame_length_lines =  customAeProcRes->frame_length_lines;
        rk_aiq_singlecam_3a_result[i]->aec.exp_tbl[0].Iris =  customAeProcRes->Iris;

        //copy common RK result
        RkAiqAlgoProcResAeShared_t* aeProcRes = &rk_aiq_singlecam_3a_result[i]->aec._aeProcRes;
        aeProcRes->LongFrmMode = customAeProcRes->is_longfrm_mode;
    }
}

static XCamReturn AeDemoPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAlgoPreAe* AePreParams = (RkAiqAlgoPreAe*)inparams;
    RkAiqAlgoPreResAe* AePreResParams = (RkAiqAlgoPreResAe*)outparams;
    RkAiqAlgoContext* algo_ctx = inparams->ctx;
    AecPreResult_t* AePreResult = &AePreResParams->ae_pre_res_rk;

    if(!inparams->u.proc.init) {
        // get current used ae exp
        RkAiqAecStats *xAecStats = (RkAiqAecStats*)AePreParams->aecStatsBuf->map(AePreParams->aecStatsBuf);
        if (!xAecStats) {
            LOGE_AEC("aec stats is null");
            return(XCAM_RETURN_ERROR_FAILED);
        }

        if(algo_ctx->rkCfg.IsHdr) {

            for(int i = 0; i < algo_ctx->rkCfg.HdrFrmNum; i++) {
                AePreResult->HdrExp[i] = xAecStats->aec_stats.ae_exp.HdrExp[i];
                ret = AeReg2RealConv(&algo_ctx->rkCfg, AePreResult->HdrExp[i].exp_sensor_params.analog_gain_code_global,
                                     AePreResult->HdrExp[i].exp_sensor_params.coarse_integration_time,
                                     AePreResult->HdrExp[i].exp_real_params.dcg_mode,
                                     AePreResult->HdrExp[i].exp_real_params.analog_gain,
                                     AePreResult->HdrExp[i].exp_real_params.integration_time);
            }

        } else {
            AePreResult->LinearExp = xAecStats->aec_stats.ae_exp.LinearExp;
            ret = AeReg2RealConv(&algo_ctx->rkCfg, AePreResult->LinearExp.exp_sensor_params.analog_gain_code_global,
                                 AePreResult->LinearExp.exp_sensor_params.coarse_integration_time,
                                 AePreResult->LinearExp.exp_real_params.dcg_mode,
                                 AePreResult->LinearExp.exp_real_params.analog_gain,
                                 AePreResult->LinearExp.exp_real_params.integration_time);
        }

    } else {
        if (algo_ctx->updateCalib) {
            LOGD_AEC_SUBM(0xff, "updateCalib, no need re-init");
            return ret;
        }

        // use init ae exp
        if(algo_ctx->rkCfg.IsHdr) {
            AePreResult->HdrExp[0].exp_real_params = algo_ctx->customRes.hdr_exp[0];
            AePreResult->HdrExp[1].exp_real_params = algo_ctx->customRes.hdr_exp[1];
            AePreResult->HdrExp[2].exp_real_params = algo_ctx->customRes.hdr_exp[2];
        } else {
            AePreResult->LinearExp.exp_real_params = algo_ctx->customRes.linear_exp;
        }
    }

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
#endif
    return ret;
}

static XCamReturn AeDemoProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{

    LOG1_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAe* AeProcParams = (RkAiqAlgoProcAe*)inparams;
    RkAiqAlgoProcResAe* AeProcResParams = (RkAiqAlgoProcResAe*)outparams;
    RkAiqAlgoContext* algo_ctx = inparams->ctx;

    if (algo_ctx->isGroupMode) {
        LOGE_AEC("wrong aec mode");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        rk_aiq_customAe_stats_t customStats;
        memset(&customStats, 0, sizeof(rk_aiq_customAe_stats_t));

        RKAiqAecStats_t* xAecStats = AeProcParams->aecStatsBuf;
        if (!xAecStats) {
            LOGE_AEC("aec stats is null");
            return(XCAM_RETURN_ERROR_FAILED);
        }

        _rkAeStats2CustomAeStats(&algo_ctx->rkCfg, &customStats, xAecStats);

        if (algo_ctx->cbs.pfn_ae_run)
            algo_ctx->cbs.pfn_ae_run(algo_ctx->aiq_ctx,
                                     &customStats,
                                     &algo_ctx->customRes
                                    );
    } else {
        if (algo_ctx->updateCalib) {
            LOGD_AEC_SUBM(0xff, "updateCalib, no need re-init");
            return ret;
        }

        if (algo_ctx->cbs.pfn_ae_run)
            algo_ctx->cbs.pfn_ae_run(algo_ctx->aiq_ctx,
                                     NULL,
                                     &algo_ctx->customRes
                                    );
    }

    if(!inparams->u.proc.init) {
        // enable all ae stats channel (channelr_xy, channelg_xy, channelb_xy, channely_xy)
        AeProcResParams->stats_cfg_to_trans.isUpdate = true;
        AeProcResParams->stats_cfg_to_trans.UpdateStats = true;
        AeProcResParams->stats_cfg_to_trans.RawStatsChnSel = RAWSTATS_CHN_ALL_EN;
        AeProcResParams->stats_cfg_to_trans.YRangeMode = (int8_t)CAM_YRANGEV2_MODE_FULL;
        AeProcResParams->stats_cfg_to_trans.BigWeight = AeProcResParams->hist_meas->rawhist3.weight;
        AeProcResParams->stats_cfg_to_trans.LiteWeight = AeProcResParams->hist_meas->rawhist0.weight;

    } else {
        AeProcResParams->stats_cfg_to_trans.isUpdate = false;
    }

    // gen patrt of hw results from initAecHwConfig
    updateAecHwConfig(AeProcResParams, &algo_ctx->rkCfg);

    // copy custom result to rk result
    _customAeRes2rkAeRes(&algo_ctx->rkCfg, AeProcResParams, &algo_ctx->customRes);

    if (algo_ctx->updateCalib) {
        algo_ctx->updateCalib = false;
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

    if (!algo_ctx->isGroupMode) {
        LOGD_AEC("wrong aec mode for group");
        return ret;
    }

    if(!inparams->u.proc.init) { // init=ture, stats=null
        rk_aiq_customAe_stats_t customStats;
        memset(&customStats, 0, sizeof(customStats));
        _rkGrpAeStats2CustomGrpAeStats(&algo_ctx->rkCfg, algo_ctx->camIdArrayLen, &customStats, AeProcParams->camgroupParmasArray);

        if (algo_ctx->cbs.pfn_ae_run)
            algo_ctx->cbs.pfn_ae_run(algo_ctx->group_ctx,
                                     &customStats,
                                     &algo_ctx->customRes
                                    );
        _customGrpAeStatsRelease(&customStats);

    } else {
        if (algo_ctx->updateCalib) {
            LOGD_AEC_SUBM(0xff, "updateCalib, no need re-init");
            return ret;
        }

        if (algo_ctx->cbs.pfn_ae_run)
            algo_ctx->cbs.pfn_ae_run(algo_ctx->group_ctx,
                                     NULL,
                                     &algo_ctx->customRes
                                    );
    }

    if(!inparams->u.proc.init) {
        for(int i = 0; i < AeProcResParams->arraySize; i++) {
            // enable all ae stats channel (channelr_xy, channelg_xy, channelb_xy, channely_xy)
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.isUpdate = true;
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.UpdateStats = true;
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.RawStatsChnSel = (RAWSTATS_CHN_RGB_EN | RAWSTATS_CHN_Y_EN);
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.YRangeMode = (int8_t)CAM_YRANGEV2_MODE_FULL;
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.BigWeight =
                AeProcResParams->camgroupParmasArray[i]->aec._aeHistMeasParams->rawhist3.weight;
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.LiteWeight =
                AeProcResParams->camgroupParmasArray[i]->aec._aeHistMeasParams->rawhist0.weight;
        }

    } else {
        for (int i = 0; i < AeProcResParams->arraySize; i++) {
            AeProcResParams->camgroupParmasArray[i]->aec.stats_cfg_to_trans.isUpdate = false;
        }
    }

    // gen patrt of hw results from initAecHwConfig
    updateGrpAecHwConfig(AeProcResParams->camgroupParmasArray, AeProcResParams->arraySize, &algo_ctx->rkCfg);
    // copy custom result to rk result
    _customGrpAeRes2rkGrpAeRes(&algo_ctx->rkCfg, AeProcResParams->camgroupParmasArray, AeProcResParams->arraySize, &algo_ctx->customRes);

    LOG1_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;

}

static XCamReturn AeDemoPostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RESULT ret = RK_AIQ_RET_SUCCESS;

    return XCAM_RETURN_NO_ERROR;
}

static std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*> g_customAe_desc_map;

XCamReturn
rk_aiq_uapi2_customAE_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_customeAe_cbs_t* cbs)
{

    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (!cbs)
        return XCAM_RETURN_ERROR_PARAM;

    const rk_aiq_camgroup_ctx_t* group_ctx = NULL;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGI_AEC_SUBM(0xff, "group ae");
        group_ctx = (const rk_aiq_camgroup_ctx_t*)ctx;

    } else {
        LOGI_AEC_SUBM(0xff, "single ae");
    }

    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAe_desc_map.find(cast_ctx);

    if (it == g_customAe_desc_map.end()) {
        desc = new RkAiqAlgoDescription();
        g_customAe_desc_map[cast_ctx] = desc;
    } else {
        desc = it->second;
    }

    desc->common.version = RKISP_ALGO_AE_DEMO_VERSION;
    desc->common.vendor  = RKISP_ALGO_AE_DEMO_VENDOR;
    desc->common.description = RKISP_ALGO_AE_DEMO_DESCRIPTION;
    desc->common.type    = RK_AIQ_ALGO_TYPE_AE;
    desc->common.id      = 0;
    desc->common.create_context  = AeDemoCreateCtx;
    desc->common.destroy_context = AeDemoDestroyCtx;
    desc->prepare = AeDemoPrepare;
    desc->pre_process = AeDemoPreProcess;
    if (!group_ctx)
        desc->processing = AeDemoProcessing;
    else
        desc->processing = AeDemoGroupProcessing;
    desc->post_process = AeDemoPostProcess;

    ret = rk_aiq_uapi_sysctl_regLib(ctx, &desc->common);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AEC_SUBM(0xff, "register %d failed !", desc->common.id);
        return ret;
    }

    RkAiqAlgoContext* algoCtx =
        rk_aiq_uapi_sysctl_getAxlibCtx(ctx,
                                       desc->common.type,
                                       desc->common.id);
    if (algoCtx == NULL) {
        LOGE_AEC_SUBM(0xff, "can't get custom ae algo %d ctx!", desc->common.id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    algoCtx->cbs = *cbs;
    algoCtx->aiq_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    LOGD_AEC_SUBM(0xff, "register custom ae algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);
    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);

    return ret;
}

XCamReturn
rk_aiq_uapi2_customAE_enable(const rk_aiq_sys_ctx_t* ctx, bool enable)
{

    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAe_desc_map.find(cast_ctx);

    if (it == g_customAe_desc_map.end()) {
        LOGE_AEC_SUBM(0xff, "can't find custom ae algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }

    ret = rk_aiq_uapi_sysctl_enableAxlib(ctx,
                                         desc->common.type,
                                         desc->common.id,
                                         enable);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AEC_SUBM(0xff, "enable custom ae lib id %d failed !");
        return ret;
    }

#if 0
    // now rk and custom ae are running concurrently,
    // because other algos will depend on results of rk ae
    if (enable)
        ret = rk_aiq_uapi_sysctl_enableAxlib(ctx,
                                             desc->common.type,
                                             0,
                                             !enable);
#endif

    LOGD_AEC_SUBM(0xff, "enable custom ae algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);
    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_customAE_unRegister(const rk_aiq_sys_ctx_t* ctx)
{

    LOGD_AEC_SUBM(0xff, "%s ENTER", __func__);
    RkAiqAlgoDescription* desc = NULL;
    rk_aiq_sys_ctx_t* cast_ctx = const_cast<rk_aiq_sys_ctx_t*>(ctx);

    std::map<rk_aiq_sys_ctx_t*, RkAiqAlgoDescription*>::iterator it =
        g_customAe_desc_map.find(cast_ctx);

    if (it == g_customAe_desc_map.end()) {
        LOGE_AEC_SUBM(0xff, "can't find custom ae algo for sys_ctx %p !", ctx);
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        desc = it->second;
    }

    rk_aiq_uapi_sysctl_unRegLib(ctx,
                                desc->common.type,
                                desc->common.id);

    LOGD_AEC_SUBM(0xff, "unregister custom ae algo sucess for sys_ctx %p, lib_id %d !",
                  ctx,
                  desc->common.id);

    delete it->second;
    g_customAe_desc_map.erase(it);

    LOGD_AEC_SUBM(0xff, "%s EXIT", __func__);
    return XCAM_RETURN_NO_ERROR;
}

extern "C" {

    void rk_aiq_uapi2_customAE_setFrameRate(RkAiqAlgoContext* context, float fps)
    {
        if (fps > 0) {
            context->rkCfg.fps_request = fps;
            context->rkCfg.update_fps = true;
        } else {
            LOGE("%s: wrong set fps value: %f\n", __func__, fps);
        }

    }

    float rk_aiq_uapi2_customAE_getFrameRate(RkAiqAlgoContext* context)
    {
        return context->rkCfg.last_fps;
    }

}


RKAIQ_END_DECLARE

/*
 * rk_aiq_algo_agamma_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
#include <string.h>
#include "xcam_common.h"
#include "rk_aiq_agamma_algo.h"
#include "rk_aiq_types_agamma_algo_int.h"
#include "rk_aiq_types_agamma_algo_prvt.h"


RKAIQ_BEGIN_DECLARE

XCamReturn AgammaInit(AgammaHandle_t** pGammaCtx, CamCalibDbV2Context_t* pCalib)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* handle = (AgammaHandle_t*)calloc(sizeof(AgammaHandle_t), 1);
    if (NULL == handle)
        return XCAM_RETURN_ERROR_MEM;

    if(CHECK_ISP_HW_V21()) {
        CalibDbV2_gamma_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, agamma_calib));

        if (!calibv2_agamma_calib) {
            free(handle);
            return XCAM_RETURN_ERROR_MEM;
        }
        memcpy(&handle->CalibDb.Gamma_v20, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_t));

        //default para
        handle->agammaAttr.atrrV21.mode = RK_AIQ_GAMMA_MODE_OFF;
        handle->agammaAttr.atrrV21.stManual.Gamma_en = true;
        handle->agammaAttr.atrrV21.stManual.Gamma_out_segnum = GAMMATYPE_LOG;
        handle->agammaAttr.atrrV21.stManual.Gamma_out_offset = 0;
        float X_isp21[CALIBDB_AGAMMA_KNOTS_NUM] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                    64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                    1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
                                                  };
        float Y_isp21[CALIBDB_AGAMMA_KNOTS_NUM];
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM; i++) {
            Y_isp21[i] = 4095 * pow(X_isp21[i] / 4095, 1 / 2.2 + 0.0);
            Y_isp21[i] = LIMIT_VALUE(Y_isp21[i], 4095, 0);
            handle->agammaAttr.atrrV21.stManual.Gamma_curve[i] = (int)(Y_isp21[i] + 0.5);
        }
        handle->agammaAttr.atrrV21.stFast.en = true;
        handle->agammaAttr.atrrV21.stFast.GammaCoef = 2.2;
        handle->agammaAttr.atrrV21.stFast.SlopeAtZero = 0.0;
    }
    else if(CHECK_ISP_HW_V30()) {
        CalibDbV2_gamma_V30_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_V30_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, agamma_calib));

        if (!calibv2_agamma_calib) {
            free(handle);
            return XCAM_RETURN_ERROR_MEM;
        }
        memcpy(&handle->CalibDb.Gamma_v30, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_V30_t));

        //default para
        handle->agammaAttr.atrrV30.mode = RK_AIQ_GAMMA_MODE_OFF;
        handle->agammaAttr.atrrV30.stManual.Gamma_en = true;
        handle->agammaAttr.atrrV30.stManual.Gamma_out_offset = 0;
        float X_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                        64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                        1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
                                                      };
        float Y_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30];
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
            Y_isp30[i] = 4095 * pow(X_isp30[i] / 4095, 1 / 2.2 + 0.0);
            Y_isp30[i] = LIMIT_VALUE(Y_isp30[i], 4095, 0);
            handle->agammaAttr.atrrV30.stManual.Gamma_curve[i] = (int)(Y_isp30[i] + 0.5);
        }
        handle->agammaAttr.atrrV30.stFast.en = true;
        handle->agammaAttr.atrrV30.stFast.GammaCoef = 2.2;
        handle->agammaAttr.atrrV30.stFast.SlopeAtZero = 0.0;
    }

    *pGammaCtx = handle;
    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);
}

XCamReturn AgammaRelease(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pGammaCtx)
        free(pGammaCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);
}

XCamReturn AgammaPreProc(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);
}

void AgammaAutoProc(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    if(CHECK_ISP_HW_V21()) {
        pGammaCtx->agamma_config.gamma_out_segnum = pGammaCtx->CalibDb.Gamma_v20.GammaTuningPara.Gamma_out_segnum;
        pGammaCtx->agamma_config.gamma_out_offset = pGammaCtx->CalibDb.Gamma_v20.GammaTuningPara.Gamma_out_offset;

        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM; i++) {
            int tmp = pGammaCtx->CalibDb.Gamma_v20.GammaTuningPara.Gamma_curve[i];
            pGammaCtx->agamma_config.gamma_table[i] = tmp;
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        pGammaCtx->agamma_config.gamma_out_segnum = ISP3X_SEGNUM_LOG_49;
        pGammaCtx->agamma_config.gamma_out_offset = pGammaCtx->CalibDb.Gamma_v30.GammaTuningPara.Gamma_out_offset;

        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
            int tmp = pGammaCtx->CalibDb.Gamma_v30.GammaTuningPara.Gamma_curve[i];
            pGammaCtx->agamma_config.gamma_table[i] = tmp;
        }
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaApiManualProc(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    LOGD_AGAMMA(" %s: Agamma api manual !!!\n", __func__);

    if(CHECK_ISP_HW_V21()) {
        pGammaCtx->agamma_config.gamma_en = pGammaCtx->agammaAttr.atrrV21.stManual.Gamma_en;
        pGammaCtx->agamma_config.gamma_out_segnum = pGammaCtx->agammaAttr.atrrV21.stManual.Gamma_out_segnum;
        pGammaCtx->agamma_config.gamma_out_offset = pGammaCtx->agammaAttr.atrrV21.stManual.Gamma_out_offset;
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM; i++) {
            int tmp = pGammaCtx->agammaAttr.atrrV21.stManual.Gamma_curve[i];
            pGammaCtx->agamma_config.gamma_table[i] = tmp;
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        pGammaCtx->agamma_config.gamma_en = pGammaCtx->agammaAttr.atrrV30.stManual.Gamma_en;
        pGammaCtx->agamma_config.gamma_out_segnum = ISP3X_SEGNUM_LOG_49;
        pGammaCtx->agamma_config.gamma_out_offset = pGammaCtx->agammaAttr.atrrV30.stManual.Gamma_out_offset;

        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
            int tmp = pGammaCtx->agammaAttr.atrrV30.stManual.Gamma_curve[i];
            pGammaCtx->agamma_config.gamma_table[i] = tmp;
        }
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaApiFast(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    LOGD_AGAMMA(" %s: Agamma api Fast !!!\n", __func__);

    float X_isp21[CALIBDB_AGAMMA_KNOTS_NUM] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
                                              };
    float Y_isp21[CALIBDB_AGAMMA_KNOTS_NUM];
    float X_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                                                    64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                                                    1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095
                                                  };
    float Y_isp30[CALIBDB_AGAMMA_KNOTS_NUM_V30];
    float coef1 = 0.0;
    float coef2 = 0.0;

    if(CHECK_ISP_HW_V21()) {
        coef1 = pGammaCtx->agammaAttr.atrrV21.stFast.GammaCoef;
        coef2 = pGammaCtx->agammaAttr.atrrV21.stFast.SlopeAtZero;
        coef2 = LIMIT_VALUE(coef2, 0.05, -0.05);
        pGammaCtx->agamma_config.gamma_en = pGammaCtx->agammaAttr.atrrV21.stFast.en ;
        pGammaCtx->agamma_config.gamma_out_segnum = ISP3X_SEGNUM_LOG_45;
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM; i++) {
            Y_isp21[i] = 4095 * pow(X_isp21[i] / 4095, 1 / coef1 + coef2) + pGammaCtx->agamma_config.gamma_out_offset;
            Y_isp21[i] = LIMIT_VALUE(Y_isp21[i], 4095, 0);
            pGammaCtx->agamma_config.gamma_table[i] = (int)(Y_isp21[i] + 0.5);
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        coef1 = pGammaCtx->agammaAttr.atrrV30.stFast.GammaCoef;
        coef2 = pGammaCtx->agammaAttr.atrrV30.stFast.SlopeAtZero;
        coef2 = LIMIT_VALUE(coef2, 0.05, -0.05);
        pGammaCtx->agamma_config.gamma_en = pGammaCtx->agammaAttr.atrrV30.stFast.en ;
        pGammaCtx->agamma_config.gamma_out_segnum = ISP3X_SEGNUM_LOG_49;
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++) {
            Y_isp30[i] = 4095 * pow(X_isp30[i] / 4095, 1 / coef1 + coef2) + pGammaCtx->agamma_config.gamma_out_offset;
            Y_isp30[i] = LIMIT_VALUE(Y_isp30[i], 4095, 0);
            pGammaCtx->agamma_config.gamma_table[i] = (int)(Y_isp30[i] + 0.5);
        }
    }
    pGammaCtx->agamma_config.gamma_out_offset = 0;

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaProcessing(AgammaHandle_t* pGammaCtx)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    rk_aiq_gamma_op_mode_t ApiMode = RK_AIQ_GAMMA_MODE_OFF;
    if(CHECK_ISP_HW_V21())
        ApiMode = pGammaCtx->agammaAttr.atrrV21.mode;
    else if(CHECK_ISP_HW_V30())
        ApiMode = pGammaCtx->agammaAttr.atrrV30.mode;

    if(ApiMode == RK_AIQ_GAMMA_MODE_OFF) { //run iq gamma
        LOGD_AGAMMA(" %s: Agamma api off !!!\n", __func__);
        pGammaCtx->agamma_config.gamma_en = pGammaCtx->CalibDb.Gamma_v20.GammaTuningPara.Gamma_en;
        AgammaAutoProc(pGammaCtx);
    }
    else if(ApiMode == RK_AIQ_GAMMA_MODE_MANUAL)//run manual gamma, for client api
        AgammaApiManualProc( pGammaCtx);
    else if(ApiMode == RK_AIQ_GAMMA_MODE_FAST)//run fast gamma
        AgammaApiFast( pGammaCtx);
    else
        LOGE_AGAMMA(" %s: Wrong gamma mode !!!\n", __func__);

    LOGD_AGAMMA(" %s: gamma_en:%d gamma_out_segnum:%d gamma_out_offset:%d\n", __func__, pGammaCtx->agamma_config.gamma_en, pGammaCtx->agamma_config.gamma_out_segnum
                , pGammaCtx->agamma_config.gamma_out_offset);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaSetProcRes(AgammaProcRes_t* AgammaProcRes, rk_aiq_gamma_cfg_t* agamma_config)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    if(CHECK_ISP_HW_V21()) {
        AgammaProcRes->Gamma_v20.gamma_en = agamma_config->gamma_en;
        AgammaProcRes->Gamma_v20.equ_segm = agamma_config->gamma_out_segnum;
        AgammaProcRes->Gamma_v20.offset = agamma_config->gamma_out_offset;
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM; i++)
            AgammaProcRes->Gamma_v20.gamma_y[i] = agamma_config->gamma_table[i];
    }
    else if(CHECK_ISP_HW_V30()) {
        AgammaProcRes->Gamma_v30.gamma_en = agamma_config->gamma_en;
        AgammaProcRes->Gamma_v30.EnableDot49 = agamma_config->gamma_out_segnum == ISP3X_SEGNUM_LOG_49 ? true : false;
        AgammaProcRes->Gamma_v30.equ_segm = agamma_config->gamma_out_segnum > 1 ? ISP3X_SEGNUM_LOG_45 : agamma_config->gamma_out_segnum;
        AgammaProcRes->Gamma_v30.offset = agamma_config->gamma_out_offset;
        for(int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V30; i++)
            AgammaProcRes->Gamma_v30.gamma_y[i] = agamma_config->gamma_table[i];
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

RKAIQ_END_DECLARE

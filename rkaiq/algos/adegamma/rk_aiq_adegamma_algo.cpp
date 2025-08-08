/*
 * rk_aiq_algo_acprc_itf.c
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
#include "rk_aiq_adegamma_algo.h"

RKAIQ_BEGIN_DECLARE

XCamReturn AdegammaInit(AdegammaHandle_t** para, CamCalibDbV2Context_t* calib)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdegammaHandle_t* handle = (AdegammaHandle_t*)calloc(sizeof(AdegammaHandle_t), 1);
    if (NULL == handle)
        return XCAM_RETURN_ERROR_MEM;

    CalibDbV2_Adegmma_t* calibv2_adegamma_calib =
        (CalibDbV2_Adegmma_t*)(CALIBDBV2_GET_MODULE_PTR(calib, adegamma_calib));
    if (!calibv2_adegamma_calib) {
        free(handle);
        return XCAM_RETURN_ERROR_MEM;
    }

    memcpy(&handle->adegammaAttr.stTool, calibv2_adegamma_calib, sizeof(CalibDbV2_Adegmma_t));
    memcpy(&handle->adegammaAttr.stManual, calibv2_adegamma_calib, sizeof(CalibDbV2_Adegmma_t));
    handle->pCalibDb = calibv2_adegamma_calib; //get adegmma paras from iq
    *para = handle;
    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

XCamReturn AdegammaRelease(AdegammaHandle_t* para)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (para)
        free(para);
    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

XCamReturn AdegammaPreProc(AdegammaHandle_t* para)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

void AdegammaAutoProc(CalibDbV2_Adegmma_t* pAtuoPara, AdegammaProcRes_t* AdegammaProcRes)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    AdegammaProcRes->degamma_en = pAtuoPara->DegammaTuningPara.degamma_en;

    //get X_axis
    int tmp[DEGAMMA_CRUVE_X_KNOTS];
    for(int i = 0; i < DEGAMMA_CRUVE_X_KNOTS; i++) {
        tmp[i] = pAtuoPara->DegammaTuningPara.X_axis[i + 1] - pAtuoPara->DegammaTuningPara.X_axis[i];
        tmp[i] = log(tmp[i]) / log(2) - DEGAMMA_CRUVE_X_NORMALIZE_FACTOR;
        tmp[i] = DEGAMMA_LIMIT_VALUE(tmp[i], DEGAMMA_CRUVE_X_MAX, DEGAMMA_CRUVE_X_MIN);
    }

    int tmp0 = 0, tmp1 = 0;
    for(int i = 0; i < 8; i++) {
        tmp0 |= ((tmp[i]) << (4 * i));
        tmp1 |= ((tmp[i + 8]) << (4 * i));
    }
    AdegammaProcRes->degamma_X_d0 = tmp0;
    AdegammaProcRes->degamma_X_d1 = tmp1;

    for(int i = 0; i < DEGAMMA_CRUVE_Y_KNOTS; i++) {
        AdegammaProcRes->degamma_tableR[i] =
            DEGAMMA_LIMIT_VALUE(pAtuoPara->DegammaTuningPara.curve_R[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
        AdegammaProcRes->degamma_tableG[i] =
            DEGAMMA_LIMIT_VALUE(pAtuoPara->DegammaTuningPara.curve_G[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
        AdegammaProcRes->degamma_tableB[i] =
            DEGAMMA_LIMIT_VALUE(pAtuoPara->DegammaTuningPara.curve_B[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
    }

    LOGD_ADEGAMMA("%s X_axis:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __func__, pAtuoPara->DegammaTuningPara.X_axis[0], pAtuoPara->DegammaTuningPara.X_axis[1],
                  pAtuoPara->DegammaTuningPara.X_axis[2], pAtuoPara->DegammaTuningPara.X_axis[3], pAtuoPara->DegammaTuningPara.X_axis[4], pAtuoPara->DegammaTuningPara.X_axis[5],
                  pAtuoPara->DegammaTuningPara.X_axis[6], pAtuoPara->DegammaTuningPara.X_axis[7], pAtuoPara->DegammaTuningPara.X_axis[8], pAtuoPara->DegammaTuningPara.X_axis[9],
                  pAtuoPara->DegammaTuningPara.X_axis[10], pAtuoPara->DegammaTuningPara.X_axis[11], pAtuoPara->DegammaTuningPara.X_axis[12], pAtuoPara->DegammaTuningPara.X_axis[13],
                  pAtuoPara->DegammaTuningPara.X_axis[14], pAtuoPara->DegammaTuningPara.X_axis[15], pAtuoPara->DegammaTuningPara.X_axis[16]);

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
}

void AdegammaApiManualProc(AdegammaHandle_t* para, AdegammaProcRes_t* AdegammaProcRes)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    AdegammaProcRes->degamma_en = para->adegammaAttr.stManual.en ;

    //get X_axis
    int tmp[DEGAMMA_CRUVE_X_KNOTS];
    for(int i = 0; i < DEGAMMA_CRUVE_X_KNOTS; i++) {
        tmp[i] = para->adegammaAttr.stManual.X_axis[i + 1] - para->adegammaAttr.stManual.X_axis[i];
        tmp[i] = log(tmp[i]) / log(2) - DEGAMMA_CRUVE_X_NORMALIZE_FACTOR;
        tmp[i] = DEGAMMA_LIMIT_VALUE(tmp[i], DEGAMMA_CRUVE_X_MAX, DEGAMMA_CRUVE_X_MIN);
    }

    int tmp0 = 0, tmp1 = 0;
    for(int i = 0; i < 8; i++) {
        tmp0 |= ((tmp[i]) << (4 * i));
        tmp1 |= ((tmp[i + 8]) << (4 * i));
    }
    AdegammaProcRes->degamma_X_d0 = tmp0;
    AdegammaProcRes->degamma_X_d1 = tmp1;

    for(int i = 0; i < DEGAMMA_CRUVE_Y_KNOTS; i++) {
        AdegammaProcRes->degamma_tableR[i] =
            DEGAMMA_LIMIT_VALUE(para->adegammaAttr.stManual.curve_R[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
        AdegammaProcRes->degamma_tableG[i] =
            DEGAMMA_LIMIT_VALUE(para->adegammaAttr.stManual.curve_G[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
        AdegammaProcRes->degamma_tableB[i] =
            DEGAMMA_LIMIT_VALUE(para->adegammaAttr.stManual.curve_B[i], DEGAMMA_CRUVE_Y_MAX, DEGAMMA_CRUVE_Y_MIN);
    }

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
}

void AdegammaProcessing(AdegammaHandle_t* para, AdegammaProcRes_t* AdegammaProcRes)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);
    LOGD_ADEGAMMA("===============================Adegamma Start===============================\n", __func__);

    if(para->adegammaAttr.mode == RK_AIQ_DEGAMMA_MODE_OFF) { //run iq degamma
        LOGD_ADEGAMMA(" %s: Adegamma api off !!!\n", __func__);
        AdegammaAutoProc(para->pCalibDb, AdegammaProcRes);
    }
    else if(para->adegammaAttr.mode == RK_AIQ_DEGAMMA_MODE_MANUAL) { //run manual degamma, for client api
        LOGD_ADEGAMMA(" %s: Adegamma api on, Mode is Manual\n", __func__);
        AdegammaApiManualProc(para, AdegammaProcRes);
    }
    else if(para->adegammaAttr.mode == RK_AIQ_DEGAMMA_MODE_TOOL) { //run tool degamma,for tool
        LOGD_ADEGAMMA(" %s: Adegamma api on, Mode is Tool\n", __func__);
        AdegammaAutoProc(&para->adegammaAttr.stTool, AdegammaProcRes);
    }
    else
        LOGE_ADEGAMMA(" %s: Wrong degamma mode !!!\n", __func__);

    //transfer scene mode to degamma api
    para->adegammaAttr.Scene_mode = para->Scene_mode;

    LOGD_ADEGAMMA("%s DEGAMMA_DX0:%d GAMMA_DX1:%d\n", __func__, AdegammaProcRes->degamma_X_d0, AdegammaProcRes->degamma_X_d1);
    LOGD_ADEGAMMA("%s DEGAMMA_R_Y:%d %d %d %d %d %d %d %d\n", __func__, AdegammaProcRes->degamma_tableR[0], AdegammaProcRes->degamma_tableR[1],
                  AdegammaProcRes->degamma_tableR[2], AdegammaProcRes->degamma_tableR[3], AdegammaProcRes->degamma_tableR[4], AdegammaProcRes->degamma_tableR[5],
                  AdegammaProcRes->degamma_tableR[6], AdegammaProcRes->degamma_tableR[7]);
    LOGD_ADEGAMMA("%s DEGAMMA_G_Y:%d %d %d %d %d %d %d %d\n", __func__, AdegammaProcRes->degamma_tableG[0], AdegammaProcRes->degamma_tableG[1],
                  AdegammaProcRes->degamma_tableG[2], AdegammaProcRes->degamma_tableG[3], AdegammaProcRes->degamma_tableG[4], AdegammaProcRes->degamma_tableG[5],
                  AdegammaProcRes->degamma_tableG[6], AdegammaProcRes->degamma_tableG[7]);
    LOGD_ADEGAMMA("%s DEGAMMA_B_Y:%d %d %d %d %d %d %d %d\n", __func__, AdegammaProcRes->degamma_tableB[0], AdegammaProcRes->degamma_tableB[1],
                  AdegammaProcRes->degamma_tableB[2], AdegammaProcRes->degamma_tableB[3], AdegammaProcRes->degamma_tableB[4], AdegammaProcRes->degamma_tableB[5],
                  AdegammaProcRes->degamma_tableB[6], AdegammaProcRes->degamma_tableB[7]);
    LOGD_ADEGAMMA("===============================Adegamma Stop================================\n", __func__);

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
}

RKAIQ_END_DECLARE



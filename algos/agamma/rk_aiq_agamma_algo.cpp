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
#include "rk_aiq_agamma_algo.h"

RKAIQ_BEGIN_DECLARE

XCamReturn AgammaInit(AgammaHandle_t** para, CamCalibDbV2Context_t* calib)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* handle = (AgammaHandle_t*)calloc(sizeof(AgammaHandle_t), 1);
    if (NULL == handle)
        return XCAM_RETURN_ERROR_MEM;

    CalibDbV2_gamma_t* calibv2_agamma_calib =
        (CalibDbV2_gamma_t*)(CALIBDBV2_GET_MODULE_PTR(calib, agamma_calib));
    if (!calibv2_agamma_calib) {
        free(handle);
        return XCAM_RETURN_ERROR_MEM;
    }
    memcpy(&handle->agammaAttr.stTool, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_t));
    handle->pCalibDb = calibv2_agamma_calib;
    *para = handle;
    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

XCamReturn AgammaRelease(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (para)
        free(para);
    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

XCamReturn AgammaPreProc(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return(ret);

}

void AgammaAutoProc(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    para->agamma_config.gamma_out_segnum = para->pCalibDb->GammaTuningPara.Gamma_out_segnum;
    para->agamma_config.gamma_out_offset = para->pCalibDb->GammaTuningPara.Gamma_out_offset;

    for(int i = 0; i < 45; i++)
    {
        int tmp = para->pCalibDb->GammaTuningPara.Gamma_curve[i];
        para->agamma_config.gamma_table[i] = tmp;
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaApiManualProc(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    LOGD_AGAMMA(" %s: Agamma api manual !!!\n", __func__);

    para->agamma_config.gamma_en = para->agammaAttr.stManual.en ;
    if(para->agammaAttr.stManual.CurveType == RK_GAMMA_CURVE_TYPE_DEFUALT)
        AgammaAutoProc(para);
    else if(para->agammaAttr.stManual.CurveType == RK_GAMMA_CURVE_TYPE_SRGB)
    {
        float x[45] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                        64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                        1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
                      };
        float y[45];
        int y_out[45];

        para->agamma_config.gamma_out_segnum = 0;
        para->agamma_config.gamma_out_offset = 0;
        for(int i = 0; i < 45; i++)
        {
            y[i] = 4095 * pow(x[i] / 4095, 1 / 2.2) + para->agamma_config.gamma_out_offset;
            y[i] = LIMIT_VALUE(y[i], 4095, 0);
            y_out[i] = (int)(y[i] + 0.5);
        }
        memcpy(para->agamma_config.gamma_table, y_out, sizeof(para->agamma_config.gamma_table));
    }
    else if(para->agammaAttr.stManual.CurveType == RK_GAMMA_CURVE_TYPE_HDR)
    {
        float x[45] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                        64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                        1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
                      };
        float y[45];
        int y_out[45];

        para->agamma_config.gamma_out_segnum = 0;
        para->agamma_config.gamma_out_offset = 0;
        for(int i = 0; i < 45; i++)
        {
            y[i] = 4095 * pow(x[i] / 4095, 1 / 2.2) + para->agamma_config.gamma_out_offset;
            y[i] = LIMIT_VALUE(y[i], 4095, 0);
            y_out[i] = (int)(y[i] + 0.5);
        }
        memcpy(para->agamma_config.gamma_table, y_out, sizeof(para->agamma_config.gamma_table));
    }
    else if(para->agammaAttr.stManual.CurveType == RK_GAMMA_CURVE_TYPE_USER_DEFINE1)
    {
        float x[45] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
                        64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 640, 768, 896, 1024,
                        1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095
                      };
        float y[45];
        int y_out[45];
        float coef1 = para->agammaAttr.stManual.user1.coef1;
        float coef2 = para->agammaAttr.stManual.user1.coef2;

        coef2 = LIMIT_VALUE(coef2, 0.05, -0.05);
        para->agamma_config.gamma_out_segnum = 0;
        para->agamma_config.gamma_out_offset = 0;
        for(int i = 0; i < 45; i++)
        {
            y[i] = 4095 * pow(x[i] / 4095, 1 / coef1 + coef2) + para->agamma_config.gamma_out_offset;
            y[i] = LIMIT_VALUE(y[i], 4095, 0);
            y_out[i] = (int)(y[i] + 0.5);
        }
        memcpy(para->agamma_config.gamma_table, y_out, sizeof(para->agamma_config.gamma_table));
    }
    else if(para->agammaAttr.stManual.CurveType == RK_GAMMA_CURVE_TYPE_USER_DEFINE2)
    {
        para->agamma_config.gamma_out_segnum = para->agammaAttr.stManual.user2.gamma_out_segnum;
        para->agamma_config.gamma_out_offset = para->agammaAttr.stManual.user2.gamma_out_offset;
        memcpy(para->agamma_config.gamma_table, para->agammaAttr.stManual.user2.gamma_table, sizeof(para->agamma_config.gamma_table));
    }
    else
        LOGE_AGAMMA(" %s: Wrong gamma api manual CurveType!!!\n", __func__);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaApiToolProc(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    LOGD_AGAMMA(" %s: Agamma api tool !!!\n", __func__);

    para->agamma_config.gamma_en = para->agammaAttr.stTool.GammaTuningPara.Gamma_en;
    para->agamma_config.gamma_out_segnum = para->agammaAttr.stTool.GammaTuningPara.Gamma_out_segnum;
    para->agamma_config.gamma_out_offset = para->agammaAttr.stTool.GammaTuningPara.Gamma_out_offset;
    for(int i = 0; i < 45; i++) {
        int tmp = para->agammaAttr.stTool.GammaTuningPara.Gamma_curve[i];
        para->agamma_config.gamma_table[i] = tmp;
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaProcessing(AgammaHandle_t* para)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    if(para->agammaAttr.mode == RK_AIQ_GAMMA_MODE_OFF)//run iq gamma
    {
        LOGD_AGAMMA(" %s: Agamma api off !!!\n", __func__);
        para->agamma_config.gamma_en = para->pCalibDb->GammaTuningPara.Gamma_en;
        AgammaAutoProc(para);
    }
    else if(para->agammaAttr.mode == RK_AIQ_GAMMA_MODE_MANUAL)//run manual gamma, for client api
        AgammaApiManualProc( para);
    else if(para->agammaAttr.mode == RK_AIQ_GAMMA_MODE_TOOL)//run tool gamma,for tool
        AgammaApiToolProc( para);
    else
        LOGE_AGAMMA(" %s: Wrong gamma mode !!!\n", __func__);

    LOGD_AGAMMA(" %s: gamma_en:%d gamma_out_segnum:%d gamma_out_offset:%d\n", __func__, para->agamma_config.gamma_en, para->agamma_config.gamma_out_segnum
                , para->agamma_config.gamma_out_offset);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaSetProcRes(AgammaProcRes_t* AgammaProcRes, rk_aiq_gamma_cfg_t* agamma_config)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    AgammaProcRes->gamma_en = agamma_config->gamma_en;
    AgammaProcRes->equ_segm = agamma_config->gamma_out_segnum;
    AgammaProcRes->offset = agamma_config->gamma_out_offset;
    for(int i = 0; i < 45; i++)
        AgammaProcRes->gamma_y[i] = agamma_config->gamma_table[i];

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

RKAIQ_END_DECLARE



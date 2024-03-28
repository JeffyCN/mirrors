/*
 * rk_aiq_agamma_algo_v10.cpp
 *
 *  Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "rk_aiq_agamma_algo_v10.h"
#include <string.h>
#include "rk_aiq_types_agamma_algo_int.h"
#include "rk_aiq_types_agamma_algo_prvt.h"
#include "xcam_common.h"

RKAIQ_BEGIN_DECLARE

XCamReturn AgammaInit(AgammaHandle_t** pGammaCtx, CamCalibDbV2Context_t* pCalib) {
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret         = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* handle = (AgammaHandle_t*)calloc(sizeof(AgammaHandle_t), 1);
    if (NULL == handle) return XCAM_RETURN_ERROR_MEM;

    CalibDbV2_gamma_v10_t* calibv2_agamma_calib =
        (CalibDbV2_gamma_v10_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, agamma_calib));

    if (!calibv2_agamma_calib) {
        free(handle);
        return XCAM_RETURN_ERROR_MEM;
    }
    memcpy(&handle->agammaAttrV10.stAuto, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_v10_t));
    handle->agammaAttrV10.mode              = RK_AIQ_GAMMA_MODE_AUTO;
    handle->ifReCalcStAuto                  = true;
    handle->ifReCalcStManual                = false;

    *pGammaCtx = handle;
    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AgammaRelease(AgammaHandle_t* pGammaCtx) {
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pGammaCtx) free(pGammaCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return (ret);
}

void AgammaSetProcResV10(AgammaHandle_t* pGammaCtx, AgammaProcRes_t* pAgammaProcRes) {
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    pAgammaProcRes->Gamma_v10.gamma_en = pGammaCtx->agamma_config.gamma_en;
    pAgammaProcRes->Gamma_v10.equ_segm = pGammaCtx->agamma_config.gamma_out_segnum;
    pAgammaProcRes->Gamma_v10.offset   = pGammaCtx->agamma_config.gamma_out_offset;
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++)
        pAgammaProcRes->Gamma_v10.gamma_y[i] = pGammaCtx->agamma_config.gamma_table[i];

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}

void AgammaProcessing(AgammaHandle_t* pGammaCtx, AgammaProcRes_t* pProcRes) {
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    if (pGammaCtx->agammaAttrV10.mode == RK_AIQ_GAMMA_MODE_AUTO) {
            pGammaCtx->agamma_config.gamma_en =
                pGammaCtx->agammaAttrV10.stAuto.GammaTuningPara.Gamma_en;
            if (pGammaCtx->agamma_config.gamma_en) {
                pGammaCtx->agamma_config.gamma_out_segnum =
                    pGammaCtx->agammaAttrV10.stAuto.GammaTuningPara.Gamma_out_segnum;
                pGammaCtx->agamma_config.gamma_out_offset =
                    pGammaCtx->agammaAttrV10.stAuto.GammaTuningPara.Gamma_out_offset;

                for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++) {
                    int tmp = pGammaCtx->agammaAttrV10.stAuto.GammaTuningPara.Gamma_curve[i];
                    pGammaCtx->agamma_config.gamma_table[i] =
                        LIMIT_VALUE(tmp, GAMMA_CRUVE_MAX, GAMMA_CRUVE_MIN);
                }
            }
    } else if (pGammaCtx->agammaAttrV10.mode == RK_AIQ_GAMMA_MODE_MANUAL) {
            pGammaCtx->agamma_config.gamma_en = pGammaCtx->agammaAttrV10.stManual.Gamma_en;
            if (pGammaCtx->agamma_config.gamma_en) {
                pGammaCtx->agamma_config.gamma_out_segnum =
                    pGammaCtx->agammaAttrV10.stManual.Gamma_out_segnum;
                pGammaCtx->agamma_config.gamma_out_offset =
                    pGammaCtx->agammaAttrV10.stManual.Gamma_out_offset;
                for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++) {
                    int tmp = pGammaCtx->agammaAttrV10.stManual.Gamma_curve[i];
                    pGammaCtx->agamma_config.gamma_table[i] =
                        LIMIT_VALUE(tmp, GAMMA_CRUVE_MAX, GAMMA_CRUVE_MIN);
                }
            }
    } else
        LOGE_AGAMMA(" %s: FrameID:%d Wrong gamma mode !!!\n", __func__, pGammaCtx->FrameID);

    LOGD_AGAMMA(
        " %s: FrameID:%d gamma_en:%d gamma_out_segnum:%d gamma_out_offset:%d apiMode:%d "
        "ifReCalcStManual:%d ifReCalcStAuto:%d\n",
        __func__, pGammaCtx->FrameID, pGammaCtx->agamma_config.gamma_en,
        pGammaCtx->agamma_config.gamma_out_segnum, pGammaCtx->agamma_config.gamma_out_offset,
        pGammaCtx->agammaAttrV10.mode, pGammaCtx->ifReCalcStManual, pGammaCtx->ifReCalcStAuto);
    LOGD_AGAMMA(" %s: Gamma_curve[0-15]:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __func__, pGammaCtx->agamma_config.gamma_table[0],
                pGammaCtx->agamma_config.gamma_table[1], pGammaCtx->agamma_config.gamma_table[2],
                pGammaCtx->agamma_config.gamma_table[3], pGammaCtx->agamma_config.gamma_table[4],
                pGammaCtx->agamma_config.gamma_table[5], pGammaCtx->agamma_config.gamma_table[6],
                pGammaCtx->agamma_config.gamma_table[7], pGammaCtx->agamma_config.gamma_table[8],
                pGammaCtx->agamma_config.gamma_table[9], pGammaCtx->agamma_config.gamma_table[10],
                pGammaCtx->agamma_config.gamma_table[11], pGammaCtx->agamma_config.gamma_table[12],
                pGammaCtx->agamma_config.gamma_table[13], pGammaCtx->agamma_config.gamma_table[14],
                pGammaCtx->agamma_config.gamma_table[15]);
    LOGD_AGAMMA(" %s: Gamma_curve[16-31]:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __func__, pGammaCtx->agamma_config.gamma_table[16],
                pGammaCtx->agamma_config.gamma_table[17], pGammaCtx->agamma_config.gamma_table[18],
                pGammaCtx->agamma_config.gamma_table[19], pGammaCtx->agamma_config.gamma_table[20],
                pGammaCtx->agamma_config.gamma_table[21], pGammaCtx->agamma_config.gamma_table[22],
                pGammaCtx->agamma_config.gamma_table[23], pGammaCtx->agamma_config.gamma_table[24],
                pGammaCtx->agamma_config.gamma_table[25], pGammaCtx->agamma_config.gamma_table[26],
                pGammaCtx->agamma_config.gamma_table[27], pGammaCtx->agamma_config.gamma_table[28],
                pGammaCtx->agamma_config.gamma_table[29], pGammaCtx->agamma_config.gamma_table[30],
                pGammaCtx->agamma_config.gamma_table[31]);
    LOGD_AGAMMA(" %s: Gamma_curve[32-44]:%d %d %d %d %d %d %d %d %d %d %d %d %d\n", __func__,
                pGammaCtx->agamma_config.gamma_table[32], pGammaCtx->agamma_config.gamma_table[33],
                pGammaCtx->agamma_config.gamma_table[34], pGammaCtx->agamma_config.gamma_table[35],
                pGammaCtx->agamma_config.gamma_table[36], pGammaCtx->agamma_config.gamma_table[37],
                pGammaCtx->agamma_config.gamma_table[38], pGammaCtx->agamma_config.gamma_table[39],
                pGammaCtx->agamma_config.gamma_table[40], pGammaCtx->agamma_config.gamma_table[41],
                pGammaCtx->agamma_config.gamma_table[42], pGammaCtx->agamma_config.gamma_table[43],
                pGammaCtx->agamma_config.gamma_table[44]);

    pGammaCtx->CurrApiMode = pGammaCtx->agammaAttrV10.mode;

    AgammaSetProcResV10(pGammaCtx, pProcRes);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
}
RKAIQ_END_DECLARE

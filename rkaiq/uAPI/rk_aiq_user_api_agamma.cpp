/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "rk_aiq_user_api_agamma.h"

#include "algo_handlers/RkAiqAgammaHandle.h"
#include "uAPI2/rk_aiq_user_api2_agamma.h"

RKAIQ_BEGIN_DECLARE


#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_GAMMA_V10
void GammaTransferSetData(rk_aiq_gamma_v10_attr_t* DegammaV2, rk_aiq_gamma_attrib_t* DegammaV1) {
    /*
        //op mode
        DegammaV2->mode = DegammaV1->mode;

        //st manual
        memcpy(&DegammaV2->stManual, &DegammaV1->stManual, sizeof(Adegamma_api_manual_t));

        //set tool

        DegammaV2->stTool.GammaTuningPara.Gamma_en = DegammaV1->stTool.gamma_en ? true : false;
        DegammaV2->stTool.GammaTuningPara.Gamma_out_offset = DegammaV1->stTool.gamma_out_offset;
        if(DegammaV1->stTool.gamma_out_segnum == 0)
            DegammaV2->stTool.GammaTuningPara.Gamma_out_segnum = GAMMATYPE_LOG;
        else
            DegammaV2->stTool.GammaTuningPara.Gamma_out_segnum = GAMMATYPE_EQU;
        int mode = DegammaV1->Scene_mode;
        if(mode == 0)
            for(int i = 0; i < 45; i++)
                DegammaV2->stTool.GammaTuningPara.Gamma_curve[i] = (int)(DegammaV1->stTool.curve_normal[i] + 0.5);
        else if(mode == 1)
            for(int i = 0; i < 45; i++)
                DegammaV2->stTool.GammaTuningPara.Gamma_curve[i] = (int)(DegammaV1->stTool.curve_hdr[i] + 0.5);
        else if(mode == 3)
            for(int i = 0; i < 45; i++)
                DegammaV2->stTool.GammaTuningPara.Gamma_curve[i] = (int)(DegammaV1->stTool.curve_night[i] + 0.5);
        else
            for(int i = 0; i < 45; i++)
                DegammaV2->stTool.GammaTuningPara.Gamma_curve[i] = (int)(DegammaV1->stTool.curve_normal[i] + 0.5);
                */
}

void GammaTransferGetData(rk_aiq_gamma_v10_attr_t* DegammaV2, rk_aiq_gamma_attrib_t* DegammaV1) {
    /*
    //op mode
    DegammaV1->mode = DegammaV2->mode;

    //st manual
    memcpy(&DegammaV1->stManual, &DegammaV2->stManual, sizeof(Adegamma_api_manual_t));

    //mode
    DegammaV1->Scene_mode = 0;

    //set tool
    DegammaV1->stTool.gamma_en = DegammaV2->stTool.GammaTuningPara.Gamma_en ? 1 : 0;
    DegammaV1->stTool.gamma_out_offset = DegammaV2->stTool.GammaTuningPara.Gamma_out_offset;
    if(DegammaV2->stTool.GammaTuningPara.Gamma_out_segnum == GAMMATYPE_LOG)
        DegammaV1->stTool.gamma_out_segnum = 0;
    else if(DegammaV2->stTool.GammaTuningPara.Gamma_out_segnum == GAMMATYPE_EQU)
        DegammaV1->stTool.gamma_out_segnum = 1;
    for(int i = 0; i < 45; i++) {
        DegammaV1->stTool.curve_normal[i] = (float)(DegammaV2->stTool.GammaTuningPara.Gamma_curve[i]);
        DegammaV1->stTool.curve_hdr[i] = (float)(DegammaV2->stTool.GammaTuningPara.Gamma_curve[i]);
        DegammaV1->stTool.curve_night[i] = (float)(DegammaV2->stTool.GammaTuningPara.Gamma_curve[i]);
    }
    */
}

XCamReturn rk_aiq_user_api_agamma_SetAttrib(
    const rk_aiq_sys_ctx_t* sys_ctx,
    rk_aiq_gamma_attrib_t
        attr) { /*
                   rk_aiq_gamma_v10_attr_t gammaAttr;
                   memset(&gammaAttr, 0, sizeof(rk_aiq_gamma_v10_attr_t));

                   GammaTransferSetData(&gammaAttr, &attr);

                   XCamReturn ret_gamma = rk_aiq_user_api2_agamma_V10_SetAttrib(sys_ctx, gammaAttr);
               */
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api_agamma_GetAttrib(
    const rk_aiq_sys_ctx_t* sys_ctx,
    rk_aiq_gamma_attrib_t* attr) { /*
                                      rk_aiq_gamma_v10_attr_t gammaAttr;
                                      memset(&gammaAttr, 0, sizeof(rk_aiq_gamma_attrib_V10_t));

                                      GammaTransferGetData(&gammaAttr, attr);

                                      XCamReturn ret_gamma =
                                      rk_aiq_user_api2_agamma_V10_GetAttrib(sys_ctx, &gammaAttr);
                                      */

    return XCAM_RETURN_ERROR_UNKNOWN;
}
#else
XCamReturn
rk_aiq_user_api_agamma_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_gamma_attrib_t attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_agamma_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_gamma_attrib_t *attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif


RKAIQ_END_DECLARE

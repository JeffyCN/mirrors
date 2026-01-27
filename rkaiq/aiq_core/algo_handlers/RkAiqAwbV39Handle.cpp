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
 */
#include "RkAiqAwbV39Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAwbV39HandleInt);

XCamReturn RkAiqAwbV39HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbV39HandleInt::setWbV39AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV39_awb_nonROI_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    ret = rk_aiq_uapiV2_awbV39_SetAwbMultiwindow(mAlgoCtx, att, false);


    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::getWbV39AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV39_awb_nonROI_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    mCfgMutex.lock();
    rk_aiq_uapiV2_awbV39_GetAwbMultiwindow(mAlgoCtx, att);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::setWbV39Attrib(rk_aiq_uapiV2_wbV39_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    ret = rk_aiq_uapiV2_awbV39_SetAttrib(mAlgoCtx, att, false);


    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::getWbV39Attrib(rk_aiq_uapiV2_wbV39_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    mCfgMutex.lock();
    rk_aiq_uapiV2_awbV39_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbV39HandleInt::writeAwbIn(rk_aiq_uapiV2_awb_wrtIn_attr_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    ret = rk_aiq_uapiV2_awb_WriteInput(mAlgoCtx, att, false);


    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::setWbV32IQAutoExtPara(const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    ret = rk_aiq_uapiV2_awb_SetIQAutoExtPara(mAlgoCtx, att, false);


    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::getWbV32IQAutoExtPara(rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetIQAutoExtPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::setWbV32IQAutoPara(const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetIQAutoPara(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::getWbV32IQAutoPara(rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetIQAutoPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbV39HandleInt::awbIqMap2Main( rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_IqMap2Main(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV39HandleInt::setAwbPreWbgain( const float att[4]) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetPreWbgain(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam

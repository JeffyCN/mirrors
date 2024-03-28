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
#ifndef _RK_AIQ_CCM_HANDLE_INT_H_
#define _RK_AIQ_CCM_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "accm/rk_aiq_uapi_accm_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_CCM
class RkAiqAccmHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAccmHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_CCM_V1
        memset(&mCurAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mCurCalibAtt, 0, sizeof(rk_aiq_ccm_calib_attrib_t));
        memset(&mNewCalibAtt, 0, sizeof(rk_aiq_ccm_calib_attrib_t));
#endif
#if RKAIQ_HAVE_CCM_V2
        memset(&mCurAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
        memset(&mNewAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
        memset(&mCurCalibAttV2, 0, sizeof(rk_aiq_ccm_v2_calib_attrib_t));
        memset(&mNewCalibAttV2, 0, sizeof(rk_aiq_ccm_v2_calib_attrib_t));
#endif
#if RKAIQ_HAVE_CCM_V3
        memset(&mCurAttV3, 0, sizeof(rk_aiq_ccm_v3_attrib_t));
        memset(&mNewAttV3, 0, sizeof(rk_aiq_ccm_v3_attrib_t));
        memset(&mCurCalibAttV3, 0, sizeof(rk_aiq_ccm_v3_calib_attrib_t));
        memset(&mNewCalibAttV3, 0, sizeof(rk_aiq_ccm_v3_calib_attrib_t));
#endif
#endif
        colorConstFlag = false;
        memset(&colorSwInfo,0,sizeof(colorSwInfo));
    };
    virtual ~RkAiqAccmHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_CCM_V1
    XCamReturn setAttrib(const rk_aiq_ccm_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ccm_attrib_t* att);
    XCamReturn setIqParam(const rk_aiq_ccm_calib_attrib_t* att);
    XCamReturn getIqParam(rk_aiq_ccm_calib_attrib_t* att);
#endif
#if RKAIQ_HAVE_CCM_V2
    XCamReturn setAttribV2(const rk_aiq_ccm_v2_attrib_t* att);
    XCamReturn getAttribV2(rk_aiq_ccm_v2_attrib_t* att);
    XCamReturn setIqParamV2(const rk_aiq_ccm_v2_calib_attrib_t* att);
    XCamReturn getIqParamV2(rk_aiq_ccm_v2_calib_attrib_t* att);
#endif
#if RKAIQ_HAVE_CCM_V3
    XCamReturn setAttribV3(const rk_aiq_ccm_v3_attrib_t* att);
    XCamReturn getAttribV3(rk_aiq_ccm_v3_attrib_t* att);
    XCamReturn setIqParamV3(const rk_aiq_ccm_v3_calib_attrib_t* att);
    XCamReturn getIqParamV3(rk_aiq_ccm_v3_calib_attrib_t* att);
#endif
    XCamReturn queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info);
    XCamReturn getAcolorSwInfo(rk_aiq_color_info_t* aColor_sw_info);
    XCamReturn setAcolorSwInfo(rk_aiq_color_info_t aColor_sw_info);


 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };
 private:
     bool colorConstFlag;
     rk_aiq_color_info_t colorSwInfo;

#ifndef DISABLE_HANDLE_ATTRIB
    // TODO
#if RKAIQ_HAVE_CCM_V1
    rk_aiq_ccm_attrib_t          mCurAtt;
    rk_aiq_ccm_calib_attrib_t    mCurCalibAtt;
    rk_aiq_ccm_attrib_t          mNewAtt;
    rk_aiq_ccm_calib_attrib_t    mNewCalibAtt;
#endif
#if RKAIQ_HAVE_CCM_V2
    rk_aiq_ccm_v2_attrib_t       mCurAttV2;
    rk_aiq_ccm_v2_calib_attrib_t mCurCalibAttV2;
    rk_aiq_ccm_v2_attrib_t       mNewAttV2;
    rk_aiq_ccm_v2_calib_attrib_t mNewCalibAttV2;
#endif
#if RKAIQ_HAVE_CCM_V3
    rk_aiq_ccm_v3_attrib_t mCurAttV3;
    rk_aiq_ccm_v3_calib_attrib_t mCurCalibAttV3;
    rk_aiq_ccm_v3_attrib_t mNewAttV3;
    rk_aiq_ccm_v3_calib_attrib_t mNewCalibAttV3;
#endif
    mutable std::atomic<bool>    updateCalibAttr;
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAccmHandleInt);
};
#endif
}  // namespace RkCam

#endif

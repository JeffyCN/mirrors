/*
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

#ifndef _CAM_HW_SIMULATOR_H_
#define _CAM_HW_SIMULATOR_H_

#include "CamHwBase.h"

namespace RkCam {

class CamHwSimulator : public CamHwBase {
public:
    explicit CamHwSimulator();
    virtual ~CamHwSimulator();
    // from ICamHw
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn deInit();
    virtual XCamReturn prepare(uint32_t width, uint32_t height, rk_aiq_working_mode_t mode);
    virtual XCamReturn start();
    virtual XCamReturn stop();
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des);
    virtual XCamReturn setIspParams(SmartPtr<RkAiqIspParamsProxy>& ispParams);
    virtual XCamReturn setHdrProcessCount(uint32_t frame_id, int count);
    virtual XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar);
    virtual XCamReturn setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    virtual XCamReturn setIspLumaListener(IspLumaListener* lumaListener);
    virtual XCamReturn setIspStatsListener(IspStatsListener* statsListener);
    virtual XCamReturn setEvtsListener(IspEvtsListener* evtListener);
    XCamReturn setModuleCtl(rk_aiq_module_id_t moduleId, bool en) {
        return XCAM_RETURN_NO_ERROR;
    };
    XCamReturn getModuleCtl(rk_aiq_module_id_t moduleId, bool& en) {
        return XCAM_RETURN_NO_ERROR;
    };
    XCamReturn notify_capture_raw() {
        return XCAM_RETURN_NO_ERROR;
    };
    XCamReturn capture_raw_ctl(bool sync) {
        return XCAM_RETURN_NO_ERROR;
    };
    virtual void getShareMemOps(isp_drv_share_mem_ops_t** mem_ops) {};
    virtual XCamReturn enqueueRawBuffer(struct rk_aiq_vbuf *vbuf, bool sync) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn enqueueRawFile(const char *path) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
private:
    XCAM_DEAD_COPY (CamHwSimulator);
};

};

#endif

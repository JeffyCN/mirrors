/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#ifndef _I_RK_AIQ_RESOURCE_TRANSLATOR_H_
#define _I_RK_AIQ_RESOURCE_TRANSLATOR_H_

#include "smartptr.h"
#include "xcam_log.h"
#include "video_buffer.h"
#include "rk_aiq_pool.h"

namespace RkCam {

class IRkAiqResourceTranslator {
public:
    IRkAiqResourceTranslator ()
        : mCamPhyId(-1)
        , mIsGroupMode(false)
        , mModuleRotation(0)
        , mWorkingMode(0) {};
    virtual ~IRkAiqResourceTranslator () {};

    virtual XCamReturn translateIspStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqIspStatsIntProxy>& to,
                                         const SmartPtr<RkAiqAecStatsProxy>& aecStat,
                                         const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                                         const SmartPtr<RkAiqAfStatsProxy>& afStat,
                                         const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
                                         const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAecStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAecStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAwbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAwbStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAfStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAfStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateOrbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqOrbStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAtmoStats(const SmartPtr<VideoBuffer>& from,
                                          SmartPtr<RkAiqAtmoStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
            SmartPtr<RkAiqAdehazeStatsProxy>& to) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateAgainStats(const SmartPtr<VideoBuffer>& from,
            SmartPtr<RkAiqAgainStatsProxy>& to){
        return XCAM_RETURN_BYPASS;
    }
#if RKAIQ_HAVE_PDAF
    virtual XCamReturn translatePdafStats (const SmartPtr<VideoBuffer> &from,
                                           SmartPtr<RkAiqPdafStatsProxy> &to, bool sns_mirror) {
        return XCAM_RETURN_BYPASS;
    }
#endif
    virtual XCamReturn getParams(const SmartPtr<VideoBuffer>& from) {
        return XCAM_RETURN_BYPASS;
    }
    virtual XCamReturn translateBay3dStats(const SmartPtr<VideoBuffer>& from,
                                           std::list<SmartPtr<RkAiqBay3dStat>>& to, Mutex& mutex) {
        return XCAM_RETURN_BYPASS;
    }
    virtual void releaseParams() {}
    void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }
    void setGroupMode(bool bGroup) {
        mIsGroupMode = bGroup;
    }
    void setModuleRot(int rot) {
        mModuleRotation = rot;
    }
    void setWorkingMode(int working_mode) {
        mWorkingMode = working_mode;
    }
    int getWorkingMode() {
        return mWorkingMode;
    }

protected:
    int mCamPhyId;
    bool mIsGroupMode;
    int mModuleRotation;
    int mWorkingMode;

private:
    XCAM_DEAD_COPY (IRkAiqResourceTranslator);
};

}

#endif //_I_RK_AIQ_CONFIG_TRANSLATOR_H_

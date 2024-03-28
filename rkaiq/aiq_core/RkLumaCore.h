/*
 * rkisp_aiq_core.h
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

#ifndef _RK_LUMA_CORE_H_
#define _RK_LUMA_CORE_H_

#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "xcam_log.h"
#include "video_buffer.h"
#include "rk_aiq_luma.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbV2Helper.h"
#include "rk_aiq_types.h"

using namespace XCam;
namespace RkCam {

#define RKAIQLUMA_CORE_CHECK_RET(ret, format, ...) \
    if (ret) { \
        LOGE_ANALYZER(format, ##__VA_ARGS__); \
        return ret; \
    }

#define LUMA_FIFO_CNT 2

class RkLumaCore;

class RkLumaAnalyzerCb {
public:
    explicit RkLumaAnalyzerCb() {};
    virtual ~RkLumaAnalyzerCb() {};
    virtual void rkLumaCalcDone(rk_aiq_luma_params_t luma_params) = 0;
    virtual void rkLumaCalcFailed(const char* msg) = 0;
private:
    XCAM_DEAD_COPY (RkLumaAnalyzerCb);
};

class RkLumaCoreThread
    : public Thread {
public:
    RkLumaCoreThread(RkLumaCore* rkLumaCore)
        : Thread("RkLumaCoreThread")
        , mRkLumaCore(rkLumaCore) {};
    ~RkLumaCoreThread() {
        mStatsQueue.clear ();
    };

    void triger_stop() {
        mStatsQueue.pause_pop ();
    };

    void triger_start() {
        mStatsQueue.clear ();
        mStatsQueue.resume_pop ();
    };

    bool push_stats (const SmartPtr<VideoBuffer> &buffer) {
        mStatsQueue.push (buffer);
        return true;
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mStatsQueue.clear ();
    };
    virtual bool loop ();
private:
    RkLumaCore* mRkLumaCore;
    SafeList<VideoBuffer> mStatsQueue;
};

class RkLumaCore {
    friend class RkLumaCoreThread;

public:
    RkLumaCore();
    virtual ~RkLumaCore();

    bool setAnalyzeResultCb(RkLumaAnalyzerCb* callback) {
        mCb = callback;
        return true;
    }

    // called only once
    XCamReturn init(const CalibDbV2_LUMA_DETECT_t* lumaDetect);
    // called only once
    XCamReturn deInit();
    // start analyze thread
    XCamReturn start();
    // stop analyze thread
    XCamReturn stop();

    XCamReturn pushStats(SmartPtr<VideoBuffer> &buffer);

    XCamReturn prepare(int mode);
private:
    // in analyzer thread
    XCamReturn analyze(const SmartPtr<VideoBuffer> &buffer);
private:
    enum rk_aiq_core_state_e {
        RK_AIQ_CORE_STATE_INVALID,
        RK_AIQ_CORE_STATE_INITED,
        RK_AIQ_CORE_STATE_PREPARED,
        RK_AIQ_CORE_STATE_STARTED,
        RK_AIQ_CORE_STATE_STOPED,
    };

    int mState;
    int mWorkingMode;
    RkLumaAnalyzerCb* mCb;
    SmartPtr<RkLumaCoreThread> mRkLumaCoreTh;
    SafeList<isp_luma_stat_t> mLumaQueueFIFO;
    const CalibDbV2_LUMA_DETECT_t* calib;

    static uint16_t DEFAULT_POOL_SIZE;
};

}

#endif //_RK_LUMA_CORE_H_

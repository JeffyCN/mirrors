/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _RK_AIQ_MANUAL_PARAMS_MANAGER_H_
#define _RK_AIQ_MANUAL_PARAMS_MANAGER_H_

#include "xcore/smartptr.h"
#include "xcore/xcam_mutex.h"
#include "common/rk_aiq_pool.h"

namespace RkCam {
// paramsManager
// Mode: Full Manual, Non Full Manual
// Full Manual: allocate and init module params (only part of manual, same as
//              result buf)
// Non Full:    module params buf are from calib ctx (contains maunual and auto
//              params buf)
// ops: get/set/query by app; get/set/clear by handler

// opMode/en/bypass update
// auto/manual params update

// aec,awb,af
// hw: stats_cfg(path,win,weight), sw: algos

// global struct for part of xx_api_attrib, manage everythings
// except for params Auto
typedef struct rk_aiq_global_params_wrap_s {
    // input & output params
    rk_aiq_op_mode_t opMode;
    bool en;
    bool bypass;
    // input params
    int type;
    int man_param_size;
    void* man_param_ptr;
    int aut_param_size;
    void* aut_param_ptr;
} rk_aiq_global_params_wrap_t;

class GlobalParamsManager {
    public:
        explicit GlobalParamsManager ();
        ~GlobalParamsManager ();
        // fullManMode has no calibDb, non
        void init(bool isFullManMode, CamCalibDbV2Context_t* calibDb);
        void switchCalibDb(CamCalibDbV2Context_t* calibDb);
        // set new params by user API
        XCamReturn set(rk_aiq_global_params_wrap_t* params);
        // get current params by user API
        XCamReturn get(rk_aiq_global_params_wrap_t* param);
        // used in aiq internal
        XCamReturn getAndClearPending(rk_aiq_global_params_wrap_t* wrap);
        SmartPtr<cam3aResult> getAndClearPending(int type);
        bool isManual(int type);
        void hold(bool hold);
        bool isFullManualMode() {
            return mFullManualMode;
        };
        SmartPtr<RkAiqFullParamsProxy>& getFullManParamsProxy() {
            return mFullManualParamsProxy;
        }
        void lockAlgoParam(int type);
        void unlockAlgoParam(int type);
        bool getAndClearAlgoParamUpdateFlagLocked(int type);

        bool checkAlgoEnableBypass(int type, bool en, bool bypass);
    private:
        XCAM_DEAD_COPY (GlobalParamsManager);
        inline XCamReturn get_locked(rk_aiq_global_params_wrap_t* param);
        inline bool isManual_locked(int type);

        typedef struct rk_aiq_global_params_ptr_wrap_s {
            rk_aiq_op_mode_t* opMode;
            bool* en;
            bool* bypass;
            void* man_param_ptr;
            void* aut_param_ptr;
        } rk_aiq_global_params_ptr_wrap_t;

        // current
        rk_aiq_global_params_ptr_wrap_t mGlobalParams[RESULT_TYPE_MAX_PARAM];
        // storage for full manual params
        SmartPtr<RkAiqFullParamsProxy> mFullManualParamsProxy;
        // for fast access
        SmartPtr<cam3aResult> mFullManualParamsProxyArray[RESULT_TYPE_MAX_PARAM];
        // mutex between set attrib and algo processing
        Mutex mAlgoMutex[RESULT_TYPE_MAX_PARAM];

        void init_fullManual();
        void init_withCalib();
        Mutex mMutex;
        bool mIsHold{false};
        bool mFullManualMode{false};
        CamCalibDbV2Context_t* mCalibDb{NULL};
        uint64_t mIsGlobalModulesUpdateBits{0};
        uint64_t mIsAlgoParamUpdateBits{0};
};

} //namespace RkCam

#endif

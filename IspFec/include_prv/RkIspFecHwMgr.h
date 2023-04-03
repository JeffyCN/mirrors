/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#ifndef _RK_ISPFEC_HW_MANAGER_H_
#define _RK_ISPFEC_HW_MANAGER_H_

#include "rkispp-config.h"
#include "RkIspFecHw.h"
#include <pthread.h>

namespace RKISPFEC {

class RkIspFecHwMgr {
public:
    static RkIspFecHwMgr* getInstance();
    // caller should ensure no process is running
    void deinit();
    int process(struct rkispp_fec_in_out& param);
    virtual ~RkIspFecHwMgr();
private:
    explicit RkIspFecHwMgr();
    RkIspFecHwMgr (const RkIspFecHwMgr&) = default;
    RkIspFecHwMgr & operator = (const RkIspFecHwMgr&) = default;

    static RkIspFecHwMgr* mInstance;
    static pthread_mutex_t mMutex;
    static pthread_cond_t mCond;
    static int mRefCnt;

    int readFileList(const char *basePath);
    void findFecEntry();
    char mFecVdPath[2][32];
    int mFecVdNum;

    RkIspFecHw* mFecHw[2];
    bool mIsFecHwWking[2];
    int selectFecHw();
};

};

#endif

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

#ifndef _RK_ISPFEC_HW_H_
#define _RK_ISPFEC_HW_H_

#include "rkfec_config.h"
#ifdef RKFEC_HW_V20
#include "rk-fec-config.h"
#else
#include "rkispp-config.h"
#endif

namespace RKISPFEC {

#ifdef RKFEC_HW_V20
using RKFecInOut = struct rkfec_in_out;
#else
using RKFecInOut = struct rkispp_fec_in_out;
#endif

class RkIspFecHw {
public:
    virtual ~RkIspFecHw();
    explicit RkIspFecHw(const char* dev);
    int process(RKFecInOut& param);
    int detach_dma_buffer(int dma_fd);

 private:
    RkIspFecHw(const RkIspFecHw&) = default;
    RkIspFecHw & operator = (const RkIspFecHw&) = default;
    int mFd;
};

};

#endif

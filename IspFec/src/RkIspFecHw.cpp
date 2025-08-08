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

#include "RkIspFecHw.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "RkIspFecComm.h"

namespace RKISPFEC {

RkIspFecHw::RkIspFecHw(const char* dev)
{
    mFd = ::open(dev, O_RDWR | O_CLOEXEC);
    if (mFd == -1) rkfec_err("open %s failed %s !", dev, strerror(errno));

    rkfec_info("open fec dev %s done !", dev);
}

RkIspFecHw::~RkIspFecHw()
{
    if (mFd >= 0 )
        ::close(mFd);
}

int RkIspFecHw::process(RKFecInOut& param) {
    if (mFd < 0) {
        rkfec_err("E: wrong fec fd ");
        return -1;
    }

#ifdef RKFEC_HW_V20
    unsigned long int cmd = RKFEC_CMD_IN_OUT;
#else
    unsigned long int cmd = RKISPP_CMD_FEC_IN_OUT;
#endif

    Profiler prof = {0};
    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    int ret = ioctl(mFd, cmd, &param);

    if (rkfec_debug > 4) rkfec_profiling_end(&prof, "RkIspFecHw::process", 30);

    if (ret == -EAGAIN) // try again
        ret = ioctl(mFd, cmd, &param);

    return ret;
}

int RkIspFecHw::detach_dma_buffer(int dma_fd) {
    if (mFd < 0) {
        rkfec_err("E: invalid fec fd");
        return -EBADF;
    }

#ifdef RKFEC_HW_V20
    unsigned long int cmd = RKFEC_CMD_BUF_DEL;
#else
    unsigned long int cmd = RKISPP_CMD_FEC_BUF_DEL;
#endif
    return ioctl(mFd, cmd, &dma_fd);
}
};

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
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>

namespace RKISPFEC {

RkIspFecHw::RkIspFecHw(const char* dev)
{
    printf("I: open fec dev %s\n", dev);
    mFd = ::open(dev, O_RDWR | O_CLOEXEC);
    if (mFd == -1)
        printf("E: open %s failed %s !", dev, strerror(errno));
    printf("I: open fec dev %s done !\n", dev);
}

RkIspFecHw::~RkIspFecHw()
{
    if (mFd >= 0 )
        ::close(mFd);
}

int
RkIspFecHw::process(struct rkispp_fec_in_out& param)
{
    if (mFd < 0) {
        printf("E: wrong fec fd \n");
        return -1;
    }

    int ret = ioctl(mFd, RKISPP_CMD_FEC_IN_OUT, &param);

    if (ret == -EAGAIN) // try again
        ret = ioctl(mFd, RKISPP_CMD_FEC_IN_OUT, &param);

    return ret;
}

};

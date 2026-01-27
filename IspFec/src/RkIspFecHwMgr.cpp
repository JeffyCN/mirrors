
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

#include "RkIspFecHwMgr.h"

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>

#include "RkIspFecComm.h"

namespace RKISPFEC {

RkIspFecHwMgr* RkIspFecHwMgr::mInstance = NULL;
pthread_mutex_t RkIspFecHwMgr::mMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t RkIspFecHwMgr::mCond = PTHREAD_COND_INITIALIZER;
int RkIspFecHwMgr::mRefCnt = 0;

#define SEARCH_MAX_VIDEO_NODES 128

RkIspFecHwMgr::RkIspFecHwMgr()
{
    mFecVdNum = 0;
    mFecVdPath[0][0] = '\0';
    mFecVdPath[1][0] = '\0';
    mFecHw[0] = NULL;
    mFecHw[1] = NULL;

    findFecEntry();

    for (int i = 0; i < mFecVdNum; i++) {
        mFecHw[i] = new RkIspFecHw(mFecVdPath[i]);
        mIsFecHwWking[i] = false;
    }
    rkfec_info("%s constructor done !", __FUNCTION__);
}

RkIspFecHwMgr::~RkIspFecHwMgr()
{
    for (int i = 0; i < mFecVdNum; i++) {
        delete mFecHw[i];
    }

}

RkIspFecHwMgr* RkIspFecHwMgr::getInstance()
{
    RkIspFecHwMgr* mgr = NULL;
    pthread_mutex_lock(&mMutex);

    if (!mInstance) {
        mgr = new RkIspFecHwMgr();
        mInstance = mgr;
    } else {
        mgr = mInstance;
    }

    mRefCnt++;
    pthread_mutex_unlock(&mMutex);

    return mgr;
}

void
RkIspFecHwMgr::deinit()
{
    pthread_mutex_lock(&mMutex);

    if (mRefCnt > 0 && mRefCnt--) {
        if (mInstance && mRefCnt == 0) {
            delete mInstance;
            mInstance = NULL;
        }
    }

    pthread_mutex_unlock(&mMutex);

    return;
}

int RkIspFecHwMgr::readFileList(const char *basePath) {
    DIR *dir;
    int found = -1;
    struct dirent *ptr;
    char filename[1000];

    if ((dir = opendir(basePath)) == NULL) {
        rkfec_err("Open dir error...");
        return found;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            /// current dir OR parrent dir
            continue;
        } else if (ptr->d_type == 8) {
            /// file, check video*/name whether has rkispp_fec info
            if (strcmp(ptr->d_name, "name") == 0) {
                memset(filename, '\0', sizeof(filename));
                strcpy(filename, basePath);
                strcat(filename, "/");
                strcat(filename, ptr->d_name);
                FILE *fp = fopen(filename, "rb");
                if (fp) {
                    char buf[128];
                    const char* ret = fgets(buf, 128, fp);
                    if (ret != NULL) {
                        rkfec_dbg(5, rkfec_debug, "Enumerate sysfs node info: %s", buf);
#ifdef RKFEC_HW_V20
                        if (strstr(buf, "rkfec_offline") != NULL) {
#else
                        if (strstr(buf, "rkispp_fec") != NULL) {
#endif
                            rkfec_info("found rk fec node: %s", buf);
                            found = 1;
                            fclose(fp);
                            break;
                        }
                    } else {
                        rkfec_err("fgets failed");
                    }
                }
                fclose(fp);
            }
        } else if (ptr->d_type == 10) {
            // TODO: nothing
        } else if (ptr->d_type == 4) {
            // TODO: nothing
        }
    }
    closedir(dir);
    return found;
}

void RkIspFecHwMgr::findFecEntry() {
    int found = -1;
    char path[128] = {0};
#ifdef RKFEC_HW_V20
#define MAX_FEC_VIDEO_NUM 1
#else
#define MAX_FEC_VIDEO_NUM 2
#endif

    for (int i = 0; i < SEARCH_MAX_VIDEO_NODES; i++) {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "/sys/class/video4linux/video%d", i);
        if (0 == access(path, F_OK)) {
            found = readFileList(path);
            if (found > 0) {
                sprintf(mFecVdPath[mFecVdNum], "/dev/video%d", i);
                mFecVdNum++;
                if (mFecVdNum == MAX_FEC_VIDEO_NUM) break;
            }
        }
    }

    if (mFecVdNum == 0)
        rkfec_err("not found fec hw !");
    else {
        rkfec_info("found %d hw !", mFecVdNum);
    }

    return;
}

int
RkIspFecHwMgr::selectFecHw()
{
    int fecHw = -1;

    if (mFecVdNum <= 0)
        return fecHw;

    static Profiler prof = {0};
    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    pthread_mutex_lock(&mMutex);

    while (fecHw == -1) {
        for (int i = 0; i < mFecVdNum; i++) {
            if (mFecHw[i] && !mIsFecHwWking[i]) {
                fecHw = i;
                mIsFecHwWking[i] = true;
                break;
            }
        }

        if (fecHw == -1) {
            pthread_cond_wait(&mCond, &mMutex);
        }
    }

    pthread_mutex_unlock(&mMutex);

    rkfec_dbg(3, rkfec_debug, "selectFecHw: %d ", fecHw);

    if (rkfec_debug > 4) rkfec_profiling_end(&prof, "selectFecHw", 100);

    return fecHw;
}

int RkIspFecHwMgr::process(RKFecInOut& param) {
    int fecHw = selectFecHw();
    int ret = -1;

    if (fecHw != -1) {
        ret = mFecHw[fecHw]->process(param);
        if (ret) {
            rkfec_err("process error:%d ", ret);
        }
        ret |= pthread_mutex_lock (&mMutex);
        mIsFecHwWking[fecHw] = false;
        pthread_cond_signal(&mCond);
        ret |= pthread_mutex_unlock (&mMutex);
    } else {
        rkfec_err("no fecHw exsist ");
    }

    return ret;
}

int RkIspFecHwMgr::detach_dma_buffer(int dma_fd) {
    int ret = 0;
    for (int i = 0; i < mFecVdNum; i++) {
        if (mFecHw[i]) {
            int hw_ret = mFecHw[i]->detach_dma_buffer(dma_fd);
            if (hw_ret != 0) {
                rkfec_dbg(5, rkfec_debug, "Failed to detach DMA buffer on FEC HW[%d], errno: %d %s",
                          i, errno, strerror(errno));
                ret = hw_ret;
            }
        }
    }
    return ret;
}

};  // namespace RKISPFEC

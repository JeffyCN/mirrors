
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>

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
    printf("I: %s constructor done !\n", __FUNCTION__);
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
    int ret = pthread_mutex_lock (&mMutex);

    if (!mInstance)
        mgr = new RkIspFecHwMgr();
    else
        mgr = mInstance;

    mRefCnt++;
    ret = pthread_mutex_unlock (&mMutex);

    return mgr;
}

void
RkIspFecHwMgr::deinit()
{
    int ret = pthread_mutex_lock (&mMutex);

    if (mRefCnt > 0 && mRefCnt--) {
        if (mInstance && mRefCnt == 0) {
            delete mInstance;
            mInstance = NULL;
        }
    }

    ret = pthread_mutex_unlock (&mMutex);

    return;
}

int RkIspFecHwMgr::readFileList(const char *basePath) {
    DIR *dir;
    int found = -1;
    struct dirent *ptr;
    char filename[1000];

    if ((dir = opendir(basePath)) == NULL) {
        printf("E: Open dir error...\n");
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
                    // printf("buf=%s\n", buf);
                    if (strstr(buf, "rkispp_fec") != NULL) {
                        printf("found ispp fec node\n");
                        found = 1;
                        fclose(fp);
                        break;
                    }
                    fclose(fp);
                }
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

    for (int i = 0; i < SEARCH_MAX_VIDEO_NODES; i++) {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "/sys/class/video4linux/video%d", i);
        if (0 == access(path, F_OK)) {
            found = readFileList(path);
            if (found > 0) {
                sprintf(mFecVdPath[mFecVdNum], "/dev/video%d", i);
                mFecVdNum++;
                if (mFecVdNum == 2)
                    break;
            }
        }
    }

    if (mFecVdNum == 0)
        printf("E: not found fec hw !\n");
    else {
        printf("I: found %d hw !\n", mFecVdNum);
    }

    return;
}

int
RkIspFecHwMgr::selectFecHw()
{
    int fecHw = -1;

    if (mFecVdNum <= 0)
        return fecHw;

    int ret = pthread_mutex_lock (&mMutex);

    if (mFecHw[0] && !mIsFecHwWking[0]) {
        fecHw = 0;
        mIsFecHwWking[0] = true;
    } else if (mFecHw[1] && !mIsFecHwWking[1]) {
        fecHw = 1;
        mIsFecHwWking[1] = true;
    } else {
        // wait for fecHw0
        ret = pthread_cond_wait (&mCond, &mMutex);
        fecHw = 0;
        mIsFecHwWking[0] = true;
    }
    ret = pthread_mutex_unlock (&mMutex);

    return fecHw;
}

int
RkIspFecHwMgr::process(struct rkispp_fec_in_out& param)
{
    int fecHw = selectFecHw();
    int ret = -1;

    if (fecHw != -1) {
        ret = mFecHw[fecHw]->process(param);
        if (ret) {
            printf("E: process error:%d \n", ret);
        }
        ret |= pthread_mutex_lock (&mMutex);
        mIsFecHwWking[fecHw] = false;
        pthread_cond_broadcast(&mCond);
        ret |= pthread_mutex_unlock (&mMutex);
    } else {
        printf("E: no fecHw exsist \n");
    }

    return ret;
}

};

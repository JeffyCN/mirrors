/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef _CAMDEV_MANAGER_FOR_PU_H
#define _CAMDEV_MANAGER_FOR_PU_H

#include "CameraBuffer.h"

class CamDevManagerForPUImp;

class CamDevManagerForPU {
public:
    CamDevManagerForPU();
    ~CamDevManagerForPU();
    /* add buffer consumer */
    void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    /* remove buffer consumer */
    bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    /* from derived class ICameraBufferOwener */
    bool releaseBufToOwener(weak_ptr<BufferBase> camBuf);
    /* init capture device */
    bool init(const char* dev);
    /* call prepare before start, format and mmap buffer num*/
    bool prepare(frm_info_t& frmFmt, int num);
    /* after this call, will output frames to notifiers */
    bool start(void);
    /* after this call, will stop outputting frames to notifiers */
    void stop(void);
    /* deinit capture device */
    void deinit(void);
private:
    CamDevManagerForPU(const CamDevManagerForPU&);
    CamDevManagerForPU& operator= (const CamDevManagerForPU&);
    shared_ptr<CamDevManagerForPUImp> _instance;
};
#endif

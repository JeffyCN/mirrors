/*
 * smart_buffer_priv.cpp - smart buffer for XCamVideoBuffer
 *
 *  Copyright (c) 2016-2017 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */
#ifndef _SMART_BUFFER_PRIV_H_
#define _SMART_BUFFER_PRIV_H_

#include <xcam_std.h>
#include "xcam_buffer.h"
#include "video_buffer.h"

namespace XCam {

#define SMTBUF_NAME_STR_LEN        (32)

typedef void (*SmartBufferCbNotify_t)
(
    void*                  pUserContext,
    XCamVideoBuffer* pVideoBuffer
);

class SmartBufferPriv
    : public XCamVideoBufferRK
{
public:
    SmartBufferPriv (const SmartPtr<VideoBuffer> &buf);
    SmartBufferPriv (uint32_t frameId, const SmartPtr<VideoBuffer> &buf);
    ~SmartBufferPriv ();

    bool is_valid () const {
        return _buf_ptr.ptr ();
    }

    int registerNotifyCb(SmartBufferCbNotify_t fpCallback, void *pUserContext);

    static void     buf_ref (XCamVideoBuffer *data);
    static uint32_t buf_unref (XCamVideoBuffer *data);
    static uint8_t *buf_map (XCamVideoBuffer *data);
    static void     buf_unmap (XCamVideoBuffer *data);
    static int      buf_get_fd (XCamVideoBuffer *data);
    static void    *buf_get_bo (XCamVideoBufferRK *data);

    SmartPtr<VideoBuffer> get_video_buffer() { return _buf_ptr; }

 private:
    XCAM_DEAD_COPY (SmartBufferPriv);
    mutable RefCount       *_ref;
    SmartPtr<VideoBuffer>   _buf_ptr;
    char nameStr[SMTBUF_NAME_STR_LEN];
};

XCamVideoBuffer *
convert_to_XCamVideoBuffer (const SmartPtr<VideoBuffer> &buf);

}
#endif

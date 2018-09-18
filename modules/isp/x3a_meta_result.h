/*
 * x3a_isp_config.h - 3A ISP config
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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

#ifndef X3A_META_RESULT__H
#define X3A_META_RESULT__H

#include <xcam_std.h>
#include <x3a_result.h>
#include <base/log.h>
#include <base/xcam_3a_result.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <string>

namespace XCam {

#define XCAM_3A_METADATA_RESULT_TYPE (XCAM_3A_RESULT_USER_DEFINED_TYPE + 0x2000)
// medata buffer size : 2k = 64 * 16 + 1024, metadat buffers should be pre allocted and
// reuse for reducing the lag of the allocation time
#define DEFAULT_ENTRY_CAP 64 
#define DEFAULT_DATA_CAP 1024

using namespace android;
class XmetaResult : public X3aResult
{
public:
    XmetaResult (
                 XCamImageProcessType process_type = XCAM_IMAGE_PROCESS_ALWAYS)
        : X3aResult (XCAM_3A_METADATA_RESULT_TYPE, process_type)
          , _meta (NULL)
          , _metadata (NULL)
    {
        _meta = allocate_camera_metadata(DEFAULT_ENTRY_CAP, DEFAULT_DATA_CAP);
        XCAM_ASSERT (_meta);
        _metadata = new CameraMetadata(_meta);
        set_ptr ((void*) _metadata);
    }

    virtual ~XmetaResult () {
        /* free_camera_metadata(_meta); */
        delete _metadata;
        _meta = NULL;
        _metadata = NULL;
    }
    void dump() {

        static unsigned int count = 0;
        count++;
        std::string fileName("/data/dump/");
        fileName += "dump_result_" + std::to_string(count);
        LOGI("%s filename is %s", __FUNCTION__, fileName.data());

        int fd = open(fileName.data(), O_RDWR | O_CREAT, 0666);
        if (fd != -1) {
            _metadata->dump(fd, 2);
        } else {
            LOGE("dumpResult: open failed, errmsg: %s\n", strerror(errno));
        }
        close(fd);
    }

    CameraMetadata* get_metadata_result () {
        return _metadata;
    }

private:
    CameraMetadata *_metadata;
    camera_metadata_t *_meta;
};
};

#endif //X3A_META_RESULT__H


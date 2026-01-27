/*
 * video_buffer.h - video buffer base
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

#ifndef XCAM_VIDEO_BUFFER_H
#define XCAM_VIDEO_BUFFER_H

#include <xcam_std.h>
#include <meta_data.h>
#include <base/xcam_buffer.h>
#include <list>

namespace XCam {

class VideoBuffer;
typedef std::list<SmartPtr<VideoBuffer>>  VideoBufferList;

struct VideoBufferPlanarInfo
    : XCamVideoBufferPlanarInfo
{
    VideoBufferPlanarInfo ();
};

struct VideoBufferInfo
    : XCamVideoBufferInfo
{
    VideoBufferInfo ();
    bool init (
        uint32_t format,
        uint32_t width, uint32_t height,
        uint32_t aligned_width = 0, uint32_t aligned_height = 0, uint32_t size = 0, bool compacted = false);

    bool fill (const XCamVideoBufferInfo &info);

    bool get_planar_info (
        VideoBufferPlanarInfo &planar, const uint32_t index = 0) const;

    bool is_valid () const;
};

class VideoBuffer {
public:
    explicit VideoBuffer (int64_t timestamp = InvalidTimestamp)
        : _buf_type(0), _timestamp(timestamp)
    {}
    explicit VideoBuffer (const VideoBufferInfo &info, int64_t timestamp = InvalidTimestamp)
        : _buf_type(0)
        , _videoinfo (info)
        , _timestamp (timestamp)
    {}
    virtual ~VideoBuffer ();

    virtual uint8_t *map () = 0;
    virtual bool unmap () = 0;
    virtual int get_fd () = 0;

    const VideoBufferInfo & get_video_info () const {
        return _videoinfo;
    }
    int64_t get_timestamp () const {
        return _timestamp;
    }

    void set_video_info (const VideoBufferInfo &info) {
        _videoinfo = info;
    }

    void set_timestamp (int64_t timestamp) {
        _timestamp = timestamp;
    }

    uint32_t get_size () const {
        return _videoinfo.size;
    }

    void set_sequence (uint32_t sequence) {
        _sequence = sequence;
    }

    uint32_t get_sequence () const {
        return _sequence;
    }

    int _buf_type;
private:
    XCAM_DEAD_COPY (VideoBuffer);

private:
    VideoBufferInfo           _videoinfo;
    int64_t                   _timestamp; // in microseconds
    uint32_t                  _sequence{0};
};

XCamVideoBuffer *convert_to_external_buffer (const SmartPtr<VideoBuffer> &buf);

}

#endif //XCAM_VIDEO_BUFFER_H

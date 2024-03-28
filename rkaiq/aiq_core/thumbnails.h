/*
 * thumbnails.h - A service to produce thumbnails for algorithms
 *
 *  Copyright (c) 2021-2022 Rockchip Electronics Co., Ltd.
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
#ifndef AIQ_CORE_THUMBNAILS_H
#define AIQ_CORE_THUMBNAILS_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>

#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_defs.h"
#include "xcore/task_service.h"
#include "xcore/task_traits.h"
#include "xcore/video_buffer.h"

XCAM_BEGIN_DECLARE

#define RAW_DOMAIN_FAKE_NODE_START_BIT 0
#define RAW_DOMAIN_FAKE_NODES_COUNT    8
#define RAW_DOMAIN_START_BIT           RAW_DOMAIN_FAKE_NODE_START_BIT + RAW_DOMAIN_FAKE_NODES_COUNT
#define RAW_DOMAIN_NODES_COUNT         8
#define RGB_DOMAIN_START_BIT           RAW_DOMAIN_START_BIT + RAW_DOMAIN_NODES_COUNT
#define RGB_DOMAIN_NODES_COUNT         8
#define YUV_DOMAIN_START_BIT           RGB_DOMAIN_START_BIT + RGB_DOMAIN_NODES_COUNT
#define YUV_DOMAIN_NODES_COUNT         16

// Hardware independent **hardware** node definition
typedef enum rkaiq_pipeline_node_e {
    // FAKE RAW pipelines
    RKAIQ_PIPELINE_NODE_FAKE_SHORT  = 1ULL << (RAW_DOMAIN_FAKE_NODE_START_BIT + 0),
    RKAIQ_PIPELINE_NODE_FAKE_MIDDLE = 1ULL << (RAW_DOMAIN_FAKE_NODE_START_BIT + 1),
    RKAIQ_PIPELINE_NODE_FAKE_LONG   = 1ULL << (RAW_DOMAIN_FAKE_NODE_START_BIT + 2),
    // Real RAW pipelines
    RKAIQ_PIPELINE_NODE_BLC      = 1ULL << (RAW_DOMAIN_START_BIT + 0),
    RKAIQ_PIPELINE_NODE_DPCC     = 1ULL << (RAW_DOMAIN_START_BIT + 1),
    RKAIQ_PIPELINE_NODE_HDRMERGE = 1ULL << (RAW_DOMAIN_START_BIT + 2),
    RKAIQ_PIPELINE_NODE_RAWNR    = 1ULL << (RAW_DOMAIN_START_BIT + 3),
    RKAIQ_PIPELINE_NODE_LSC      = 1ULL << (RAW_DOMAIN_START_BIT + 4),
    RKAIQ_PIPELINE_NODE_HDRTMO   = 1ULL << (RAW_DOMAIN_START_BIT + 5),
    RKAIQ_PIPELINE_NODE_GIC      = 1ULL << (RAW_DOMAIN_START_BIT + 6),
    // RGB domains
    RKAIQ_PIPELINE_NODE_DEBAYER = 1ULL << (RGB_DOMAIN_START_BIT + 0),
    RKAIQ_PIPELINE_NODE_CCM     = 1ULL << (RGB_DOMAIN_START_BIT + 1),
    RKAIQ_PIPELINE_NODE_GAMMA   = 1ULL << (RGB_DOMAIN_START_BIT + 2),
    RKAIQ_PIPELINE_NODE_WDR     = 1ULL << (RGB_DOMAIN_START_BIT + 3),
    RKAIQ_PIPELINE_NODE_DEHAZE  = 1ULL << (RGB_DOMAIN_START_BIT + 4),
    RKAIQ_PIPELINE_NODE_3DLUT   = 1ULL << (RGB_DOMAIN_START_BIT + 5),
    // YUV domains
    RKAIQ_PIPELINE_NODE_LDCH = 1ULL << (YUV_DOMAIN_START_BIT + 0),
    RKAIQ_PIPELINE_NODE_CSM  = 1ULL << (YUV_DOMAIN_START_BIT + 1),
    RKAIQ_PIPELINE_NODE_CP   = 1ULL << (YUV_DOMAIN_START_BIT + 2),
    RKAIQ_PIPELINE_NODE_IE   = 1ULL << (YUV_DOMAIN_START_BIT + 3),
    // YUV PP domains
    RKAIQ_PIPELINE_NODE_TNR        = 1ULL << (YUV_DOMAIN_START_BIT + 4),
    RKAIQ_PIPELINE_NODE_YNR        = 1ULL << (YUV_DOMAIN_START_BIT + 5),
    RKAIQ_PIPELINE_NODE_UVNR       = 1ULL << (YUV_DOMAIN_START_BIT + 6),
    RKAIQ_PIPELINE_NODE_SHARP      = 1ULL << (YUV_DOMAIN_START_BIT + 7),
    RKAIQ_PIPELINE_NODE_EDGEFILTER = 1ULL << (YUV_DOMAIN_START_BIT + 8),
    RKAIQ_PIPELINE_NODE_ORB        = 1ULL << (YUV_DOMAIN_START_BIT + 9),
    RKAIQ_PIPELINE_NODE_FEC        = 1ULL << (YUV_DOMAIN_START_BIT + 10),
    // ...
} rkaiq_pipeline_node_t;

typedef struct rkaiq_thumbnails_s {
    uint32_t frame_id;
    rkaiq_thumbnails_config_t config;
    XCamVideoBuffer* buffer;
} rkaiq_thumbnails_t;

// For hwi only
typedef enum rkaiq_stream_type_e {
    RKISP_STREAM_NONE,
    RKISP20_STREAM_MIPITX_S,
    RKISP20_STREAM_MIPITX_M,
    RKISP20_STREAM_MIPITX_L,
    RKISP20_STREAM_SP_RAW,
    RKISP20_STREAM_SP,
    RKISP20_STREAM_TNR,
    RKISP20_STREAM_NR,
    RKISP20_STREAM_FEC,
    // RKISP21_SP,
    RKISP_STREAM_MAX,
} rkaiq_stream_type_t;

// For hwi only
typedef struct rkaiq_image_source_s {
    uint32_t frame_id;
    // video streams that produce image sources
    rkaiq_stream_type_t src_type;
    // the full size image info
    XCamVideoBufferInfo full_image_info;
    // The buffer of image source
    // maybe smaller than fullsize image
    XCamVideoBuffer* image_source;
} rkaiq_image_source_t;

XCAM_END_DECLARE

namespace RkCam {

namespace thumbnails {
class ThumbnailsConfig;
class ThumbnailsBufferManager;
struct ScalerParam;
using ScalerService = XCam::TaskService<ScalerParam>;
}  // namespace thumbnails

class ThumbnailsService {
 public:
    ThumbnailsService();
    virtual ~ThumbnailsService();
    ThumbnailsService(const ThumbnailsService&) = delete;
    ThumbnailsService& operator=(const ThumbnailsService&) = delete;

    XCamReturn Prepare(const CalibDbV2_Thumbnails_Param_t* calib);
    XCamReturn Start();
    XCamReturn Stop();

    using ResultCallback = std::function<void(const rkaiq_thumbnails_t& thumbnail)>;

    void SetResultCallback(const ResultCallback& cb);
    void OnFrameEvent(const rkaiq_image_source_t& source);

 private:
    std::shared_ptr<thumbnails::ThumbnailsConfig> config_;
    std::unique_ptr<thumbnails::ThumbnailsBufferManager> bufferManager_;
    std::map<int, std::unique_ptr<thumbnails::ScalerService>> scalers_;
    ResultCallback callback_;
    std::atomic_bool stopped_;
};

}  // namespace RkCam

#endif  // AIQ_CORE_THUMBNAILS_H

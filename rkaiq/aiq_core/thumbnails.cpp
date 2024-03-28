/*
 * thumbnails.cpp - A service to produce thumbnails for algorithms
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
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
#include "thumbnails.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "image_processor.h"
#include "xcore/base/xcam_buffer.h"
#include "xcore/base/xcam_log.h"
#include "xcore/drm_buffer.h"
#include "xcore/drm_device.h"
#include "xcore/video_buffer.h"

namespace RkCam {

using namespace XCam;
using namespace RkCam::thumbnails;

namespace thumbnails {
// clang-format off
// Any bits of this mask is '1' means
// the image stream has to be get from SP device
#define RKISP20_MIPITX_S_NODE_MASK \
    RKAIQ_PIPELINE_NODE_FAKE_SHORT

#define RKISP20_MIPITX_M_NODE_MASK \
    RKAIQ_PIPELINE_NODE_FAKE_MIDDLE

#define RKISP20_MIPITX_L_NODE_MASK \
    RKAIQ_PIPELINE_NODE_FAKE_LONG

#define RKISP20_SP_RAW_NODE_MASK \
    RKAIQ_PIPELINE_NODE_FAKE_SHORT | \
    RKAIQ_PIPELINE_NODE_FAKE_MIDDLE | \
    RKAIQ_PIPELINE_NODE_FAKE_LONG

#define RKISP20_SP_NODE_MASK \
    RKISP20_SP_RAW_NODE_MASK | \
    RKAIQ_PIPELINE_NODE_BLC | \
    RKAIQ_PIPELINE_NODE_DPCC | \
    RKAIQ_PIPELINE_NODE_HDRMERGE | \
    RKAIQ_PIPELINE_NODE_RAWNR | \
    RKAIQ_PIPELINE_NODE_LSC | \
    RKAIQ_PIPELINE_NODE_HDRTMO | \
    RKAIQ_PIPELINE_NODE_GIC | \
    RKAIQ_PIPELINE_NODE_DEBAYER | \
    RKAIQ_PIPELINE_NODE_CCM | \
    RKAIQ_PIPELINE_NODE_GAMMA | \
    RKAIQ_PIPELINE_NODE_WDR | \
    RKAIQ_PIPELINE_NODE_DEHAZE | \
    RKAIQ_PIPELINE_NODE_3DLUT | \
    RKAIQ_PIPELINE_NODE_LDCH | \
    RKAIQ_PIPELINE_NODE_CSM | \
    RKAIQ_PIPELINE_NODE_CP | \
    RKAIQ_PIPELINE_NODE_IE

#define RKISP20_TNR_NODE_MASK \
    RKISP20_SP_NODE_MASK | \
    RKAIQ_PIPELINE_NODE_TNR

#define RKISP20_NR_NODE_MASK \
    RKISP20_TNR_NODE_MASK | \
    RKAIQ_PIPELINE_NODE_UVNR | \
    RKAIQ_PIPELINE_NODE_YNR | \
    RKAIQ_PIPELINE_NODE_SHARP | \
    RKAIQ_PIPELINE_NODE_ORB

#define RKISP20_FEC_NODE_MASK \
    RKISP20_NR_NODE_MASK | \
    RKAIQ_PIPELINE_NODE_FEC
// clang-format on

const std::map<rkaiq_stream_type_t, uint64_t> Isp20DevToMaskMap = {
    {RKISP20_STREAM_MIPITX_S, RKISP20_MIPITX_S_NODE_MASK},
    {RKISP20_STREAM_MIPITX_M, RKISP20_MIPITX_M_NODE_MASK},
    {RKISP20_STREAM_MIPITX_L, RKISP20_MIPITX_L_NODE_MASK},
    {RKISP20_STREAM_SP_RAW, RKISP20_SP_RAW_NODE_MASK},
    {RKISP20_STREAM_SP, RKISP20_SP_NODE_MASK},
    {RKISP20_STREAM_NR, RKISP20_NR_NODE_MASK},
    {RKISP20_STREAM_FEC, RKISP20_FEC_NODE_MASK}};

const std::map<rkaiq_stream_type_t, uint64_t> Isp21DevToMaskMap = {
    {RKISP20_STREAM_MIPITX_S, RKISP20_MIPITX_S_NODE_MASK},
    {RKISP20_STREAM_MIPITX_M, RKISP20_MIPITX_M_NODE_MASK},
    {RKISP20_STREAM_MIPITX_L, RKISP20_MIPITX_L_NODE_MASK},
    {RKISP20_STREAM_SP_RAW, RKISP20_SP_RAW_NODE_MASK},
    {RKISP20_STREAM_SP, RKISP20_SP_NODE_MASK}};

bool ConfigLess(const rkaiq_thumbnails_config_t& lhs, const rkaiq_thumbnails_config_t& rhs) {
    if (lhs.width_intfactor <= rhs.width_intfactor &&
        lhs.height_intfactor <= rhs.height_intfactor) {
        return true;
    } else if (lhs.width_intfactor > rhs.width_intfactor &&
               lhs.height_intfactor > rhs.height_intfactor) {
        return false;
    } else {
        // Unsupported
        XCAM_ASSERT(0);
        return true;
    }
}

bool ConfigEqual(const rkaiq_thumbnails_config_t& lhs, const rkaiq_thumbnails_config_t& rhs) {
    return (lhs.width_intfactor == rhs.width_intfactor &&
            lhs.height_intfactor == rhs.height_intfactor &&
            !memcmp(lhs.format, rhs.format, sizeof(lhs.format)) &&
            lhs.after_nodes == rhs.after_nodes);
}

struct RefCountedVideoBuffer {
    RefCountedVideoBuffer() = delete;

    explicit RefCountedVideoBuffer(XCamVideoBuffer* buffer, bool takeRef = true) {
        XCAM_ASSERT(buffer != nullptr);
        this->buffer = buffer;
        this->buffer->ref(this->buffer);
        auto cnt = this->buffer->unref(this->buffer);
        if (takeRef) {
            this->buffer->ref(this->buffer);
            cnt++;
        }
        LOGV_ANALYZER("%p ref count %d", this->buffer, cnt);
    }

    RefCountedVideoBuffer(const RefCountedVideoBuffer& other) {
        this->buffer = other.buffer;
        this->buffer->ref(this->buffer);
    }

    RefCountedVideoBuffer& operator=(const RefCountedVideoBuffer& other) {
        if (this->buffer != other.buffer) {
            this->buffer->unref(this->buffer);
            this->buffer = other.buffer;
            this->buffer->ref(this->buffer);
        }

        return *this;
    }

    RefCountedVideoBuffer(RefCountedVideoBuffer&& other) {
        this->buffer = other.buffer;
        other.buffer = nullptr;
    }

    RefCountedVideoBuffer& operator=(RefCountedVideoBuffer&& other) {
        if (this->buffer != other.buffer) {
            this->buffer->unref(this->buffer);
            this->buffer = other.buffer;
            // this->buffer->ref(this->buffer);
            // other.buffer->unref(other.buffer);
            other.buffer = nullptr;
        }

        return *this;
    }

    ~RefCountedVideoBuffer() {
        auto cnt = this->buffer->unref(this->buffer);
        LOGV_ANALYZER("%p unref count %d", this->buffer, cnt);
    }

    void importBuffer(XCamVideoBuffer* buffer) {
        if (this->buffer == buffer) {
            return;
        }
        this->buffer->unref(this->buffer);
        this->buffer = buffer;
        this->buffer->ref(this->buffer);
    }

    XCamVideoBuffer* exportBuffer() {
        this->buffer->ref(this->buffer);
        return this->buffer;
    }

    friend bool operator<(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return (lhs.buffer->info.width * lhs.buffer->info.height) <
               (rhs.buffer->info.width * rhs.buffer->info.height);
    }

    friend bool operator>(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return (lhs.buffer->info.width * lhs.buffer->info.height) >
               (rhs.buffer->info.width * rhs.buffer->info.height);
    }

    friend bool operator<=(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return !(lhs > rhs);
    }

    friend bool operator>=(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return !(lhs < rhs);
    }

    friend bool operator!=(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return (lhs < rhs) || (lhs > rhs);
    }

    friend bool operator==(const RefCountedVideoBuffer& lhs, const RefCountedVideoBuffer& rhs) {
        return !(lhs < rhs) && !(lhs > rhs);
    }

    static bool IsValid(XCamVideoBuffer* buffer) {
        // TODO(Cody): buffer should support mem type
        return buffer->get_fd(buffer) >= 0;
    }

    XCamVideoBuffer* buffer;
};

struct ScalerParam {
    std::shared_ptr<RefCountedVideoBuffer> src;
    std::pair<rkaiq_thumbnails_config_t, std::shared_ptr<RefCountedVideoBuffer>> thumbnail;
};

class ThumbnailsConfig {
 public:
    ThumbnailsConfig()                        = default;
    ~ThumbnailsConfig()                       = default;
    ThumbnailsConfig(const ThumbnailsConfig&) = delete;
    ThumbnailsConfig& operator=(const ThumbnailsConfig&) = delete;

    bool ParseRequests(const CalibDbV2_Thumbnails_Param_t* configs);
    void DumpConfig(const rkaiq_thumbnails_config_t& config);
    void DumpConfigs();
    std::vector<rkaiq_stream_type_t> GetEnabledStream();
    const std::vector<rkaiq_thumbnails_config_t> GetStreamConfig(
        const rkaiq_stream_type_t type) const;

 private:
    rkaiq_stream_type_t PipeNodesToStreamType(const rkaiq_thumbnails_config_t& config);
    void GetStreamNodeMask(const rkaiq_stream_type_t type, uint64_t& after_nodes);

    std::unordered_map<int, std::vector<rkaiq_thumbnails_config_t>> stream_configs_;
};

class ThumbnailsBufferManager {
 public:
    ThumbnailsBufferManager(const std::shared_ptr<ThumbnailsConfig>& config);
    ~ThumbnailsBufferManager() = default;

    XCamReturn InitializeBufferPools(const rkaiq_stream_type_t& type,
                                     const XCamVideoBufferInfo& fullImageInfo);
    void ReleasePools();
    XCamVideoBuffer* GetBufferByConfig(const rkaiq_stream_type_t type,
                                       const rkaiq_thumbnails_config_t& config);

 private:
    std::mutex mutex_;
    std::shared_ptr<ThumbnailsConfig> config_;
    std::vector<std::pair<rkaiq_thumbnails_config_t, SmartPtr<XCam::BufferPool>>> pools_;
};

class ScalerTask final : public XCam::ServiceTask<ScalerParam> {
 public:
    ScalerTask() = delete;
    explicit ScalerTask(std::unique_ptr<ImageProcessor> proc);
    ~ScalerTask()                 = default;
    ScalerTask(const ScalerTask&) = delete;
    const ScalerTask& operator=(const ScalerTask&) = delete;

    XCam::TaskResult operator()(XCam::ServiceParam<ScalerParam>& p);

 private:
    std::unique_ptr<ImageProcessor> proc_;
};

void ThumbnailsConfig::GetStreamNodeMask(const rkaiq_stream_type_t type, uint64_t& after_nodes) {
    after_nodes = Isp20DevToMaskMap.at(type);
}

rkaiq_stream_type_t ThumbnailsConfig::PipeNodesToStreamType(
    const rkaiq_thumbnails_config_t& config) {
    rkaiq_stream_type_t type = RKISP_STREAM_NONE;

    for (auto it : Isp20DevToMaskMap) {
        LOGD_ANALYZER("type %d, mask %" PRIx64 "", it.first, it.second);
    }

    for (auto it : Isp20DevToMaskMap) {
        if ((config.after_nodes & it.second) &&
            (config.before_node != (config.before_node & it.second))) {
            type = it.first;
            LOGI_ANALYZER("owner %d matched type %d, before %" PRIx64 " after %" PRIx64 "",
                          config.owner_cookies, type, config.before_node, config.after_nodes);
            break;
        } else {
            continue;
        }
    }

    return type;
}

bool ThumbnailsConfig::ParseRequests(const CalibDbV2_Thumbnails_Param_t* db) {
    XCAM_ASSERT(db != nullptr);

    LOGD_ANALYZER("Dump configs db: ");
    for (uint32_t j = 0; j < db->thumbnail_configs_len; j++) {
        auto config = db->thumbnail_configs[j];
        DumpConfig(config);
    }

    for (uint32_t j = 0; j < db->thumbnail_configs_len; j++) {
        auto config = db->thumbnail_configs[j];
        auto type   = PipeNodesToStreamType(config);
        if (type == RKISP_STREAM_NONE) {
            LOGD_ANALYZER("Cannot find suitable stream for %d nodes after %" PRIx64 " before %" PRIx64 "",
                 config.owner_cookies, config.after_nodes, config.before_node);
            continue;
        }
        if (!stream_configs_.count(static_cast<int>(type))) {
            stream_configs_.emplace(type, std::vector<rkaiq_thumbnails_config_t>());
        }
        config.stream_type = type;
        GetStreamNodeMask(type, config.after_nodes);
        stream_configs_.at(type).push_back(config);
    }

    for (auto& stream_config : stream_configs_) {
        auto& configs = stream_config.second;
        std::sort(configs.begin(), configs.end(), ConfigLess);
        configs.erase(std::unique(configs.begin(), configs.end(), ConfigEqual), configs.end());
    }

    return true;
}

void ThumbnailsConfig::DumpConfig(const rkaiq_thumbnails_config_t& config) {
    LOGD_ANALYZER("type: %u, before: %" PRIx64 ", after: %" PRIx64
                  ", format: %c%c%c%c, w: 1/%u, h: 1/%u, count: %d",
                  config.stream_type, config.before_node, config.after_nodes, config.format[0],
                  config.format[1], config.format[2], config.format[3], config.width_intfactor,
                  config.height_intfactor, config.buffer_count);
}

void ThumbnailsConfig::DumpConfigs() {
    for (auto& stream_config : stream_configs_) {
        LOGD_ANALYZER("Dump stream %d configs:", stream_config.first);
        for (auto config : stream_config.second) {
            LOGD_ANALYZER("type: %u, before: %" PRIx64 ", after: %" PRIx64
                          ", format: %c%c%c%c, w: 1/%u, h: 1/%u, count: %d",
                          config.stream_type, config.before_node, config.after_nodes,
                          config.format[0], config.format[1], config.format[2], config.format[3],
                          config.width_intfactor, config.height_intfactor, config.buffer_count);
        }
    }
}

std::vector<rkaiq_stream_type_t> ThumbnailsConfig::GetEnabledStream() {
    std::vector<rkaiq_stream_type_t> types;

    for (auto stream_config : stream_configs_) {
        types.push_back(static_cast<rkaiq_stream_type_t>(stream_config.first));
    }

    return types;
}

const std::vector<rkaiq_thumbnails_config_t> ThumbnailsConfig::GetStreamConfig(
    const rkaiq_stream_type_t type) const {
    auto config_iter = stream_configs_.find(type);
    if (config_iter != stream_configs_.end()) {
        return config_iter->second;
    } else {
        return {};
    }
}

ThumbnailsBufferManager::ThumbnailsBufferManager(const std::shared_ptr<ThumbnailsConfig>& config)
    : config_(config) {}

XCamReturn ThumbnailsBufferManager::InitializeBufferPools(
    const rkaiq_stream_type_t& type, const XCamVideoBufferInfo& fullImageInfo) {
    std::unique_lock<std::mutex> lock(mutex_);
#if HAS_LIBDRM
    if (!DrmDevice::Available()) {
        LOGE_ANALYZER("drm device is not available!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    auto dev = std::make_shared<DrmDevice>();
    if (dev == nullptr) {
        LOGE_ANALYZER("Failed get drm device");
        return XCAM_RETURN_ERROR_MEM;
    }

    for (auto& config : config_->GetStreamConfig(type)) {
        VideoBufferInfo info;
        info.width          = fullImageInfo.width / config.width_intfactor;
        info.height         = fullImageInfo.height / config.height_intfactor;
#if 0
        info.width          = XCAM_ALIGN_UP(info.width, 2);
        info.height         = XCAM_ALIGN_UP(info.height, 2);
        info.aligned_width  = XCAM_ALIGN_UP(info.width, 16);
        info.aligned_height = XCAM_ALIGN_UP(info.height, 4);
#endif
        info.format =
            rk_fmt_fourcc(config.format[0], config.format[1], config.format[2], config.format[3]);
        info.init(info.format, info.width, info.height, info.aligned_width, info.aligned_height);
        LOGE_ANALYZER("Initialize thumb: wxh: %dx%d, %dx%d", info.width, info.height,
                      info.aligned_width, info.aligned_height);
        auto pool = SmartPtr<DrmBufferPool>(new DrmBufferPool(dev));
        pool->set_video_info(info);
        pool->reserve(config.buffer_count);
        pools_.push_back(std::make_pair(config, std::move(pool)));
    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_MEM;
#endif
}

void ThumbnailsBufferManager::ReleasePools() {
    std::unique_lock<std::mutex> lock(mutex_);
    pools_.clear();
}

XCamVideoBuffer* ThumbnailsBufferManager::GetBufferByConfig(
    const rkaiq_stream_type_t type, const rkaiq_thumbnails_config_t& config) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto poolIt = std::find_if(
        pools_.begin(), pools_.end(),
        [&type, &config](const std::pair<rkaiq_thumbnails_config_t, SmartPtr<BufferPool>>& pool) {
            return ((type == pool.first.stream_type) && ConfigEqual(config, pool.first));
        });
    if (poolIt != pools_.end()) {
        auto& pool = poolIt->second;
        LOGD_ANALYZER("thumbnail pool size %d", pool->get_free_buffer_size());
        if (pool->has_free_buffers()) {
            auto buffer = pool->get_buffer();
            return convert_to_external_buffer(buffer);
        }
    }
    LOGE_ANALYZER("thumbnail cannot find available buffer pool");
    return nullptr;
}

template <typename To, typename From>
To convert(From&);

template <>
img_buffer_t convert(std::shared_ptr<RefCountedVideoBuffer>& dma) {
    auto& info       = dma->buffer->info;
    img_buffer_t buf = {
        NULL, NULL,
        dma->buffer->get_fd(dma->buffer),
        (int)info.width,
        (int)info.height,
        (int)info.aligned_width,
        (int)info.aligned_height,
        static_cast<rk_aiq_format_t>(info.format)
    };

    return buf;
}

ScalerTask::ScalerTask(std::unique_ptr<ImageProcessor> proc) : proc_(std::move(proc)) {}

TaskResult ScalerTask::operator()(ServiceParam<ScalerParam>& p) {
    auto& full       = p.payload->src;
    auto& scaled     = p.payload->thumbnail.second;
    auto& config     = p.payload->thumbnail.first;
    img_buffer_t src = convert<img_buffer_t>(full);
    img_buffer_t dst = convert<img_buffer_t>(scaled);
    auto ret         = proc_->resize(src, dst, 0, 0);
    LOGD_ANALYZER("thumbnail processed id:%d type: %d 1/%dx1/%d %dx%d->%dx%d, result: %d",
                  p.unique_id, config.stream_type, config.width_intfactor, config.height_intfactor,
                  full->buffer->info.width, full->buffer->info.height, scaled->buffer->info.width,
                  scaled->buffer->info.height, ret);
    return ret == XCAM_RETURN_NO_ERROR ? TaskResult::kSuccess : TaskResult::kFailed;
}

}  // namespace thumbnails

ThumbnailsService::ThumbnailsService() : config_(new ThumbnailsConfig()), stopped_(true) {}

ThumbnailsService::~ThumbnailsService() = default;

XCamReturn ThumbnailsService::Prepare(const CalibDbV2_Thumbnails_Param_t* calib) {
    config_->ParseRequests(calib);
    config_->DumpConfigs();

    auto types = config_->GetEnabledStream();
    if (types.size() > 0) {
        bufferManager_ =
            std::unique_ptr<ThumbnailsBufferManager>(new ThumbnailsBufferManager(config_));
        for (auto t : types) {
            std::unique_ptr<ImageProcessor> proc(new ImageProcessor());
            proc->set_operator("rga");
            auto scaler = std::unique_ptr<ScalerService>(new ScalerService(
                std::unique_ptr<ScalerTask>(new ScalerTask(std::move(proc))), false, 0));
            scalers_[static_cast<int>(t)] = std::move(scaler);
            LOGV_ANALYZER("Created scaler for type %d", t);
        }
    } else {
        LOGW_ANALYZER("thumbnail disabled");
        return XCAM_RETURN_BYPASS;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ThumbnailsService::Start() {
    // TODO(Cody): Add get full stream info from HWI
    // Use buffer manager to initialize buffer pool
    if (!stopped_) {
        LOGW_ANALYZER("thumbnail already started");
        return XCAM_RETURN_ERROR_PARAM;
    }
    for (auto t : config_->GetEnabledStream()) {
        XCamVideoBufferInfo fullImageInfo = {
            RK_PIX_FMT_NV12, 16, 2688, 1520, 2688, 1520, XCAM_ALIGN_UP(int(2688*1520*3/2), 8), 2, {0}, {0}};
        bufferManager_->InitializeBufferPools(t, fullImageInfo);
        LOGD_ANALYZER("Initialize buffer for type %d", t);
    }

    for (auto& scaler : scalers_) {
        scaler.second->start();
    }

    stopped_ = false;
    LOGV_ANALYZER("thumbnail started");

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ThumbnailsService::Stop() {
    if (stopped_) {
        LOGW_ANALYZER("thumbnail stopped");
        return XCAM_RETURN_ERROR_PARAM;
    }

    stopped_ = true;

    for (auto& scaler : scalers_) {
        scaler.second->stop();
    }

    bufferManager_->ReleasePools();

    LOGV_ANALYZER("thumbnail stopped");
    return XCAM_RETURN_NO_ERROR;
}

void ThumbnailsService::SetResultCallback(const ResultCallback& cb) { callback_ = cb; }

void ThumbnailsService::OnFrameEvent(const rkaiq_image_source_t& source) {
    if (stopped_ || !scalers_.count(source.src_type)) {
        LOGE_ANALYZER("Unsuported image source type %d or stopped", source.src_type);
        return;
    }

#if 1
    if (source.frame_id == 1) {
        std::string path = "/data/source_";
        path.append(std::to_string(source.frame_id));
        path.append("_");
        path.append(std::to_string(source.image_source->info.width));
        path.append(std::to_string(source.image_source->info.height));
        path.append(".yuv");
        std::ofstream ofs(path, std::ios::binary);
        char* ptr   = reinterpret_cast<char*>(source.image_source->map(source.image_source));
        size_t size = source.image_source->info.size;
        ofs.write(ptr, size);
    }
#endif

    if (RefCountedVideoBuffer::IsValid(source.image_source)) {
        LOGD_ANALYZER(">>>>>> source type %d , w %d h %d", source.src_type, source.image_source->info.width,
             source.image_source->info.height);
        auto src = std::make_shared<RefCountedVideoBuffer>(source.image_source);
        auto& scaler  = scalers_.at(source.src_type);
        auto& configs = config_->GetStreamConfig(source.src_type);
        for (auto& config : configs) {
            auto* buf = bufferManager_->GetBufferByConfig(source.src_type, config);
            if (buf) {
                auto dst = std::make_shared<RefCountedVideoBuffer>(buf, false);
                if (*(src) <= *(dst)) {
                    LOGW_ANALYZER("thumbnail src %dx%d is smaller than or equal to dst %dx%d",
                         src->buffer->info.width, src->buffer->info.height, dst->buffer->info.width,
                         dst->buffer->info.height);
                    continue;
                }
                ServiceParam<ScalerParam> param;
                param.state              = XCam::ParamState::kAllocated;
                param.unique_id          = source.frame_id;
                param.payload            = std::make_shared<ScalerParam>();
                param.payload->src       = src;
                src                      = dst;
                param.payload->thumbnail = std::make_pair(config, dst);
                scaler->enqueue(param);
                LOGI_ANALYZER("thumbnail enqueue id %d type %d 1/%d x 1/%d to scaler",
                              source.frame_id, source.src_type, config.width_intfactor,
                              config.height_intfactor);
            } else {
                LOGE_ANALYZER("Cannot get buffer for config : ");
                config_->DumpConfig(config);
            }
        }
    }

    for (auto& s : scalers_) {
        while (true) {
            auto p = s.second->dequeue();
            if (p.state == ParamState::kProcessedSuccess) {
                auto& config = p.payload->thumbnail.first;
                auto& buf    = p.payload->thumbnail.second;
                LOGD_ANALYZER("thumbnail dequeue id %d type %d 1/%d x 1/%d from scaler",
                              p.unique_id, config.stream_type, config.width_intfactor,
                              config.height_intfactor);
                rkaiq_thumbnails_t result;
                result.config   = config;
                result.frame_id = p.unique_id;
                result.buffer   = buf->exportBuffer();
                callback_(result);
                result.buffer->unref(result.buffer);
            } else {
                break;
            }
        }
    }
}

}  // namespace RkCam

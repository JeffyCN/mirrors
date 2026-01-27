/*
 * image_processor.cpp - 2D Image Process Hardware Implementation
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
#include "image_processor.h"

#include <xcam_log.h>

#include <map>
#include <memory>
#include <string>

#ifdef HAS_LIBRGA
#ifdef HAS_LIBDL
#include <dlfcn.h>
#endif
#include "rga/im2d.h"
#include "rga/rga.h"
#endif

namespace RkCam {

#ifdef HAS_LIBRGA
#define RGA_LIBRARY_NAME "librga.so"

using fp_imresize_t   = int (*)(const rga_buffer_t src, rga_buffer_t dst, double fx, double fy,
                              int interpolation, int sync);
using fp_imcrop_t     = int (*)(const rga_buffer_t src, rga_buffer_t dst, im_rect rect, int sync);
using fp_imcvtcolor_t = int (*)(rga_buffer_t src, rga_buffer_t dst, int sfmt, int dfmt, int mode,
                                int sync);
using fp_imrotate_t   = int (*)(const rga_buffer_t src, rga_buffer_t dst, int rotation, int sync);
using fp_imflip_t     = int (*)(const rga_buffer_t src, rga_buffer_t dst, int mode, int sync);
using fp_imcopy_t     = int (*)(const rga_buffer_t src, rga_buffer_t dst, int sync);
using fp_wrapbuffer_virtualaddr_t  = rga_buffer_t (*)(void* vir_addr, int width, int height,
                                                     int wstride, int hstride, int format);
using fp_wrapbuffer_physicaladdr_t = rga_buffer_t (*)(void* phy_addr, int width, int height,
                                                      int wstride, int hstride, int format);
using fp_wrapbuffer_fd_t = rga_buffer_t (*)(int fd, int width, int height, int wstride, int hstride,
                                            int format);

struct rga_ops_s {
    fp_imresize_t imresize_t;
    fp_imcrop_t imcrop_t;
    fp_imcvtcolor_t imcvtcolor_t;
    fp_imrotate_t imrotate_t;
    fp_imflip_t imflip_t;
    fp_imcopy_t imcopy_t;
    fp_wrapbuffer_virtualaddr_t wrapbuffer_virtualaddr_t;
    fp_wrapbuffer_physicaladdr_t wrapbuffer_physicaladdr_t;
    fp_wrapbuffer_fd_t wrapbuffer_fd_t;
};

class RGAOperator : public ImageOperator {
 public:
    RGAOperator() : ImageOperator("rga")
                  , handle_(NULL) {
        memset(&rga_ops_, 0, sizeof(rga_ops_));
    };

    virtual ~RGAOperator() {
#ifdef HAS_LIBDL
        if (handle_) {
            dlclose(handle_);
        }
#endif
    };

    RGAOperator(const RGAOperator&) = delete;
    RGAOperator& operator=(const RGAOperator&) = delete;

    int WrapFormat(rk_aiq_format_t fmt) {
        std::map<rk_aiq_format_t, int> rga_format_map = {
            {RK_PIX_FMT_Y4, RK_FORMAT_Y4},
            {RK_PIX_FMT_GREY, RK_FORMAT_YCbCr_400},
            {RK_PIX_FMT_NV12, RK_FORMAT_YCbCr_420_SP},
            {RK_PIX_FMT_NV21, RK_FORMAT_YCrCb_420_SP},
            {RK_PIX_FMT_NV16, RK_FORMAT_YCbCr_422_SP},
            {RK_PIX_FMT_NV61, RK_FORMAT_YCrCb_422_SP},
            {RK_PIX_FMT_YVYU, RK_FORMAT_YVYU_422},
            {RK_PIX_FMT_YVU420, RK_FORMAT_YVYU_420},
            {RK_PIX_FMT_YUYV, RK_FORMAT_YUYV_422},
            {RK_PIX_FMT_UYVY, RK_FORMAT_UYVY_422},
        };
        auto it = rga_format_map.find(fmt);
        if (it != rga_format_map.end()) {
            return it->second;
        }
        return -1;
    }

    XCamReturn resolve_symbles(void) {
#ifdef HAS_LIBDL
        char* error;
        handle_ = dlopen(RGA_LIBRARY_NAME, RTLD_LAZY);
        if (!handle_) {
            LOGE("Failed to dlopen librga.so : %s", dlerror());
            return XCAM_RETURN_ERROR_UNKNOWN;
        }
        dlerror();
        rga_ops_.wrapbuffer_virtualaddr_t =
            (fp_wrapbuffer_virtualaddr_t)dlsym(handle_, "wrapbuffer_virtualaddr_t");
        error = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble wrapbuffer_virtualaddr_t: %s", error);
        }

        rga_ops_.wrapbuffer_physicaladdr_t =
            (fp_wrapbuffer_physicaladdr_t)dlsym(handle_, "wrapbuffer_physicaladdr_t");
        error = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble wrapbuffer_physicaladdr_t: %s", error);
        }

        rga_ops_.wrapbuffer_fd_t = (fp_wrapbuffer_fd_t)dlsym(handle_, "wrapbuffer_fd_t");
        error                    = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble wrapbuffer_fd_t : %s", error);
        }

        rga_ops_.imresize_t = (fp_imresize_t)dlsym(handle_, "imresize_t");
        error               = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imreize_t : %s", error);
        }

        rga_ops_.imcrop_t = (fp_imcrop_t)dlsym(handle_, "imcrop_t");
        error             = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imcrop_t: %s", error);
        }

        rga_ops_.imcvtcolor_t = (fp_imcvtcolor_t)dlsym(handle_, "imcvtcolor_t");
        error                 = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imcvtcolor_t : %s", error);
        }

        rga_ops_.imrotate_t = (fp_imrotate_t)dlsym(handle_, "imrotate_t");
        error               = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imrotete_t : %s", error);
        }

        rga_ops_.imflip_t = (fp_imflip_t)dlsym(handle_, "imflip_t");
        error             = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imflip_t : %s", error);
        }

        rga_ops_.imcopy_t = (fp_imcopy_t)dlsym(handle_, "imcopy_t");
        error             = dlerror();
        if (error != NULL) {
            LOGE("Failed to resolve symble imcopy_t : %s", error);
        }
#endif  // HAS_LIBDL

        return XCAM_RETURN_NO_ERROR;
    };

    virtual XCamReturn resize(const img_buffer_t& src, img_buffer_t& dst, double factor_x,
                              double factor_y, int interpolation, int sync) final {
        if (rga_ops_.imresize_t) {
            rga_buffer_t rga_src = rga_ops_.wrapbuffer_fd_t(
                src.fd, src.width, src.height, src.wstride, src.hstride, WrapFormat(src.format));
            rga_buffer_t rga_dst = rga_ops_.wrapbuffer_fd_t(
                dst.fd, dst.width, dst.height, dst.wstride, dst.hstride, WrapFormat(dst.format));

            if (0 > rga_ops_.imresize_t(rga_src, rga_dst, factor_x, factor_y, interpolation, sync)) {
                return XCAM_RETURN_ERROR_FAILED;
            }

            return XCAM_RETURN_NO_ERROR;
        }
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

    virtual XCamReturn crop(const img_buffer_t& src, img_buffer_t& dst, img_rect_t rect,
                            int sync) final {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

    virtual XCamReturn cvtcolor(img_buffer_t& src, img_buffer_t& dst, int sfmt, int dfmt, int mode,
                                int sync) final {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

    virtual XCamReturn rotate(const img_buffer_t& src, img_buffer_t& dst, int rotation,
                              int sync) final {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

    virtual XCamReturn flip(const img_buffer_t& src, img_buffer_t& dst, int mode, int sync) final {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

    virtual XCamReturn copy(const img_buffer_t& src, img_buffer_t& dst, int sync) final {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    };

 private:
    void* handle_;
    struct rga_ops_s rga_ops_;
};
#endif  // HAS_LIBRGA

ImageOperator::ImageOperator(const std::string name) : name_(name) {}

XCamReturn ImageOperator::resize(const img_buffer_t& src, img_buffer_t& dst, double fx, double fy,
                                 int interpolation, int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn ImageOperator::crop(const img_buffer_t& src, img_buffer_t& dst, img_rect_t rect,
                               int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn ImageOperator::cvtcolor(img_buffer_t& src, img_buffer_t& dst, int sfmt, int dfmt,
                                   int mode, int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn ImageOperator::rotate(const img_buffer_t& src, img_buffer_t& dst, int rotation,
                                 int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn ImageOperator::flip(const img_buffer_t& src, img_buffer_t& dst, int mode, int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn ImageOperator::copy(const img_buffer_t& src, img_buffer_t& dst, int sync) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

void ImageProcessor::set_operator(const std::string& name) {
    if (operator_ != nullptr && !name.compare(operator_->get_name())) {
        return;
    }
#ifdef HAS_LIBRGA
#ifdef HAS_LIBDL
    if (!name.compare("rga")) {
        auto ops = std::unique_ptr<RGAOperator>{new RGAOperator};
        ops->resolve_symbles();
        operator_ = std::move(ops);
    }
#else
#warning "Using RGA without dynamic loading is NOT implemented yet!!!"
#endif
#endif
}

XCamReturn ImageProcessor::resize(const img_buffer_t& src, img_buffer_t& dst, double factor_x,
                                  double factor_y) {
    if (operator_) {
        return operator_->resize(src, dst, factor_x, factor_y);
    } else {
        LOGE("%s : Not implemented", __func__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }
}

XCamReturn ImageProcessor::crop(const img_buffer_t& src, img_buffer_t& dst,
                                const img_rect_t& rect) {
    LOGE("%s : Not implemented", __func__);
    return XCAM_RETURN_ERROR_UNKNOWN;
}

}  // namespace RkCam

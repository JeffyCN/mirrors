/*
 * image_processor.h - 2D Image Process Hardware Implementation
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#ifndef _IMAGE_PROC_HW_H_
#define _IMAGE_PROC_HW_H_

#include <memory>
#include <string>

#include "common/rk_aiq_types.h"
#include "xcam_common.h"

namespace RkCam {

typedef struct {
    int x;      /* upper-left x */
    int y;      /* upper-left y */
    int width;  /* width */
    int height; /* height */
} img_rect_t;

typedef struct {
    void* vir_addr;         /* virtual address */
    void* phy_addr;         /* physical address */
    int fd;                 /* shared fd */
    int width;              /* width */
    int height;             /* height */
    int wstride;            /* wstride */
    int hstride;            /* hstride */
    rk_aiq_format_t format; /* format */
} img_buffer_t;

class ImageOperator {
 public:
    ImageOperator(const std::string name);
    ImageOperator(const ImageOperator&) = delete;
    ImageOperator& operator=(const ImageOperator&) = delete;
    virtual ~ImageOperator() = default;

    const std::string get_name() { return name_; };

    virtual XCamReturn crop(const img_buffer_t& src, img_buffer_t& dst, img_rect_t rect,
                            int sync = 1);
    virtual XCamReturn resize(const img_buffer_t& src, img_buffer_t& dst, double fx, double fy,
                              int interpolation = 0, int sync = 1);
    virtual XCamReturn cvtcolor(img_buffer_t& src, img_buffer_t& dst, int sfmt, int dfmt, int mode,
                                int sync = 1);
    virtual XCamReturn rotate(const img_buffer_t& src, img_buffer_t& dst, int rotation,
                              int sync = 1);
    virtual XCamReturn flip(const img_buffer_t& src, img_buffer_t& dst, int mode, int sync = 1);
    virtual XCamReturn copy(const img_buffer_t& src, img_buffer_t& dst, int sync = 1);

 private:
    std::string name_;
};

class ImageProcessor {
 public:
    ImageProcessor()                      = default;
    ImageProcessor(const ImageProcessor&) = delete;
    ImageProcessor& operator=(const ImageProcessor&) = delete;
    ~ImageProcessor()                                = default;

    void set_operator(const std::string& name);

    XCamReturn resize(const img_buffer_t& src, img_buffer_t& dst, double factor_x, double factor_y);
    XCamReturn crop(const img_buffer_t& src, img_buffer_t& dst, const img_rect_t& rect);
    XCamReturn cvtcolor(img_buffer_t& src, img_buffer_t& dst, int sfmt, int dfmt, int mode,
                        int sync = 1);
    XCamReturn rotate(const img_buffer_t& src, img_buffer_t& dst, int rotation, int sync = 1);
    XCamReturn flip(const img_buffer_t& src, img_buffer_t& dst, int mode, int sync = 1);
    XCamReturn copy(const img_buffer_t& src, img_buffer_t& dst, int sync = 1);

 private:
    std::unique_ptr<ImageOperator> operator_;
};

}  // namespace RkCam

#endif  // _IMAGE_PROC_HW_H_

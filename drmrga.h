/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _rk_drm_rga_
#define _rk_drm_rga_

#define DRMRGA_HARDWARE_MODULE_ID "librga"

#include <stdint.h>
#include <sys/cdefs.h>

#include <hardware/gralloc.h>
#include <hardware/hardware.h>
#include <cutils/native_handle.h>


/*****************************************************************************/

/* for compatibility */
#define DRM_RGA_MODULE_API_VERSION      HWC_MODULE_API_VERSION_0_1
#define DRM_RGA_DEVICE_API_VERSION      HWC_DEVICE_API_VERSION_0_1
#define DRM_RGA_API_VERSION             HWC_DEVICE_API_VERSION

#define DRM_RGA_TRANSFORM_ROT_MASK      0x0000000F
#define DRM_RGA_TRANSFORM_ROT_0         0x00000000
#define DRM_RGA_TRANSFORM_ROT_90        0x00000001
#define DRM_RGA_TRANSFORM_ROT_180       0x00000002
#define DRM_RGA_TRANSFORM_ROT_270       0x00000004

#define DRM_RGA_TRANSFORM_FLIP_MASK     0x000000F0
#define DRM_RGA_TRANSFORM_FLIP_H        0x00000020
#define DRM_RGA_TRANSFORM_FLIP_V        0x00000010
/*****************************************************************************/

/*
@value size:     user not need care about.For avoid read/write out of memory
*/
typedef struct rga_rect {
    int xoffset;
    int yoffset;
    int width;
    int height;
    int wstride;
    int format;
    int size;
} rga_rect_t;

typedef struct drm_rga {
    rga_rect_t src;
    rga_rect_t dst;
} drm_rga_t;

typedef struct rga_module {
    /**
     * Common methods of the hardware composer module.  This *must* be the first member of
     * hwc_module as users of this structure will cast a hw_module_t to
     * hwc_module pointer in contexts where it's known the hw_module_t references a
     * hwc_module.
     */
    struct hw_module_t common;
} rga_module_t;

typedef struct rga_device {

    struct hw_device_t common;

    /*
    @fun rgaCopy:copy data from one buffer to another

    @param dev:rga dev handle that user get from open the moudle
    @param src:the source buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param dst:the target buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    */
    int (*rgaCopy)(struct rga_device *dev,
                    buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects);

    /*
    @fun rgaConvert:Convert data from one buffer to another

    @param dev:rga dev handle that user get from open the moudle
    @param src:the source buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param dst:the target buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    */
    int (*rgaConvert)(struct rga_device *dev,
                    buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects);

    /*
    @fun rgaScale:Scale data from one buffer to another

    @param dev:rga dev handle that user get from open the moudle
    @param src:the source buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param dst:the target buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    */
    int (*rgaScale)(struct rga_device *dev,
                    buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects);


    /*
    @fun rgaRotateScale:Rotate and scale data from one buffer to another

    @param dev:rga dev handle that user get from open the moudle
    @param src:the source buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param dst:the target buffer_handle_t which accord to the rects.If rects is null
               than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param rects:
    @param rotation: see system/core/include/system/graphic.h
                DRM_RGA_TRANSFORM_ROT_MASK  
                DRM_RGA_TRANSFORM_ROT_0     
                DRM_RGA_TRANSFORM_ROT_90    
                DRM_RGA_TRANSFORM_ROT_180   
                DRM_RGA_TRANSFORM_ROT_270   

                DRM_RGA_TRANSFORM_FLIP_MASK 
                DRM_RGA_TRANSFORM_FLIP_H    
                DRM_RGA_TRANSFORM_FLIP_V

                DRM_RGA_TRANSFORM_FLIP_H | DRM_RGA_TRANSFORM_ROT_90 is same to 
                                                            DRM_RGA_TRANSFORM_ROT_270

                .
                .
                .
    */
    int (*rgaRotateScale)(struct rga_device *dev,
               buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects,int rotation);

    /*
    @fun rgaFillColor:Fill one buffer to the given data

    @param dev:rga dev handle that user get from open the moudle
    @param handle:the source buffer_handle_t which accord to the rects.If rects is
               null than will accord to the attribute of buffer_handle_t.Of course the
               rect of source/target must small than the stride of buffer_handle_t.
    @param 
    */
    int (*rgaFillColor)(struct rga_device *dev,
                    buffer_handle_t handle,int data,drm_rga_t* rects);

    /*
    @fun rgaQuery:Witch the cmd,user can get some info,for the future like fence async

    @param dev:rga dev handle that user get from open the moudle
    @param cmd:the command the user want to use
    @param 
    */
    int (*rgaQuery)(struct rga_device* dev, int cmd, ...);

    /*
    @fun rgaControl:Witch the cmd,user can control the rga to do something

    @param dev:rga dev handle that user get from open the moudle
    @param cmd:the command the user want to use
    @param 
    */
    int (*rgaControl)(struct rga_device* dev, int cmd, ...);

    /*
    @fun rgaDump:Dump something

    @param dev:rga dev handle that user get from open the moudle
    @param cmd:the command the user want to use
    @param 
    */
    int (*rgaDump)(struct rga_device* dev, char *buff, int buff_len);
} rga_device_t;

static inline int rga_open(const struct hw_module_t* module,
        rga_device_t** device) {
    return module->methods->open(module,
            "RGA Moudle", (struct hw_device_t**)device);
}

static inline int rga_close(rga_device_t* device) {
    return device->common.close(&device->common);
}

/*
@fun rga_set_rect:For use to set the rects esayly

@param rect:The rect user want to set,like setting the src rect:
    drm_rga_t rects;
    rga_set_rect(rects.src,0,0,1920,1080,1920,NV12);
    mean to set the src rect to the value.
*/
static inline int rga_set_rect(rga_rect *rect, int x, int y, int w, int h, int s, int f)
{
    if (!rect)
        return -EINVAL;
    
    rect->xoffset = x;
    rect->yoffset = y;
    rect->width = w;
    rect->height = h;
    rect->wstride = s;
    rect->format = f;

    return 0;
}
/*****************************************************************************/

#endif

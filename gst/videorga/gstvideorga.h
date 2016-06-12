/* GStreamer
 *  Copyright (C) 2016 Intel Corp
 *  Author: Sudip Jain <sudip.jain@intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_VIDEORGA_H__
#define __GST_VIDEORGA_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define RGA_BLIT_SYNC	0x5017
#define RGA_BLIT_ASYNC  0x5018
#define RGA_FLUSH       0x5019
#define RGA_GET_RESULT  0x501a
#define RGA_GET_VERSION 0x501b

#define RGA_REG_CTRL_LEN    0x8
#define RGA_REG_CMD_LEN     0x1c
#define RGA_CMD_BUF_SIZE    0x700


/* RGA process mode enum */
enum
{
    bitblt_mode               = 0x0,
    color_palette_mode        = 0x1,
    color_fill_mode           = 0x2,
    line_point_drawing_mode   = 0x3,
    blur_sharp_filter_mode    = 0x4,
    pre_scaling_mode          = 0x5,
    update_palette_table_mode = 0x6,
    update_patten_buff_mode   = 0x7,
};

enum
{
    rop_enable_mask          = 0x2,
    dither_enable_mask       = 0x8,
    fading_enable_mask       = 0x10,
    PD_enbale_mask           = 0x20,
};

typedef enum
{
    yuv2rgb_mode0            = 0x0,
    yuv2rgb_mode1            = 0x1,
    yuv2rgb_mode2            = 0x2,
} t_rga_yuv2rgb_mode;

enum
{
    color_palette_mode0      = 0x0,
    color_palette_mode1      = 0x1,
    color_palette_mode2      = 0x2,
    color_palette_mode3      = 0x3,
};

/* rga rotate mode */
typedef enum
{
    BB_BYPASS   = 0x0,
    BB_ROTATE   = 0x1,
    BB_X_MIRROR = 0x2,
    BB_Y_MIRROR = 0x3
} t_rga_rotate_mode;

typedef enum
{
    nearby   = 0x0,
    bilinear = 0x1,
    bicubic  = 0x2,
}t_rga_scale_mode;

typedef struct
{
    unsigned int yrgb_addr;
    unsigned int uv_addr;
    unsigned int v_addr;
    unsigned int format;

    unsigned short act_w;
    unsigned short act_h;
    unsigned short x_offset;
    unsigned short y_offset;

    unsigned short vir_w;
    unsigned short vir_h;

    unsigned short endian_mode;
    unsigned short alpha_swap;
}t_rga_img_info;


typedef struct
{
    unsigned short w;
    unsigned short h;
    short x_off;
    short y_off;
}t_mdp_img_act;

typedef struct
{
    unsigned short min;
    unsigned short max;
}t_range;

typedef struct
{
    unsigned short x;
    unsigned short y;
}
t_point;

typedef struct
{
    unsigned short xmin;
    unsigned short xmax;
    unsigned short ymin;
    unsigned short ymax;
} t_rect;

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char res;
}t_rgb;


typedef struct
{
    gchar mmu_en;
    unsigned int base_addr;
    unsigned int mmu_flag;
} t_mmu_info;

typedef struct
{
    short gr_x_a;
    short gr_y_a;
    short gr_x_b;
    short gr_y_b;
    short gr_x_g;
    short gr_y_g;
    short gr_x_r;
    short gr_y_r;
} t_color_fill;

typedef struct
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char res;
} t_fading;


typedef struct line_draw_t
{
    t_point start_point;
    t_point end_point;
    unsigned int   color;
    unsigned int   flag;
    unsigned int   line_width;
} t_line_draw;

struct rga_req {
    guchar render_mode;
    t_rga_img_info src;
    t_rga_img_info dst;
    t_rga_img_info pat;
    unsigned int rop_mask_addr;
    unsigned int LUT_addr;
    t_rect clip;
    int sina;
    int cosa;
    unsigned short alpha_rop_flag;
    unsigned char  scale_mode;
    unsigned int color_key_max;
    unsigned int color_key_min;
    unsigned int fg_color;
    unsigned int bg_color;
    t_color_fill gr_color;
    t_line_draw line_draw_info;
    t_fading fading;
    unsigned char PD_mode;
    unsigned char alpha_global_value;
    unsigned short rop_code;
    unsigned char bsfilter_flag;
    unsigned char palette_mode;
    unsigned char yuv2rgb_mode;
    unsigned char endian_mode;
    unsigned char rotate_mode;
    unsigned char color_fill_mode;
    t_mmu_info mmu_info;
    unsigned char  alpha_rop_mode;
    unsigned char  src_trans_mode;
    unsigned char CMD_fin_int_enable;
    void (*complete)(int retval);
};

typedef enum {
  GST_VIDEO_RGA_MODE_METHOD_MMAP    = 0x0,    /* for mmaped buffers  */
  GST_VIDEO_RGA_MODE_METHOD_DMABUF  = 0x1,    /* for dmabuf memory */
  GST_VIDEO_RGA_MODE_METHOD_USER    = 0x2,    /* user memory */
  GST_VIDEO_RGA_MODE_METHOD_CUSTOM  = 0x3,    /* customized memory */
} GstVideoRgaModeMethod;

typedef enum {
  GST_VIDEO_RGA_ROTATION_NONE = 0x0, /* no rotation*/
  GST_VIDEO_RGA_ROTATION_90R  = 0x1, /* right rotation by 90 deg */
  GST_VIDEO_RGA_ROTATION_180  = 0x2, /* flip horizontally, rotation by 180 deg */
  GST_VIDEO_RGA_ROTATION_90L  = 0x3, /* left rotation by 90 deg*/
} GstVideoRgaFlipMethod;

typedef enum
{
  GST_RGA_VIDEO_FORMAT_RGBA       = 0x0,
  GST_RGA_VIDEO_FORMAT_RGBx       = 0x1,
  GST_RGA_VIDEO_FORMAT_RGB        = 0x2,
  GST_RGA_VIDEO_FORMAT_BGRA       = 0x3,
  GST_RGA_VIDEO_FORMAT_RGB16      = 0x4,
  GST_RGA_VIDEO_FORMAT_RGB555     = 0x5,
  GST_RGA_VIDEO_FORMAT_RGB444     = 0x6,
  GST_RGA_VIDEO_FORMAT_BGR        = 0x7,
  GST_RGA_VIDEO_FORMAT_NV16       = 0x8,
  GST_RGA_VIDEO_FORMAT_YUYV       = 0x9,
  GST_RGA_VIDEO_FORMAT_NV12       = 0xa,
  GST_RGA_VIDEO_FORMAT_I420       = 0xb,
  GST_RGA_VIDEO_FORMAT_NV61       = 0xc,
  GST_RGA_VIDEO_FORMAT_YVYU       = 0xd,
  GST_RGA_VIDEO_FORMAT_NV21       = 0xe,
  GST_RGA_VIDEO_FORMAT_YVU420     = 0xf,
  GST_RGA_VIDEO_FORMAT_BPP1       = 0x10,
  GST_RGA_VIDEO_FORMAT_BPP2       = 0x11,
  GST_RGA_VIDEO_FORMAT_BPP4       = 0x12,
  GST_RGA_VIDEO_FORMAT_BPP8       = 0x13,
  GST_RGA_VIDEO_FORMAT_UNKNOWN    = 0x100,
} GstRgaVideoFormat;

#define GST_TYPE_VIDEO_RGA            (gst_video_rga_get_type())
#define GST_VIDEO_RGA(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEO_RGA,GstVideoRga))
#define GST_VIDEO_RGA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEO_RGA,GstVideoRgaClass))
#define GST_IS_VIDEO_RGA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEO_RGA))
#define GST_IS_VIDEO_RGA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEO_RGA))
#define GST_VIDEO_RGA_CAST(obj)       ((GstVideoRga *)(obj))

typedef struct _GstVideoRgaImageInfo GstVideoRgaImageInfo;
typedef struct _GstVideoRga GstVideoRga;
typedef struct _GstVideoRgaClass GstVideoRgaClass;

struct _GstVideoRgaImageInfo {
    gchar* filename;
    const gchar*  format_name;
    GstRgaVideoFormat format;
    guint width;
    guint height;
    guint stride;
    guint crop_w;
    guint crop_h;
    guint off_x;
    guint off_y;
    guint angle;
    gpointer* base;
    guint size;
    guint8 *data[3]; /*for 3 YUV planes and 1 BGRx... plane. */
};

/**
 * GstVideoRga:
 *
 * Opaque object data structure.
 */
struct _GstVideoRga {
  GstVideoFilter parent;

  //private
  gint fd;
  gint fd_ion;
  struct rga_req req;
  GstVideoRgaImageInfo src_img;
  GstVideoRgaImageInfo dst_img;
  GstVideoRgaFlipMethod flip;
  GstVideoRgaModeMethod mode;
};

struct _GstVideoRgaClass
{
  GstVideoFilterClass parent_class;
};

static GstRgaVideoFormat gst_to_rga_format(const char * );
static void gst_video_rga_flip (GstVideoRga * rga);

G_END_DECLS

#endif /* __GST_VIDEORGA_H__ */

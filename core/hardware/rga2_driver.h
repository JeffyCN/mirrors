/*
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Cerf Yu <cerf.yu@rock-chips.com>
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

#ifndef _RGA2_DRIVER_COMPAT_IOCTL_H_
#define _RGA2_DRIVER_COMPAT_IOCTL_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct rga2_img_info_t {
#if defined(__arm64__) || defined(__aarch64__)
    unsigned long yrgb_addr;      /* yrgb    mem addr         */
    unsigned long uv_addr;        /* cb/cr   mem addr         */
    unsigned long v_addr;         /* cr      mem addr         */
#else
    unsigned int yrgb_addr;      /* yrgb    mem addr         */
    unsigned int uv_addr;        /* cb/cr   mem addr         */
    unsigned int v_addr;         /* cr      mem addr         */
#endif
    unsigned int format;         //definition by RK_FORMAT
    unsigned short act_w;
    unsigned short act_h;
    unsigned short x_offset;
    unsigned short y_offset;

    unsigned short vir_w;
    unsigned short vir_h;

    unsigned short endian_mode; //for BPP
    unsigned short alpha_swap;
} rga2_img_info_t;

typedef struct rga2_rect {
    unsigned short xmin;
    unsigned short xmax; // width - 1
    unsigned short ymin;
    unsigned short ymax; // height - 1
} rga2_rect_t;

typedef struct rga2_color_fill {
    short gr_x_a;
    short gr_y_a;
    short gr_x_b;
    short gr_y_b;
    short gr_x_g;
    short gr_y_g;
    short gr_x_r;
    short gr_y_r;

    //u8  cp_gr_saturation;
} rga2_color_fill_t;

typedef struct rga2_point {
    unsigned short x;
    unsigned short y;
} rga2_point_t;

typedef struct rga2_line_draw {
    rga2_point_t start_point;           /* LineDraw_start_point                */
    rga2_point_t end_point;             /* LineDraw_end_point                  */
    unsigned int   color;               /* LineDraw_color                      */
    unsigned int   flag;                /* (enum) LineDrawing mode sel         */
    unsigned int   line_width;          /* range 1~16 */
} rga2_line_draw_t;

typedef struct rga2_fading {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char res;
} rga2_fading_t;

typedef struct rga2_mmu {
    unsigned char mmu_en;
#if defined(__arm64__) || defined(__aarch64__)
    unsigned long base_addr;
#else
    unsigned int base_addr;
#endif
    unsigned int mmu_flag;     /* [0] mmu enable [1] src_flush [2] dst_flush [3] CMD_flush [4~5] page size*/
} rga2_mmu_t;

/* color space convert coefficient. */
typedef struct rga2_csc_coe {
    int16_t r_v;
    int16_t g_y;
    int16_t b_u;
    int32_t off;
} rga2_csc_coe_t;

typedef struct rga2_full_csc {
    unsigned char flag;
    rga2_csc_coe_t coe_y;
    rga2_csc_coe_t coe_u;
    rga2_csc_coe_t coe_v;
} rga2_full_csc_t;

struct rga2_req {
    unsigned char render_mode;            /* (enum) process mode sel */

    rga2_img_info_t src;                   /* src image info */
    rga2_img_info_t dst;                   /* dst image info */
    rga2_img_info_t pat;                   /* patten image info */

#if defined(__arm64__) || defined(__aarch64__)
    unsigned long rop_mask_addr;          /* rop4 mask addr */
    unsigned long LUT_addr;               /* LUT addr */
#else
    unsigned int rop_mask_addr;           /* rop4 mask addr */
    unsigned int LUT_addr;                /* LUT addr */
#endif

    rga2_rect_t clip;                     /* dst clip window default value is dst_vir */
                                          /* value from [0, w-1] / [0, h-1]*/

    int sina;                             /* dst angle  default value 0  16.16 scan from table */
    int cosa;                             /* dst angle  default value 0  16.16 scan from table */

    unsigned short alpha_rop_flag;        /* alpha rop process flag           */
                                          /* ([0] = 1 alpha_rop_enable)       */
                                          /* ([1] = 1 rop enable)             */
                                          /* ([2] = 1 fading_enable)          */
                                          /* ([3] = 1 PD_enable)              */
                                          /* ([4] = 1 alpha cal_mode_sel)     */
                                          /* ([5] = 1 dither_enable)          */
                                          /* ([6] = 1 gradient fill mode sel) */
                                          /* ([7] = 1 AA_enable)              */
                                          /* ([8] = 1 nn_quantize)            */
                                          /* ([9] = 1 Real color mode)        */

    unsigned char  scale_mode;            /* 0 nearst / 1 bilnear / 2 bicubic */

    unsigned int color_key_max;           /* color key max */
    unsigned int color_key_min;           /* color key min */

    unsigned int fg_color;                /* foreground color */
    unsigned int bg_color;                /* background color */

    rga2_color_fill_t gr_color;           /* color fill use gradient */

    rga2_line_draw_t line_draw_info;

    rga2_fading_t fading;

    unsigned char PD_mode;                /* porter duff alpha mode sel */

    unsigned char alpha_global_value;     /* global alpha value */

    unsigned short rop_code;              /* rop2/3/4 code  scan from rop code table*/

    unsigned char bsfilter_flag;          /* [2] 0 blur 1 sharp / [1:0] filter_type*/

    unsigned char palette_mode;           /* (enum) color palatte  0/1bpp, 1/2bpp 2/4bpp 3/8bpp*/

    unsigned char yuv2rgb_mode;           /* (enum) BT.601 MPEG / BT.601 JPEG / BT.709  */

    unsigned char endian_mode;            /* 0/big endian 1/little endian*/

    unsigned char rotate_mode;            /* (enum) rotate mode  */
                                          /* 0x0,     no rotate  */
                                          /* 0x1,     rotate     */
                                          /* 0x2,     x_mirror   */
                                          /* 0x3,     y_mirror   */

    unsigned char color_fill_mode;        /* 0 solid color / 1 patten color */

    rga2_mmu_t mmu_info;                  /* mmu information */

    unsigned char  alpha_rop_mode;        /* ([0~1] alpha mode)       */
                                          /* ([2~3] rop   mode)       */
                                          /* ([4]   zero  mode en)    */
                                          /* ([5]   dst   alpha mode) (RGA1) */

    unsigned char  src_trans_mode;

    unsigned char  dither_mode;

    rga2_full_csc_t full_csc;            /* full color space convert */

    unsigned char CMD_fin_int_enable;

    /* completion is reported through a callback */
    void (*complete)(int retval);
};

#ifdef __cplusplus
}
#endif

#endif /* _RGA2_DRIVER_COMPATIBLE_IOCTL_H_ */
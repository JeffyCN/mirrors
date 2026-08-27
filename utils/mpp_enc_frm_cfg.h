/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2026 Rockchip Electronics Co., Ltd.
 */

#ifndef MPP_ENC_FRM_CFG_H
#define MPP_ENC_FRM_CFG_H

#include "mpp_cfg_io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *MppEncFrmCfgObj;

/*
 * Fixed-point ratio for coordinate fields.
 *
 * Coordinate fields (MppEncFrmRoi.x/y/w/h and MppEncFrmOsd.lt_x/lt_y/
 * rb_x/rb_y) use Q16 fixed-point ratio instead of absolute pixel values:
 *   0      = top / left edge
 *   COORD  = full width / height  (65536 = 2^16)
 *
 * Runtime conversion to pixel:  px = (val * width) >> 16
 *
 * Q16 (2^16 divisor) is used because the conversion is one multiply + one
 * shift, no division. Max intermediate value val * width = 2^16 * 2^14
 * (16K picture) = 2^30, safe in 32-bit signed arithmetic.
 *
 * Rationale: config is decoupled from frame resolution, one JSON/entry works
 * for any picture size, and the value can never exceed the frame (no
 * out-of-range ROI/OSD on small pictures).
 *
 * Non-coordinate fields (intra/quality/abs_qp_en, stride/fmt/enable, etc.)
 * keep their original semantics.
 */
#define MPP_ENC_FRM_CFG_COORD_MAX       (1 << 16)
#define MPP_ENC_FRM_ROI_MAX_NUM         8
#define MPP_ENC_FRM_OSD_MAX_NUM         8
#define MPP_ENC_FRM_JPEG_ROI_MAX_NUM    16

typedef struct MppEncFrmRoi_t {
    rk_s32  x;              /* Q16 ratio: left edge position */
    rk_s32  y;              /* Q16 ratio: top edge position */
    rk_s32  w;              /* Q16 ratio: region width */
    rk_s32  h;              /* Q16 ratio: region height */
    rk_s32  intra;
    rk_s32  quality;
    rk_s32  abs_qp_en;
} MppEncFrmRoi;

typedef struct MppEncFrmJpegRoi_t {
    rk_s32  x;              /* Q16 ratio: left edge position */
    rk_s32  y;              /* Q16 ratio: top edge position */
    rk_s32  w;              /* Q16 ratio: region width */
    rk_s32  h;              /* Q16 ratio: region height */
    rk_s32  level;
    rk_s32  roi_en;
} MppEncFrmJpegRoi;

typedef struct MppEncFrmOsd_t {
    rk_u32  enable;
    rk_u32  range_trns_en;
    rk_u32  range_trns_sel;
    rk_u32  fmt;
    rk_u32  rbuv_swap;
    rk_u32  lt_x;           /* Q16 ratio: left top corner x */
    rk_u32  lt_y;           /* Q16 ratio: left top corner y */
    rk_u32  rb_x;           /* Q16 ratio: right bottom corner x */
    rk_u32  rb_y;           /* Q16 ratio: right bottom corner y */
    rk_u32  stride;
    rk_u32  ch_ds_mode;
    rk_u32  osd_endn;
} MppEncFrmOsd;

/* Per-frame entry — VLA capable */
typedef struct MppEncFrmCfg_t {
    rk_s32  frame_idx;
    rk_s32  repeat;
    rk_s32  input_idr_req;
    rk_s32  input_pskip;
    rk_s32  input_pskip_non_ref;
    rk_s32  input_pskip_num;
    rk_s32  enc_mark_ltr;
    rk_s32  enc_use_ltr;
    rk_s32  enc_frame_qp;
    rk_s32  enc_base_layer_pid;
    rk_s32  temporal_id;
    rk_u32  userdata;
    rk_u32  userdatas;

    rk_u32  roi_cnt;
    rk_u32  roi_off;
    rk_s32  new_roi_cap;
    rk_s32  roi_cap;

    rk_u32  jpeg_roi_cnt;
    rk_u32  jpeg_roi_off;
    rk_s32  new_jpeg_roi_cap;
    rk_s32  jpeg_roi_cap;
    rk_u32  jpeg_non_roi_level;
    rk_u32  jpeg_non_roi_en;

    rk_u32  osd_cnt;
    rk_u32  osd_off;
    rk_s32  new_osd_cap;
    rk_s32  osd_cap;

    rk_u8  *ud_uuid;
    rk_u8   *ud_buf;
    rk_u32  ud_buf_size;
} MppEncFrmCfg;

#define MPP_ENC_FRM_ROI_ARR(e)  ((MppEncFrmRoi *)((rk_u8 *)(e) + (e)->roi_off))
#define MPP_ENC_FRM_OSD_ARR(e)  ((MppEncFrmOsd *)((rk_u8 *)(e) + (e)->osd_off))
#define MPP_ENC_FRM_JPEG_ROI_ARR(e) ((MppEncFrmJpegRoi *)((rk_u8 *)(e) + (e)->jpeg_roi_off))

typedef struct MppEncFrmCfgSet_t {
    rk_u32              count;
    const MppEncFrmCfg  **entries;
} MppEncFrmCfgSet;

rk_s32 mpp_enc_frm_cfg_get(MppEncFrmCfgObj *obj);
rk_s32 mpp_enc_frm_cfg_put(MppEncFrmCfgObj obj);

MPP_RET mpp_enc_frm_cfg_apply(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, char *buf);
MPP_RET mpp_enc_frm_cfg_apply_file(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, const char *path);
MPP_RET mpp_enc_frm_cfg_extract(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, char **buf);

const MppEncFrmCfg *mpp_enc_frm_cfg_get_entry(MppEncFrmCfgObj obj);
const MppEncFrmCfg *mpp_enc_frm_cfg_lookup(const MppEncFrmCfgSet *cfgs, rk_s32 frame_idx);

extern const MppEncFrmCfgSet mpp_enc_test_frm_cfg;

#ifdef __cplusplus
}
#endif

#endif

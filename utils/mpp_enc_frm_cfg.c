/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2026 Rockchip Electronics Co., Ltd.
 */

#include <string.h>

#include "mpp_log.h"
#include "mpp_common.h"

#include "kmpp_obj.h"
#include "utils_singleton.h"
#include "mpp_enc_frm_cfg.h"

static rk_s32 mpp_enc_frm_cfg_impl_init(void *entry, KmppObj obj, const char *caller)
{
    MppEncFrmCfg *e = (MppEncFrmCfg *)entry;

    e->repeat = -1;
    e->input_idr_req = -1;
    e->input_pskip = -1;
    e->input_pskip_non_ref = -1;
    e->input_pskip_num = -1;
    e->enc_mark_ltr = -1;
    e->enc_use_ltr = -1;
    e->enc_frame_qp = -1;
    e->enc_base_layer_pid = -1;
    e->temporal_id = -1;

    (void)obj;
    (void)caller;
    return rk_ok;
}

static rk_s32 mpp_enc_frm_cfg_impl_resize(void *entry, KmppObj obj,
                                          const char *caller)
{
    MppEncFrmCfg *e = (MppEncFrmCfg *)entry;
    KmppObjDef def = kmpp_obj_to_objdef(obj);
    rk_u32 old_osd_off = e->osd_off;
    rk_u32 old_jpeg_roi_off = e->jpeg_roi_off;
    rk_s32 old_osd_cap = e->osd_cap;
    rk_s32 old_jpeg_roi_cap = e->jpeg_roi_cap;
    rk_s32 data_off;

    (void)caller;
    data_off = kmpp_objdef_get_entry_size(def) + kmpp_obj_to_flags_size(obj);
    e->roi_off = data_off;
    e->osd_off = data_off + e->new_roi_cap * sizeof(MppEncFrmRoi);
    e->jpeg_roi_off = e->osd_off + e->new_osd_cap * sizeof(MppEncFrmOsd);

    if (old_jpeg_roi_off && e->jpeg_roi_off != old_jpeg_roi_off &&
        e->jpeg_roi_cnt > 0) {
        rk_s32 move_cnt = MPP_MIN3((rk_s32)e->jpeg_roi_cnt,
                                   e->new_jpeg_roi_cap,
                                   old_jpeg_roi_cap);

        if (move_cnt > 0)
            memmove((char *)e + e->jpeg_roi_off,
                    (char *)e + old_jpeg_roi_off,
                    move_cnt * sizeof(MppEncFrmJpegRoi));
    }
    if (old_osd_off && e->osd_off != old_osd_off && e->osd_cnt > 0) {
        rk_s32 move_cnt = MPP_MIN3((rk_s32)e->osd_cnt,
                                   e->new_osd_cap, old_osd_cap);

        if (move_cnt > 0)
            memmove((char *)e + e->osd_off, (char *)e + old_osd_off,
                    move_cnt * sizeof(MppEncFrmOsd));
    }
    e->roi_cap = e->new_roi_cap;
    e->osd_cap = e->new_osd_cap;
    e->jpeg_roi_cap = e->new_jpeg_roi_cap;

    return rk_ok;
}

#define MPP_ENC_FRM_CFG_ENTRY_TABLE(prefix, ENTRY, STRCT, EHOOK, SHOOK, ALIAS) \
    CFG_DEF_START() \
    ENTRY(prefix, s32,  rk_s32, frame_idx,           FLAG_NONE, frame_idx) \
    ENTRY(prefix, s32,  rk_s32, repeat,              FLAG_NONE, repeat) \
    ENTRY(prefix, s32,  rk_s32, input_idr_req,       FLAG_NONE, input_idr_req) \
    ENTRY(prefix, s32,  rk_s32, input_pskip,         FLAG_NONE, input_pskip) \
    ENTRY(prefix, s32,  rk_s32, input_pskip_non_ref, FLAG_NONE, input_pskip_non_ref) \
    ENTRY(prefix, s32,  rk_s32, input_pskip_num,     FLAG_NONE, input_pskip_num) \
    ENTRY(prefix, s32,  rk_s32, enc_mark_ltr,        FLAG_NONE, enc_mark_ltr) \
    ENTRY(prefix, s32,  rk_s32, enc_use_ltr,         FLAG_NONE, enc_use_ltr) \
    ENTRY(prefix, s32,  rk_s32, enc_frame_qp,        FLAG_NONE, enc_frame_qp) \
    ENTRY(prefix, s32,  rk_s32, enc_base_layer_pid,  FLAG_NONE, enc_base_layer_pid) \
    ENTRY(prefix, s32,  rk_s32, temporal_id,         FLAG_NONE, temporal_id) \
    ENTRY(prefix, u32,  rk_u32, userdata,            FLAG_NONE, userdata) \
    ENTRY(prefix, u32,  rk_u32, userdatas,           FLAG_NONE, userdatas) \
    ARRAY_START_FLEX_CNT_OFF(roi, MppEncFrmRoi, FLAG_NONE, roi_cnt, roi_off) \
    ARRAY_ENTRY(s32,  x,                    FLAG_NONE,  x) \
    ARRAY_ENTRY(s32,  y,                    FLAG_NONE,  y) \
    ARRAY_ENTRY(s32,  w,                    FLAG_NONE,  w) \
    ARRAY_ENTRY(s32,  h,                    FLAG_NONE,  h) \
    ARRAY_ENTRY(s32,  intra,                FLAG_NONE,  intra) \
    ARRAY_ENTRY(s32,  quality,              FLAG_NONE,  quality) \
    ARRAY_ENTRY(s32,  abs_qp_en,            FLAG_NONE,  abs_qp_en) \
    ARRAY_END(roi) \
    ENTRY(prefix, u32,  rk_u32, jpeg_non_roi_level, FLAG_NONE, jpeg_non_roi_level) \
    ENTRY(prefix, u32,  rk_u32, jpeg_non_roi_en,    FLAG_NONE, jpeg_non_roi_en) \
    ARRAY_START_FLEX_CNT_OFF(jpeg_roi, MppEncFrmJpegRoi, FLAG_NONE, jpeg_roi_cnt, jpeg_roi_off) \
    ARRAY_ENTRY(s32,  x,                    FLAG_NONE,  x) \
    ARRAY_ENTRY(s32,  y,                    FLAG_NONE,  y) \
    ARRAY_ENTRY(s32,  w,                    FLAG_NONE,  w) \
    ARRAY_ENTRY(s32,  h,                    FLAG_NONE,  h) \
    ARRAY_ENTRY(s32,  level,                FLAG_NONE,  level) \
    ARRAY_ENTRY(s32,  roi_en,               FLAG_NONE,  roi_en) \
    ARRAY_END(jpeg_roi) \
    ARRAY_START_FLEX_CNT_OFF(osd, MppEncFrmOsd, FLAG_NONE, osd_cnt, osd_off) \
    ARRAY_ENTRY(u32,  enable,               FLAG_NONE,  enable) \
    ARRAY_ENTRY(u32,  range_trns_en,        FLAG_NONE,  range_trns_en) \
    ARRAY_ENTRY(u32,  range_trns_sel,       FLAG_NONE,  range_trns_sel) \
    ARRAY_ENTRY(u32,  fmt,                  FLAG_NONE,  fmt) \
    ARRAY_ENTRY(u32,  rbuv_swap,            FLAG_NONE,  rbuv_swap) \
    ARRAY_ENTRY(u32,  lt_x,                 FLAG_NONE,  lt_x) \
    ARRAY_ENTRY(u32,  lt_y,                 FLAG_NONE,  lt_y) \
    ARRAY_ENTRY(u32,  rb_x,                 FLAG_NONE,  rb_x) \
    ARRAY_ENTRY(u32,  rb_y,                 FLAG_NONE,  rb_y) \
    ARRAY_ENTRY(u32,  stride,               FLAG_NONE,  stride) \
    ARRAY_ENTRY(u32,  ch_ds_mode,           FLAG_NONE,  ch_ds_mode) \
    ARRAY_ENTRY(u32,  osd_endn,             FLAG_NONE,  osd_endn) \
    ARRAY_END(osd) \
    CFG_DEF_END()

#define KMPP_OBJ_NAME               mpp_enc_frm_cfg
#define KMPP_OBJ_INTF_TYPE          MppEncFrmCfgObj
#define KMPP_OBJ_IMPL_TYPE          MppEncFrmCfg
#define KMPP_OBJ_SGLN               UTILS_SINGLETON
#define KMPP_OBJ_SGLN_ID            UTILS_SGLN_ENC_FRM_CFG
#define KMPP_OBJ_FUNC_INIT          mpp_enc_frm_cfg_impl_init
#define KMPP_OBJ_FUNC_RESIZE        mpp_enc_frm_cfg_impl_resize
#define KMPP_OBJ_ENTRY_TABLE        MPP_ENC_FRM_CFG_ENTRY_TABLE
#define KMPP_OBJ_HIERARCHY_ENABLE
#define KMPP_OBJ_FLEX_ENTRY_ENABLE
#include "kmpp_obj_helper.h"

static rk_s32 mpp_enc_frm_cfg_resize(MppEncFrmCfg *entry, KmppObj obj, rk_s32 roi_cnt,
                                     rk_s32 osd_cnt, rk_s32 jpeg_roi_cnt,
                                     const char *caller)
{
    entry->new_roi_cap = MPP_MAX(roi_cnt, entry->roi_cap);
    entry->new_osd_cap = MPP_MAX(osd_cnt, entry->osd_cap);
    entry->new_jpeg_roi_cap = MPP_MAX(jpeg_roi_cnt, entry->jpeg_roi_cap);

    return kmpp_obj_resize(obj,
                           entry->new_roi_cap * sizeof(MppEncFrmRoi) +
                           entry->new_osd_cap * sizeof(MppEncFrmOsd) +
                           entry->new_jpeg_roi_cap * sizeof(MppEncFrmJpegRoi), caller);
}

static MPP_RET mpp_enc_frm_cfg_apply_obj(MppEncFrmCfgObj obj, MppCfgObj tree, MppCfgStrFmt fmt)
{
    MppEncFrmCfg *entry = kmpp_obj_to_entry(obj);
    MppCfgObj root;
    MPP_RET ret = MPP_NOK;

    root = kmpp_objdef_get_cfg_root(mpp_enc_frm_cfg_objdef());

    /* read VLA counts from parsed tree before to_struct */
    {
        static char roi_cnt_name[] = "roi_cnt";
        static char osd_cnt_name[] = "osd_cnt";
        static char jpeg_roi_cnt_name[] = "jpeg_roi_cnt";
        MppCfgObj node = NULL;
        MppCfgVal val = { 0 };
        rk_s32 roi_cnt = 0;
        rk_s32 osd_cnt = 0;
        rk_s32 jpeg_roi_cnt = 0;

        if (mpp_cfg_find(&node, tree, roi_cnt_name, fmt) == 0 && node) {
            if (mpp_cfg_get_val(node, MPP_CFG_TYPE_s32, &val) == 0)
                roi_cnt = val.s32;
        }
        node = NULL;
        val.s32 = 0;
        if (mpp_cfg_find(&node, tree, osd_cnt_name, fmt) == 0 && node) {
            if (mpp_cfg_get_val(node, MPP_CFG_TYPE_s32, &val) == 0)
                osd_cnt = val.s32;
        }
        node = NULL;
        val.s32 = 0;
        if (mpp_cfg_find(&node, tree, jpeg_roi_cnt_name, fmt) == 0 && node) {
            if (mpp_cfg_get_val(node, MPP_CFG_TYPE_s32, &val) == 0)
                jpeg_roi_cnt = val.s32;
        }

        if (roi_cnt < 0 || roi_cnt > MPP_ENC_FRM_ROI_MAX_NUM ||
            osd_cnt < 0 || osd_cnt > MPP_ENC_FRM_OSD_MAX_NUM ||
            jpeg_roi_cnt < 0 || jpeg_roi_cnt > MPP_ENC_FRM_JPEG_ROI_MAX_NUM) {
            mpp_loge_f("invalid array count roi %d osd %d jpeg_roi %d\n",
                       roi_cnt, osd_cnt, jpeg_roi_cnt);
            goto done;
        }

        /* resize VLA if needed */
        if (roi_cnt > entry->roi_cap || osd_cnt > entry->osd_cap ||
            jpeg_roi_cnt > entry->jpeg_roi_cap) {
            if (mpp_enc_frm_cfg_resize(entry, obj, roi_cnt, osd_cnt, jpeg_roi_cnt,
                                       __FUNCTION__)) {
                mpp_loge_f("resize to roi %d osd %d jpeg_roi %d failed\n",
                           roi_cnt, osd_cnt, jpeg_roi_cnt);
                goto done;
            }
            entry = kmpp_obj_to_entry(obj);
        }
    }

    if (mpp_cfg_to_struct(tree, root, entry)) {
        mpp_loge_f("failed to convert config to struct\n");
        goto done;
    }
    ret = MPP_OK;

done:
    mpp_cfg_put_all(tree);
    return ret;
}

MPP_RET mpp_enc_frm_cfg_apply(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, char *buf)
{
    MppCfgObj tree = NULL;

    if (!obj || !buf)
        return MPP_ERR_NULL_PTR;

    if (mpp_cfg_from_string(&tree, fmt, buf) || !tree) {
        mpp_cfg_put_all(tree);
        return MPP_NOK;
    }

    return mpp_enc_frm_cfg_apply_obj(obj, tree, fmt);
}

MPP_RET mpp_enc_frm_cfg_apply_file(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, const char *path)
{
    MppCfgObj tree = NULL;

    if (!obj || !path)
        return MPP_ERR_NULL_PTR;

    if (mpp_cfg_from_file(&tree, fmt, path) || !tree) {
        mpp_cfg_put_all(tree);
        return MPP_NOK;
    }

    return mpp_enc_frm_cfg_apply_obj(obj, tree, fmt);
}

MPP_RET mpp_enc_frm_cfg_extract(MppEncFrmCfgObj obj, MppCfgStrFmt fmt, char **buf)
{
    MppEncFrmCfg *entry = kmpp_obj_to_entry(obj);
    MppCfgObj tree = NULL;
    MppCfgObj root;

    if (!obj || !buf)
        return MPP_ERR_NULL_PTR;

    *buf = NULL;
    root = kmpp_objdef_get_cfg_root(mpp_enc_frm_cfg_objdef());
    if (mpp_cfg_from_struct(&tree, root, entry) || !tree)
        return MPP_NOK;

    mpp_cfg_to_string(tree, fmt, buf);
    mpp_cfg_put_all(tree);

    return *buf ? MPP_OK : MPP_NOK;
}

const MppEncFrmCfg *mpp_enc_frm_cfg_get_entry(MppEncFrmCfgObj obj)
{
    if (!obj)
        return NULL;
    return kmpp_obj_to_entry(obj);
}

const MppEncFrmCfg *mpp_enc_frm_cfg_lookup(const MppEncFrmCfgSet *cfgs, rk_s32 frame_idx)
{
    rk_u32 i;

    if (!cfgs || !cfgs->entries || !cfgs->count)
        return NULL;

    for (i = 0; i < cfgs->count; i++) {
        const MppEncFrmCfg *e = cfgs->entries[i];

        if (!e)
            continue;
        if (e->repeat < 0 ||
            (rk_s64)frame_idx <= (rk_s64)e->frame_idx + e->repeat) {
            if (frame_idx >= e->frame_idx)
                return e;
        }
    }

    return NULL;
}

/* only one or two frames carry userdata: enough to verify the
 * user/kernel boundary crossing without adding UD SEI to every frame.
 * Optional scalars use -1 as the "not set" sentinel; leaving them at the
 * C zero default would materialize them as real frame config (frame qp 0,
 * non-IDR request, ...). */
static const MppEncFrmCfg mpp_enc_test_entries[] = {
    {
        .frame_idx           = 0,
        .repeat              = 0,
        .input_idr_req       = -1,
        .input_pskip         = -1,
        .input_pskip_non_ref = -1,
        .input_pskip_num     = -1,
        .enc_mark_ltr        = -1,
        .enc_use_ltr         = -1,
        .enc_frame_qp        = -1,
        .enc_base_layer_pid  = -1,
        .temporal_id         = -1,
        .userdatas           = 1,
    },
    {
        .frame_idx           = 1,
        .repeat              = 0,
        .input_idr_req       = -1,
        .input_pskip         = -1,
        .input_pskip_non_ref = -1,
        .input_pskip_num     = -1,
        .enc_mark_ltr        = -1,
        .enc_use_ltr         = -1,
        .enc_frame_qp        = -1,
        .enc_base_layer_pid  = -1,
        .temporal_id         = -1,
        .userdata            = 1,
    },
};

static const MppEncFrmCfg *mpp_enc_test_entry_ptrs[] = {
    &mpp_enc_test_entries[0], &mpp_enc_test_entries[1],
};

const MppEncFrmCfgSet mpp_enc_test_frm_cfg = {
    .count   = MPP_ARRAY_ELEMS(mpp_enc_test_entry_ptrs),
    .entries = mpp_enc_test_entry_ptrs,
};

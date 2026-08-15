/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2026 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_enc_frm_cfg_test"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_common.h"
#include "mpp_cfg_io.h"
#include "mpp_enc_frm_cfg.h"
#include "kmpp_meta.h"
#include "kmpp_venc_utils.h"

#define TEST_PASS(fmt, ...)  mpp_logi("[PASS] " fmt, ## __VA_ARGS__)
#define TEST_FAIL(fmt, ...)  do { mpp_loge("[FAIL] " fmt, ## __VA_ARGS__); ret = -1; } while (0)

static rk_s32 test_frame_meta(MppEncFrmCfgObj obj)
{
    const MppEncFrmCfg *entry = mpp_enc_frm_cfg_get_entry(obj);
    const MppEncFrmCfg *kmpp_entry = entry;
    MppEncFrmMetaData mpp_data = { 0 };
    MppEncFrmCfg *dup = NULL;
    MppMeta meta = NULL;
    KmppMeta kmeta = NULL;
    RK_U8 ud_buf[64];
    void *ptr = NULL;
    RK_S32 val = -1;
    rk_s32 ret = 0;

    if (!entry)
        return rk_nok;

    if ((entry->userdata || entry->userdatas) && !entry->ud_buf) {
        memset(ud_buf, 0x5a, sizeof(ud_buf));
        mpp_data.ud_buf = ud_buf;
        mpp_data.ud_buf_size = sizeof(ud_buf);

        dup = venc_dup_frm_cfg_with_ud(entry, ud_buf, sizeof(ud_buf));
        if (!dup) {
            TEST_FAIL("duplicate KMPP entry");
            goto done;
        }

        kmpp_entry = dup;
    }

    if (mpp_meta_get(&meta) || mpp_venc_gen_frame_meta(meta, 1920, 1080, entry, &mpp_data)) {
        TEST_FAIL("materialize MppMeta");
        goto done;
    }

    if (entry->userdata && (mpp_meta_get_ptr(meta, KEY_USER_DATA, &ptr) || !ptr))
        TEST_FAIL("MppMeta USER_DATA");
    ptr = NULL;

    if (entry->userdatas && (mpp_meta_get_ptr(meta, KEY_USER_DATAS, &ptr) || !ptr))
        TEST_FAIL("MppMeta USER_DATAS");
    ptr = NULL;

    if (entry->roi_cnt && (mpp_meta_get_ptr(meta, KEY_ROI_DATA, &ptr) || !ptr))
        TEST_FAIL("MppMeta ROI");
    ptr = NULL;

    if (entry->osd_cnt && (mpp_meta_get_ptr(meta, KEY_OSD_DATA3, &ptr) || !ptr))
        TEST_FAIL("MppMeta OSD");
    ptr = NULL;

    if (entry->jpeg_roi_cnt && (mpp_meta_get_ptr(meta, KEY_JPEG_ROI_DATA, &ptr) || !ptr))
        TEST_FAIL("MppMeta JPEG ROI");

#define CHECK_MPP_SCALAR(field, key) \
    do { \
        if (entry->field >= 0 && \
            (mpp_meta_get_s32(meta, key, &val) || val != entry->field)) \
            TEST_FAIL("MppMeta " #field); \
    } while (0)

    CHECK_MPP_SCALAR(input_idr_req, KEY_INPUT_IDR_REQ);
    CHECK_MPP_SCALAR(input_pskip, KEY_INPUT_PSKIP);
    CHECK_MPP_SCALAR(input_pskip_non_ref, KEY_INPUT_PSKIP_NON_REF);
    CHECK_MPP_SCALAR(input_pskip_num, KEY_INPUT_PSKIP_NUM);
    CHECK_MPP_SCALAR(enc_mark_ltr, KEY_ENC_MARK_LTR);
    CHECK_MPP_SCALAR(enc_use_ltr, KEY_ENC_USE_LTR);
    CHECK_MPP_SCALAR(enc_frame_qp, KEY_ENC_FRAME_QP);
    CHECK_MPP_SCALAR(enc_base_layer_pid, KEY_ENC_BASE_LAYER_PID);
    CHECK_MPP_SCALAR(temporal_id, KEY_TEMPORAL_ID);

#undef CHECK_MPP_SCALAR

    if (kmpp_meta_get_f(&kmeta) || kmpp_venc_gen_frame_meta(kmeta, 1920, 1080, kmpp_entry)) {
        TEST_FAIL("materialize KmppMeta");
        goto done;
    }

    if (entry->userdata && (kmpp_meta_get_ptr(kmeta, KEY_USER_DATA, &ptr) || !ptr))
        TEST_FAIL("KmppMeta USER_DATA");
    ptr = NULL;

    if (entry->userdatas && (kmpp_meta_get_ptr(kmeta, KEY_USER_DATAS, &ptr) || !ptr))
        TEST_FAIL("KmppMeta USER_DATAS");
    ptr = NULL;

    if (entry->roi_cnt && (kmpp_meta_get_ptr(kmeta, KEY_ROI_DATA, &ptr) || !ptr))
        TEST_FAIL("KmppMeta ROI");
    ptr = NULL;

    if (entry->osd_cnt && (kmpp_meta_get_ptr(kmeta, KEY_OSD_DATA4, &ptr) || !ptr))
        TEST_FAIL("KmppMeta OSD");
    ptr = NULL;

    if (entry->jpeg_roi_cnt && (kmpp_meta_get_ptr(kmeta, KEY_JPEG_ROI_DATA, &ptr) || !ptr))
        TEST_FAIL("KmppMeta JPEG ROI");

#define CHECK_KMPP_SCALAR(field, key) \
    do { \
        if (entry->field >= 0 && \
            (kmpp_meta_get_s32(kmeta, key, &val) || val != entry->field)) \
            TEST_FAIL("KmppMeta " #field); \
    } while (0)

    CHECK_KMPP_SCALAR(input_idr_req, KEY_INPUT_IDR_REQ);
    CHECK_KMPP_SCALAR(input_pskip, KEY_INPUT_PSKIP);
    CHECK_KMPP_SCALAR(input_pskip_num, KEY_INPUT_PSKIP_NUM);
    CHECK_KMPP_SCALAR(enc_mark_ltr, KEY_ENC_MARK_LTR);
    CHECK_KMPP_SCALAR(enc_use_ltr, KEY_ENC_USE_LTR);
    CHECK_KMPP_SCALAR(enc_frame_qp, KEY_ENC_FRAME_QP);
    CHECK_KMPP_SCALAR(enc_base_layer_pid, KEY_ENC_BASE_LAYER_PID);
    CHECK_KMPP_SCALAR(temporal_id, KEY_TEMPORAL_ID);

#undef CHECK_KMPP_SCALAR

    if (!ret)
        TEST_PASS("MppMeta and KmppMeta materialization");

done:
    if (meta)
        mpp_meta_put(meta);
    if (kmeta)
        kmpp_meta_put_f(kmeta);
    mpp_venc_frm_meta_deinit(&mpp_data);
    MPP_FREE(dup);

    return ret;
}

/*
 * External config file mode: read a JSON/TOML file from argv[1], apply it,
 * extract the result and dump it, then verify an idempotent round-trip
 * (re-apply the extracted text to the same already-sized object and compare).
 *
 * The object is resized by roi_cnt/osd_cnt found in the input, so the
 * re-apply to the same object is safe (capacity is already sufficient).
 *
 * Usage: mpp_enc_frm_cfg_test <cfg/enc/frm_roi.json>
 */
static rk_s32 test_file(const char *path)
{
    MppEncFrmCfgObj obj = NULL;
    MppCfgStrFmt fmt = MPP_CFG_STR_FMT_JSON;
    char *out = NULL;
    char *re_out = NULL;
    rk_s32 ret = 0;
    char *ext = strrchr(path, '.');

    if (ext) {
        if (!strcmp(ext, ".toml"))
            fmt = MPP_CFG_STR_FMT_TOML;
        else if (!strcmp(ext, ".json"))
            fmt = MPP_CFG_STR_FMT_JSON;
    }
    mpp_logi("file %s fmt %d\n", path, fmt);

    if (mpp_enc_frm_cfg_get(&obj)) {
        TEST_FAIL("get obj failed");
        goto DONE;
    }

    /* apply: VLA counts (roi_cnt/osd_cnt) in the file trigger resize */
    if (mpp_enc_frm_cfg_apply_file(obj, fmt, path)) {
        TEST_FAIL("apply %s failed", path);
        goto DONE;
    }
    TEST_PASS("apply %s ok", path);

    if (test_frame_meta(obj)) {
        TEST_FAIL("frame meta %s failed", path);
        goto DONE;
    }

    /* export: extract and dump */
    if (mpp_enc_frm_cfg_extract(obj, fmt, &out)) {
        TEST_FAIL("extract failed");
        goto DONE;
    }
    mpp_logi("extract:\n%s", out);

    /* idempotent round-trip on the same (already-sized) object */
    if (mpp_enc_frm_cfg_apply(obj, fmt, out)) {
        TEST_FAIL("re-apply failed");
        goto DONE;
    }
    if (mpp_enc_frm_cfg_extract(obj, fmt, &re_out)) {
        TEST_FAIL("re-extract failed");
        goto DONE;
    }
    if (out && re_out && !strcmp(out, re_out))
        TEST_PASS("roundtrip ok\n%s", out);
    else
        TEST_FAIL("roundtrip mismatch\n--- in ---\n%s--- out ---\n%s", out, re_out);

DONE:
    MPP_FREE(out);
    MPP_FREE(re_out);

    if (obj)
        mpp_enc_frm_cfg_put(obj);

    return ret;
}

/* ---------- multi-frame CfgSet mode ---------- */

#define MAX_FRM_CFGS    16
#define EXPECT_EQ(lbl, act, exp) \
    do { if ((act) == (exp)) TEST_PASS("%s %s=%d", name, lbl, (int)(act)); \
         else TEST_FAIL("%s %s expect=%d actual=%d", name, lbl, (int)(exp), (int)(act)); \
    } while (0)

static void basename_no_ext(const char *path, char *out, rk_s32 out_size)
{
    const char *base = strrchr(path, '/');
    const char *dot;
    rk_s32 len;

    base = base ? base + 1 : path;
    dot = strrchr(base, '.');
    len = (dot && dot > base) ? (rk_s32)(dot - base) : (rk_s32)strlen(base);
    if (len >= out_size)
        len = out_size - 1;
    memcpy(out, base, len);
    out[len] = '\0';
}

/*
 * Verify one frame entry's fields in-memory against the known config for
 * the given basename. Direct struct access (no JSON reparse).
 */
static rk_s32 verify_frame(const char *name, const MppEncFrmCfg *e)
{
    rk_s32 ret = 0;
    const MppEncFrmRoi *roi;
    const MppEncFrmOsd *osd;

    (void)roi;
    (void)osd;

    if (!e) {
        TEST_FAIL("%s null entry", name);
        return -1;
    }

    if (!strcmp(name, "frm_ud")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 5);
        EXPECT_EQ("repeat",    e->repeat,    3);
        EXPECT_EQ("userdata",  e->userdata,  1);
        EXPECT_EQ("userdatas", e->userdatas, 0);
        EXPECT_EQ("roi_cnt",   e->roi_cnt,   0);
        EXPECT_EQ("osd_cnt",   e->osd_cnt,   0);
    } else if (!strcmp(name, "frm_uds")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 7);
        EXPECT_EQ("repeat",    e->repeat,    -1);
        EXPECT_EQ("userdata",  e->userdata,  0);
        EXPECT_EQ("userdatas", e->userdatas, 1);
        EXPECT_EQ("roi_cnt",   e->roi_cnt,   0);
        EXPECT_EQ("osd_cnt",   e->osd_cnt,   0);
    } else if (!strcmp(name, "frm_roi")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 0);
        EXPECT_EQ("repeat",    e->repeat,    -1);
        EXPECT_EQ("userdata",  e->userdata,  0);
        EXPECT_EQ("userdatas", e->userdatas, 0);
        EXPECT_EQ("roi_cnt",   e->roi_cnt,   2);
        EXPECT_EQ("osd_cnt",   e->osd_cnt,   0);
        roi = MPP_ENC_FRM_ROI_ARR(e);
        EXPECT_EQ("roi0.x",        roi[0].x,        0);
        EXPECT_EQ("roi0.y",        roi[0].y,        0);
        EXPECT_EQ("roi0.w",        roi[0].w,        32768);
        EXPECT_EQ("roi0.h",        roi[0].h,        16384);
        EXPECT_EQ("roi0.intra",    roi[0].intra,    0);
        EXPECT_EQ("roi0.quality",  roi[0].quality,  45);
        EXPECT_EQ("roi0.abs_qp_en", roi[0].abs_qp_en, 1);
        EXPECT_EQ("roi1.x",        roi[1].x,        16384);
        EXPECT_EQ("roi1.y",        roi[1].y,        16384);
        EXPECT_EQ("roi1.w",        roi[1].w,        32768);
        EXPECT_EQ("roi1.h",        roi[1].h,        16384);
        EXPECT_EQ("roi1.intra",    roi[1].intra,    1);
        EXPECT_EQ("roi1.quality",  roi[1].quality,  32);
        EXPECT_EQ("roi1.abs_qp_en", roi[1].abs_qp_en, 0);
    } else if (!strcmp(name, "frm_osd")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 1);
        EXPECT_EQ("repeat",    e->repeat,    10);
        EXPECT_EQ("userdata",  e->userdata,  0);
        EXPECT_EQ("userdatas", e->userdatas, 0);
        EXPECT_EQ("roi_cnt",   e->roi_cnt,   0);
        EXPECT_EQ("osd_cnt",   e->osd_cnt,   2);
        osd = MPP_ENC_FRM_OSD_ARR(e);
        EXPECT_EQ("osd0.enable", osd[0].enable, 1);
        EXPECT_EQ("osd0.fmt",    osd[0].fmt,    65546);
        EXPECT_EQ("osd0.lt_x",   osd[0].lt_x,   0);
        EXPECT_EQ("osd0.lt_y",   osd[0].lt_y,   0);
        EXPECT_EQ("osd0.rb_x",   osd[0].rb_x,   16384);
        EXPECT_EQ("osd0.rb_y",   osd[0].rb_y,   32768);
        EXPECT_EQ("osd1.enable", osd[1].enable, 1);
        EXPECT_EQ("osd1.fmt",    osd[1].fmt,    65546);
        EXPECT_EQ("osd1.lt_x",   osd[1].lt_x,   32768);
        EXPECT_EQ("osd1.lt_y",   osd[1].lt_y,   16384);
        EXPECT_EQ("osd1.rb_x",   osd[1].rb_x,   65535);
        EXPECT_EQ("osd1.rb_y",   osd[1].rb_y,   65535);
    } else if (!strcmp(name, "frm_default")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 0);
        EXPECT_EQ("repeat",    e->repeat,    -1);
        EXPECT_EQ("userdata",  e->userdata,  0);
        EXPECT_EQ("userdatas", e->userdatas, 0);
        EXPECT_EQ("roi_cnt",   e->roi_cnt,   0);
        EXPECT_EQ("osd_cnt",   e->osd_cnt,   0);
    } else if (!strcmp(name, "frm_scalar")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 4);
        EXPECT_EQ("repeat", e->repeat, 0);
        EXPECT_EQ("input_idr_req", e->input_idr_req, 1);
        EXPECT_EQ("input_pskip", e->input_pskip, -1);
        EXPECT_EQ("enc_mark_ltr", e->enc_mark_ltr, -1);
        EXPECT_EQ("enc_use_ltr", e->enc_use_ltr, -1);
        EXPECT_EQ("enc_frame_qp", e->enc_frame_qp, 37);
        EXPECT_EQ("enc_base_layer_pid", e->enc_base_layer_pid, 1);
        EXPECT_EQ("temporal_id", e->temporal_id, 1);
    } else if (!strcmp(name, "frm_pskip")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 9);
        EXPECT_EQ("input_pskip", e->input_pskip, 1);
    } else if (!strcmp(name, "frm_pskip_non_ref")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 9);
        EXPECT_EQ("input_pskip_non_ref", e->input_pskip_non_ref, 1);
    } else if (!strcmp(name, "frm_pskip_num")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 4);
        EXPECT_EQ("input_pskip_num", e->input_pskip_num, 2);
    } else if (!strcmp(name, "frm_ltr_mark")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 4);
        EXPECT_EQ("enc_mark_ltr", e->enc_mark_ltr, 0);
    } else if (!strcmp(name, "frm_ltr_use")) {
        EXPECT_EQ("frame_idx", e->frame_idx, 6);
        EXPECT_EQ("enc_use_ltr", e->enc_use_ltr, 0);
    } else if (!strcmp(name, "frm_jpeg_roi")) {
        const MppEncFrmJpegRoi *jpeg = MPP_ENC_FRM_JPEG_ROI_ARR(e);

        EXPECT_EQ("frame_idx", e->frame_idx, 0);
        EXPECT_EQ("jpeg_roi_cnt", e->jpeg_roi_cnt, 1);
        EXPECT_EQ("jpeg_non_roi_level", e->jpeg_non_roi_level, 1);
        EXPECT_EQ("jpeg.level", jpeg[0].level, 3);
    } else {
        TEST_FAIL("unknown frame config %s", name);
        ret = -1;
    }

    return ret;
}

/*
 * Multi-frame mode: apply each JSON to its own obj, assemble a CfgSet,
 * verify per-frame fields and lookup. argc >= 3 selects this path.
 */
static rk_s32 test_multi(rk_s32 n, char **paths)
{
    MppEncFrmCfgObj obj[MAX_FRM_CFGS] = { NULL };
    const MppEncFrmCfg *entry_ptrs[MAX_FRM_CFGS] = { NULL };
    char names[MAX_FRM_CFGS][32] = { { 0 } };
    MppEncFrmCfgSet set = { 0, NULL };
    rk_s32 ret = 0;
    rk_s32 i;

    if (n > MAX_FRM_CFGS) {
        TEST_FAIL("too many files %d > %d", n, MAX_FRM_CFGS);
        return -1;
    }

    for (i = 0; i < n; i++) {
        MppEncFrmCfgObj o = NULL;
        char name[32];

        basename_no_ext(paths[i], name, sizeof(name));

        mpp_logi("--- [%d/%d] load %s ---\n", i + 1, n, paths[i]);

        if (mpp_enc_frm_cfg_get(&o)) {
            TEST_FAIL("get obj for %s failed", paths[i]);
            ret = -1;
            continue;
        }

        if (mpp_enc_frm_cfg_apply_file(o, MPP_CFG_STR_FMT_JSON, paths[i])) {
            TEST_FAIL("apply %s failed", paths[i]);
            mpp_enc_frm_cfg_put(o);
            ret = -1;
            continue;
        }

        TEST_PASS("apply %s ok", paths[i]);
        obj[i] = o;
        entry_ptrs[i] = mpp_enc_frm_cfg_get_entry(o);
        snprintf(names[i], sizeof(names[i]), "%s", name);
    }

    set.count = n;
    set.entries = entry_ptrs;

    mpp_logi("\n=== CfgSet built: %d frames ===\n", n);

    /* per-frame field verification */
    for (i = 0; i < n; i++) {
        if (!entry_ptrs[i] || !names[i][0])
            continue;
        mpp_logi("--- verify frame[%d] %s ---\n", i, names[i]);
        if (verify_frame(names[i], entry_ptrs[i]) || test_frame_meta(obj[i]))
            ret = -1;
    }

    /* lookup sanity: lookup(set, frame_idx) must return a covering entry */
    for (i = 0; i < n; i++) {
        const MppEncFrmCfg *e = entry_ptrs[i];
        const MppEncFrmCfg *hit;

        if (!e)
            continue;
        hit = mpp_enc_frm_cfg_lookup(&set, e->frame_idx);
        if (!hit) {
            TEST_FAIL("lookup frame_idx=%d returned NULL", e->frame_idx);
            ret = -1;
        } else if (e->frame_idx < hit->frame_idx ||
                   (hit->repeat >= 0 &&
                    e->frame_idx > hit->frame_idx + hit->repeat)) {
            TEST_FAIL("lookup frame_idx=%d not covered by hit idx=%d", e->frame_idx, hit->frame_idx);
            ret = -1;
        } else {
            TEST_PASS("lookup frame_idx=%d -> entry idx=%d", e->frame_idx, hit->frame_idx);
        }
    }

    for (i = 0; i < n; i++) {
        if (obj[i])
            mpp_enc_frm_cfg_put(obj[i]);
    }

    return ret;
}

static rk_s32 test_lookup(void)
{
    const MppEncFrmCfg entries[] = {
        { .frame_idx = 10, .repeat = 0,  .userdata = 1 },
        { .frame_idx = 20, .repeat = 2,  .userdatas = 1 },
        { .frame_idx = 21, .repeat = -1, .userdata = 1 },
        { .frame_idx = 30, .repeat = 5,  .userdatas = 1 },
        { .frame_idx = INT_MAX - 1, .repeat = 4, .userdata = 1 },
    };
    const MppEncFrmCfg *entry_ptrs[] = {
        &entries[0], &entries[1], &entries[2], &entries[3], &entries[4],
    };
    const MppEncFrmCfgSet set = {
        .count = MPP_ARRAY_ELEMS(entry_ptrs),
        .entries = entry_ptrs,
    };
    const MppEncFrmCfgSet empty = { 0 };
    const MppEncFrmCfg overflow_entry = {
        .frame_idx = INT_MAX - 1,
        .repeat = 4,
        .userdata = 1,
    };
    const MppEncFrmCfg *overflow_entry_ptr = &overflow_entry;
    const MppEncFrmCfgSet overflow_set = {
        .count = 1,
        .entries = &overflow_entry_ptr,
    };
    rk_s32 ret = 0;

    if (mpp_enc_frm_cfg_lookup(NULL, 0) || mpp_enc_frm_cfg_lookup(&empty, 0))
        TEST_FAIL("empty lookup");

    if (mpp_enc_frm_cfg_lookup(&set, 9) || mpp_enc_frm_cfg_lookup(&set, 11))
        TEST_FAIL("exact lookup boundary");

    if (mpp_enc_frm_cfg_lookup(&set, 10) != &entries[0])
        TEST_FAIL("exact lookup");

    if (mpp_enc_frm_cfg_lookup(&set, 20) != &entries[1] || mpp_enc_frm_cfg_lookup(&set, 22) != &entries[1])
        TEST_FAIL("finite repeat lookup");

    if (mpp_enc_frm_cfg_lookup(&set, 23) != &entries[2] || mpp_enc_frm_cfg_lookup(&set, 30) != &entries[2])
        TEST_FAIL("open repeat and first-match lookup");

    if (mpp_enc_frm_cfg_lookup(&overflow_set, INT_MAX) != &overflow_entry)
        TEST_FAIL("overflow-safe lookup");

    if (!ret)
        TEST_PASS("lookup boundaries");

    return ret;
}

static rk_s32 test_defaults(void)
{
    MppEncFrmCfgObj obj = NULL;
    const MppEncFrmCfg *entry;
    rk_s32 ret = 0;

    if (mpp_enc_frm_cfg_get(&obj))
        return rk_nok;

    entry = mpp_enc_frm_cfg_get_entry(obj);
    if (!entry || entry->input_idr_req != -1 || entry->input_pskip != -1 ||
        entry->input_pskip_non_ref != -1 || entry->input_pskip_num != -1 ||
        entry->enc_mark_ltr != -1 || entry->enc_use_ltr != -1 ||
        entry->enc_frame_qp != -1 || entry->enc_base_layer_pid != -1 || entry->temporal_id != -1)
        TEST_FAIL("optional scalar defaults");
    else
        TEST_PASS("optional scalar defaults");

    mpp_enc_frm_cfg_put(obj);

    return ret;
}

static rk_s32 test_vla_grow(void)
{
    static char first[] =
        "{\"roi_cnt\":1,\"roi\":[{\"w\":100,\"h\":100}],"
        "\"osd_cnt\":1,\"osd\":[{\"enable\":1,\"fmt\":65546,"
        "\"rb_x\":100,\"rb_y\":100}],"
        "\"jpeg_roi_cnt\":1,\"jpeg_roi\":[{\"w\":100,\"h\":100,"
        "\"level\":7,\"roi_en\":1}]}";
    static char grow[] =
        "{\"roi_cnt\":2,\"roi\":[{\"w\":100,\"h\":100},"
        "{\"x\":100,\"y\":100,\"w\":200,\"h\":200}]}";
    MppEncFrmCfgObj obj = NULL;
    const MppEncFrmCfg *entry;
    const MppEncFrmOsd *osd;
    const MppEncFrmJpegRoi *jpeg;
    rk_s32 ret = 0;

    if (mpp_enc_frm_cfg_get(&obj) || mpp_enc_frm_cfg_apply(obj, MPP_CFG_STR_FMT_JSON, first) ||
        mpp_enc_frm_cfg_apply(obj, MPP_CFG_STR_FMT_JSON, grow)) {
        TEST_FAIL("VLA grow apply");
        goto done;
    }

    entry = mpp_enc_frm_cfg_get_entry(obj);
    if (!entry) {
        TEST_FAIL("VLA grow entry");
        goto done;
    }

    osd = MPP_ENC_FRM_OSD_ARR(entry);
    jpeg = MPP_ENC_FRM_JPEG_ROI_ARR(entry);
    if (entry->roi_cnt != 2 || entry->osd_cnt != 1 || entry->jpeg_roi_cnt != 1 || osd[0].enable != 1 ||
        jpeg[0].level != 7)
        TEST_FAIL("VLA grow preserves trailing arrays");
    else
        TEST_PASS("VLA grow preserves trailing arrays");

done:
    if (obj)
        mpp_enc_frm_cfg_put(obj);

    return ret;
}

static rk_s32 test_invalid_inputs(void)
{
    typedef struct InvalidFrmEntry_t {
        MppEncFrmCfg cfg;
        MppEncFrmRoi roi;
        MppEncFrmOsd osd;
    } InvalidFrmEntry;
    InvalidFrmEntry entry = { 0 };
    MppEncFrmMetaData data = { 0 };
    MppMeta meta = NULL;
    rk_s32 ret = 0;

    if (mpp_meta_get(&meta))
        return rk_nok;

    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data))
        TEST_FAIL("accept disabled pskip modes");

    entry.cfg.input_idr_req = 2;
    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data) != MPP_ERR_VALUE)
        TEST_FAIL("reject invalid scalar");

    memset(&entry, 0, sizeof(entry));
    entry.cfg.input_pskip = 1;
    entry.cfg.input_pskip_non_ref = 1;
    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data) != MPP_ERR_VALUE)
        TEST_FAIL("reject conflicting pskip modes");

    memset(&entry, 0, sizeof(entry));
    entry.cfg.roi_cnt = 1;
    entry.cfg.roi_off = offsetof(InvalidFrmEntry, roi);
    entry.roi.x = -1;
    entry.roi.w = 16;
    entry.roi.h = 16;
    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data) != MPP_ERR_VALUE)
        TEST_FAIL("reject invalid ROI");

    memset(&entry, 0, sizeof(entry));
    entry.cfg.osd_cnt = 1;
    entry.cfg.osd_off = offsetof(InvalidFrmEntry, osd);
    entry.osd.enable = 1;
    entry.osd.fmt = MPP_FMT_ARGB8888;
    entry.osd.rb_x = 127;
    entry.osd.rb_y = 127;
    entry.osd.stride = 1;
    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data) != MPP_ERR_VALUE)
        TEST_FAIL("reject invalid OSD stride");

    memset(&entry, 0, sizeof(entry));
    entry.cfg.userdata = 1;
    if (mpp_venc_gen_frame_meta(meta, 1920, 1080, &entry.cfg, &data) != MPP_ERR_NULL_PTR)
        TEST_FAIL("reject missing userdata buffer");

    if (!ret)
        TEST_PASS("invalid materialization inputs");

    mpp_venc_frm_meta_deinit(&data);
    mpp_meta_put(meta);

    return ret;
}

static rk_s32 test_scalar_roundtrip(void)
{
    MppEncFrmCfgObj obj = NULL;
    char *json = NULL;
    rk_s32 ret = 0;

    mpp_logi("=== scalar apply + extract ===\n");
    if (mpp_enc_frm_cfg_get(&obj)) {
        TEST_FAIL("get obj failed");
        return -1;
    }
    TEST_PASS("get obj ok");

    {
        const char *input =
            "{ \"frame_idx\" : 3, \"userdata\" : 1, \"userdatas\" : 0, \"repeat\" : 5 }";
        if (mpp_enc_frm_cfg_apply(obj, MPP_CFG_STR_FMT_JSON, (char *)input)) {
            TEST_FAIL("apply failed");
            mpp_enc_frm_cfg_put(obj);
            return -1;
        }
        TEST_PASS("apply ok");
    }

    if (mpp_enc_frm_cfg_extract(obj, MPP_CFG_STR_FMT_JSON, &json)) {
        TEST_FAIL("extract failed");
    } else {
        TEST_PASS("extract ok\n%s", json);

        if (strstr(json, "\"frame_idx\" : 3"))
            TEST_PASS("frame_idx");
        else TEST_FAIL("frame_idx");

        if (strstr(json, "\"userdata\" : 1"))
            TEST_PASS("userdata");
        else TEST_FAIL("userdata");

        if (strstr(json, "\"userdatas\" : 0"))
            TEST_PASS("userdatas");
        else TEST_FAIL("userdatas");

        if (strstr(json, "\"repeat\" : 5"))
            TEST_PASS("repeat");
        else TEST_FAIL("repeat");

        MPP_FREE(json);
    }

    mpp_logi("\n=== roundtrip: re-apply extracted JSON ===\n");
    {
        MppEncFrmCfgObj obj2 = NULL;
        char *rejson = NULL;
        char *json2 = NULL;

        if (mpp_enc_frm_cfg_get(&obj2)) {
            TEST_FAIL("get obj2 failed");
        } else if (mpp_enc_frm_cfg_extract(obj, MPP_CFG_STR_FMT_JSON, &rejson)) {
            TEST_FAIL("re-extract failed");
        } else if (mpp_enc_frm_cfg_apply(obj2, MPP_CFG_STR_FMT_JSON, rejson)) {
            TEST_FAIL("re-apply failed");
        } else if (mpp_enc_frm_cfg_extract(obj2, MPP_CFG_STR_FMT_JSON, &json2)) {
            TEST_FAIL("re-extract obj2 failed");
        } else if (strstr(json2, "\"frame_idx\" : 3") &&
                   strstr(json2, "\"userdata\" : 1") &&
                   strstr(json2, "\"repeat\" : 5")) {
            TEST_PASS("roundtrip ok");
        } else {
            TEST_FAIL("roundtrip mismatch:\n%s", json2);
        }
        MPP_FREE(rejson);
        MPP_FREE(json2);
        if (obj2)
            mpp_enc_frm_cfg_put(obj2);
    }

    mpp_enc_frm_cfg_put(obj);

    /* test: VLA roundtrip (ROI + OSD) */
    mpp_logi("\n=== VLA roundtrip ===\n");
    {
        MppEncFrmCfgObj vla_obj = NULL;
        const char *vla_json =
            "{ \"frame_idx\" : 0, \"userdata\" : 1, \"roi_cnt\" : 2, \"osd_cnt\" : 1,"
            "  \"roi\" : ["
            "    { \"x\" : 16, \"y\" : 16, \"w\" : 64, \"h\" : 64,"
            "      \"intra\" : 1, \"quality\" : 51 },"
            "    { \"x\" : 200, \"y\" : 100, \"w\" : 64, \"h\" : 128 }"
            "  ],"
            "  \"osd\" : ["
            "    { \"enable\" : 1, \"fmt\" : 1,"
            "      \"lt_x\" : 0, \"lt_y\" : 0, \"rb_x\" : 640, \"rb_y\" : 48 }"
            "  ]}";
        char *vla_out = NULL;

        if (mpp_enc_frm_cfg_get(&vla_obj)) {
            TEST_FAIL("get vla_obj failed");
        } else if (mpp_enc_frm_cfg_apply(vla_obj, MPP_CFG_STR_FMT_JSON,
                                         (char *)vla_json)) {
            TEST_FAIL("apply VLA JSON failed");
        } else if (mpp_enc_frm_cfg_extract(vla_obj, MPP_CFG_STR_FMT_JSON,
                                           &vla_out)) {
            TEST_FAIL("extract VLA failed");
        } else {
            if (strstr(vla_out, "\"frame_idx\" : 0") &&
                strstr(vla_out, "\"userdata\" : 1") &&
                strstr(vla_out, "\"userdatas\" : 0") &&
                strstr(vla_out, "\"repeat\" : -1") &&
                strstr(vla_out, "\"x\" : 16") &&
                strstr(vla_out, "\"y\" : 16") &&
                strstr(vla_out, "\"w\" : 64") &&
                strstr(vla_out, "\"h\" : 64") &&
                strstr(vla_out, "\"intra\" : 1") &&
                strstr(vla_out, "\"quality\" : 51") &&
                strstr(vla_out, "\"x\" : 200") &&
                strstr(vla_out, "\"y\" : 100") &&
                strstr(vla_out, "\"h\" : 128") &&
                strstr(vla_out, "\"enable\" : 1") &&
                strstr(vla_out, "\"fmt\" : 1") &&
                strstr(vla_out, "\"lt_x\" : 0") &&
                strstr(vla_out, "\"lt_y\" : 0") &&
                strstr(vla_out, "\"rb_x\" : 640") &&
                strstr(vla_out, "\"rb_y\" : 48"))
                TEST_PASS("VLA roundtrip ok\n%s", vla_out);
            else
                TEST_FAIL("VLA roundtrip mismatch\n%s", vla_out);
            MPP_FREE(vla_out);
        }
        if (vla_obj)
            mpp_enc_frm_cfg_put(vla_obj);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    RK_S32 ret = 0;

    /* file mode: dispatch on number of config files given */
    if (argc > 1) {
        if (argc == 2) {
            /* single file: apply/extract/roundtrip on its own obj */
            ret = test_file(argv[1]);
        } else {
            /* multiple files: assemble a CfgSet and verify per-frame */
            mpp_logi("=== multi-frame CfgSet: %d files ===\n", argc - 1);
            ret = test_multi(argc - 1, &argv[1]);
        }
    } else {
        ret = test_scalar_roundtrip();
    }

    /* built-in whitebox tests run after the file tests */
    if (test_defaults())
        ret = -1;

    if (test_vla_grow())
        ret = -1;

    if (test_lookup())
        ret = -1;

    if (test_invalid_inputs())
        ret = -1;

    mpp_logi("\n=== %s ===\n", ret ? "FAILED" : "ALL PASSED");
    return ret;
}

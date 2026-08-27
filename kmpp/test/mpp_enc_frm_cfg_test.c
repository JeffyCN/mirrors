/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2026 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_enc_frm_cfg_test"

#include <stdio.h>
#include <string.h>

#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_common.h"
#include "mpp_cfg_io.h"

#include "kmpp_obj.h"
#include "mpp_enc_frm_cfg.h"

#define TEST_PASS(fmt, ...)  mpp_logi("[PASS] " fmt, ## __VA_ARGS__)
#define TEST_FAIL(fmt, ...)  do { mpp_loge("[FAIL] " fmt, ## __VA_ARGS__); ret = -1; } while (0)

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
        EXPECT_EQ("osd0.fmt",    osd[0].fmt,    1);
        EXPECT_EQ("osd0.lt_x",   osd[0].lt_x,   0);
        EXPECT_EQ("osd0.lt_y",   osd[0].lt_y,   0);
        EXPECT_EQ("osd0.rb_x",   osd[0].rb_x,   16384);
        EXPECT_EQ("osd0.rb_y",   osd[0].rb_y,   32768);
        EXPECT_EQ("osd1.enable", osd[1].enable, 1);
        EXPECT_EQ("osd1.fmt",    osd[1].fmt,    1);
        EXPECT_EQ("osd1.lt_x",   osd[1].lt_x,   32768);
        EXPECT_EQ("osd1.lt_y",   osd[1].lt_y,   16384);
        EXPECT_EQ("osd1.rb_x",   osd[1].rb_x,   65535);
        EXPECT_EQ("osd1.rb_y",   osd[1].rb_y,   65535);
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
        entry_ptrs[i] = (const MppEncFrmCfg *)kmpp_obj_to_entry(o);
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
        if (verify_frame(names[i], entry_ptrs[i]))
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
            TEST_FAIL("lookup frame_idx=%d not covered by hit idx=%d",
                      e->frame_idx, hit->frame_idx);
            ret = -1;
        } else {
            TEST_PASS("lookup frame_idx=%d -> entry idx=%d",
                      e->frame_idx, hit->frame_idx);
        }
    }

    for (i = 0; i < n; i++) {
        if (obj[i])
            mpp_enc_frm_cfg_put(obj[i]);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    MppEncFrmCfgObj obj = NULL;
    char *json = NULL;
    RK_S32 ret = 0;

    /* file mode: dispatch on number of config files given */
    if (argc > 1) {
        if (argc == 2) {
            /* single file: apply/extract/roundtrip on its own obj */
            ret = test_file(argv[1]);
            mpp_logi("\n=== %s ===\n", ret ? "FAILED" : "ALL PASSED");
        } else {
            /* multiple files: assemble a CfgSet and verify per-frame */
            mpp_logi("=== multi-frame CfgSet: %d files ===\n", argc - 1);
            ret = test_multi(argc - 1, &argv[1]);
            mpp_logi("\n=== %s ===\n", ret ? "FAILED" : "ALL PASSED");
        }
        return ret;
    }

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

    mpp_logi("\n=== %s ===\n", ret ? "FAILED" : "ALL PASSED");
    return ret;
}

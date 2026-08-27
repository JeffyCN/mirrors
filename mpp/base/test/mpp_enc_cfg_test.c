/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2015 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_enc_cfg_test"

#include "mpp_mem.h"
#include "mpp_common.h"
#include "mpp_test.h"

#include "rk_venc_cfg.h"
#include "rk_venc_kcfg.h"
#include "mpp_enc_cfg.h"
#include "mpp_cfg_io.h"

typedef struct {
    MppEncCfg cfg;
    rk_u32    flag;
} TestCtx;

/* option handlers */
static rk_s32 test_opt_v(void *ctx, const char *next)
{
    TestCtx *tc = (TestCtx *)ctx;
    (void)next;

    tc->flag |= MPP_FLAG_VERBOSE;
    return 1;
}

static rk_s32 test_opt_d(void *ctx, const char *next)
{
    TestCtx *tc = (TestCtx *)ctx;

    if (next) {
        tc->flag |= (rk_u32)strtol(next, NULL, 0);
        return 2;
    }
    return 1;
}

static MppTestOpt test_opts[] = {
    { 'v', "verbose", "enable verbose output",       test_opt_v },
    { 'd', "debug",   "set debug flags (hex/dec)",   test_opt_d },
};

MPP_TEST(test_scalar_set_get)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    MppEncCfgSet *impl = (MppEncCfgSet *)kmpp_obj_to_entry(cfg);
    rk_u32 flag = tc->flag;
    rk_s32 rc_mode = 1;
    rk_s32 bps_target = 400000;
    rk_s32 ret;

    MPP_VERBOSE(flag, "before set: rc mode %d bps_target %d\n",
                impl->rc.rc_mode, impl->rc.bps_target);

    mpp_enc_cfg_set_s32(cfg, "rc:mode", rc_mode);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", bps_target);

    MPP_VERBOSE(flag, "after  set: rc mode %d bps_target %d\n",
                impl->rc.rc_mode, impl->rc.bps_target);

    rc_mode = 0;
    bps_target = 0;

    ret = mpp_enc_cfg_get_s32(cfg, "rc:mode", &rc_mode);
    MPP_ASSERT(!ret);
    ret = mpp_enc_cfg_get_s32(cfg, "rc:bps_target", &bps_target);
    MPP_ASSERT(!ret);

    MPP_VERBOSE(flag, "after  get: rc mode %d bps_target %d\n", rc_mode, bps_target);

    MPP_ASSERT_EQ(1, rc_mode);
    MPP_ASSERT_EQ(400000, bps_target);

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_array_set_get)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    rk_s32 aq_thrd_i[16] = {
        0,  0,  0,  0,   3,  3,  5,  5,
        8,  8,  8,  15, 15, 20, 25, 35
    };
    rk_s32 aq_thrd_i_ret[16];
    rk_s32 ret;
    rk_s32 i;

    memset(aq_thrd_i_ret, -1, sizeof(aq_thrd_i_ret));

    MPP_VERBOSE(flag, "before set: aq_step_i: "
                "%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n",
                aq_thrd_i_ret[0], aq_thrd_i_ret[1], aq_thrd_i_ret[2], aq_thrd_i_ret[3],
                aq_thrd_i_ret[4], aq_thrd_i_ret[5], aq_thrd_i_ret[6], aq_thrd_i_ret[7],
                aq_thrd_i_ret[8], aq_thrd_i_ret[9], aq_thrd_i_ret[10], aq_thrd_i_ret[11],
                aq_thrd_i_ret[12], aq_thrd_i_ret[13], aq_thrd_i_ret[14], aq_thrd_i_ret[15]);

    ret = mpp_enc_cfg_set_st(cfg, "hw:aq_step_i", aq_thrd_i);
    MPP_ASSERT(!ret);
    ret = mpp_enc_cfg_get_st(cfg, "hw:aq_step_i", aq_thrd_i_ret);
    MPP_ASSERT(!ret);

    MPP_VERBOSE(flag, "after  get: aq_step_i: "
                "%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n",
                aq_thrd_i_ret[0], aq_thrd_i_ret[1], aq_thrd_i_ret[2], aq_thrd_i_ret[3],
                aq_thrd_i_ret[4], aq_thrd_i_ret[5], aq_thrd_i_ret[6], aq_thrd_i_ret[7],
                aq_thrd_i_ret[8], aq_thrd_i_ret[9], aq_thrd_i_ret[10], aq_thrd_i_ret[11],
                aq_thrd_i_ret[12], aq_thrd_i_ret[13], aq_thrd_i_ret[14], aq_thrd_i_ret[15]);

    for (i = 0; i < 16; i++) {
        if (aq_thrd_i_ret[i] != aq_thrd_i[i]) {
            mpp_loge("aq_step_i[%d] mismatch: got %d expect %d\n",
                     i, aq_thrd_i_ret[i], aq_thrd_i[i]);
            MPP_FAIL();
        }
    }

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_extract)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    char *buf = NULL;
    rk_s32 ret;

    MPP_VERBOSE(flag, "extract to LOG:\n");
    ret = mpp_enc_cfg_extract(cfg, MPP_CFG_STR_FMT_LOG, &buf);
    MPP_ASSERT(ret == 0 && buf != NULL);

    if (flag & MPP_FLAG_VERBOSE)
        mpp_cfg_print_string(buf);

    MPP_FREE(buf);
    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_apply_scalar)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    const char *json = "{\"rc\":{\"mode\":2,\"bps_target\":800000}}";
    rk_s32 mode = 0;
    rk_s32 bps = 0;
    rk_s32 ret;

    MPP_VERBOSE(flag, "apply JSON: %s\n", json);
    ret = mpp_enc_cfg_apply(cfg, MPP_CFG_STR_FMT_JSON, (char *)json);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_s32(cfg, "rc:mode", &mode);
    mpp_enc_cfg_get_s32(cfg, "rc:bps_target", &bps);
    MPP_VERBOSE(flag, "after apply: rc:mode=%d (expect 2) bps_target=%d (expect 800000)\n", mode, bps);

    MPP_ASSERT_EQ(2, mode);
    MPP_ASSERT_EQ(800000, bps);

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_apply_array)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    const char *json = "{\"hw\":{\"aq_step_i\":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]}}";
    rk_s32 aq_step_ret[16] = {0};
    rk_s32 ret;
    rk_s32 i;

    MPP_VERBOSE(flag, "apply JSON: %s\n", json);
    ret = mpp_enc_cfg_apply(cfg, MPP_CFG_STR_FMT_JSON, (char *)json);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_st(cfg, "hw:aq_step_i", aq_step_ret);
    if (flag & MPP_FLAG_VERBOSE) {
        char _abuf[128];
        rk_s32 _pos = 0;
        for (i = 0; i < 16; i++)
            _pos += snprintf(_abuf + _pos, sizeof(_abuf) - _pos, " %d", aq_step_ret[i]);
        mpp_logi("after apply: aq_step_i:%s", _abuf);
    }

    for (i = 0; i < 16; i++) {
        if (aq_step_ret[i] != i + 1) {
            mpp_loge("aq_step_i[%d] = %d, expect %d\n", i, aq_step_ret[i], i + 1);
            MPP_FAIL();
        }
    }

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_apply_qbias)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    const char *json = "{\"hw\":{\"qbias_arr\":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]}}";
    rk_s32 qbias_ret[18] = {0};
    rk_s32 i;
    rk_s32 ret;

    MPP_VERBOSE(flag, "apply JSON: %s\n", json);
    ret = mpp_enc_cfg_apply(cfg, MPP_CFG_STR_FMT_JSON, (char *)json);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_st(cfg, "hw:qbias_arr", qbias_ret);
    if (flag & MPP_FLAG_VERBOSE) {
        char _abuf[128];
        rk_s32 _pos = 0;
        for (i = 0; i < 18; i++)
            _pos += snprintf(_abuf + _pos, sizeof(_abuf) - _pos, " %d", qbias_ret[i]);
        mpp_logi("after apply: qbias_arr:%s", _abuf);
    }

    for (i = 0; i < 18; i++) {
        if (qbias_ret[i] != i) {
            mpp_loge("qbias_arr[%d] = %d, expect %d\n", i, qbias_ret[i], i);
            MPP_FAIL();
        }
    }

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_roundtrip)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    char *buf = NULL;
    rk_s32 mode_after = 0;
    rk_s32 ret;

    mpp_enc_cfg_set_s32(cfg, "rc:mode", 0);
    mpp_enc_cfg_set_s32(cfg, "rc:bps_target", 100000);

    ret = mpp_enc_cfg_extract(cfg, MPP_CFG_STR_FMT_JSON, &buf);
    MPP_ASSERT(ret == 0 && buf != NULL);

    MPP_VERBOSE(flag, "roundtrip saved JSON:\n%s\n", buf);

    /* modify value */
    mpp_enc_cfg_set_s32(cfg, "rc:mode", 3);

    /* restore from saved JSON */
    ret = mpp_enc_cfg_apply(cfg, MPP_CFG_STR_FMT_JSON, buf);
    MPP_FREE(buf);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_s32(cfg, "rc:mode", &mode_after);
    MPP_VERBOSE(flag, "after restore: rc:mode=%d (expect 0)\n", mode_after);

    MPP_ASSERT_EQ(0, mode_after);

    MPP_PASS();
done:
    return _mpp_ret;
}

MPP_TEST(test_json_string)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    MppEncCfg cfg = tc->cfg;
    rk_u32 flag = tc->flag;
    const char *json = "{\"rc\":{\"mode\":5,\"bps_target\":200000},"
                       "\"hw\":{\"aq_step_i\":[10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160]}}";
    rk_s32 mode = 0;
    rk_s32 bps = 0;
    rk_s32 aq_step_ret[16] = {0};
    rk_s32 i;
    rk_s32 ret;

    MPP_VERBOSE(flag, "apply JSON: %s\n", json);
    ret = mpp_enc_cfg_apply(cfg, MPP_CFG_STR_FMT_JSON, (char *)json);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_s32(cfg, "rc:mode", &mode);
    mpp_enc_cfg_get_s32(cfg, "rc:bps_target", &bps);
    MPP_VERBOSE(flag, "after apply: rc:mode=%d (expect 5) bps_target=%d (expect 200000)\n", mode, bps);

    MPP_ASSERT_EQ(5, mode);
    MPP_ASSERT_EQ(200000, bps);

    mpp_enc_cfg_get_st(cfg, "hw:aq_step_i", aq_step_ret);
    if (flag & MPP_FLAG_VERBOSE) {
        char _abuf[128];
        rk_s32 _pos = 0;
        for (i = 0; i < 16; i++)
            _pos += snprintf(_abuf + _pos, sizeof(_abuf) - _pos, " %d", aq_step_ret[i]);
        mpp_logi("after apply: aq_step_i:%s", _abuf);
    }

    for (i = 0; i < 16; i++) {
        if (aq_step_ret[i] != (i + 1) * 10) {
            mpp_loge("aq_step_i[%d] = %d, expect %d\n", i, aq_step_ret[i], (i + 1) * 10);
            MPP_FAIL();
        }
    }

    MPP_PASS();
done:
    return _mpp_ret;
}

/*
 * test_kobj_roundtrip — verify apply/extract on a kernel-backed cfg object.
 *
 * Unlike the user-space cfg used by other cases, a kobj's entry lives in kernel
 * shared memory (KmppVencStCfg layout).  If mpp_enc_cfg_apply/extract use the
 * user-space objdef's cfg_root (MppEncCfgSet layout), the field offsets mismatch
 * and the round-trip corrupts data.  This case exposes that.
 */
MPP_TEST(test_kobj_roundtrip)
{
    MPP_RET_VARS;
    TestCtx *tc = (TestCtx *)ctx;
    rk_u32 flag = tc->flag;
    MppEncCfg kcfg = NULL;
    char *buf = NULL;
    rk_s32 mode_before = 0;
    rk_s32 bps_before = 0;
    rk_s32 mode_after = 0;
    rk_s32 bps_after = 0;
    rk_s32 ret;

    /* create a kernel-backed st_cfg object */
    ret = mpp_venc_kcfg_init((MppVencKcfg *)&kcfg, MPP_VENC_KCFG_TYPE_ST_CFG);
    if (ret || !kcfg) {
        mpp_logi("kobj path skipped (no kmpp st_cfg objdef)\n");
        MPP_PASS();
    }
    MPP_ASSERT(kcfg != NULL);

    /* seed via ioctl-backed accessors (kernel-correct offsets) */
    mpp_enc_cfg_set_s32(kcfg, "rc:mode", 7);
    mpp_enc_cfg_set_s32(kcfg, "rc:bps_target", 654321);

    mpp_enc_cfg_get_s32(kcfg, "rc:mode", &mode_before);
    mpp_enc_cfg_get_s32(kcfg, "rc:bps_target", &bps_before);
    MPP_VERBOSE(flag, "kobj before: rc:mode=%d bps_target=%d\n", mode_before, bps_before);

    /* extract to JSON — uses cfg_root, may read wrong offsets on kobj */
    ret = mpp_enc_cfg_extract(kcfg, MPP_CFG_STR_FMT_JSON, &buf);
    MPP_ASSERT(ret == 0 && buf != NULL);
    MPP_VERBOSE(flag, "kobj extracted JSON:\n%s\n", buf);

    /* perturb, then restore from the saved JSON via apply */
    mpp_enc_cfg_set_s32(kcfg, "rc:mode", 0);
    mpp_enc_cfg_set_s32(kcfg, "rc:bps_target", 0);

    ret = mpp_enc_cfg_apply(kcfg, MPP_CFG_STR_FMT_JSON, buf);
    MPP_FREE(buf);
    MPP_ASSERT(!ret);

    mpp_enc_cfg_get_s32(kcfg, "rc:mode", &mode_after);
    mpp_enc_cfg_get_s32(kcfg, "rc:bps_target", &bps_after);
    MPP_VERBOSE(flag, "kobj after restore: rc:mode=%d (expect %d) bps_target=%d (expect %d)\n",
                mode_after, mode_before, bps_after, bps_before);

    MPP_ASSERT_EQ(mode_before, mode_after);
    MPP_ASSERT_EQ(bps_before, bps_after);

    MPP_PASS();
done:
    if (kcfg)
        mpp_enc_cfg_deinit(kcfg);
    return _mpp_ret;
}

static rk_s32 test_apply_from_file(MppEncCfg cfg, rk_u32 flag, const char *path)
{
    char *fbuf = NULL;
    MppCfgStrFmt fmt = MPP_CFG_STR_FMT_JSON;
    char *ext;
    rk_s32 ret = 0;

    ext = strrchr(path, '.');
    if (ext && !strcmp(ext, ".toml"))
        fmt = MPP_CFG_STR_FMT_TOML;

    MPP_VERBOSE(flag, "apply from file %s (format %s)\n", path,
                (fmt == MPP_CFG_STR_FMT_JSON) ? "JSON" : "TOML");

    ret = mpp_enc_cfg_apply_file(cfg, fmt, path);
    if (ret) {
        mpp_loge("apply from file %s failed ret %d\n", path, ret);
        return ret;
    }

    /* extract and show result */
    ret = mpp_enc_cfg_extract(cfg, MPP_CFG_STR_FMT_JSON, &fbuf);
    if (ret == 0 && fbuf) {
        if (flag & MPP_FLAG_VERBOSE)
            mpp_logi("after apply:\n%s\n", fbuf);
        MPP_FREE(fbuf);
    }

    return ret;
}

static MppTestCase test_cases[] = {
    { "scalar set/get",            MPP_FLAG_VERBOSE, 0, test_scalar_set_get },
    { "array set/get",             MPP_FLAG_VERBOSE, 0, test_array_set_get  },
    { "extract LOG",               MPP_FLAG_VERBOSE, 1, test_extract        },
    { "apply scalar",              MPP_FLAG_VERBOSE, 1, test_apply_scalar   },
    { "apply array (aq_step)",     MPP_FLAG_VERBOSE, 1, test_apply_array    },
    { "apply array (qbias)",       MPP_FLAG_VERBOSE, 1, test_apply_qbias    },
    { "roundtrip extract/apply",   MPP_FLAG_VERBOSE, 2, test_roundtrip      },
    { "JSON string apply",         MPP_FLAG_VERBOSE, 2, test_json_string    },
    { "kobj roundtrip",            MPP_FLAG_VERBOSE, 2, test_kobj_roundtrip },
};

int main(int argc, char *argv[])
{
    MPP_RET_VARS;
    MppEncCfg cfg = NULL;
    TestCtx tc;
    rk_u32 test_count = MPP_ARRAY_ELEMS(test_cases);
    rk_s32 start = 0;
    rk_s32 end = test_count - 1;
    rk_s32 ret;

    memset(&tc, 0, sizeof(tc));
    ret = mpp_test_parse_opts(&tc, test_opts,
                              MPP_ARRAY_ELEMS(test_opts), argc, argv);

    mpp_enc_cfg_show();

    /* show cfg tree detail */
    {
        KmppObjDef def = mpp_enc_cfg_objdef();

        if (def) {
            MppCfgObj cfg_root = kmpp_objdef_get_cfg_root(def);

            mpp_logi("cfg tree dump:\n");
            if (cfg_root)
                mpp_cfg_dump(cfg_root, "enc_cfg");
            else
                mpp_loge("cfg root is NULL\n");
        } else {
            mpp_loge("objdef is NULL\n");
        }
    }

    MPP_TEST_START("mpp_enc_cfg_test");

    _mpp_ret = mpp_enc_cfg_init(&cfg);
    if (_mpp_ret) {
        mpp_loge("mpp_enc_cfg_init failed\n");
        goto done;
    }

    tc.cfg = cfg;

    /* optional: apply config from file (first non-option arg) */
    if (ret < argc) {
        mpp_logi("\n--- Apply from file: %s ---\n", argv[ret]);
        _mpp_ret = test_apply_from_file(cfg, tc.flag, argv[ret]);
        if (_mpp_ret)
            goto done;
    }

    /* optional: specify test range (1-based index) */
    if (ret + 1 < argc)
        start = MPP_MAX(0, MPP_MIN(atoi(argv[ret + 1]) - 1, (rk_s32)test_count - 1));
    if (ret + 2 < argc)
        end = MPP_MAX(start, MPP_MIN(atoi(argv[ret + 2]) - 1, (rk_s32)test_count - 1));

    MPP_TEST_RUN_RANGE(&tc, test_cases, test_count, start, end);
    MPP_TEST_END("mpp_enc_cfg_test");

done:
    if (cfg)
        mpp_enc_cfg_deinit(cfg);
    return _mpp_ret;
}

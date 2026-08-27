/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2015 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_enc_ref"

#include <string.h>

#include "mpp_log.h"
#include "mpp_trie.h"
#include "mpp_common.h"

#include "mpp_cfg_io.h"
#include "mpp_enc_ref.h"
#include "mpp_enc_refs.h"
#include "rk_venc_kcfg.h"

#define ENC_REF_POS_SEEK(ref, pos, arr, idx, name) do {                      \
    rk_s32 _r = kmpp_obj_pos_seek((ref), (pos), (name), (idx));              \
    if (_r) {                                                                \
        mpp_loge_f("kobj %s [%d] seek failed ret %d\n", (arr), (idx), _r);   \
        return MPP_NOK;                                                      \
    }                                                                        \
} while (0)

#define ENC_REF_ST_POS_GET_FIELD(ref, pos, idx, field, val) do {             \
    rk_s32 _r = kmpp_obj_pos_get_s32((ref), (pos), #field, (val));           \
    if (_r) {                                                                \
        mpp_loge_f("kobj st_cfg [%d] get %s failed ret %d\n",                \
                   (idx), #field, _r);                                       \
        return MPP_NOK;                                                      \
    }                                                                        \
} while (0)

#define ENC_REF_ST_POS_SET_FIELD(ref, pos, idx, field, val) do {             \
    rk_s32 _r = kmpp_obj_pos_set_s32((ref), (pos), #field, (val));           \
    if (_r) {                                                                \
        mpp_loge_f("kobj st_cfg [%d] set %s failed ret %d\n",                \
                   (idx), #field, _r);                                       \
        return MPP_NOK;                                                      \
    }                                                                        \
} while (0)

#define ENC_REF_LT_POS_GET_FIELD(ref, pos, idx, field, val) do {             \
    rk_s32 _r = kmpp_obj_pos_get_s32((ref), (pos), #field, (val));           \
    if (_r) {                                                                \
        mpp_loge_f("kobj lt_cfg [%d] get %s failed ret %d\n",                \
                   (idx), #field, _r);                                       \
        return MPP_NOK;                                                      \
    }                                                                        \
} while (0)

#define ENC_REF_LT_POS_SET_FIELD(ref, pos, idx, field, val) do {             \
    rk_s32 _r = kmpp_obj_pos_set_s32((ref), (pos), #field, (val));           \
    if (_r) {                                                                \
        mpp_loge_f("kobj lt_cfg [%d] set %s failed ret %d\n",                \
                   (idx), #field, _r);                                       \
        return MPP_NOK;                                                      \
    }                                                                        \
} while (0)

/*
 * kmpp_obj definition for MppEncRefCfgImpl
 */
#define MPP_ENC_REF_CFG_ENTRY_TABLE(prefix, ENTRY, STRCT, EHOOK, SHOOK, ALIAS) \
    CFG_DEF_START() \
    ENTRY(prefix, s32,  rk_s32,     keep_cpb,       FLAG_INCR,      keep_cpb) \
    ENTRY(prefix, s32,  rk_s32,     lt_cfg_cap,     FLAG_INCR,      new_lt_cfg_cap) \
    ENTRY(prefix, s32,  rk_s32,     st_cfg_cap,     FLAG_INCR,      new_st_cfg_cap) \
    ENTRY(prefix, u32,  rk_u32,     lt_cfg_off,     FLAG_NONE,      lt_cfg_off) \
    ENTRY(prefix, u32,  rk_u32,     st_cfg_off,     FLAG_NONE,      st_cfg_off) \
    ENTRY(prefix, s32,  rk_s32,     lt_cfg_cnt,     FLAG_INCR,      lt_cfg_cnt) \
    ENTRY(prefix, s32,  rk_s32,     st_cfg_cnt,     FLAG_INCR,      st_cfg_cnt) \
    ENTRY(prefix, s32,  rk_s32,     max_tlayers,    FLAG_INCR,      max_tlayers) \
    ENTRY(prefix, s32,  rk_s32,     ready,          FLAG_INCR,      ready) \
    ARRAY_START_FLEX_CNT_OFF(st_cfg, MppEncRefStFrmCfg, FLAG_INCR,  st_cfg_cap, st_cfg_off) \
    ARRAY_ENTRY(s32,                is_non_ref,     FLAG_PREV,      is_non_ref) \
    ARRAY_ENTRY(s32,                temporal_id,    FLAG_PREV,      temporal_id) \
    ARRAY_ENTRY(s32,                ref_mode,       FLAG_PREV,      ref_mode) \
    ARRAY_ENTRY(s32,                ref_arg,        FLAG_PREV,      ref_arg) \
    ARRAY_ENTRY(s32,                repeat,         FLAG_PREV,      repeat) \
    ARRAY_END(st_cfg) \
    ARRAY_START_FLEX_CNT_OFF(lt_cfg, MppEncRefLtFrmCfg, FLAG_INCR,  lt_cfg_cap, lt_cfg_off) \
    ARRAY_ENTRY(s32,                lt_idx,         FLAG_PREV,      lt_idx) \
    ARRAY_ENTRY(s32,                temporal_id,    FLAG_PREV,      temporal_id) \
    ARRAY_ENTRY(s32,                ref_mode,       FLAG_PREV,      ref_mode) \
    ARRAY_ENTRY(s32,                ref_arg,        FLAG_PREV,      ref_arg) \
    ARRAY_ENTRY(s32,                lt_gap,         FLAG_PREV,      lt_gap) \
    ARRAY_ENTRY(s32,                lt_delay,       FLAG_PREV,      lt_delay) \
    ARRAY_END(lt_cfg) \
    CFG_DEF_END()

static rk_s32 mpp_enc_ref_cfg_impl_dump(void *entry)
{
    MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)entry;
    rk_s32 i;

    if (!cfg) {
        mpp_loge_f("invalid param entry NULL\n");
        return rk_nok;
    }

    mpp_logi("keep_cpb      %d\n", cfg->keep_cpb);
    mpp_logi("st_cfg_cnt    %d / %d  off %u\n",
             cfg->st_cfg_cnt, cfg->st_cfg_cap, cfg->st_cfg_off);
    mpp_logi("lt_cfg_cnt    %d / %d  off %u\n",
             cfg->lt_cfg_cnt, cfg->lt_cfg_cap, cfg->lt_cfg_off);
    mpp_logi("max_tlayers   %d\n", cfg->max_tlayers);
    mpp_logi("ready         %d\n", cfg->ready);

    for (i = 0; i < cfg->st_cfg_cnt; i++) {
        MppEncRefStFrmCfg *st = &MPP_REF_ST_ARR(cfg)[i];
        mpp_logi("  st[%d] non_ref %d tid %d mode %x arg %d repeat %d\n",
                 i, st->is_non_ref, st->temporal_id,
                 st->ref_mode, st->ref_arg, st->repeat);
    }

    for (i = 0; i < cfg->lt_cfg_cnt; i++) {
        MppEncRefLtFrmCfg *lt = &MPP_REF_LT_ARR(cfg)[i];
        mpp_logi("  lt[%d] idx %d tid %d mode %x arg %d gap %d delay %d\n",
                 i, lt->lt_idx, lt->temporal_id,
                 lt->ref_mode, lt->ref_arg, lt->lt_gap, lt->lt_delay);
    }

    return rk_ok;
}

static rk_s32 mpp_enc_ref_cfg_impl_resize(void *entry, KmppObj obj,
                                          const char *caller)
{
    MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)entry;
    KmppObjDef def = kmpp_obj_to_objdef(obj);
    rk_u32 old_lt_off = cfg->lt_cfg_off;
    rk_s32 data_off;

    (void)caller;

    data_off = kmpp_objdef_get_entry_size(def) + kmpp_obj_to_flags_size(obj);
    cfg->st_cfg_off = data_off;
    cfg->lt_cfg_off = data_off + cfg->new_st_cfg_cap * sizeof(MppEncRefStFrmCfg);

    /* relocate lt cfg data when offset shifts */
    {
        rk_s32 old_lt_cfg_cap = cfg->lt_cfg_cap;
        rk_s32 move_cnt = MPP_MIN3(cfg->lt_cfg_cnt, cfg->new_lt_cfg_cap, old_lt_cfg_cap);

        if (old_lt_off && cfg->lt_cfg_off != old_lt_off && move_cnt > 0)
            memmove((char *)cfg + cfg->lt_cfg_off, (char *)cfg + old_lt_off,
                    move_cnt * sizeof(MppEncRefLtFrmCfg));
    }

    /* commit new caps to actual fields */
    cfg->lt_cfg_cap = cfg->new_lt_cfg_cap;
    cfg->st_cfg_cap = cfg->new_st_cfg_cap;

    return rk_ok;
}

#define KMPP_OBJ_NAME               mpp_enc_ref_cfg
#define KMPP_OBJ_INTF_TYPE          MppEncRefCfg
#define KMPP_OBJ_IMPL_TYPE          MppEncRefCfgImpl
#define KMPP_OBJ_SGLN_ID            MPP_SGLN_ENC_REF_CFG
#define KMPP_OBJ_ENTRY_TABLE        MPP_ENC_REF_CFG_ENTRY_TABLE
#define KMPP_OBJ_FUNC_DUMP          mpp_enc_ref_cfg_impl_dump
#define KMPP_OBJ_FUNC_RESIZE        mpp_enc_ref_cfg_impl_resize
#define KMPP_OBJ_ACCESS_DISABLE
#define KMPP_OBJ_HIERARCHY_ENABLE
#define KMPP_OBJ_FLEX_ENTRY_ENABLE
#include "kmpp_obj_helper.h"

/*
 * ref_cfg_resize - set pending caps and resize VLA
 *
 * Stores requested caps in new_lt/st_cfg_cap, then calls kmpp_obj_resize.
 * The resize callback commits new_* to actual cap fields.
 * On failure, clears new_* (old state unchanged since callback not called).
 */
static rk_s32 ref_cfg_resize(MppEncRefCfg obj, rk_s32 new_lt_cap, rk_s32 new_st_cap,
                             const char *caller)
{
    rk_s32 old_lt_cap;
    rk_s32 old_st_cap;
    rk_s32 vla_size;
    rk_s32 ret;

    kmpp_obj_get_s32(obj, "lt_cfg_cap", &old_lt_cap);
    kmpp_obj_get_s32(obj, "st_cfg_cap", &old_st_cap);

    kmpp_obj_set_s32(obj, "lt_cfg_cap", new_lt_cap);
    kmpp_obj_set_s32(obj, "st_cfg_cap", new_st_cap);

    vla_size = new_st_cap * sizeof(MppEncRefStFrmCfg) + new_lt_cap * sizeof(MppEncRefLtFrmCfg);

    ret = kmpp_obj_resize(obj, vla_size, caller);
    if (ret) {
        mpp_loge("%s: resize to %d failed ret %d, revert to cap lt %d st %d\n",
                 caller, vla_size, ret, old_lt_cap, old_st_cap);
        kmpp_obj_set_s32(obj, "lt_cfg_cap", old_lt_cap);
        kmpp_obj_set_s32(obj, "st_cfg_cap", old_st_cap);
    }

    return ret;
}

MPP_RET mpp_enc_ref_cfg_setup(MppEncRefCfg obj, RK_S32 lt_cnt, RK_S32 st_cnt)
{
    rk_s32 ret;

    if (!obj || lt_cnt < 0 || st_cnt < 0) {
        mpp_loge_f("invalid param obj %p lt_cnt %d st_cnt %d\n", obj, lt_cnt, st_cnt);
        return MPP_NOK;
    }

    if (!kmpp_obj_to_entry(obj))
        return MPP_ERR_NULL_PTR;

    ret = ref_cfg_resize(obj, lt_cnt, st_cnt, __FUNCTION__);
    if (ret)
        return ret;

    kmpp_obj_set_s32(obj, "lt_cfg_cnt", 0);
    kmpp_obj_set_s32(obj, "st_cfg_cnt", 0);

    return MPP_OK;
}

/*
 * mpp_enc_ref_cfg API implementation
 */
MPP_RET mpp_enc_ref_cfg_init(MppEncRefCfg *ref)
{
    MPP_RET ret;

    if (NULL == ref) {
        mpp_loge_f("invalid NULL input ref_cfg\n");
        return MPP_ERR_NULL_PTR;
    }

    ret = mpp_enc_ref_cfg_get(ref);
    if (ret)
        return ret;

    return mpp_enc_ref_cfg_reset(*ref);
}

MPP_RET mpp_enc_ref_cfg_create(MppEncRefCfg *ref, RK_U32 mode)
{
    if (NULL == ref)
        return MPP_ERR_NULL_PTR;

    if (mode >= 2)
        return mpp_venc_kcfg_init((MppVencKcfg *)ref,
                                  MPP_VENC_KCFG_TYPE_REF_CFG);

    return mpp_enc_ref_cfg_init(ref);
}

MPP_RET mpp_enc_ref_cfg_deinit(MppEncRefCfg *ref)
{
    if (!ref || !*ref)
        return MPP_ERR_VALUE;

    MPP_RET ret = kmpp_obj_put_f(*ref);
    *ref = NULL;

    return ret;
}

MPP_RET mpp_enc_ref_cfg_reset(MppEncRefCfg ref)
{
    if (!ref)
        return MPP_ERR_VALUE;

    MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    MppEncRefStFrmCfg *st;
    rk_s32 vla_size = sizeof(MppEncRefStFrmCfg);
    rk_s32 ret;

    if (!cfg)
        return MPP_ERR_VALUE;

    /* restore default: st=1 simple forward reference */
    cfg->keep_cpb       = 0;
    cfg->new_st_cfg_cap = 1;
    cfg->new_lt_cfg_cap = 0;
    cfg->st_cfg_cap     = 1;
    cfg->lt_cfg_cap     = 0;
    cfg->st_cfg_cnt     = 0;
    cfg->lt_cfg_cnt     = 0;

    ret = kmpp_obj_resize_f(ref, vla_size);
    if (ret) {
        mpp_loge_f("resize to %d failed ret %d\n", vla_size, ret);
        return ret;
    }

    cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    st = MPP_REF_ST_ARR(cfg);

    st[0].is_non_ref   = 0;
    st[0].temporal_id  = 0;
    st[0].ref_mode     = REF_TO_PREV_REF_FRM;
    st[0].ref_arg      = 0;
    st[0].repeat       = 0;

    cfg->st_cfg_cnt    = 1;
    cfg->max_tlayers   = 1;
    cfg->ready         = 1;
    memset(&cfg->cpb_info, 0, sizeof(cfg->cpb_info));
    cfg->cpb_info.dpb_size    = 1;
    cfg->cpb_info.max_st_cnt  = 1;

    return MPP_OK;
}

MPP_RET mpp_enc_ref_cfg_set_cfg_cnt(MppEncRefCfg ref, RK_S32 lt_cnt, RK_S32 st_cnt)
{
    return mpp_enc_ref_cfg_setup(ref, lt_cnt, st_cnt);
}

MPP_RET mpp_enc_ref_cfg_add_lt_cfg(MppEncRefCfg ref, RK_S32 cnt, MppEncRefLtFrmCfg *frm)
{
    rk_s32 lt_cnt;
    rk_s32 lt_cap;
    rk_s32 i;
    rk_s32 j;

    if (!ref || !frm || cnt <= 0)
        return MPP_ERR_VALUE;

    if (!kmpp_obj_to_entry(ref))
        return MPP_ERR_VALUE;

    kmpp_obj_get_s32(ref, "lt_cfg_cnt", &lt_cnt);
    kmpp_obj_get_s32(ref, "lt_cfg_cap", &lt_cap);

    if (lt_cnt + cnt > lt_cap) {
        mpp_loge_f("lt_cfg overflow cnt %d + add %d > cap %d\n", lt_cnt, cnt, lt_cap);
        return MPP_ERR_VALUE;
    }

    if (kmpp_obj_is_kobj(ref)) {
        KmppObjPos pos;

        kmpp_obj_pos_init(&pos);

        for (i = 0, j = lt_cnt; i < cnt; i++, j++) {
            /* seek by name only on the first element; NULL reuses the same array */
            ENC_REF_POS_SEEK(ref, &pos, "lt_cfg", j, (i == 0) ? "lt_cfg" : NULL);

            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, lt_idx,      frm[i].lt_idx);
            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, temporal_id, frm[i].temporal_id);
            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, ref_mode,    frm[i].ref_mode);
            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, ref_arg,     frm[i].ref_arg);
            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, lt_gap,      frm[i].lt_gap);
            ENC_REF_LT_POS_SET_FIELD(ref, &pos, j, lt_delay,    frm[i].lt_delay);
        }
        kmpp_obj_set_s32(ref, "lt_cfg_cnt", lt_cnt + cnt);
    } else {
        MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);

        memcpy(&MPP_REF_LT_ARR(cfg)[cfg->lt_cfg_cnt], frm, sizeof(*frm) * cnt);
        cfg->lt_cfg_cnt += cnt;
    }

    return MPP_OK;
}

MPP_RET mpp_enc_ref_cfg_add_st_cfg(MppEncRefCfg ref, RK_S32 cnt, MppEncRefStFrmCfg *frm)
{
    rk_s32 st_cnt;
    rk_s32 st_cap;
    rk_s32 i;
    rk_s32 j;

    if (!ref || !frm || cnt <= 0)
        return MPP_ERR_VALUE;

    if (!kmpp_obj_to_entry(ref))
        return MPP_ERR_VALUE;

    kmpp_obj_get_s32(ref, "st_cfg_cnt", &st_cnt);
    kmpp_obj_get_s32(ref, "st_cfg_cap", &st_cap);

    if (st_cnt + cnt > st_cap) {
        mpp_loge_f("st_cfg overflow cnt %d + add %d > cap %d\n", st_cnt, cnt, st_cap);
        return MPP_ERR_VALUE;
    }

    if (kmpp_obj_is_kobj(ref)) {
        KmppObjPos pos;

        kmpp_obj_pos_init(&pos);

        for (i = 0, j = st_cnt; i < cnt; i++, j++) {
            /* seek by name only on the first element; NULL reuses the same array */
            ENC_REF_POS_SEEK(ref, &pos, "st_cfg", j, (i == 0) ? "st_cfg" : NULL);

            ENC_REF_ST_POS_SET_FIELD(ref, &pos, j, is_non_ref,  frm[i].is_non_ref);
            ENC_REF_ST_POS_SET_FIELD(ref, &pos, j, temporal_id, frm[i].temporal_id);
            ENC_REF_ST_POS_SET_FIELD(ref, &pos, j, ref_mode,    frm[i].ref_mode);
            ENC_REF_ST_POS_SET_FIELD(ref, &pos, j, ref_arg,     frm[i].ref_arg);
            ENC_REF_ST_POS_SET_FIELD(ref, &pos, j, repeat,      frm[i].repeat);
        }
        kmpp_obj_set_s32(ref, "st_cfg_cnt", st_cnt + cnt);
    } else {
        MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);

        memcpy(&MPP_REF_ST_ARR(cfg)[cfg->st_cfg_cnt], frm, sizeof(*frm) * cnt);
        cfg->st_cfg_cnt += cnt;
    }

    return MPP_OK;
}

MPP_RET mpp_enc_ref_cfg_check(MppEncRefCfg ref)
{
    if (!ref)
        return MPP_ERR_VALUE;

    if (kmpp_obj_is_kobj((KmppObj)ref))
        return kmpp_venc_ref_cfg_check((MppVencKcfg)ref);

    MppEncRefCfgImpl *p = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    RK_S32 lt_cfg_cnt = p->lt_cfg_cnt;
    RK_S32 st_cfg_cnt = p->st_cfg_cnt;
    RK_S32 max_lt_ref_cnt   = 0;
    RK_S32 max_lt_ref_idx   = 0;
    RK_S32 lt_idx_used_mask = 0;
    RK_S32 lt_dryrun_length = 0;
    RK_S32 max_st_ref_cnt   = 0;
    RK_S32 max_st_tid       = 0;
    RK_S32 st_tid_used_mask = 0;
    RK_S32 st_dryrun_length = 0;
    RK_S32 ready = 1;

    /* parse and check gop config for encoder */
    if (lt_cfg_cnt) {
        RK_S32 pos;
        MppEncRefLtFrmCfg *lt_cfg = MPP_REF_LT_ARR(p);

        for (pos = 0; pos < lt_cfg_cnt; pos++) {
            MppEncRefLtFrmCfg *c = &lt_cfg[pos];
            MppEncRefMode ref_mode = c->ref_mode;
            RK_S32 temporal_id = c->temporal_id;
            RK_S32 lt_idx = c->lt_idx;
            RK_U32 lt_idx_mask = 1 << lt_idx;

            /* check lt idx */
            if (lt_idx >= MPP_ENC_MAX_LT_REF_NUM) {
                mpp_loge_f("ref cfg %p lt cfg %d with invalid lt_idx %d larger than MPP_ENC_MAX_LT_REF_NUM\n",
                           ref, pos, lt_idx);
                ready = 0;
            }

            if (lt_idx_used_mask & lt_idx_mask) {
                mpp_loge_f("ref cfg %p lt cfg %d with redefined lt_idx %d config\n",
                           ref, pos, lt_idx);
                ready = 0;
            }

            if (!(lt_idx_used_mask & lt_idx_mask)) {
                lt_idx_used_mask |= lt_idx_mask;
                max_lt_ref_cnt++;
            }

            if (lt_idx > max_lt_ref_idx)
                max_lt_ref_idx = lt_idx;

            /* check temporal id */
            if (temporal_id != 0) {
                mpp_loge_f("ref cfg %p lt cfg %d with invalid temporal_id %d is non-zero\n",
                           ref, pos, temporal_id);
                ready = 0;
            }

            /* check gop mode */
            if (!REF_MODE_IS_GLOBAL(ref_mode) && !REF_MODE_IS_LT_MODE(ref_mode)) {
                mpp_loge_f("ref cfg %p lt cfg %d with invalid ref mode %x\n",
                           ref, pos, ref_mode);
                ready = 0;
            }

            /* if check failed just quit */
            if (!ready)
                break;

            if (c->lt_gap && (c->lt_gap + c->lt_delay > lt_dryrun_length))
                lt_dryrun_length = c->lt_gap + c->lt_delay;
        }
    }

    /* check st-ref config */
    if (ready && st_cfg_cnt) {
        RK_S32 pos;
        MppEncRefStFrmCfg *st_cfg = MPP_REF_ST_ARR(p);

        for (pos = 0; pos < st_cfg_cnt; pos++) {
            MppEncRefStFrmCfg *c = &st_cfg[pos];
            MppEncRefMode ref_mode = c->ref_mode;
            RK_S32 temporal_id = c->temporal_id;
            RK_U32 tid_mask = 1 << temporal_id;

            /* check temporal_id */
            if (temporal_id > MPP_ENC_MAX_TEMPORAL_LAYER_NUM - 1) {
                mpp_loge_f("ref cfg %p st cfg %d with invalid temporal_id %d larger than MPP_ENC_MAX_TEMPORAL_LAYER_NUM\n",
                           ref, pos, temporal_id);
                ready = 0;
            }

            /* check gop mode */
            if (!REF_MODE_IS_GLOBAL(ref_mode) && !REF_MODE_IS_ST_MODE(ref_mode)) {
                mpp_loge_f("ref cfg %p st cfg %d with invalid ref mode %x\n",
                           ref, pos, ref_mode);
                ready = 0;
            }

            if (c->repeat < 0) {
                mpp_loge_f("ref cfg %p st cfg %d with negative repeat %d set to zero\n",
                           ref, pos, c->repeat);
                c->repeat = 0;
            }

            /* constrain on head and tail frame */
            if (pos == 0 || (pos == st_cfg_cnt - 1)) {
                if (c->is_non_ref) {
                    mpp_loge_f("ref cfg %p st cfg %d with invalid non-ref frame on head/tail frame\n",
                               ref, pos);
                    ready = 0;
                }

                if (temporal_id > 0) {
                    mpp_loge_f("ref cfg %p st cfg %d with invalid non-zero temporal id %d on head/tail frame\n",
                               ref, pos, temporal_id);
                    ready = 0;
                }
            }

            if (!ready)
                break;

            if (!c->is_non_ref && !(st_tid_used_mask & tid_mask)) {
                max_st_ref_cnt++;
                st_tid_used_mask |= tid_mask;
            }

            if (temporal_id > max_st_tid)
                max_st_tid = temporal_id;

            st_dryrun_length++;
            st_dryrun_length += c->repeat;
        }
    }

    if (ready) {
        MppEncCpbInfo *cpb_info = &p->cpb_info;
        MppEncRefs refs = NULL;
        MPP_RET ret = MPP_OK;

        p->max_tlayers = max_st_tid + 1;

        cpb_info->dpb_size = 0;
        cpb_info->max_lt_cnt = max_lt_ref_cnt;
        cpb_info->max_st_cnt = max_st_ref_cnt;
        cpb_info->max_lt_idx = max_lt_ref_idx;
        cpb_info->max_st_tid = max_st_tid;
        cpb_info->lt_gop     = lt_dryrun_length;
        cpb_info->st_gop     = st_dryrun_length - 1;

        ret = mpp_enc_refs_init(&refs);
        ready = (ret) ? 0 : (ready);
        ret = mpp_enc_refs_set_cfg(refs, ref);
        ready = (ret) ? 0 : (ready);
        ret = mpp_enc_refs_dryrun(refs);
        ready = (ret) ? 0 : (ready);

        /* update dpb size */
        ret = mpp_enc_refs_get_cpb_info(refs, cpb_info);
        ready = (ret) ? 0 : (ready);

        ret = mpp_enc_refs_deinit(&refs);
        ready = (ret) ? 0 : (ready);
    } else {
        mpp_loge_f("check ref cfg %p failed\n", ref);
    }

    p->ready = ready;

    return ready ? MPP_OK : MPP_NOK;
}

MPP_RET mpp_enc_ref_cfg_set_keep_cpb(MppEncRefCfg ref, RK_S32 keep)
{
    if (!ref)
        return MPP_ERR_VALUE;

    MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    if (!cfg)
        return MPP_ERR_VALUE;

    cfg->keep_cpb = keep;

    return MPP_OK;
}

void mpp_enc_ref_cfg_show(void)
{
    KmppObjDef def = mpp_enc_ref_cfg_objdef();
    MppTrie trie = kmpp_objdef_get_trie(def);
    MppTrieInfo *node;
    rk_s32 len;

    if (!trie)
        return;

    len = mpp_trie_get_name_max(trie);

    mpp_logi("dumping ref_cfg entries start\n");

    node = mpp_trie_get_info_first(trie);
    while (node) {
        if (!mpp_trie_info_is_self(node) && node->ctx_len == sizeof(KmppEntry)) {
            KmppEntry *e = (KmppEntry *)mpp_trie_info_ctx(node);
            const char *name = mpp_trie_info_name(node);

            if (e->type == ENTRY_TYPE_VLA_INFO)
                mpp_logi("%-*s | vla    | base%c  %4d | size %4d | cnt%c %4d\n",
                         len, name,
                         e->vla.flex_base ? '@' : ' ',
                         e->vla.base_off,
                         e->vla.elem_size,
                         e->vla.flex_count ? '@' : ' ',
                         e->vla.flex_count ? e->vla.count_off : e->vla.elem_count);
            else
                mpp_logi("%-*s | %-6s | offset %4d | size %4d | flag %4x\n",
                         len, name, strof_elem_type(e->tbl.elem_type),
                         e->tbl.elem_offset, e->tbl.elem_size,
                         e->tbl.flag_offset);
        }

        node = mpp_trie_get_info_next(trie, node);
    }

    mpp_logi("dumping ref_cfg entries done\n");
}

static MPP_RET mpp_enc_ref_cfg_copy_k(MppEncRefCfg dst, MppEncRefCfg src)
{
    rk_s32 lt_cap;
    rk_s32 st_cap;
    rk_s32 lt_cnt;
    rk_s32 st_cnt;
    rk_s32 keep_cpb;
    rk_s32 max_tlayers;
    rk_s32 ready;
    rk_s32 val;
    KmppObjPos pos;
    rk_s32 i;
    rk_s32 ret;

    /* read src caps and resize dst to match */
    kmpp_obj_get_s32(src, "lt_cfg_cap", &lt_cap);
    kmpp_obj_get_s32(src, "st_cfg_cap", &st_cap);
    ret = ref_cfg_resize(dst, lt_cap, st_cap, __FUNCTION__);
    if (ret)
        return ret;

    /* read remaining src header scalars to copy */
    kmpp_obj_get_s32(src, "lt_cfg_cnt",  &lt_cnt);
    kmpp_obj_get_s32(src, "st_cfg_cnt",  &st_cnt);
    kmpp_obj_get_s32(src, "keep_cpb",    &keep_cpb);
    kmpp_obj_get_s32(src, "max_tlayers", &max_tlayers);
    kmpp_obj_get_s32(src, "ready",       &ready);

    kmpp_obj_set_s32(dst, "lt_cfg_cnt",  lt_cnt);
    kmpp_obj_set_s32(dst, "st_cfg_cnt",  st_cnt);
    kmpp_obj_set_s32(dst, "keep_cpb",    keep_cpb);
    kmpp_obj_set_s32(dst, "max_tlayers", max_tlayers);
    kmpp_obj_set_s32(dst, "ready",       ready);

    /* copy st_cfg array elements field by field via schema */
    for (i = 0; i < st_cnt; i++) {
        kmpp_obj_pos_init(&pos);
        ENC_REF_POS_SEEK(src, &pos, "st_cfg", i, (i == 0) ? "st_cfg" : NULL);

        ENC_REF_ST_POS_GET_FIELD(src, &pos, i, is_non_ref,  &val);
        ENC_REF_ST_POS_SET_FIELD(dst, &pos, i, is_non_ref,  val);
        ENC_REF_ST_POS_GET_FIELD(src, &pos, i, temporal_id, &val);
        ENC_REF_ST_POS_SET_FIELD(dst, &pos, i, temporal_id, val);
        ENC_REF_ST_POS_GET_FIELD(src, &pos, i, ref_mode,    &val);
        ENC_REF_ST_POS_SET_FIELD(dst, &pos, i, ref_mode,    val);
        ENC_REF_ST_POS_GET_FIELD(src, &pos, i, ref_arg,     &val);
        ENC_REF_ST_POS_SET_FIELD(dst, &pos, i, ref_arg,     val);
        ENC_REF_ST_POS_GET_FIELD(src, &pos, i, repeat,      &val);
        ENC_REF_ST_POS_SET_FIELD(dst, &pos, i, repeat,      val);
    }

    /* copy lt_cfg array elements field by field via schema */
    for (i = 0; i < lt_cnt; i++) {
        kmpp_obj_pos_init(&pos);
        ENC_REF_POS_SEEK(src, &pos, "lt_cfg", i, (i == 0) ? "lt_cfg" : NULL);

        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, lt_idx,      &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, lt_idx,      val);
        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, temporal_id, &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, temporal_id, val);
        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, ref_mode,    &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, ref_mode,    val);
        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, ref_arg,     &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, ref_arg,     val);
        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, lt_gap,      &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, lt_gap,      val);
        ENC_REF_LT_POS_GET_FIELD(src, &pos, i, lt_delay,    &val);
        ENC_REF_LT_POS_SET_FIELD(dst, &pos, i, lt_delay,    val);
    }

    return MPP_OK;
}

MPP_RET mpp_enc_ref_cfg_copy(MppEncRefCfg dst, MppEncRefCfg src)
{
    rk_s32 src_kobj;
    rk_s32 dst_kobj;

    if (!dst || !src)
        return MPP_ERR_VALUE;

    if (!kmpp_obj_to_entry(src) || !kmpp_obj_to_entry(dst))
        return MPP_ERR_VALUE;

    src_kobj = kmpp_obj_is_kobj((KmppObj)src);
    dst_kobj = kmpp_obj_is_kobj((KmppObj)dst);
    if (src_kobj != dst_kobj) {
        mpp_loge_f("src/dst kind mismatch: src_kobj %d dst_kobj %d\n",
                   src_kobj, dst_kobj);
        return MPP_ERR_VALUE;
    }

    if (src_kobj)
        return mpp_enc_ref_cfg_copy_k(dst, src);

    MppEncRefCfgImpl *s = (MppEncRefCfgImpl *)kmpp_obj_to_entry(src);
    MppEncRefCfgImpl *d = (MppEncRefCfgImpl *)kmpp_obj_to_entry(dst);
    rk_s32 ret;

    ret = ref_cfg_resize(dst, s->lt_cfg_cap, s->st_cfg_cap, __FUNCTION__);
    if (ret)
        return ret;

    d = (MppEncRefCfgImpl *)kmpp_obj_to_entry(dst);

    /* copy header (offsets will be the same, overwrite is harmless) */
    *d = *s;

    /* copy array data */
    if (s->st_cfg_cnt)
        memcpy(MPP_REF_ST_ARR(d), MPP_REF_ST_ARR(s),
               s->st_cfg_cnt * sizeof(MppEncRefStFrmCfg));

    if (s->lt_cfg_cnt)
        memcpy(MPP_REF_LT_ARR(d), MPP_REF_LT_ARR(s),
               s->lt_cfg_cnt * sizeof(MppEncRefLtFrmCfg));

    return MPP_OK;
}

MppEncCpbInfo *mpp_enc_ref_cfg_get_cpb_info(MppEncRefCfg ref)
{
    if (!ref)
        return NULL;

    MppEncRefCfgImpl *cfg = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    if (!cfg)
        return NULL;

    return &cfg->cpb_info;
}

/*
 * JSON config apply / extract — symmetric with mpp_enc_cfg_apply/extract
 */
static MPP_RET mpp_enc_ref_cfg_apply_obj(MppEncRefCfg ref, MppCfgObj obj, MppCfgStrFmt fmt)
{
    MppCfgObj root = NULL;
    MppEncRefCfgImpl *impl;
    RK_S32 st_cnt = 0;
    RK_S32 lt_cnt = 0;
    MPP_RET ret = MPP_NOK;

    /* use the obj's own objdef so kobj entries carry kernel-layout offsets */
    root = kmpp_objdef_get_cfg_root(kmpp_obj_to_objdef((KmppObj)ref));
    if (!root)
        root = kmpp_objdef_get_cfg_root(mpp_enc_ref_cfg_def);
    impl = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);

    /* read VLA capacity from parsed tree before to_struct */
    {
        MppCfgObj node = NULL;
        char st_cnt_name[] = "st_cfg_cnt";
        char lt_cnt_name[] = "lt_cfg_cnt";
        MppCfgVal val = { 0 };

        if (mpp_cfg_find(&node, obj, st_cnt_name, fmt) || !node) {
            mpp_loge_f("failed to find st_cfg_cnt\n");
            goto done;
        }
        if (mpp_cfg_get_val(node, MPP_CFG_TYPE_s32, &val)) {
            mpp_loge_f("failed to read st_cfg_cnt\n");
            goto done;
        }
        st_cnt = val.s32;

        node = NULL;
        if (mpp_cfg_find(&node, obj, lt_cnt_name, fmt) || !node) {
            mpp_loge_f("failed to find lt_cfg_cnt\n");
            goto done;
        }
        if (mpp_cfg_get_val(node, MPP_CFG_TYPE_s32, &val)) {
            mpp_loge_f("failed to read lt_cfg_cnt\n");
            goto done;
        }
        lt_cnt = val.s32;
    }

    if (st_cnt < 0 || lt_cnt < 0 ||
        st_cnt > MPP_ENC_MAX_REF_CFG_NUM ||
        lt_cnt > MPP_ENC_MAX_REF_CFG_NUM) {
        mpp_loge_f("invalid count st %d lt %d (max %d)\n",
                   st_cnt, lt_cnt, MPP_ENC_MAX_REF_CFG_NUM);
        goto done;
    }

    /* resize VLA to count from config */
    if (st_cnt > impl->st_cfg_cap || lt_cnt > impl->lt_cfg_cap) {
        if (ref_cfg_resize(ref, lt_cnt, st_cnt, __FUNCTION__))
            goto done;

        impl = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    }

    if (mpp_cfg_to_struct(obj, root, impl)) {
        mpp_loge_f("failed to convert config to struct\n");
        goto done;
    }
    ret = mpp_enc_ref_cfg_check(ref);
    if (ret) {
        mpp_loge_f("ref cfg check failed\n");
        if (impl->st_cfg_cap > 0)
            memset(MPP_REF_ST_ARR(impl), 0, impl->st_cfg_cap * sizeof(MppEncRefStFrmCfg));
        if (impl->lt_cfg_cap > 0)
            memset(MPP_REF_LT_ARR(impl), 0, impl->lt_cfg_cap * sizeof(MppEncRefLtFrmCfg));
        impl->st_cfg_cnt = 0;
        impl->lt_cfg_cnt = 0;
    }

done:
    mpp_cfg_put_all(obj);
    return ret;
}

MPP_RET mpp_enc_ref_cfg_apply(MppEncRefCfg ref, MppCfgStrFmt fmt, char *buf)
{
    MppCfgObj obj = NULL;

    if (!ref || !buf)
        return MPP_NOK;

    if (mpp_cfg_from_string(&obj, fmt, buf) || !obj) {
        mpp_loge_f("failed to parse config string\n");
        mpp_cfg_put_all(obj);
        return MPP_NOK;
    }

    return mpp_enc_ref_cfg_apply_obj(ref, obj, fmt);
}

MPP_RET mpp_enc_ref_cfg_apply_file(MppEncRefCfg ref, MppCfgStrFmt fmt, const char *path)
{
    MppCfgObj obj = NULL;

    if (!ref || !path)
        return MPP_NOK;

    if (mpp_cfg_from_file(&obj, fmt, path) || !obj) {
        mpp_loge_f("failed to parse config file %s\n", path);
        mpp_cfg_put_all(obj);
        return MPP_NOK;
    }

    return mpp_enc_ref_cfg_apply_obj(ref, obj, fmt);
}

MPP_RET mpp_enc_ref_cfg_extract(MppEncRefCfg ref, MppCfgStrFmt fmt, char **buf)
{
    MppEncRefCfgImpl *impl;
    MppCfgObj root;
    MppCfgObj obj = NULL;
    rk_s32 ret;

    if (!ref || !buf)
        return MPP_NOK;

    *buf = NULL;

    impl = (MppEncRefCfgImpl *)kmpp_obj_to_entry(ref);
    root = kmpp_objdef_get_cfg_root(kmpp_obj_to_objdef((KmppObj)ref));
    if (!root)
        root = kmpp_objdef_get_cfg_root(mpp_enc_ref_cfg_def);

    ret = mpp_cfg_from_struct(&obj, root, impl);
    if (ret || !obj) {
        mpp_loge_f("failed to convert struct to config\n");
        return MPP_NOK;
    }

    ret = mpp_cfg_to_string(obj, fmt, buf);
    if (ret)
        mpp_loge_f("failed to convert config to string\n");

    mpp_cfg_put_all(obj);

    return *buf ? MPP_OK : MPP_NOK;
}

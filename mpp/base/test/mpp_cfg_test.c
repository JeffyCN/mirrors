/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2025 Rockchip Electronics Co., Ltd.
 */

#define MODULE_TAG "mpp_cfg_test"

#include <stdio.h>
#include <string.h>

#include "mpp_mem.h"
#include "mpp_debug.h"

#include "mpp_cfg_io.h"
#include "mpp_trie.h"
#include "kmpp_obj.h"

#define RUN_AND_CHECK(_tag, _call) do { \
        ret = (_call);                    \
        if (ret < 0) {                        \
            mpp_loge("%s " #_tag " failed %d\n", str, ret); \
            goto DONE;                     \
        }                                \
    } while (0)

#define RUN_CHECK_IDX(_tag, _call) do { \
        ret = (_call);                    \
        if (ret < 0) {                        \
            mpp_loge("%s " #_tag " idx %d failed %d\n", str, i, ret); \
            goto DONE;                     \
        }                                \
    } while (0)

static const char *str_fmt[] = {
    "log",
    "json",
    "toml",
    "invalid"
};

static rk_s32 add_array_element(MppCfgObj array, MppCfgType type, MppCfgVal *val)
{
    MppCfgObj obj = NULL;
    rk_s32 ret;

    ret = mpp_cfg_get_object(&obj, NULL, type, val);
    if (ret) {
        mpp_loge("mpp_cfg_get_object array element type %d failed\n", type);
        return ret;
    }

    ret = mpp_cfg_add(array, obj);
    if (ret) {
        mpp_loge("mpp_cfg_add array element failed\n");
        mpp_cfg_put_all(obj);
        return ret;
    }

    return rk_ok;
}

static rk_s32 test_typed_arrays(MppCfgObj root)
{
    MppCfgObj array = NULL;
    MppCfgVal val;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    mpp_logi("test typed arrays\n");

    ret = mpp_cfg_get_array(&array, "s8_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array s8 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.s8 = (rk_s8)(-128 + i * 10);
        ret = add_array_element(array, MPP_CFG_TYPE_s8, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add s8_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "u8_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array u8 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.u8 = (rk_u8)(i * 50);
        ret = add_array_element(array, MPP_CFG_TYPE_u8, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add u8_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "s16_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array s16 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.s16 = (rk_s16)(-1000 + i * 100);
        ret = add_array_element(array, MPP_CFG_TYPE_s16, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add s16_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "u16_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array u16 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.u16 = (rk_u16)(i * 1000);
        ret = add_array_element(array, MPP_CFG_TYPE_u16, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add u16_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "s64_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array s64 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.s64 = (rk_s64)(-1000000LL + i * 100000LL);
        ret = add_array_element(array, MPP_CFG_TYPE_s64, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add s64_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "u64_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array u64 failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.u64 = (rk_u64)(i * 1000000ULL);
        ret = add_array_element(array, MPP_CFG_TYPE_u64, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add u64_array failed\n");
        goto DONE;
    }
    array = NULL;

    ret = mpp_cfg_get_array(&array, "bool_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array bool failed\n");
        goto DONE;
    }
    for (i = 0; i < 4; i++) {
        val.b1 = (i % 2 == 0) ? (rk_bool)RK_TRUE : (rk_bool)RK_FALSE;
        ret = add_array_element(array, MPP_CFG_TYPE_BOOL, &val);
        if (ret)
            goto DONE;
    }
    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add bool_array failed\n");
        goto DONE;
    }
    array = NULL;

    {
        const char *str_values[] = {"str0", "str1", "str2", "str3"};
        ret = mpp_cfg_get_array(&array, "string_array");
        if (ret) {
            mpp_loge("mpp_cfg_get_array string failed\n");
            goto DONE;
        }
        for (i = 0; i < 4; i++) {
            val.str = (char *)str_values[i];
            ret = add_array_element(array, MPP_CFG_TYPE_STRING, &val);
            if (ret)
                goto DONE;
        }
        ret = mpp_cfg_add(root, array);
        if (ret) {
            mpp_loge("mpp_cfg_add string_array failed\n");
            goto DONE;
        }
        array = NULL;
    }

    ret = rk_ok;
DONE:
    return ret;
}

static rk_s32 test_object_array(MppCfgObj root)
{
    MppCfgObj array = NULL;
    MppCfgObj obj = NULL;
    MppCfgObj inner_obj = NULL;
    MppCfgVal val;
    rk_s32 ret = rk_nok;
    rk_s32 i;
    const char *obj_names[] = {"obj_a", "obj_b", "obj_c"};

    mpp_logi("test object array\n");

    ret = mpp_cfg_get_array(&array, "object_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array object failed\n");
        goto DONE;
    }

    for (i = 0; i < 3; i++) {
        ret = mpp_cfg_get_object(&obj, NULL, MPP_CFG_TYPE_OBJECT, NULL);
        if (ret) {
            mpp_loge("mpp_cfg_get_object for inner object failed\n");
            goto DONE;
        }

        val.s32 = i + 100;
        ret = mpp_cfg_get_object(&inner_obj, "id", MPP_CFG_TYPE_s32, &val);
        if (ret) {
            mpp_loge("mpp_cfg_get_object id failed\n");
            goto DONE;
        }
        ret = mpp_cfg_add(obj, inner_obj);
        if (ret) {
            mpp_loge("mpp_cfg_add id failed\n");
            goto DONE;
        }
        inner_obj = NULL;

        val.str = (char *)obj_names[i];
        ret = mpp_cfg_get_object(&inner_obj, "name", MPP_CFG_TYPE_STRING, &val);
        if (ret) {
            mpp_loge("mpp_cfg_get_object name failed\n");
            goto DONE;
        }
        ret = mpp_cfg_add(obj, inner_obj);
        if (ret) {
            mpp_loge("mpp_cfg_add name failed\n");
            goto DONE;
        }
        inner_obj = NULL;

        val.b1 = (i == 0) ? (rk_bool)RK_TRUE : (rk_bool)RK_FALSE;
        ret = mpp_cfg_get_object(&inner_obj, "active", MPP_CFG_TYPE_BOOL, &val);
        if (ret) {
            mpp_loge("mpp_cfg_get_object active failed\n");
            goto DONE;
        }
        ret = mpp_cfg_add(obj, inner_obj);
        if (ret) {
            mpp_loge("mpp_cfg_add active failed\n");
            goto DONE;
        }
        inner_obj = NULL;

        ret = mpp_cfg_add(array, obj);
        if (ret) {
            mpp_loge("mpp_cfg_add object to array failed\n");
            goto DONE;
        }
        obj = NULL;
    }

    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add object_array failed\n");
        goto DONE;
    }

    ret = rk_ok;
DONE:
    if (inner_obj)
        mpp_cfg_put_all(inner_obj);
    if (obj)
        mpp_cfg_put_all(obj);
    return ret;
}

static rk_s32 test_nested_array(MppCfgObj root)
{
    MppCfgObj outer_array = NULL;
    MppCfgObj inner_array = NULL;
    MppCfgVal val;
    rk_s32 ret = rk_nok;
    rk_s32 i, j;

    mpp_logi("test nested array\n");

    ret = mpp_cfg_get_array(&outer_array, "nested_array");
    if (ret) {
        mpp_loge("mpp_cfg_get_array outer failed\n");
        goto DONE;
    }

    for (i = 0; i < 3; i++) {
        ret = mpp_cfg_get_array(&inner_array, NULL);
        if (ret) {
            mpp_loge("mpp_cfg_get_array inner failed\n");
            goto DONE;
        }

        for (j = 0; j < 3; j++) {
            val.s32 = i * 10 + j;
            ret = add_array_element(inner_array, MPP_CFG_TYPE_s32, &val);
            if (ret)
                goto DONE;
        }

        ret = mpp_cfg_add(outer_array, inner_array);
        if (ret) {
            mpp_loge("mpp_cfg_add inner array failed\n");
            goto DONE;
        }
        inner_array = NULL;
    }

    ret = mpp_cfg_add(root, outer_array);
    if (ret) {
        mpp_loge("mpp_cfg_add nested_array failed\n");
        goto DONE;
    }

    ret = rk_ok;
DONE:
    if (inner_array)
        mpp_cfg_put_all(inner_array);
    return ret;
}

/*
 * Build a tree the way objdef macros do: every field carries a KmppEntry
 * (mpp_cfg_set_entry) and every array carries a VLA entry (mpp_cfg_set_vla).
 * This is the "simple objdef" shape mpp_cfg_to_trie / mpp_cfg_from_trie are
 * designed for, before dealing with hand-built (entry-less) trees.
 */
static rk_s32 test_trie_entryful(MppCfgObj *root_out)
{
    MppCfgObj root = NULL;
    MppCfgObj obj = NULL;
    MppCfgObj array = NULL;
    KmppEntry e;
    MppCfgVal val;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    mpp_logi("test trie entryful build\n");

    ret = mpp_cfg_get_object(&root, NULL, MPP_CFG_TYPE_OBJECT, NULL);
    if (ret)
        goto DONE;

    /* scalar fields with entries */
    ret = mpp_cfg_get_object(&obj, "width", MPP_CFG_TYPE_s32, NULL);
    if (ret)
        goto DONE;
    e.val = 0;
    e.tbl.type = ENTRY_TYPE_LOC_TBL;
    e.tbl.elem_type = ELEM_TYPE_s32;
    e.tbl.elem_size = sizeof(rk_s32);
    mpp_cfg_set_entry(obj, &e);
    mpp_cfg_add(root, obj);
    obj = NULL;

    ret = mpp_cfg_get_object(&obj, "height", MPP_CFG_TYPE_u32, NULL);
    if (ret)
        goto DONE;
    e.val = 0;
    e.tbl.type = ENTRY_TYPE_LOC_TBL;
    e.tbl.elem_type = ELEM_TYPE_u32;
    e.tbl.elem_size = sizeof(rk_u32);
    mpp_cfg_set_entry(obj, &e);
    mpp_cfg_add(root, obj);
    obj = NULL;

    /* nested struct with entries */
    ret = mpp_cfg_get_object(&obj, "prep", MPP_CFG_TYPE_OBJECT, NULL);
    if (ret)
        goto DONE;
    {
        MppCfgObj sub = NULL;

        ret = mpp_cfg_get_object(&sub, "width", MPP_CFG_TYPE_s32, NULL);
        if (ret)
            goto DONE;
        e.val = 0;
        e.tbl.type = ENTRY_TYPE_LOC_TBL;
        e.tbl.elem_type = ELEM_TYPE_s32;
        e.tbl.elem_size = sizeof(rk_s32);
        mpp_cfg_set_entry(sub, &e);
        mpp_cfg_add(obj, sub);
    }
    mpp_cfg_add(root, obj);
    obj = NULL;

    /* simple array with vla entry */
    ret = mpp_cfg_get_array(&array, "values");
    if (ret)
        goto DONE;
    e.val = 0;
    e.vla.type = ENTRY_TYPE_VLA_INFO;
    e.vla.elem_size = sizeof(rk_s32);
    e.vla.elem_count = 4;
    ret = mpp_cfg_set_vla(array, &e, MPP_CFG_TYPE_s32);
    if (ret)
        goto DONE;
    for (i = 0; i < 4; i++) {
        val.s32 = i * 10;
        ret = mpp_cfg_vla_add_raw(array, i, &val);
        if (ret)
            goto DONE;
    }
    mpp_cfg_add(root, array);
    array = NULL;

    *root_out = root;
    root = NULL;
    ret = rk_ok;

DONE:
    if (root)
        mpp_cfg_put_all(root);
    if (array)
        mpp_cfg_put_all(array);

    return ret;
}

/* real objdef getters built by the objdef macros (declared in .c) */
extern KmppObjDef mpp_enc_ref_cfg_objdef(void);
extern KmppObjDef mpp_enc_cfg_objdef(void);

/*
 * Step 2: verify to/from trie on real, macro-built objdef cfg_roots
 * (entry-complete, VLA arrays, nested structs). The trie is structure-only,
 * so compare the entry sets (name + type) of the two to_trie passes.
 */
typedef struct {
    char    name[128];
    rk_s32  type;
} TrieEntryRec;

typedef struct {
    TrieEntryRec *recs;
    rk_s32       cnt;
    rk_s32       cap;
} TrieEntrySet;

static rk_s32 trie_collect_cb(const char *name, void *info, rk_s32 subroot, void *arg)
{
    TrieEntrySet *set = (TrieEntrySet *)arg;
    KmppEntry *entry = (KmppEntry *)info;
    TrieEntryRec *grown;
    rk_s32 len;

    if (set->cnt >= set->cap) {
        rk_s32 new_cap = set->cap ? set->cap * 2 : 32;

        grown = mpp_realloc(set->recs, TrieEntryRec, new_cap);
        if (!grown)
            return rk_nok;
        set->recs = grown;
        set->cap = new_cap;
    }

    len = strlen(name);
    if (len >= (rk_s32)sizeof(set->recs[set->cnt].name))
        len = sizeof(set->recs[set->cnt].name) - 1;
    memcpy(set->recs[set->cnt].name, name, len);
    set->recs[set->cnt].name[len] = '\0';
    set->recs[set->cnt].type = entry->type;
    set->cnt++;

    (void)subroot;
    return rk_ok;
}

static rk_s32 test_objdef_trie_roundtrip(KmppObjDef def, const char *defname)
{
    MppCfgObj root = NULL;
    MppCfgObj rebuilt = NULL;
    MppTrie trie1 = NULL;
    MppTrie trie2 = NULL;
    TrieEntrySet set1 = {0};
    TrieEntrySet set2 = {0};
    rk_s32 ret = rk_nok;
    rk_s32 i, j;

    mpp_logi("test objdef %s trie roundtrip\n", defname);

    root = kmpp_objdef_get_cfg_root(def);
    if (!root) {
        mpp_loge("objdef %s cfg_root is NULL\n", defname);
        goto DONE;
    }

    trie1 = mpp_cfg_to_trie(root);
    if (!trie1) {
        mpp_loge("objdef %s to_trie failed\n", defname);
        goto DONE;
    }

    rebuilt = mpp_cfg_from_trie(trie1);
    if (!rebuilt) {
        mpp_loge("objdef %s from_trie failed\n", defname);
        goto DONE;
    }

    trie2 = mpp_cfg_to_trie(rebuilt);
    if (!trie2) {
        mpp_loge("objdef %s rebuilt to_trie failed\n", defname);
        goto DONE;
    }

    mpp_trie_for_each_entry(trie1, trie_collect_cb, &set1);
    mpp_trie_for_each_entry(trie2, trie_collect_cb, &set2);

    mpp_logi("objdef %s entries: %d -> %d\n", defname, set1.cnt, set2.cnt);

    if (set1.cnt != set2.cnt) {
        mpp_loge("objdef %s entry count mismatch\n", defname);
        goto DONE;
    }

    /* simple O(n^2) match: every entry in set1 must exist in set2 */
    for (i = 0; i < set1.cnt; i++) {
        rk_s32 found = 0;

        for (j = 0; j < set2.cnt; j++) {
            if (!strcmp(set1.recs[i].name, set2.recs[j].name) &&
                set1.recs[i].type == set2.recs[j].type) {
                found = 1;
                break;
            }
        }

        if (!found) {
            mpp_loge("objdef %s entry %s type %d missing in rebuilt\n",
                     defname, set1.recs[i].name, set1.recs[i].type);
            goto DONE;
        }
    }

    mpp_logi("objdef %s to/from trie roundtrip success (%d entries)\n",
             defname, set1.cnt);
    ret = rk_ok;

DONE:
    MPP_FREE(set1.recs);
    MPP_FREE(set2.recs);
    /* NOTE: trie1/trie2 are cached on their owning cfg trees; root is the
     * objdef's cfg_root (owned by the objdef), only rebuilt is ours */
    mpp_cfg_put_all(rebuilt);

    return ret;
}

static rk_s32 test_trie_roundtrip(MppCfgObj obj)
{
    MppCfgObj out = NULL;
    MppTrie trie1 = NULL;
    MppTrie trie2 = NULL;
    TrieEntrySet set1 = {0};
    TrieEntrySet set2 = {0};
    rk_s32 ret = rk_nok;
    rk_s32 i, j;

    mpp_logi("test to / from trie\n");

    trie1 = mpp_cfg_to_trie(obj);
    if (!trie1) {
        mpp_loge("mpp_cfg_to_trie failed\n");
        ret = rk_nok;
        goto DONE;
    }

    out = mpp_cfg_from_trie(trie1);
    if (!out) {
        mpp_loge("mpp_cfg_from_trie failed\n");
        ret = rk_nok;
        goto DONE;
    }

    trie2 = mpp_cfg_to_trie(out);
    if (!trie2) {
        mpp_loge("rebuilt to_trie failed\n");
        ret = rk_nok;
        goto DONE;
    }

    mpp_trie_for_each_entry(trie1, trie_collect_cb, &set1);
    mpp_trie_for_each_entry(trie2, trie_collect_cb, &set2);

    if (set1.cnt != set2.cnt) {
        mpp_loge("entry count mismatch %d -> %d\n", set1.cnt, set2.cnt);
        goto DONE;
    }

    for (i = 0; i < set1.cnt; i++) {
        rk_s32 found = 0;

        for (j = 0; j < set2.cnt; j++) {
            if (!strcmp(set1.recs[i].name, set2.recs[j].name) &&
                set1.recs[i].type == set2.recs[j].type) {
                found = 1;
                break;
            }
        }

        if (!found) {
            mpp_loge("entry %s type %d missing in rebuilt\n",
                     set1.recs[i].name, set1.recs[i].type);
            goto DONE;
        }
    }

    mpp_logi("mpp_cfg to/from trie structural roundtrip success (%d entries)\n",
             set1.cnt);
    ret = rk_ok;

DONE:
    MPP_FREE(set1.recs);
    MPP_FREE(set2.recs);
    /* NOTE: trie1/trie2 are cached on their owning cfg trees */
    mpp_cfg_put_all(out);

    return ret;
}

static rk_s32 test_to_from(MppCfgObj obj, MppCfgStrFmt fmt)
{
    MppCfgObj out = NULL;
    char *std = NULL;
    char *str = NULL;
    rk_s32 ret = rk_nok;

    ret = mpp_cfg_to_string(obj, fmt, &std);
    if (ret) {
        mpp_loge("mpp_cfg obj to %s string failed\n", str_fmt[fmt]);
        goto DONE;
    }
    if (!std || !std[0]) {
        mpp_loge("mpp_cfg obj to %s string returned empty\n", str_fmt[fmt]);
        ret = rk_nok;
        goto DONE;
    }

    ret = mpp_cfg_from_string(&out, fmt, std);
    if (ret) {
        mpp_loge("mpp_cfg out from %s string failed ret %d\n", str_fmt[fmt], ret);
        goto DONE;
    }
    if (!out) {
        mpp_loge("mpp_cfg out from %s string returned NULL object\n", str_fmt[fmt]);
        ret = rk_nok;
        goto DONE;
    }

    ret = mpp_cfg_to_string(out, fmt, &str);
    if (ret) {
        mpp_loge("mpp_cfg out to %s string failed ret %d\n", str_fmt[fmt], ret);
        goto DONE;
    }
    if (!str || !str[0]) {
        mpp_loge("mpp_cfg out to %s string returned empty\n", str_fmt[fmt]);
        ret = rk_nok;
        goto DONE;
    }

    if (strcmp(std, str)) {
        mpp_loge("mpp_cfg mismatch on from / to %s string\n", str_fmt[fmt]);
        mpp_logi("string std:\n");
        mpp_cfg_print_string(std);
        mpp_logi("string out:\n");
        mpp_cfg_print_string(str);
        ret = rk_nok;
    } else {
        ret = rk_ok;
    }

DONE:
    MPP_FREE(std);
    MPP_FREE(str);
    mpp_cfg_put_all(out);

    return ret;
}

static rk_s32 test_fix_raw_array(MppCfgObj root)
{
    const char *str = "fix_raw_arr";
    MppCfgObj array = NULL;
    MppCfgVal val;
    KmppEntry vla;
    rk_s32 elem_count = 4;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    /* vla_add_val: fixed-size simple type array */
    RUN_AND_CHECK(get_array, mpp_cfg_get_array(&array, str));

    vla.val = 0;
    vla.vla.type = ENTRY_TYPE_VLA_INFO;
    vla.vla.elem_size = sizeof(rk_s32);
    vla.vla.elem_count = elem_count;

    /* MPP_CFG_TYPE_s32 -> simple type */
    RUN_AND_CHECK(set_vla,   mpp_cfg_set_vla(array, &vla, MPP_CFG_TYPE_s32));

    /* check idx out of fixed range */
    val.s32 = 1000 + elem_count * 100;
    ret = mpp_cfg_vla_add_raw(array, elem_count, &val);
    if (ret != rk_nok) {
        mpp_loge("%s mpp_cfg_vla_add_raw idx %d must nok failed %d\n", str, ret);
        ret = rk_nok;
        goto DONE;
    }

    /* check idx in fixed range */
    for (i = 0; i < elem_count; i++) {
        val.s32 = 1000 + i * 100;
        RUN_CHECK_IDX(add_raw, mpp_cfg_vla_add_raw(array, i, &val));
    }

    RUN_AND_CHECK(add,      mpp_cfg_add(root, array));

    array = NULL;
    ret = rk_ok;

DONE:
    if (ret)
        mpp_cfg_put_all(array);

    mpp_logi("test %s %s\n", str, ret ? "failed" : "success");

    return ret;
}

static rk_s32 test_flex_raw_array(MppCfgObj root)
{
    const char *str = "flex_raw_arr";
    MppCfgObj array = NULL;
    MppCfgVal val;
    KmppEntry vla;
    rk_s32 elem_count = 4;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    /* vla_add_val: variable-count simple type array */
    RUN_AND_CHECK(get_array, mpp_cfg_get_array(&array, str));

    vla.val = 0;
    vla.vla.type = ENTRY_TYPE_VLA_INFO;
    vla.vla.flex_count = 1;
    vla.vla.elem_size = sizeof(rk_s32);
    vla.vla.elem_count = elem_count;

    /* MPP_CFG_TYPE_s32 -> simple type */
    RUN_AND_CHECK(set_vla, mpp_cfg_set_vla(array, &vla, MPP_CFG_TYPE_s32));

    /* check idx out of fixed range */
    val.s32 = -1;
    ret = mpp_cfg_vla_add_raw(array, elem_count * 2, &val);
    if (ret != rk_nok) {
        mpp_loge("%s mpp_cfg_vla_add_raw idx %d must nok failed %d\n", str, ret);
        ret = rk_nok;
        goto DONE;
    }

    for (i = 0; i < elem_count * 2; i++) {
        val.s32 = 1000 + i * 100;
        RUN_CHECK_IDX(add_raw, mpp_cfg_vla_add_raw(array, i, &val));
    }

    RUN_AND_CHECK(add, mpp_cfg_add(root, array));

    array = NULL;
    ret = rk_ok;

DONE:
    if (ret)
        mpp_cfg_put_all(array);

    mpp_logi("test %s %s\n", str, ret ? "failed" : "success");

    return ret;
}

static rk_s32 test_fix_elem_array(MppCfgObj root)
{
    const char *str = "fix_elem_arr";
    MppCfgObj array = NULL;
    MppCfgObj elem = NULL;
    MppCfgVal val;
    KmppEntry vla;
    rk_s32 elem_count = 4;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    /* vla_add_elem: fixed-size complex type array */
    RUN_AND_CHECK(get_array, mpp_cfg_get_array(&array, str));

    vla.val = 0;
    vla.vla.type = ENTRY_TYPE_VLA_INFO;
    vla.vla.elem_count = elem_count;

    RUN_AND_CHECK(set_vla,   mpp_cfg_set_vla(array, &vla, MPP_CFG_TYPE_OBJECT));

    /* check idx in fixed range */
    for (i = 0; i < elem_count; i++) {
        val.s32 = 100 + i;
        RUN_CHECK_IDX(get_obj,  mpp_cfg_get_object(&elem, NULL, MPP_CFG_TYPE_s32, &val));
        RUN_CHECK_IDX(add_elem, mpp_cfg_vla_add_elem(array, i, elem));
        elem = NULL;
    }

    /* check idx out of fixed range */
    val.s32 = 100 + elem_count;
    RUN_CHECK_IDX(get_obj,  mpp_cfg_get_object(&elem, NULL, MPP_CFG_TYPE_s32, &val));
    ret = mpp_cfg_vla_add_elem(array, elem_count, elem);
    if (ret != rk_nok) {
        mpp_loge("%s mpp_cfg_vla_add_elem idx %d must nok failed %d\n", str, ret);
        ret = rk_nok;
        goto DONE;
    }
    mpp_cfg_put_all(elem);
    elem = NULL;

    RUN_AND_CHECK(add,      mpp_cfg_add(root, array));

    array = NULL;
    ret = rk_ok;

DONE:
    if (ret) {
        mpp_cfg_put_all(array);
        mpp_cfg_put_all(elem);
    }

    mpp_logi("test %s %s\n", str, ret ? "failed" : "success");

    return ret;
}

static rk_s32 test_flex_elem_array(MppCfgObj root)
{
    const char *str = "flex_elem_arr";
    MppCfgObj array = NULL;
    MppCfgObj elem = NULL;
    MppCfgVal val;
    KmppEntry vla;
    rk_s32 elem_count = 3;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    /* vla_add_elem: fixed-size complex type array */
    RUN_AND_CHECK(get_array, mpp_cfg_get_array(&array, str));

    vla.val = 0;
    vla.vla.type = ENTRY_TYPE_VLA_INFO;
    vla.vla.flex_count  = 1;
    vla.vla.elem_count = elem_count;

    RUN_AND_CHECK(set_vla,   mpp_cfg_set_vla(array, &vla, MPP_CFG_TYPE_OBJECT));

    /* check idx out of fixed range */
    val.s64 = (rk_s64)(3000LL + elem_count * 2 * 111LL);
    RUN_AND_CHECK(get_obj,  mpp_cfg_get_object(&elem, NULL, MPP_CFG_TYPE_s32, &val));
    ret = mpp_cfg_vla_add_elem(array, elem_count * 2, elem);
    if (ret != rk_nok) {
        mpp_loge("%s mpp_cfg_vla_add_elem idx %d must nok failed %d\n", str, ret);
        ret = rk_nok;
        goto DONE;
    }
    mpp_cfg_put_all(elem);
    elem = NULL;

    /* check idx in fixed range */
    for (i = 0; i < elem_count * 2; i++) {
        val.s64 = (rk_s64)(3000LL + i * 111LL);
        RUN_AND_CHECK(get_obj,  mpp_cfg_get_object(&elem, NULL, MPP_CFG_TYPE_s32, &val));
        RUN_CHECK_IDX(add_elem, mpp_cfg_vla_add_elem(array, i, elem));
        elem = NULL;
    }

    RUN_AND_CHECK(add,      mpp_cfg_add(root, array));

    array = NULL;
    ret = rk_ok;

DONE:
    if (ret) {
        mpp_cfg_put_all(array);
        mpp_cfg_put_all(elem);
    }

    mpp_logi("test %s %s\n", str, ret ? "failed" : "success");

    return ret;
}

int main(int argc, char *argv[])
{
    MppCfgObj root = NULL;
    MppCfgObj array = NULL;
    MppCfgObj obj = NULL;
    MppCfgVal val;
    rk_s32 array_size = 4;
    rk_s32 ret = rk_nok;
    rk_s32 i;

    mpp_logi("start\n");

    if (argc > 1) {
        char *path = argv[1];
        MppCfgStrFmt file_fmt = MPP_CFG_STR_FMT_JSON;
        char *ext = strrchr(path, '.');

        if (ext) {
            if (!strcmp(ext, ".toml"))
                file_fmt = MPP_CFG_STR_FMT_TOML;
            else if (!strcmp(ext, ".json"))
                file_fmt = MPP_CFG_STR_FMT_JSON;
        }
        mpp_logi("file %s format %s\n", path, str_fmt[file_fmt]);

        ret = mpp_cfg_from_file(&root, file_fmt, path);
        if (ret) {
            mpp_loge("mpp_cfg_from_file failed\n");
            goto FILE_DONE;
        }

        mpp_logi("cfg object %p from file\n", root);
        mpp_cfg_dump_f(root);

        ret = test_to_from(root, file_fmt);
        mpp_logi("test to / from %s string %s\n", str_fmt[file_fmt], ret ? "failed" : "success");

    FILE_DONE:
        mpp_cfg_put_all(root);
        root = NULL;

        if (ret)
            return ret;
    }

    ret = mpp_cfg_get_object(&root, NULL, MPP_CFG_TYPE_OBJECT, NULL);
    if (ret) {
        mpp_loge("mpp_cfg_get_object failed\n");
        goto DONE;
    }

    mpp_logi("test basic s32 array\n");
    ret = mpp_cfg_get_array(&array, NULL);
    if (ret) {
        mpp_loge("mpp_cfg_get_array failed\n");
        goto DONE;
    }

    for (i = 0; i < array_size; i++) {
        obj = NULL;
        val.s32 = i;
        ret = mpp_cfg_get_object(&obj, NULL, MPP_CFG_TYPE_s32, &val);
        if (ret) {
            mpp_loge("mpp_cfg_get_object array element failed\n");
            goto DONE;
        }

        ret = mpp_cfg_add(array, obj);
        if (ret) {
            mpp_loge("mpp_cfg_add array element failed\n");
            goto DONE;
        }
    }

    ret = mpp_cfg_add(root, array);
    if (ret) {
        mpp_loge("mpp_cfg_add failed\n");
        goto DONE;
    }

    obj = NULL;
    val.s32 = 1920;
    ret = mpp_cfg_get_object(&obj, "width", MPP_CFG_TYPE_s32, &val);
    if (ret) {
        mpp_loge("mpp_cfg_get s32 failed\n");
        goto DONE;
    }
    ret = mpp_cfg_add(root, obj);
    if (ret) {
        mpp_loge("mpp_cfg_add s32 failed\n");
        goto DONE;
    }

    obj = NULL;
    val.u32 = 1080;
    ret = mpp_cfg_get_object(&obj, "height", MPP_CFG_TYPE_u32, &val);
    if (ret) {
        mpp_loge("mpp_cfg_get u32 failed\n");
        goto DONE;
    }
    ret = mpp_cfg_add(root, obj);
    if (ret) {
        mpp_loge("mpp_cfg_add u32 failed\n");
        goto DONE;
    }

    obj = NULL;
    val.str = "hello world";
    ret = mpp_cfg_get_object(&obj, "test", MPP_CFG_TYPE_STRING, &val);
    if (ret) {
        mpp_loge("mpp_cfg_get string failed\n");
        goto DONE;
    }
    ret = mpp_cfg_add(root, obj);
    if (ret) {
        mpp_loge("mpp_cfg_add string failed\n");
        goto DONE;
    }

    ret = test_typed_arrays(root);
    if (ret) {
        mpp_loge("test_typed_arrays failed\n");
        goto DONE;
    }
    array = NULL;

    ret = test_object_array(root);
    if (ret) {
        mpp_loge("test_object_array failed\n");
        goto DONE;
    }

    ret = test_nested_array(root);
    if (ret) {
        mpp_loge("test_nested_array failed\n");
        goto DONE;
    }

    ret = test_fix_raw_array(root);
    if (ret) {
        mpp_loge("test_fix_raw_array failed\n");
        goto DONE;
    }

    ret = test_flex_raw_array(root);
    if (ret) {
        mpp_loge("test_simple_fix_array failed\n");
        goto DONE;
    }

    ret = test_fix_elem_array(root);
    if (ret) {
        mpp_loge("test_vla_add_elem_fix failed\n");
        goto DONE;
    }

    ret = test_flex_elem_array(root);
    if (ret) {
        mpp_loge("test_flex_elem_array failed\n");
        goto DONE;
    }

    {
        /* step 1: entry-ful tree (objdef-shaped) to/from trie */
        MppCfgObj entryful = NULL;

        ret = test_trie_entryful(&entryful);
        if (ret) {
            mpp_loge("test_trie_entryful build failed\n");
            goto DONE;
        }
        ret = test_trie_roundtrip(entryful);
        mpp_cfg_put_all(entryful);
        if (ret) {
            mpp_loge("test_trie_roundtrip (entryful) failed\n");
            goto DONE;
        }
    }

    /* step 2: hand-built (entry-less) tree to/from trie */
    ret = test_trie_roundtrip(root);
    if (ret) {
        mpp_loge("test_trie_roundtrip failed\n");
        goto DONE;
    }

    /* step 3: real objdef cfg_roots to/from trie */
    ret = test_objdef_trie_roundtrip(mpp_enc_ref_cfg_objdef(), "ref_cfg");
    if (ret) {
        mpp_loge("test_objdef_trie_roundtrip ref_cfg failed\n");
        goto DONE;
    }

    ret = test_objdef_trie_roundtrip(mpp_enc_cfg_objdef(), "enc_cfg");
    if (ret) {
        mpp_loge("test_objdef_trie_roundtrip enc_cfg failed\n");
        goto DONE;
    }

    mpp_cfg_dump_f(root);

    {
        MppCfgObj simple_root = NULL;
        MppCfgObj simple_array = NULL;
        rk_s32 ser_ret;

        mpp_logi("test serialization\n");

        ret = mpp_cfg_get_object(&simple_root, NULL, MPP_CFG_TYPE_OBJECT, NULL);
        if (ret)
            goto DONE;

        val.s32 = 1920;
        ret = mpp_cfg_get_object(&obj, "width", MPP_CFG_TYPE_s32, &val);
        if (ret)
            goto DONE;
        ret = mpp_cfg_add(simple_root, obj);
        if (ret)
            goto DONE;
        obj = NULL;

        ret = mpp_cfg_get_array(&simple_array, "values");
        if (ret)
            goto DONE;
        for (i = 0; i < 4; i++) {
            val.s32 = i * 10;
            ret = add_array_element(simple_array, MPP_CFG_TYPE_s32, &val);
            if (ret)
                goto DONE;
        }
        ret = mpp_cfg_add(simple_root, simple_array);
        if (ret)
            goto DONE;
        simple_array = NULL;

        ser_ret = test_to_from(simple_root, MPP_CFG_STR_FMT_LOG);
        mpp_logi("to/from log %s\n", ser_ret ? "failed" : "success");

        ser_ret = test_to_from(simple_root, MPP_CFG_STR_FMT_JSON);
        mpp_logi("to/from json %s\n", ser_ret ? "failed" : "success");

        mpp_cfg_put_all(simple_root);
    }

    ret = rk_ok;

DONE:
    if (root) {
        mpp_cfg_put_all(root);
        root = NULL;
    }
    if (array) {
        mpp_cfg_put_all(array);
        array = NULL;
    }

    mpp_logi("done %s\n", ret ? "failed" : "success");

    return ret;
}

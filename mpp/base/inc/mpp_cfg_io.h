/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2025 Rockchip Electronics Co., Ltd.
 */

#ifndef MPP_CFG_IO_H
#define MPP_CFG_IO_H

#include "rk_mpp_cfg.h"

#include "mpp_internal.h"

typedef enum MppCfgType_e {
    MPP_CFG_TYPE_INVALID = 0,

    /* invalid or empty value type */
    MPP_CFG_TYPE_NULL,

    /* leaf type must with name */
    MPP_CFG_TYPE_BOOL,
    MPP_CFG_TYPE_s8,
    MPP_CFG_TYPE_u8,
    MPP_CFG_TYPE_s16,
    MPP_CFG_TYPE_u16,
    MPP_CFG_TYPE_s32,
    MPP_CFG_TYPE_u32,
    MPP_CFG_TYPE_s64,
    MPP_CFG_TYPE_u64,
    MPP_CFG_TYPE_f32,
    MPP_CFG_TYPE_f64,
    MPP_CFG_TYPE_STRING,
    MPP_CFG_TYPE_RAW,

    /* branch type */
    MPP_CFG_TYPE_OBJECT,
    MPP_CFG_TYPE_ARRAY,

    MPP_CFG_TYPE_BUTT,
} MppCfgType;

typedef union MppCfgVal_u {
    rk_bool     b1;
    rk_s8       s8;
    rk_u8       u8;
    rk_s16      s16;
    rk_u16      u16;
    rk_s32      s32;
    rk_u32      u32;
    rk_s64      s64;
    rk_u64      u64;
    rk_float    f32;
    rk_double   f64;
    void        *str;
    void        *ptr;
} MppCfgVal;

typedef void* MppCfgObj;

#ifdef __cplusplus
extern "C" {
#endif

/* object create / destroy */
rk_s32 mpp_cfg_get_object(MppCfgObj *obj, const char *name, MppCfgType type, MppCfgVal *val);
rk_s32 mpp_cfg_get_array(MppCfgObj *obj, const char *name);
rk_s32 mpp_cfg_put_all(MppCfgObj obj);

/* set KmppEntry.KmppEntryLocTbl for access location */
rk_s32 mpp_cfg_set_entry(MppCfgObj obj, KmppEntry *entry);
/* set KmppEntry.KmppEntryArrInf for Variable Length Array (VLA) info */
rk_s32 mpp_cfg_set_vla(MppCfgObj obj, KmppEntry *entry, MppCfgType type);

/* object tree management */
rk_s32 mpp_cfg_add(MppCfgObj root, MppCfgObj leaf);
/* VLA with simple raw value: s8 / u8 / s16 / u16 / s32 / u32 / f32 / f64 */
rk_s32 mpp_cfg_vla_add_raw(MppCfgObj array, rk_s32 idx, MppCfgVal *val);
/* VLA with complex element object: string / raw / object / array */
rk_s32 mpp_cfg_vla_add_elem(MppCfgObj array, rk_s32 idx, MppCfgObj elem);
/* VLA complex object detail description  */
rk_s32 mpp_cfg_add_detail(MppCfgObj root, MppCfgObj detail);

/* find by name string */
rk_s32 mpp_cfg_find(MppCfgObj *obj, MppCfgObj root, char *name, rk_s32 type);
/* get object value from the found cfg node */
rk_s32 mpp_cfg_get_val(MppCfgObj obj, MppCfgType type, MppCfgVal *val);

void mpp_cfg_dump(MppCfgObj obj, const char *func);
#define mpp_cfg_dump_f(obj) mpp_cfg_dump(obj, __FUNCTION__)

/* reconstruct a cfg tree from a trie (root named by the trie's name) */
MppCfgObj mpp_cfg_from_trie(MppTrie trie);
/* mark all MppCfgObject ready and build trie for string access */
MppTrie mpp_cfg_to_trie(MppCfgObj obj);

/* mpp_cfg output to string and input from string or file */
rk_s32 mpp_cfg_to_string(MppCfgObj obj, MppCfgStrFmt fmt, char **buf);
rk_s32 mpp_cfg_from_string(MppCfgObj *obj, MppCfgStrFmt fmt, const char *buf);
rk_s32 mpp_cfg_from_file(MppCfgObj *obj, MppCfgStrFmt fmt, const char *path);

/*
 * obj  - read from file or string and get an object as source
 * type - struct type object root for location table indexing and access
 * st   - struct body to write obj values to
 */
rk_s32 mpp_cfg_to_struct(MppCfgObj obj, MppCfgObj type, void *st);
/*
 * obj  - output object root for the struct values
 * type - struct type object root for location table access
 * st   - struct body to write obj values
 */
rk_s32 mpp_cfg_from_struct(MppCfgObj *obj, MppCfgObj type, void *st);

rk_s32 mpp_cfg_print_string(char *buf);

/* convert element type to config type */
MppCfgType mpp_cfg_type_from_elem_type(ElemType type);

#ifdef __cplusplus
}
#endif

#endif /* MPP_CFG_IO_H */

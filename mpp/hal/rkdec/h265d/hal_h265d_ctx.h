/* SPDX-License-Identifier: Apache-2.0 OR MIT */
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 */

#ifndef __HAL_H265D_CTX_H__
#define __HAL_H265D_CTX_H__

#include "mpp_device.h"
#include "mpp_hal.h"
#include "hal_bufs.h"

#define MAX_GEN_REG 3
/* before vdpu383 10 buf */
#define H265D_RCB_BUF_COUNT 11

typedef struct H265dRegBuf_t {
    RK_S32    use_flag;
    MppBuffer scaling_list_data;
    MppBuffer pps_data;
    MppBuffer rps_data;
    void*     hw_regs;
} H265dRegBuf;

typedef struct H265dRcbInfo_t {
    RK_S32              reg;
    RK_S32              size;
    RK_S32              offset;
} H265dRcbInfo;

typedef struct HalH265dCtx_t {
    /* for hal api call back */
    const MppHalApi *api;

    /* for hardware info */
    MppClientType   client_type;
    RK_U32          hw_id;
    MppDev          dev;
    MppDecCfgSet    *cfg;

    /* for resource */
    MppBufSlots     slots;
    MppBufSlots     packet_slots;
    MppBufferGroup  group;
    MppBuffer       cabac_table_data;
    MppBuffer       scaling_list_data;
    MppBuffer       pps_data;
    MppBuffer       rps_data;

    RK_S32          width;
    RK_S32          height;
    RK_S32          rcb_buf_size;
    H265dRcbInfo    rcb_info[H265D_RCB_BUF_COUNT];
    MppBuffer       rcb_buf[MAX_GEN_REG];

    void*           hw_regs;
    H265dRegBuf     g_buf[MAX_GEN_REG];
    RK_U32          fast_mode;
    MppCbCtx        *dec_cb;
    RK_U32          fast_mode_err_found;
    void            *scaling_rk;
    void            *scaling_qm;
    HalBufs         cmv_bufs;
    RK_U32          mv_size;
    RK_S32          mv_count;

    RK_U32          is_v341;
    RK_U32          is_v345;
    RK_U32          is_v34x;
    RK_U32          is_v383;
    /* rcb info */
    RK_U32          chroma_fmt_idc;
    RK_U8           ctu_size;
    RK_U8           num_row_tiles;
    RK_U8           bit_depth;
    RK_U8           error_index;
    /* for vdpu34x */
    MppBuffer       bufs;
    RK_S32          bufs_fd;
    RK_U32          offset_cabac;
    RK_U32          offset_spspps[MAX_GEN_REG];
    RK_U32          offset_rps[MAX_GEN_REG];
    RK_U32          offset_sclst[MAX_GEN_REG];
    RK_U32          spspps_offset;
    RK_U32          rps_offset;
    RK_U32          sclst_offset;
    void            *pps_buf;
    void            *sw_rps_buf;
    HalBufs         origin_bufs;

    const MppDecHwCap   *hw_info;
} HalH265dCtx;

typedef struct ScalingList {
    /* This is a little wasteful, since sizeID 0 only needs 8 coeffs,
     * and size ID 3 only has 2 arrays, not 6. */
    RK_U8 sl[4][6][64];
    RK_U8 sl_dc[2][6];
} scalingList_t;

typedef struct ScalingFactor_Model {
    RK_U8 scalingfactor0[1248];
    RK_U8 scalingfactor1[96];     /*4X4 TU Rotate, total 16X4*/
    RK_U8 scalingdc[12];          /*N1005 Vienna Meeting*/
    RK_U8 reserverd[4];           /*16Bytes align*/
} scalingFactor_t;

#endif

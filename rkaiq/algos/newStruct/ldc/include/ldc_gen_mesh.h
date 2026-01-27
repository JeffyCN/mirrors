/*
 *  ldc_gen_mesh.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
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
 *
 */

#ifndef _RK_AIQ_ALGO_ALDC_GEN_MESH_H_
#define _RK_AIQ_ALGO_ALDC_GEN_MESH_H_

#include "algos/newStruct/ldc/gen_mesh/include/rkAlgoGenFloatMeshFromRK.h"
#include "algos/newStruct/ldc/gen_mesh/include/rkAlgoGenLdchLdcvMesh.h"
#include "algos/newStruct/ldc/gen_mesh/include/rkAlgoMapSeparate.h"
#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"
#include "c_base/aiq_cond.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_mutex.h"
#include "c_base/aiq_thread.h"
#include "include/algos/rk_aiq_api_types_ldc.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"
#include "xcore/base/xcam_log.h"

typedef enum LdcEnState_e {
    kLdcEnLdch     = 0,
    kLdcEnLdcv     = 1,
    kLdcEnLdchLdcv = 2,
} LdcEnState;

typedef enum LdcGenMeshState_e {
    kLdcGenMeshIdle = 0,
    kLdcGenMeshWorking,
    kLdcGenMeshFinish,
} LdcGenMeshState;

typedef struct LdcMeshBufInfo_s {
    int32_t fd;
    void* addr;
} LdcMeshBufInfo_t;

typedef struct LdcGenMeshCfg_s {
    uint8_t level;

    LdcMeshBufInfo_t ldch;
    LdcMeshBufInfo_t ldcv;
} LdcGenMeshCfg_t;

typedef struct GenMeshCb_s {
    void* ctx_;
    void (*gen_mesh_done)(void* ctx, LdcGenMeshCfg_t cfg);
    void (*gen_mesh_failed)(void* ctx, LdcGenMeshCfg_t cfg);
} GenMeshCb;

typedef struct LdcAlgoGenMesh_s {
    bool enable_;
    LdcEnState state_;
    bool dump_mesh_;

    /* 原始全分辨率浮点表 */
    float* pf32MapxOri;
    float* pf32MapyOri;
    /* 拆分转换后的下采样浮点表 */
    float* pf32MeshX;
    float* pf32MeshY;
    /* LDCH/LDCV定点化表 */
    uint16_t* pu16MeshX;
    uint16_t* pu16MeshY;

    /* RK模型参数 */
    RKALGO_CAM_COEFF_INFO_S cam_coeff_;
    /* 原始全分辨率浮点表的相关参数 */
    RKALGO_GEN_FLOAT_MESH_INFO_S ori_float_mesh_Info_;
    /* 分步校正下采样浮点表相关参数 */
    RKALGO_SEPARATE_MESH_INFO_S st_separate_mesh_info_;
    /* LDCH和LDCV定点化表相关参数 */
    RKALGO_LDCH_LDCV_INFO_S st_ldch_ldcv_Info_;
} LdcAlgoGenMesh;

XCamReturn LdcAlgoGenMesh_init(LdcAlgoGenMesh* gen_mesh, LdcEnState state, int32_t srcW,
                               int32_t srcH, int32_t dstW, int32_t dstH,
                               const ldc_api_attrib_t* calib);
XCamReturn LdcAlgoGenMesh_deinit(LdcAlgoGenMesh* gen_mesh);
XCamReturn LdcAlgoGenMesh_genMeshNLevel(LdcAlgoGenMesh* gen_mesh, uint8_t isp_id, uint32_t level,
                                        uint16_t* pMeshX, uint16_t* pMeshY);
static inline bool LdcAlgoGenMesh_IsEnabled(LdcAlgoGenMesh* gen_mesh) { return gen_mesh->enable_; }

/*
 *
 *      L D C   G E N   M E S H   H E L P E R   T H R E A D
 *
 */

typedef struct LdcGenMeshHelperThd_s {
    AiqThread_t* _base;

    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;

    LdcAlgoGenMesh* _gen_mesh;
    GenMeshCb* _callback;
} LdcGenMeshHelperThd_t;

XCamReturn LdcGenMeshHelperThd_init(LdcGenMeshHelperThd_t* pHelpThd, LdcAlgoGenMesh* gen_mesh);

void LdcGenMeshHelperThd_deinit(LdcGenMeshHelperThd_t* pHelpThd);

XCamReturn LdcGenMeshHelperThd_start(LdcGenMeshHelperThd_t* pHelpThd);

XCamReturn LdcGenMeshHelperThd_stop(LdcGenMeshHelperThd_t* pHelpThd);

bool LdcGenMeshHelperThd_pushAttr(LdcGenMeshHelperThd_t* pHelpThd, LdcGenMeshCfg_t* params);

static inline void LdcGenMeshHelperThd_setCb(LdcGenMeshHelperThd_t* helper, GenMeshCb* cb) {
    helper->_callback = cb;
}

#endif  //_RK_AIQ_ALGO_ALDC_GEN_MESH_H_

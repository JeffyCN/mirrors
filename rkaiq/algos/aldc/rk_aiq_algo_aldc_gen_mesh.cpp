/*
 * rk_aiq_algo_aldc_gen_mesh.cpp
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#include "algos/aldc/rk_aiq_algo_aldc_gen_mesh.h"

#include <stdio.h>

#include "algos/aldc/gen_mesh/include/rkAlgoGenFloatMesh.h"
#include "algos/aldc/gen_mesh/include/rkAlgoGenFloatMeshFromRK.h"

namespace RkCam {

XCamReturn LdcAlgoGenMesh::Init(LdcEnState state, int32_t srcW, int32_t srcH, int32_t dstW,
                                int32_t dstH, const CalibDbV2_LDC_t* calib) {
    if (enable_) {
        LOGV_ALDC("GenMesh has been inited, bypass");
        return XCAM_RETURN_BYPASS;
    }

    if (!calib) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    cam_coeff_.cx = calib->param.light_center[0];
    cam_coeff_.cy = calib->param.light_center[1];
    cam_coeff_.a0 = calib->param.coefficient[0];
    cam_coeff_.a2 = calib->param.coefficient[1];
    cam_coeff_.a3 = calib->param.coefficient[2];
    cam_coeff_.a4 = calib->param.coefficient[3];

    LOGD_ALDC("calib: light center: %.16f, %.16f", cam_coeff_.cx, cam_coeff_.cy);
    LOGD_ALDC("calib: coeff: %.16f, %.16f, %.16f, %.16f", cam_coeff_.a0, cam_coeff_.a2,
              cam_coeff_.a3, cam_coeff_.a4);

    /* 配置最终需要输出的定点化表的类型 */
    if (state == LdcEnState::kLdcEnLdchLdcv)
        ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCH_LDCV;
    else if (state == LdcEnState::kLdcEnLdch)
        ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCH;
    else if (state == LdcEnState::kLdcEnLdcv)
        ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCV;
    /* 配置硬件FEC版本，若不需要的必须配置为NO_NEED */
    ori_float_mesh_Info_.enFecVersion = RKALGO_FEC_VERSION_NO_NEED;
    /* 配置硬件LDCH版本，若不需要的必须配置为NO_NEED */
    ori_float_mesh_Info_.enLdchVersion = RKALGO_LDCH_VERSION_1;
    /* 配置硬件LDCH + LDCV版本，若不需要的必须配置为NO_NEED */
    ori_float_mesh_Info_.enLdchLdcvVersion = RKALGO_LDCH_LDCV_VERSION_3576;

    /* 根据最终生成的定点化表的类型，配置和计算浮点表的相关参数 */
    ori_float_mesh_Info_.srcW     = srcW;
    ori_float_mesh_Info_.srcH     = srcH;
    ori_float_mesh_Info_.dstW     = dstW;
    ori_float_mesh_Info_.dstH     = dstH;
    ori_float_mesh_Info_.correctX = 1; /* 水平x方向校正: 1代表校正, 0代表不校正 */
    ori_float_mesh_Info_.correctY = 1; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    /* 设置原始浮点表的采样步长，此处必须按照LDCH+LDCV要求配置采样步长 */
    if (state == LdcEnState::kLdcEnLdchLdcv) {
        ori_float_mesh_Info_.mapStepW = 1;
        ori_float_mesh_Info_.mapStepH = 1;
    } else if (state == LdcEnState::kLdcEnLdch) {
        ori_float_mesh_Info_.mapStepW = 16;
        ori_float_mesh_Info_.mapStepH = 8;
    } else if (state == LdcEnState::kLdcEnLdcv) {
        ori_float_mesh_Info_.mapStepW = 16;
        ori_float_mesh_Info_.mapStepH = 16;
    }
    ori_float_mesh_Info_.saveMaxFovX = 0; /* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
    int32_t ret                      = calcFloatMeshSizeRK(&ori_float_mesh_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcFloatMeshSize");
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = genFloatMeshNLevelInit(&cam_coeff_, &ori_float_mesh_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to genFloatMeshNLevelInit\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    /* 原始全分辨率浮点表的内存申请 */
    if (state == LdcEnState::kLdcEnLdchLdcv || state == LdcEnState::kLdcEnLdch)
        pf32MapxOri = std::unique_ptr<float>(new float[ori_float_mesh_Info_.u64MapBufSize]);
    if (state == LdcEnState::kLdcEnLdchLdcv || state == LdcEnState::kLdcEnLdcv)
        pf32MapyOri = std::unique_ptr<float>(new float[ori_float_mesh_Info_.u64MapBufSize]);

    /* 计算分步校正下采样浮点表相关参数 */
    ret = calcSeparateMeshInfo(srcW, srcH, dstW, dstH, &st_separate_mesh_info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcSeparateMeshInfo\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    /* 下采样浮点表的内存申请 */
    if (state == LdcEnState::kLdcEnLdchLdcv || state == LdcEnState::kLdcEnLdch)
        pf32MeshX =
            std::unique_ptr<float>(new float[st_separate_mesh_info_.u64LdchFloatMeshBufSize]);
    if (state == LdcEnState::kLdcEnLdchLdcv || state == LdcEnState::kLdcEnLdcv)
        pf32MeshY =
            std::unique_ptr<float>(new float[st_separate_mesh_info_.u64LdcvFloatMeshBufSize]);

    /* 根据LDCH + LDCV的版本，计算LDCH和LDCV定点化表所需的buffer大小 */
    st_ldch_ldcv_Info_.enLdchLdcvVersion = RKALGO_LDCH_LDCV_VERSION_3576;
    st_ldch_ldcv_Info_.srcW              = srcW;
    st_ldch_ldcv_Info_.srcH              = srcH;
    st_ldch_ldcv_Info_.dstW              = dstW;
    st_ldch_ldcv_Info_.dstH              = dstH;
    ret                                  = calcLdchLdcvMeshSize(&st_ldch_ldcv_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcLdchLdcvMeshSize\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    /* LDCH和LDCV定点化表的内存申请 */
    /*
     * pu16MeshX = std::unique_ptr<uint16_t>(new uint16_t[st_ldch_ldcv_Info_.u64LdchMeshBufSize]);
     * pu16MeshY = std::unique_ptr<uint16_t>(new uint16_t[st_ldch_ldcv_Info_.u64LdcvMeshBufSize]);
     */

    state_  = state;
    enable_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoGenMesh::Deinit() { return XCAM_RETURN_NO_ERROR; }

XCamReturn LdcAlgoGenMesh::GenMeshNLevel(uint8_t isp_id, uint32_t level, uint16_t* pMeshX,
                                         uint16_t* pMeshY) {
    ENTER_ALDC_FUNCTION();
    LOGD_ALDC("%s, isp id: %d, level: %d, state_: %d", __FUNCTION__, isp_id, level, state_);

    if (!pMeshX && !pMeshY) {
        LOGE_ALDC("pMeshX and pMeshY is null!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    /* 生成原始全分辨率浮点表 */
    XCAM_STATIC_PROFILING_START(genFloatMeshNLevel);
    int32_t ret = genFloatMeshNLevel(&cam_coeff_, &ori_float_mesh_Info_, level, pf32MapxOri.get(),
                                     pf32MapyOri.get());
    if (-1 == ret) {
        LOGE_ALDC("Failed to genFloatMeshNLevel");
        return XCAM_RETURN_ERROR_FAILED;
    }
    XCAM_STATIC_PROFILING_END(genFloatMeshNLevel, 0);

    /* 原始全分辨率浮点表 ---> 分步校正下采样浮点表 */
    if (state_ == LdcEnState::kLdcEnLdchLdcv) {
        XCAM_STATIC_PROFILING_START(genSeparateMeshFromOriMap);
        ret = genSeparateMeshFromOriMap(&st_separate_mesh_info_, pf32MapxOri.get(),
                                        pf32MapyOri.get(), pf32MeshX.get(), pf32MeshY.get());
        if (-1 == ret) {
            LOGE_ALDC("Failed to genSeparateMeshFromOriMap\n");
            return XCAM_RETURN_ERROR_FAILED;
        }
        XCAM_STATIC_PROFILING_END(genSeparateMeshFromOriMap, 0);
    }

    uint32_t ldchFloatMeshW, ldchFloatMeshH;         /* LDCH下采样浮点表的宽高 */
    uint32_t ldcvFloatMeshW = 0, ldcvFloatMeshH = 0; /* LDCV下采样浮点表的宽高 */

    /* 分步校正下采样浮点表 ---> LDCH和LDCV定点化表 */
    ldchFloatMeshW = st_separate_mesh_info_.ldchFloatMeshW;
    ldchFloatMeshH = st_separate_mesh_info_.ldchFloatMeshH;
    const float *pf32MapxIn, *pf32MapyIn;
    if (state_ == LdcEnState::kLdcEnLdchLdcv) {
        ldcvFloatMeshW = st_separate_mesh_info_.ldcvFloatMeshW;
        ldcvFloatMeshH = st_separate_mesh_info_.ldcvFloatMeshH;
        pf32MapxIn     = pf32MeshX.get();
        pf32MapyIn     = pf32MeshY.get();
    } else {
        pf32MapxIn = pf32MapxOri.get();
        pf32MapyIn = pf32MapyOri.get();
    }
    XCAM_STATIC_PROFILING_START(genLdchLdcvMeshFromFloatMesh);
    ret = genLdchLdcvMeshFromFloatMesh(&st_ldch_ldcv_Info_, ldchFloatMeshW, ldchFloatMeshH,
                                       ldcvFloatMeshW, ldcvFloatMeshH, pf32MapxIn, pf32MapyIn,
                                       pMeshX, pMeshY);
    if (-1 == ret) {
        LOGE_ALDC("Failed to genLdchLdcvMeshFromSeparateMesh\n");
        return XCAM_RETURN_ERROR_FAILED;
    }
    XCAM_STATIC_PROFILING_END(genLdchLdcvMeshFromFloatMesh, 0);

    /* 保存LDCH定点化表 */
    if (0) {
        char ldchMeshPath[256];
        sprintf(ldchMeshPath, "/tmp/LDCH_meshX_src%dx%d_dst%dx%d_level%03d.bin",
                ori_float_mesh_Info_.srcW, ori_float_mesh_Info_.srcH, ori_float_mesh_Info_.dstW,
                ori_float_mesh_Info_.dstH, level);
        FILE* fpMeshX = fopen(ldchMeshPath, "wb");
        if (fpMeshX == NULL) {
            printf("%s error: failed to save LDCH mesh\n", __func__);
            return XCAM_RETURN_NO_ERROR;
        }
        fwrite(&ori_float_mesh_Info_.dstW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&ori_float_mesh_Info_.dstH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&st_ldch_ldcv_Info_.ldchAlignMeshW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&st_ldch_ldcv_Info_.ldchMeshH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&st_ldch_ldcv_Info_.ldchMeshStepW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&st_ldch_ldcv_Info_.ldchMeshStepH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(pMeshX, sizeof(uint16_t), st_ldch_ldcv_Info_.u64LdchMeshBufSize, fpMeshX);
        fclose(fpMeshX);
    }

    /* 保存LDCV定点化表 */
    if (0) {
        char ldcvMeshPath[256];
        sprintf(ldcvMeshPath, "/tmp/LDCV_meshY_src%dx%d_dst%dx%d_level%03d.bin",
                ori_float_mesh_Info_.srcW, ori_float_mesh_Info_.srcH, ori_float_mesh_Info_.dstW,
                ori_float_mesh_Info_.dstH, level);
        FILE* fpMeshY = fopen(ldcvMeshPath, "wb");
        if (fpMeshY == NULL) {
            printf("%s error: failed to save LDCV mesh\n", __func__);
            return XCAM_RETURN_NO_ERROR;
        }
        fwrite(pMeshY, sizeof(uint16_t), st_ldch_ldcv_Info_.u64LdcvMeshBufSize, fpMeshY);
        fclose(fpMeshY);
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

bool LdcGenMeshThread::loop() {
    ENTER_ALDC_FUNCTION();
    int32_t ret = 0;

    const static int32_t timeout  = -1;
    SmartPtr<LdcGenMeshCfg_t> cfg = attr_queue_.pop(timeout);

    if (!cfg.ptr()) {
        LOGW_ALDC("LdcGenMeshThread got empty config, stop thread");
        return false;
    }

    if (!gen_mesh_.ptr() || !gen_mesh_->IsEnabled()) {
        LOGE_ALDC("gen_mesh_ hasn't been inited, bypass");
        return true;
    }

    ret = gen_mesh_->GenMeshNLevel(0, cfg->level, reinterpret_cast<uint16_t*>(cfg->ldch.addr),
                                   reinterpret_cast<uint16_t*>(cfg->ldcv.addr));
    if (ret < 0) {
        if (callback_) callback_->GenMeshFailedCallback(0);
        LOGE_ALDC("Failed to GenMeshNLevel: %d", cfg->level);
    } else {
        if (callback_) callback_->GenMeshDoneCallback(*cfg.ptr());
    }

    EXIT_ALDC_FUNCTION();

    return true;
}

}  // namespace RkCam

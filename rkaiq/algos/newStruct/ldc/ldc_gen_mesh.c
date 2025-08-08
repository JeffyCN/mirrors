/*
 * rk_aiq_algo_aldc_gen_mesh.cpp
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

#include "algos/newStruct/ldc/include/ldc_gen_mesh.h"

#include <stdio.h>

#include "algos/newStruct/ldc/gen_mesh/include/rkAlgoGenFloatMeshComm.h"
#include "algos/newStruct/ldc/gen_mesh/include/rkAlgoGenFloatMeshFromRK.h"
#include "c_base/aiq_base.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_thread.h"
#include "xcore/xcam_obj_debug.h"

#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

#define LDC_HELP_THREAD_MSG_MAX (3)

/*
 *
 *      L D C   G E N   M E S H
 *
 */

static bool LdcAlgoGenMesh_isDumpMesh() {
    bool dump = false;
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.ldc.dump", property_value, "1");
    if (strtoull(property_value, NULL, 16) > 0) dump = true;
#else
    char* valueStr = getenv("rkisp_dump_ldc_mesh");
    if (valueStr) {
        if (strtoull(valueStr, NULL, 16) > 0) dump = true;
    }
#endif

    LOGD_ALDC("%s mesh to file", dump ? "dump" : "not dump");

    return dump;
}

XCamReturn LdcAlgoGenMesh_init(LdcAlgoGenMesh* gen_mesh, LdcEnState state, int32_t srcW,
                               int32_t srcH, int32_t dstW, int32_t dstH,
                               const ldc_api_attrib_t* calib) {
    if (gen_mesh->enable_) {
        LOGV_ALDC("GenMesh has been inited, bypass");
        return XCAM_RETURN_BYPASS;
    }

    if (!calib) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    gen_mesh->cam_coeff_.cx = calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_opticCenter_x;
    gen_mesh->cam_coeff_.cy = calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_opticCenter_y;
    gen_mesh->cam_coeff_.a0 =
        calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_lensDistor_coeff[0];
    gen_mesh->cam_coeff_.a2 =
        calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_lensDistor_coeff[1];
    gen_mesh->cam_coeff_.a3 =
        calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_lensDistor_coeff[2];
    gen_mesh->cam_coeff_.a4 =
        calib->tunning.autoGenMesh.lensDistorCoeff.sw_ldcC_lensDistor_coeff[3];

    LOGD_ALDC("calib: light center: %.16f, %.16f", gen_mesh->cam_coeff_.cx,
              gen_mesh->cam_coeff_.cy);
    LOGD_ALDC("calib: coeff: %.16f, %.16f, %.16f, %.16f", gen_mesh->cam_coeff_.a0,
              gen_mesh->cam_coeff_.a2, gen_mesh->cam_coeff_.a3, gen_mesh->cam_coeff_.a4);
    LOGD_ALDC("calib: level %d, maxLevel %d, saveMaxFovX %d",
              calib->tunning.autoGenMesh.sw_ldcT_correctStrg_val,
              calib->tunning.autoGenMesh.sw_ldcC_correctStrg_maxLimit,
              calib->tunning.autoGenMesh.sw_ldcT_saveMaxFovX_bit);

    /* 配置最终需要输出的定点化表的类型 */
    if (state == kLdcEnLdchLdcv)
        gen_mesh->ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCH_LDCV;
    else if (state == kLdcEnLdch)
        gen_mesh->ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCH;
    else if (state == kLdcEnLdcv)
        gen_mesh->ori_float_mesh_Info_.enMeshType = RKALGO_MESH_TYPE_LDCV;
    /* 配置硬件FEC版本，若不需要的必须配置为NO_NEED */
    gen_mesh->ori_float_mesh_Info_.enFecVersion = RKALGO_FEC_VERSION_NO_NEED;
    /* 配置硬件LDCH版本，若不需要的必须配置为NO_NEED */
    gen_mesh->ori_float_mesh_Info_.enLdchVersion = RKALGO_LDCH_VERSION_1;
    /* 配置硬件LDCH + LDCV版本，若不需要的必须配置为NO_NEED */
    if (state == kLdcEnLdcv)
        gen_mesh->ori_float_mesh_Info_.enLdchLdcvVersion = RKALGO_LDCH_LDCV_VERSION_3576;
    else
        gen_mesh->ori_float_mesh_Info_.enLdchLdcvVersion = RKALGO_LDCH_LDCV_VERSION_NO_NEED;

    /* 根据最终生成的定点化表的类型，配置和计算浮点表的相关参数 */
    gen_mesh->ori_float_mesh_Info_.srcW = srcW;
    gen_mesh->ori_float_mesh_Info_.srcH = srcH;
    gen_mesh->ori_float_mesh_Info_.dstW = dstW;
    gen_mesh->ori_float_mesh_Info_.dstH = dstH;
    /* 设置原始浮点表的采样步长，此处必须按照LDCH+LDCV要求配置采样步长 */
    if (state == kLdcEnLdchLdcv) {
        gen_mesh->ori_float_mesh_Info_.mapStepW = 1;
        gen_mesh->ori_float_mesh_Info_.mapStepH = 1;
        gen_mesh->ori_float_mesh_Info_.correctX = 1; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        gen_mesh->ori_float_mesh_Info_.correctY = 1; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    } else if (state == kLdcEnLdch) {
        gen_mesh->ori_float_mesh_Info_.mapStepW = 16;
        gen_mesh->ori_float_mesh_Info_.mapStepH = 8;
        gen_mesh->ori_float_mesh_Info_.correctX = 1; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        gen_mesh->ori_float_mesh_Info_.correctY = 0; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    } else if (state == kLdcEnLdcv) {
        gen_mesh->ori_float_mesh_Info_.mapStepW = 16;
        gen_mesh->ori_float_mesh_Info_.mapStepH = 16;
        gen_mesh->ori_float_mesh_Info_.correctX = 0; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        gen_mesh->ori_float_mesh_Info_.correctY = 1; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    }

    /* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
    gen_mesh->ori_float_mesh_Info_.saveMaxFovX = calib->tunning.autoGenMesh.sw_ldcT_saveMaxFovX_bit;

    int32_t ret = calcFloatMeshSizeRK(&gen_mesh->ori_float_mesh_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcFloatMeshSize");
        return XCAM_RETURN_ERROR_FAILED;
    }

    XCAM_STATIC_PROFILING_START(genFloatMeshNLevelInit);
    ret = genFloatMeshNLevelInit(&gen_mesh->cam_coeff_, &gen_mesh->ori_float_mesh_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to genFloatMeshNLevelInit\n");
        return XCAM_RETURN_ERROR_FAILED;
    }
    XCAM_STATIC_PROFILING_END(genFloatMeshNLevelInit, 0);

    /* 原始全分辨率浮点表的内存申请 */
    if (state == kLdcEnLdchLdcv || state == kLdcEnLdch)
        gen_mesh->pf32MapxOri =
            (float*)aiq_mallocz(sizeof(float) * gen_mesh->ori_float_mesh_Info_.u64MapBufSize);
#if RKAIQ_HAVE_LDCV
    if (state == kLdcEnLdchLdcv || state == kLdcEnLdcv)
        gen_mesh->pf32MapyOri =
            (float*)aiq_mallocz(sizeof(float) * gen_mesh->ori_float_mesh_Info_.u64MapBufSize);
#endif

#if RKAIQ_HAVE_LDCV
    /* 计算分步校正下采样浮点表相关参数 */
    ret = calcSeparateMeshInfo(srcW, srcH, dstW, dstH, &gen_mesh->st_separate_mesh_info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcSeparateMeshInfo\n");
        return XCAM_RETURN_ERROR_FAILED;
    }
#endif

    /* 下采样浮点表的内存申请 */
    if (state == kLdcEnLdchLdcv || state == kLdcEnLdch)
        gen_mesh->pf32MeshX = (float*)aiq_mallocz(
            sizeof(float) * gen_mesh->st_separate_mesh_info_.u64LdchFloatMeshBufSize);
#if RKAIQ_HAVE_LDCV
    if (state == kLdcEnLdchLdcv || state == kLdcEnLdcv)
        gen_mesh->pf32MeshY = (float*)aiq_mallocz(
            sizeof(float) * gen_mesh->st_separate_mesh_info_.u64LdcvFloatMeshBufSize);
#endif

    /* 根据LDCH + LDCV的版本，计算LDCH和LDCV定点化表所需的buffer大小 */
    gen_mesh->st_ldch_ldcv_Info_.enLdchLdcvVersion = RKALGO_LDCH_LDCV_VERSION_3576;
    gen_mesh->st_ldch_ldcv_Info_.srcW              = srcW;
    gen_mesh->st_ldch_ldcv_Info_.srcH              = srcH;
    gen_mesh->st_ldch_ldcv_Info_.dstW              = dstW;
    gen_mesh->st_ldch_ldcv_Info_.dstH              = dstH;
    ret = calcLdchLdcvMeshSize(&gen_mesh->st_ldch_ldcv_Info_);
    if (-1 == ret) {
        LOGE_ALDC("Failed to calcLdchLdcvMeshSize\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    /* LDCH和LDCV定点化表的内存申请 */
    gen_mesh->pu16MeshX =
        (uint16_t*)aiq_mallocz(sizeof(uint16_t) * gen_mesh->st_ldch_ldcv_Info_.u64LdchMeshBufSize);
#if RKAIQ_HAVE_LDCV
    gen_mesh->pu16MeshY =
        (uint16_t*)aiq_mallocz(sizeof(uint16_t) * gen_mesh->st_ldch_ldcv_Info_.u64LdcvMeshBufSize);
#endif

    gen_mesh->dump_mesh_ = LdcAlgoGenMesh_isDumpMesh();
    gen_mesh->state_     = state;
    gen_mesh->enable_    = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoGenMesh_deinit(LdcAlgoGenMesh* gen_mesh) {
    if (gen_mesh->pf32MapxOri) {
        aiq_free(gen_mesh->pf32MapxOri);
        gen_mesh->pf32MapxOri = NULL;
    }
    if (gen_mesh->pf32MapyOri) {
        aiq_free(gen_mesh->pf32MapyOri);
        gen_mesh->pf32MapyOri = NULL;
    }
    if (gen_mesh->pf32MeshX) {
        aiq_free(gen_mesh->pf32MeshX);
        gen_mesh->pf32MeshX = NULL;
    }
    if (gen_mesh->pf32MeshY) {
        aiq_free(gen_mesh->pf32MeshY);
        gen_mesh->pf32MeshY = NULL;
    }
    if (gen_mesh->pu16MeshX) {
        aiq_free(gen_mesh->pu16MeshX);
        gen_mesh->pu16MeshX = NULL;
    }
    if (gen_mesh->pu16MeshY) {
        aiq_free(gen_mesh->pu16MeshY);
        gen_mesh->pu16MeshY = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoGenMesh_genMeshNLevel(LdcAlgoGenMesh* gen_mesh, uint8_t isp_id, uint32_t level,
                                        uint16_t* pMeshX, uint16_t* pMeshY) {
    ENTER_ALDC_FUNCTION();
    LOGD_ALDC("%s, isp id: %d, level: %d, state_: %d", __FUNCTION__, isp_id, level,
              gen_mesh->state_);

    if (!pMeshX && !pMeshY) {
        LOGE_ALDC("pMeshX and pMeshY is null!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    /* 生成原始全分辨率浮点表 */
    XCAM_STATIC_PROFILING_START(genFloatMeshNLevel);
    int32_t ret = genFloatMeshNLevel(&gen_mesh->cam_coeff_, &gen_mesh->ori_float_mesh_Info_, level,
                                     gen_mesh->pf32MapxOri, gen_mesh->pf32MapyOri);
    if (-1 == ret) {
        LOGE_ALDC("Failed to genFloatMeshNLevel");
        return XCAM_RETURN_ERROR_FAILED;
    }
    XCAM_STATIC_PROFILING_END(genFloatMeshNLevel, 0);

#if RKAIQ_HAVE_LDCV
    /* 原始全分辨率浮点表 ---> 分步校正下采样浮点表 */
    if (gen_mesh->state_ == kLdcEnLdchLdcv) {
        XCAM_STATIC_PROFILING_START(genSeparateMeshFromOriMap);
        ret = genSeparateMeshFromOriMap(&gen_mesh->st_separate_mesh_info_, gen_mesh->pf32MapxOri,
                                        gen_mesh->pf32MapyOri, gen_mesh->pf32MeshX,
                                        gen_mesh->pf32MeshY);
        if (-1 == ret) {
            LOGE_ALDC("Failed to genSeparateMeshFromOriMap\n");
            return XCAM_RETURN_ERROR_FAILED;
        }
        XCAM_STATIC_PROFILING_END(genSeparateMeshFromOriMap, 0);
    }
#endif

    uint32_t ldchFloatMeshW, ldchFloatMeshH;         /* LDCH下采样浮点表的宽高 */
    uint32_t ldcvFloatMeshW = 0, ldcvFloatMeshH = 0; /* LDCV下采样浮点表的宽高 */

    /* 分步校正下采样浮点表 ---> LDCH和LDCV定点化表 */
    const float *pf32MapxIn, *pf32MapyIn;
    if (gen_mesh->state_ == kLdcEnLdchLdcv) {
        ldchFloatMeshW = gen_mesh->st_separate_mesh_info_.ldchFloatMeshW;
        ldchFloatMeshH = gen_mesh->st_separate_mesh_info_.ldchFloatMeshH;
        ldcvFloatMeshW = gen_mesh->st_separate_mesh_info_.ldcvFloatMeshW;
        ldcvFloatMeshH = gen_mesh->st_separate_mesh_info_.ldcvFloatMeshH;
        pf32MapxIn     = gen_mesh->pf32MeshX;
        pf32MapyIn     = gen_mesh->pf32MeshY;
    } else {
        ldchFloatMeshW = gen_mesh->ori_float_mesh_Info_.mapW;
        ldchFloatMeshH = gen_mesh->ori_float_mesh_Info_.mapH;
        ldcvFloatMeshW = 0;
        ldcvFloatMeshH = 0;
        pf32MapxIn     = gen_mesh->pf32MapxOri;
        pf32MapyIn     = gen_mesh->pf32MapyOri;
    }
    XCAM_STATIC_PROFILING_START(genLdchLdcvMeshFromFloatMesh);
    ret = genLdchLdcvMeshFromFloatMesh(&gen_mesh->st_ldch_ldcv_Info_, ldchFloatMeshW,
                                       ldchFloatMeshH, ldcvFloatMeshW, ldcvFloatMeshH, pf32MapxIn,
                                       pf32MapyIn, pMeshX, pMeshY);
    if (-1 == ret) {
        LOGE_ALDC("Failed to genLdchLdcvMeshFromSeparateMesh\n");
        return XCAM_RETURN_ERROR_FAILED;
    }
    XCAM_STATIC_PROFILING_END(genLdchLdcvMeshFromFloatMesh, 0);

    /* 保存LDCH定点化表 */
    if (gen_mesh->dump_mesh_) {
        char ldchMeshPath[256];
#ifdef ANDROID_OS
        sprintf(ldchMeshPath, "/data/LDCH_meshX_src%dx%d_dst%dx%d_level%03d.bin",
                gen_mesh->ori_float_mesh_Info_.srcW, gen_mesh->ori_float_mesh_Info_.srcH,
                gen_mesh->ori_float_mesh_Info_.dstW, gen_mesh->ori_float_mesh_Info_.dstH, level);
#else
        sprintf(ldchMeshPath, "/tmp/LDCH_meshX_src%dx%d_dst%dx%d_level%03d.bin",
                gen_mesh->ori_float_mesh_Info_.srcW, gen_mesh->ori_float_mesh_Info_.srcH,
                gen_mesh->ori_float_mesh_Info_.dstW, gen_mesh->ori_float_mesh_Info_.dstH, level);
#endif
        FILE* fpMeshX = fopen(ldchMeshPath, "wb");
        if (fpMeshX == NULL) {
            printf("%s error: failed to save LDCH mesh\n", __func__);
            return XCAM_RETURN_NO_ERROR;
        }
        fwrite(&gen_mesh->ori_float_mesh_Info_.dstW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&gen_mesh->ori_float_mesh_Info_.dstH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&gen_mesh->st_ldch_ldcv_Info_.ldchAlignMeshW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&gen_mesh->st_ldch_ldcv_Info_.ldchMeshH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&gen_mesh->st_ldch_ldcv_Info_.ldchMeshStepW, sizeof(uint16_t), 1, fpMeshX);
        fwrite(&gen_mesh->st_ldch_ldcv_Info_.ldchMeshStepH, sizeof(uint16_t), 1, fpMeshX);
        fwrite(pMeshX, sizeof(uint16_t), gen_mesh->st_ldch_ldcv_Info_.u64LdchMeshBufSize, fpMeshX);
        fclose(fpMeshX);
    }

    /* 保存LDCV定点化表 */
    if (gen_mesh->dump_mesh_ && gen_mesh->state_ == kLdcEnLdchLdcv) {
        char ldcvMeshPath[256];
#ifdef ANDROID_OS
        sprintf(ldcvMeshPath, "/data/LDCV_meshY_src%dx%d_dst%dx%d_level%03d.bin",
                gen_mesh->ori_float_mesh_Info_.srcW, gen_mesh->ori_float_mesh_Info_.srcH,
                gen_mesh->ori_float_mesh_Info_.dstW, gen_mesh->ori_float_mesh_Info_.dstH, level);
#else
        sprintf(ldcvMeshPath, "/tmp/LDCV_meshY_src%dx%d_dst%dx%d_level%03d.bin",
                gen_mesh->ori_float_mesh_Info_.srcW, gen_mesh->ori_float_mesh_Info_.srcH,
                gen_mesh->ori_float_mesh_Info_.dstW, gen_mesh->ori_float_mesh_Info_.dstH, level);
#endif
        FILE* fpMeshY = fopen(ldcvMeshPath, "wb");
        if (fpMeshY == NULL) {
            printf("%s error: failed to save LDCV mesh\n", __func__);
            return XCAM_RETURN_NO_ERROR;
        }
        fwrite(pMeshY, sizeof(uint16_t), gen_mesh->st_ldch_ldcv_Info_.u64LdcvMeshBufSize, fpMeshY);
        fclose(fpMeshY);
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

/*
 *
 *      L D C   G E N   M E S H   H E L P E R
 *
 */

static bool LdcGenMeshHelperThd_loop(void* user_data) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    LdcGenMeshHelperThd_t* pHdlTh = (LdcGenMeshHelperThd_t*)user_data;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGE_CAMHW_SUBM(LENS_SUBM, "quit LdcGenMeshHelperTh !");
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    AiqListItem_t* pItem = aiqList_get_item(pHdlTh->mMsgsQueue, NULL);
    if (!pItem) {
        // ignore error
        return true;
    }

    LdcGenMeshCfg_t* cfg = (LdcGenMeshCfg_t*)pItem->_pData;
    ret                  = LdcAlgoGenMesh_genMeshNLevel(pHdlTh->_gen_mesh, 0, cfg->level,
                                       (uint16_t*)(cfg->ldch.addr), (uint16_t*)(cfg->ldcv.addr));
    if (ret < 0) {
        if (pHdlTh->_callback) pHdlTh->_callback->gen_mesh_failed(pHdlTh->_callback->ctx_, *cfg);
        LOGE_ALDC("Failed to GenMeshNLevel: %d", cfg->level);
    } else {
        if (pHdlTh->_callback) pHdlTh->_callback->gen_mesh_done(pHdlTh->_callback->ctx_, *cfg);
    }

    aiqList_erase_item(pHdlTh->mMsgsQueue, pItem);

    if (ret == XCAM_RETURN_NO_ERROR) return true;

    LOGE_CAMHW_SUBM(LENS_SUBM, "LdcGenMeshHelperThd failed to run command!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

bool LdcGenMeshHelperThd_pushAttr(LdcGenMeshHelperThd_t* pHelpThd, LdcGenMeshCfg_t* params) {
    if (!pHelpThd->_base->_started) return false;

    int ret = aiqList_push(pHelpThd->mMsgsQueue, params);
    if (ret) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "push LdcGenMeshCfg params failed", __func__);
        return false;
    }

    aiqCond_broadcast(&pHelpThd->_cond);

    return true;
}

XCamReturn LdcGenMeshHelperThd_init(LdcGenMeshHelperThd_t* pHelpThd, LdcAlgoGenMesh* gen_mesh) {
    pHelpThd->_gen_mesh = gen_mesh;

    aiqMutex_init(&pHelpThd->_mutex);
    aiqCond_init(&pHelpThd->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name        = "LdcHMsgQ";
    msgqCfg._item_nums   = LDC_HELP_THREAD_MSG_MAX;
    msgqCfg._item_size   = sizeof(LdcGenMeshCfg_t);
    pHelpThd->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHelpThd->mMsgsQueue) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "init %s error", msgqCfg._name);
        goto fail;
    }

    pHelpThd->_base = aiqThread_init("LdcHelpTh", LdcGenMeshHelperThd_loop, pHelpThd);
    if (!pHelpThd->_base) goto fail;

    return XCAM_RETURN_NO_ERROR;

fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void LdcGenMeshHelperThd_deinit(LdcGenMeshHelperThd_t* pHelpThd) {
    ENTER_ANALYZER_FUNCTION();
    if (pHelpThd->mMsgsQueue) aiqList_deinit(pHelpThd->mMsgsQueue);
    if (pHelpThd->_base) aiqThread_deinit(pHelpThd->_base);

    aiqMutex_deInit(&pHelpThd->_mutex);
    aiqCond_deInit(&pHelpThd->_cond);
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn LdcGenMeshHelperThd_start(LdcGenMeshHelperThd_t* pHelpThd) {
    ENTER_ANALYZER_FUNCTION();
    pHelpThd->bQuit = false;
    if (!aiqThread_start(pHelpThd->_base)) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s failed", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcGenMeshHelperThd_stop(LdcGenMeshHelperThd_t* pHelpThd) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHelpThd->_mutex);
    pHelpThd->bQuit = true;
    aiqMutex_unlock(&pHelpThd->_mutex);
    aiqCond_broadcast(&pHelpThd->_cond);
    aiqThread_stop(pHelpThd->_base);
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pHelpThd->mMsgsQueue, pItem, rm) {
        rm    = true;
        pItem = aiqList_erase_item_locked(pHelpThd->mMsgsQueue, pItem);
    }
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

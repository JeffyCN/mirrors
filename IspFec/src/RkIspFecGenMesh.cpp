/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#include "include_prv/RkIspFecGenMesh.h"

#include <bits/unique_ptr.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>

#include "RkIspFecComm.h"

#define ENABLE_TIME_PROFILE 0

namespace RKISPFEC {

int32_t
RkIspFecGenMesh::init(int32_t srcWidth, int32_t srcHeight,
                         int32_t dstWidth, int32_t dstHeight,
                         const double *lightCenter,
                         const double *coeff,
                         enum rk_ispfec_correct_direction direction,
                         enum rk_ispfec_correct_style style)
{
    int32_t ret = -1;
    Profiler prof = {0};

    if (mInited) {
        rkfec_dbg(5, rkfec_debug, "genFecMesh has been initialized!!");
        return 0;
    }

    if (!lightCenter || !coeff) {
        rkfec_err("lightCenter or coeff is null!");
        return ret;
    }

    rkfec_info("src w: %d, h: %d, dst w: %d, h: %d", srcWidth, srcHeight, dstWidth, dstHeight);
    rkfec_info("the light center of lens: %.16f, %.16f", lightCenter[0], lightCenter[1]);
    rkfec_info("the coeff of lens: %.16f, %.16f, %.16f, %.16f", coeff[0], coeff[1], coeff[2],
               coeff[3]);

    mCoeff.cx = lightCenter[0];
    mCoeff.cy = lightCenter[1];
    mCoeff.a0 = coeff[0];
    mCoeff.a2 = coeff[1];
    mCoeff.a3 = coeff[2];
    mCoeff.a4 = coeff[3];

    RKALGO_MESH_TYPE_E enMeshType        = RKALGO_MESH_TYPE_FEC;     /* 硬件类型 */
    RKALGO_FEC_VERSION_E enFecVersion    = RKALGO_FEC_VERSION_1126B; /* 硬件FEC版本 */
    RKALGO_MESH_STEP_OPT_E enMeshStepOpt = RKALGO_MESH_STEP_32x16;   /* 映射表的采样步长选项 */

    /* 下采样浮点表的相关参数 */
    memset(&mFloatMeshInfo, 0, sizeof(mFloatMeshInfo));

    /* FEC定点化表的相关参数 */
    memset(&mFecInfo, 0, sizeof(mFecInfo));
    memset(&mFecMeshBuf, 0, sizeof(mFecMeshBuf));

    /* 计算下采样浮点表的尺寸参数 */
    mFloatMeshInfo.srcW          = srcWidth;
    mFloatMeshInfo.srcH          = srcHeight;
    mFloatMeshInfo.dstW          = dstWidth;
    mFloatMeshInfo.dstH          = dstHeight;
    mFloatMeshInfo.enMeshType    = enMeshType;    /* 硬件类型 */
    mFloatMeshInfo.enFecVersion  = enFecVersion;  /* 硬件FEC版本 */
    mFloatMeshInfo.enMeshStepOpt = enMeshStepOpt; /* 浮点映射表的采样步长选项 */
    if (direction == RK_ISPFEC_CORRECT_DIRECTION_X) {
        mFloatMeshInfo.correctX = 1; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        mFloatMeshInfo.correctY = 0; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    } else if (direction == RK_ISPFEC_CORRECT_DIRECTION_Y) {
        mFloatMeshInfo.correctX = 0; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        mFloatMeshInfo.correctY = 1; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    } else {
        mFloatMeshInfo.correctX = 1; /* 水平x方向校正: 1代表校正, 0代表不校正 */
        mFloatMeshInfo.correctY = 1; /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    }

    if (style == RK_ISPFEC_COMPRES_IMAGE_KEEP_FOV) {
        mFloatMeshInfo.saveMaxFovX = 0; /* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
    } else if (style == RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV) {
        mFloatMeshInfo.saveMaxFovX = 1; /* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
    }

    ret = calcFloatMeshSizeRK(&mFloatMeshInfo);
    if (-1 == ret) {
        rkfec_err("%s: failed to calcFloatMeshSizeRK", __func__);
        goto exit;
    }

    /* 下采样浮点表的内存申请 */
    mPf32MeshX = std::unique_ptr<float[]>(new float[mFloatMeshInfo.u64FloatMeshBufSize]);
    mPf32MeshY = std::unique_ptr<float[]>(new float[mFloatMeshInfo.u64FloatMeshBufSize]);

    /* 计算FEC定点化表的尺寸参数 */
    mFecInfo.srcW          = srcWidth;
    mFecInfo.srcH          = srcHeight;
    mFecInfo.dstW          = dstWidth;
    mFecInfo.dstH          = dstHeight;
    mFecInfo.enFecVersion  = enFecVersion;  /* 硬件FEC版本 */
    mFecInfo.enMeshStepOpt = enMeshStepOpt; /* FEC定点化表的采样步长选项 */
    ret                    = calcFecMeshSize(&mFecInfo);
    if (-1 == ret) {
        rkfec_err("%s: failed to calcFecMeshSize", __func__);
        goto exit;
    }

    /* 生成下采样浮点表的初始化 */
    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    ret = genFloatMeshNLevelInit(&mCoeff, &mFloatMeshInfo);
    if (-1 == ret) {
        rkfec_err("%s: failed to genFloatMeshNLevelInit", __func__);
        goto exit;
    }

    if (rkfec_debug > 4) rkfec_profiling_end(&prof, "genFloatMeshNLevelInit", 30);

    rkfec_info("I: fec mode: %d, direction: %d, mesh: w: %d, h: %d, total size: %" PRId64 " bytes",
               mStyle, mDirection, mFecInfo.meshW, mFecInfo.meshH, mFecInfo.u64FecMeshBufSize);

    mInited = true;

    return 0;

exit:
    return -1;
}

int32_t RkIspFecGenMesh::deinit()
{
    if (!mInited) {
        rkfec_err("genFecMesh hasn't been initialized!");
        return 0;
    }

    return 0;
}

int32_t RkIspFecGenMesh::genMesh(int32_t level)
{
    int32_t ret = -1;

    if (!mInited) {
        rkfec_err("GenfecMesh hasn't been initialized!");
        return 0;
    }

#ifdef RKFEC_HW_V20
    if (!mMesh) {
        rkfec_err("Mesh buffer is null!");
        return ret;
    }
#else
    if (!mMeshXi || !mMeshYi || !mMeshXf || !mMeshYf) {
        rkfec_err("Mesh buffer is null!");
        return ret;
    }
#endif

    rkfec_dbg(3, rkfec_debug, "the light center of lens: %.16f, %.16f", mCoeff.cx, mCoeff.cy);
    rkfec_dbg(3, rkfec_debug, "the coeff of lens: %.16f, %.16f, %.16f, %.16f", mCoeff.a0, mCoeff.a2,
              mCoeff.a3, mCoeff.a4);

    rkfec_dbg(3, rkfec_debug, "the level of mesh to be generated: %d", level);

    static Profiler prof = {0};
    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    /* 根据校正等级level，生成下采样浮点表 */
    ret = genFloatMeshNLevel(&mCoeff, &mFloatMeshInfo, level, mPf32MeshX.get(), mPf32MeshY.get());
    if (-1 == ret) {
        rkfec_err("%s: failed to genFloatMeshNLevel", __func__);
        return ret;
    }

    if (rkfec_debug > 4) rkfec_profiling_end(&prof, "genFloatMeshNLevel", 30);

        /* 下采样浮点表 ---> FEC定点化表 */
#ifdef RKFEC_HW_V20
    mFecMeshBuf.pu8FecMesh = (uint8_t*)mMesh;
#else
    mFecMeshBuf.pu16MeshXI = (uint16_t*)mMeshXi;
    mFecMeshBuf.pu8MeshXF  = (uint8_t*)mMeshXf;
    mFecMeshBuf.pu16MeshYI = (uint16_t*)mMeshYi;
    mFecMeshBuf.pu8MeshYF  = (uint8_t*)mMeshYf;
#endif

    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    ret = genFecMeshFromFloatMesh(&mFecInfo, mPf32MeshX.get(), mPf32MeshY.get(), &mFecMeshBuf);
    if (-1 == ret) {
        rkfec_err("%s: failed to genFecMeshFromFloatMesh", __func__);
        return ret;
    }

    if (rkfec_debug > 5) rkfec_profiling_end(&prof, "genFecMeshFromFloatMesh", 30);

    if (rkfec_debug > 4) {
#ifdef ANDROID_OS
        char savePath[512] = "/data/";
#else
        char savePath[512] = "/tmp/";
#endif

        /* 保存下采样浮点表 */
        saveFloatMeshRK(savePath, &mFloatMeshInfo, mPf32MeshX.get(), mPf32MeshY.get(), level);

        /* 保存FEC定点化表 */
        saveFecMesh(savePath, &mFecInfo, &mFecMeshBuf, level);
    }

    return 0;
}

int32_t RkIspFecGenMesh::calcFloateshSizeRK(RKALGO_FLOAT_MESH_INFO_RK_S* pStFloatMeshInfoRk) {
    uint32_t dstW                                = pStFloatMeshInfoRk->dstW;
    uint32_t dstH                                = pStFloatMeshInfoRk->dstH;
    RKALGO_MESH_TYPE_E enMeshType                = pStFloatMeshInfoRk->enMeshType;
    RKALGO_FEC_VERSION_E enFecVersion            = pStFloatMeshInfoRk->enFecVersion;
    RKALGO_LDCH_VERSION_E enLdchVersion          = pStFloatMeshInfoRk->enLdchVersion;
    RKALGO_LDCH_LDCV_VERSION_E enLdchLdcvVersion = pStFloatMeshInfoRk->enLdchLdcvVersion;
    RKALGO_MESH_STEP_OPT_E enMeshStepOpt         = pStFloatMeshInfoRk->enMeshStepOpt;

    uint32_t dstWidAlign = 0, dstHgtAlign = 0, meshStepW = 0, meshStepH = 0, meshW = 0, meshH = 0;
    uint64_t u64FloatMeshBufSize;
    /* 输出图像宽高对齐 */
    switch (enMeshType) {
        case RKALGO_MESH_TYPE_DEBUG:
            dstWidAlign = 32 * ((dstW + 31) / 32);
            dstHgtAlign = 32 * ((dstH + 31) / 32);
            break;
        case RKALGO_MESH_TYPE_FEC:
            dstWidAlign = 32 * ((dstW + 31) / 32);
            dstHgtAlign = 32 * ((dstH + 31) / 32);
            break;
        case RKALGO_MESH_TYPE_LDCH:
            dstWidAlign = 16 * ((dstW + 15) / 16);
            dstHgtAlign = 8 * ((dstH + 7) / 8);
            break;
        case RKALGO_MESH_TYPE_LDCV:
            dstWidAlign = 16 * ((dstW + 15) / 16);
            dstHgtAlign = 16 * ((dstH + 15) / 16);
            break;
        case RKALGO_MESH_TYPE_LDCH_LDCV:
            dstWidAlign = 16 * ((dstW + 15) / 16);
            dstHgtAlign = 16 * ((dstH + 15) / 16);
            break;
        default:
            rkfec_err("%s: error: unsupported enMeshType", __func__);
            goto error_exit;
    }

    /* 确定浮点映射表的采样步长 */
    switch (enMeshStepOpt) {
        case RKALGO_MESH_STEP_32x16:
            meshStepW = 32;
            meshStepH = 16;
            break;
        case RKALGO_MESH_STEP_16x8:
            meshStepW = 16;
            meshStepH = 8;
            break;
        case RKALGO_MESH_STEP_4x4:
            meshStepW = 4;
            meshStepH = 4;
            break;
        case RKALGO_MESH_STEP_1x1:
            meshStepW = 1;
            meshStepH = 1;
            break;
        case RKALGO_MESH_STEP_16x16:
            meshStepW = 16;
            meshStepH = 16;
            break;
        default:
            rkfec_err("%s: error: unsupported enMeshStepOpt", __func__);
            goto error_exit;
    } /* switch (enMeshStepOpt) */

    /* 判断采样步长与硬件是否匹配 */
    if (RKALGO_MESH_TYPE_DEBUG == enMeshType) {
        meshW = (1 == meshStepW) ? dstW : ((dstW + meshStepW - 1) / meshStepW + 1);
        meshH = (1 == meshStepH) ? dstH : ((dstH + meshStepH - 1) / meshStepH + 1);
    } else if (RKALGO_MESH_TYPE_FEC == enMeshType) {
        RKALGO_BOOL_E hwOpt0 = (RKALGO_FEC_VERSION_1109_1126 == enFecVersion)
                                   ? RKALGO_TRUE
                                   : RKALGO_FALSE; /* hardware for FEC in RV1109, RV1126 */
        RKALGO_BOOL_E hwOpt1 = (RKALGO_FEC_VERSION_3588 == enFecVersion)
                                   ? RKALGO_TRUE
                                   : RKALGO_FALSE; /* hardware for FEC in RK3588 */
        RKALGO_BOOL_E hwOpt2 = (RKALGO_FEC_VERSION_1126B == enFecVersion)
                                   ? RKALGO_TRUE
                                   : RKALGO_FALSE; /* hardware for FEC in RV1126B */
        RKALGO_BOOL_E stepOpt0 =
            (RKALGO_MESH_STEP_32x16 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt1 =
            (RKALGO_MESH_STEP_16x8 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt2 =
            (RKALGO_MESH_STEP_4x4 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        /* 对于FEC，采样步长只能是32x16或16x8或4x4 */
        RKALGO_BOOL_E b0 =
            ((hwOpt0 || hwOpt1) && stepOpt0 && (dstW > 1920)) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E b1 =
            ((hwOpt0 || hwOpt1) && stepOpt1 && (dstW <= 1920)) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E b2 = (hwOpt2 && (stepOpt0 || stepOpt1)) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E b3 =
            (hwOpt2 && stepOpt2 && (dstW <= 4096 && dstH <= 4096)) ? RKALGO_TRUE : RKALGO_FALSE;
        if (b0 || b1 || b2 || b3) {
            meshW = (dstWidAlign + meshStepW - 1) / meshStepW + 1;
            meshH = (dstHgtAlign + meshStepH - 1) / meshStepH + 1;
        } else {
            rkfec_err("%s: for FEC float mesh, enMeshStepOpt and enFecVersion not match", __func__);
            goto error_exit;
        }
    } else if (RKALGO_MESH_TYPE_LDCH == enMeshType) {
        RKALGO_BOOL_E hwOpt0 =
            (RKALGO_LDCH_VERSION_0 == enLdchVersion)
                ? RKALGO_TRUE
                : RKALGO_FALSE; /* hardware for LDCH in rv1109，rv1126，rk356x，rk3588 */
        RKALGO_BOOL_E hwOpt1 = (RKALGO_LDCH_VERSION_1 == enLdchVersion)
                                   ? RKALGO_TRUE
                                   : RKALGO_FALSE; /* hardware for LDCH in rv1106, rk3562, rk3576 */
        RKALGO_BOOL_E stepOpt0 =
            (RKALGO_MESH_STEP_16x8 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        /* 对于LDCH，采样步长只能是16x8 */
        RKALGO_BOOL_E b0 = ((hwOpt0 || hwOpt1) && stepOpt0) ? RKALGO_TRUE : RKALGO_FALSE;
        if (b0) {
            meshW = (dstWidAlign + meshStepW - 1) / meshStepW + 1;
            meshH = (dstHgtAlign + meshStepH - 1) / meshStepH + 1;
        } else {
            rkfec_err("%s: for LDCH float mesh, meshStepW and meshStepH must be 16x8", __func__);
            goto error_exit;
        }
    } else if (RKALGO_MESH_TYPE_LDCV == enMeshType) {
        RKALGO_BOOL_E stepOpt0 =
            (RKALGO_MESH_STEP_16x16 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        /* 对于LDCV，采样步长只能是16x16 */
        RKALGO_BOOL_E b0 = stepOpt0 ? RKALGO_TRUE : RKALGO_FALSE;
        if (b0) {
            meshW = (dstWidAlign + meshStepW - 1) / meshStepW + 1;
            meshH = (dstHgtAlign + meshStepH - 1) / meshStepH + 1;
        } else {
            rkfec_err("%s: for only LDCV float mesh, meshStepW and meshStepH must be 16x16",
                      __func__);
            goto error_exit;
        }

    } else if (RKALGO_MESH_TYPE_LDCH_LDCV == enMeshType) {
        RKALGO_BOOL_E hwOpt0 = (RKALGO_LDCH_LDCV_VERSION_3576 == enLdchLdcvVersion)
                                   ? RKALGO_TRUE
                                   : RKALGO_FALSE; /* hardware for LDCH + LDCV in rk3576 */
        RKALGO_BOOL_E stepOpt0 =
            (RKALGO_MESH_STEP_32x16 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt1 =
            (RKALGO_MESH_STEP_16x8 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt2 =
            (RKALGO_MESH_STEP_4x4 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt3 =
            (RKALGO_MESH_STEP_1x1 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        RKALGO_BOOL_E stepOpt4 =
            (RKALGO_MESH_STEP_16x16 == enMeshStepOpt) ? RKALGO_TRUE : RKALGO_FALSE;
        /* 对于LDCH + LDCV，采样步长可以任意 */
        RKALGO_BOOL_E b0 = (hwOpt0 && (stepOpt0 || stepOpt1 || stepOpt2 || stepOpt3 || stepOpt4))
                               ? RKALGO_TRUE
                               : RKALGO_FALSE;
        if (b0) { /* 注意: 当步长为1x1时，映射表宽高刚好等于(dstWidAlign+1, dstHgtAlign+1) */
            meshW = (dstWidAlign + meshStepW - 1) / meshStepW + 1;
            meshH = (dstHgtAlign + meshStepH - 1) / meshStepH + 1;
        } else {
            rkfec_err("%s: for LDCH + LDCV float mesh, meshStepW and meshStepH must be 1x1",
                      __func__);
            goto error_exit;
        }
    } else {
        rkfec_err("%s: unsupported mesh type", __func__);
        goto error_exit;
    }
    u64FloatMeshBufSize = meshW * meshH;

    pStFloatMeshInfoRk->dstWidAlign         = dstWidAlign;
    pStFloatMeshInfoRk->dstHgtAlign         = dstHgtAlign;
    pStFloatMeshInfoRk->meshStepW           = meshStepW;
    pStFloatMeshInfoRk->meshStepH           = meshStepH;
    pStFloatMeshInfoRk->meshW               = meshW;
    pStFloatMeshInfoRk->meshH               = meshH;
    pStFloatMeshInfoRk->u64FloatMeshBufSize = u64FloatMeshBufSize;

    return 0;

error_exit:
    pStFloatMeshInfoRk->dstWidAlign         = 0;
    pStFloatMeshInfoRk->dstHgtAlign         = 0;
    pStFloatMeshInfoRk->meshStepW           = 0;
    pStFloatMeshInfoRk->meshStepH           = 0;
    pStFloatMeshInfoRk->meshW               = 0;
    pStFloatMeshInfoRk->meshH               = 0;
    pStFloatMeshInfoRk->u64FloatMeshBufSize = 0;
    return -1;
}

int32_t RkIspFecGenMesh::calcFecMeshSize(RKALGO_FEC_INFO_S* pStFecInfo) {
    uint32_t dstW                        = pStFecInfo->dstW;
    uint32_t dstH                        = pStFecInfo->dstH;
    RKALGO_FEC_VERSION_E enFecVersion    = pStFecInfo->enFecVersion;
    RKALGO_MESH_STEP_OPT_E enMeshStepOpt = pStFecInfo->enMeshStepOpt;

    uint32_t dstWidAlign = 0, dstHgtAlign = 0, meshStepW = 0, meshStepH = 0, meshW = 0, meshH = 0;
    uint64_t u64FecMeshBufSize = 0;
    uint32_t spbNum = 0, spbMeshPNumH = 0, lastSpbH = 0, lastSpbMeshPNumH = 0;

    /* 输出图像宽高对齐 */
    dstWidAlign = 32 * ((dstW + 31) / 32);
    dstHgtAlign = 32 * ((dstH + 31) / 32);

    /* 确定FEC定点化表的采样步长 */
    switch (enMeshStepOpt) {
        case RKALGO_MESH_STEP_32x16:
            meshStepW = 32;
            meshStepH = 16;
            break;
        case RKALGO_MESH_STEP_16x8:
            meshStepW = 16;
            meshStepH = 8;
            break;
        case RKALGO_MESH_STEP_4x4:
            meshStepW = 4;
            meshStepH = 4;
            break;
        default:
            rkfec_err("%s: error: unsupported enMeshStepOpt for FEC", __func__);
            goto error_exit;
    } /* switch (enMeshStepOpt) */

    /* 计算FEC定点化表的大小 */
    meshW = (dstWidAlign + meshStepW - 1) / meshStepW + 1;
    meshH = (dstHgtAlign + meshStepH - 1) / meshStepH + 1;

    switch (enFecVersion) {
        case RKALGO_FEC_VERSION_1109_1126:
        case RKALGO_FEC_VERSION_3588:
            /* FEC定点化表的步长 */
            if ((RKALGO_MESH_STEP_32x16 != enMeshStepOpt) &&
                (RKALGO_MESH_STEP_16x8 != enMeshStepOpt)) {
                rkfec_err("%s: error: unsupported enMeshStepOpt for enFecVersion", __func__);
                goto error_exit;
            }

            /* 计算4个定点化表的相关参数 */
            spbNum           = (dstH + 128 - 1) / 128;
            spbMeshPNumH     = 128 / meshStepH + 1;
            lastSpbH         = (dstHgtAlign % 128 == 0) ? 128 : (dstHgtAlign % 128);
            lastSpbMeshPNumH = lastSpbH / meshStepH + 1;
            /* 计算每个定点化表(xi,xf,yi,yf)的大小 */
            u64FecMeshBufSize = (spbNum - 1) * meshW * spbMeshPNumH + meshW * lastSpbMeshPNumH;
            break;
        case RKALGO_FEC_VERSION_1126B:
            /* 计算整个交织定点化表的大小: {meshyf[7:0], 2'b00, meshyi[13:0], meshxf[7:0], 2'b00,
             * meshxi[13:0]} */
            u64FecMeshBufSize = meshW * meshH * 6;
            break;
        default:
            rkfec_err("%s: error: unsupported enFecVersion", __func__);
            goto error_exit;
    } /* switch (enFecVersion) */

    pStFecInfo->dstWidAlign       = dstWidAlign;
    pStFecInfo->dstHgtAlign       = dstHgtAlign;
    pStFecInfo->meshStepW         = meshStepW;
    pStFecInfo->meshStepH         = meshStepH;
    pStFecInfo->meshW             = meshW;
    pStFecInfo->meshH             = meshH;
    pStFecInfo->spbNum            = spbNum;
    pStFecInfo->spbMeshPNumH      = spbMeshPNumH;
    pStFecInfo->lastSpbH          = lastSpbH;
    pStFecInfo->lastSpbMeshPNumH  = lastSpbMeshPNumH;
    pStFecInfo->u64FecMeshBufSize = u64FecMeshBufSize;
    return 0;

error_exit:
    pStFecInfo->dstWidAlign       = 0;
    pStFecInfo->dstHgtAlign       = 0;
    pStFecInfo->meshStepW         = 0;
    pStFecInfo->meshStepH         = 0;
    pStFecInfo->meshW             = 0;
    pStFecInfo->meshH             = 0;
    pStFecInfo->spbNum            = 0;
    pStFecInfo->spbMeshPNumH      = 0;
    pStFecInfo->lastSpbH          = 0;
    pStFecInfo->lastSpbMeshPNumH  = 0;
    pStFecInfo->u64FecMeshBufSize = 0;
    return -1;
}

int32_t RkIspFecGenMesh::saveFloatMeshRK(const char* savePath,
                                         const RKALGO_FLOAT_MESH_INFO_RK_S* pStFloatMeshInfoRk,
                                         const float* pf32MeshX, const float* pf32MeshY,
                                         int32_t level) {
    uint32_t srcW                = pStFloatMeshInfoRk->srcW;
    uint32_t srcH                = pStFloatMeshInfoRk->srcH;
    uint32_t dstW                = pStFloatMeshInfoRk->dstW;
    uint32_t dstH                = pStFloatMeshInfoRk->dstH;
    uint32_t meshStepW           = pStFloatMeshInfoRk->meshStepW;
    uint32_t meshStepH           = pStFloatMeshInfoRk->meshStepH;
    uint64_t u64FloatMeshBufSize = pStFloatMeshInfoRk->u64FloatMeshBufSize;

    char floatMeshXPath[1024];
    char floatMeshYPath[1024];
    if (level >= 0) {
        sprintf(floatMeshXPath, "%s/floatMeshX_src%dx%d_dst%dx%d_level%03d.bin", savePath, srcW,
                srcH, dstW, dstH, level);
        sprintf(floatMeshYPath, "%s/floatMeshY_src%dx%d_dst%dx%d_level%03d.bin", savePath, srcW,
                srcH, dstW, dstH, level);
    } else {
        sprintf(floatMeshXPath, "%s/floatMeshX_src%dx%d_dst%dx%d.bin", savePath, srcW, srcH, dstW,
                dstH);
        sprintf(floatMeshYPath, "%s/floatMeshY_src%dx%d_dst%dx%d.bin", savePath, srcW, srcH, dstW,
                dstH);
    }

    FILE* fpFloatMeshX = fopen(floatMeshXPath, "wb");
    if (NULL == fpFloatMeshX) {
        rkfec_err("%s error: failed to save float mesh", __func__);
        return -1;
    } else {
        if (NULL != pf32MeshX) {
            fwrite(pf32MeshX, sizeof(float), u64FloatMeshBufSize, fpFloatMeshX);
        }
        fclose(fpFloatMeshX);
    }

    FILE* fpFloatMeshY = fopen(floatMeshYPath, "wb");
    if (NULL == fpFloatMeshY) {
        rkfec_err("%s error: failed to save float mesh", __func__);
        return -1;
    } else {
        if (NULL != pf32MeshY) {
            fwrite(pf32MeshY, sizeof(float), u64FloatMeshBufSize, fpFloatMeshY);
        }
        fclose(fpFloatMeshY);
    }
    return 0;
}

int32_t RkIspFecGenMesh::saveFecMesh(const char* savePath, const RKALGO_FEC_INFO_S* pStFecInfo,
                                     const RKALGO_FEC_MESH_S* pStFecMeshBuf, int32_t level) {
    uint32_t srcW                     = pStFecInfo->srcW;
    uint32_t srcH                     = pStFecInfo->srcH;
    uint32_t dstW                     = pStFecInfo->dstW;
    uint32_t dstH                     = pStFecInfo->dstH;
    RKALGO_FEC_VERSION_E enFecVersion = pStFecInfo->enFecVersion;
    uint32_t meshStepW                = pStFecInfo->meshStepW;
    uint32_t meshStepH                = pStFecInfo->meshStepH;
    uint64_t u64FecMeshBufSize        = pStFecInfo->u64FecMeshBufSize;

    uint16_t* pu16MeshXI = pStFecMeshBuf->pu16MeshXI;
    uint8_t* pu8MeshXF   = pStFecMeshBuf->pu8MeshXF;
    uint16_t* pu16MeshYI = pStFecMeshBuf->pu16MeshYI;
    uint8_t* pu8MeshYF   = pStFecMeshBuf->pu8MeshYF;
    uint8_t* pu8FecMesh  = pStFecMeshBuf->pu8FecMesh;

    if (RKALGO_FEC_VERSION_1126B == enFecVersion) {
        /* for FEC in RV1126B */
        char fecMeshPath[1024];
        if (level >= 0) {
            sprintf(fecMeshPath, "%s/rv1126b_fecMesh_src%dx%d_dst%dx%d_step%dx%d_level%03d.bin",
                    savePath, srcW, srcH, dstW, dstH, meshStepW, meshStepH, level);
        } else {
            sprintf(fecMeshPath, "%s/rv1126b_fecMesh_src%dx%d_dst%dx%d_step%dx%d.bin", savePath,
                    srcW, srcH, dstW, dstH, meshStepW, meshStepH);
        }
        FILE* fpMesh = fopen(fecMeshPath, "wb");
        if (NULL == fpMesh) {
            rkfec_err("%s error: failed to save FEC mesh!", __func__);
            return -1;
        } else {
            fwrite(pu8FecMesh, sizeof(uint8_t), u64FecMeshBufSize, fpMesh);
            fclose(fpMesh);
        }
    } else {
        /* for FEC in RV1109, RV1126, RK3588 */
        char fecMeshPathXi[1024];
        char fecMeshPathXf[1024];
        char fecMeshPathYi[1024];
        char fecMeshPathYf[1024];
        if (level >= 0) {
            sprintf(fecMeshPathXi, "%s/fecMesh_src%dx%d_dst%dx%d_level%03d_meshxi.bin", savePath,
                    srcW, srcH, dstW, dstH, level);
            sprintf(fecMeshPathXf, "%s/fecMesh_src%dx%d_dst%dx%d_level%03d_meshxf.bin", savePath,
                    srcW, srcH, dstW, dstH, level);
            sprintf(fecMeshPathYi, "%s/fecMesh_src%dx%d_dst%dx%d_level%03d_meshyi.bin", savePath,
                    srcW, srcH, dstW, dstH, level);
            sprintf(fecMeshPathYf, "%s/fecMesh_src%dx%d_dst%dx%d_level%03d_meshyf.bin", savePath,
                    srcW, srcH, dstW, dstH, level);
        } else {
            sprintf(fecMeshPathXi, "%s/fecMesh_src%dx%d_dst%dx%d_meshxi.bin", savePath, srcW, srcH,
                    dstW, dstH);
            sprintf(fecMeshPathXf, "%s/fecMesh_src%dx%d_dst%dx%d_meshxf.bin", savePath, srcW, srcH,
                    dstW, dstH);
            sprintf(fecMeshPathYi, "%s/fecMesh_src%dx%d_dst%dx%d_meshyi.bin", savePath, srcW, srcH,
                    dstW, dstH);
            sprintf(fecMeshPathYf, "%s/fecMesh_src%dx%d_dst%dx%d_meshyf.bin", savePath, srcW, srcH,
                    dstW, dstH);
        }
        FILE* fpMeshxi = NULL;
        FILE* fpMeshxf = NULL;
        FILE* fpMeshyi = NULL;
        FILE* fpMeshyf = NULL;
        fpMeshxi       = fopen(fecMeshPathXi, "wb");
        fpMeshxf       = fopen(fecMeshPathXf, "wb");
        fpMeshyi       = fopen(fecMeshPathYi, "wb");
        fpMeshyf       = fopen(fecMeshPathYf, "wb");
        if (NULL == fpMeshxi || NULL == fpMeshxf || NULL == fpMeshyi || NULL == fpMeshyf) {
            rkfec_err("%s error: failed to save FEC mesh 4 bin!", __func__);
            return -1;
        } else {
            fwrite(pu16MeshXI, sizeof(uint16_t), u64FecMeshBufSize, fpMeshxi);
            fwrite(pu8MeshXF, sizeof(uint8_t), u64FecMeshBufSize, fpMeshxf);
            fwrite(pu16MeshYI, sizeof(uint16_t), u64FecMeshBufSize, fpMeshyi);
            fwrite(pu8MeshYF, sizeof(uint8_t), u64FecMeshBufSize, fpMeshyf);
            fclose(fpMeshxi);
            fclose(fpMeshxf);
            fclose(fpMeshyi);
            fclose(fpMeshyf);
        }
    }
    return 0;
}
};

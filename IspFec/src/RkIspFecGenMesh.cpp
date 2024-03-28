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

#include "RkIspFecGenMesh.h"

#include <stdio.h>

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

    if (mInited) {
        printf("genFecMesh has been initialized!!\n");
        return 0;
    }

    if (!lightCenter || !coeff) {
        printf("E: lightCenter or coeff is null!\n");
        return ret;
    }

    printf("I: src w: %d, h: %d, dst w: %d, h: %d\n", srcWidth, srcHeight, dstWidth, dstHeight);
    printf("I: the light center of lens: %.16f, %.16f\n", lightCenter[0], lightCenter[1]);
    printf("I: the coeff of lens: %.16f, %.16f, %.16f, %.16f\n", coeff[0], coeff[1], coeff[2], coeff[3]);

    mParams.isFecOld = 1;

    if (direction == RK_ISPFEC_CORRECT_DIRECTION_X) {
        mParams.correctX = 1;
        mParams.correctY = 0;
    } else if (direction == RK_ISPFEC_CORRECT_DIRECTION_X) {
        mParams.correctX = 0;
        mParams.correctY = 1;
    } else {
        mParams.correctX = 1;
        mParams.correctY = 1;
    }

    if (style == RK_ISPFEC_COMPRES_IMAGE_KEEP_FOV) {
        mParams.saveMaxFovX = 1;
    } else if (style == RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV) {
        mParams.saveMaxFovX = 0;
    }

    mParams.saveMesh4bin = false;
    if (mParams.saveMesh4bin) {
        sprintf(mParams.mesh4binPath, "/tmp/");
    }

    mCoeff.cx = lightCenter[0];
    mCoeff.cy = lightCenter[1];
    mCoeff.a0 = coeff[0];
    mCoeff.a2 = coeff[1];
    mCoeff.a3 = coeff[2];
    mCoeff.a4 = coeff[3];

    genFecMeshInit(srcWidth, srcHeight, dstWidth, dstHeight, mParams, mCoeff);

    printf("I: fec mode: %d, direction: %d, level: %d, mesh: w: %d, h: %d, total size: %d\n",
           mStyle, mDirection, mLevel,
           mParams.meshSizeW, mParams.meshSizeH, mParams.meshSize4bin);

    mInited = true;

    return 0;
}

int32_t RkIspFecGenMesh::deinit()
{
    if (!mInited) {
        printf("genFecMesh hasn't been initialized!\n");
        return 0;
    }

    genFecMeshDeInit(mParams);

    return 0;
}

int32_t RkIspFecGenMesh::genMesh(int32_t level)
{
    int32_t ret = -1;

    if (!mInited) {
        printf("E: genfecMesh hasn't been initialized!\n");
        return 0;
    }

    if (!mMeshXi || !mMeshYi || !mMeshXf || !mMeshYf) {
        printf("E: mesh buffer is null!\n");
        return ret;
    }

    printf("I: the light center of lens: %.16f, %.16f\n", mCoeff.cx, mCoeff.cy);
    printf("I: the coeff of lens: %.16f, %.16f, %.16f, %.16f\n",
           mCoeff.a0, mCoeff.a2, mCoeff.a3, mCoeff.a4);

    printf("I: the level of mesh to be generated: %d\n", level);

    bool b_ret = genFECMeshNLevel(mParams, mCoeff, level,
            (uint16_t*)mMeshXi, (uint8_t*)mMeshXf,
            (uint16_t*)mMeshYi, (uint8_t*)mMeshYf);
    if (!b_ret) {
        printf("E: Failed to generate fec mesh!\n");
        return ret;
    }

    return 0;
}

};

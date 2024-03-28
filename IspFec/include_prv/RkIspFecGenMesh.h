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

#ifndef _RK_ISPFEC_GEN_MESH_MANAGER_H_
#define _RK_ISPFEC_GEN_MESH_MANAGER_H_

#include "genMesh.h"
#include "rk_ispfec_api.h"

#include <stdint.h>

namespace RKISPFEC {

class RkIspFecGenMesh {
public:
    RkIspFecGenMesh() = default;
    virtual ~RkIspFecGenMesh() = default;

    int32_t init(int32_t srcWidth, int32_t srcHeight, int32_t dstWidth, int32_t dstHeight,
                 const double *lightCenter, const double *coeff,
                 enum rk_ispfec_correct_direction direction, enum rk_ispfec_correct_style style);
    int32_t deinit();
    int32_t genMesh(int32_t level);

    /*
     * Warning: the direction/style take effect when configured before init
     */
    void setCorrectDirection(enum rk_ispfec_correct_direction direction) {
		mDirection = direction;
	};

    void setCorrectStyle(enum rk_ispfec_correct_style style) {
		mStyle = style;
	};

    void setMeshBuf(void* meshXi, void* meshYi, void* meshXf, void* meshYf) {
        mMeshXi = meshXi;
        mMeshYi = meshYi;
        mMeshXf = meshXf;
        mMeshYf = meshYf;
    };

private:
    bool                				mInited {false};

    enum rk_ispfec_correct_direction 	mDirection {RK_ISPFEC_CORRECT_DIRECTION_XY};
    enum rk_ispfec_correct_style 	    mStyle {RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV};
    int32_t                     		mLevel {100};

    struct CameraCoeff          		mCoeff;
    struct FecParams            		mParams;

    void*                       		mMeshXi;
    void*                       		mMeshYi;
    void*                       		mMeshXf;
    void*                       		mMeshYf;

};

};
#endif

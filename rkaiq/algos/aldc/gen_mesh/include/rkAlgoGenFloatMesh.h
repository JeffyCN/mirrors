#ifndef __RKALGO_GEN_FLOAT_MESH_H__
#define __RKALGO_GEN_FLOAT_MESH_H__
// clang-format off

#include "rkAlgoGenMeshComm.h"
#include "rkAlgoGenFloatMeshFromRK.h"

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <string>
#include <algorithm>
// #include <opencv2/opencv.hpp>

/*
    [Function]      : 根据采样步长计算下采样的浮点mesh表的宽高
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-02-16
    [input]         : enMeshType: 指定所需的mesh表类型: DEBUG, FEC, LDCH, LDCV, etc., 主要和对齐方式有关。
                      dstW, dstH: 目标输出图的宽高
                      meshStepW, meshStepH: 采样步长，当meshStepW == 1 && meshStepH == 1时，退化到全分辨率的浮点mesh表
    [output]        : meshW, meshH: 浮点mesh表的宽高
*/
int32_t calcFloatMeshSize(
    const RKALGO_MESH_TYPE_E enMeshType, const uint32_t dstW, const uint32_t dstH,
    const uint32_t meshStepW, const uint32_t meshStepH, uint32_t *meshW, uint32_t *meshH
);


int32_t calcFloatMeshSizeRK(RKALGO_GEN_FLOAT_MESH_INFO_S *genFloatMeshInfo);

// clang-format on
#endif  // !__RKALGO_GEN_FLOAT_MESH_H__

#ifndef __RKALGO_GEN_LDCH_MESH_H__
#define __RKALGO_GEN_LDCH_MESH_H__

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkAlgoGenMeshComm.h"

#ifdef __cplusplus
#include <algorithm>
#include <string>
#endif  // __cplusplus

/*
    [Function]      : 根据LDCH的版本，计算LDCH mesh表所需的buffer大小
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-02-16
    [input]         : enLdchVersion: 指定LDCH版本: 1109, 1126, 3588, etc.
                      dstW, dstH: 输出图像的宽高
    [output]        : u64LdchMeshBufSize: 表示LDCH mesh表所需的buffer大小
*/
int32_t calcLdchMeshSize(const RKALGO_LDCH_VERSION_E enLdchVersion, const uint32_t dstW,
                         const uint32_t dstH, uint64_t* pu64LdchMeshBufSize);

/*
    [Function]      : float类型的mesh表转换成LDCH mesh表
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-02-16
    [input]         : enLdchVersion: 指定LDCH版本：1109, 1126, 3588, etc.
                      srcW, srcH: 输入图的宽高
                      dstW, dstH: 输出图像的宽高
                      pf32MeshX: float类型的mesh表
    [output]        : pu16MeshX: LDCH mesh表
*/
int32_t genLdchMeshFromFloatMesh(const RKALGO_LDCH_VERSION_E enLdchVersion, const uint32_t srcW,
                                 const uint32_t srcH, const uint32_t dstW, const uint32_t dstH,
                                 const float* pf32MeshX, uint16_t* pu16MeshX);

/*
    [Function]      : 生成无畸变校正的LDCH mesh表
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-03-06
    [input]         : enLdchVersion: 指定LDCH版本：1109, 1126, 3588, etc.
                      srcW, srcH: 输入图的宽高
                      dstW, dstH: 输出图像的宽高
    [output]        : pu16MeshX: LDCH mesh表
*/
int32_t genLdchMeshZeroCorrect(const RKALGO_LDCH_VERSION_E enLdchVersion, const uint32_t srcW,
                               const uint32_t srcH, const uint32_t dstW, const uint32_t dstH,
                               uint16_t* pu16MeshX);

#endif  // !__RKALGO_GEN_LDCH_MESH_H__

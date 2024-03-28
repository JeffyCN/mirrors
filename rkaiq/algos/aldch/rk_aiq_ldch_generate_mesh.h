/*
 * rk_aiq_ldch_generate_mesh.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
#ifndef _RK_AIQ_LDCH_GENERATE_MESH_H_
#define _RK_AIQ_LDCH_GENERATE_MESH_H_

#include "aldch/rk_aiq_types_aldch_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

XCamReturn get_ldch_buf(LDCHContext_t* ldchCtx, uint8_t isp_id);
XCamReturn release_ldch_buf(LDCHContext_t* ldchCtx, uint8_t isp_id);
bool read_mesh_from_file(LDCHContext_t* ldchCtx, const char* fileName);
XCamReturn aiqGenLdchMeshInit(LDCHContext_t* ldchCtx);
XCamReturn aiqGenLdchMeshDeInit(LDCHContext_t* ldchCtx);
bool aiqGenMesh(LDCHContext_t* ldchCtx, int32_t level, uint8_t isp_id);
void put_ldch_buf(LDCHContext_t* ldchCtx, uint8_t isp_id);

RKAIQ_END_DECLARE

#endif

/*
 * rk_aiq_ldch_generate_mesh.cpp
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

#include "xcam_log.h"
#include "rk_aiq_ldch_generate_mesh.h"

#define LDCH_CUSTOM_MESH "ldch_custom_mesh.bin"

RKAIQ_BEGIN_DECLARE


static XCamReturn alloc_ldch_buf(LDCHContext_t* ldchCtx)
{
    if (!ldchCtx->hasAllocShareMem.load(std::memory_order_acquire)) {
        LOGD_ALDCH("alloc ldch buf");
        release_ldch_buf(ldchCtx);
        rk_aiq_share_mem_config_t share_mem_config;
        share_mem_config.alloc_param.width =  ldchCtx->dst_width;
        share_mem_config.alloc_param.height = ldchCtx->dst_height;
        share_mem_config.mem_type = MEM_TYPE_LDCH;
        ldchCtx->share_mem_ops->alloc_mem(0, ldchCtx->share_mem_ops,
                &share_mem_config,
                &ldchCtx->share_mem_ctx);
        if (ldchCtx->share_mem_ctx) {
            ldchCtx->hasAllocShareMem.store(true, std::memory_order_release);
        } else {
            LOGE_ALDCH("Failed to alloc shared mem");
            return XCAM_RETURN_ERROR_MEM;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn release_ldch_buf(LDCHContext_t* ldchCtx)
{
    if (ldchCtx->hasAllocShareMem.load(std::memory_order_acquire)) {
        LOGD_ALDCH("release ldch buf");
        if (ldchCtx->share_mem_ctx)
            ldchCtx->share_mem_ops->release_mem(0, ldchCtx->share_mem_ctx);
        ldchCtx->hasAllocShareMem.store(false, std::memory_order_release);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn get_ldch_buf(LDCHContext_t* ldchCtx)
{
    if (alloc_ldch_buf(ldchCtx) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to alloc ldch buf\n");
        return XCAM_RETURN_ERROR_MEM;
    }

    ldchCtx->ldch_mem_info = (rk_aiq_ldch_share_mem_info_t *)
            ldchCtx->share_mem_ops->get_free_item(0, ldchCtx->share_mem_ctx);
    if (ldchCtx->ldch_mem_info == NULL || \
        (ldchCtx->ldch_mem_info && \
         ldchCtx->ldch_mem_info->state[0] != MESH_BUF_INIT)) {
        LOGE_ALDCH("%s: no free ldch buf", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ldchCtx->ldch_mem_info->state[0] = MESH_BUF_WAIT2CHIP; //mark that this buf is using.

    LOGD_ALDCH("LDCH get buf: fd %d, addr %p, size %d",
            ldchCtx->ldch_mem_info->fd,
            ldchCtx->ldch_mem_info->addr,
            ldchCtx->ldch_mem_info->size);

    return XCAM_RETURN_NO_ERROR;
}

void put_ldch_buf(LDCHContext_t* ldchCtx)
{
    if (ldchCtx->ldch_mem_info && ldchCtx->ldch_mem_info->state[0] == MESH_BUF_WAIT2CHIP) {
        ldchCtx->ldch_mem_info->state[0] = MESH_BUF_INIT;
        LOGD_ALDCH("LDCH put buf: fd %d, addr %p, size %d",
                ldchCtx->ldch_mem_info->fd,
                ldchCtx->ldch_mem_info->addr,
                ldchCtx->ldch_mem_info->size);

        ldchCtx->ldch_mem_info = NULL;
    }
}

bool
read_mesh_from_file(LDCHContext_t* ldchCtx, const char* fileName)
{
    FILE* ofp;
    ofp = fopen(fileName, "rb");
    if (ofp != NULL) {
        unsigned short hpic, vpic, hsize, vsize, hstep, vstep = 0;
        uint32_t lut_size = 0;

        fread(&hpic, sizeof(unsigned short), 1, ofp);
        fread(&vpic, sizeof(unsigned short), 1, ofp);
        fread(&hsize, sizeof(unsigned short), 1, ofp);
        fread(&vsize, sizeof(unsigned short), 1, ofp);
        fread(&hstep, sizeof(unsigned short), 1, ofp);
        fread(&vstep, sizeof(unsigned short), 1, ofp);

        lut_size = hsize * vsize *  sizeof(uint16_t);
        LOGW_ALDCH("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);
        unsigned int num = fread(ldchCtx->ldch_mem_info->addr, 1, lut_size, ofp);
        fclose(ofp);

        if (num != lut_size) {
            LOGE_ALDCH("mismatched lut calib file");
            return false;
        }

        if (ldchCtx->src_width != hpic || ldchCtx->src_height != vpic) {
            LOGE_ALDCH("mismatched lut pic resolution: src %dx%d, lut %dx%d",
                    ldchCtx->src_width, ldchCtx->src_height, hpic, vpic);
            return false;
        }

        ldchCtx->lut_h_size = hsize;
        ldchCtx->lut_v_size = vsize;
        ldchCtx->lut_mapxy_size = lut_size;
        ldchCtx->lut_h_size = hsize / 2; //word unit

        LOGW_ALDCH("check file, size: %d, num: %d", ldchCtx->lut_mapxy_size, num);
    } else {
        LOGE_ALDCH("lut file %s not exist", fileName);
        return false;
    }

    return true;
}


#if GENMESH_ONLINE
XCamReturn aiqGenLdchMeshInit(LDCHContext_t* ldchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ldchCtx->genLdchMeshInit) {
        LOGW_ALDCH("genLDCHMesh has been initialized!!\n");
        if (get_ldch_buf(ldchCtx) != XCAM_RETURN_NO_ERROR) {
            LOGE_ALDCH("Failed to get ldch buf\n");
            return XCAM_RETURN_ERROR_MEM;
        }
        return XCAM_RETURN_NO_ERROR;
    }

    ldchCtx->ldchParams.isLdchOld = 1;
    ldchCtx->ldchParams.saveMeshX = false;
    if (ldchCtx->ldchParams.saveMeshX) {
        sprintf(ldchCtx->ldchParams.meshPath, "/tmp/");
    }
	genLdchMeshInit(ldchCtx->src_width, ldchCtx->src_height,
                    ldchCtx->dst_width, ldchCtx->dst_height,
                    ldchCtx->ldchParams, ldchCtx->camCoeff);

    ldchCtx->lut_h_size = (ldchCtx->ldchParams.meshSizeW + 1) / 2; //word unit
    ldchCtx->lut_v_size = ldchCtx->ldchParams.meshSizeH;
    ldchCtx->lut_mapxy_size = ldchCtx->ldchParams.meshSize * sizeof(unsigned short);
    LOGI_ALDCH("ldch en %d, h/v size(%dx%d), mapxy size(%d), correct_level: %d",
               ldchCtx->ldch_en,
               ldchCtx->lut_h_size,
               ldchCtx->lut_v_size,
               ldchCtx->lut_mapxy_size ,
               ldchCtx->correct_level);

    if (get_ldch_buf(ldchCtx) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to get ldch buf\n");
        ret = XCAM_RETURN_ERROR_MEM;
    }

    ldchCtx->genLdchMeshInit = true;

    return ret;
}

XCamReturn aiqGenLdchMeshDeInit(LDCHContext_t* ldchCtx)
{
    if (!ldchCtx->genLdchMeshInit) {
        return XCAM_RETURN_NO_ERROR;
    }

	genLdchMeshDeInit(ldchCtx->ldchParams);
    release_ldch_buf(ldchCtx);

    ldchCtx->genLdchMeshInit = false;

    return XCAM_RETURN_NO_ERROR;
}

bool aiqGenMesh(LDCHContext_t* ldchCtx, int32_t level)
{
    bool success = false;

    if (level == 0) {
        char filename[512];
        sprintf(filename, "%s/%s",
                ldchCtx->resource_path,
                LDCH_CUSTOM_MESH);
        success = read_mesh_from_file(ldchCtx, filename);
        if (success) {
            LOGW_ALDCH("read mesh from %s", filename);
        }
    }

    if (!success && ldchCtx->ldch_mem_info)
        success = genLDCMeshNLevel(ldchCtx->ldchParams, ldchCtx->camCoeff,
                                   level, (uint16_t *)ldchCtx->ldch_mem_info->addr);

    return success;
}
#endif

RKAIQ_END_DECLARE

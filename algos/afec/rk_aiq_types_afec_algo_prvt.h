/*
 *rk_aiq_types_afec_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_AFEC_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_AFEC_ALGO_PRVT_H_

#include "RkAiqCalibDbTypes.h"
#include <xcam_mutex.h>
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "xcam_log.h"
#include "gen_mesh/genMesh.h"
#include "afec/rk_aiq_types_afec_algo_int.h"
#include "rk_aiq_types_priv.h"

RKAIQ_BEGIN_DECLARE

using namespace XCam;

typedef enum {
    FEC_CORRECT_LEVEL0,          // 100%
    FEC_CORRECT_LEVEL1,          // 75%
    FEC_CORRECT_LEVEL2,          // 50%
    FEC_CORRECT_LEVEL3,          // 25%
    FEC_BYPASS
} FECCorrectLevel;

typedef enum FECState_e {
    FEC_STATE_INVALID           = 0,                   /**< initialization value */
    FEC_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    FEC_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    FEC_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    FEC_STATE_MAX                                      /**< max */
} FECState_t;

class RKAiqAfecThread;

typedef struct FECContext_s {
    unsigned char initialized;
    unsigned int fec_en;
    fec_correct_mode_t mode;
    unsigned int mesh_density; //0:16x8 1:32x16
    unsigned int fec_mesh_h_size;
    unsigned int fec_mesh_v_size;
    unsigned int fec_mesh_size;
    int correct_level;
    fec_correct_direction_t correct_direction;
    int src_width;
    int src_height;
    int dst_width;
    int dst_height;
    unsigned int sw_rd_vir_stride;
    unsigned int sw_wr_yuv_format; //0:YUV420 1:YUV422
    unsigned int sw_wr_vir_stride;
    unsigned int sw_fec_wr_fbce_mode; //0:normal 1:fbec
    unsigned short* meshxi;
    unsigned char* meshxf;
    unsigned short* meshyi;
    unsigned char* meshyf;
    char meshfile[256];
    const char* resource_path;
    int meshSizeW;
    int meshSizeH;
    int meshStepW;
    int meshStepH;
    struct CameraCoeff camCoeff;
    FecParams fecParams;
    FECState_t eState;

    std::atomic<bool> isAttribUpdated;
    rk_aiq_fec_cfg_t user_config;
    SmartPtr<RKAiqAfecThread> afecReadMeshThread;
    isp_drv_share_mem_ops_t *share_mem_ops;
    rk_aiq_fec_share_mem_info_t *fec_mem_info;
    void* share_mem_ctx;
} FECContext_t;

typedef struct FECContext_s* FECHandle_t;

typedef struct _RkAiqAlgoContext {
    FECHandle_t hFEC;
    void* place_holder[0];
} RkAiqAlgoContext;

RKAIQ_END_DECLARE

class RKAiqAfecThread
    : public Thread {
public:
    RKAiqAfecThread(FECHandle_t fecHandle)
        : Thread("afecThread")
          , hFEC(fecHandle) {};
    ~RKAiqAfecThread() {
        mAttrQueue.clear ();
    };

    void triger_stop() {
        mAttrQueue.pause_pop ();
    };

    void triger_start() {
        mAttrQueue.resume_pop ();
    };

    bool push_attr (const SmartPtr<rk_aiq_fec_cfg_t> buffer) {
        mAttrQueue.push (buffer);
        return true;
    };

    bool is_empty () {
        return mAttrQueue.is_empty();
    };

    void clear_attr () {
        mAttrQueue.clear ();
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mAttrQueue.clear ();
    };
    virtual bool loop ();
private:
    FECHandle_t hFEC;
    SafeList<rk_aiq_fec_cfg_t> mAttrQueue;
};

#endif


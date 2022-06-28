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
#include "aldch/rk_aiq_types_aldch_algo_int.h"
#include "rk_aiq_types_priv.h"
RKAIQ_BEGIN_DECLARE

using namespace XCam;

#define MAP_TO_255LEVEL(level, level_max) \
    ((float)(level) / 255 * (level_max));

typedef enum {
    LDCH_CORRECT_LEVEL0,        // 100%
    LDCH_CORRECT_LEVEL1,        // 75%
    LDCH_CORRECT_LEVEL2,        // 100%
    LDCH_CORRECT_LEVEL3,        // 75%
    LDCH_BYPASS
} LDCHCorrectLevel;

typedef enum LDCHState_e {
    LDCH_STATE_INVALID           = 0,                   /**< initialization value */
    LDCH_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    LDCH_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    LDCH_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    LDCH_STATE_MAX                                      /**< max */
} LDCHState_t;

class RKAiqAldchThread;

typedef struct LDCHContext_s {
    unsigned char initialized;
    unsigned int src_width;
    unsigned int src_height;
    unsigned int dst_width;
    unsigned int dst_height;
    unsigned int ldch_en;
    unsigned int lut_h_size;
    unsigned int lut_v_size;
    unsigned int lut_mapxy_size;
    unsigned short* lut_mapxy;
    char meshfile[256];
    int correct_level;
    int correct_level_max;
    const char* resource_path;
    std::atomic<bool> genLdchMeshInit;

    struct CameraCoeff camCoeff;
    LdchParams ldchParams;
    LDCHState_t eState;
    std::atomic<bool> isAttribUpdated;
    rk_aiq_ldch_cfg_t user_config;
    SmartPtr<RKAiqAldchThread> aldchReadMeshThread;
    isp_drv_share_mem_ops_t *share_mem_ops;
    rk_aiq_ldch_share_mem_info_t *ldch_mem_info;
    void* share_mem_ctx;
} LDCHContext_t;

typedef struct LDCHContext_s* LDCHHandle_t;

typedef struct _RkAiqAlgoContext {
    LDCHHandle_t hLDCH;
    void* place_holder[0];
} RkAiqAlgoContext;

RKAIQ_END_DECLARE

class RKAiqAldchThread
    : public Thread {
public:
    RKAiqAldchThread(LDCHHandle_t ldchHandle)
        : Thread("ldchThread")
          , hLDCH(ldchHandle) {};
    ~RKAiqAldchThread() {
        mAttrQueue.clear ();
    };

    void triger_stop() {
        mAttrQueue.pause_pop ();
    };

    void triger_start() {
        mAttrQueue.resume_pop ();
    };

    bool push_attr (const SmartPtr<rk_aiq_ldch_cfg_t> buffer) {
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
    LDCHHandle_t hLDCH;
    SafeList<rk_aiq_ldch_cfg_t> mAttrQueue;
};

#endif


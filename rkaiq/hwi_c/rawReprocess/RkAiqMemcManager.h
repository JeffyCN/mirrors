/*
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

#ifndef _RK_AIQ_MEMC_MANAGER_C_H_
#define _RK_AIQ_MEMC_MANAGER_C_H_

#include "aiq_core_c/aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "rk_aiq_isp39_modules.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "hwi_c/rawReprocess/gyro_memc.h"

#define RKAIQ_MEMC_ONLINE 0

typedef struct AiqMencManager_s AiqMemcManager_t;

typedef struct memc_callback_result_s {
    uint32_t frame_id;
    AiqV4l2Buffer_t*    buf_s;
    AiqV4l2Buffer_t*    buf_m;
    AiqV4l2Buffer_t*    buf_l;
    const rk_aiq_btnr_share_mem_info_t* mesh_info;
    AiqSensorExpInfo_t* expParam;
    uint64_t sof_time;
    void *iir_addr;
} memc_callback_result_t;

typedef struct AiqMemcProcThread_s
{
    AiqThread_t* _base;
    /* data */
    AiqList_t* mMsgsQueue;
    AiqList_t* mSclList;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    AiqMutex_t _iir_mutex;
    AiqCond_t _iir_cond;
    bool bQuit;
    AiqMemcManager_t *mMemcMan;
} AiqMemcProcThread_t;

typedef struct MemcIirMsg_s
{
    uint32_t frame_id;
    uint8_t  iir_index;
} MemcIirMsg_t;

typedef struct AiqMemcIirThread_s
{
    AiqThread_t* _base;
    /* data */
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
    AiqMemcManager_t *mMemcMan;
    uint32_t mDebugFrame;
    uint32_t mLastIirId;
    uint32_t mLastIirEvtCnt;
    uint32_t mNewIirId;
    uint32_t mNewIirEvtCnt;
    bool mIirPkWarpEn;
    struct timeval start;
    struct timeval end;
    double mWarpTime;
    double mWarpTimeTotal;
} AiqMemcIirThread_t;

typedef struct AiqMemcParamsMsg_s
{
    uint32_t frameId;
    uint8_t type;
    void *data;
} AiqMemcParamsMsg_t;

typedef struct AiqMemcParamsArray_s
{
    uint32_t frameId;
    uint8_t mark;
    const rk_aiq_btnr_share_mem_info_t* mesh;
    AiqV4l2Buffer_t *param_buf;
    bool ready;
} AiqMemcParamsArray_t;

typedef struct AiqMemcParams_s
{
    AiqThread_t* _base;
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
    AiqMemcManager_t *mMemcMan;
    AiqMap_t* mParamsMap;
} AiqMemcParamsThread_t;

typedef struct AiqMencManager_s {
    // key: frame_id, val: pending_params_t
    AiqMap_t* mFullParamsPendingMap;
    AiqMutex_t _mFullParam_mutex;
    AiqMemcParamsThread_t mIspParamTh;
    AiqMap_t* mSofTimeMap;
    AiqRawStreamProcUnit_t* _proc_stream;
    AiqAnalyzerCb_t mMencManagerCb;
    AiqAnalyzerCb_t *mCb;
    uint32_t *mLastResId;
    AiqPool_t *mMemcResPool;
    AiqList_t* mIirMsgList;
    AiqMemcProcThread_t mProcTh;
    AiqMemcIirThread_t mIirProcTh;
    AiqMemcIirThread_t mIirSyncTh;
    rk_aiq_exposure_sensor_descriptor mSnsDes;
    uint32_t _rolling_shutter_skew;
    bool _import_buffer;
    AiqImuData_t mImuData[200];
    int mImuNum;
    AiqList_t *mImuDataList;
    uint64_t mNewImuTime;
    AiqCamHwBase_t* mCamHw;

    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    uint32_t Width;
    uint32_t Height;

    rk_gyro_memc_param_t _param;

    int mCamPhyId;
    bool _first_param;
    bool _stream_start;

    bool mMemcEn;
    bool mIirPkWarp;
    rk_gyro_memc_internal_param_t *_algo_ctx;

    int iir_fd[RKISP_BUFFER_MAX];
    void* iir_address[RKISP_BUFFER_MAX];
    uint32_t pk_attr_off;
    uint32_t iir_size;
    uint32_t mPerProLineCnt;
    uint32_t mIirPrcLineCnt;
    struct rkisp_aiisp_cfg mAiispCfg;

    uint32_t _last_iir_id;
    uint32_t _last_proc_id;

    AiqV4l2Buffer_t *pre_buf_s;
    AiqV4l2Buffer_t *pre_buf_m;
    AiqV4l2Buffer_t *pre_buf_l;

    AiqV4l2Buffer_t *pre_buf_scl;
    btnr_api_attrib_t *btnr_attr;
    int _memc_mode;
} AiqMemcManager_t;

XCamReturn AiqMemcManager_init(AiqMemcManager_t* pMemcMan, const isp_drv_share_mem_ops_t *mem_ops, AiqCamHwBase_t* camHw);
void AiqMemcManager_deinit(AiqMemcManager_t* pMemcMan);
void AiqMemcManager_prepare(AiqMemcManager_t* pMemcMan, rk_aiq_exposure_sensor_descriptor *snsDes);
void AiqMemcManager_start(AiqMemcManager_t* pMemcMan);
void AiqMemcManager_stop(AiqMemcManager_t* pMemcMan);
void AiqMemcManager_send_ispparam(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* results);
XCamReturn AiqMemcManager_result_callback(AiqMemcManager_t* pMemcMan, memc_callback_result_t *res);
XCamReturn AiqMemcManager_send_sync_buf(void* memc_ctx,
    AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m,
    AiqV4l2Buffer_t* buf_l);
void AiqMemcManager_setProcStreamCtx(AiqMemcManager_t* pMemcMan, AiqRawStreamProcUnit_t* proc_stream);
void AiqMemcManager_pushImuData(AiqMemcManager_t* pMemcMan, AiqImuData_t *data);
void AiqMemcManager_pushSofTime(AiqMemcManager_t* pMemcMan, uint32_t frame_id, uint64_t time);
void AiqMemcManager_setMemcAlgoStatus(AiqMemcManager_t* pMemcMan);
bool AiqMemcManager_getMemcIsSyncBaseImuMode(AiqMemcManager_t* pMemcMan);
bool AiqMemcManager_getMemcEn(AiqMemcManager_t* pMemcMan);
void AiqMemcManager_setIirAddress(AiqMemcManager_t* pMemcMan, void* iir_fd, void* iir_address, uint32_t iir_size);
void AiqMemcManager_pushIirMsg(AiqMemcManager_t* pMemcMan, uint8_t iir_index, uint32_t frame_id);
bool AiqMemcManager_push_scl_buffer(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* buffer);
void AiqMemcManager_setCalib(AiqMemcManager_t* pMemcMan, const CamCalibDbV2Context_t *attr);

#define AiqMemcManager_setAiispCfg(pMemcMan, cfg) \
    pMemcMan->mAiispCfg = cfg



#endif

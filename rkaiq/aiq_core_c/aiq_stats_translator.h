/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef _AIQ_STATS_TRANSLATOR_H_
#define _AIQ_STATS_TRANSLATOR_H_

#include "rk_aiq_types_priv_c.h"
#include "xcore_c/aiq_video_buffer.h"
#include "algos/algo_types_priv.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_pool.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqCamHwBase_s AiqCamHwBase_t;
typedef struct AiqLensHw_s AiqLensHw_t;
typedef struct AiqSensorHw_s AiqSensorHw_t;

typedef struct AiqStatsTranslator_s AiqStatsTranslator_t;

typedef struct Rectangle_s {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} Rectangle_t;

typedef struct aeAlgoStatsCfg_s {
    bool UpdateStats;      /* update stats every frame */
    bool UseSubWinStats;
    int8_t RawStatsChnSel; /* RawStatsChnEn_t */
    int8_t YRangeMode;     /* CalibDb_CamYRangeModeV2_t */
    unsigned char BigWeight[RAWHISTBIG_WIN_NUM];
    unsigned char LiteWeight[RAWHISTLITE_WIN_NUM];
} aeAlgoStatsCfg_t;

struct AiqStatsTranslator_s {
    XCamReturn (*translateAecStats)(AiqStatsTranslator_t* pStatsTrans,
                                    const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateAwbStats)(AiqStatsTranslator_t* pStatsTrans,
                                    const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateAfStats)(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                   aiq_stats_base_t* to);
    XCamReturn (*translateAdehazeStats)(AiqStatsTranslator_t* pStatsTrans,
                                        const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateAgainStats)(AiqStatsTranslator_t* pStatsTrans,
                                      const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
#if RKAIQ_HAVE_PDAF
    XCamReturn (*translatePdafStats)(AiqStatsTranslator_t* pStatsTrans, const rk_aiq_isp_pdaf_meas_t* pdaf_meas,
                                     const aiq_VideoBuffer_t* from, aiq_stats_base_t* to, bool sns_mirror);
#endif
    XCamReturn (*translateBay3dStats)(AiqStatsTranslator_t* pStatsTrans,
                                      const aiq_VideoBuffer_t* from, AiqList_t* to, AiqPool_t* pool,
                                      AiqMutex_t* lock);
#if defined(RKAIQ_HAVE_MULTIISP)
    XCamReturn (*translateMultiAecStats)(AiqStatsTranslator_t* pStatsTrans,
                                         const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateMultiAwbStats)(AiqStatsTranslator_t* pStatsTrans,
                                         const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateMultiAfStats)(AiqStatsTranslator_t* pStatsTrans,
                                        const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
    XCamReturn (*translateMultiAdehazeStats)(AiqStatsTranslator_t* pStatsTrans,
            const aiq_VideoBuffer_t* from, aiq_stats_base_t* to);
#endif

    aiq_isp_effect_params_t* _ispParams;
    aiq_sensor_exp_info_t* _expParams;
    aeAlgoStatsCfg_t _aeAlgoStatsCfg;
    uint16_t _lastHistMean[3];
    RKAiqAecStats_t _lastAeStats;

#if RKAIQ_HAVE_PDAF
    int mPdafDumpCnt;
    bool mEnPdDump;
#endif

    int mCamPhyId;
    bool mIsGroupMode;
    int mModuleRotation;
    int mWorkingMode;
    bool mIsMultiIsp;
    Rectangle_t pic_rect_;
    Rectangle_t left_isp_rect_;
    Rectangle_t right_isp_rect_;
    Rectangle_t bottom_left_isp_rect_;
    Rectangle_t bottom_right_isp_rect_;
    RkAiqIspUniteMode mIspUniteMode;
    AiqCamHwBase_t* mCamHw;
    AiqLensHw_t* mFocusLensHw;
    AiqLensHw_t* mIrishw;
    AiqSensorHw_t* mSensorHw;

    void* _bottom_mge_af_stats;
    void* _bottom_mge_awb_stats;
};

XCamReturn AiqStatsTranslator_init(AiqStatsTranslator_t* pStatsTrans);
void AiqStatsTranslator_deinit(AiqStatsTranslator_t* pStatsTrans);

XCamReturn AiqStatsTranslator_getParams(AiqStatsTranslator_t* pStatsTrans,
                                        const aiq_VideoBuffer_t* from);
void AiqStatsTranslator_releaseParams(AiqStatsTranslator_t* pStatsTrans);

void AiqStatsTranslator_setCamPhyId(AiqStatsTranslator_t* pStatsTrans, int phyId);
void AiqStatsTranslator_setGroupMode(AiqStatsTranslator_t* pStatsTrans, bool bGroup);
void AiqStatsTranslator_setModuleRot(AiqStatsTranslator_t* pStatsTrans, int rot);
void AiqStatsTranslator_setWorkingMode(AiqStatsTranslator_t* pStatsTrans, int working_mode);
int AiqStatsTranslator_getWorkingMode(AiqStatsTranslator_t* pStatsTrans);
void AiqStatsTranslator_setAeAlgoStatsCfg(AiqStatsTranslator_t* pStatsTrans,
        const RkAiqSetStatsCfg* cfg);
bool AiqStatsTranslator_getAeStatsRunFlag(AiqStatsTranslator_t* pStatsTrans, uint16_t* HistMean);

XCamReturn AiqStatsTranslator_translatePdafStats(AiqStatsTranslator_t* pStatsTrans,
        const rk_aiq_isp_pdaf_meas_t* pdaf_meas,
        const aiq_VideoBuffer_t* from,
        aiq_stats_base_t* to, bool sns_mirror);

AiqStatsTranslator_t* AiqStatsTranslator_SetMultiIspMode(AiqStatsTranslator_t* pStatsTrans,
        bool isMultiIsp);
AiqStatsTranslator_t* AiqStatsTranslator_SetIspUniteMode(AiqStatsTranslator_t* pStatsTrans,
        RkAiqIspUniteMode mode);
AiqStatsTranslator_t* AiqStatsTranslator_SetPicInfo(AiqStatsTranslator_t* pStatsTrans,
        Rectangle_t* pic_rect);
AiqStatsTranslator_t* AiqStatsTranslator_SetLeftIspRect(AiqStatsTranslator_t* pStatsTrans,
        Rectangle_t* left_isp_rect);
AiqStatsTranslator_t* AiqStatsTranslator_SetRightIspRect(AiqStatsTranslator_t* pStatsTrans,
        Rectangle_t* right_isp_rect);
AiqStatsTranslator_t* AiqStatsTranslator_SetBottomLeftIspRect(AiqStatsTranslator_t* pStatsTrans,
        Rectangle_t* left_isp_rect);
AiqStatsTranslator_t* AiqStatsTranslator_SetBottomRightIspRect(AiqStatsTranslator_t* pStatsTrans,
        Rectangle_t* right_isp_rect);
bool AiqStatsTranslator_IsMultiIspMode(AiqStatsTranslator_t* pStatsTrans);
RkAiqIspUniteMode AiqStatsTranslator_GetIspUniteMode(AiqStatsTranslator_t* pStatsTrans);
Rectangle_t AiqStatsTranslator_GetPicInfo(AiqStatsTranslator_t* pStatsTrans);
Rectangle_t AiqStatsTranslator_GetLeftIspRect(AiqStatsTranslator_t* pStatsTrans);
Rectangle_t AiqStatsTranslator_GetRightIspRect(AiqStatsTranslator_t* pStatsTrans);
Rectangle_t AiqStatsTranslator_GetBottomLeftIspRect(AiqStatsTranslator_t* pStatsTrans);
Rectangle_t AiqStatsTranslator_GetBottomRightIspRect(AiqStatsTranslator_t* pStatsTrans);

RKAIQ_END_DECLARE

#endif

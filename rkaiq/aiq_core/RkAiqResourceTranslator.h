/*
 * RkAiqConfigTranslator.h
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

#ifndef _RK_AIQ_RESOURCE_TRANSLATOR_H_
#define _RK_AIQ_RESOURCE_TRANSLATOR_H_

#include "IRkAiqResourceTranslator.h"
#include "rk_aiq_types.h"

namespace RkCam {

class RkAiqResourceTranslator
    : public IRkAiqResourceTranslator
{
public:
    RkAiqResourceTranslator ()          = default;
    virtual ~RkAiqResourceTranslator () = default;

    virtual XCamReturn translateIspStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqIspStatsIntProxy>& to,
                                         const SmartPtr<RkAiqAecStatsProxy>& aecStat,
                                         const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                                         const SmartPtr<RkAiqAfStatsProxy>& afStat,
                                         const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
                                         const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat);
    virtual XCamReturn translateAecStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAecStatsProxy>& to);
    virtual XCamReturn translateAwbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAwbStatsProxy>& to);
    virtual XCamReturn translateAfStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAfStatsProxy>& to);
    virtual XCamReturn translateOrbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqOrbStatsProxy>& to);
    virtual XCamReturn translateAtmoStats(const SmartPtr<VideoBuffer>& from,
                                          SmartPtr<RkAiqAtmoStatsProxy>& to);
#if RKAIQ_HAVE_DEHAZE_V10
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
            SmartPtr<RkAiqAdehazeStatsProxy>& to);
#endif
#if RKAIQ_HAVE_PDAF
    bool getFileValue(const char* path, int* pos);
    virtual XCamReturn translatePdafStats(const SmartPtr<VideoBuffer>& from,
                                          SmartPtr<RkAiqPdafStatsProxy>& to, bool sns_mirror);
#endif

    virtual XCamReturn getParams(const SmartPtr<VideoBuffer>& from);
    virtual void releaseParams();
    void setAeAlgoStatsCfg(const RkAiqSetStatsCfg* cfg) {
        _aeAlgoStatsCfg.UpdateStats = cfg->UpdateStats;
        _aeAlgoStatsCfg.RawStatsChnSel = cfg->RawStatsChnSel;
        _aeAlgoStatsCfg.YRangeMode = cfg->YRangeMode;
        memcpy(_aeAlgoStatsCfg.BigWeight, cfg->BigWeight, RAWHISTBIG_WIN_NUM);
        memcpy(_aeAlgoStatsCfg.LiteWeight, cfg->LiteWeight, RAWHISTLITE_WIN_NUM);
    }
    bool getAeStatsRunFlag(uint16_t* HistMean);

protected:
    rkisp_effect_params_v20 _ispParams;
    SmartPtr<RkAiqSensorExpParamsProxy> _expParams;
    typedef struct aeAlgoStatsCfg_s {
        bool UpdateStats;           /* update stats every frame */
        int8_t RawStatsChnSel;      /* RawStatsChnEn_t */
        int8_t YRangeMode;          /* CalibDb_CamYRangeModeV2_t */
        unsigned char BigWeight[RAWHISTBIG_WIN_NUM];
        unsigned char LiteWeight[RAWHISTLITE_WIN_NUM];
    } aeAlgoStatsCfg_t;
    aeAlgoStatsCfg_t _aeAlgoStatsCfg;
    uint16_t _lastHistMean[3] {0, 0, 0};
    RkAiqAecHwStatsRes_t _lastAeStats;

#if RKAIQ_HAVE_PDAF
    int mPdafDumpCnt;
    bool mEnPdDump;
#endif
private:
    XCAM_DEAD_COPY (RkAiqResourceTranslator);
};

}

#endif //_RK_AIQ_CONFIG_TRANSLATOR_H_

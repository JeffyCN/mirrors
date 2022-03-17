
/*
 * RkAiqSharedDataWrapper.cpp
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

#include "RkAiqSharedDataManager.h"

namespace RkCam {

bool RkAiqSharedDataManager::get_exposure_params(int32_t id, RKAiqAecExpInfo_t **exp_param)
{
    SmartPtr<sharedData> data = nullptr;
    if (find(id, RK_AIQ_SHARED_TYPE_SOF_INFO, data)) {
        if (!data.ptr()) {
            LOGE_ANALYZER("can't find id(%d) in mSharedDataMap");
            return false;
        }

        const SmartPtr<RkAiqSofInfoWrapperProxy> sofInfo =
            data.dynamic_cast_ptr<RkAiqSofInfoWrapperProxy>();
        if (sofInfo.ptr())
            *exp_param = &sofInfo->data()->curExp->data()->aecExpInfo;

        return true;
    } else {
        LOGE_ANALYZER("can't find id(%d) in mSharedDataMap", id);
        return false;
    }
}

bool RkAiqSharedDataManager::get_ae_stats(int32_t id, rk_aiq_isp_aec_stats_t *aec_stats)
{
    SmartPtr<sharedData> data = nullptr;
    if (find(id, RK_AIQ_SHARED_TYPE_3A_STATS, data)) {
        const SmartPtr<RkAiqIspStatsIntProxy> stats_3a =
            data.dynamic_cast_ptr<RkAiqIspStatsIntProxy>();

        aec_stats = &stats_3a->data()->aec_stats;
        return true;
    } else {
        aec_stats = NULL;
        return false;
    }
}

bool RkAiqSharedDataManager::get_awb_stats(int32_t id, rk_aiq_awb_stat_res_v200_t *awb_stats)
{
    SmartPtr<sharedData> data = nullptr;
    if (find(id, RK_AIQ_SHARED_TYPE_3A_STATS, data)) {
        const SmartPtr<RkAiqIspStatsIntProxy> stats_3a =
            data.dynamic_cast_ptr<RkAiqIspStatsIntProxy>();

        awb_stats = &stats_3a->data()->awb_stats;
        return true;
    } else {
        awb_stats = NULL;
        return false;
    }

}

bool RkAiqSharedDataManager::get_af_stats(int32_t id, rk_aiq_isp_af_stats_t *af_stats)
{
    SmartPtr<sharedData> data = nullptr;
    if (find(id, RK_AIQ_SHARED_TYPE_3A_STATS, data)) {
        const SmartPtr<RkAiqIspStatsIntProxy> stats_3a =
            data.dynamic_cast_ptr<RkAiqIspStatsIntProxy>();

        af_stats = &stats_3a->data()->af_stats;
        return true;
    } else {
        af_stats = NULL;
        return false;
    }
}

};

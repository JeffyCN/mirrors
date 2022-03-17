
/*
 * RkAiqSharedDataManager.cpp
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

#include "shared_data_api_wrapper.h"

#include "aiq_core/RkAiqSharedDataManager.h"

static RkCam::RkAiqSharedDataManager *handler;

RKAIQ_BEGIN_DECLARE

int shared_data_api_set_instance(void *instance)
{
    handler = (RkCam::RkAiqSharedDataManager* )instance;
    return 0;
}

bool shared_data_api_get_exposure_params(int32_t id, RKAiqAecExpInfo_t **exp_param)
{
    return handler->get_exposure_params(id, exp_param);
}

bool shared_data_api_get_ae_stats(int32_t id, rk_aiq_isp_aec_stats_t *aec_stats)
{
    return handler->get_ae_stats(id, aec_stats);
}

bool shared_data_api_get_awb_stats(int32_t id, rk_aiq_awb_stat_res_v200_t *awb_stats)
{
    return handler->get_awb_stats(id, awb_stats);
}

bool shared_data_api_get_af_stats(int32_t id, rk_aiq_isp_af_stats_t *af_stats)
{
    return handler->get_af_stats(id, af_stats);
}

RKAIQ_END_DECLARE

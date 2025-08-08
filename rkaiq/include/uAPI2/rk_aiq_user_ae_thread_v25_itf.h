/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _RK_AIQ_USER_AE_THREAD_H_
#define _RK_AIQ_USER_AE_THREAD_H_

#include "uAPI2/rk_aiq_user_api2_sysctl.h" /* rk_aiq_sys_ctx_t */
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "uAPI2/rk_aiq_pfnAe.h"

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_ae_algo_config_s {
    bool         isInit;
    bool         updateCalib;
    bool         isHdr;
    bool         isGroupMode;
    bool         isBuildInHdr;

    uint8_t      hdrFrmNum;
    int          workMode;//values look up in rk_aiq_working_mode_t definiton
    int          rawWidth;
    int          rawHeight;

    int          camId; //not for group mode
    int          camIdArray[6];
    int          camIdArrayLen;

    float        last_fps;
    float        init_fps;
    float        last_vts;

    bool         update_fps;
    float        vts_request;
    float        fps_request;

    CalibDb_Sensor_ParaV2_t stSensorInfo;
    aeStats_cfg_t           aeStatsCfg;

    rk_aiq_sensor_nr_switch_t stNRswitch; //get from sensor
    float        LinePeriodsPerField; //get from sensor
    float        PixelClockFreqMHZ; //get from sensor
    float        PixelPeriodsPerLine; //get from sensor

} rk_aiq_ae_algo_config_t;

typedef struct rk_aiq_ae_info_s {
    ae_pfnAe_info_t pfn_info;
} rk_aiq_ae_info_t;

typedef struct rk_aiq_ae_result_s {
    ae_pfnAe_results_t pfn_result;
} rk_aiq_ae_result_t;

/*!
 * \brief register custom Ae algo
 *
 * \param[in] ctx             context
 * \param[in] cbs             custom Ae callbacks
 * \note should be called after rk_aiq_uapi_sysctl_init
 */
XCamReturn
rk_aiq_uapi2_ae_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_pfnAe_t* cbs);

/*!
 * \brief enable/disable custom Ae algo
 *
 * \param[in] ctx             context
 * \param[in] enable          enable/diable custom Ae
 * \note should be called after rk_aiq_uapi_customAE_register. If custom Ae was enabled,
 *       Rk ae will be stopped, vice versa.
 */
XCamReturn
rk_aiq_uapi2_ae_enable(const rk_aiq_sys_ctx_t* ctx, bool enable);

/*!
 * \brief unregister custom Ae algo
 *
 * \param[in] ctx             context
 * \note should be called after rk_aiq_uapi_customAE_register.
 */
XCamReturn
rk_aiq_uapi2_ae_unRegister(const rk_aiq_sys_ctx_t* ctx);

RKAIQ_END_DECLARE

#endif

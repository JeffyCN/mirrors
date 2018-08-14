/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef _RK_PARAMS_TRANSLATE_H
#define _RK_PARAMS_TRANSLATE_H

#include "rkiq_handler.h"

namespace XCam {

class ParamsTranslate {
public:
    ParamsTranslate() {}
    virtual ~ParamsTranslate() {}

    static void convert_from_rkisp_aec_result(rk_aiq_ae_results* aec_result,
                                       AecResult_t* result, struct CamIA10_SensorModeData *sensor_desc);
    static void convert_from_rkisp_awb_result(rk_aiq_awb_results* aiq_awb_result,
                                       CamIA10_AWB_Result_t* result, struct CamIA10_SensorModeData *sensor_desc);
    static void convert_from_rkisp_af_result(rk_aiq_af_results* aiq_af_result,
                                       XCam3aResultFocus* result, struct CamIA10_SensorModeData *sensor_desc);

    static void convert_to_rkisp_aec_config(XCamAeParam* aec_params,
                                            HAL_AecCfg* config, struct CamIA10_SensorModeData *sensor_desc);
    static void convert_to_rkisp_awb_config(XCamAwbParam* awb_params,
                                            HAL_AwbCfg* config, struct CamIA10_SensorModeData *sensor_desc);
    static void convert_to_rkisp_af_config(XCamAfParam* af_params,
                                            HAL_AfcCfg* config, struct CamIA10_SensorModeData *sensor_desc);
};
};
#endif //_RK_PARAMS_TRANSLATE_H

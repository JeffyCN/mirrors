/*
* rk_aiq_awb_algo_v200.h

* for rockchip v2.0.0
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
/* for rockchip v2.0.0*/
#ifndef __RK_AIQ_AWB_ALGO_V200_H__
#define __RK_AIQ_AWB_ALGO_V200_H__

#include "rk_aiq_types_awb_stat_v200.h"
#include "rk_aiq_types_awb_algo_prvt.h"

XCamReturn AwbInitV200(awb_contex_t** para, const CamCalibDbV2Context_t* calib);
XCamReturn AwbPrepareV200(awb_contex_t *para);
XCamReturn AwbReconfigV200(awb_contex_t *para);
XCamReturn AwbPreProcV200(rk_aiq_awb_stat_res_v200_t awb_measure_result, awb_contex_t* para);
XCamReturn AwbProcessingV200(awb_contex_t* para);
XCamReturn AwbReleaseV200(awb_contex_t* para);

#endif

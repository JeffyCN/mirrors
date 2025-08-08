/*
 *rk_aiq_alsc_algo.h
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

#ifndef _RK_AIQ_ALSC_ALGO_H_
#define _RK_AIQ_ALSC_ALGO_H_
#include "alsc/rk_aiq_types_alsc_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

XCamReturn alscGetOtpInfo(RkAiqAlgoCom* params);
XCamReturn convertSensorLscOTP(resolution_t *cur_res, alsc_otp_grad_t *otpGrad,
                               RkAiqBayerPattern_t bayerPattern);
XCamReturn AlscInit(alsc_handle_t *hAlsc, const CamCalibDbV2Context_t* calib2);
XCamReturn AlscRelease(alsc_handle_t hAlsc);
XCamReturn AlscPrepare(alsc_handle_t hAlsc);
XCamReturn AlscConfig(alsc_handle_t hAlsc);
XCamReturn AlscPreProc(alsc_handle_t hAlsc);
XCamReturn AlscProcessing(alsc_handle_t hAlsc);

XCamReturn AlscAutoParamClone(alsc_handle_t hAlsc);
XCamReturn AlscAutoParamFree(alsc_handle_t hAlsc);
XCamReturn UpdateLscCalibPtr(alsc_handle_t hAlsc);

RKAIQ_END_DECLARE

#endif


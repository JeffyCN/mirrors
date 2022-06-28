/*
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

#ifndef __AEC_XML_2JSON_H__
#define __AEC_XML_2JSON_H__

#include "RkAiqCalibDbV2.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

void CalibV2AecFree(CamCalibDbV2Context_t* calibV2);
#ifdef RKAIQ_ENABLE_PARSER_V1
void convertSensorinfoCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CalibDb_Sensor_ParaV2_t* sensor_info);
void convertModuleinfoCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CalibDb_Module_ParaV2_t* module_info);
void convertAecCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CamCalibDbV2Context_t* calibV2);
#endif


#endif


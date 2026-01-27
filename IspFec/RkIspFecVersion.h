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

#ifndef _RK_ISP_FEC_VERSION_H_

#include "rk_ispfec_api.h"

/*!
 * ==================== RK ISP FEC VERSION HISTORY ====================
 *
 * v1.0.0
 * - FEC: supports different resolution of input and output
 * - add IspFec lib
 *
 * v1.0.1
 * - add the function that generating mesh online
 * - support that prepare and process can be called by different threads
 *
 * v2.0.0
 * - add version and size fields for version validation.
 *
 * v3.0.0
 * - Add support for configuring virtual width and horizontal offset.
 *
 * v3.1.0
 * - rename calibration parameter
 * - add internal support for parsing calib ini file
 *
 * v4.0.0
 * - fix INI field parsing error
 * - modify calib_ini_from from a pointer to an array
 *
 * v4.0.2
 * - Add API: rk_ispfec_api_load_online_config_from_ini
 * - rename FEC calibration file for clarity
 *
 * v4.0.3
 * - release buffers on single-instance deinit in multi-instance mode
 *
 * v4.0.4
 * - install FEC calibration ini files to /usr/share/fec_calib
 *
 * v4.0.5
 * - fix mesh sampling step selection based on resolution
 *
 */

#define RK_ISP_FEC_RELEASE_DATE "2025-05-15"
#define RK_ISP_FEC_VERSION      RK_ISP_FEC_API_VERSION

#endif
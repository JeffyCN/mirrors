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

#ifndef _RK_AIQ_CALIB_VERSION_H_
/*!
 * ==================== AIQ CALIB VERSION HISTORY ====================
 *
 * v1.0.0
 *  - initial version
 * v1.0.1
 * v1.1.0
 *  - add xml tag check
 * v1.1.1
 *  - magic code:996625
 *  - awb
 *  - difference tolerance and run interval in difference luma value
 * v1.1.2
 *  - magic code:998276
 *  - add COLOR_AS_GREY tag
 * v1.1.3
 *  - magic code:1003001
 *  - Aec: add sensor-AEC-LinearAECtrl-RawStatsEn, ae stats can use rawae/yuvae
 *  - Aec: add system-dcg_setting divide into 2 parts -normal /Hdr
 *  - Aec: add system-Gainrange-Linear/NonLinear, support db_mode
 * v1.1.4
 *  - magic code:1003993
 *  - Ahdr: add Band prior mode
 * v1.1.5
 *  - magic code: 1005025
 *  - sensor_para: add default sensor flip setting
 * v1.1.6
 *  - magic code:1011737
 *  - Aec: add AecEnvLvCalib
 *  - add ModuleInfo
 * v1.1.7
 *  - magic code:1023357
 *  - Adpcc: Redefine DPCC in IQ files, add Expert Mode
 *  - Adpcc: Add fast mode in DPCC
 * v1.1.8
 *  - magic code:1027323
 *  - Ahdr: remove Band_Prior from moresetting in IQ files
 *  - Ahdr: Add Band_Prior module in IQ files
 * v1.1.9
 *  - simplify params in Backlight, add MeasArea
 *  - add OverExpCtrl in AEC
 *  - delete useless iq params in AEC
 *  - including:AOE,Hist2Hal,AecRange,InternalAdjust
 * v1.1.a
 *  - ExpDelay support Hdr/Normal mode
 *  - Tolerance divide into ToleranceIn/ToleranceOut
 * v1.2.1
 *  - magic code:1007256
 *  - Ahdr:divide Ahdr into two parts,merge and tmo
 *  - Tmo: add linear tmo function
 * v1.2.2
 *  - Afec: add light_center and distortion_coeff
 * v1.2.3
 *  - LumaDetect: add the level2 of mutation threshold
 * v1.2.4
 *  - Aldch: add light_center and distortion_coeff
 * v1.2.5
 *  - dehaze: add normal, HDR and night mode
 * v1.2.6
 *  -sensorinfo: add CISHdrGainIndSetEn for Stagger+same gain
 * v1.2.7
 *  -aldch: add correct_level_max filed
 * v1.2.8
 *  - magic code:1017263
 *  -Tmo: rename BandPror as GlobalTMO
 *  -Tmo:add IIR control in GlobalTMO
 *  -Tmo: rename TmoContrast as LocalTMO
 *  -Tmo: delete moresetting
 * v1.2.9
 *  -LumaDetect: add fixed times of readback
 * v1.2.a
 *  -AF: add vcmconfig
 * v1.3.0
 *  -dynamic mfnr enable in iqfiles
 * v1.3.1
 *  - magic code:1034424
 *  - Redifine dehaze para in IQ files
 * v1.3.2
 *  - magic code:1034813
 *  - TMO: add normal HDR night mode
 * v1.3.3
 *  - magic code:1044335
 *  - AE: add SyncTest to debug
 * v1.3.4
 *  - magic code:1061311
 *  - AE: add IrisCtrl, and also modify some params in manualCtrl & initialExp
 * v1.3.5
 *  - magic code:1054757
 *  - Adehze: remove some para in iq
 * v1.3.6
 *  - magic code:1054435
 *  - Tmo: add normal HDR night mode en
 * v1.3.7
 *  - magic code:1056480
 *  - sensorinfo: rename CISTimeRegSumFac as CISHdrTimeRegSumFac
 *                add CISLinTimeRegMaxFac
 * v1.3.8
 *  - magic code:1058686
 *  - AF: add afmeas_iso
 * v1.3.9
 *  - magic code:1054936
 *  - Gamma: Remove gamma_out_mode curve_user
 * v1.4.0
 *  - magic code:1055312
 *  - merge: change matrix from [1,6] to [1,13]
 *  - tmo: change matrix from [1,6] to [1,13]
 * v1.4.1
 *  - magic code:1088367
 *  - DPCC: redefine set cell struct
  * v1.4.2
 *  - magic code:1089142
 *  - ACCM: support Hdr/Normal mode
 * v1.4.3
 *  - magic code:1123951
 *  - ACPROC: support hue/bright/saturation settings
 *  - AIE: support ie mode setting
 * v1.4.4
 *  - mfnr: add motion detect module enable/disable control
  * v1.4.5
 *  - sharp & edgefilter: add kernel coeffs interpolation
 *v1.4.6
 *  - AF: add SceneDiff/ValidValue/WeightMatrix/zoomfocus_tbl
 *v1.4.7
 *  - mfnr:motion_detection: rename reserved9&reserved8 to
 *  - frame_limit_y&frame_limit_uv.
*v1.4.8
 *  -AWB: add wbGainOffset in AWB
 */
#define RK_AIQ_CALIB_VERSION_REAL_V          "v1.4.8"
#define RK_AIQ_CALIB_VERSION_MAGIC_V         "1170944"


/******* DO NOT EDIT THE FOLLOWINGS ***********/

#define RK_AIQ_CALIB_VERSION_HEAD            "Calib "
#define RK_AIQ_CALIB_VERSION_MAGIC_JOINT     ","
#define RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD "magicCode:"
#define RK_AIQ_CALIB_VERSION \
    RK_AIQ_CALIB_VERSION_HEAD\
    RK_AIQ_CALIB_VERSION_REAL_V\
    RK_AIQ_CALIB_VERSION_MAGIC_JOINT\
    RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD\
    RK_AIQ_CALIB_VERSION_MAGIC_V

#endif

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

#ifndef __ADRC_UAPI_HEAD_H__
#define __ADRC_UAPI_HEAD_H__

#include "RkAiqCalibDbTypesV2.h"

enum { ADRC_NORMAL = 0, ADRC_HDR = 1, ADRC_NIGHT = 2 };

typedef enum drc_OpMode_s {
  DRC_OPMODE_API_OFF = 0, // run IQ ahdr
  DRC_OPMODE_MANU = 1,    // run api manual ahdr
  DRC_OPMODE_AUTO = 2,    // for dark area luma inprove, no matter the scene is
                          // night, normal, or hdr
  DRC_OPMODE_DRC_GAIN = 3,
  DRC_OPMODE_HILIT = 4,
  DRC_OPMODE_LOCAL_TMO = 5,
  DRC_OPMODE_COMPRESS = 6,
} drc_OpMode_t;

typedef struct mDrcGain_t {
  float DrcGain;
  float Alpha;
  float Clip;
} mDrcGain_t;

typedef struct mDrcHiLit_s {
  float Strength;
} mDrcHiLit_t;

typedef struct mDrcLocal_s {
  float LocalWeit;
  float GlobalContrast;
  float LoLitContrast;
} mDrcLocal_t;

typedef struct mDrcCompress_s {
  CompressMode_t Mode;
  uint16_t Manual_curve[ADRC_Y_NUM];
} mDrcCompress_t;

typedef struct mdrcAttr_s {
  mDrcGain_t DrcGain;
  mDrcHiLit_t HiLit;
  mDrcLocal_t Local;
  mDrcCompress_t Compress;
} mdrcAttr_t;

typedef struct DrcInfo_s {
  // M4_NUMBER_DESC("EnvLv", "f32", M4_RANGE(0,1), "0", M4_DIGIT(6))
  float EnvLv;
} DrcInfo_t;

typedef struct drcAttr_s {
  bool Enable;
  drc_OpMode_t opMode;
  mdrcAttr_t stManual;
  // M4_STRUCT_DESC("Info", "normal_ui_style")
  DrcInfo_t Info;
} drcAttr_t;

#endif /*__ADRC_UAPI_HEAD_H__*/

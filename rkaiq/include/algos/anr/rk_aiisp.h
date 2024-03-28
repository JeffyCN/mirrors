/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#ifndef __RK_AIISP_H__
#define __RK_AIISP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rk_aiisp_param_t
{
    // rawInfo
    int rawHgt;
    int rawWid;
    int rawHgtStd;
    int rawWidStd;
    int rawBit;     //  the valid bit, 12

    // bufferInfo
    int bufType;    //  0-iir raw, 1-normal raw
    void* pBuf;     //  raw virtual addr
    int bufFd;      //  raw fd

    // gainmap Info
    int gainHgt;
    int gainWid;
    int gainHgtStd;
    int gainWidStd;

    //  gainmap bufferInfo
    int gainMapFd;
    void* pGainMapBuf;

    // ISO/AWB Info
    float ISO;
    float rGain;
    float bGain;

    // sensor
    // 0-os04a10    1-imx415    2-imx464    3-sc200ai
    int sensorType;
}rk_aiisp_param;


extern int rk_aiisp_init(rk_aiisp_param *param);


extern int rk_aiisp_proc(rk_aiisp_param *param);


extern int rk_aiisp_deinit(rk_aiisp_param *param);

#ifdef __cplusplus
}
#endif

#endif /*__RK_AIISP_H__*/

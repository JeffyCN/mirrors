/*
 *  Copyright (c) 2024 Rockchip Corporation
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
#ifndef _AIQ_DUMP_RK_RAW_H_
#define _AIQ_DUMP_RK_RAW_H_

#include <stdint.h>

#include "c_base/aiq_cond.h"
#include "c_base/aiq_mutex.h"
#include "common/linux/videodev2.h"
#include "include/common/rk_aiq_types.h"

#define CAPTURERAW_SUBM (0x80)

#if defined(ISP_HW_V20)
#define ISP_REGS_BASE 0xffb50000
#define ISP_REGS_SIZE 0x7000
#elif defined(ISP_HW_V21)
#define ISP_REGS_BASE 0xfdcb0000
#define ISP_REGS_SIZE 0x7000
#elif defined(ISP_HW_V30)
#define ISP_REGS_BASE 0xfdcb0000
#define ISP_REGS_SIZE 0x7000
#elif defined(ISP_HW_V39)
#define ISP_REGS_BASE 0xf9010000
#define ISP_REGS_SIZE 0x7000
#elif defined(ISP_HW_V33)
#define ISP_REGS_BASE 0xf9010000
#define ISP_REGS_SIZE 0x7000
#elif defined(ISP_HW_V35)
#define ISP_REGS_BASE 0xf9010000
#define ISP_REGS_SIZE 0x7000
#else
#define ISP_REGS_BASE 0x0
#define ISP_REGS_SIZE 0x0
#endif

enum {
    DUMP_RKRAW_DEFAULT = 0,
    DUMP_RKRAW1,
    DUMP_RKRAW2,
};

struct ispInfo_s {
    char sns_name[32];
    uint32_t sns_width;
    uint32_t sns_height;
    uint32_t pixelformat;
    int8_t bayer_fmt;
    int8_t bpp;
    uint32_t bytesPerLine;
    uint32_t stridePerLine;

    int32_t working_mode;
    int32_t mCamPhyId;
};

typedef struct {
    uint32_t fourcc;
    uint8_t bayer_fmt;
    uint8_t pcpp;
    uint8_t bpp[VIDEO_MAX_PLANES];
} capture_fmt_t;

typedef struct aiq_isp_effect_params_s aiq_isp_effect_params_t;
typedef struct aiq_DumpRkRaw_s aiq_DumpRkRaw_t;
typedef struct AiqSensorExpInfo_s aiq_sensor_exp_info_t;
typedef struct AiqAfInfoWrapper_s AiqAfInfoWrapper_t;

struct aiq_DumpRkRaw_s {
    XCamReturn (*dumpRkRawBlock)(aiq_DumpRkRaw_t* pDpRkRaw, int32_t dev_index, uint32_t sequence,
                                 unsigned long userptr, uint32_t bytesused);
    XCamReturn (*dumpIspRegBlock)(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                  uint32_t offset_addr, int32_t len, uint32_t sequence);
    XCamReturn (*dumpMetadataBlock)(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                    aiq_isp_effect_params_t* ispParams,
                                    aiq_sensor_exp_info_t* expParams, AiqAfInfoWrapper_t* afParams);
    XCamReturn (*dumpPlatformInfoBlock)(aiq_DumpRkRaw_t* pDpRkRaw);

    XCamReturn (*dumpIspRegHeaderBlock)(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                        uint32_t offset_addr, int32_t length, uint32_t sequence);
    XCamReturn (*dumpBlockHeader)(aiq_DumpRkRaw_t* pDpRkRaw, uint16_t blockID, uint32_t blockSize);

    /*
     * synchronize with external dump,
     * for example sync with dump yuv
     */
    AiqMutex_t dumpExternMutex;
    AiqCond_t dumpExternCond;

    /*
     * sync with frameEnd
     */
    AiqMutex_t dumpInnerMutex;
    AiqCond_t dumpInnerCond;

    struct ispInfo_s ispInfo;
    char dumpDirPath[64];
    char userSetDumpDir[64];
    char raw_name[128];
    FILE* fp;
    bool isDumpDirExist;
    int32_t dumpNum;
    uint32_t dumpTotalSize;
    capture_raw_t dumpType;
    int8_t dumpRkRawType;
    int32_t _camId;
};

void aiq_dumpRkRaw_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId);
void aiq_dumpRkRaw_deinit(aiq_DumpRkRaw_t* pDpRkRaw);

void aiq_dumpRkRaw_setIspInfo(aiq_DumpRkRaw_t* pDpRkRaw, struct ispInfo_s* info);
XCamReturn aiq_dumpRkRaw_isDumpInFrameEnd(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                          aiq_isp_effect_params_t* ispParams,
                                          aiq_sensor_exp_info_t* expParams);
XCamReturn aiq_dumpRkRaw_detectDumpStatus(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence);
XCamReturn aiq_dumpRkRaw_updateDumpStatus(aiq_DumpRkRaw_t* pDpRkRaw);

/*
 * sync with frameEnd
 */
XCamReturn aiq_dumpRkRaw_waitDumpRawDone(aiq_DumpRkRaw_t* pDpRkRaw);
XCamReturn aiq_dumpRkRaw_notifyDumpRaw(aiq_DumpRkRaw_t* pDpRkRaw);

/*
 * synchronize with external dump,
 * for example sync with dump yuv
 */
XCamReturn aiq_dumpRkRaw_waitThirdPartyDumpDone(aiq_DumpRkRaw_t* pDpRkRaw, bool isFirstTrigger);
XCamReturn aiq_dumpRkRaw_notifyDumpRawFromThirdParty(aiq_DumpRkRaw_t* pDpRkRaw);
XCamReturn aiq_dumpRkRaw_dumpControl(aiq_DumpRkRaw_t* pDpRkRaw, capture_raw_t type, int32_t count,
                                     const char* capture_dir, char* output_dir);

void aiq_dumpRkRaw1_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId);
void aiq_dumpRkRaw1_deinit(aiq_DumpRkRaw_t* pDpRkRaw);

struct raw2Header_s {
    uint16_t version;
    char snsName[32];
    char scenario[32];

    uint32_t frameId;
    uint16_t sns_width;
    uint16_t sns_height;
    int8_t bpp;
    int8_t bayer_fmt;
    int8_t working_mode;
    int8_t storage_type;
    uint16_t stridePerLine;
    uint16_t bytesPerLine;
    int8_t endian;
} __attribute__((packed));

struct raw2expRealParam_s {
    float integration_time;
    float analog_gain;
} __attribute__((packed));

struct raw2expSensorParam_s {
    uint32_t integration_time;
    uint32_t analog_gain;
} __attribute__((packed));

struct raw2expParam_s {
    struct raw2expRealParam_s exp_real;      // real value
    struct raw2expSensorParam_s exp_sensor;  // reg value
} __attribute__((packed));

struct raw2metadata_s {
    uint16_t version;
    uint32_t frameId;
    struct raw2expParam_s linearExp;
    struct raw2expParam_s HdrExp[3];

    float rgain;
    float bgain;
} __attribute__((packed));

struct raw2ispRegHeader_s {
    uint16_t version;

    uint32_t frameId;
    uint32_t base_addr;
    uint32_t offset_addr;
    uint32_t count;
} __attribute__((packed));

struct raw2isppRegHeader_s {
    uint16_t version;

    uint32_t frameId;
    uint32_t base_addr;
    uint32_t offset_addr;
    uint32_t count;
} __attribute__((packed));

struct raw2platform_info_s {
    char chipModel[32];
    char ispVersion[32];
    char aiqVersion[32];
} __attribute__((packed));

void aiq_dumpRkRaw2_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId);
void aiq_dumpRkRaw2_deinit(aiq_DumpRkRaw_t* pDpRkRaw);

#endif  // _AIQ_DUMP_RK_RAW_H_

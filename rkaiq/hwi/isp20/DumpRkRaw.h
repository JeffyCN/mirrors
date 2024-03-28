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
#ifndef _DUMP_RK_RAW_H_
#define _DUMP_RK_RAW_H_

#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

using namespace XCam;

namespace RkCam {

#define CAPTURERAW_SUBM (0x80)

#if defined(ISP_HW_V20)
    #define ISP_REGS_BASE               0xffb50000
    #define ISP_REGS_SIZE               0x7000
#elif defined(ISP_HW_V21)
    #define ISP_REGS_BASE               0xfdcb0000
    #define ISP_REGS_SIZE               0x7000
#elif defined(ISP_HW_V30)
    #define ISP_REGS_BASE               0xfdcb0000
    #define ISP_REGS_SIZE               0x7000
#elif defined(ISP_HW_V39)
    #define ISP_REGS_BASE               0xf9010000
    #define ISP_REGS_SIZE               0x7000
#else
    #define ISP_REGS_BASE               0x0
    #define ISP_REGS_SIZE               0x0
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
    sint8_t bayer_fmt;
    sint8_t bpp;
    uint32_t bytesPerLine;
    uint32_t stridePerLine;

    sint32_t working_mode;
    sint32_t mCamPhyId;
};

typedef struct {
    u32 fourcc;
    u8 bayer_fmt;
    u8 pcpp;
    u8 bpp[VIDEO_MAX_PLANES];
} capture_fmt_t;

class DumpRkRaw
{
public:
    DumpRkRaw()
        : fp(nullptr)
          , isDumpDirExist(false)
          , dumpNum(0)
          , dumpTotalSize(0)
          , dumpType(CAPTURE_RAW_ASYNC)
          , dumpRkRawType(DUMP_RKRAW_DEFAULT)
          , _camId(-1) {};

    explicit DumpRkRaw(int32_t camId)
        : fp(nullptr)
          , isDumpDirExist(false)
          , dumpNum(0)
          , dumpTotalSize(0)
          , dumpType(CAPTURE_RAW_ASYNC)
          , dumpRkRawType(DUMP_RKRAW_DEFAULT)
          , _camId(camId) {};

    virtual ~DumpRkRaw() {};

    virtual XCamReturn dumpRkRawBlock(sint32_t dev_index, uint32_t sequence,
                                    unsigned long userptr, uint32_t bytesused) = 0;
    virtual XCamReturn dumpIspRegBlock(uint32_t base_addr, uint32_t offset_addr,
                                       sint32_t len, uint32_t sequence) = 0;
    virtual XCamReturn dumpMetadataBlock(uint32_t sequence,
                            rkisp_effect_params_v20& ispParams,
                            SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
                            SmartPtr<RkAiqAfInfoProxy>& afParams) = 0;
    virtual XCamReturn dumpPlatformInfoBlock() {
		return  XCAM_RETURN_ERROR_FAILED;
    };

    virtual void setIspInfo(struct ispInfo_s &info);
    virtual XCamReturn isDumpInFrameEnd(uint32_t sequence, rkisp_effect_params_v20& ispParams,
                                        SmartPtr<RkAiqSensorExpParamsProxy>& expParams);
    virtual XCamReturn detectDumpStatus(uint32_t sequence);
    virtual XCamReturn updateDumpStatus();

    /*
     * sync with frameEnd
     */
    virtual XCamReturn waitDumpRawDone();
    virtual XCamReturn notifyDumpRaw();

    /*
     * synchronize with external dump,
     * for example sync with dump yuv
     */
    virtual XCamReturn waitThirdPartyDumpDone(bool isFirstTrigger);
    virtual XCamReturn notifyDumpRawFromThirdParty();
    virtual XCamReturn dumpControl(capture_raw_t type, sint32_t count,
                           const sint8_t* capture_dir,
                           sint8_t* output_dir);

protected:
    virtual XCamReturn dumpIspRegHeaderBlock(uint32_t base_addr, uint32_t offset_addr,
                                  sint32_t length, uint32_t sequence) {
		return  XCAM_RETURN_ERROR_FAILED;
	};

    const capture_fmt_t* findFmt(const uint32_t pixelformat);
    void calculate_stride_per_line(const  capture_fmt_t& fmt,
                                  uint32_t& bytesPerLine);
    bool getValueFromFile(const sint8_t *path, sint32_t &value, uint32_t &frameId);
    bool saveValueToFile(const sint8_t *path, sint32_t value, uint32_t sequence=0);
    XCamReturn creatDumpDir(const sint8_t* path);
    XCamReturn creatDumpFile(uint32_t sequence);
	static void memread_memory(unsigned long phys_addr, uint8_t *addr, int len, int iosize);
    virtual XCamReturn dumpBlockHeader(uint16_t blockID, uint32_t blockSize) {
		return  XCAM_RETURN_ERROR_FAILED;
    }

    /*
     * synchronize with external dump,
     * for example sync with dump yuv
     */
    Mutex dumpExternMutex;
    Cond dumpExternCond;

    /*
     * sync with frameEnd
     */
    Mutex dumpInnerMutex;
    Cond dumpInnerCond;

    struct ispInfo_s ispInfo;
    sint8_t dumpDirPath[64];
    sint8_t userSetDumpDir[64];
    sint8_t raw_name[128];
    static const capture_fmt_t csirx_fmts[];
    FILE *fp;
    bool isDumpDirExist;
    sint32_t dumpNum;
    uint32_t dumpTotalSize;
    capture_raw_t dumpType;
    int8_t dumpRkRawType;
    int32_t _camId;

private:
    XCAM_DEAD_COPY (DumpRkRaw);
};

class DumpRkRaw1
    : public DumpRkRaw
{
public:
    DumpRkRaw1()
        : DumpRkRaw() {
            dumpRkRawType = DUMP_RKRAW1;
        };
   explicit DumpRkRaw1(int32_t camId)
        : DumpRkRaw(camId) {
            dumpRkRawType = DUMP_RKRAW1;
        };
    ~DumpRkRaw1() = default;

    XCamReturn dumpRkRawBlock(sint32_t dev_index, uint32_t sequence,
                            unsigned long userptr, uint32_t bytesused);
    XCamReturn dumpIspRegBlock(uint32_t base_addr, uint32_t offset_addr,
                          sint32_t len, uint32_t sequence);
    XCamReturn dumpMetadataBlock(uint32_t sequence,
                            rkisp_effect_params_v20& ispParams,
                            SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
                            SmartPtr<RkAiqAfInfoProxy>& afParams);
private:
    XCAM_DEAD_COPY (DumpRkRaw1);
    XCamReturn dumpRkRawHeaderBlock(FILE *fpp, uint32_t sequence, int32_t dev_index);
    FILE *creatHdrDumpFile(sint32_t dev_index, uint32_t sequence);
};

class DumpRkRaw2
    : public DumpRkRaw
{
public:
    DumpRkRaw2()
        : DumpRkRaw() {
            dumpRkRawType = DUMP_RKRAW2;
        };
    explicit DumpRkRaw2(int32_t camId)
        : DumpRkRaw(camId) {
            dumpRkRawType = DUMP_RKRAW2;
        };
    ~DumpRkRaw2() = default;

    XCamReturn dumpBlockHeader(uint16_t blockID, uint32_t blockSize);
    XCamReturn dumpRkRawBlock(sint32_t dev_index, uint32_t sequence,
                            unsigned long userptr, uint32_t bytesused);
    XCamReturn dumpIspRegBlock(uint32_t base_addr, uint32_t offset_addr,
                          sint32_t len, uint32_t sequence);
    XCamReturn dumpMetadataBlock(uint32_t sequence,
                            rkisp_effect_params_v20& ispParams,
                            SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
                            SmartPtr<RkAiqAfInfoProxy>& afParams);
    XCamReturn dumpPlatformInfoBlock();

private:
    XCamReturn dumpRkRawHeaderBlock(uint32_t sequence, int32_t dev_index);
    XCamReturn dumpIspRegHeaderBlock(uint32_t base_addr, uint32_t offset_addr,
                                  sint32_t length, uint32_t sequence);

    XCamReturn dumpRkRawHeader(sint32_t sequence);
    XCamReturn dumpRkRaw(sint32_t dev_index, sint32_t sequence, void* userptr, sint32_t size);
    XCamReturn dumpMetadata(uint32_t sequence, rkisp_effect_params_v20& ispParams,
                            SmartPtr<RkAiqSensorExpParamsProxy>& expParams);
    XCamReturn dumpIspRegHeader(uint32_t base_addr, uint32_t offset_addr, sint32_t length, uint32_t sequence);
    XCamReturn dumpIspReg(uint32_t base_addr, uint32_t offset_addr, sint32_t length, uint32_t sequence);
    XCamReturn dumpPlatformInfo();

private:
    XCAM_DEAD_COPY (DumpRkRaw2);

    struct rawHeader_s {
        uint16_t version;
        sint8_t snsName[32];
        sint8_t scenario[32];

        uint32_t frameId;
        uint16_t sns_width;
        uint16_t sns_height;
        sint8_t bpp;
        sint8_t bayer_fmt;
        sint8_t working_mode;
        sint8_t storage_type;
        uint16_t stridePerLine;
        uint16_t bytesPerLine;
        sint8_t endian;
    } __attribute__ ((packed));

    struct expRealParam_s {
        float integration_time;
        float analog_gain;
    } __attribute__ ((packed));

    struct expSensorParam_s {
        uint32_t integration_time;
        uint32_t  analog_gain;
    } __attribute__ ((packed));

    struct expParam_s {
        struct expRealParam_s exp_real; //real value
        struct expSensorParam_s exp_sensor; //reg value
    } __attribute__ ((packed));

    struct metadata_s {
        uint16_t version;
        uint32_t frameId;
        struct expParam_s linearExp;
        struct expParam_s HdrExp[3];

        float rgain;
        float bgain;
    } __attribute__ ((packed));

    struct ispRegHeader_s {
        uint16_t version;

        uint32_t frameId;
        uint32_t base_addr;
        uint32_t offset_addr;
        uint32_t count;
    } __attribute__ ((packed));

    struct isppRegHeader_s {
        uint16_t version;

        uint32_t frameId;
        uint32_t base_addr;
        uint32_t offset_addr;
        uint32_t count;
    } __attribute__ ((packed));

    struct platform_info_s {
        sint8_t chipModel[32];
        sint8_t ispVersion[32];
        sint8_t aiqVersion[32];
    } __attribute__ ((packed));
};

};

#endif

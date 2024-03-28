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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "rk_aiq_comm.h"
#include "CaptureRawData.h"
#include "../RkAiqVersion.h"
#include "rkisp2-config.h"
#include "DumpRkRaw.h"

#ifdef ANDROID_OS
#define DEFAULT_CAPTURE_RAW_PATH "/data/capture_image"
#define CAPTURE_CNT_FILENAME "/data/.capture_cnt"
#else
#define DEFAULT_CAPTURE_RAW_PATH "/tmp/capture_image"
#define CAPTURE_CNT_FILENAME "/tmp/.capture_cnt"
#endif

#define WRITE_RAW_FILE_HEADER

#define RAW_FILE_IDENT 0x8080
#define HEADER_LEN 128U

#define RAW_HEADER_VERSION          0x0100
#define RAW_METADATA_VERSION        0x0100
#define ISP_REG_VERSION             0x0100
#define ISPP_REG_VERSION            0x0100

#define START_ID                    0xFF00
#define END_ID                      0x00FF
#define RAW_HEADER_BLOCK_ID         0xFF01
#define NORMAL_RAW_BLOCK_ID         0xFF02
#define HDR_S_RAW_BLOCK_ID          0xFF03
#define HDR_M_RAW_BLOCK_ID          0xFF04
#define HDR_L_RAW_BLOCK_ID          0xFF05
#define METADATA_BLOCK_ID           0xFF06
#define ISP_HEADER_BLOCK_ID         0xFF07
#define ISP_BLOCK_ID                0xFF08
#define ISPP_HEADER_BLOCK_ID        0xFF09
#define ISPP_BLOCK_ID               0xFF0a
#define PLATFORM_INFO_BLOCK_ID      0xFF0b

// #define PRINT_ISP_REG


/*
 * RkRaw1 file structure:
 *
+------------+-----------------+-------------+-----------------+---------------------------+
|    ITEM    |    PARAMETER    |  DATA TYPE  |  LENGTH(Bytes)  |        DESCRIPTION        |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |     Identifier  |  uint16_t   |       2         |  fixed 0x8080             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |  Header length  |  uint16_t   |       2         |  fixed 128U               |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Frame index  |  uint32_t   |       4         |                           |
|            +-----------------+-------------+-----------------+---------------------------+
|            |          Width  |  uint16_t   |       2         |  image width              |
|            +-----------------+-------------+-----------------+---------------------------+
|            |         Height  |  uint16_t   |       2         |  image height             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |      Bit depth  |   uint8_t   |       1         |  image bit depth          |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  0: BGGR;  1: GBRG;       |
|            |   Bayer format  |   uint8_t   |       1         |  2: GRBG;  3: RGGB;       |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: linear                |
|    FRAME   |  Number of HDR  |             |                 |  2: long + short          |
|   HEADER   |      frame      |   uint8_t   |       1         |  3: long + mid + short    |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: short                 |
|            |  Current frame  |             |                 |  2: mid                   |
|            |       type      |   uint8_t   |       1         |  3: long                  |
|            +-----------------+-------------+-----------------+---------------------------+
|            |   Storage type  |   uint8_t   |       1         |  0: packed; 1: unpacked   |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |     Effective   |             |                 |                           |
|            |    line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |       Reserved  |   uint8_t   |      107        |                           |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |                           |
|  RAW DATA  |       RAW DATA  |    RAW      |  W * H * bpp    |  RAW DATA                 |
|            |                 |             |                 |                           |
+------------+-----------------+-------------+-----------------+---------------------------+

 */

/*
 * the structure of measuure parameters from isp in meta_data file:
 *
 * "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]"
 *
 */

namespace RkCam {

const capture_fmt_t DumpRkRaw::csirx_fmts[] =
{
    /* raw */
    {
        .fourcc = V4L2_PIX_FMT_SRGGB8,
        .bayer_fmt = 3,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG8,
        .bayer_fmt = 2,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG8,
        .bayer_fmt = 1,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR8,
        .bayer_fmt = 0,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SRGGB10,
        .bayer_fmt = 3,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG10,
        .bayer_fmt = 2,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG10,
        .bayer_fmt = 1,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR10,
        .bayer_fmt = 0,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SRGGB12,
        .bayer_fmt = 3,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG12,
        .bayer_fmt = 2,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG12,
        .bayer_fmt = 1,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR12,
        .bayer_fmt = 0,
        .pcpp = 2,
        .bpp = { 12 },
    },
};

const capture_fmt_t*
DumpRkRaw::findFmt(const uint32_t pixelformat)
{
    const capture_fmt_t *fmt;

    for (uint32_t i = 0; i < sizeof(csirx_fmts); i++) {
        fmt = &csirx_fmts[i];
        if (fmt->fourcc == pixelformat)
            return fmt;
    }

    return NULL;
}

void
DumpRkRaw::calculate_stride_per_line(const capture_fmt_t& fmt,
        uint32_t& bytesPerLine)
{
    bytesPerLine = ispInfo.sns_width * fmt.bpp[0] / 8;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "sns_width: %d, bytesPerLine: %d\n",
                    ispInfo.sns_width, bytesPerLine);
}

void
DumpRkRaw::setIspInfo(struct ispInfo_s &info)
{
    strcpy(ispInfo.sns_name, info.sns_name);
    ispInfo.working_mode = info.working_mode;

    ispInfo.pixelformat = info.pixelformat;
    ispInfo.sns_width = info.sns_width;
    ispInfo.sns_height = info.sns_height;
    ispInfo.bpp = info.bpp;
    ispInfo.stridePerLine = info.stridePerLine;

    const capture_fmt_t* fmt = nullptr;
    if ((fmt = findFmt(ispInfo.pixelformat)))
        calculate_stride_per_line(*fmt, ispInfo.bytesPerLine);
    ispInfo.bpp = fmt->bpp[0];
    ispInfo.bayer_fmt = fmt->bayer_fmt;

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "sns_name(%s), mode(%d), rect(%dx%d), bpp(%d), bytesPerLine(%d), stridePerLine(%d)\n",
                    ispInfo.sns_name, ispInfo.working_mode,
                    ispInfo.sns_width, ispInfo.sns_height,
                    ispInfo.bpp, ispInfo.bytesPerLine,
                    ispInfo.stridePerLine);
}
bool
DumpRkRaw::getValueFromFile(const sint8_t *path, sint32_t &value, uint32_t &frameId)
{
    const sint8_t *delim = " ";
    sint8_t buffer[16] = {0};
    sint32_t fd;

    fd = open(path, O_RDONLY | O_SYNC);
    if (fd != -1) {
        if (read(fd, buffer, sizeof(buffer)) <= 0) {
            LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "%s read %s failed!\n", __func__, path);
        } else {
            char *p = nullptr;

            p = strtok(buffer, delim);
            if (p != nullptr) {
                value = atoi(p);
                p = strtok(nullptr, delim);
                if (p != nullptr)
                    frameId = atoi(p);
            }
        }
        close(fd);
        return true;
    }

    return false;
}

bool
DumpRkRaw::saveValueToFile(const sint8_t *path, sint32_t value, uint32_t sequence)
{
    sint8_t buffer[16] = {0};
    sint32_t fd;

    fd = open(path, O_CREAT | O_RDWR | O_SYNC, S_IRWXU|S_IRUSR|S_IXUSR|S_IROTH|S_IXOTH);
    if (fd != -1) {
        ftruncate(fd, 0);
        lseek(fd, 0, SEEK_SET);
        snprintf(buffer, sizeof(buffer), "%3d %8d\n", dumpNum, sequence);
        if (write(fd, buffer, sizeof(buffer)) <= 0) {
            LOGW_CAMHW_SUBM(CAPTURERAW_SUBM, "%s write %s failed!\n", __func__, path);
        }
        close(fd);
        return true;
    }

    return false;
}

void
DumpRkRaw::memread_memory(unsigned long phys_addr, uint8_t *addr, int len, int iosize)
{
    int i;

    while (len) {
        printf("%08lx: ", phys_addr);
        i = 0;
        while (i < 16 && len) {
            switch(iosize) {
                case 1:
                    printf(" %02x", *(uint8_t *)addr);
                    break;
                case 2:
                    printf(" %04x", *(uint16_t *)addr);
                    break;
                case 4:
                    printf(" %08x", *(uint32_t *)addr);
                    break;
            }
            i += iosize;
            addr += iosize;
            len -= iosize;
        }
        phys_addr += 16;
        printf("\n");
    }
}

XCamReturn
DumpRkRaw::isDumpInFrameEnd(uint32_t sequence,
                            rkisp_effect_params_v20& ispParams,
                            SmartPtr<RkAiqSensorExpParamsProxy>& expParams)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sint32_t num = 0;
    uint32_t rawFrmId = 0;

    getValueFromFile(CAPTURE_CNT_FILENAME, num, rawFrmId);
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "rawFrmId: %d, frameEndId: %d, dumpNum: %d\n",
            rawFrmId, sequence,
            dumpNum);

    if (sequence >= rawFrmId)
        return XCAM_RETURN_NO_ERROR;

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn
DumpRkRaw::detectDumpStatus(uint32_t sequence)
{
    if (dumpNum <= 0) {
        uint32_t rawFrmId = 0;

        bool ret = getValueFromFile(CAPTURE_CNT_FILENAME, dumpNum, rawFrmId);
        if (!ret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
            getValueFromFile(file_name,  dumpNum, rawFrmId);
        }

        if (dumpNum > 0) {
            bool ret = saveValueToFile(CAPTURE_CNT_FILENAME, dumpNum, sequence);
            if (!ret) {
                // test multi cam mode
                char file_name[64] = {0};
                snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
                saveValueToFile(file_name, dumpNum, sequence);
            }

            if (!isDumpDirExist) {
                if (dumpType == CAPTURE_RAW_SYNC)
                    creatDumpDir(userSetDumpDir);
                else
                    creatDumpDir(DEFAULT_CAPTURE_RAW_PATH);
            }
        }
    }

    if (dumpNum > 0)
        creatDumpFile(sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::updateDumpStatus()
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (!--dumpNum) {
        isDumpDirExist = false;
        if (dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
            dumpExternMutex.lock();
            dumpExternCond.broadcast();
            dumpExternMutex.unlock();
        }

        bool ret = saveValueToFile(CAPTURE_CNT_FILENAME, dumpNum);
        if (!ret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
            saveValueToFile(file_name, dumpNum);
        }

        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "stop capturing raw!\n");
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::creatDumpDir(const sint8_t* path)
{
    time_t now;
    struct tm* timenow;

    if (!path)
        return XCAM_RETURN_ERROR_FAILED;

    time(&now);
    timenow = localtime(&now);

    if (access(path, W_OK) == -1) {
        if (mkdir(path, 0755) < 0) {
            LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!\n",
                            path, strerror(errno));
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    snprintf(dumpDirPath, sizeof(dumpDirPath), "%s/Cam%d-rkraw%d_%04d-%02d-%02d_%02d-%02d-%02d",
             path,
             _camId,
             dumpRkRawType,
             timenow->tm_year + 1900,
             timenow->tm_mon + 1,
             timenow->tm_mday,
             timenow->tm_hour,
             timenow->tm_min,
             timenow->tm_sec);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s for capturing %d frames raw!\n",
                    dumpDirPath, dumpNum);

    if(mkdir(dumpDirPath, 0755) < 0) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!!!\n",
                        dumpDirPath, strerror(errno));
        return XCAM_RETURN_ERROR_PARAM;
    }

    isDumpDirExist = true;

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn
DumpRkRaw::creatDumpFile(uint32_t sequence)
{
    if (fp != nullptr) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM,
                "fp don't be clesed, maybe the frameEnd delayed!\n");
        fclose(fp);
        fp = nullptr;
    }

    // char raw_name[128] = {0};
    memset(raw_name, 0, sizeof(raw_name));
    snprintf(raw_name, sizeof(raw_name),
            "%s/%s_frame%d_%dx%d.rkraw",
            dumpDirPath,
            ispInfo.sns_name,
            sequence,
            ispInfo.sns_width,
            ispInfo.sns_height);

    fp = fopen(raw_name, "ab+");
    if (fp == nullptr) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen %s failed!\n", raw_name);
        return XCAM_RETURN_ERROR_FILE;
    } else {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen (%s) sucessfully, fp(%p)\n",
                raw_name, fp);
    }

    // the start identifier of RK-RAW file
    dumpBlockHeader(START_ID, 0);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::waitDumpRawDone()
{
    if (dumpNum > 0) {
        dumpInnerMutex.lock();
        if (dumpInnerCond.timedwait(dumpInnerMutex, 500 * 1000) != 0)
            LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "wait 500ms timeout!!!\n");
        dumpInnerMutex.unlock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::notifyDumpRaw()
{
    if (fp != nullptr) {
        dumpBlockHeader(END_ID, 0);
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                "fclose fp(%p), dumpTotalSize(%d)!\n",
                fp, dumpTotalSize);

        fclose(fp);
        fp = nullptr;
    }

    dumpTotalSize = 0;
    for (sint32_t i = 0; i <= dumpNum; i++)
        printf(">");
    printf("\n");

    SmartLock locker(dumpInnerMutex);
    dumpInnerCond.broadcast();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::waitThirdPartyDumpDone(bool isFirstTrigger)
{
    if (dumpNum > 0 && !isFirstTrigger) {
        if (dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
            dumpExternMutex.lock();
            dumpExternCond.timedwait(dumpExternMutex, 3000000);
            dumpExternMutex.unlock();
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::notifyDumpRawFromThirdParty()
{
    SmartLock locker(dumpExternMutex);
    dumpExternCond.broadcast();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw::dumpControl(capture_raw_t type, sint32_t count,
                           const sint8_t* capture_dir,
                           sint8_t* output_dir)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    dumpType = type;
    if (dumpType == CAPTURE_RAW_SYNC) {
        if (capture_dir != nullptr)
            snprintf(userSetDumpDir, sizeof(userSetDumpDir),
                     "%s/capture_image", capture_dir);
        else
            strcpy(userSetDumpDir, DEFAULT_CAPTURE_RAW_PATH);

        bool bret = saveValueToFile(CAPTURE_CNT_FILENAME, count);
        if (!bret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
            saveValueToFile(file_name, count);
        }

        dumpExternMutex.lock();
        if (dumpExternCond.timedwait(dumpExternMutex, 30000000) != 0)
            ret = XCAM_RETURN_ERROR_TIMEOUT;
        else
            strncpy(output_dir, dumpDirPath, strlen(output_dir));
        dumpExternMutex.unlock();
    } else if (dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
        LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "capture raw and yuv images simultaneously!");
    }

    return ret;
}

/****************************** dump RKRAW1 *******************************/

FILE *
DumpRkRaw1::creatHdrDumpFile(sint32_t dev_index, uint32_t sequence)
{
    FILE *fpp = nullptr;

    int32_t working_mode = ispInfo.working_mode;
    // char raw_name[128] = {0};
    memset(raw_name, 0, sizeof(raw_name));
    if (working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
        working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        snprintf(raw_name, sizeof(raw_name),
                "%s/%s_frame%d_%dx%d_%s",
                dumpDirPath,
                ispInfo.sns_name,
                sequence,
                ispInfo.sns_width,
                ispInfo.sns_height,
                dev_index == 1 ? "middle" : "long");
    else if (working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
             working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        snprintf(raw_name, sizeof(raw_name),
                "%s/%s_frame%d_%dx%d_%s",
                dumpDirPath,
                ispInfo.sns_name,
                sequence,
                ispInfo.sns_width,
                ispInfo.sns_height,
                "long");

    fpp = fopen(raw_name, "ab+");
    if (fpp == nullptr) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Failed to fopen %s: %s\n",
                        raw_name, strerror(errno));
        return nullptr;
    } else {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen (%s) sucessfully, fp(%p)\n",
                raw_name, fpp);
    }

    return fpp;
}

XCamReturn
DumpRkRaw1::dumpRkRawHeaderBlock(FILE *fpp, uint32_t sequence, int32_t dev_index)
{
    if (fpp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fpp(%p), dumpNum(%d)!\n", fpp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    char buffer[128] = {0};
    uint8_t mode = 0;
    uint8_t frame_type = 0, storage_type = 0;

    if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
            ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        mode = 3;
        frame_type = dev_index == 0 ? 1 : dev_index == 1 ? 2 : 3;
    } else if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
               ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        mode = 2;
        frame_type = dev_index == 0 ? 1 : 3;
    } else {
        mode = 1;
    }

    *((uint16_t* )buffer)           = RAW_FILE_IDENT;           // Identifier
    *((uint16_t* )(buffer + 2))     = HEADER_LEN;               // Header length
    *((uint32_t* )(buffer + 4))     = sequence;                 // Frame number
    *((uint16_t* )(buffer + 8))     = ispInfo.sns_width;        // Image width
    *((uint16_t* )(buffer + 10))    = ispInfo.sns_height;       // Image height
    *(buffer + 12)                  = ispInfo.bpp;              // Bit depth
    *(buffer + 13)                  = ispInfo.bayer_fmt;        // Bayer format
    *(buffer + 14)                  = mode;                     // Number of HDR frame
    *(buffer + 15)                  = frame_type;               // Current frame type
    *(buffer + 16)                  = storage_type;             // Storage type
    *((uint16_t* )(buffer + 17))    = ispInfo.stridePerLine;    // Line stride
    *((uint16_t* )(buffer + 19))    = ispInfo.bytesPerLine;     // Effective line stride

    fwrite(buffer, sizeof(buffer), 1, fpp);
    fflush(fpp);

    dumpTotalSize += strlen(buffer);
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "frame%d: image rect: %dx%d, %d bit depth, Bayer fmt: %d, "
                    "hdr frame number: %d, frame type: %d, Storage type: %d, "
                    "line stride: %d, Effective line stride: %d\n",
                    sequence, ispInfo.sns_width, ispInfo.sns_height,
                    ispInfo.bpp, ispInfo.bayer_fmt, mode,
                    frame_type, storage_type, ispInfo.stridePerLine,
                    ispInfo.bytesPerLine);

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
DumpRkRaw1::dumpRkRawBlock(sint32_t dev_index, uint32_t sequence,
        unsigned long userptr, uint32_t bytesused)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE *fpp = fp;
    if (dev_index != 0)
        fpp = creatHdrDumpFile(dev_index, sequence);

    if (!fpp)
        return XCAM_RETURN_ERROR_FILE;

    dumpRkRawHeaderBlock(fpp, sequence, dev_index);

    fwrite((void *)userptr, bytesused, 1, fpp);
    fflush(fpp);
    if (dev_index != 0)
        fclose(fpp);

    dumpTotalSize += bytesused;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "frame%d: dump raw data: dev_index(%d), size(%d), offset(0x%x)\n",
            sequence, dev_index, bytesused, dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw1::dumpIspRegBlock(uint32_t base_addr, uint32_t offset_addr,
        sint32_t len, uint32_t sequence)
{
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
DumpRkRaw1::dumpMetadataBlock(uint32_t sequence,
        rkisp_effect_params_v20& ispParams,
        SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
        SmartPtr<RkAiqAfInfoProxy>& afParams)
{
    if (dumpNum <= 0 || !expParams.ptr()) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "dumpNum(%d)!\n", dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE *ffp = nullptr;
    char file_name[128] = {0};
    char buffer[256] = {0};
    int32_t focusCode = 0;
    int32_t zoomCode = 0;
    int32_t working_mode = ispInfo.working_mode;

    snprintf(file_name, sizeof(file_name), "%s/meta_data", dumpDirPath);

    ffp = fopen(file_name, "ab+");
    if (!ffp)
        return XCAM_RETURN_ERROR_FILE;

    if (afParams.ptr()) {
        focusCode = afParams->data()->focusCode;
        zoomCode = afParams->data()->zoomCode;
    }

    if (working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
        working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
#if defined(ISP_HW_V20)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                    ispParams.isp_params.others.awb_gain_cfg.gain_red,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                    ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                    1,
                    focusCode,
                    zoomCode);
#elif defined(ISP_HW_V21)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                    1,
                    focusCode,
                    zoomCode);
#endif
    } else if (working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
               working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
#if defined(ISP_HW_V20)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                    ispParams.isp_params.others.awb_gain_cfg.gain_red,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                    ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                    1,
                    focusCode,
                    zoomCode);
#elif defined(ISP_HW_V21)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                    1,
                    focusCode,
                    zoomCode);
#elif defined(ISP_HW_V30)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                    expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue,
                    1,
                    focusCode,
                    zoomCode);
#endif
    } else {
#if defined(ISP_HW_V20)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-gain[%08.5f]-time[%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                    ispParams.isp_params.others.awb_gain_cfg.gain_red,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                    ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                    ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                    1,
                    focusCode,
                    zoomCode);
#elif defined(ISP_HW_V21)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-gain[%08.5f]-time[%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                    ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                    1,
                    focusCode,
                    zoomCode);
#elif defined(ISP_HW_V30)
            snprintf(buffer,
                    sizeof(buffer),
                    "frame%08d-gain[%08.5f]-time[%08.5f]-"
                    "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                    sequence,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                    expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                    ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue,
                    1,
                    focusCode,
                    zoomCode);
#endif
    }

    fwrite((void *)buffer, strlen(buffer), 1, ffp);
    fflush(ffp);
    fclose(ffp);

    return XCAM_RETURN_NO_ERROR;
}

/****************************** dump RKRAW2 *******************************/

XCamReturn
DumpRkRaw2::dumpBlockHeader(uint16_t blockID, uint32_t blockSize)
{
    if (blockID != END_ID && dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if(fp == nullptr)
        return XCAM_RETURN_ERROR_FILE;

    fwrite(&blockID, sizeof(blockID), 1, fp);
    if(blockSize > 0)
        fwrite(&blockSize, sizeof(blockSize), 1, fp);
    fflush(fp);

    uint32_t size = blockSize > 0 ? sizeof(blockID) + sizeof(blockSize) : sizeof(blockID);
    dumpTotalSize += size;

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "dump blockId(0x%x), size(%d), offset(0x%x)\n",
            blockID, size, dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpRkRawHeaderBlock(uint32_t sequence, int32_t dev_index)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dumpBlockHeader(RAW_HEADER_BLOCK_ID, sizeof(struct rawHeader_s));
    dumpRkRawHeader(sequence);

    return XCAM_RETURN_NO_ERROR;
}

/*
 * Refer to "Raw file structure" in the header of this file
 */
XCamReturn
DumpRkRaw2::dumpRkRawHeader(sint32_t sequence)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint8_t working_mode = 0;

    if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
        ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        working_mode = 3;
    else if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
               ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        working_mode = 2;
    else if (ispInfo.working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        working_mode = 1;

    struct rawHeader_s rawHeader = {};

    rawHeader.version = RAW_HEADER_VERSION;
    strcpy(rawHeader.snsName, ispInfo.sns_name);
    rawHeader.frameId = sequence;
    rawHeader.sns_width = ispInfo.sns_width;
    rawHeader.sns_height = ispInfo.sns_height;
    rawHeader.bpp = ispInfo.bpp;
    rawHeader.bayer_fmt = ispInfo.bayer_fmt;
    rawHeader.working_mode = working_mode;
    rawHeader.storage_type = 0;
    rawHeader.stridePerLine = ispInfo.stridePerLine;
    rawHeader.bytesPerLine = ispInfo.bytesPerLine;
    rawHeader.endian = 1;

    fwrite(&rawHeader, sizeof(rawHeader), 1, fp);
    fflush(fp);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                    "frame%d: image rect(%dx%d), bpp(%d), bayer_fmt(%d), "
                    "hdr frame number(%d), Storage type(%d), "
                    "line stride(%d), Effective line stride(%d)\n",
                    sequence, ispInfo.sns_width, ispInfo.sns_height,
                    ispInfo.bpp, ispInfo.bayer_fmt, working_mode,
                    rawHeader.storage_type,
                    ispInfo.stridePerLine,
                    ispInfo.bytesPerLine);

    dumpTotalSize += sizeof(rawHeader);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "frame%d: dump rawHeader size(%d), offset(0x%x)\n",
            sequence, sizeof(rawHeader), dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpRkRawBlock(sint32_t dev_index, uint32_t sequence, unsigned long userptr,
                        uint32_t bytesused)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dumpRkRawHeaderBlock(sequence, dev_index);

    uint32_t rawBlockId = 0;

    XCAM_STATIC_PROFILING_START(write_raw);
    if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
        ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        rawBlockId = dev_index == 0 ? HDR_S_RAW_BLOCK_ID : dev_index == 1 ? \
                     HDR_M_RAW_BLOCK_ID : HDR_L_RAW_BLOCK_ID;
    else if (ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
            ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        rawBlockId = dev_index == 0 ? HDR_S_RAW_BLOCK_ID :  HDR_M_RAW_BLOCK_ID;
    else if (ispInfo.working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        rawBlockId = NORMAL_RAW_BLOCK_ID;

    dumpBlockHeader(rawBlockId, bytesused);
    dumpRkRaw(dev_index, sequence, (void *)userptr, bytesused);
    XCAM_STATIC_PROFILING_END(write_raw, 0);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpRkRaw(sint32_t dev_index,
                   sint32_t sequence, void* userptr,
                   sint32_t size)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    fwrite(userptr, size, 1, fp);
    fflush(fp);

    dumpTotalSize += size;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "frame%d: dump raw data: dev_index(%d), size(%d), offset(0x%x)\n",
            sequence, dev_index, size, dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpMetadataBlock(uint32_t sequence,
        rkisp_effect_params_v20& ispParams,
        SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
        SmartPtr<RkAiqAfInfoProxy>& afParams)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dumpBlockHeader(METADATA_BLOCK_ID, sizeof(struct metadata_s));
    dumpMetadata(sequence, ispParams, expParams);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpMetadata(uint32_t sequence,
        rkisp_effect_params_v20& ispParams,
        SmartPtr<RkAiqSensorExpParamsProxy>& expParams)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct metadata_s metaData = {};

    metaData.version = RAW_METADATA_VERSION;
    metaData.frameId = sequence;

    metaData.linearExp.exp_real.integration_time =
        expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time;
    metaData.linearExp.exp_real.analog_gain =
        expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain;
    metaData.linearExp.exp_sensor.integration_time =
        expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time;
    metaData.linearExp.exp_sensor.analog_gain =
        expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global;

    metaData.HdrExp[0].exp_real.integration_time =
                expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time;
    metaData.HdrExp[0].exp_real.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain;
    metaData.HdrExp[0].exp_sensor.integration_time =
                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[0].exp_real.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global;
    metaData.HdrExp[1].exp_real.integration_time =
                expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time;
    metaData.HdrExp[1].exp_real.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain;
    metaData.HdrExp[1].exp_sensor.integration_time =
                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[1].exp_sensor.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global;
    metaData.HdrExp[2].exp_real.integration_time =
                expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time;
    metaData.HdrExp[2].exp_real.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain;
    metaData.HdrExp[2].exp_sensor.integration_time =
                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[2].exp_sensor.analog_gain =
                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global;

#if defined(ISP_HW_V30)
    metaData.rgain = ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red;
    metaData.bgain = ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue;
#endif

    fwrite(&metaData, sizeof(metaData), 1, fp);
    fflush(fp);

    dumpTotalSize += sizeof(metaData);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "frame(%d): dump metaData size(%d), offset(0x%x)\n",
            sequence, sizeof(metaData), dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpIspRegHeaderBlock(uint32_t base_addr, uint32_t offset_addr,
                              sint32_t length, uint32_t sequence)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint32_t regBlockId = 0, regBlockSize = 0;
    if (base_addr == ISP_REGS_BASE) {
        regBlockId = 0xff07;
        regBlockSize = sizeof(struct ispRegHeader_s);
    } else {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Invalid register addr(0x%x)\n", base_addr);
        return XCAM_RETURN_ERROR_MEM;
    }

    dumpBlockHeader(regBlockId, regBlockSize);
    dumpIspRegHeader(base_addr, offset_addr, length, sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpIspRegHeader(uint32_t base_addr, uint32_t offset_addr,
                         sint32_t length, uint32_t sequence)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint32_t regBlockId = 0, regBlockSize = 0;
    if (base_addr == ISP_REGS_BASE) {
        struct ispRegHeader_s ispRegHeader = {};

        ispRegHeader.version = ISP_REG_VERSION;
        ispRegHeader.frameId = sequence;
        ispRegHeader.base_addr = base_addr;
        ispRegHeader.offset_addr = offset_addr;
        ispRegHeader.count = length;

        fwrite(&ispRegHeader, sizeof(ispRegHeader_s), 1, fp);
        fflush(fp);

        dumpTotalSize += sizeof(ispRegHeader);
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                "frame(%d): dump isp reg header base(0x%x), size(%d), offset(0x%x)\n",
                sequence, base_addr, sizeof(ispRegHeader), dumpTotalSize);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpIspRegBlock(uint32_t base_addr, uint32_t offset_addr,
                   sint32_t length, uint32_t sequence)
{
#ifndef DUMP_ISP_REG
    return XCAM_RETURN_ERROR_PARAM;
#endif

    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dumpIspRegHeaderBlock(ISP_REGS_BASE, 0, ISP_REGS_SIZE, sequence);

    uint32_t regBlockId = 0;
    if (base_addr == ISP_REGS_BASE)
        regBlockId = 0xff08;
    else {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Invalid register addr(0x%x)\n", base_addr);
        return XCAM_RETURN_ERROR_MEM;
    }

    dumpBlockHeader(regBlockId, length);
    dumpIspReg(base_addr, offset_addr, length, sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpIspReg(uint32_t base_addr, uint32_t offset_addr,
                   sint32_t length, uint32_t sequence)
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sint32_t fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd == -1) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "open /dev/mem failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
    }

    uint8_t *mmap_base = (uint8_t *)mmap64(NULL, length, PROT_READ, MAP_SHARED,
                                             fd, base_addr);
    if (mmap_base == (void *)(-1)) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mmap() failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_MEM;
    }

	int8_t ffd = open(raw_name, O_WRONLY | O_APPEND);
	if (ffd < 0) {
		LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Failed to open destination file '%s': %s\n", raw_name, strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
	}

	int n = write(ffd, mmap_base + offset_addr, length);
	if (n < 0) {
		LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "File write failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
	} else if (n != length) {
		LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Only wrote %d of %d bytes to file\n",
				n, length);
        return XCAM_RETURN_ERROR_FILE;
	}

#ifdef PRINT_ISP_REG
	memread_memory(base_addr, mmap_base + offset_addr, 0x100, 4);
#endif

    munmap(mmap_base, length);
    close (ffd);
    close (fd);

    dumpTotalSize += length;

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "frame%d: dump reg(0x%x), size(0x%x), offset(0x%x)\n",
            sequence, base_addr + offset_addr, length,  dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpPlatformInfoBlock()
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dumpBlockHeader(PLATFORM_INFO_BLOCK_ID, sizeof(struct platform_info_s));
    dumpPlatformInfo();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
DumpRkRaw2::dumpPlatformInfo()
{
    if (fp == nullptr || dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", fp, dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct platform_info_s platform_info = {};

    if (CHECK_ISP_HW_V20())
        strcpy(platform_info.chipModel, "rv1109");
    else if (CHECK_ISP_HW_V21())
        strcpy(platform_info.chipModel, "rv356x");
    else if (CHECK_ISP_HW_V30())
        strcpy(platform_info.chipModel, "rv3588");

    strcpy(platform_info.aiqVersion, RK_AIQ_VERSION_REAL_V);
#if 0
    snprintf(platform_info.ispVersion, sizeof(platform_info.ispVersion),
            "v%d.%d.%d",
            (RKISP_API_VERSION >> 16) & 0xFF,
            (RKISP_API_VERSION >> 8) & 0xFF,
            RKISP_API_VERSION & 0xFF);
#endif

    fwrite(&platform_info, sizeof(platform_info), 1, fp);
    fflush(fp);

    dumpTotalSize += sizeof(platform_info);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
            "dump platform_info size(%d), offset(0x%x)\n",
            sizeof(platform_info), dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

};

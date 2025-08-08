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

#include "aiq_dumpRkRaw.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RkAiqVersion.h"
#include "common/rk_aiq_types_priv_c.h"
#include "xcore/xcam_obj_debug.h"

#ifdef ANDROID_OS
#define DEFAULT_CAPTURE_RAW_PATH "/data/capture_image"
#define CAPTURE_CNT_FILENAME     "/data/.capture_cnt"
#else
#define DEFAULT_CAPTURE_RAW_PATH "/tmp/capture_image"
#define CAPTURE_CNT_FILENAME     "/tmp/.capture_cnt"
#endif

#define WRITE_RAW_FILE_HEADER

#define RAW_FILE_IDENT 0x8080
#define HEADER_LEN     128U

#define RAW_HEADER_VERSION   0x0100
#define RAW_METADATA_VERSION 0x0100
#define ISP_REG_VERSION      0x0100
#define ISPP_REG_VERSION     0x0100

#define START_ID               0xFF00
#define END_ID                 0x00FF
#define RAW_HEADER_BLOCK_ID    0xFF01
#define NORMAL_RAW_BLOCK_ID    0xFF02
#define HDR_S_RAW_BLOCK_ID     0xFF03
#define HDR_M_RAW_BLOCK_ID     0xFF04
#define HDR_L_RAW_BLOCK_ID     0xFF05
#define METADATA_BLOCK_ID      0xFF06
#define ISP_HEADER_BLOCK_ID    0xFF07
#define ISP_BLOCK_ID           0xFF08
#define ISPP_HEADER_BLOCK_ID   0xFF09
#define ISPP_BLOCK_ID          0xFF0a
#define PLATFORM_INFO_BLOCK_ID 0xFF0b

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

static const capture_fmt_t csirx_fmts[] = {
    /* raw */
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB8,
        .bayer_fmt = 3,
        .pcpp      = 1,
        .bpp       = {8},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG8,
        .bayer_fmt = 2,
        .pcpp      = 1,
        .bpp       = {8},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG8,
        .bayer_fmt = 1,
        .pcpp      = 1,
        .bpp       = {8},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR8,
        .bayer_fmt = 0,
        .pcpp      = 1,
        .bpp       = {8},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB10,
        .bayer_fmt = 3,
        .pcpp      = 4,
        .bpp       = {10},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG10,
        .bayer_fmt = 2,
        .pcpp      = 4,
        .bpp       = {10},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG10,
        .bayer_fmt = 1,
        .pcpp      = 4,
        .bpp       = {10},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR10,
        .bayer_fmt = 0,
        .pcpp      = 4,
        .bpp       = {10},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SRGGB12,
        .bayer_fmt = 3,
        .pcpp      = 2,
        .bpp       = {12},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGRBG12,
        .bayer_fmt = 2,
        .pcpp      = 2,
        .bpp       = {12},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SGBRG12,
        .bayer_fmt = 1,
        .pcpp      = 2,
        .bpp       = {12},
    },
    {
        .fourcc    = V4L2_PIX_FMT_SBGGR12,
        .bayer_fmt = 0,
        .pcpp      = 2,
        .bpp       = {12},
    },
};

static const capture_fmt_t* findFmt(const uint32_t pixelformat) {
    const capture_fmt_t* fmt;

    for (uint32_t i = 0; i < sizeof(csirx_fmts); i++) {
        fmt = &csirx_fmts[i];
        if (fmt->fourcc == pixelformat) return fmt;
    }

    return NULL;
}

void calculate_stride_per_line(struct ispInfo_s* ispInfo, const capture_fmt_t fmt,
                               uint32_t* bytesPerLine) {
    *bytesPerLine = ispInfo->sns_width * fmt.bpp[0] / 8;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "sns_width: %d, bytesPerLine: %d\n", ispInfo->sns_width,
                    *bytesPerLine);
}

bool getValueFromFile(const sint8_t* path, sint32_t* value, uint32_t* frameId) {
    const sint8_t* delim = " ";
    sint8_t buffer[16]   = {0};
    sint32_t fd;

    fd = open(path, O_RDONLY | O_SYNC);
    if (fd != -1) {
        if (read(fd, buffer, sizeof(buffer)) <= 0) {
            LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "%s read %s failed!\n", __func__, path);
        } else {
            char* p = NULL;

            p = strtok(buffer, delim);
            if (p != NULL) {
                *value = atoi(p);
                p      = strtok(NULL, delim);
                if (p != NULL) *frameId = atoi(p);
            }
        }
        close(fd);
        return true;
    }

    return false;
}

bool saveValueToFile(const sint8_t* path, sint32_t value, uint32_t sequence, int32_t dumpNum) {
    sint8_t buffer[16] = {0};
    sint32_t fd;

    fd = open(path, O_CREAT | O_RDWR | O_SYNC, S_IRWXU | S_IRUSR | S_IXUSR | S_IROTH | S_IXOTH);
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

static XCamReturn creatDumpDir(aiq_DumpRkRaw_t* pDpRkRaw, const char* path) {
    time_t now;
    struct tm* timenow;

    if (!path) return XCAM_RETURN_ERROR_FAILED;

    time(&now);
    timenow = localtime(&now);

    if (access(path, W_OK) == -1) {
        if (mkdir(path, 0755) < 0) {
            LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!\n", path, strerror(errno));
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    snprintf(pDpRkRaw->dumpDirPath, sizeof(pDpRkRaw->dumpDirPath),
             "%s/Cam%d-rkraw%d_%04d-%02d-%02d_%02d-%02d-%02d", path, pDpRkRaw->_camId,
             pDpRkRaw->dumpRkRawType, timenow->tm_year + 1900, timenow->tm_mon + 1,
             timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s for capturing %d frames raw!\n",
                    pDpRkRaw->dumpDirPath, pDpRkRaw->dumpNum);

    if (mkdir(pDpRkRaw->dumpDirPath, 0755) < 0) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!!!\n", pDpRkRaw->dumpDirPath,
                        strerror(errno));
        return XCAM_RETURN_ERROR_PARAM;
    }

    pDpRkRaw->isDumpDirExist = true;

    return XCAM_RETURN_ERROR_PARAM;
}

static XCamReturn creatDumpFile(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence) {
    if (pDpRkRaw->fp != NULL) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "fp don't be clesed, maybe the frameEnd delayed!\n");
        fclose(pDpRkRaw->fp);
        pDpRkRaw->fp = NULL;
    }

    // char raw_name[128] = {0};
    memset(pDpRkRaw->raw_name, 0, sizeof(pDpRkRaw->raw_name));
    snprintf(pDpRkRaw->raw_name, sizeof(pDpRkRaw->raw_name), "%s/%s_frame%d_%dx%d.rkraw",
             pDpRkRaw->dumpDirPath, pDpRkRaw->ispInfo.sns_name, sequence,
             pDpRkRaw->ispInfo.sns_width, pDpRkRaw->ispInfo.sns_height);

    pDpRkRaw->fp = fopen(pDpRkRaw->raw_name, "ab+");
    if (pDpRkRaw->fp == NULL) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen %s failed!\n", pDpRkRaw->raw_name);
        return XCAM_RETURN_ERROR_FILE;
    } else {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen (%s) sucessfully, fp(%p)\n", pDpRkRaw->raw_name,
                        pDpRkRaw->fp);
    }

    // the start identifier of RK-RAW file
    if (pDpRkRaw->dumpBlockHeader) pDpRkRaw->dumpBlockHeader(pDpRkRaw, START_ID, 0);

    return XCAM_RETURN_NO_ERROR;
}

static void memread_memory(unsigned long phys_addr, uint8_t* addr, int len, int iosize) {
    int i;

    while (len) {
        printf("%08lx: ", phys_addr);
        i = 0;
        while (i < 16 && len) {
            switch (iosize) {
                case 1:
                    printf(" %02x", *(uint8_t*)addr);
                    break;
                case 2:
                    printf(" %04x", *(uint16_t*)addr);
                    break;
                case 4:
                    printf(" %08x", *(uint32_t*)addr);
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

void aiq_dumpRkRaw_setIspInfo(aiq_DumpRkRaw_t* pDpRkRaw, struct ispInfo_s* info) {
    strcpy(pDpRkRaw->ispInfo.sns_name, info->sns_name);
    pDpRkRaw->ispInfo.working_mode = info->working_mode;

    pDpRkRaw->ispInfo.pixelformat   = info->pixelformat;
    pDpRkRaw->ispInfo.sns_width     = info->sns_width;
    pDpRkRaw->ispInfo.sns_height    = info->sns_height;
    pDpRkRaw->ispInfo.bpp           = info->bpp;
    pDpRkRaw->ispInfo.stridePerLine = info->stridePerLine;

    const capture_fmt_t* fmt = NULL;
    if ((fmt = findFmt(pDpRkRaw->ispInfo.pixelformat)))
        calculate_stride_per_line(&pDpRkRaw->ispInfo, *fmt, &pDpRkRaw->ispInfo.bytesPerLine);
    pDpRkRaw->ispInfo.bpp       = fmt->bpp[0];
    pDpRkRaw->ispInfo.bayer_fmt = fmt->bayer_fmt;

    LOGV_CAMHW_SUBM(
        CAPTURERAW_SUBM,
        "sns_name(%s), mode(%d), rect(%dx%d), bpp(%d), bytesPerLine(%d), stridePerLine(%d)\n",
        pDpRkRaw->ispInfo.sns_name, pDpRkRaw->ispInfo.working_mode, pDpRkRaw->ispInfo.sns_width,
        pDpRkRaw->ispInfo.sns_height, pDpRkRaw->ispInfo.bpp, pDpRkRaw->ispInfo.bytesPerLine,
        pDpRkRaw->ispInfo.stridePerLine);
}

XCamReturn aiq_dumpRkRaw_isDumpInFrameEnd(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                          aiq_isp_effect_params_t* ispParams,
                                          aiq_sensor_exp_info_t* expParams) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sint32_t num      = 0;
    uint32_t rawFrmId = 0;

    getValueFromFile(CAPTURE_CNT_FILENAME, &num, &rawFrmId);
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "rawFrmId: %d, frameEndId: %d, dumpNum: %d\n", rawFrmId,
                    sequence, pDpRkRaw->dumpNum);

    if (sequence >= rawFrmId) return XCAM_RETURN_NO_ERROR;

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn aiq_dumpRkRaw_detectDumpStatus(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence) {
    if (pDpRkRaw->dumpNum <= 0) {
        uint32_t rawFrmId = 0;

        bool ret = getValueFromFile(CAPTURE_CNT_FILENAME, &pDpRkRaw->dumpNum, &rawFrmId);
        if (!ret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME,
                     pDpRkRaw->_camId);
            getValueFromFile(file_name, &pDpRkRaw->dumpNum, &rawFrmId);
        }

        if (pDpRkRaw->dumpNum > 0) {
            bool ret = saveValueToFile(CAPTURE_CNT_FILENAME, pDpRkRaw->dumpNum, sequence,
                                       pDpRkRaw->dumpNum);
            if (!ret) {
                // test multi cam mode
                char file_name[64] = {0};
                snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME,
                         pDpRkRaw->_camId);
                saveValueToFile(file_name, pDpRkRaw->dumpNum, sequence, pDpRkRaw->dumpNum);
            }

            if (!pDpRkRaw->isDumpDirExist) {
                if (pDpRkRaw->dumpType == CAPTURE_RAW_SYNC)
                    creatDumpDir(pDpRkRaw, pDpRkRaw->userSetDumpDir);
                else
                    creatDumpDir(pDpRkRaw, DEFAULT_CAPTURE_RAW_PATH);
            }
        }
    }

    if (pDpRkRaw->dumpNum > 0) creatDumpFile(pDpRkRaw, sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_updateDumpStatus(aiq_DumpRkRaw_t* pDpRkRaw) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (!--pDpRkRaw->dumpNum) {
        pDpRkRaw->isDumpDirExist = false;
        if (pDpRkRaw->dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
            aiqMutex_lock(&pDpRkRaw->dumpExternMutex);
            aiqCond_broadcast(&pDpRkRaw->dumpExternCond);
            aiqMutex_unlock(&pDpRkRaw->dumpExternMutex);
        }

        bool ret = saveValueToFile(CAPTURE_CNT_FILENAME, pDpRkRaw->dumpNum, 0, 0);
        if (!ret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME,
                     pDpRkRaw->_camId);
            saveValueToFile(file_name, pDpRkRaw->dumpNum, 0, 0);
        }

        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "stop capturing raw!\n");
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_waitDumpRawDone(aiq_DumpRkRaw_t* pDpRkRaw) {
    if (pDpRkRaw->dumpNum > 0) {
        aiqMutex_lock(&pDpRkRaw->dumpInnerMutex);
        if (aiqCond_timedWait(&pDpRkRaw->dumpInnerCond, &pDpRkRaw->dumpInnerMutex, 500 * 1000) != 0)
            LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "wait 500ms timeout!!!\n");
        aiqMutex_unlock(&pDpRkRaw->dumpInnerMutex);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_notifyDumpRaw(aiq_DumpRkRaw_t* pDpRkRaw) {
    if (pDpRkRaw->fp != NULL) {
        if (pDpRkRaw->dumpBlockHeader) pDpRkRaw->dumpBlockHeader(pDpRkRaw, END_ID, 0);
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fclose fp(%p), dumpTotalSize(%d)!\n", pDpRkRaw->fp,
                        pDpRkRaw->dumpTotalSize);

        fclose(pDpRkRaw->fp);
        pDpRkRaw->fp = NULL;
    }

    pDpRkRaw->dumpTotalSize = 0;
    for (sint32_t i = 0; i <= pDpRkRaw->dumpNum; i++) printf(">");
    printf("\n");

    aiqMutex_lock(&pDpRkRaw->dumpInnerMutex);
    aiqCond_broadcast(&pDpRkRaw->dumpInnerCond);
    aiqMutex_unlock(&pDpRkRaw->dumpInnerMutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_waitThirdPartyDumpDone(aiq_DumpRkRaw_t* pDpRkRaw, bool isFirstTrigger) {
    if (pDpRkRaw->dumpNum > 0 && !isFirstTrigger) {
        if (pDpRkRaw->dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
            aiqMutex_lock(&pDpRkRaw->dumpExternMutex);
            aiqCond_timedWait(&pDpRkRaw->dumpExternCond, &pDpRkRaw->dumpExternMutex, 3000000);
            aiqMutex_unlock(&pDpRkRaw->dumpExternMutex);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_notifyDumpRawFromThirdParty(aiq_DumpRkRaw_t* pDpRkRaw) {
    aiqMutex_lock(&pDpRkRaw->dumpExternMutex);
    aiqCond_broadcast(&pDpRkRaw->dumpExternCond);
    aiqMutex_unlock(&pDpRkRaw->dumpExternMutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn aiq_dumpRkRaw_dumpControl(aiq_DumpRkRaw_t* pDpRkRaw, capture_raw_t type, int32_t count,
                                     const char* capture_dir, char* output_dir) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pDpRkRaw->dumpType = type;
    if (pDpRkRaw->dumpType == CAPTURE_RAW_SYNC) {
        if (capture_dir != NULL)
            snprintf(pDpRkRaw->userSetDumpDir, sizeof(pDpRkRaw->userSetDumpDir), "%s/capture_image",
                     capture_dir);
        else
            strcpy(pDpRkRaw->userSetDumpDir, DEFAULT_CAPTURE_RAW_PATH);

        bool bret = saveValueToFile(CAPTURE_CNT_FILENAME, count, 0, 0);
        if (!bret) {
            // test multi cam mode
            char file_name[64] = {0};
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME,
                     pDpRkRaw->_camId);
            saveValueToFile(file_name, count, 0, 0);
        }

        aiqMutex_lock(&pDpRkRaw->dumpExternMutex);
        if (aiqCond_timedWait(&pDpRkRaw->dumpExternCond, &pDpRkRaw->dumpExternMutex, 3000000) != 0)
            ret = XCAM_RETURN_ERROR_TIMEOUT;
        else
            strncpy(output_dir, pDpRkRaw->dumpDirPath, strlen(output_dir));
        aiqMutex_unlock(&pDpRkRaw->dumpExternMutex);
    } else if (pDpRkRaw->dumpType == CAPTURE_RAW_AND_YUV_SYNC) {
        LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "capture raw and yuv images simultaneously!");
    }

    return ret;
}

void aiq_dumpRkRaw_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId) {
    pDpRkRaw->dumpType              = CAPTURE_RAW_ASYNC;
    pDpRkRaw->dumpRkRawType         = DUMP_RKRAW_DEFAULT;
    pDpRkRaw->_camId                = camId;
    pDpRkRaw->dumpRkRawBlock        = NULL;
    pDpRkRaw->dumpIspRegBlock       = NULL;
    pDpRkRaw->dumpMetadataBlock     = NULL;
    pDpRkRaw->dumpPlatformInfoBlock = NULL;
    pDpRkRaw->dumpIspRegHeaderBlock = NULL;
    pDpRkRaw->dumpBlockHeader       = NULL;
    aiqMutex_init(&pDpRkRaw->dumpExternMutex);
    aiqCond_init(&pDpRkRaw->dumpExternCond);
    aiqMutex_init(&pDpRkRaw->dumpInnerMutex);
    aiqCond_init(&pDpRkRaw->dumpInnerCond);
}

void aiq_dumpRkRaw_deinit(aiq_DumpRkRaw_t* pDpRkRaw) {
    aiqMutex_deInit(&pDpRkRaw->dumpExternMutex);
    aiqMutex_deInit(&pDpRkRaw->dumpInnerMutex);
    aiqCond_deInit(&pDpRkRaw->dumpExternCond);
    aiqCond_deInit(&pDpRkRaw->dumpInnerCond);
}

/****************************** dump RKRAW1 *******************************/

static FILE* DumpRkRaw1_creatHdrDumpFile(aiq_DumpRkRaw_t* pDpRkRaw, sint32_t dev_index,
                                         uint32_t sequence) {
    FILE* fpp = NULL;

    int32_t working_mode = pDpRkRaw->ispInfo.working_mode;
    // char raw_name[128] = {0};
    memset(pDpRkRaw->raw_name, 0, sizeof(pDpRkRaw->raw_name));
    if (working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
        working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        snprintf(pDpRkRaw->raw_name, sizeof(pDpRkRaw->raw_name), "%s/%s_frame%d_%dx%d_%s",
                 pDpRkRaw->dumpDirPath, pDpRkRaw->ispInfo.sns_name, sequence,
                 pDpRkRaw->ispInfo.sns_width, pDpRkRaw->ispInfo.sns_height,
                 dev_index == 1 ? "middle" : "long");
    else if (working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
             working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        snprintf(pDpRkRaw->raw_name, sizeof(pDpRkRaw->raw_name), "%s/%s_frame%d_%dx%d_%s",
                 pDpRkRaw->dumpDirPath, pDpRkRaw->ispInfo.sns_name, sequence,
                 pDpRkRaw->ispInfo.sns_width, pDpRkRaw->ispInfo.sns_height, "long");

    fpp = fopen(pDpRkRaw->raw_name, "ab+");
    if (fpp == NULL) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Failed to fopen %s: %s\n", pDpRkRaw->raw_name,
                        strerror(errno));
        return NULL;
    } else {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fopen (%s) sucessfully, fp(%p)\n", pDpRkRaw->raw_name,
                        fpp);
    }

    return fpp;
}

static XCamReturn DumpRkRaw1_dumpRkRawHeaderBlock(aiq_DumpRkRaw_t* pDpRkRaw, FILE* fpp,
                                                  uint32_t sequence, int32_t dev_index) {
    if (fpp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fpp(%p), dumpNum(%d)!\n", fpp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    char buffer[128]   = {0};
    uint8_t mode       = 0;
    uint8_t frame_type = 0, storage_type = 0;

    if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
        pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        mode       = 3;
        frame_type = dev_index == 0 ? 1 : dev_index == 1 ? 2 : 3;
    } else if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        mode       = 2;
        frame_type = dev_index == 0 ? 1 : 3;
    } else {
        mode = 1;
    }

    *((uint16_t*)buffer)        = RAW_FILE_IDENT;                   // Identifier
    *((uint16_t*)(buffer + 2))  = HEADER_LEN;                       // Header length
    *((uint32_t*)(buffer + 4))  = sequence;                         // Frame number
    *((uint16_t*)(buffer + 8))  = pDpRkRaw->ispInfo.sns_width;      // Image width
    *((uint16_t*)(buffer + 10)) = pDpRkRaw->ispInfo.sns_height;     // Image height
    *(buffer + 12)              = pDpRkRaw->ispInfo.bpp;            // Bit depth
    *(buffer + 13)              = pDpRkRaw->ispInfo.bayer_fmt;      // Bayer format
    *(buffer + 14)              = mode;                             // Number of HDR frame
    *(buffer + 15)              = frame_type;                       // Current frame type
    *(buffer + 16)              = storage_type;                     // Storage type
    *((uint16_t*)(buffer + 17)) = pDpRkRaw->ispInfo.stridePerLine;  // Line stride
    *((uint16_t*)(buffer + 19)) = pDpRkRaw->ispInfo.bytesPerLine;   // Effective line stride

    fwrite(buffer, sizeof(buffer), 1, fpp);
    fflush(fpp);

    pDpRkRaw->dumpTotalSize += strlen(buffer);
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                    "frame%d: image rect: %dx%d, %d bit depth, Bayer fmt: %d, "
                    "hdr frame number: %d, frame type: %d, Storage type: %d, "
                    "line stride: %d, Effective line stride: %d\n",
                    sequence, pDpRkRaw->ispInfo.sns_width, pDpRkRaw->ispInfo.sns_height,
                    pDpRkRaw->ispInfo.bpp, pDpRkRaw->ispInfo.bayer_fmt, mode, frame_type,
                    storage_type, pDpRkRaw->ispInfo.stridePerLine, pDpRkRaw->ispInfo.bytesPerLine);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn DumpRkRaw1_dumpRkRawBlock(aiq_DumpRkRaw_t* pDpRkRaw, sint32_t dev_index,
                                     uint32_t sequence, unsigned long userptr, uint32_t bytesused) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE* fpp = pDpRkRaw->fp;
    if (dev_index != 0) fpp = DumpRkRaw1_creatHdrDumpFile(pDpRkRaw, dev_index, sequence);

    if (!fpp) return XCAM_RETURN_ERROR_FILE;

    DumpRkRaw1_dumpRkRawHeaderBlock(pDpRkRaw, fpp, sequence, dev_index);

    fwrite((void*)userptr, bytesused, 1, fpp);
    fflush(fpp);
    if (dev_index != 0) fclose(fpp);

    pDpRkRaw->dumpTotalSize += bytesused;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                    "frame%d: dump raw data: dev_index(%d), size(%d), offset(0x%x)\n", sequence,
                    dev_index, bytesused, pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw1_dumpMetadataBlock(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                               aiq_isp_effect_params_t* ispParams,
                                               aiq_sensor_exp_info_t* expParams,
                                               AiqAfInfoWrapper_t* afParams) {
    if (pDpRkRaw->dumpNum <= 0 || !expParams) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "dumpNum(%d)!\n", pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE* ffp            = NULL;
    char file_name[128]  = {0};
    char buffer[256]     = {0};
    int32_t focusCode    = 0;
    int32_t zoomCode     = 0;
    int32_t working_mode = pDpRkRaw->ispInfo.working_mode;

    snprintf(file_name, sizeof(file_name), "%s/meta_data", pDpRkRaw->dumpDirPath);

    ffp = fopen(file_name, "ab+");
    if (!ffp) return XCAM_RETURN_ERROR_FILE;

    if (afParams) {
        focusCode = afParams->focusCode;
        zoomCode  = afParams->zoomCode;
    }

    if (RK_AIQ_HDR_IS_HDR2(working_mode)) {
#if defined(ISP_HW_V21)
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                 expParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_red,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                 ispParams - <> isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_blue, 1, focusCode, zoomCode);
#endif
    } else if (RK_AIQ_HDR_IS_HDR3(working_mode)) {
#if defined(ISP_HW_V21)
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_red,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_blue, 1, focusCode, zoomCode);
#elif defined(ISP_HW_V30)
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_blue, 1, focusCode,
                 zoomCode);
#else
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 expParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 expParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 ispParams->awb_gain_cfg.gain0_red, ispParams->awb_gain_cfg.gain0_green_r,
                 ispParams->awb_gain_cfg.gain0_green_b, ispParams->awb_gain_cfg.gain0_blue, 1,
                 focusCode, zoomCode);
#endif
    } else {
#if defined(ISP_HW_V21)
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-gain[%08.5f]-time[%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                 expParams->aecExpInfo.LinearExp.exp_real_params.integration_time,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_red,
                 ispParams->.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                 ispParams->isp_params_v21.others.awb_gain_cfg.gain0_blue, 1, focusCode, zoomCode);
#elif defined(ISP_HW_V30)
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-gain[%08.5f]-time[%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                 expParams->aecExpInfo.LinearExp.exp_real_params.integration_time,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                 ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_blue, 1, focusCode,
                 zoomCode);
#else
        snprintf(buffer, sizeof(buffer),
                 "frame%08d-gain[%08.5f]-time[%08.5f]-"
                 "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                 sequence, expParams->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                 expParams->aecExpInfo.LinearExp.exp_real_params.integration_time,
                 ispParams->awb_gain_cfg.gain0_red, ispParams->awb_gain_cfg.gain0_green_r,
                 ispParams->awb_gain_cfg.gain0_green_b, ispParams->awb_gain_cfg.gain0_blue, 1,
                 focusCode, zoomCode);
#endif
    }

    fwrite((void*)buffer, strlen(buffer), 1, ffp);
    fflush(ffp);
    fclose(ffp);

    return XCAM_RETURN_NO_ERROR;
}

void aiq_dumpRkRaw1_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId) {
    aiq_dumpRkRaw_init(pDpRkRaw, camId);
    pDpRkRaw->dumpRkRawBlock    = DumpRkRaw1_dumpRkRawBlock;
    pDpRkRaw->dumpIspRegBlock   = NULL;
    pDpRkRaw->dumpRkRawBlock    = DumpRkRaw1_dumpRkRawBlock;
    pDpRkRaw->dumpMetadataBlock = DumpRkRaw1_dumpMetadataBlock;
}

void aiq_dumpRkRaw1_deinit(aiq_DumpRkRaw_t* pDpRkRaw) { return aiq_dumpRkRaw_deinit(pDpRkRaw); }

/****************************** dump RKRAW2 *******************************/

static XCamReturn DumpRkRaw2_dumpBlockHeader(aiq_DumpRkRaw_t* pDpRkRaw, uint16_t blockID,
                                             uint32_t blockSize) {
    if (blockID != END_ID && pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pDpRkRaw->fp == NULL) return XCAM_RETURN_ERROR_FILE;

    fwrite(&blockID, sizeof(blockID), 1, pDpRkRaw->fp);
    if (blockSize > 0) fwrite(&blockSize, sizeof(blockSize), 1, pDpRkRaw->fp);
    fflush(pDpRkRaw->fp);

    uint32_t size = blockSize > 0 ? sizeof(blockID) + sizeof(blockSize) : sizeof(blockID);
    pDpRkRaw->dumpTotalSize += size;

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "dump blockId(0x%x), size(%d), offset(0x%x)\n", blockID, size,
                    pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

/*
 * Refer to "Raw file structure" in the header of this file
 */
XCamReturn DumpRkRaw2_dumpRkRawHeader(aiq_DumpRkRaw_t* pDpRkRaw, sint32_t sequence) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint8_t working_mode = 0;

    if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
        pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        working_mode = 3;
    else if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
             pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        working_mode = 2;
    else if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
            RK_AIQ_HDR_IS_SENSOR_BUILTIN(pDpRkRaw->ispInfo.working_mode))
        working_mode = 1;

    struct raw2Header_s rawHeader = {};

    rawHeader.version = RAW_HEADER_VERSION;
    strcpy(rawHeader.snsName, pDpRkRaw->ispInfo.sns_name);
    rawHeader.frameId       = sequence;
    rawHeader.sns_width     = pDpRkRaw->ispInfo.sns_width;
    rawHeader.sns_height    = pDpRkRaw->ispInfo.sns_height;
    rawHeader.bpp           = pDpRkRaw->ispInfo.bpp;
    rawHeader.bayer_fmt     = pDpRkRaw->ispInfo.bayer_fmt;
    rawHeader.working_mode  = working_mode;
    rawHeader.storage_type  = 0;
    rawHeader.stridePerLine = pDpRkRaw->ispInfo.stridePerLine;
    rawHeader.bytesPerLine  = pDpRkRaw->ispInfo.bytesPerLine;
    rawHeader.endian        = 1;

    fwrite(&rawHeader, sizeof(rawHeader), 1, pDpRkRaw->fp);
    fflush(pDpRkRaw->fp);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                    "frame%d: image rect(%dx%d), bpp(%d), bayer_fmt(%d), "
                    "hdr frame number(%d), Storage type(%d), "
                    "line stride(%d), Effective line stride(%d)\n",
                    sequence, pDpRkRaw->ispInfo.sns_width, pDpRkRaw->ispInfo.sns_height,
                    pDpRkRaw->ispInfo.bpp, pDpRkRaw->ispInfo.bayer_fmt, working_mode,
                    rawHeader.storage_type, pDpRkRaw->ispInfo.stridePerLine,
                    pDpRkRaw->ispInfo.bytesPerLine);

    pDpRkRaw->dumpTotalSize += sizeof(rawHeader);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "frame%d: dump rawHeader size(%d), offset(0x%x)\n", sequence,
                    sizeof(rawHeader), pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpRkRawHeaderBlock(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                                  int32_t dev_index) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, RAW_HEADER_BLOCK_ID, sizeof(struct raw2Header_s));
    DumpRkRaw2_dumpRkRawHeader(pDpRkRaw, sequence);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpRkRaw(aiq_DumpRkRaw_t* pDpRkRaw, sint32_t dev_index,
                                       sint32_t sequence, void* userptr, sint32_t size) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    fwrite(userptr, size, 1, pDpRkRaw->fp);
    fflush(pDpRkRaw->fp);

    pDpRkRaw->dumpTotalSize += size;
    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                    "frame%d: dump raw data: dev_index(%d), size(%d), offset(0x%x)\n", sequence,
                    dev_index, size, pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpRkRawBlock(aiq_DumpRkRaw_t* pDpRkRaw, sint32_t dev_index,
                                            uint32_t sequence, unsigned long userptr,
                                            uint32_t bytesused) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOG1_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DumpRkRaw2_dumpRkRawHeaderBlock(pDpRkRaw, sequence, dev_index);

    uint32_t rawBlockId = 0;

    XCAM_STATIC_PROFILING_START(write_raw);
    if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
        pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        rawBlockId = dev_index == 0 ? HDR_S_RAW_BLOCK_ID
                                    : dev_index == 1 ? HDR_M_RAW_BLOCK_ID : HDR_L_RAW_BLOCK_ID;
    else if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
             pDpRkRaw->ispInfo.working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        rawBlockId = dev_index == 0 ? HDR_S_RAW_BLOCK_ID : HDR_M_RAW_BLOCK_ID;
    else if (pDpRkRaw->ispInfo.working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
            RK_AIQ_HDR_IS_SENSOR_BUILTIN(pDpRkRaw->ispInfo.working_mode))
        rawBlockId = NORMAL_RAW_BLOCK_ID;

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, rawBlockId, bytesused);
    DumpRkRaw2_dumpRkRaw(pDpRkRaw, dev_index, sequence, (void*)userptr, bytesused);
    XCAM_STATIC_PROFILING_END(write_raw, 0);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpMetadata(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                          aiq_isp_effect_params_t* ispParams,
                                          aiq_sensor_exp_info_t* expParams) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct raw2metadata_s metaData = {};

    metaData.version = RAW_METADATA_VERSION;
    metaData.frameId = sequence;

    metaData.linearExp.exp_real.integration_time =
        expParams->aecExpInfo.LinearExp.exp_real_params.integration_time;
    metaData.linearExp.exp_real.analog_gain =
        expParams->aecExpInfo.LinearExp.exp_real_params.analog_gain;
    metaData.linearExp.exp_sensor.integration_time =
        expParams->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time;
    metaData.linearExp.exp_sensor.analog_gain =
        expParams->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global;

    metaData.HdrExp[0].exp_real.integration_time =
        expParams->aecExpInfo.HdrExp[2].exp_real_params.integration_time;
    metaData.HdrExp[0].exp_real.analog_gain =
        expParams->aecExpInfo.HdrExp[2].exp_real_params.analog_gain;
    metaData.HdrExp[0].exp_sensor.integration_time =
        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[0].exp_real.analog_gain =
        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global;
    metaData.HdrExp[1].exp_real.integration_time =
        expParams->aecExpInfo.HdrExp[1].exp_real_params.integration_time;
    metaData.HdrExp[1].exp_real.analog_gain =
        expParams->aecExpInfo.HdrExp[1].exp_real_params.analog_gain;
    metaData.HdrExp[1].exp_sensor.integration_time =
        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[1].exp_sensor.analog_gain =
        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global;
    metaData.HdrExp[2].exp_real.integration_time =
        expParams->aecExpInfo.HdrExp[0].exp_real_params.integration_time;
    metaData.HdrExp[2].exp_real.analog_gain =
        expParams->aecExpInfo.HdrExp[0].exp_real_params.analog_gain;
    metaData.HdrExp[2].exp_sensor.integration_time =
        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time;
    metaData.HdrExp[2].exp_sensor.analog_gain =
        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global;

#if defined(ISP_HW_V30)
    metaData.rgain = ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_red;
    metaData.bgain = ispParams->isp_params_v3x[0].others.awb_gain_cfg.gain0_blue;
#endif

    fwrite(&metaData, sizeof(metaData), 1, pDpRkRaw->fp);
    fflush(pDpRkRaw->fp);

    pDpRkRaw->dumpTotalSize += sizeof(metaData);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "frame(%d): dump metaData size(%d), offset(0x%x)\n", sequence,
                    sizeof(metaData), pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpMetadataBlock(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t sequence,
                                               aiq_isp_effect_params_t* ispParams,
                                               aiq_sensor_exp_info_t* expParams,
                                               AiqAfInfoWrapper_t* afParams) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, METADATA_BLOCK_ID, sizeof(struct raw2metadata_s));
    DumpRkRaw2_dumpMetadata(pDpRkRaw, sequence, ispParams, expParams);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpIspRegHeader(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                              uint32_t offset_addr, sint32_t length,
                                              uint32_t sequence) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint32_t regBlockId = 0, regBlockSize = 0;
    if (base_addr == ISP_REGS_BASE) {
        struct raw2ispRegHeader_s ispRegHeader = {};

        ispRegHeader.version     = ISP_REG_VERSION;
        ispRegHeader.frameId     = sequence;
        ispRegHeader.base_addr   = base_addr;
        ispRegHeader.offset_addr = offset_addr;
        ispRegHeader.count       = length;

        fwrite(&ispRegHeader, sizeof(ispRegHeader), 1, pDpRkRaw->fp);
        fflush(pDpRkRaw->fp);

        pDpRkRaw->dumpTotalSize += sizeof(ispRegHeader);
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM,
                        "frame(%d): dump isp reg header base(0x%x), size(%d), offset(0x%x)\n",
                        sequence, base_addr, sizeof(ispRegHeader), pDpRkRaw->dumpTotalSize);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpIspRegHeaderBlock(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                                   uint32_t offset_addr, sint32_t length,
                                                   uint32_t sequence) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uint32_t regBlockId = 0, regBlockSize = 0;
    if (base_addr == ISP_REGS_BASE) {
        regBlockId   = 0xff07;
        regBlockSize = sizeof(struct raw2ispRegHeader_s);
    } else {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Invalid register addr(0x%x)\n", base_addr);
        return XCAM_RETURN_ERROR_MEM;
    }

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, regBlockId, regBlockSize);
    DumpRkRaw2_dumpIspRegHeader(pDpRkRaw, base_addr, offset_addr, length, sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn DumpRkRaw2_dumpIspReg(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                 uint32_t offset_addr, sint32_t length, uint32_t sequence) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sint32_t fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd == -1) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "open /dev/mem failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
    }

    uint8_t* mmap_base = (uint8_t*)mmap64(NULL, length, PROT_READ, MAP_SHARED, fd, base_addr);
    if (mmap_base == (void*)(-1)) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mmap() failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_MEM;
    }

    int8_t ffd = open(pDpRkRaw->raw_name, O_WRONLY | O_APPEND);
    if (ffd < 0) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Failed to open destination file '%s': %s\n",
                        pDpRkRaw->raw_name, strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
    }

    int n = write(ffd, mmap_base + offset_addr, length);
    if (n < 0) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "File write failed: %s\n", strerror(errno));
        return XCAM_RETURN_ERROR_FILE;
    } else if (n != length) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Only wrote %d of %d bytes to file\n", n, length);
        return XCAM_RETURN_ERROR_FILE;
    }

#ifdef PRINT_ISP_REG
    memread_memory(base_addr, mmap_base + offset_addr, 0x100, 4);
#endif

    munmap(mmap_base, length);
    close(ffd);
    close(fd);

    pDpRkRaw->dumpTotalSize += length;

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "frame%d: dump reg(0x%x), size(0x%x), offset(0x%x)\n",
                    sequence, base_addr + offset_addr, length, pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpIspRegBlock(aiq_DumpRkRaw_t* pDpRkRaw, uint32_t base_addr,
                                             uint32_t offset_addr, sint32_t length,
                                             uint32_t sequence) {
#ifndef DUMP_ISP_REG
    return XCAM_RETURN_ERROR_PARAM;
#endif

    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DumpRkRaw2_dumpIspRegHeaderBlock(pDpRkRaw, ISP_REGS_BASE, 0, ISP_REGS_SIZE, sequence);

    uint32_t regBlockId = 0;
    if (base_addr == ISP_REGS_BASE)
        regBlockId = 0xff08;
    else {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "Invalid register addr(0x%x)\n", base_addr);
        return XCAM_RETURN_ERROR_MEM;
    }

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, regBlockId, length);
    DumpRkRaw2_dumpIspReg(pDpRkRaw, base_addr, offset_addr, length, sequence);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpPlatformInfo(aiq_DumpRkRaw_t* pDpRkRaw) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct raw2platform_info_s platform_info = {};

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

    fwrite(&platform_info, sizeof(platform_info), 1, pDpRkRaw->fp);
    fflush(pDpRkRaw->fp);

    pDpRkRaw->dumpTotalSize += sizeof(platform_info);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "dump platform_info size(%d), offset(0x%x)\n",
                    sizeof(platform_info), pDpRkRaw->dumpTotalSize);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn DumpRkRaw2_dumpPlatformInfoBlock(aiq_DumpRkRaw_t* pDpRkRaw) {
    if (pDpRkRaw->fp == NULL || pDpRkRaw->dumpNum <= 0) {
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "fp(%p), dumpNum(%d)!\n", pDpRkRaw->fp, pDpRkRaw->dumpNum);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DumpRkRaw2_dumpBlockHeader(pDpRkRaw, PLATFORM_INFO_BLOCK_ID,
                               sizeof(struct raw2platform_info_s));
    DumpRkRaw2_dumpPlatformInfo(pDpRkRaw);

    return XCAM_RETURN_NO_ERROR;
}

void aiq_dumpRkRaw2_init(aiq_DumpRkRaw_t* pDpRkRaw, int32_t camId) {
    aiq_dumpRkRaw_init(pDpRkRaw, camId);
    pDpRkRaw->dumpRkRawBlock        = DumpRkRaw2_dumpRkRawBlock;
    pDpRkRaw->dumpIspRegBlock       = DumpRkRaw2_dumpIspRegBlock;
    pDpRkRaw->dumpMetadataBlock     = DumpRkRaw2_dumpMetadataBlock;
    pDpRkRaw->dumpBlockHeader       = DumpRkRaw2_dumpBlockHeader;
    pDpRkRaw->dumpIspRegHeaderBlock = DumpRkRaw2_dumpIspRegHeaderBlock;
    pDpRkRaw->dumpPlatformInfoBlock = DumpRkRaw2_dumpPlatformInfoBlock;
}

void aiq_dumpRkRaw2_deinit(aiq_DumpRkRaw_t* pDpRkRaw) { aiq_dumpRkRaw_deinit(pDpRkRaw); }

/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#include <fcntl.h>
#include "aiq_stats_translator.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "hwi_c/aiq_pdafStreamProcUnit.h"
#include "hwi_c/aiq_sensorHw.h"
#include "hwi_c/aiq_CamHwBase.h"

#ifdef __ARM_NEON
#define NEON_OPT
#endif
#ifdef NEON_OPT
#include <arm_neon.h>
#endif

#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

#define DEFAULT_PD_RAW_PATH "/data/pdaf/frm%04u_pdAll.raw"
#define DEFAULT_PD_LRAW_PATH "/data/pdaf/frm%04u_pdLeft.raw"
#define DEFAULT_PD_RRAW_PATH "/data/pdaf/frm%04u_pdRight.raw"

#define MAX_8BITS ((1 << 8) - 1)

void AiqStatsTranslator_setCamPhyId(AiqStatsTranslator_t* pStatsTrans, int phyId) {
    pStatsTrans->mCamPhyId = phyId;
}

void AiqStatsTranslator_setGroupMode(AiqStatsTranslator_t* pStatsTrans, bool bGroup) {
    pStatsTrans->mIsGroupMode = bGroup;
}

void AiqStatsTranslator_setModuleRot(AiqStatsTranslator_t* pStatsTrans, int rot) {
    pStatsTrans->mModuleRotation = rot;
}

void AiqStatsTranslator_setWorkingMode(AiqStatsTranslator_t* pStatsTrans, int working_mode) {
    pStatsTrans->mWorkingMode = working_mode;
}

int AiqStatsTranslator_getWorkingMode(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->mWorkingMode;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetMultiIspMode(AiqStatsTranslator_t* pStatsTrans,
                                                         bool isMultiIsp) {
    pStatsTrans->mIsMultiIsp = isMultiIsp;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetIspUniteMode(AiqStatsTranslator_t* pStatsTrans,
                                                          RkAiqIspUniteMode mode) {
    pStatsTrans->mIspUniteMode = mode;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetPicInfo(AiqStatsTranslator_t* pStatsTrans,
                                                    Rectangle_t* pic_rect) {
    pStatsTrans->pic_rect_ = *pic_rect;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetLeftIspRect(AiqStatsTranslator_t* pStatsTrans,
                                                        Rectangle_t* left_isp_rect) {
    pStatsTrans->left_isp_rect_ = *left_isp_rect;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetRightIspRect(AiqStatsTranslator_t* pStatsTrans,
                                                         Rectangle_t* right_isp_rect) {
    pStatsTrans->right_isp_rect_ = *right_isp_rect;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetBottomLeftIspRect(AiqStatsTranslator_t* pStatsTrans,
                                                              Rectangle_t* bottom_left_isp_rect) {
    pStatsTrans->bottom_left_isp_rect_ = *bottom_left_isp_rect;
    return pStatsTrans;
}

AiqStatsTranslator_t* AiqStatsTranslator_SetBottomRightIspRect(AiqStatsTranslator_t* pStatsTrans,
                                                               Rectangle_t* bottom_right_isp_rect) {
    pStatsTrans->bottom_right_isp_rect_ = *bottom_right_isp_rect;
    return pStatsTrans;
}

bool AiqStatsTranslator_IsMultiIspMode(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->mIsMultiIsp;
}

RkAiqIspUniteMode AiqStatsTranslator_GetIspUniteMode(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->mIspUniteMode;
}

Rectangle_t AiqStatsTranslator_GetPicInfo(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->pic_rect_;
}

Rectangle_t AiqStatsTranslator_GetLeftIspRect(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->left_isp_rect_;
}

Rectangle_t AiqStatsTranslator_GetRightIspRect(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->right_isp_rect_;
}

Rectangle_t AiqStatsTranslator_GetBottomLeftIspRect(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->bottom_left_isp_rect_;
}

Rectangle_t AiqStatsTranslator_GetBottomRightIspRect(AiqStatsTranslator_t* pStatsTrans) {
    return pStatsTrans->bottom_right_isp_rect_;
}

#if RKAIQ_HAVE_PDAF
static bool getFileValue(const char* path, int* pos) {
    const char* delim = " ";
    char buffer[16]   = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGE_AF("read %s failed!", path);
            goto OUT;
        } else {
            char* p = NULL;

            p = strtok(buffer, delim);
            if (p != NULL) {
                *pos = atoi(p);
            }
        }
        close(fp);
        return true;
    }

OUT:
    return false;
}

static inline uint32_t getStrideValue(uint32_t pixelperline, uint32_t bytesperline) {
    uint32_t stride;

#if defined(ISP_HW_V35)
    stride = pixelperline;
#else
    stride = bytesperline >> 1;
#endif

    return stride;
}

XCamReturn AiqStatsTranslator_translatePdafStats(AiqStatsTranslator_t* pStatsTrans,
                                                 const rk_aiq_isp_pdaf_meas_t* pdaf,
                                                 const aiq_VideoBuffer_t* from,
                                                 aiq_stats_base_t* to, bool sns_mirror) {
    XCamReturn ret  = XCAM_RETURN_NO_ERROR;
    void* pdafstats = (void*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
    rk_aiq_isp_pdaf_stats_t* statsInt = (rk_aiq_isp_pdaf_stats_t*)to->_data;
    AiqPdafBuffer_t* pBuf             = (AiqPdafBuffer_t*)from;
    if (pdafstats == NULL || pdaf == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint16_t *pdLData, *pdRData, *pdData;
    uint32_t i, j, pixelperline;
    unsigned short pdWidth;
    unsigned short pdHeight;
    bool dumppdraw    = false;
    uint32_t frame_id = AiqV4l2Buffer_getSequence((AiqV4l2Buffer_t*)from);
    uint32_t stride = pdaf->bytesperline >> 1;

    pdLData = statsInt->pdLData;
    pdRData = statsInt->pdRData;
    pdData  = (uint16_t*)pdafstats;

    // LOGD_AF("%s: frame_id %d, timestamp %lld, pdLData %p, pdRData %p, pdData %p",
    //    __func__, frame_id, AiqV4l2Buffer_getTimestamp((AiqV4l2Buffer_t*)from), pdLData, pdRData,
    //    pdData);

#ifdef PDAF_RAW_DUMP
    {
        FILE* fp;
        char name[64];
        frame_id = AiqV4l2Buffer_getSequence((AiqV4l2Buffer_t*)from) % 10;

        ALOGD("@%s: pdWidthxpdHeight: %dx%d !\n", __FUNCTION__, pdaf->pdWidth, pdaf->pdHeight);
        memset(name, 0, sizeof(name));
        if (frame_id < 3) {
            sprintf(name, DEFAULT_PD_RAW_PATH, frame_id);
            fp = fopen(name, "wb");
            if (fp) {
                fwrite(pdData, pdaf->pdWidth * pdaf->pdHeight, 2, fp);
                fflush(fp);
                fclose(fp);
            } else {
                LOGE_AF("can not write to %s file", name);
            }
        }
    }
#endif

    if (pdaf->pdafSensorType == PDAF_SENSOR_TYPE3) {
        // OV13855
        if (strcasecmp(pdaf->snsName, "OV13855") == 0) {
            pdWidth = pdaf->pdWidth;
            pdHeight = pdaf->pdHeight >> 1;
            pixelperline = pdaf->pdWidth;
            stride = getStrideValue(pixelperline, pdaf->bytesperline);
            for (j = 0; j < 2 * pdHeight; j += 4) {
                for (i = 0; i < pixelperline; i++) {
                    *pdRData++ = pdData[i] >> 6;
                }
                pdData += stride;
                for (i = 0; i < pixelperline; i++) {
                    *pdLData++ = pdData[i] >> 6;
                }
                pdData += stride;

                for (i = 0; i < pixelperline; i++) {
                    *pdLData++ = pdData[i] >> 6;
                }
                pdData += stride;
                for (i = 0; i < pixelperline; i++) {
                    *pdRData++ = pdData[i] >> 6;
                }
                pdData += stride;
            }
        } else {
            if (pdaf->pdLRInDiffLine == 0) {
                pdWidth  = pdaf->pdWidth >> 1;
                pdHeight = pdaf->pdHeight;
                pixelperline = 2 * pdWidth;
                stride = getStrideValue(pixelperline, pdaf->bytesperline);
                for (j = 0; j < pdHeight; j++) {
                    pdData = (uint16_t*)pdafstats + j * stride;
                    for (i = 0; i < pixelperline; i += 2) {
                        *pdLData++ = pdData[i] >> 6;
                        *pdRData++ = pdData[i + 1] >> 6;
                    }
                }
            } else {
                pdWidth      = pdaf->pdWidth;
                pdHeight     = pdaf->pdHeight >> 1;
                pixelperline = pdaf->pdWidth;
                stride = getStrideValue(pixelperline, pdaf->bytesperline);
                for (j = 0; j < 2 * pdHeight; j += 2) {
                    for (i = 0; i < pixelperline; i++) {
                        *pdLData++ = pdData[i] >> 6;
                    }
                    pdData += stride;
                    for (i = 0; i < pixelperline; i++) {
                        *pdRData++ = pdData[i] >> 6;
                    }
                    pdData += stride;
                }
            }
        }
    } else {
        if (pdaf->pdLRInDiffLine == 0) {
            pdWidth  = pdaf->pdWidth >> 1;
            pdHeight = pdaf->pdHeight;

#ifdef NEON_OPT
            uint16x8x2_t vld2_data;
            uint16x8_t vrev_data;
            pixelperline = 2 * pdWidth;
            for (j = 0; j < pdHeight; j++) {
                pdData = (uint16_t*)pdafstats + j * stride;
                for (i = 0; i < pixelperline / 16 * 16; i += 16) {
                    vld2_data = vld2q_u16(pdData);
                    vst1q_u16(pdLData, vld2_data.val[0]);
                    vst1q_u16(pdRData, vld2_data.val[1]);
                    pdLData += 8;
                    pdRData += 8;
                    pdData += 16;
                }

                if (pixelperline % 16) {
                    for (i = 0; i < pixelperline % 16; i += 2) {
                        *pdLData++ = pdData[i];
                        *pdRData++ = pdData[i + 1];
                    }
                }
            }
#else
            pixelperline = 2 * pdWidth;
            for (j = 0; j < pdHeight; j++) {
                pdData = (uint16_t*)pdafstats + j * stride;
                for (i = 0; i < pixelperline; i += 2) {
                    *pdLData++ = pdData[i];
                    *pdRData++ = pdData[i + 1];
                }
            }
#endif
        } else {
            pdWidth      = pdaf->pdWidth;
            pdHeight     = pdaf->pdHeight >> 1;
            pixelperline = pdaf->pdWidth;
            for (j = 0; j < 2 * pdHeight; j += 2) {
                memcpy(pdRData, pdData, pixelperline * sizeof(uint16_t));
                pdData += stride;
                memcpy(pdLData, pdData, pixelperline * sizeof(uint16_t));
                pdData += stride;
                pdLData += pixelperline;
                pdRData += pixelperline;
            }
        }
    }

#ifdef PDAF_RAW_DUMP
    frame_id = AiqV4l2Buffer_getSequence((AiqV4l2Buffer_t*)from) % 10;

    if (frame_id < 3) {
        dumppdraw = true;
    }
#endif

    pStatsTrans->mEnPdDump = false;
#ifndef ANDROID_OS
    char* valueStr = getenv("rkaiq_pddump");
    if (valueStr) {
        pStatsTrans->mEnPdDump = atoi(valueStr) > 0 ? true : false;
    }
#else
    char property_value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("persist.vendor.rkaiq_pddump", property_value, "-1");
    int val = atoi(property_value);
    if (val != -1) pStatsTrans->mEnPdDump = atoi(property_value) > 0 ? true : false;
#endif

    if (pStatsTrans->mEnPdDump) {
        int dump_cnt = 0;
        if (getFileValue("/data/.dump_pd", &dump_cnt) == true) {
            if (dump_cnt > 0) {
                pStatsTrans->mPdafDumpCnt = dump_cnt;
                system("echo 0 > /data/.dump_pd");
            }
        } else {
            pStatsTrans->mPdafDumpCnt = 0;
        }

        if (pStatsTrans->mPdafDumpCnt > 0) {
            frame_id = AiqV4l2Buffer_getSequence((AiqV4l2Buffer_t*)from);

            LOGI_AF("dump pd raw, mPdafDumpCnt %d, frame_id %d", pStatsTrans->mPdafDumpCnt,
                    frame_id);
            pStatsTrans->mPdafDumpCnt--;
            dumppdraw = true;
        }
    }

    if (dumppdraw) {
        FILE* fp;
        char name[64];

        memset(name, 0, sizeof(name));
        sprintf(name, DEFAULT_PD_LRAW_PATH, frame_id);
        fp = fopen(name, "wb");
        if (fp) {
            fwrite(statsInt->pdLData, pdWidth * pdHeight, 2, fp);
            fflush(fp);
            fclose(fp);
        } else {
            LOGE_AF("can not write to %s file", name);
        }

        memset(name, 0, sizeof(name));
        sprintf(name, DEFAULT_PD_RRAW_PATH, frame_id);
        fp = fopen(name, "wb");
        if (fp) {
            fwrite(statsInt->pdRData, pdWidth * pdHeight, 2, fp);
            fflush(fp);
            fclose(fp);
        } else {
            LOGE_AF("can not write to %s file", name);
        }
    }

    to->bValid         = true;
    to->frame_id       = AiqV4l2Buffer_getSequence((AiqV4l2Buffer_t*)from);
    statsInt->pdWidth  = pdWidth;
    statsInt->pdHeight = pdHeight;
    statsInt->pdMirror = sns_mirror;
    statsInt->pdMean   = 0;

    return ret;
}
#endif

XCamReturn AiqStatsTranslator_getParams(AiqStatsTranslator_t* pStatsTrans,
                                        const aiq_VideoBuffer_t* from) {
    AiqV4l2Buffer_t* buf = (AiqV4l2Buffer_t*)from;
#ifdef ISP_HW_V39
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif ISP_HW_V35
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif ISP_HW_V33
    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif ISP_HW_V32_LITE
    struct rkisp32_lite_stat_buffer* stats =
        (struct rkisp32_lite_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif ISP_HW_V32
    struct rkisp32_isp_stat_buffer* stats =
        (struct rkisp32_isp_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif defined(ISP_HW_V30)
    struct rkisp3x_isp_stat_buffer* stats =
        (struct rkisp3x_isp_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif defined(ISP_HW_V21)
    struct rkisp_isp21_stat_buffer* stats =
        (struct rkisp_isp21_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#elif defined(ISP_HW_V20)
    struct rkisp_isp2x_stat_buffer* stats =
        (struct rkisp_isp2x_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr(buf));
#endif

    // TODO: check if needed
    // memset(&ispParams, 0, sizeof(_expParams));

    AiqSensorExpInfo_t* pExp = NULL;
    if (pStatsTrans->mSensorHw)
        pExp = pStatsTrans->mSensorHw->getEffectiveExpParams(pStatsTrans->mSensorHw, stats->frame_id);
    if (!pExp) {
        LOGE("fid:%d fail to get expParams", stats->frame_id);
    } else {
        if (pStatsTrans->_expParams) AIQ_REF_BASE_UNREF(&pStatsTrans->_expParams->_base._ref_base);
        pStatsTrans->_expParams = pExp;
    }

    aiq_isp_effect_params_t* tmp = NULL;
    if (pStatsTrans->mCamHw)
        AiqCamHw_getEffectiveIspParams(pStatsTrans->mCamHw, &tmp, stats->params_id);
    if (!tmp) {
        LOGE("fid:%d fail to get ispParams ,ignore", stats->params_id);
        return XCAM_RETURN_BYPASS;
    } else {
        if (pStatsTrans->_ispParams) AIQ_REF_BASE_UNREF(&pStatsTrans->_ispParams->_ref_base);
        pStatsTrans->_ispParams = tmp;
    }

    return XCAM_RETURN_NO_ERROR;
}

void AiqStatsTranslator_releaseParams(AiqStatsTranslator_t* pStatsTrans) {
    if (pStatsTrans->_expParams) {
        AIQ_REF_BASE_UNREF(&pStatsTrans->_expParams->_base._ref_base);
        pStatsTrans->_expParams = NULL;
    }

    if (pStatsTrans->_ispParams) {
        AIQ_REF_BASE_UNREF(&pStatsTrans->_ispParams->_ref_base);
        pStatsTrans->_ispParams = NULL;
    }
}

void AiqStatsTranslator_setAeAlgoStatsCfg(AiqStatsTranslator_t * pStatsTrans,
										  const RkAiqSetStatsCfg* cfg) {
	pStatsTrans->_aeAlgoStatsCfg.UpdateStats    = cfg->UpdateStats;
	pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel = cfg->RawStatsChnSel;
	pStatsTrans->_aeAlgoStatsCfg.YRangeMode     = cfg->YRangeMode;
	memcpy(pStatsTrans->_aeAlgoStatsCfg.BigWeight, cfg->BigWeight, RAWHISTBIG_WIN_NUM);
	memcpy(pStatsTrans->_aeAlgoStatsCfg.LiteWeight, cfg->LiteWeight, RAWHISTLITE_WIN_NUM);
}

bool AiqStatsTranslator_getAeStatsRunFlag(AiqStatsTranslator_t * pStatsTrans,
										  uint16_t * HistMean) {
	bool run_flag        = false;
	int FrameNum         = 0;
	int cur_working_mode = pStatsTrans->mWorkingMode;

	if (cur_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
		FrameNum = 1;
	else if (cur_working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
			 cur_working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)
		FrameNum = 2;
	else if (cur_working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)
		FrameNum = 3;
	else
		LOGE("wrong working mode=%d\n", cur_working_mode);

	for (int i = 0; i < FrameNum; i++) {
		if (pStatsTrans->_lastHistMean[i] != HistMean[i]) {
			run_flag                      = true;
			pStatsTrans->_lastHistMean[i] = HistMean[i];
		}
	}

	return run_flag;
}

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

#include "hwi_c/aiq_fake_sensor.h"

#include "common/code_to_pixel_format.h"
#include "xcore_c/aiq_fake_v4l2_device.h"

RKAIQ_BEGIN_DECLARE

#define FAKECAM_SUBM (0x40)

// TODO: _get_selection and _getFormat can't access AiqFakeSensorHw_t now
static int g_width;
static int g_height;
static uint32_t g_fmt_code;

struct CTimer_s {
    pthread_t thread_timer;
    long m_second;
    long m_microsecond;
    AiqFakeSensorHw_t* _dev;
};

static uint32_t rk_format_to_media_format(rk_aiq_format_t format) {
    uint32_t pixelformat = -1;
    switch (format) {
    case RK_PIX_FMT_SBGGR8:
        pixelformat = MEDIA_BUS_FMT_SBGGR8_1X8;
        break;
    case RK_PIX_FMT_SRGGB8:
        pixelformat = MEDIA_BUS_FMT_SRGGB8_1X8;
        break;
    case RK_PIX_FMT_SGBRG8:
        pixelformat = MEDIA_BUS_FMT_SGBRG8_1X8;
        break;
    case RK_PIX_FMT_SGRBG8:
        pixelformat = MEDIA_BUS_FMT_SGRBG8_1X8;
        break;
    case RK_PIX_FMT_SBGGR10:
        pixelformat = MEDIA_BUS_FMT_SBGGR10_1X10;
        break;
    case RK_PIX_FMT_SRGGB10:
        pixelformat = MEDIA_BUS_FMT_SRGGB10_1X10;
        break;
    case RK_PIX_FMT_SGBRG10:
        pixelformat = MEDIA_BUS_FMT_SGBRG10_1X10;
        break;
    case RK_PIX_FMT_SGRBG10:
        pixelformat = MEDIA_BUS_FMT_SGRBG10_1X10;
        break;
    case RK_PIX_FMT_SBGGR12:
        pixelformat = MEDIA_BUS_FMT_SBGGR12_1X12;
        break;
    case RK_PIX_FMT_SRGGB12:
        pixelformat = MEDIA_BUS_FMT_SRGGB12_1X12;
        break;
    case RK_PIX_FMT_SGBRG12:
        pixelformat = MEDIA_BUS_FMT_SGBRG12_1X12;
        break;
    case RK_PIX_FMT_SGRBG12:
        pixelformat = MEDIA_BUS_FMT_SGRBG12_1X12;
        break;
    case RK_PIX_FMT_SBGGR14:
        pixelformat = MEDIA_BUS_FMT_SBGGR14_1X14;
        break;
    case RK_PIX_FMT_SRGGB14:
        pixelformat = MEDIA_BUS_FMT_SRGGB14_1X14;
        break;
    case RK_PIX_FMT_SGBRG14:
        pixelformat = MEDIA_BUS_FMT_SGBRG14_1X14;
        break;
    case RK_PIX_FMT_SGRBG14:
        pixelformat = MEDIA_BUS_FMT_SGRBG14_1X14;
        break;
    case RK_PIX_FMT_SBGGR16:
        pixelformat = MEDIA_BUS_FMT_SBGGR16_1X16;
        break;
    case RK_PIX_FMT_SGBRG16:
        pixelformat = MEDIA_BUS_FMT_SGBRG16_1X16;
        break;
    case RK_PIX_FMT_SGRBG16:
        pixelformat = MEDIA_BUS_FMT_SGRBG16_1X16;
        break;
    case RK_PIX_FMT_SRGGB16:
        pixelformat = MEDIA_BUS_FMT_SRGGB16_1X16;
        break;
    default:
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "%s no support format: %d\n", __func__, format);
    }
    return pixelformat;
}

static void CTimer_init(CTimer_t* pTimer, AiqFakeSensorHw_t* pFakeSns) {
    aiq_memset(pTimer, 0, sizeof(pTimer));
    pTimer->_dev = pFakeSns;
}

static void CTimer_deinit(CTimer_t* pTimer) {}

static void CTimer_SetTimer(CTimer_t* pTimer, long second, long microsecond) {
    pTimer->m_second      = second;
    pTimer->m_microsecond = microsecond;
}

static void _OnTimer(CTimer_t* pTimer) {
    AiqFakeV4l2Device_t* fake_v4l2_dev = NULL;
    AiqSensorHw_t* pSnsHw              = &pTimer->_dev->_base;

    ENTER_CAMHW_FUNCTION();
    if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
            RK_AIQ_HDR_IS_SENSOR_BUILTIN(pSnsHw->_working_mode)) {
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[0]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
    } else if (pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[0]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[1]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
    } else if (pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
               pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[0]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[1]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pTimer->_dev->_mipi_tx_dev[2]);
        (*fake_v4l2_dev->on_timer_proc)(fake_v4l2_dev);
    }
    EXIT_XCORE_FUNCTION();
}

static void _thread_proc(CTimer_t* pTimer) {
    while (true) {
        struct timeval tempval;
        _OnTimer(pTimer);
#ifndef __ANDROID__
        pthread_testcancel();
#endif
        tempval.tv_sec  = pTimer->m_second;
        tempval.tv_usec = pTimer->m_microsecond;
        select(0, NULL, NULL, NULL, &tempval);
    }
}

static void* _OnTimer_stub(void* p) {
    _thread_proc((CTimer_t*)p);
    return NULL;
}

static void CTimer_StartTimer(CTimer_t* pTimer) {
    pthread_create(&pTimer->thread_timer, NULL, _OnTimer_stub, pTimer);
}

static void CTimer_StopTimer(CTimer_t* pTimer) {
#ifndef __ANDROID__
    pthread_cancel(pTimer->thread_timer);
#endif
    pthread_join(pTimer->thread_timer, NULL);
}

static int _get_blank(rk_aiq_exposure_sensor_descriptor* sns_des) {
    sns_des->pixel_periods_per_line = sns_des->sensor_output_width;
    sns_des->line_periods_per_field = sns_des->sensor_output_height;
    return 0;
}

static int _get_pixel(rk_aiq_exposure_sensor_descriptor* sns_des) {
    sns_des->pixel_clock_freq_mhz = 600.0f;
    return 0;
}

static int _get_sensor_fps(float* fps) {
    *fps = 25.0f;

    return 0;
}

XCamReturn _get_sensor_desc(AiqFakeSensorHw_t* pFakeSnsHw,
                            rk_aiq_exposure_sensor_descriptor* sns_des) {
    sns_des->sensor_output_width  = pFakeSnsHw->_width;
    sns_des->sensor_output_height = pFakeSnsHw->_height;
    sns_des->sensor_pixelformat   = get_v4l2_pixelformat(pFakeSnsHw->_fmt_code);

    return XCAM_RETURN_NO_ERROR;
}

static int _get_exposure_range(rk_aiq_exposure_sensor_descriptor* sns_des) {
    sns_des->coarse_integration_time_min        = 1;
    sns_des->coarse_integration_time_max_margin = 10;
    return 0;
}

static int _get_nr_switch(rk_aiq_sensor_nr_switch_t* nr_switch) {
    nr_switch->valid      = false;
    nr_switch->direct     = 0;
    nr_switch->up_thres   = 0;
    nr_switch->down_thres = 0;
    nr_switch->div_coeff  = 0;

    return 0;
}

static int _get_dcg_ratio(rk_aiq_sensor_dcg_ratio_t* dcg_ratio) {
    dcg_ratio->valid     = false;
    dcg_ratio->integer   = 0;
    dcg_ratio->decimal   = 0;
    dcg_ratio->div_coeff = 0;

    return 0;
}

static XCamReturn _get_sensor_descriptor(AiqSensorHw_t* pSnsHw,
        rk_aiq_exposure_sensor_descriptor* sns_des) {
    AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pSnsHw;
    float fps                     = 0;

    memset(sns_des, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (_get_sensor_desc((AiqFakeSensorHw_t*)pSnsHw, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (_get_blank(sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    /*
     * pixel rate is not equal to pclk sometimes
     * prefer to use pclk = ppl * lpp * fps
     */
    if (_get_sensor_fps(&fps) == 0)
        sns_des->pixel_clock_freq_mhz = (float)(sns_des->pixel_periods_per_line) *
                                        sns_des->line_periods_per_field * fps / 1000000.0;
    else if (_get_pixel(sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (_get_exposure_range(sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (_get_nr_switch(&sns_des->nr_switch)) {
        // do nothing;
    }
    if (_get_dcg_ratio(&sns_des->dcg_ratio)) {
        // do nothing;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _setExposureParams(AiqSensorHw_t* pBaseSns, AiqAecExpInfoWrapper_t* expPar) {
    AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pBaseSns;
    AiqSensorExpInfo_t* pSnsExp   = NULL;
    AiqMapItem_t* pItem           = NULL;
    int ret                       = 0;

    ENTER_CAMHW_FUNCTION();

    if (pBaseSns->_first && !pFakeSnsHw->use_rkrawstream) {
        AiqAecExpInfoWrapper_t* aec_exp = expPar;
        AiqSensorExpInfo_t* pSnsExp     = NULL;
        if (aec_exp->ae_proc_res_rk.exp_set_cnt > 0) {
            int lastIdx = aec_exp->ae_proc_res_rk.exp_set_cnt - 1;

            if (aiqPool_freeNums(pBaseSns->_expParamsPool)) {
                AiqPoolItem_t* pItem = aiqPool_getFree(pBaseSns->_expParamsPool);
                pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
            } else {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
                return XCAM_RETURN_ERROR_MEM;
            }

            pSnsExp->aecExpInfo.LinearExp = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].LinearExp;
            pSnsExp->aecExpInfo.HdrExp[0] = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].HdrExp[0];
            pSnsExp->aecExpInfo.HdrExp[1] = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].HdrExp[1];
            pSnsExp->aecExpInfo.HdrExp[2] = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].HdrExp[2];
            pSnsExp->aecExpInfo.frame_length_lines =
                aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].frame_length_lines;
            pSnsExp->aecExpInfo.line_length_pixels =
                aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].line_length_pixels;
            pSnsExp->aecExpInfo.pixel_clock_freq_mhz =
                aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx].pixel_clock_freq_mhz;
        }

        pItem = aiqMap_insert(pBaseSns->_effecting_exp_map, (void*)0, &pSnsExp);
        if (!pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            return XCAM_RETURN_ERROR_FAILED;
        }
        pBaseSns->_first = false;

        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "exp-sync: first set exp, add id[0] to the effected exp map");
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _getSensorModeData(AiqSensorHw_t* pBaseSns, const char* sns_ent_name,
                                     rk_aiq_exposure_sensor_descriptor* sns_des) {
    rk_aiq_exposure_sensor_descriptor sensor_desc;
    _get_sensor_descriptor(pBaseSns, &sensor_desc);

    pBaseSns->_sns_entity_name                  = sns_ent_name;
    sns_des->coarse_integration_time_min        = sensor_desc.coarse_integration_time_min;
    sns_des->coarse_integration_time_max_margin = sensor_desc.coarse_integration_time_max_margin;
    sns_des->fine_integration_time_min          = sensor_desc.fine_integration_time_min;
    sns_des->fine_integration_time_max_margin   = sensor_desc.fine_integration_time_max_margin;

    sns_des->frame_length_lines   = sensor_desc.line_periods_per_field;
    sns_des->line_length_pck      = sensor_desc.pixel_periods_per_line;
    sns_des->vt_pix_clk_freq_hz   = sensor_desc.pixel_clock_freq_mhz /*  * 1000000 */;
    sns_des->pixel_clock_freq_mhz = sensor_desc.pixel_clock_freq_mhz /* * 1000000 */;

    // add nr_switch
    sns_des->nr_switch = sensor_desc.nr_switch;
    sns_des->dcg_ratio = sensor_desc.dcg_ratio;

    sns_des->sensor_output_width  = sensor_desc.sensor_output_width;
    sns_des->sensor_output_height = sensor_desc.sensor_output_height;
    sns_des->sensor_pixelformat   = sensor_desc.sensor_pixelformat;

    LOGD_CAMHW_SUBM(FAKECAM_SUBM, "vts-hts-pclk: %d-%d-%d-%f, rect: [%dx%d]\n",
                    sns_des->frame_length_lines, sns_des->line_length_pck,
                    sns_des->vt_pix_clk_freq_hz, sns_des->pixel_clock_freq_mhz,
                    sns_des->sensor_output_width, sns_des->sensor_output_height);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handle_sof(AiqSensorHw_t* pSnsHw, int64_t time, uint32_t frameid) {
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setExpMode(AiqSensorHw_t* pSnsHw, uint32_t mode) {
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _set_working_mode(AiqSensorHw_t* pSnsHw, int mode) {
    __u32 hdr_mode = NO_HDR;

    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        hdr_mode = NO_HDR;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        hdr_mode = HDR_X2;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        hdr_mode = HDR_X3;
    } else if (RK_AIQ_HDR_IS_SENSOR_BUILTIN(mode)) {
        hdr_mode = HDR_CIS_MERGE;
    } else {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "failed to set hdr mode to %d", mode);
        return XCAM_RETURN_ERROR_FAILED;
    }

    pSnsHw->_working_mode = mode;

    LOGD_CAMHW_SUBM(FAKECAM_SUBM, "%s _working_mode: %d\n", __func__, pSnsHw->_working_mode);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _set_mirror_flip(AiqSensorHw_t* pSnsHw, bool mirror, bool flip,
                                   int32_t* skip_frame_sequence) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _get_mirror_flip(AiqSensorHw_t* pSnsHw, bool* mirror, bool* flip) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _set_exp_delay_info(AiqSensorHw_t* pSnsHw, int time_delay, int gain_delay,
                                      int hcg_lcg_mode_delay) {
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _start(AiqSensorHw_t* pSnsHw, bool prepared) {
    AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pSnsHw;
    AiqV4l2Device_t* v4l2_dev     = (AiqV4l2Device_t*)(pSnsHw->mSd);

    ENTER_CAMHW_FUNCTION();

    (*v4l2_dev->start)(v4l2_dev, prepared);

    if (!pFakeSnsHw->use_rkrawstream) {
        CTimer_SetTimer(pFakeSnsHw->_timer, 0, 5000);
        CTimer_StartTimer(pFakeSnsHw->_timer);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _stop(AiqSensorHw_t* pSnsHw) {
    AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pSnsHw;
    AiqV4l2Device_t* v4l2_dev     = (AiqV4l2Device_t*)(pSnsHw->mSd);

    ENTER_CAMHW_FUNCTION();
    if (!pFakeSnsHw->use_rkrawstream) {
        CTimer_StopTimer(pFakeSnsHw->_timer);
        aiqList_reset(pFakeSnsHw->_vbuf_list);
        _SensorHw_stop(pSnsHw);
    }
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _get_selection(AiqV4l2SubDevice_t* v4l2_dev, int pad, uint32_t target,
                                 struct v4l2_subdev_selection* select) {
    // XCAM_ASSERT (is_opened());

    return XCAM_RETURN_ERROR_IOCTL;
}

static XCamReturn _getFormat(AiqV4l2SubDevice_t* v4l2_dev, struct v4l2_subdev_format* aFormat) {
    // TODO: can't cast directly, because mSd in AiqSensorHw_t is a point
    // AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pSnsHw;

    ENTER_CAMHW_FUNCTION();
    aFormat->format.width      = g_width;
    aFormat->format.height     = g_height;
    aFormat->format.code       = g_fmt_code;
    aFormat->format.field      = V4L2_FIELD_NONE;
    aFormat->format.colorspace = V4L2_COLORSPACE_470_SYSTEM_M;
    LOGD_CAMHW_SUBM(FAKECAM_SUBM,
                    "pad: %d, which: %d, width: %d, "
                    "height: %d, format: 0x%x, field: %d, color space: %d",
                    aFormat->pad, aFormat->which, aFormat->format.width, aFormat->format.height,
                    aFormat->format.code, aFormat->format.field, aFormat->format.colorspace);
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _prepare(AiqSensorHw_t* pSnsHw, rk_aiq_raw_prop_t* prop) {
    AiqFakeSensorHw_t* pFakeSnsHw = (AiqFakeSensorHw_t*)pSnsHw;

    ENTER_CAMHW_FUNCTION();
    g_width = pFakeSnsHw->_width = prop->frame_width;
    g_height = pFakeSnsHw->_height = prop->frame_height;
    g_fmt_code = pFakeSnsHw->_fmt_code = rk_format_to_media_format(prop->format);
    pFakeSnsHw->_rawbuf_type           = prop->rawbuf_type;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _enqueue_rawbuffer(AiqFakeSensorHw_t* pFakeSns, struct rk_aiq_vbuf* vbuf,
                                     bool sync) {
    int max_count                      = 0;
    AiqFakeV4l2Device_t* fake_v4l2_dev = NULL;
    AiqSensorHw_t* pSnsHw              = (AiqSensorHw_t*)pFakeSns;
    int fid                            = -1;
    bool exp_val_valid                 = true;
    AiqSensorExpInfo_t* pSnsExp        = NULL;

    ENTER_CAMHW_FUNCTION();
    if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
            RK_AIQ_HDR_IS_SENSOR_BUILTIN(pSnsHw->_working_mode)) {
        max_count = 1;
    } else if (pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        max_count = 2;
    } else if (pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
               pSnsHw->_working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        max_count = 3;
    }

    if (pSnsHw->_frame_sequence != (uint32_t)(-1) &&
            vbuf->buf_info[0].frame_id != pSnsHw->_frame_sequence + 1) {
        LOGW_CAMHW_SUBM(FAKECAM_SUBM, "enqueue frame id(%u) isn't contiguous, change the id to %u",
                        vbuf->buf_info[0].frame_id, pSnsHw->_frame_sequence + 1);
        vbuf->buf_info[0].frame_id = ++pSnsHw->_frame_sequence;
    } else
        pSnsHw->_frame_sequence = vbuf->buf_info[0].frame_id;

    vbuf->buf_info[1].frame_id = vbuf->buf_info[0].frame_id;
    vbuf->buf_info[2].frame_id = vbuf->buf_info[0].frame_id;

    for (int i = 0; i < max_count; i++) {
        fake_v4l2_dev = (AiqFakeV4l2Device_t*)(pFakeSns->_mipi_tx_dev[i]);
        (*fake_v4l2_dev->enqueue_rawbuffer)(fake_v4l2_dev, &vbuf->buf_info[i]);
    }
    aiqMutex_lock(&pSnsHw->_mutex);
    aiqList_push(pFakeSns->_vbuf_list, vbuf);

    while (aiqMap_size(pSnsHw->_effecting_exp_map) > 4) {
        AiqMapItem_t* pItem = aiqMap_begin(pSnsHw->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
        }
    }

    fid = vbuf->buf_info[0].frame_id;
    // check valid firstly
    exp_val_valid = true;

#if 1
    // check float exposure, not reg value
    if (fabs(vbuf->buf_info[0].exp_time - 0.0f) < FLT_EPSILON ||
            fabs(vbuf->buf_info[0].exp_gain - 0.0f) < FLT_EPSILON)
        exp_val_valid = false;

    if (exp_val_valid &&
            (RK_AIQ_HDR_GET_WORKING_MODE(pSnsHw->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2)) {
        if (fabs(vbuf->buf_info[1].exp_time - 0.0f) < FLT_EPSILON ||
                fabs(vbuf->buf_info[1].exp_gain - 0.0f) < FLT_EPSILON)
            exp_val_valid = false;
    }

    if (exp_val_valid &&
            (RK_AIQ_HDR_GET_WORKING_MODE(pSnsHw->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3)) {
        if (fabs(vbuf->buf_info[2].exp_time - 0.0f) < FLT_EPSILON ||
                fabs(vbuf->buf_info[2].exp_gain - 0.0f) < FLT_EPSILON)
            exp_val_valid = false;
    }

#else
    if ((vbuf->buf_info[0].exp_gain_reg) == 0) exp_val_valid = false;

    if (exp_val_valid &&
            (RK_AIQ_HDR_GET_WORKING_MODE(_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2)) {
        if (vbuf->buf_info[1].exp_gain_reg == 0) exp_val_valid = false;
    }

    if (exp_val_valid &&
            (RK_AIQ_HDR_GET_WORKING_MODE(_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3)) {
        if (vbuf->buf_info[2].exp_gain_reg == 0) exp_val_valid = false;
    }
#endif

    if (exp_val_valid) {
        if (aiqPool_freeNums(pSnsHw->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSnsHw->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSnsHw->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global =
            vbuf->buf_info[0].exp_gain_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time =
            vbuf->buf_info[0].exp_time_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.analog_gain      = vbuf->buf_info[0].exp_gain;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.cis_total_gain   = vbuf->buf_info[0].exp_gain;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.integration_time = vbuf->buf_info[0].exp_time;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.isp_dgain             = vbuf->buf_info[0].exp_ispdgain;

        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global =
            vbuf->buf_info[2].exp_gain_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time =
            vbuf->buf_info[2].exp_time_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain      = vbuf->buf_info[2].exp_gain;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.integration_time = vbuf->buf_info[2].exp_time;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain             = vbuf->buf_info[2].exp_ispdgain;

        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global =
            vbuf->buf_info[1].exp_gain_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time =
            vbuf->buf_info[1].exp_time_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain      = vbuf->buf_info[1].exp_gain;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.integration_time = vbuf->buf_info[1].exp_time;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain             = vbuf->buf_info[1].exp_ispdgain;

        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global =
            vbuf->buf_info[0].exp_gain_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time =
            vbuf->buf_info[0].exp_time_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain      = vbuf->buf_info[0].exp_gain;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.integration_time = vbuf->buf_info[0].exp_time;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain             = vbuf->buf_info[0].exp_ispdgain;

        aiqMap_insert(pSnsHw->_effecting_exp_map, (void*)(intptr_t)fid, &pSnsExp);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "add id[%d] to the effected exp map", fid);
    } else {
        LOGW_CAMHW_SUBM(FAKECAM_SUBM, "invalid expo info of fid %d", fid);
    }

    aiqMutex_unlock(&pSnsHw->_mutex);

    if (sync) {
        pFakeSns->_need_sync = sync;
        if (aiqCond_timedWait(&pFakeSns->_sync_cond, &pFakeSns->_sync_mutex, 5000000) != 0) {
            LOGE_CAMHW_SUBM(FAKECAM_SUBM, "wait raw buffer process done timeout");
            return XCAM_RETURN_ERROR_TIMEOUT;
        }
    }
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _register_rawdata_callback(AiqFakeSensorHw_t* pFakeSnsHw,
        void (*callback)(void*)) {
    ENTER_XCORE_FUNCTION();
    XCamReturn ret    = XCAM_RETURN_NO_ERROR;
    pFakeSnsHw->pFunc = callback;
    EXIT_XCORE_FUNCTION();
    return ret;
}

typedef struct vbuflist_cb_args_s {
    int raw_type;
    AiqV4l2Buffer_t* buf;
    AiqListItem_t* item;
    int dev_idx;
} vbuflist_cb_args_t;

static int _vbuf_list_cb(AiqListItem_t* item, void* args) {
    vbuflist_cb_args_t* cb_args = (vbuflist_cb_args_t*)args;
    AiqV4l2Buffer_t* v4l2_buf   = cb_args->buf;
    int raw_type                = cb_args->raw_type;
    int dev_idx                 = cb_args->dev_idx;
    struct rk_aiq_vbuf* buf     = (struct rk_aiq_vbuf*)(item->_pData);
    int ret                     = 0;

    if (raw_type == RK_AIQ_RAW_DATA) {
        uintptr_t ptr = (uintptr_t)AiqV4l2Buffer_getReserved(v4l2_buf);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "rawbuf_type(data): %p vs 0x%x",
                        buf->buf_info[dev_idx].data_addr, ptr);
        if (buf->buf_info[dev_idx].data_addr == (uint8_t*)ptr) {
            buf->buf_info[dev_idx].valid = false;
            cb_args->item                = item;
            goto out;
        }
    } else if (raw_type == RK_AIQ_RAW_ADDR) {
        uintptr_t ptr = AiqV4l2Buffer_getExpbufUsrptr(v4l2_buf);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "rawbuf_type(addr): %p vs 0x%lx",
                        buf->buf_info[dev_idx].data_addr, ptr);
        if (buf->buf_info[dev_idx].data_addr == (uint8_t*)ptr) {
            buf->buf_info[dev_idx].valid = false;
            cb_args->item                = item;
            goto out;
        }
    } else if (raw_type == RK_AIQ_RAW_FD) {
        uint32_t buf_fd = AiqV4l2Buffer_getExpbufFd(v4l2_buf);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "rawbuf_type(fd): %d vs %d", buf->buf_info[dev_idx].data_fd,
                        buf_fd);
        if (buf->buf_info[dev_idx].data_fd == buf_fd) {
            buf->buf_info[dev_idx].valid = false;
            cb_args->item                = item;
            goto out;
        }
    } else if (raw_type == RK_AIQ_RAW_FILE) {
        uintptr_t ptr = AiqV4l2Buffer_getExpbufUsrptr(v4l2_buf);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "rawbuf_type(file): %p vs %p",
                        buf->buf_info[dev_idx].data_addr, (uint8_t*)ptr);
        if (buf->buf_info[dev_idx].data_addr == (uint8_t*)ptr) {
            buf->buf_info[dev_idx].valid = false;
            cb_args->item                = item;
            goto out;
        }
    } else {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "raw buf type is wrong:0x%x", raw_type);
        ret = -1;
        goto out;
    }
out:
    return ret;
}

static XCamReturn _on_dqueue(AiqFakeSensorHw_t* pFakeSns, int dev_idx, AiqV4l2Buffer_t* v4l2_buf) {
    AiqSensorHw_t* pSnsHw    = (AiqSensorHw_t*)pFakeSns;
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqListItem_t* next_item = NULL;

    ENTER_XCORE_FUNCTION();

    aiqMutex_lock(&pSnsHw->_mutex);

    if (aiqList_size(pFakeSns->_vbuf_list)) {
        vbuflist_cb_args_t vb_args;
        int ret1         = 0;
        vb_args.raw_type = pFakeSns->_rawbuf_type;
        vb_args.buf      = v4l2_buf;
        vb_args.item     = NULL;
        vb_args.dev_idx  = dev_idx;

        ret1 = aiqList_foreach(pFakeSns->_vbuf_list, _vbuf_list_cb, &vb_args);

        if (ret1 < 0) {
            aiqMutex_unlock(&pSnsHw->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        } else if (ret1 == 1) {
            // have raw data
            next_item = aiqList_get_item(pFakeSns->_vbuf_list, vb_args.item);
        } else {
            // get first
            next_item = aiqList_get_item(pFakeSns->_vbuf_list, NULL);
        }

        if (next_item) {
            struct rk_aiq_vbuf* buf = (struct rk_aiq_vbuf*)(next_item->_pData);

            switch (pSnsHw->_working_mode) {
            case RK_AIQ_WORKING_MODE_NORMAL:
            case RK_AIQ_ISP_HDR_MODE_2_BUILTIN:
            case RK_AIQ_ISP_HDR_MODE_3_BUILTIN:
                if (!buf->buf_info[0].valid) {
                    goto out;
                }
                break;
            case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
            case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
                if (!buf->buf_info[0].valid && !buf->buf_info[1].valid) {
                    goto out;
                }
                break;
            case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
            case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
                if (!buf->buf_info[0].valid && !buf->buf_info[1].valid &&
                        !buf->buf_info[2].valid) {
                    goto out;
                }
                break;
            }
        }
    }
    EXIT_XCORE_FUNCTION();
    aiqMutex_unlock(&pSnsHw->_mutex);
    return ret;
out:
    if (pFakeSns->_need_sync) {
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "give off signal");
        aiqCond_signal(&pFakeSns->_sync_cond);
    } else {
        if (pFakeSns->pFunc && next_item) {
            struct rk_aiq_vbuf* buf = (struct rk_aiq_vbuf*)(next_item->_pData);
            (*pFakeSns->pFunc)(buf->base_addr);
        }
    }
    aiqList_erase_item(pFakeSns->_vbuf_list, next_item);

    EXIT_XCORE_FUNCTION();
    aiqMutex_unlock(&pSnsHw->_mutex);
    return ret;
}

static XCamReturn _set_mipi_tx_devs(AiqFakeSensorHw_t* pFakeSns, AiqV4l2Device_t* mipi_tx_devs[3]) {
    pFakeSns->_mipi_tx_dev[0] = mipi_tx_devs[0];
    pFakeSns->_mipi_tx_dev[1] = mipi_tx_devs[1];
    pFakeSns->_mipi_tx_dev[2] = mipi_tx_devs[2];
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
_set_fake_sensor_format(AiqFakeSensorHw_t* pFakeSns,
                        int width, int height, uint32_t fmt_core)
{
    ENTER_CAMHW_FUNCTION();
    pFakeSns->_width = width;
    pFakeSns->_height = height;
    pFakeSns->_fmt_code = fmt_core;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void AiqFakeSensorHw_init(AiqFakeSensorHw_t* pFakeSnsHw, const char* name, int cid) {
    aiq_memset(pFakeSnsHw, 0, sizeof(AiqFakeSensorHw_t));
    AiqSensorHw_t* pSnsHw   = (AiqSensorHw_t*)pFakeSnsHw;
    AiqV4l2SubDevice_t* pSd = pSnsHw->mSd;

    AiqSensorHw_init(pSnsHw, name, cid);
    // re-alloc fake subdev
    if (pSnsHw->mSd) {
        AiqV4l2SubDevice_deinit(pSnsHw->mSd);
        aiq_free(pSnsHw->mSd);
        pSnsHw->mSd = (AiqV4l2SubDevice_t*)aiq_malloc(sizeof(AiqFakeV4l2Device_t));
        AiqFakeV4l2Device_init((AiqFakeV4l2Device_t*)(pSnsHw->mSd), name);
    }

    aiqMutex_init(&pFakeSnsHw->_sync_mutex);
    aiqCond_init(&pFakeSnsHw->_sync_cond);

    pSnsHw->start                         = _start;
    pSnsHw->stop                          = _stop;
    pSnsHw->setExposureParams             = _setExposureParams;
    pSnsHw->getSensorModeData             = _getSensorModeData;
    pSnsHw->handle_sof                    = _handle_sof;
    pSnsHw->get_sensor_descriptor         = _get_sensor_descriptor;
    pSnsHw->set_working_mode              = _set_working_mode;
    pSnsHw->set_exp_delay_info            = _set_exp_delay_info;
    pSnsHw->set_mirror_flip               = _set_mirror_flip;
    pSnsHw->get_mirror_flip               = _get_mirror_flip;
    pSnsHw->mSd->get_selection            = _get_selection;
    pSnsHw->mSd->getFormat                = _getFormat;
    pFakeSnsHw->prepare                   = _prepare;
    pFakeSnsHw->set_mipi_tx_devs          = _set_mipi_tx_devs;
    pFakeSnsHw->enqueue_rawbuffer         = _enqueue_rawbuffer;
    pFakeSnsHw->on_dqueue                 = _on_dqueue;
    pFakeSnsHw->set_fake_sensor_format    = _set_fake_sensor_format;
    pFakeSnsHw->register_rawdata_callback = _register_rawdata_callback;
    pSnsHw->set_exposure_mode     = _SensorHw_setExpMode;
#if RKAIQ_HAVE_DUMPSYS
    pSnsHw->dump = NULL;
#endif
    {
        // init list
        AiqListConfig_t vBufListCfg;
        vBufListCfg._name      = "vbuf_list";
        vBufListCfg._item_nums = 10;
        vBufListCfg._item_size = sizeof(struct rk_aiq_vbuf);
        pFakeSnsHw->_vbuf_list = aiqList_init(&vBufListCfg);
        if (!pFakeSnsHw->_vbuf_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "init %s error", vBufListCfg._name);
    }
    pFakeSnsHw->_timer = aiq_mallocz(sizeof(CTimer_t));
    CTimer_init(pFakeSnsHw->_timer, pFakeSnsHw);
}

void AiqFakeSensorHw_deinit(AiqFakeSensorHw_t* pFakeSnsHw) {
    AiqSensorHw_t* pSnsHw = (AiqSensorHw_t*)pFakeSnsHw;

    CTimer_deinit(pFakeSnsHw->_timer);
    if (pFakeSnsHw->_timer) free(pFakeSnsHw->_timer);

    AiqFakeV4l2Device_deinit((AiqFakeV4l2Device_t*)(pSnsHw->mSd));
    AiqSensorHw_deinit(pSnsHw);
    aiqMutex_deInit(&pFakeSnsHw->_sync_mutex);
    aiqCond_deInit(&pFakeSnsHw->_sync_cond);
}

RKAIQ_END_DECLARE

/*
 *  Copyright (c) 2024 Rockchip Electronics Co., Ltd
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

#include "hwi_c/aiq_spStreamProcUnit.h"

#include "hwi_c/aiq_lensHw.h"

static void set_af_img_size(AiqSPStreamProcUnit_t* pSpSrmU, int w, int h, int w_align,
                            int h_align) {
    if (!pSpSrmU) return;

    pSpSrmU->af_img_width        = w;
    pSpSrmU->af_img_height       = h;
    pSpSrmU->af_img_width_align  = w_align;
    pSpSrmU->af_img_height_align = h_align;
    LOGI("af_img_width %d af_img_height %d af_img_width_align: %d af_img_height_align: %d\n", w, h,
         w_align, h_align);
}

static int32_t get_lowpass_fv(AiqSPStreamProcUnit_t* pSpSrmU, uint32_t sequence,
                              AiqV4l2Buffer_t* buf) {
    if (!pSpSrmU) return -1;

#if RKAIQ_HAVE_AF_V20
    uint8_t* image_buf = (uint8_t*)buf->_expbuf_usrptr;
    rk_aiq_af_algo_meas_t meas_param;

    aiqMutex_lock(&pSpSrmU->_afmeas_param_mutex);
    meas_param = pSpSrmU->_af_meas_params;
    aiqMutex_unlock(&pSpSrmU->_afmeas_param_mutex);

    if (meas_param.sp_meas.enable) {
        meas_param.wina_h_offs /= pSpSrmU->img_ds_size_x;
        meas_param.wina_v_offs /= pSpSrmU->img_ds_size_y;
        meas_param.wina_h_size /= pSpSrmU->img_ds_size_x;
        meas_param.wina_v_size /= pSpSrmU->img_ds_size_y;
        get_lpfv(sequence, image_buf, pSpSrmU->af_img_width, pSpSrmU->af_img_height,
                 pSpSrmU->af_img_width_align, pSpSrmU->af_img_height_align, pSpSrmU->pAfTmp,
                 pSpSrmU->sub_shp4_4, pSpSrmU->sub_shp8_8, pSpSrmU->high_light,
                 pSpSrmU->high_light2, &meas_param);

        AiqLensHw_setLowPassFv(pSpSrmU->_focus_dev, pSpSrmU->sub_shp4_4, pSpSrmU->sub_shp8_8,
                               pSpSrmU->high_light, pSpSrmU->high_light2, sequence);
    }
#endif
    return 0;
}

XCamReturn AiqSPStreamProcUnit_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt, int dev_index) {
    if (!ctx || !evt) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_AF_V20
    if (pSpSrmU->ldg_enable) {
        AiqSPStreamProcUnit_t* pSpSrmU = (AiqSPStreamProcUnit_t*)ctx;
        get_lowpass_fv(pSpSrmU, evt->frame_id, (AiqV4l2Buffer_t*)evt->vb);
    }
#endif

    return ret;
}

XCamReturn AiqSPStreamProcUnit_init(AiqSPStreamProcUnit_t* pSpSrmU, AiqV4l2Device_t* isp_sp_dev,
                                    int type, int isp_ver) {
    if (!pSpSrmU) return XCAM_RETURN_ERROR_PARAM;

    memset(pSpSrmU, 0, sizeof(*pSpSrmU));

    pSpSrmU->_spStream = (AiqStream_t*)aiq_mallocz(sizeof(AiqStream_t));
    if (AiqStream_init(pSpSrmU->_spStream, isp_sp_dev, ISP_POLL_SP) < 0) goto fail;
    pSpSrmU->_isp_ver = isp_ver;

    aiqMutex_init(&pSpSrmU->_afmeas_param_mutex);

    if (!pSpSrmU->_pcb) {
        AiqPollCallback_t* pcb = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
        pcb->_pCtx             = pSpSrmU;
        pcb->poll_buffer_ready = AiqSPStreamProcUnit_poll_buffer_ready;
        pcb->poll_event_ready  = NULL;
        pSpSrmU->_pcb          = pcb;
        AiqPollThread_setPollCallback(pSpSrmU->_spStream->_poll_thread, pcb);
    }

    return XCAM_RETURN_NO_ERROR;
fail:
    AiqSPStreamProcUnit_deinit(pSpSrmU);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqSPStreamProcUnit_deinit(AiqSPStreamProcUnit_t* pSpSrmU) {
    aiqMutex_deInit(&pSpSrmU->_afmeas_param_mutex);

    if (pSpSrmU->_pcb) {
        aiq_free(pSpSrmU->_pcb);
        pSpSrmU->_pcb = NULL;
    }

    if (pSpSrmU->_spStream) {
        aiq_free(pSpSrmU->_spStream);
        pSpSrmU->_spStream = NULL;
    }
}

XCamReturn AiqSPStreamProcUnit_start(AiqSPStreamProcUnit_t* pSpSrmU) {
    if (!pSpSrmU) return XCAM_RETURN_ERROR_PARAM;

    if (pSpSrmU->ldg_enable) {
        pSpSrmU->pAfTmp = (uint8_t*)aiq_mallocz(pSpSrmU->_ds_width_align * pSpSrmU->_ds_height_align *
                                           sizeof(pSpSrmU->pAfTmp[0]) * 3 / 2);
    }

    if (pSpSrmU->_spStream) pSpSrmU->_spStream->start(pSpSrmU->_spStream);

    return XCAM_RETURN_ERROR_FAILED;
}

void AiqSPStreamProcUnit_stop(AiqSPStreamProcUnit_t* pSpSrmU) {
    if (!pSpSrmU) return;

    if (pSpSrmU->_spStream) pSpSrmU->_spStream->stop(pSpSrmU->_spStream);

    if (pSpSrmU->pAfTmp) {
        aiq_free(pSpSrmU->pAfTmp);
        pSpSrmU->pAfTmp = NULL;
    }
}

XCamReturn AiqSPStreamProcUnit_prepare(AiqSPStreamProcUnit_t* pSpSrmU,
                                       CalibDbV2_Af_LdgParam_t* ldg_param,
                                       CalibDbV2_Af_HighLightParam_t* highlight, int width,
                                       int height, int stride) {
    if (!pSpSrmU) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t pixelformat, plane_cnt;
    uint32_t ds_size_w = 4;
    uint32_t ds_size_h = 4;

    pixelformat = V4L2_PIX_FMT_NV12;
    plane_cnt   = 1;

    if (!width && !height && pSpSrmU->_spStream) {
        struct v4l2_subdev_format isp_src_fmt;
        isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        isp_src_fmt.pad   = 2;
        ret               = pSpSrmU->_isp_core_dev->getFormat(pSpSrmU->_isp_core_dev, &isp_src_fmt);
        if (ret) {
            LOGE("get mIspCoreDev src fmt failed !\n");
            return XCAM_RETURN_ERROR_FAILED;
        }
        pSpSrmU->_src_width       = isp_src_fmt.format.width;
        pSpSrmU->_src_height      = isp_src_fmt.format.height;
        pSpSrmU->_ds_width        = (pSpSrmU->_src_width + ds_size_w - 1) / ds_size_w;
        pSpSrmU->_ds_height       = (pSpSrmU->_src_height + ds_size_h - 1) / ds_size_h;
        pSpSrmU->_ds_width_align  = (pSpSrmU->_ds_width + 7) & (~7);
        pSpSrmU->_ds_height_align = (pSpSrmU->_ds_height + 7) & (~7);
        int _stride               = XCAM_ALIGN_UP(pSpSrmU->_ds_width_align, 64);
        pSpSrmU->img_ds_size_x    = ds_size_w;
        pSpSrmU->img_ds_size_y    = ds_size_h;
        LOGD("set sp format: _src_width %d, _src_height %d, width %d %d height %d %d, stride %d\n",
             _src_width, _src_height, _ds_width, _ds_width_align, _ds_height, _ds_height_align,
             _stride);
        ret = AiqV4l2Device_setFmt(pSpSrmU->_spStream->_dev, pSpSrmU->_ds_width_align,
                                   pSpSrmU->_ds_height_align, pixelformat, V4L2_FIELD_NONE, 0);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }

        struct v4l2_format format;
        pSpSrmU->_spStream->_dev->get_format(pSpSrmU->_spStream->_dev, &format);
        set_af_img_size(pSpSrmU, format.fmt.pix_mp.width, format.fmt.pix_mp.height,
                        format.fmt.pix_mp.plane_fmt[0].bytesperline, format.fmt.pix_mp.height);
    } else {
        LOGD("set sp format: width %d height %d\n", width, height);
        ret = AiqV4l2Device_setFmt(pSpSrmU->_spStream->_dev, width, height, pixelformat,
                                   V4L2_FIELD_NONE, stride);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }
    }

    AiqV4l2Device_setMemType(pSpSrmU->_spStream->_dev, V4L2_MEMORY_MMAP);
    AiqV4l2Device_setBufType(pSpSrmU->_spStream->_dev, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    AiqV4l2Device_setBufCnt(pSpSrmU->_spStream->_dev, 6);
    AiqV4l2Device_setMplanesCount(pSpSrmU->_spStream->_dev, plane_cnt);

    pSpSrmU->ldg_enable = ldg_param->enable;
    if (pSpSrmU->ldg_enable) {
        pSpSrmU->_af_meas_params.sp_meas.ldg_xl        = ldg_param->ldg_xl;
        pSpSrmU->_af_meas_params.sp_meas.ldg_yl        = ldg_param->ldg_yl;
        pSpSrmU->_af_meas_params.sp_meas.ldg_kl        = ldg_param->ldg_kl;
        pSpSrmU->_af_meas_params.sp_meas.ldg_xh        = ldg_param->ldg_xh;
        pSpSrmU->_af_meas_params.sp_meas.ldg_yh        = ldg_param->ldg_yh;
        pSpSrmU->_af_meas_params.sp_meas.ldg_kh        = ldg_param->ldg_kh;
        pSpSrmU->_af_meas_params.sp_meas.highlight_th  = highlight->ther0;
        pSpSrmU->_af_meas_params.sp_meas.highlight2_th = highlight->ther1;
    }

    return ret;
}

void AiqSPStreamProcUnit_set_devices(AiqSPStreamProcUnit_t* pSpSrmU, AiqCamHwBase_t* camHw,
                                     AiqV4l2SubDevice_t* isp_core_dev, AiqLensHw_t* lensdev) {
    if (!pSpSrmU) return;

    pSpSrmU->_camHw        = camHw;
    pSpSrmU->_isp_core_dev = isp_core_dev;
    pSpSrmU->_focus_dev    = (struct AiqLensHw_s*)lensdev;
}

XCamReturn AiqSPStreamProcUnit_get_sp_resolution(AiqSPStreamProcUnit_t* pSpSrmU, int* width,
                                                 int* height, int* aligned_w, int* aligned_h) {
    if (!pSpSrmU) return XCAM_RETURN_ERROR_PARAM;

    *width     = pSpSrmU->_ds_width;
    *height    = pSpSrmU->_ds_height;
    *aligned_w = pSpSrmU->_ds_width_align;
    *aligned_h = pSpSrmU->_ds_height_align;

    return XCAM_RETURN_NO_ERROR;
}

void AiqSPStreamProcUnit_upd_af_meas(AiqSPStreamProcUnit_t* pSpSrmU,
                                     rk_aiq_isp_af_meas_t* af_meas) {
    if (!pSpSrmU) return;

    aiqMutex_lock(&pSpSrmU->_afmeas_param_mutex);
    if (af_meas &&
        (0 != memcmp(af_meas, &pSpSrmU->_af_meas_params, sizeof(rk_aiq_af_algo_meas_t)))) {
        pSpSrmU->_af_meas_params = *af_meas;
    }
    aiqMutex_unlock(&pSpSrmU->_afmeas_param_mutex);
}

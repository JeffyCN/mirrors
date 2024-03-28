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


#include "FecParamStream.h"
#include "rkispp-config.h"
#include "CamHwIsp20.h"
namespace RkCam {

FecParamStream::FecParamStream (SmartPtr<V4l2Device> dev, int type)
    : RKStream(dev, type)
{
    _dev->open();
    mParamsAssembler = new IspParamsAssembler("FEC_PARAMS_ASSEMBLER");
    mCamHw = NULL;
    memset(&last_ispp_fec_params, 0, sizeof(last_ispp_fec_params));
}

FecParamStream::FecParamStream (const rk_sensor_full_info_t *s_info)
    : RKStream(s_info->ispp_info->pp_fec_params_path, ISPP_POLL_FEC_PARAMS)
{
    _dev->open();
    mParamsAssembler = new IspParamsAssembler("FEC_PARAMS_ASSEMBLER");
    mCamHw = NULL;
    memset(&last_ispp_fec_params, 0, sizeof(last_ispp_fec_params));
}


FecParamStream::~FecParamStream()
{
}

void
FecParamStream::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!_dev->is_activated()) {
        RKStream::start();
    }
    // set inital params
    ret = mParamsAssembler->start();
    if (ret < 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "params assembler start err: %d\n", ret);
        return;
    }

    if (mParamsAssembler->ready())
        configToDrv(0);
    else
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "no inital fec params ready");
}

void
FecParamStream::stop()
{
    RKStream::stop();
    if (mParamsAssembler.ptr())
        mParamsAssembler->stop();
}

void FecParamStream::set_devices(CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> isppdev)
{
    mIsppSubDev = isppdev;
    mCamHw = camHw;
}

XCamReturn FecParamStream::configToDrv(uint32_t frameId)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());

    SmartPtr<V4l2Buffer> v4l2buf_fec;
    struct rkispp_params_feccfg *ispp_fec_params = NULL;

    if (!mParamsAssembler->ready()) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "have no fec new parameter\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    ret = _dev->get_buffer(v4l2buf_fec);
    if (ret) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "Can not get ispp fec params buffer\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    cam3aResultList ready_results;
    ret = mParamsAssembler->deQueOne(ready_results, frameId);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "deque parameter failed\n");
        ret = XCAM_RETURN_ERROR_PARAM;
        goto ret_fec_buf;
    }

    ispp_fec_params = (struct rkispp_params_feccfg*)v4l2buf_fec->get_buf().m.userptr;
    if (mCamHw->get_fec_cfg_params(ready_results, *ispp_fec_params) != XCAM_RETURN_NO_ERROR)
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "fec parameter translation error\n");

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "fec: en update 0x%x, ens 0x%x, cfg update 0x%x",
                    ispp_fec_params->head.module_en_update, ispp_fec_params->head.module_ens,
                    ispp_fec_params->head.module_cfg_update);
    if (memcmp(&last_ispp_fec_params, ispp_fec_params, sizeof(last_ispp_fec_params)) == 0) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "fec: no need update !");
        ret = XCAM_RETURN_NO_ERROR;
        goto ret_fec_buf;
    }

    last_ispp_fec_params = *ispp_fec_params;
    if ((ispp_fec_params->head.module_cfg_update) || (ispp_fec_params->head.module_en_update)) {
        ispp_fec_params->head.frame_id = frameId;
        //ispp_fec_params->image.index = ispp_params->fec_output_buf_index;
        //ispp_fec_params->image.size = ispp_params->fec_output_buf_size;
        //TODO set update bits
        if (_dev->queue_buffer (v4l2buf_fec) != 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "RKISP1: fec: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n",
                            v4l2buf_fec->get_buf().index, errno, strerror(errno));
            goto ret_fec_buf;
        }
    } else
        goto ret_fec_buf;

    return XCAM_RETURN_NO_ERROR;

ret_fec_buf:
    if (v4l2buf_fec.ptr())
        _dev->return_buffer_to_pool (v4l2buf_fec);
    return ret;
}

XCamReturn FecParamStream::config_params(uint32_t frameId, SmartPtr<cam3aResult>& result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());

    // params device should started befor any params queued
    if (!_dev->is_activated()) {
        mParamsAssembler->addReadyCondition(result->getType());
        mParamsAssembler->queue(result);
    } else {
        mParamsAssembler->queue(result);
        // set all ready params to drv
        while (mParamsAssembler->ready()) {
            if (configToDrv(frameId) != XCAM_RETURN_NO_ERROR)
                break;
        }
    }

    return ret;
}
} //namspace RkCam

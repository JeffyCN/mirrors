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

#include "TnrStreamProcUnit.h"
#include "CamHwIsp20.h"

namespace RkCam {

#define ISPP_TNR_SUBM  (0x03)

TnrStreamProcUnit::TnrStreamProcUnit (const rk_sensor_full_info_t *s_info)
{
    mTnrStatsDev = new V4l2Device(s_info->ispp_info->pp_tnr_stats_path);
    mTnrStatsDev->open();
    mTnrParamsDev = new V4l2Device (s_info->ispp_info->pp_tnr_params_path);
    mTnrParamsDev->open();
    mTnrStatsStream = new TnrStatsStream(mTnrStatsDev, ISPP_POLL_TNR_STATS);
    mTnrParamStream = new RKStream(mTnrParamsDev, ISPP_POLL_TNR_PARAMS);
    mParamsAssembler = new IspParamsAssembler("TNR_PARAMS_ASSEMBLER");
    mCamHw = NULL;
    memset(&last_ispp_tnr_params, 0, sizeof(last_ispp_tnr_params));
}

TnrStreamProcUnit::~TnrStreamProcUnit()
{
}

void
TnrStreamProcUnit::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mTnrParamsDev.ptr() && !mTnrParamsDev->is_activated()) {
        if (mTnrStatsStream.ptr())
            mTnrStatsStream->start();
        if (mTnrParamStream.ptr())
            //mTnrParamStream->startThreadOnly();
            mTnrParamStream->start();
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
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "no inital tnr params ready");
}

void
TnrStreamProcUnit::stop()
{
    if (mTnrParamStream.ptr())
        mTnrParamStream->stop();
    if (mTnrStatsStream.ptr())
        mTnrStatsStream->stop();
    if (mParamsAssembler.ptr())
        mParamsAssembler->stop();
}

void
TnrStreamProcUnit::pause()
{
}

void
TnrStreamProcUnit::resume()
{
}

void
TnrStreamProcUnit::set_devices(CamHwIsp20 *camHw, SmartPtr<V4l2SubDevice> isppdev)
{
    mIsppSubDev = isppdev;
    mCamHw = camHw;
    mTnrStatsStream->set_device(camHw, isppdev);
}

XCamReturn TnrStreamProcUnit::configToDrv(uint32_t frameId)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());

    SmartPtr<V4l2Buffer> v4l2buf_tnr;
    if (mTnrParamsDev.ptr()) {

        struct rkispp_params_tnrcfg *ispp_tnr_params = NULL;

        if (!mParamsAssembler->ready()) {
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "have no tnr new parameter\n");
            return XCAM_RETURN_ERROR_PARAM;
        }

        ret = mTnrParamsDev->get_buffer(v4l2buf_tnr);
        if (ret) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "Can not get ispp tnr params buffer\n");
            return XCAM_RETURN_ERROR_PARAM;
        }

        cam3aResultList ready_results;
        ret = mParamsAssembler->deQueOne(ready_results, frameId);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "deque parameter failed\n");
            ret = XCAM_RETURN_ERROR_PARAM;
            goto ret_tnr_buf;
        }

        ispp_tnr_params = (struct rkispp_params_tnrcfg*)v4l2buf_tnr->get_buf().m.userptr;
        ispp_tnr_params->head.frame_id = frameId;

        if (mCamHw->get_tnr_cfg_params(ready_results, *ispp_tnr_params) != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "tnr parameter translation error\n");

        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "tnr: en update 0x%x, ens 0x%x, cfg update 0x%x",
                        ispp_tnr_params->head.module_en_update, ispp_tnr_params->head.module_ens,
                        ispp_tnr_params->head.module_cfg_update);

        if (memcmp(&last_ispp_tnr_params, ispp_tnr_params, sizeof(last_ispp_tnr_params)) == 0) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "tnr: no need update !");
            ret = XCAM_RETURN_NO_ERROR;
            goto ret_tnr_buf;
        }

        last_ispp_tnr_params = *ispp_tnr_params;

        if ((ispp_tnr_params->head.module_cfg_update) || (ispp_tnr_params->head.module_en_update)) {
            if (mTnrParamsDev->queue_buffer (v4l2buf_tnr) != 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "RKISP1: tnr: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n",
                                v4l2buf_tnr->get_buf().index, errno, strerror(errno));
                goto ret_tnr_buf;
            }
        } else
            goto ret_tnr_buf;
    } else
        return XCAM_RETURN_BYPASS;

    return XCAM_RETURN_NO_ERROR;

ret_tnr_buf:
    if (v4l2buf_tnr.ptr())
        mTnrParamsDev->return_buffer_to_pool (v4l2buf_tnr);
    return ret;
}

XCamReturn TnrStreamProcUnit::config_params(uint32_t frameId, SmartPtr<cam3aResult>& result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());

    // params device should started befor any params queued
    if (mTnrParamsDev.ptr() && !mTnrParamsDev->is_activated()) {
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

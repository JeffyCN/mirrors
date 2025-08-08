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

#include "NrStreamProcUnit.h"
#include "CamHwIsp20.h"

namespace RkCam {

#define ISPP_NR_SUBM  (0x02)

NrStreamProcUnit::NrStreamProcUnit (const rk_sensor_full_info_t *s_info)
{
    mNrStatsDev = new V4l2Device(s_info->ispp_info->pp_nr_stats_path);
    mNrStatsDev->open();
    mNrParamsDev = new V4l2Device (s_info->ispp_info->pp_nr_params_path);
    mNrParamsDev->open();
    mNrStatsStream = new NrStatsStream(mNrStatsDev, ISPP_POLL_NR_STATS);
    mNrParamStream = new RKStream(mNrParamsDev, ISPP_POLL_NR_PARAMS);
    mParamsAssembler = new IspParamsAssembler("NR_PARAMS_ASSEMBLER");
    mCamHw = NULL;
    memset(&last_ispp_nr_params, 0, sizeof(last_ispp_nr_params));
}

NrStreamProcUnit::~NrStreamProcUnit()
{
}

void
NrStreamProcUnit::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mNrParamsDev.ptr() && !mNrParamsDev->is_activated()) {
        if (mNrStatsStream.ptr())
            mNrStatsStream->start();
        if (mNrParamStream.ptr())
            //mNrParamStream->startThreadOnly();
            mNrParamStream->start();
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
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "no inital nr params ready");
}

void
NrStreamProcUnit::stop()
{
    if (mNrParamStream.ptr())
        mNrParamStream->stop();
    if (mNrStatsStream.ptr())
        mNrStatsStream->stop();
    if (mParamsAssembler.ptr())
        mParamsAssembler->stop();
}

void
NrStreamProcUnit::pause()
{
}

void
NrStreamProcUnit::resume()
{
}

void
NrStreamProcUnit::set_devices(CamHwIsp20 *camHw, SmartPtr<V4l2SubDevice> isppdev)
{
    mIsppSubDev = isppdev;
    mCamHw = camHw;
    mNrStatsStream->set_device(camHw, isppdev);
}

XCamReturn NrStreamProcUnit::configToDrv(uint32_t frameId)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());

    SmartPtr<V4l2Buffer> v4l2buf_nr;
    if (mNrParamsDev.ptr()) {
        if (!mParamsAssembler->ready()) {
            LOGI_CAMHW_SUBM(ISPP_NR_SUBM, "have no nr new parameter\n");
            return XCAM_RETURN_ERROR_PARAM;
        }

        ret = mNrParamsDev->get_buffer(v4l2buf_nr);
        if (ret) {
            LOGW_CAMHW_SUBM(ISPP_NR_SUBM, "Can not get ispp nr params buffer\n");
            return XCAM_RETURN_ERROR_PARAM;
        }

        cam3aResultList ready_results;
        ret = mParamsAssembler->deQueOne(ready_results, frameId);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGI_CAMHW_SUBM(ISPP_NR_SUBM, "deque parameter failed\n");
            ret = XCAM_RETURN_ERROR_PARAM;
            goto ret_nr_buf;
        }

        struct rkispp_params_nrcfg* ispp_nr_params = (struct rkispp_params_nrcfg*)v4l2buf_nr->get_buf().m.userptr;
        if (mCamHw->merge_results(ready_results, *ispp_nr_params) != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISPP_NR_SUBM, "nr parameter translation error\n");
        LOGD_CAMHW_SUBM(ISPP_NR_SUBM, "nr: frameid %u, en update 0x%x, ens 0x%x, cfg update 0x%x",frameId,
                        ispp_nr_params->head.module_en_update, ispp_nr_params->head.module_ens,
                        ispp_nr_params->head.module_cfg_update);

        if (memcmp(&last_ispp_nr_params, ispp_nr_params, sizeof(last_ispp_nr_params)) == 0) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "nr: no need update !");
            ret = XCAM_RETURN_NO_ERROR;
            goto ret_nr_buf;
        }

        last_ispp_nr_params = *ispp_nr_params;

        if ((ispp_nr_params->head.module_cfg_update) || (ispp_nr_params->head.module_en_update))
        {
            ispp_nr_params->head.frame_id = frameId;
            ispp_nr_params->gain.index = -1;
            ispp_nr_params->gain.size = 0;
            //mNrStatsStream->get_nrgain_fd();
            //ispp_nr_params->gain.index = mNrStatsStream->get_nrgain_fd();
            //ispp_nr_params->gain = ispp_params->gain;
            //TODO set update bits
            if (mNrParamsDev->queue_buffer (v4l2buf_nr) != 0) {
                LOGE_CAMHW_SUBM(ISPP_NR_SUBM, "RKISP1: nr: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n",
                                v4l2buf_nr->get_buf().index, errno, strerror(errno));
                goto ret_nr_buf;
            }
            //setIsppSharpFbcRot(&ispp_nr_params->shp_cfg);
        } else
            goto ret_nr_buf;
     } else
        ret = XCAM_RETURN_BYPASS;

    return XCAM_RETURN_NO_ERROR;

ret_nr_buf:
    if (v4l2buf_nr.ptr())
        mNrParamsDev->return_buffer_to_pool (v4l2buf_nr);
    return ret;

}

XCamReturn NrStreamProcUnit::config_params(uint32_t frameId, SmartPtr<cam3aResult>& result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT (mParamsAssembler.ptr());
    // params device should started befor any params queued
    if (mNrParamsDev.ptr() && !mNrParamsDev->is_activated()) {
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

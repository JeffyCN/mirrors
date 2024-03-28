#include "rk_aiq_comm.h"
#include "rkispp-config.h"
#include "rkisp2-config.h"
#include "PdafStreamProcUnit.h"
#include "CamHwIsp20.h"
#include "code_to_pixel_format.h"
#include "rkcif-config.h"

namespace RkCam {

PdafStreamProcUnit::PdafStreamProcUnit (int type)
{
    mCamHw = NULL;
    mPdafDev = NULL;
    mPdafStream = NULL;
    mStartFlag = false;
    mStartStreamFlag = false;
    mBufType = type;
    mHelperThd = new PdafStreamHelperThd(this);
    mHelperThd->triger_start();
    mHelperThd->start();
}

PdafStreamProcUnit::~PdafStreamProcUnit ()
{
    mHelperThd->triger_stop();
    mHelperThd->stop();
}

void PdafStreamProcUnit::set_devices(CamHwIsp20* camHw)
{
    mCamHw = camHw;
}

XCamReturn
PdafStreamProcUnit::prepare(rk_sensor_pdaf_info_t *pdaf_inf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    stop();
    mPdafInf = *pdaf_inf;

    mPdafDev = new V4l2Device(mPdafInf.pdaf_vdev);
    mPdafDev->open();
    mPdafStream = new RKPdafStream(mPdafDev, ISP_POLL_PDAF_STATS);
    mPdafStream->setPollCallback (this);

    mPdafMeas.pdafSensorType = mPdafInf.pdaf_type;
    mPdafMeas.pdLRInDiffLine = mPdafInf.pdaf_lrdiffline;
    mPdafMeas.pdWidth = mPdafInf.pdaf_width;
    mPdafMeas.pdHeight = mPdafInf.pdaf_height;

    LOGD_AF("%s: pd inf: pdaf_vdev %s, pdafSensorType %d, pdLRInDiffLine %d, pdWidth %d, pdHeight %d",
        __func__, mPdafInf.pdaf_vdev, mPdafMeas.pdafSensorType, mPdafMeas.pdLRInDiffLine,
        mPdafMeas.pdWidth, mPdafMeas.pdHeight);
    ret = mPdafDev->set_format(mPdafInf.pdaf_width, mPdafInf.pdaf_height,
                               mPdafInf.pdaf_pixelformat, V4L2_FIELD_NONE, 0);

    return ret;
}

void PdafStreamProcUnit::start()
{
    int mem_mode;

    mStreamMutex.lock();
    if (mPdafStream.ptr() && !mStartFlag) {
        if (mPdafInf.pdaf_type != PDAF_SENSOR_TYPE3) {
            mPdafDev->io_control (RKCIF_CMD_GET_CSI_MEMORY_MODE, &mem_mode);
            if (mem_mode != CSI_LVDS_MEM_WORD_LOW_ALIGN) {
                mem_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
                mPdafDev->io_control (RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
                LOGI_AF("memory mode of pdaf video need low align, mem_mode %d", mem_mode);
            }

            XCam::SmartPtr<PdafStreamParam> attrPtr = new PdafStreamParam;

            attrPtr->valid = true;
            attrPtr->stream_flag = true;
            mHelperThd->clear_attr();
            mHelperThd->push_attr(attrPtr);
        } else {
            start_stream(false);
        }

        mStartFlag = true;
    }
    mStreamMutex.unlock();
}

void PdafStreamProcUnit::stop()
{
    mStreamMutex.lock();
    if (mPdafStream.ptr() && mStartFlag) {
        if (mPdafInf.pdaf_type != PDAF_SENSOR_TYPE3) {
            XCam::SmartPtr<PdafStreamParam> attrPtr = new PdafStreamParam;

            attrPtr->valid = true;
            attrPtr->stream_flag = false;
            mHelperThd->clear_attr();
            mHelperThd->push_attr(attrPtr);
        } else {
            stop_stream(false);
        }

        mStartFlag = false;
    }
    mStreamMutex.unlock();
}

int64_t PdafStreamProcUnit::get_systime_us()
{
    struct timespec times = {0, 0};
    int64_t time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec * 1000000LL + times.tv_nsec / 1000LL;

    return time;
}

XCamReturn
PdafStreamProcUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mCamHw->mHwResLintener) {
        SmartPtr<VideoBuffer> video_buf = buf.dynamic_cast_ptr<VideoBuffer>();
        SmartPtr<PdafBufferProxy> pdaf_buf = buf.dynamic_cast_ptr<PdafBufferProxy>();

        pdaf_buf->pdaf_meas = mPdafMeas;
        //LOGD_AF("%s: PDAF_STATS seq: %d, driver_time : %lld, aiq_time: %lld", __func__,
        //    video_buf->get_sequence(), video_buf->get_timestamp(), get_systime_us());

        if (mPdafInf.pdaf_type != PDAF_SENSOR_TYPE3) {
            // change timestamp as vicap driver set timestamp using fs, we need fe time as 3a stats use fe time.
            video_buf->set_timestamp(get_systime_us());
        }
        mCamHw->mHwResLintener->hwResCb(video_buf);
    }

    return ret;
}

XCamReturn
PdafStreamProcUnit::deinit()
{
    mHelperThd->triger_stop();
    mHelperThd->stop();
    stop_stream(true);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PdafStreamProcUnit::start_stream(bool lock)
{
    if (lock)
        mStreamMutex.lock();
    if (mPdafStream.ptr() && !mStartStreamFlag) {
        mPdafStream->start();
        mStartStreamFlag = true;
        LOGD_AF("start pdaf stream device");
    }
    if (lock)
        mStreamMutex.unlock();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PdafStreamProcUnit::stop_stream(bool lock)
{
    if (lock)
        mStreamMutex.lock();
    if (mPdafStream.ptr() && mStartStreamFlag) {
        mPdafStream->stop();
        mStartStreamFlag = false;
        LOGD_AF("stop pdaf stream device");
    }
    if (lock)
        mStreamMutex.unlock();

    return XCAM_RETURN_NO_ERROR;
}

bool PdafStreamHelperThd::loop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const static int32_t timeout = -1;
    XCam::SmartPtr<PdafStreamParam> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGI_AF("PdafStreamHelperThd got empty attrib, stop thread");
        return false;
    }

    if (attrib->valid) {
        if (attrib->stream_flag) {
            ret = mPdafStreamProc->start_stream(true);
        } else {
            ret = mPdafStreamProc->stop_stream(true);
        }
    }

    if (ret == XCAM_RETURN_NO_ERROR)
        return true;

    LOGI_AF("PdafStreamHelperThd stop");

    return false;
}

}

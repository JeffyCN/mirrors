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
    uint32_t pixelformat;

    stop();
    mPdafDev = new V4l2Device(pdaf_inf->pdaf_vdev);
    mPdafDev->open();
    mPdafStream = new RKPdafStream(mPdafDev, ISP_POLL_PDAF_STATS);
    mPdafStream->setPollCallback (this);

    mPdafMeas.pdLRInDiffLine = pdaf_inf->pdaf_lrdiffline;
    mPdafMeas.pdWidth = pdaf_inf->pdaf_width;
    mPdafMeas.pdHeight = pdaf_inf->pdaf_height;
    ret = mPdafDev->set_format(pdaf_inf->pdaf_width, pdaf_inf->pdaf_height,
            pdaf_inf->pdaf_pixelformat, V4L2_FIELD_NONE, 0);

    return ret;
}

void PdafStreamProcUnit::start()
{
    int mem_mode;

    mStreamMutex.lock();
    if (mPdafStream.ptr() && !mStartFlag) {
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

        mStartFlag = true;
    }
    mStreamMutex.unlock();
}

void PdafStreamProcUnit::stop()
{
    mStreamMutex.lock();
    if (mPdafStream.ptr() && mStartFlag) {
        XCam::SmartPtr<PdafStreamParam> attrPtr = new PdafStreamParam;

        attrPtr->valid = true;
        attrPtr->stream_flag = false;
        mHelperThd->clear_attr();
        mHelperThd->push_attr(attrPtr);

        mStartFlag = false;
    }
    mStreamMutex.unlock();
}

XCamReturn
PdafStreamProcUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mCamHw->mHwResLintener) {
        SmartPtr<VideoBuffer> video_buf = buf.dynamic_cast_ptr<VideoBuffer>();
        SmartPtr<PdafBufferProxy> pdaf_buf = buf.dynamic_cast_ptr<PdafBufferProxy>();

        pdaf_buf->pdaf_meas = mPdafMeas;
        mCamHw->mHwResLintener->hwResCb(video_buf);
    }

    return ret;
}

XCamReturn
PdafStreamProcUnit::deinit()
{
    mHelperThd->triger_stop();
    mHelperThd->stop();
    stop_stream();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PdafStreamProcUnit::start_stream()
{
    mStreamMutex.lock();
    if (mPdafStream.ptr() && !mStartStreamFlag) {
        mPdafStream->start();
        mStartStreamFlag = true;
        LOGD_AF("start pdaf stream device");
    }
    mStreamMutex.unlock();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PdafStreamProcUnit::stop_stream()
{
    mStreamMutex.lock();
    if (mPdafStream.ptr() && mStartStreamFlag) {
        mPdafStream->stop();
        mStartStreamFlag = false;
        LOGD_AF("stop pdaf stream device");
    }
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
            ret = mPdafStreamProc->start_stream();
        } else {
            ret = mPdafStreamProc->stop_stream();
        }
    }

    if (ret == XCAM_RETURN_NO_ERROR)
        return true;

    LOGI_AF("PdafStreamHelperThd stop");

    return false;
}

}

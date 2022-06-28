#include "rk_aiq_comm.h"
#include "rkispp-config.h"
#include "rkisp2-config.h"
#include "PdafStreamProcUnit.h"
#include "CamHwIsp20.h"
#include "code_to_pixel_format.h"

namespace RkCam {

PdafStreamProcUnit::PdafStreamProcUnit (int type)
{
    mPdafDev = NULL;
    mPdafStream = NULL;
    mStartFlag = false;
    mBufType = type;
}

PdafStreamProcUnit::~PdafStreamProcUnit ()
{
}

void PdafStreamProcUnit::set_devices(CamHwIsp20* camHw)
{
    mCamHw = camHw;
}

XCamReturn
PdafStreamProcUnit::prepare(CalibDbV2_Af_Pdaf_t *pdaf_calib, rk_sensor_pdaf_info_t *pdaf_inf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t pixelformat;

    stop();
    mPdafDev = new V4l2Device(pdaf_inf->pdaf_vdev);
    mPdafDev->open();
    mPdafStream = new RKPdafStream(mPdafDev, ISP_POLL_PDAF_STATS);
    mPdafStream->setPollCallback (this);

    mPdafMeas.pdMirrorInCalib = pdaf_calib->pdMirrorInCalib;
    mPdafMeas.pdWidth = pdaf_calib->pdWidth;
    mPdafMeas.pdHeight = pdaf_calib->pdHeight;

    ret = mPdafDev->set_format(pdaf_inf->pdaf_width, pdaf_inf->pdaf_height,
            pdaf_inf->pdaf_pixelformat, V4L2_FIELD_NONE, 0);

    return ret;
}

void PdafStreamProcUnit::start()
{
    if (mPdafStream.ptr() && !mStartFlag) {
        mPdafStream->start();
        mStartFlag = true;
    }
}

void PdafStreamProcUnit::stop()
{
    if (mPdafStream.ptr() && mStartFlag) {
        mPdafStream->stop();
        mStartFlag = false;
    }
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

}

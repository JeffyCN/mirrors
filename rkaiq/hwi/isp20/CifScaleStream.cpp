#include "CamHwIsp20.h"
#include "rk_aiq_comm.h"
#include "fake_v4l2_device.h"
#include "rkcif-config.h"

namespace RkCam {

CifSclStream::CifSclStream()
    : _working_mode(0)
    , _bpp(0)
    , index(0)
    , _width(0)
    , _height(0)
    , _sns_v4l_pix_fmt(0)
    , _ratio(0)
    , _first_start(true)
{}

CifSclStream::~CifSclStream() {}

bool
CifSclStream::setPollCallback (PollCallback *callback)
{
    for (int i = 0; i < index; i++) {
        _stream[i]->setPollCallback(callback);
    }
    return true;
}

XCamReturn
CifSclStream::set_format(const struct v4l2_subdev_format& sns_sd_fmt,
                         uint32_t sns_v4l_pix_fmt, int bpp)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    _width = sns_sd_fmt.format.width;
    _height = sns_sd_fmt.format.height;
    _bpp = bpp;

    switch (sns_v4l_pix_fmt)
    {
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SBGGR16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SBGGR16;
        break;
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGBRG16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SGBRG16;
        break;
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SGRBG16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SGRBG16;
        break;
    case V4L2_PIX_FMT_SRGGB8:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SRGGB14:
    case V4L2_PIX_FMT_SRGGB16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SRGGB16;
        break;
    default:
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "unknown format");
        return XCAM_RETURN_ERROR_PARAM;
    }

    return ret;
}

XCamReturn
CifSclStream::set_format(const struct v4l2_subdev_selection& sns_sd_sel,
                         uint32_t sns_v4l_pix_fmt, int bpp)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    _width = sns_sd_sel.r.width;
    _height = sns_sd_sel.r.height;
    _bpp = bpp;

    switch (sns_v4l_pix_fmt)
    {
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SBGGR16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SBGGR16;
        break;
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGBRG16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SGBRG16;
        break;
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SGRBG16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SGRBG16;
        break;
    case V4L2_PIX_FMT_SRGGB8:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SRGGB14:
    case V4L2_PIX_FMT_SRGGB16:
        _sns_v4l_pix_fmt = V4L2_PIX_FMT_SRGGB16;
        break;
    default:
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "unknown format");
        return XCAM_RETURN_ERROR_PARAM;
    }
    return ret;
}

void
CifSclStream::set_working_mode(int mode)
{
    _working_mode = mode;
}

XCamReturn CifSclStream::start()
{
    if (_init && !_active) {
        for (int i = 0; i < index; i++) {
            if (_stream[i].ptr())
                _stream[i]->start();
        }
        _active = true;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn CifSclStream::stop()
{
    for (int i = 0; i < index; i++) {
        if (_stream[i].ptr())
            _stream[i]->stop();
    }
    _active = false;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CifSclStream::restart(const rk_sensor_full_info_t *s_info, int ratio, PollCallback *callback, int mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mode) {
        if (!_init)
            ret = init(s_info, callback);

        if (ratio != _ratio) {
            if (_active) {
                stop();
            }
            ret = set_ratio_fmt(ratio);
        }

        if (!_active && _init) {
            ret = prepare();
            ret = start();
        }
    } else {
        if (_active)
            ret = stop();
    }
    return ret;
}

XCamReturn
CifSclStream::init(const rk_sensor_full_info_t *s_info, PollCallback *callback)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!s_info->cif_info) {
        LOGD_CAMHW("no link to vicap\n");
        return XCAM_RETURN_NO_ERROR;
    }

    switch (_working_mode)
    {
    case 0x0:
        index = 1;
        break;
    case 0x11:
    case 0x12:
        index = 2;
        break;
    case 0x21:
    case 0x22:
        index = 3;
        break;
    default:
        index = 0;
        break;
    }


    if (index >= 1) {
        if (strlen(s_info->cif_info->mipi_scl0)) {
            _dev[0] = new V4l2Device(s_info->cif_info->mipi_scl0);
        }
        if (_dev[0].ptr())
            _dev[0]->open();
    }
    if (index >= 2) {
        if (strlen(s_info->cif_info->mipi_scl1)) {
            _dev[1] = new V4l2Device(s_info->cif_info->mipi_scl1);
        }
        if (_dev[1].ptr())
            _dev[1]->open();
    }
    if (index == 3) {
        if (strlen(s_info->cif_info->mipi_scl2)) {
            _dev[2] = new V4l2Device(s_info->cif_info->mipi_scl2);
        }
        if (_dev[2].ptr())
            _dev[2]->open();
    }
    for (int i = 0; i < index; i++) {
        if (_dev[i].ptr()) {
            _stream[i] = new RKRawStream(_dev[i], i, VICAP_POLL_SCL);
            _stream[i]->setPollCallback(callback);
            SmartPtr<RKRawStream> stream = _stream[i].dynamic_cast_ptr<RKRawStream>();
            stream->set_reserved_data(_bpp);
        }
    }

    _init = true;

    return ret;
}

XCamReturn
CifSclStream::set_ratio_fmt(int ratio)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ratio > 0 && ratio <= 8) {
        _ratio = 8;
    } else if (ratio > 8 && ratio <= 16) {
        _ratio = 16;
    } else {
        _ratio = 32;
    }

    uint32_t dst_width = _width / _ratio;
    if ((dst_width % 2) != 0)
        dst_width -= 1;

    uint32_t dst_height = _height / _ratio;
    if ((dst_height % 2) != 0)
        dst_height -= 1;

    for (int i = 0; i < index; i++) {
        ret = _dev[i]->set_format(dst_width,
                                  dst_height,
                                  _sns_v4l_pix_fmt,
                                  V4L2_FIELD_NONE,
                                  0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed", _dev[i]->get_device_name());
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            _dev[i]->get_device_name(), _sns_v4l_pix_fmt,
                            dst_width, dst_height);
        }
    }

    return ret;
}

XCamReturn
CifSclStream::prepare()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!_init || _active)
        return XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < index; i++) {
        ret = _dev[i]->prepare();
        if (ret < 0)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "mipi tx:%d prepare err: %d\n", ret);

        _stream[i]->set_device_prepared(true);
    }

    return ret;
}

bool
CifSclStream::getIsActive()
{
    return _active;
}

}
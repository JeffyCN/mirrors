#include "rk_aiq_comm.h"
#include "rkispp-config.h"
#include "rkisp2-config.h"
#include "SPStreamProcUnit.h"
#include "CamHwIsp20.h"
namespace RkCam {

SPStreamProcUnit::SPStreamProcUnit (SmartPtr<V4l2Device> isp_sp_dev, int type, int isp_ver)
    : RKStream(isp_sp_dev, type)
    , _first(true)
{
    setPollCallback (this);
    _ds_width = 0;
    _ds_height = 0;
    _ds_width_align = 0;
    _ds_height_align = 0;
    _isp_ver = isp_ver;
}

SPStreamProcUnit::~SPStreamProcUnit ()
{
}

void SPStreamProcUnit::start()
{
    if (_isp_ver == ISP_V20) {
        struct rkispp_trigger_mode tnr_trigger;
        tnr_trigger.module = ISPP_MODULE_TNR;
        tnr_trigger.on = 1;
        _ispp_dev->io_control(RKISPP_CMD_TRIGGER_MODE, &tnr_trigger);
    }

    if (ldg_enable) {
        pAfTmp = (uint8_t*)malloc(_ds_width_align * _ds_height_align * sizeof(pAfTmp[0]) * 3 / 2);
    }

    RKStream::start();
    return;
}

void SPStreamProcUnit::stop()
{
    if (_isp_ver == ISP_V20) {
        struct rkispp_trigger_mode tnr_trigger;
        tnr_trigger.module = ISPP_MODULE_TNR;
        tnr_trigger.on = 0;
        _ispp_dev->io_control(RKISPP_CMD_TRIGGER_MODE, &tnr_trigger);
    }

    RKStream::stop();//stopDeviceOnly&stopThreadOnly
    if (_isp_ver == ISP_V20)
        deinit_fbcbuf_fd();
    if (pAfTmp) {
        free(pAfTmp);
        pAfTmp = NULL;
    }
}

SmartPtr<VideoBuffer>
SPStreamProcUnit::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    if (_isp_ver == ISP_V21) {
        SmartPtr<V4l2BufferProxy> img_buf = new V4l2BufferProxy (buf, dev);
        img_buf->_buf_type = _dev_type;

        return img_buf;
    }

    if (_first) {
        init_fbcbuf_fd();
        _first = false;
    }

    //image
    struct VideoBufferInfo vbufInfo;
    vbufInfo.init(V4L2_PIX_FMT_NV12, _ds_width_align, _ds_height_align,
                 XCAM_ALIGN_UP(_ds_width_align, 4), _ds_height_align, 0);
    SmartPtr<SPImagBufferProxy> img_buf = new SPImagBufferProxy (buf, dev);
    struct isp2x_ispgain_buf *ispgain = (struct isp2x_ispgain_buf *)img_buf->get_v4l2_planar_userptr(1);
    img_buf->_buf_type = _dev_type;
    img_buf->set_buff_fd(img_buf->get_expbuf_fd());
    img_buf->set_video_info(vbufInfo);

    //ispgain
    SmartPtr<V4l2BufferProxy> v4l2buf = img_buf.dynamic_cast_ptr<V4l2BufferProxy>();
    _ispgain = new SubVideoBuffer (v4l2buf);
    _ispgain->_buf_type = ISP_GAIN;
    _ispgain->set_sequence(img_buf->get_sequence());
    _ispgain->set_buff_info(get_fd_by_index(ispgain->gain_dmaidx), ispgain->gain_size);

    return img_buf;
}

int SPStreamProcUnit::get_lowpass_fv(uint32_t sequence, SmartPtr<V4l2BufferProxy> buf_proxy)
{
#if RKAIQ_HAVE_AF_V20
    SmartPtr<LensHw> lensHw = _focus_dev.dynamic_cast_ptr<LensHw>();
    uint8_t *image_buf = (uint8_t *)buf_proxy->get_v4l2_planar_userptr(0);
    rk_aiq_af_algo_meas_t meas_param;

    _afmeas_param_mutex.lock();
    meas_param = _af_meas_params;
    _afmeas_param_mutex.unlock();

    if (meas_param.sp_meas.enable) {
        meas_param.wina_h_offs /= img_ds_size_x;
        meas_param.wina_v_offs /= img_ds_size_y;
        meas_param.wina_h_size /= img_ds_size_x;
        meas_param.wina_v_size /= img_ds_size_y;
        get_lpfv(sequence, image_buf, af_img_width, af_img_height,
            af_img_width_align, af_img_height_align, pAfTmp, sub_shp4_4,
            sub_shp8_8, high_light, high_light2, &meas_param);

        lensHw->setLowPassFv(sub_shp4_4, sub_shp8_8, high_light, high_light2, sequence);
    }
#endif

    return 0;
}

XCamReturn
SPStreamProcUnit::poll_buffer_ready (SmartPtr<VideoBuffer> &buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_AF_V20
    if (ldg_enable) {
        SmartPtr<V4l2BufferProxy> buf_proxy = buf.dynamic_cast_ptr<V4l2BufferProxy>();
        get_lowpass_fv(buf->get_sequence(), buf_proxy);
    }
#endif

    if (_isp_ver == ISP_V20) {
        if (_camHw->mHwResLintener) {
            _camHw->mHwResLintener->hwResCb(buf);
            SmartPtr<VideoBuffer> vbuf = _ispgain.dynamic_cast_ptr<VideoBuffer>();
            _camHw->mHwResLintener->hwResCb(vbuf);
        }
    }

    return ret;
}

XCamReturn SPStreamProcUnit::prepare(CalibDbV2_Af_LdgParam_t *ldg_param, CalibDbV2_Af_HighLightParam_t *highlight, int width, int height, int stride)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t pixelformat, plane_cnt;
    uint32_t ds_size_w = 4;
    uint32_t ds_size_h = 4;

    if (_isp_ver == ISP_V20) {
        pixelformat = V4L2_PIX_FMT_FBCG;
        plane_cnt = 2;
    } else {
        pixelformat = V4L2_PIX_FMT_NV12;
        plane_cnt = 1;
    }

    if (!width && !height) {
        struct v4l2_subdev_format isp_src_fmt; 
        isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        isp_src_fmt.pad = 2;
        ret = _isp_core_dev->getFormat(isp_src_fmt);
        if (ret) {
            LOGE("get mIspCoreDev src fmt failed !\n");
            return XCAM_RETURN_ERROR_FAILED;
        }
        _src_width           = isp_src_fmt.format.width;
        _src_height          = isp_src_fmt.format.height;
        _ds_width            = (_src_width + ds_size_w - 1) / ds_size_w;
        _ds_height           = (_src_height + ds_size_h - 1) / ds_size_h;
        _ds_width_align      = (_ds_width  + 7) & (~7);
        _ds_height_align     = (_ds_height + 7) & (~7);
        int _stride          = XCAM_ALIGN_UP(_ds_width_align, 64);
        img_ds_size_x        = ds_size_w;
        img_ds_size_y        = ds_size_h;
        LOGD( "set sp format: _src_width %d, _src_height %d, width %d %d height %d %d, stride %d\n",
               _src_width, _src_height, _ds_width, _ds_width_align, _ds_height, _ds_height_align, _stride);
        ret = _dev->set_format(_ds_width_align, _ds_height_align, pixelformat, V4L2_FIELD_NONE, 0);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }

        struct v4l2_format format;
        _dev->get_format (format);
        set_af_img_size(format.fmt.pix_mp.width, format.fmt.pix_mp.height,
                        format.fmt.pix_mp.plane_fmt[0].bytesperline, format.fmt.pix_mp.height);
    } else {
        LOGD( "set sp format: width %d height %d\n", width, height);
        ret = _dev->set_format(width, height, pixelformat, V4L2_FIELD_NONE, stride);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }
    }
    _dev->set_mem_type(V4L2_MEMORY_MMAP);
    _dev->set_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    _dev->set_buffer_count(6);
    _dev->set_mplanes_count(plane_cnt);

    ldg_enable = ldg_param->enable;
    if (ldg_enable) {
        _af_meas_params.sp_meas.ldg_xl      = ldg_param->ldg_xl;
        _af_meas_params.sp_meas.ldg_yl      = ldg_param->ldg_yl;
        _af_meas_params.sp_meas.ldg_kl      = ldg_param->ldg_kl;
        _af_meas_params.sp_meas.ldg_xh      = ldg_param->ldg_xh;
        _af_meas_params.sp_meas.ldg_yh      = ldg_param->ldg_yh;
        _af_meas_params.sp_meas.ldg_kh      = ldg_param->ldg_kh;
        _af_meas_params.sp_meas.highlight_th  = highlight->ther0;
        _af_meas_params.sp_meas.highlight2_th = highlight->ther1;
    }

    return ret;
}

void SPStreamProcUnit::set_devices(CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> isp_core_dev, SmartPtr<V4l2SubDevice> ispp_dev, SmartPtr<V4l2SubDevice> lensdev)
{
    _camHw = camHw;
    _isp_core_dev = isp_core_dev;
    _ispp_dev = ispp_dev;
    _focus_dev = lensdev;
}

bool SPStreamProcUnit::init_fbcbuf_fd()
{
    struct isp2x_buf_idxfd ispbuf_fd;
    int res = -1;

    memset(&ispbuf_fd, 0, sizeof(ispbuf_fd));
    res = _isp_core_dev->io_control(RKISP_CMD_GET_FBCBUF_FD , &ispbuf_fd);
    if (res) {
        LOGE("ioctl RKISP_CMD_GET_FBCBUF_FD failed");
        return false;
    }
    LOGD(" fbc buf num=%d",ispbuf_fd.buf_num);
    for (uint32_t i = 0; i < ispbuf_fd.buf_num; i++) {
        if (ispbuf_fd.dmafd[i] < 0) {
            LOGE("fbcbuf_fd[%u]:%d is illegal!", ispbuf_fd.index[i], ispbuf_fd.dmafd[i]);
            LOGE("\n*** ASSERT: In File %s,line %d ***\n", __FILE__, __LINE__);
            assert(0);
        }
        _buf_fd_map[ispbuf_fd.index[i]] = ispbuf_fd.dmafd[i];
        LOGD("fbcbuf_fd[%u]:%d", ispbuf_fd.index[i], ispbuf_fd.dmafd[i]);
    }
    return true;
}

bool SPStreamProcUnit::deinit_fbcbuf_fd()
{
    std::map<int,int>::iterator it;
     for (it = _buf_fd_map.begin(); it != _buf_fd_map.end(); ++it)
        ::close(it->second);

    _buf_fd_map.clear();
    return true;               
}

int SPStreamProcUnit::get_fd_by_index(int index)
{
    if (index < 0)
        return -1;
    if (_buf_fd_map.find(index) == _buf_fd_map.end())
        return -1;
    return _buf_fd_map[index];               
}

//void SPStreamProcUnit::connect_tnr_stream(SmartPtr<TnrStatsStream> stream)
//{
//    _tnr_stream_unit = stream;
//}

XCamReturn SPStreamProcUnit::get_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h)
{
    width = _ds_width;
    height = _ds_height;
    aligned_w = _ds_width_align;
    aligned_h = _ds_height_align;
    return XCAM_RETURN_NO_ERROR;
}

void SPStreamProcUnit::update_af_meas_params(rk_aiq_af_algo_meas_t *af_meas)
{
    SmartLock locker (_afmeas_param_mutex);
    if (af_meas && (0 != memcmp(af_meas, &_af_meas_params, sizeof(rk_aiq_af_algo_meas_t)))) {
        _af_meas_params = *af_meas;
    }
}

void SPStreamProcUnit::set_af_img_size(int w, int h, int w_align, int h_align)
{
    af_img_width         = w;
    af_img_height        = h;
    af_img_width_align   = w_align;
    af_img_height_align  = h_align;
    LOGI("af_img_width %d af_img_height %d af_img_width_align: %d af_img_height_align: %d\n",
        w, h, w_align, h_align);
}

}

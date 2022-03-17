#include "rk_aiq_comm.h"
#include "rkispp-config.h"
#include "rkisp2-config.h"
#include "SPStreamProcUnit.h"
#include "CamHwIsp20.h"
namespace RkCam {

SPStreamProcUnit::SPStreamProcUnit (SmartPtr<V4l2Device> isp_sp_dev, int type)
    : RKStream(isp_sp_dev, type)
    , _first(true)
{
    setPollCallback (this);
    _ds_width = 0;
    _ds_height = 0;
    _ds_width_align = 0;
    _ds_height_align = 0;
}

SPStreamProcUnit::~SPStreamProcUnit ()
{
}

void SPStreamProcUnit::start()
{
    struct rkispp_trigger_mode tnr_trigger;
    tnr_trigger.module = ISPP_MODULE_TNR;
    tnr_trigger.on = 1;
    int ret = _ispp_dev->io_control(RKISPP_CMD_TRIGGER_MODE, &tnr_trigger);

    RKStream::start();
    return;
}

void SPStreamProcUnit::stop()
{
    struct rkispp_trigger_mode tnr_trigger;
    tnr_trigger.module = ISPP_MODULE_TNR;
    tnr_trigger.on = 0;
    int ret = _ispp_dev->io_control(RKISPP_CMD_TRIGGER_MODE, &tnr_trigger);
    RKStream::stop();//stopDeviceOnly&stopThreadOnly
    deinit_fbcbuf_fd();
}

SmartPtr<VideoBuffer>
SPStreamProcUnit::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{ 
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


XCamReturn
SPStreamProcUnit::poll_buffer_ready (SmartPtr<VideoBuffer> &buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (_camHw->mHwResLintener) {
        _camHw->mHwResLintener->hwResCb(buf);
        SmartPtr<VideoBuffer> vbuf = _ispgain.dynamic_cast_ptr<VideoBuffer>();
        _camHw->mHwResLintener->hwResCb(vbuf);
    }

    return ret;
}


XCamReturn SPStreamProcUnit::prepare(int width, int height, int stride)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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
        _ds_width            = (_src_width + 3) / 4;
        _ds_height           = (_src_height + 7) / 8;
        _ds_width_align      = XCAM_ALIGN_UP(_ds_width, 2);    
        _ds_height_align     = XCAM_ALIGN_UP(_ds_height, 2);
        int _stride           = XCAM_ALIGN_UP(_ds_width_align, 32);
        LOGD( "set sp format: width %d %d height %d %d, stride %d\n",
               _ds_width, _ds_width_align, _ds_height, _ds_height_align, _stride);
        ret = _dev->set_format(_ds_width_align, _ds_height_align, V4L2_PIX_FMT_FBCG, V4L2_FIELD_NONE, _stride);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }
    } else {
        LOGD( "set sp format: width %d height %d\n", width, height);
        ret = _dev->set_format(width, height, V4L2_PIX_FMT_FBCG, V4L2_FIELD_NONE, stride);
        if (ret) {
            LOGE("set isp_sp_dev src fmt failed !\n");
            ret = XCAM_RETURN_ERROR_FAILED;
        }
    }
    _dev->set_mem_type(V4L2_MEMORY_MMAP);
    _dev->set_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    _dev->set_buffer_count(6);
    _dev->set_mplanes_count(2);
    return ret;
}

void SPStreamProcUnit::set_devices(CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> isp_core_dev, SmartPtr<V4l2SubDevice> ispp_dev)
{
    _camHw = camHw;
    _isp_core_dev = isp_core_dev;
    _ispp_dev = ispp_dev;
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
}

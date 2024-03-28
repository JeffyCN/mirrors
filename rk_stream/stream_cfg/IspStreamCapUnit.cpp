#include "IspStreamCapUnit.h"
#include "rkcif-config.h"
#include "MediaInfo.h"
#include "xcam_defs.h"

namespace RkRawStream {

IspStreamCapUnit::IspStreamCapUnit (const rk_sensor_full_info_t *s_info)
    :_mipi_dev_max(0)
    ,_state(CAP_STATE_INVALID)
    ,_memory_type(V4L2_MEMORY_MMAP)
    ,_buffer_count(RKRAWSTREAM_DEF_BUFCNT)
    ,user_on_frame_capture_cb(NULL)
    ,user_priv_data(NULL)
{
    bool linked_to_isp = s_info->linked_to_isp;

    strncpy(_sns_name, s_info->sensor_name.c_str(), 32);

    if (strlen(s_info->isp_info->main_path)) {
        _dev[0] = new V4l2Device (s_info->isp_info->main_path);
        _dev[0]->open();
        _dev[0]->set_mem_type(_memory_type);
    }
    _mipi_dev_max = 1;
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_dev[i].ptr())
            _dev[i]->set_buffer_count(_buffer_count);

        //if (_dev[i].ptr())
        //    _dev[i]->set_buf_sync (true);

        _dev_index[i] = i;
        _stream[i] =  new RKRawStream(_dev[i], i, RKRAWSTREAM_POLL_ISP);
        _stream[i]->setPollCallback(this);
    }

    //_sensor_dev = new V4l2SubDevice(s_info->device_name.c_str());
    //_sensor_dev->open();
    _state = CAP_STATE_INITED;
}

IspStreamCapUnit::~IspStreamCapUnit ()
{
    _state = CAP_STATE_INVALID;
}

XCamReturn IspStreamCapUnit::start()
{
    LOGD_RKSTREAM( "%s enter", __FUNCTION__);
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->start();
    }
    _state = CAP_STATE_STARTED;
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn IspStreamCapUnit::stop ()
{
    LOGD_RKSTREAM( "%s enter", __FUNCTION__);
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopThreadOnly();
    }
    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        buf_list[i].clear ();
    }
    for (int i = 0; i < _mipi_dev_max; i++) {
        user_used_buf_list[i].clear ();
    }
    _buf_mutex.unlock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopDeviceOnly();
    }
    _state = CAP_STATE_STOPPED;
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspStreamCapUnit::prepare(uint8_t buf_memory_type, uint8_t buf_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_RKSTREAM("IspStreamCapUnit %s:buf_type: %d buf_cnt %d\n",__func__, buf_memory_type, buf_cnt);
    for (int i = 0; i < _mipi_dev_max; i++) {
        if(buf_memory_type) {
            _memory_type = (enum v4l2_memory)buf_memory_type;
            _dev[i]->set_mem_type(_memory_type);
        }

        if(buf_cnt) {
            _buffer_count = buf_cnt;
            _dev[i]->set_buffer_count(buf_cnt);
        }

        ret = _dev[i]->prepare();
        if (ret < 0)
            LOGE_RKSTREAM( "mipi tx:%d prepare err: %d\n", i, ret);

        _stream[i]->set_device_prepared(true);
    }
    _state = CAP_STATE_PREPARED;
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
    return ret;
}

void
IspStreamCapUnit::set_isp_format(uint32_t width, uint32_t height)
{
    struct v4l2_format format;
    struct v4l2_selection selection;

    if (width && height) {
        LOGD_RKSTREAM("IspStreamCapUnit %s:%dx%d\n", __func__, width, height);
        memset(&format, 0, sizeof(format));
        memset(&selection, 0, sizeof(selection));
        _dev[0]->get_format(format);
        format.fmt.pix.width = width;
        format.fmt.pix.height = height;
        format.fmt.pix.bytesperline = 0;

        format.fmt.pix_mp.width = width;
        format.fmt.pix_mp.height = height;
        format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
        format.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
        format.fmt.pix_mp.quantization = V4L2_QUANTIZATION_FULL_RANGE;
        for (unsigned i = 0; i < format.fmt.pix_mp.num_planes; i++)
            format.fmt.pix_mp.plane_fmt[i].bytesperline = 0;

        _dev[0]->set_format(format);

        selection.type = format.type;
        selection.target = V4L2_SEL_TGT_CROP;
        selection.flags = 0;
        selection.r.left = 0;
        selection.r.top = 0;
        selection.r.width = width;
        selection.r.height = height;
        _dev[0]->set_selection(selection);
    }

    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
}

/*
void
IspStreamCapUnit::set_working_mode(int mode)
{
    LOGD_RKSTREAM("IspStreamCapUnit %s enter,mode=0x%x", __FUNCTION__, mode);
    _working_mode = mode;

    switch (_working_mode) {
    case RKRAWSTREAM_HDR_MODE_ISP_HDR3:
        _mipi_dev_max = 3;
        break;
    case RKRAWSTREAM_HDR_MODE_ISP_HDR2:
        _mipi_dev_max = 2;
        break;
    default:
        _mipi_dev_max = 1;
    }
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
}

void
IspStreamCapUnit::set_tx_format(uint32_t width, uint32_t height, uint32_t pix_fmt, int mode)
{
    struct v4l2_format format;
    uint32_t w,h,pf;
    memset(&format, 0, sizeof(format));

    LOGD_RKSTREAM("IspStreamCapUnit %s:%dx%d, 0x%x mode %d\n", __func__, width, height, pix_fmt, mode);
    for (int i = 0; i < _mipi_dev_max; i++) {
        _dev[i]->get_format (format);
        w = width? width: format.fmt.pix.width;
        h = height? height: format.fmt.pix.height;
        pf = pix_fmt? pix_fmt: format.fmt.pix.pixelformat;

        int bpp = pixFmt2Bpp(pf);
        int mem_mode = mode;
        int ret1 = _dev[i]->io_control (RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
        if (ret1)
            LOGE_RKSTREAM("set RKCIF_CMD_SET_CSI_MEMORY_MODE failed !\n");

        LOGI_RKSTREAM("IspStreamCapUnit %s:use format %dx%d, 0x%x\n", __func__, w, h, pf);
        _dev[i]->set_format(w, h, pf, V4L2_FIELD_NONE, 0);
    }
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
}

void
IspStreamCapUnit::set_sensor_format(uint32_t width, uint32_t height, uint32_t pixfmt)
{

    LOGD_RKSTREAM("IspStreamCapUnit %s: %dx%d 0x%x", __func__, width, height, pixfmt);
    if(!width && !height && !pixfmt) {
        LOGI_RKSTREAM("%s: skip set sensor format.", __func__);
        return;
    }
    if(_sensor_dev.ptr()){
        struct v4l2_subdev_format format;
        memset(&format, 0, sizeof(format));
        _sensor_dev->getFormat(format);

        format.pad = 0;
        format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        format.format.width = width;
        format.format.height = height;

        if(pixfmt)
            format.format.code = pixfmt;
        _sensor_dev->setFormat(format);
    }
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
}

void
IspStreamCapUnit::set_sensor_mode(uint32_t mode)
{
    LOGD_RKSTREAM("IspStreamCapUnit %s: %d", __func__, mode);
    rkmodule_hdr_cfg hdr_cfg;
    __u32 hdr_mode = NO_HDR;
    if(_sensor_dev.ptr()){
        switch (mode) {
        case RKRAWSTREAM_HDR_MODE_ISP_HDR3:
            hdr_mode = HDR_X3;
            break;
        case RKRAWSTREAM_HDR_MODE_ISP_HDR2:
            hdr_mode = HDR_X2;
            break;
        default:
            hdr_mode = NO_HDR;
        }

        hdr_cfg.hdr_mode = hdr_mode;
        if (_sensor_dev->io_control(RKMODULE_SET_HDR_CFG, &hdr_cfg) < 0) {
            LOGE_RKSTREAM("set_sensor_mode failed to set hdr mode %d\n", hdr_mode);
        }
    }
    LOGD_RKSTREAM( "%s exit", __FUNCTION__);
}
*/

XCamReturn
IspStreamCapUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_RKSTREAM( "%s enter,dev_index=0x%x", __FUNCTION__, dev_index);

    if (ret == XCAM_RETURN_NO_ERROR) {

        if (user_on_frame_capture_cb){
            //struct timespec tx_timestamp;
            //int tx_timems;
            user_takes_buf = false;

            do_capture_callback(buf);

            //clock_gettime(CLOCK_MONOTONIC, &tx_timestamp);
            //tx_timems = XCAM_TIMESPEC_2_USEC(tx_timestamp) / 1000;
            //LOGI_RKSTREAM("BUFDEBUG vicapdq [%s] index %d  seq %d tx_time %d", _sns_name, buf_s->get_v4l2_buf_index(), buf_s->get_sequence(),tx_timems);
            if(user_takes_buf){
                user_used_buf_list[0].push(buf);
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspStreamCapUnit::do_capture_callback
(
    SmartPtr<V4l2BufferProxy> &buf
)
{
    rkrawstream_isp_cb_param_t cb_param;
    cb_param.user_data = user_priv_data;
    cb_param.seq = buf->get_sequence();
    cb_param.fd = buf->get_expbuf_fd();
    cb_param.addr = (void *)buf->get_v4l2_userptr();
    cb_param.size = buf->get_v4l2_buf_planar_length(0);

    if(user_on_frame_capture_cb){
        user_on_frame_capture_cb(&cb_param);
    }

    return XCAM_RETURN_NO_ERROR;
}

void IspStreamCapUnit::release_user_taked_buf(int dev_index)
{
    _buf_mutex.lock();
    if (!user_used_buf_list[dev_index].is_empty()) {
        SmartPtr<V4l2BufferProxy> rx_buf = user_used_buf_list[dev_index].pop(-1);
        struct timespec rx_timestamp;
        clock_gettime(CLOCK_MONOTONIC, &rx_timestamp);
        int64_t rx_timems = XCAM_TIMESPEC_2_USEC(rx_timestamp)  / 1000;
        LOGI_RKSTREAM("BUFDEBUG vicapq [%s] index %d  seq %d  rx_timems %ld \n", _sns_name,
            rx_buf->get_v4l2_buf_index(), rx_buf->get_sequence(),rx_timems);
    }
    _buf_mutex.unlock();
}

/*
void IspStreamCapUnit::set_dma_buf(int dev_index, int buf_index, int fd)
{
    int ret, i;
    struct v4l2_format format;
    i = dev_index;
    SmartPtr<V4l2Buffer> v4l2buf;

    ret = _dev[i]->get_buffer(v4l2buf, buf_index);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_RKSTREAM( "set_dma_buf can not get buffer\n", i);
        return;
    }
    v4l2buf->set_expbuf_fd((const int)fd);
    ret = _dev[i]->queue_buffer(v4l2buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_RKSTREAM( "set_dma_buf queue buffer failed\n", i);
    }
    return;
}
*/

}

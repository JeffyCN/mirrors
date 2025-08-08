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


#include "NrStatsStream.h"
#include "rkispp-config.h"
#include "CamHwIsp20.h"
namespace RkCam {

static void Nr_imag_buf_process
(
    void*                  pUserContext,
    XCamVideoBuffer* pVideoBuffer
)
{
    NrStatsStream* nrStreamProc = (NrStatsStream*)pUserContext;
    nrStreamProc->queue_NRImg_fd(pVideoBuffer->get_fd(pVideoBuffer), pVideoBuffer->frame_id);
}

NrStatsStream::NrStatsStream (SmartPtr<V4l2Device> dev, int type)
    : RKStream(dev, type)
    , _fd_init_flag(true)
    , _NrImage(nullptr)
{
    setPollCallback (this);
}

NrStatsStream::~NrStatsStream()
{
}

void
NrStatsStream::start()
{
    RKStream::start();
}

void
NrStatsStream::stop()
{
    RKStream::stopThreadOnly();
    deinit_nrbuf_fd();
    _list_mutex.lock();
    _NrImg_ready_map.clear();
    _list_mutex.unlock();
    RKStream::stopDeviceOnly();
}

void NrStatsStream::set_device(CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> dev)
{
    _camHw = camHw;
    _ispp_dev = dev;
}

SmartPtr<VideoBuffer>
NrStatsStream::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{ 
    if (_fd_init_flag) {
        init_nrbuf_fd();
        _fd_init_flag = false;

        struct v4l2_subdev_format fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.pad = 0;
        fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        XCamReturn ret = _ispp_dev->getFormat(fmt);
        if (ret) {
            LOGE("get ispp_dev fmt failed !\n");
        }
        _ispp_fmt = fmt;
        LOGD("ispp fmt info: fmt 0x%x, %dx%d !",
                        fmt.format.code, fmt.format.width, fmt.format.height);

        //struct v4l2_format format;
        //memset(&format, 0, sizeof(format));
        //ret = _dev->get_format(format);
        //if (ret) {
        //    LOGE("get ispp_dev get_format failed !\n");
        //} else {
        //    LOGE("nr fmt info: fmt 0x%x, %dx%d !",
        //                    format.fmt.pix.pixelformat, format.fmt.pix.width, format.fmt.pix.height);
        //}
    }
    SmartPtr<V4l2BufferProxy> nrstats = new V4l2BufferProxy (buf, dev);
    nrstats->_buf_type = _dev_type;

    return nrstats;
}

XCamReturn
NrStatsStream::poll_buffer_ready (SmartPtr<VideoBuffer> &buf)
{
    if (_camHw->mHwResLintener) {
        _camHw->mHwResLintener->hwResCb(buf);
        //NR image
        SmartPtr<V4l2BufferProxy> nrstats = buf.dynamic_cast_ptr<V4l2BufferProxy>();
        struct rkispp_stats_nrbuf *stats;
        stats = (struct rkispp_stats_nrbuf *)(nrstats->get_v4l2_userptr());
        struct VideoBufferInfo vbufInfo;
        vbufInfo.init(V4L2_PIX_FMT_NV12, _ispp_fmt.format.width, _ispp_fmt.format.height,
                     _ispp_fmt.format.width, _ispp_fmt.format.height, stats->image.size);
        SmartPtr<VideoBuffer> nrImage = new SubVideoBuffer(
            _buf_num, stats->image.index, get_NRimg_fd_by_index(stats->image.index), vbufInfo);
        nrImage->set_sequence(stats->frame_id);
        nrImage->_buf_type = ISP_NR_IMG;
        _camHw->mHwResLintener->hwResCb(nrImage);
    }

    return XCAM_RETURN_NO_ERROR;
}

bool NrStatsStream::init_nrbuf_fd()
{
    struct rkispp_buf_idxfd isppbuf_fd;
    int res = -1;

    memset(&isppbuf_fd, 0, sizeof(isppbuf_fd));
    res = _ispp_dev->io_control(RKISPP_CMD_GET_NRBUF_FD , &isppbuf_fd);
    if (res)
        return false;
    LOGD("%s: buf_num=%d",__FUNCTION__, isppbuf_fd.buf_num);
    _buf_num = isppbuf_fd.buf_num;
    for (uint32_t i=0; i<isppbuf_fd.buf_num; i++) {
        if (isppbuf_fd.dmafd[i] < 0) {
            LOGE("nrbuf_fd[%u]:%d is illegal!",isppbuf_fd.index[i],isppbuf_fd.dmafd[i]);
            LOGE("\n*** ASSERT: In File %s,line %d ***\n", __FILE__, __LINE__);
            assert(0);
        }
        _idx_array[i] = isppbuf_fd.index[i];
        _fd_array[i] = isppbuf_fd.dmafd[i];
        LOGD("nrbuf_fd[%u]:%d",isppbuf_fd.index[i],isppbuf_fd.dmafd[i]);
    }

    return true;
}

int NrStatsStream::get_NRimg_fd_by_index(int index)
{
    int ret = -1;

    if (index < 0)
        return ret;
    for (int i=0; i<_buf_num; i++) {
        if (_idx_array[i] == index)
            return _fd_array[i];
    }
    return ret;
}

bool NrStatsStream::deinit_nrbuf_fd()
{ 
    for (int i=0; i<_buf_num; i++) {
        ::close(_fd_array[i]);
    }
    _buf_num = 0;
    return true;               
}

int NrStatsStream::queue_NRImg_fd(int fd, uint32_t frameid)
{
    if (fd < 0)
        return -1;
    _list_mutex.lock();
    _NrImg_ready_map[frameid] = fd;
    _list_mutex.unlock();
    return 0;
}


int NrStatsStream::get_NRImg_fd(uint32_t frameid)
{
    int fd = -1;
    std::map<uint32_t, int>::iterator it;
    _list_mutex.lock();
    it = _NrImg_ready_map.find(frameid);
    if (it == _NrImg_ready_map.end())
        fd = -1;
    else
        fd = it->second;
    _list_mutex.unlock();
    return fd;
}

} //namspace RkCam

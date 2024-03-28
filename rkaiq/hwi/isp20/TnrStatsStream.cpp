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

#include "TnrStatsStream.h"
#include "SPStreamProcUnit.h"
#include "rkispp-config.h"
#include "CamHwIsp20.h"
namespace RkCam {


TnrStatsStream::TnrStatsStream (SmartPtr<V4l2Device> dev, int type)
: RKStream(dev, type)
, _fd_init_flag(true)
{
    setPollCallback (this);
}

TnrStatsStream::~TnrStatsStream()
{
}

void
TnrStatsStream::start()
{
    RKStream::start();
}

void
TnrStatsStream::stop()
{
    RKStream::stopThreadOnly();
    deinit_tnrbuf();
    RKStream::stopDeviceOnly();
}

void TnrStatsStream::set_device(CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> dev)
{
    _ispp_dev = dev;
    _camHw = camHw;
}

SmartPtr<VideoBuffer>
TnrStatsStream::new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev)
{
    if (_fd_init_flag) {
        init_tnrbuf();
        _fd_init_flag = false;
    }

    struct rkispp_stats_tnrbuf *stats;
    //gainkg
    SmartPtr<SubV4l2BufferProxy> gainkg = new SubV4l2BufferProxy(buf, dev);
    stats = (struct rkispp_stats_tnrbuf *)(gainkg->get_v4l2_userptr());
    LOGD("%s:kg_indx=%d,g_indx=%d,frame_id=%d\n",__FUNCTION__, stats->gainkg.index, stats->gain.index, stats->frame_id);
    gainkg->set_buff_info(get_fd_by_index(stats->gainkg.index), stats->gainkg.size);
    gainkg->_buf_type = ISPP_GAIN_KG;

    return gainkg;
}

XCamReturn
TnrStatsStream::poll_buffer_ready (SmartPtr<VideoBuffer> &buf)
{
    if (_camHw->mHwResLintener) {
        struct rkispp_stats_tnrbuf *stats;
        SmartPtr<SubV4l2BufferProxy> gainkg = buf.dynamic_cast_ptr<SubV4l2BufferProxy>();
        stats = (struct rkispp_stats_tnrbuf *)(gainkg->get_v4l2_userptr());
        //gainwr
        SmartPtr<V4l2BufferProxy> v4l2buf = gainkg.dynamic_cast_ptr<V4l2BufferProxy>();
        SmartPtr<SubVideoBuffer> gainwr = new SubVideoBuffer(v4l2buf);
        gainwr->_buf_type = ISPP_GAIN_WR;
        gainwr->set_buff_info(get_fd_by_index(stats->gain.index), stats->gain.size);
        gainwr->set_sequence(stats->frame_id);
        SmartPtr<VideoBuffer> vbuf = gainwr.dynamic_cast_ptr<VideoBuffer>();
        _camHw->mHwResLintener->hwResCb(vbuf); //send gainwr
        _camHw->mHwResLintener->hwResCb(buf); //send gainkg

    }

    return XCAM_RETURN_NO_ERROR;
}

int TnrStatsStream::get_fd_by_index(uint32_t index)
{
#if 0
    if (index < 0)
        return -1;
    for (int i=0; i<_buf_num; i++) {
        if (index == (int)_idx_array[i]) {
            return _fd_array[i];
        }
    }
    return -1;
#else
    int fd = -1;
    std::map<uint32_t, int>::iterator it;
    it = _idx_fd_map.find(index);
    if (it == _idx_fd_map.end())
        fd = -1;
    else
        fd = it->second;
    return fd;
#endif
}

bool TnrStatsStream::init_tnrbuf()
{
    struct rkispp_buf_idxfd isppbuf_fd;
    int res = -1;

    memset(&isppbuf_fd, 0, sizeof(isppbuf_fd));
    res = _ispp_dev->io_control(RKISPP_CMD_GET_TNRBUF_FD , &isppbuf_fd);
    if (res)
        return false;
    LOGD("tnr buf_num=%d",isppbuf_fd.buf_num);
    for (uint32_t i=0; i<isppbuf_fd.buf_num; i++) {
        if (isppbuf_fd.dmafd[i] < 0) {
            LOGE("tnrbuf_fd[%u]:%d is illegal!",isppbuf_fd.index[i],isppbuf_fd.dmafd[i]);
            LOGE("\n*** ASSERT: In File %s,line %d ***\n", __FILE__, __LINE__);
            assert(0);
        }
        _fd_array[i] = isppbuf_fd.dmafd[i];
        _idx_array[i] = isppbuf_fd.index[i];
        _idx_fd_map[isppbuf_fd.index[i]] = isppbuf_fd.dmafd[i];
        LOGD("tnrbuf_fd[%u]:%d",isppbuf_fd.index[i],isppbuf_fd.dmafd[i]);
    }
    _buf_num = isppbuf_fd.buf_num;
    return true;
}

void
TnrStatsStream::deinit_tnrbuf()
{
    LOGD("%s enter", __FUNCTION__);
    std::map<uint32_t,int>::iterator it;
     for (it=_idx_fd_map.begin(); it!=_idx_fd_map.end(); ++it)
        ::close(it->second);

    _idx_fd_map.clear();
    LOGD("%s exit", __FUNCTION__);
}

//void TnrStatsStream::connect_nr_stream(SmartPtr<NrStatsStream> stream)
//{
//    _nr_stream_unit = stream;
//}

} //namspace RkCam

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

#include "FakeCamHwIsp20.h"
#include "Isp20Evts.h"
#include "FakeSensorHw.h"
#include "rk_isp20_hw.h"
#include "Isp20_module_dbg.h"
#include "mediactl/mediactl-priv.h"
#include <linux/v4l2-subdev.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace RkCam {
FakeCamHwIsp20::FakeCamHwIsp20() : isp_index(0)
{
    ENTER_CAMHW_FUNCTION();
    _rx_memory_type = V4L2_MEMORY_DMABUF;
    _tx_memory_type = V4L2_MEMORY_DMABUF;
    mNoReadBack = false;
    EXIT_CAMHW_FUNCTION();
}

FakeCamHwIsp20::~FakeCamHwIsp20()
{
    ENTER_CAMHW_FUNCTION();
    if (!use_rkrawstream)
        setupOffLineLink(isp_index, false);
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp20::init(const char* sns_ent_name)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = CamHwIsp20::init(sns_ent_name);
    if (ret)
        return ret;

    // special for fake sensor
    SmartPtr<BaseSensorHw> sensorHw;
    mSensorDev->close();
    sensorHw = new FakeSensorHw();
    mSensorDev = sensorHw;
    mSensorDev->open();
    mIspStatsStream->set_event_handle_dev(sensorHw);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeCamHwIsp20::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<BaseSensorHw> sensorHw;

    ENTER_CAMHW_FUNCTION();

    SmartPtr<FakeSensorHw> fakeSensorHw = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    fakeSensorHw->use_rkrawstream = use_rkrawstream;

    std::unordered_map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator it;
    if ((it = mSensorHwInfos.find(sns_name)) == mSensorHwInfos.end()) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    rk_sensor_full_info_t *s_info = it->second.ptr();
    isp_index = s_info->isp_info->logic_id;

    if (!use_rkrawstream) {
        setupOffLineLink(isp_index, true);
        init_mipi_devices(s_info);
        fakeSensorHw->set_mipi_tx_devs(_mipi_tx_devs);

        mRawCapUnit->set_tx_devices(_mipi_tx_devs);
        mRawProcUnit->set_rx_devices(_mipi_rx_devs);
        mRawProcUnit->setPollCallback(this);
    }

    ret = CamHwIsp20::prepare(width, height, mode, t_delay, g_delay);
    if (ret)
        return ret;

    if (mIspSofStream.ptr())
        mIspSofStream->setPollCallback (NULL);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
FakeCamHwIsp20::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    // notify fakesensor one frame has been processed by ISP
    return fakeSensor->on_dqueue(dev_index, buf);
}

XCamReturn
FakeCamHwIsp20::init_mipi_devices(rk_sensor_full_info_t *s_info)
{
    /*
     * for _mipi_tx_devs, index 0 refer to short frame always, inedex 1 refer
     * to middle frame always, index 2 refert to long frame always.
     * for CIF usecase, because mipi_id0 refert to long frame always, so we
     * should know the HDR mode firstly befor building the relationship between
     * _mipi_tx_devs array and mipi_idx. here we just set the mipi_idx to
     * _mipi_tx_devs, we will build the real relation in start.
     * for CIF usecase, rawwr2_path is always connected to _mipi_tx_devs[0],
     * rawwr0_path is always connected to _mipi_tx_devs[1], and rawwr1_path is always
     * connected to _mipi_tx_devs[0]
     */
    //short frame
    _mipi_tx_devs[0] = new FakeV4l2Device ();
    _mipi_tx_devs[0]->open();
    _mipi_tx_devs[0]->set_mem_type(_tx_memory_type);
    _mipi_tx_devs[0]->set_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

    _mipi_rx_devs[0] = new V4l2Device (s_info->isp_info->rawrd2_s_path);//rkisp_rawrd2_s
    _mipi_rx_devs[0]->open();
    _mipi_rx_devs[0]->set_mem_type(_rx_memory_type);
    //mid frame
    _mipi_tx_devs[1] = new FakeV4l2Device ();
    _mipi_tx_devs[1]->open();
    _mipi_tx_devs[1]->set_mem_type(_tx_memory_type);
    _mipi_tx_devs[1]->set_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

    _mipi_rx_devs[1] = new V4l2Device (s_info->isp_info->rawrd0_m_path);//rkisp_rawrd0_m
    _mipi_rx_devs[1]->open();
    _mipi_rx_devs[1]->set_mem_type(_rx_memory_type);
    //long frame
    _mipi_tx_devs[2] = new FakeV4l2Device ();
    _mipi_tx_devs[2]->open();
    _mipi_tx_devs[2]->set_mem_type(_tx_memory_type);
    _mipi_tx_devs[2]->set_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

    _mipi_rx_devs[2] = new V4l2Device (s_info->isp_info->rawrd1_l_path);//rkisp_rawrd1_l
    _mipi_rx_devs[2]->open();
    _mipi_rx_devs[2]->set_mem_type(_rx_memory_type);
    for (int i = 0; i < 3; i++) {
        if (_linked_to_isp) {
            if (_rawbuf_type == RK_AIQ_RAW_FILE) {
                _mipi_tx_devs[0]->set_use_type(2);
                _mipi_tx_devs[i]->set_buffer_count(1);
                _mipi_rx_devs[i]->set_buffer_count(1);
            } else if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 _mipi_tx_devs[0]->set_use_type(1);
                _mipi_tx_devs[i]->set_buffer_count(ISP_TX_BUF_NUM);
                _mipi_rx_devs[i]->set_buffer_count(ISP_TX_BUF_NUM);
            } else {
                _mipi_tx_devs[i]->set_buffer_count(ISP_TX_BUF_NUM);
                _mipi_rx_devs[i]->set_buffer_count(ISP_TX_BUF_NUM);
            }
        } else {
            _mipi_tx_devs[i]->set_buffer_count(VIPCAP_TX_BUF_NUM);
            _mipi_rx_devs[i]->set_buffer_count(VIPCAP_TX_BUF_NUM);
        }
        SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
        rk_aiq_exposure_sensor_descriptor sns_des;
        fakeSensor->get_sensor_desc(&sns_des);
        _mipi_tx_devs[i]->set_format(sns_des.sensor_output_width,
                                     sns_des.sensor_output_height,
                                     sns_des.sensor_pixelformat);
        _mipi_rx_devs[i]->set_format(sns_des.sensor_output_width,
                                     sns_des.sensor_output_height,
                                     sns_des.sensor_pixelformat);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeCamHwIsp20::poll_event_ready (uint32_t sequence, int type)
{
    struct v4l2_event event;
    event.u.frame_sync.frame_sequence = sequence;

    mIspSofStream->stop();
    SmartPtr<VideoBuffer> buf =
        mIspSofStream->new_video_buffer(event, NULL);

    CamHwBase::poll_buffer_ready(buf);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeCamHwIsp20::enqueueRawBuffer(void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rk_aiq_vbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    parse_rk_rawdata(rawdata, &vbuf);
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    fakeSensor->enqueue_rawbuffer(&vbuf, sync);
    poll_event_ready(vbuf.buf_info[0].frame_id, V4L2_EVENT_FRAME_SYNC);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
FakeCamHwIsp20::enqueueRawFile(const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rk_aiq_vbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    if (0 != access(path, F_OK)) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "file: %s is not exist!", path);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "open file: %s failed", path);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = parse_rk_rawfile(fp, &vbuf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Failed to parse rk rawfile!");
        fclose(fp);
        return ret;
    }
    fclose(fp);
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    fakeSensor->enqueue_rawbuffer(&vbuf, true);
    poll_event_ready(vbuf.buf_info[0].frame_id, V4L2_EVENT_FRAME_SYNC);
    EXIT_XCORE_FUNCTION();
    return ret;
}

void
FakeCamHwIsp20::parse_rk_rawdata(void *rawdata, struct rk_aiq_vbuf *vbuf)
{
    unsigned short tag = 0;
    struct _block_header header;
    uint8_t *p = (uint8_t *)rawdata;
    uint8_t *actual_raw[3] = {NULL, NULL, NULL};
    int actual_raw_len[3] = {0, 0, 0};
    bool is_actual_rawdata = false;
    bool bExit = false;
    while(!bExit){
        tag = *((unsigned short*)p);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "tag=0x%04x\n",tag);
        switch (tag)
        {
        	case START_TAG:
            	p = p+TAG_BYTE_LEN;
            	memset(_st_addr, 0, sizeof(_st_addr));
            	memset(&_rawfmt, 0, sizeof(_rawfmt));
            	memset(&_finfo, 0, sizeof(_finfo));
            	break;
        	case NORMAL_RAW_TAG:
        	{
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    _st_addr[0] = *((struct _st_addrinfo*)p);
                }else{
                    //actual raw data
                    is_actual_rawdata = true;
                    actual_raw[0] = p;
                    actual_raw_len[0] = header.block_length;
                }
                p = p + header.block_length;

            	break;
        	}
        	case HDR_S_RAW_TAG:
        	{
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    _st_addr[0] = *((struct _st_addrinfo*)p);
                }else{
                    //actual raw data
                    is_actual_rawdata = true;
                    actual_raw[0] = p;
                    actual_raw_len[0] = header.block_length;
                }
                p = p + header.block_length;
        	    break;
        	}
        	case HDR_M_RAW_TAG:
        	{
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    _st_addr[1] = *((struct _st_addrinfo*)p);
                }else{
                    //actual raw data
                    is_actual_rawdata = true;
                    actual_raw[1] = p;
                    actual_raw_len[1] = header.block_length;
                }
                p = p + header.block_length;
            	break;
        	}
        	case HDR_L_RAW_TAG:
        	{
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    _st_addr[2] = *((struct _st_addrinfo*)p);
                }else{
                    //actual raw data
                    is_actual_rawdata = true;
                    actual_raw[2] = p;
                    actual_raw_len[2] = header.block_length;
                }
                p = p + header.block_length;
            	break;
        	}
        	case FORMAT_TAG:
        	{
            	_rawfmt = *((struct _raw_format *)p);
            	LOGD_CAMHW_SUBM(FAKECAM_SUBM, "hdr_mode=%d,bayer_fmt=%d\n",_rawfmt.hdr_mode,_rawfmt.bayer_fmt);
            	p = p + sizeof(struct _block_header) + _rawfmt.size;
            	break;
        	}
        	case STATS_TAG:
        	{
            	_finfo = *((rk_aiq_frame_info_t *)p);
            	p = p + sizeof(struct _block_header) + _finfo.size;
            	break;
        	}
        	case ISP_REG_FMT_TAG:
        	{
        	    header = *((struct _block_header *)p);
        	    p += sizeof(struct _block_header);
        	    p = p + header.block_length;
        	    break;
        	}
        	case ISP_REG_TAG:
        	{
        	    header = *((struct _block_header *)p);
        	    p += sizeof(struct _block_header);
        	    p = p + header.block_length;
        	    break;
        	}
        	case ISPP_REG_FMT_TAG:
        	{
        	    header = *((struct _block_header *)p);
        	    p += sizeof(struct _block_header);
        	    p = p + header.block_length;
        	    break;
        	}
        	case ISPP_REG_TAG:
        	{
        	    header = *((struct _block_header *)p);
        	    p += sizeof(struct _block_header);
        	    p = p + header.block_length;
        	    break;
        	}
        	case PLATFORM_TAG:
        	{
            	header = *((struct _block_header *)p);
        	    p += sizeof(struct _block_header);
        	    p = p + header.block_length;
        	    break;
        	}
        	case END_TAG:
        	{
                bExit = true;
            	break;
            }
            default:
            {
            	LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Not support TAG(0x%04x)\n", tag);
            	bExit = true;
            	break;
        	}
        }
    }

     vbuf->frame_width = _rawfmt.width;
     vbuf->frame_height = _rawfmt.height;
     vbuf->base_addr = rawdata;
     if (_rawfmt.hdr_mode == 1) {
         if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr = actual_raw[0];
            vbuf->buf_info[0].data_fd = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[0].data_addr = (uint8_t*)(long)(_st_addr[0].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[0].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);
                 }

                 vbuf->buf_info[0].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[0].data_fd   = _st_addr[0].fd;
                 vbuf->buf_info[0].data_addr = NULL;
             }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
          LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr=%p,fd=%d,length=%d\n",
                                       vbuf->buf_info[0].data_addr,
                                       vbuf->buf_info[0].data_fd,
                                       vbuf->buf_info[0].data_length);

         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.normal_gain;
         vbuf->buf_info[0].exp_time = (float)_finfo.normal_exp;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.normal_gain_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.normal_exp_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);
    } else if (_rawfmt.hdr_mode == 2) {
         if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr = actual_raw[0];
            vbuf->buf_info[0].data_fd = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[0].data_addr = (uint8_t*)(long)(_st_addr[0].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[0].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);
                 }
                 vbuf->buf_info[0].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[0].data_addr = NULL;
                 vbuf->buf_info[0].data_fd   = _st_addr[0].fd;
             }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.hdr_gain_s;
         vbuf->buf_info[0].exp_time = (float)_finfo.hdr_exp_s;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);
         if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr = actual_raw[1];//actual_raw[1]
            vbuf->buf_info[1].data_fd = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[1];//actual_raw_len[1]
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[1].data_addr = (uint8_t*)(long)(_st_addr[1].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[1].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[1].haddr << 32) + _st_addr[1].laddr);
                 }
                 vbuf->buf_info[1].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[1].data_addr = NULL;
                 vbuf->buf_info[1].data_fd   = _st_addr[1].fd;
             }
             vbuf->buf_info[1].data_length = _st_addr[1].size;
         }
         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (float)_finfo.hdr_gain_m;
         vbuf->buf_info[1].exp_time = (float)_finfo.hdr_exp_m;
         vbuf->buf_info[1].exp_gain_reg = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time_reg = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time,
                                      vbuf->buf_info[1].exp_gain_reg,
                                      vbuf->buf_info[1].exp_time_reg);
    } else if (_rawfmt.hdr_mode == 3) {
         if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr = actual_raw[0];
            vbuf->buf_info[0].data_fd = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[0].data_addr = (uint8_t*)(long)(_st_addr[0].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[0].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);
                 }
                 vbuf->buf_info[0].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[0].data_addr = NULL;
                 vbuf->buf_info[0].data_fd   = _st_addr[0].fd;
             }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.hdr_gain_s;
         vbuf->buf_info[0].exp_time = (float)_finfo.hdr_exp_s;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);

         if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr = actual_raw[1];
            vbuf->buf_info[1].data_fd = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[1];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[1].data_addr = (uint8_t*)(long)(_st_addr[1].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[1].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[1].haddr << 32) + _st_addr[1].laddr);
                 }
                 vbuf->buf_info[1].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[1].data_addr = NULL;
                 vbuf->buf_info[1].data_fd   = _st_addr[1].fd;
             }
             vbuf->buf_info[1].data_length = _st_addr[1].size;
         }
         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (float)_finfo.hdr_gain_m;
         vbuf->buf_info[1].exp_time = (float)_finfo.hdr_exp_m;
         vbuf->buf_info[1].exp_gain_reg = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time_reg = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time,
                                      vbuf->buf_info[1].exp_gain_reg,
                                      vbuf->buf_info[1].exp_time_reg);

         if (is_actual_rawdata) {
            vbuf->buf_info[2].data_addr = actual_raw[2];
            vbuf->buf_info[2].data_fd = 0;
            vbuf->buf_info[2].data_length = actual_raw_len[2];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4) {
                     vbuf->buf_info[2].data_addr = (uint8_t*)(long)(_st_addr[2].laddr);
                 } else if (sizeof(uint8_t*) == 8) {
                     vbuf->buf_info[2].data_addr =
                         (uint8_t*)(((uint64_t)_st_addr[2].haddr << 32) + _st_addr[2].laddr);
                 }
                 vbuf->buf_info[2].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[2].data_addr = NULL;
                 vbuf->buf_info[2].data_fd   = _st_addr[2].fd;
             }
             vbuf->buf_info[2].data_length = _st_addr[2].size;
         }
         vbuf->buf_info[2].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[2].exp_gain = (float)_finfo.hdr_gain_l;
         vbuf->buf_info[2].exp_time = (float)_finfo.hdr_exp_l;
         vbuf->buf_info[2].exp_gain_reg = (uint32_t)_finfo.hdr_gain_l_reg;
         vbuf->buf_info[2].exp_time_reg = (uint32_t)_finfo.hdr_exp_l_reg;
         vbuf->buf_info[2].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[2]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[2].data_addr,
                                      vbuf->buf_info[2].data_fd,
                                      vbuf->buf_info[2].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[2]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[2].exp_gain,
                                      vbuf->buf_info[2].exp_time,
                                      vbuf->buf_info[2].exp_gain_reg,
                                      vbuf->buf_info[2].exp_time_reg);
    }

}

XCamReturn
FakeCamHwIsp20::parse_rk_rawfile(FILE *fp, struct rk_aiq_vbuf *vbuf)
{
    unsigned short tag = 0;
    struct _block_header header;
    bool bExit = false;

    if (!_mipi_rx_devs[0]->is_activated()) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "device(%s) hasn't activated(%d) yet!!!\n",
                        XCAM_STR (_mipi_rx_devs[0]->get_device_name()),
                        _mipi_rx_devs[0]->is_activated());
        return XCAM_RETURN_ERROR_FAILED;
    }

    while(!bExit){
        int ret = fread(&tag, sizeof(tag), 1, fp);
        if (ret == 0)
            break;
        fseek(fp, TAG_BYTE_LEN*(-1), SEEK_CUR);//backforwad to tag start
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "tag=0x%04x\n",tag);
        switch (tag)
        {
        	case START_TAG:
            	fseek(fp, TAG_BYTE_LEN, SEEK_CUR);
            	memset(_st_addr, 0, sizeof(_st_addr));
            	memset(&_rawfmt, 0, sizeof(_rawfmt));
            	memset(&_finfo, 0, sizeof(_finfo));
            	break;
        	case NORMAL_RAW_TAG:
        	{
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[0].data_addr = (uint8_t*)_mipi_rx_devs[0]->get_buffer_by_index(0)->get_expbuf_usrptr();
                    fread(vbuf->buf_info[0].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[0].data_length = header.block_length;
                }
            	break;
        	}
        	case HDR_S_RAW_TAG:
        	{
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[0].data_addr = (uint8_t*)_mipi_rx_devs[0]->get_buffer_by_index(0)->get_expbuf_usrptr();
                    fread(vbuf->buf_info[0].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[0].data_length = header.block_length;
                }
        	    break;
        	}
        	case HDR_M_RAW_TAG:
        	{
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[1].data_addr = (uint8_t*)_mipi_rx_devs[1]->get_buffer_by_index(0)->get_expbuf_usrptr();
                    fread(vbuf->buf_info[1].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[1].data_length = header.block_length;
                }
            	break;
        	}
        	case HDR_L_RAW_TAG:
        	{
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[2].data_addr = (uint8_t*)_mipi_rx_devs[1]->get_buffer_by_index(0)->get_expbuf_usrptr();
                    fread(vbuf->buf_info[2].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[2].data_length = header.block_length;
                }
            	break;
        	}
        	case FORMAT_TAG:
        	{
            	fread(&_rawfmt, sizeof(_rawfmt), 1, fp);
            	LOGD_CAMHW_SUBM(FAKECAM_SUBM, "hdr_mode=%d,bayer_fmt=%d\n",_rawfmt.hdr_mode,_rawfmt.bayer_fmt);
            	break;
        	}
        	case STATS_TAG:
        	{
            	fread(&_finfo, sizeof(_finfo), 1, fp);
            	break;
        	}
        	case ISP_REG_FMT_TAG:
        	{
        	    fread(&header, sizeof(header), 1, fp);
        	    fseek(fp, header.block_length, SEEK_CUR);
        	    break;
        	}
        	case ISP_REG_TAG:
        	{
        	    fread(&header, sizeof(header), 1, fp);
        	    fseek(fp, header.block_length, SEEK_CUR);
        	    break;
        	}
        	case ISPP_REG_FMT_TAG:
        	{
        	    fread(&header, sizeof(header), 1, fp);
        	    fseek(fp, header.block_length, SEEK_CUR);
        	    break;
        	}
        	case ISPP_REG_TAG:
        	{
        	    fread(&header, sizeof(header), 1, fp);
        	    fseek(fp, header.block_length, SEEK_CUR);
        	    break;
        	}
        	case PLATFORM_TAG:
        	{
            	fread(&header, sizeof(header), 1, fp);
        	    fseek(fp, header.block_length, SEEK_CUR);
        	    break;
        	}
        	case END_TAG:
        	{
                bExit = true;
            	break;
            }
            default:
            {
            	LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Not support TAG(0x%04x)\n", tag);
            	bExit = true;
            	break;
        	}
        }
    }

     vbuf->frame_width = _rawfmt.width;
     vbuf->frame_height = _rawfmt.height;
     if (_rawfmt.hdr_mode == 1) {
          LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr=%p,fd=%d,length=%d\n",
                                       vbuf->buf_info[0].data_addr,
                                       vbuf->buf_info[0].data_fd,
                                       vbuf->buf_info[0].data_length);

         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.normal_gain;
         vbuf->buf_info[0].exp_time = (float)_finfo.normal_exp;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.normal_gain_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.normal_exp_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);
    }else if (_rawfmt.hdr_mode == 2) {
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.hdr_gain_s;
         vbuf->buf_info[0].exp_time = (float)_finfo.hdr_exp_s;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);

         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (float)_finfo.hdr_gain_m;
         vbuf->buf_info[1].exp_time = (float)_finfo.hdr_exp_m;
         vbuf->buf_info[1].exp_gain_reg = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time_reg = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time,
                                      vbuf->buf_info[1].exp_gain_reg,
                                      vbuf->buf_info[1].exp_time_reg);
    }else if (_rawfmt.hdr_mode == 3) {
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (float)_finfo.hdr_gain_s;
         vbuf->buf_info[0].exp_time = (float)_finfo.hdr_exp_s;
         vbuf->buf_info[0].exp_gain_reg = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time_reg = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time,
                                      vbuf->buf_info[0].exp_gain_reg,
                                      vbuf->buf_info[0].exp_time_reg);

         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (float)_finfo.hdr_gain_m;
         vbuf->buf_info[1].exp_time = (float)_finfo.hdr_exp_m;
         vbuf->buf_info[1].exp_gain_reg = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time_reg = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time,
                                      vbuf->buf_info[1].exp_gain_reg,
                                      vbuf->buf_info[1].exp_time_reg);

         vbuf->buf_info[2].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[2].exp_gain = (float)_finfo.hdr_gain_l;
         vbuf->buf_info[2].exp_time = (float)_finfo.hdr_exp_l;
         vbuf->buf_info[2].exp_gain_reg = (uint32_t)_finfo.hdr_gain_l_reg;
         vbuf->buf_info[2].exp_time_reg = (uint32_t)_finfo.hdr_exp_l_reg;
         vbuf->buf_info[2].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[2]: data_addr=%p,fd=%d,,length=%d\n",
                                      vbuf->buf_info[2].data_addr,
                                      vbuf->buf_info[2].data_fd,
                                      vbuf->buf_info[2].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"buf_info[2]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                                      vbuf->buf_info[2].exp_gain,
                                      vbuf->buf_info[2].exp_time,
                                      vbuf->buf_info[2].exp_gain_reg,
                                      vbuf->buf_info[2].exp_time_reg);
    }

     return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeCamHwIsp20::registRawdataCb(void (*callback)(void *))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    ret = fakeSensor->register_rawdata_callback(callback);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
FakeCamHwIsp20::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    _rawbuf_type = prop.rawbuf_type;
    if (_rawbuf_type == RK_AIQ_RAW_ADDR)
    {
        _rx_memory_type = V4L2_MEMORY_USERPTR;
        _tx_memory_type = V4L2_MEMORY_USERPTR;
    }
    else if (_rawbuf_type == RK_AIQ_RAW_FD)
    {
        _rx_memory_type = V4L2_MEMORY_DMABUF;
        _tx_memory_type = V4L2_MEMORY_DMABUF;
    }
    else if(_rawbuf_type == RK_AIQ_RAW_DATA)
    {
        _rx_memory_type = V4L2_MEMORY_MMAP;
        _tx_memory_type = V4L2_MEMORY_USERPTR;
    }
    else if(_rawbuf_type == RK_AIQ_RAW_FILE)
    {
        _rx_memory_type = V4L2_MEMORY_MMAP;
        _tx_memory_type = V4L2_MEMORY_USERPTR;
    }
    else {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM,"Not support raw data type:%d", _rawbuf_type);
        return XCAM_RETURN_ERROR_PARAM;
    }
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    ret = fakeSensor->prepare(prop);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn FakeCamHwIsp20::setupOffLineLink(int isp_index, bool enable)
{
    media_device* device  = NULL;
    media_entity* entity  = NULL;
    media_pad* src_pad    = NULL;
    media_pad* sink_pad   = NULL;
    int lvds_max_entities = 6;
    int lvds_entity       = 0;

    device = media_device_new(mIspHwInfos.isp_info[isp_index].media_dev_path);
    if (!device) return XCAM_RETURN_ERROR_FAILED;

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (!entity) {
        goto FAIL;
    }

    sink_pad = (media_pad*)media_entity_get_pad(entity, 0);
    if (!sink_pad) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get rkisp-isp-subdev sink pad failed!\n");
        goto FAIL;
    }

    for (lvds_entity = 0; lvds_entity < lvds_max_entities; lvds_entity++) {
        char entity_name[128] = {0};
        src_pad               = NULL;
        if (!lvds_entity) {
          snprintf(entity_name, 128, "rkcif-mipi-lvds");
        } else {
          snprintf(entity_name, 128, "rkcif-mipi-lvds%d", lvds_entity);
        }
        entity = media_get_entity_by_name(device, entity_name, strlen(entity_name));
        if (entity) {
            src_pad = (media_pad*)media_entity_get_pad(entity, 0);
            if (!src_pad) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get rkcif-mipi-lvds%d source pad s failed!\n",
                                lvds_entity);
                goto FAIL;
            }
        }

        if (src_pad && sink_pad) {
            if (enable) {
                media_setup_link(device, src_pad, sink_pad, 0);
            } else
                media_setup_link(device, src_pad, sink_pad, MEDIA_LNK_FL_ENABLED);
        }
    }

    media_device_unref(device);
    return XCAM_RETURN_NO_ERROR;
FAIL:
    media_device_unref(device);
    return XCAM_RETURN_ERROR_FAILED;
}

FakeCamHwIsp21::FakeCamHwIsp21()
: FakeCamHwIsp20() {
}

FakeCamHwIsp21::~FakeCamHwIsp21()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp21::init(const char* sns_ent_name)
{
    return FakeCamHwIsp20::init(sns_ent_name);
}

XCamReturn
FakeCamHwIsp21::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    return FakeCamHwIsp20::prepare(width, height, mode, t_delay, g_delay);
}

XCamReturn
FakeCamHwIsp21::enqueueRawBuffer(void *rawdata, bool sync)
{
   return  FakeCamHwIsp20::enqueueRawBuffer(rawdata, sync);
}

XCamReturn
FakeCamHwIsp21::enqueueRawFile(const char *path)
{
   return  FakeCamHwIsp20::enqueueRawFile(path);
}

XCamReturn
FakeCamHwIsp21::registRawdataCb(void (*callback)(void *))
{
   return  FakeCamHwIsp20::registRawdataCb(callback);
}

XCamReturn
FakeCamHwIsp21::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
   return  FakeCamHwIsp20::rawdataPrepare(prop);
}

XCamReturn
FakeCamHwIsp21::poll_event_ready (uint32_t sequence, int type)
{
   return  FakeCamHwIsp20::poll_event_ready (sequence, type);
}

FakeCamHwIsp3x::FakeCamHwIsp3x()
: FakeCamHwIsp20() {
}

FakeCamHwIsp3x::~FakeCamHwIsp3x()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp3x::init(const char* sns_ent_name)
{
    return FakeCamHwIsp20::init(sns_ent_name);
}

XCamReturn
FakeCamHwIsp3x::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    return FakeCamHwIsp20::prepare(width, height, mode, t_delay, g_delay);
}

XCamReturn
FakeCamHwIsp3x::enqueueRawBuffer(void *rawdata, bool sync)
{
   return  FakeCamHwIsp20::enqueueRawBuffer(rawdata, sync);
}

XCamReturn
FakeCamHwIsp3x::enqueueRawFile(const char *path)
{
   return  FakeCamHwIsp20::enqueueRawFile(path);
}

XCamReturn
FakeCamHwIsp3x::registRawdataCb(void (*callback)(void *))
{
   return  FakeCamHwIsp20::registRawdataCb(callback);
}

XCamReturn
FakeCamHwIsp3x::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
   return  FakeCamHwIsp20::rawdataPrepare(prop);
}

XCamReturn
FakeCamHwIsp3x::poll_event_ready (uint32_t sequence, int type)
{
   return  FakeCamHwIsp20::poll_event_ready (sequence, type);
}

FakeCamHwIsp32::FakeCamHwIsp32()
: FakeCamHwIsp20() {
}

FakeCamHwIsp32::~FakeCamHwIsp32()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp32::init(const char* sns_ent_name)
{
    return FakeCamHwIsp20::init(sns_ent_name);
}

XCamReturn
FakeCamHwIsp32::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    return FakeCamHwIsp20::prepare(width, height, mode, t_delay, g_delay);
}

XCamReturn
FakeCamHwIsp32::enqueueRawBuffer(void *rawdata, bool sync)
{
   return  FakeCamHwIsp20::enqueueRawBuffer(rawdata, sync);
}

XCamReturn
FakeCamHwIsp32::enqueueRawFile(const char *path)
{
   return  FakeCamHwIsp20::enqueueRawFile(path);
}

XCamReturn
FakeCamHwIsp32::registRawdataCb(void (*callback)(void *))
{
   return  FakeCamHwIsp20::registRawdataCb(callback);
}

XCamReturn
FakeCamHwIsp32::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
   return  FakeCamHwIsp20::rawdataPrepare(prop);
}

XCamReturn
FakeCamHwIsp32::poll_event_ready (uint32_t sequence, int type)
{
   return  FakeCamHwIsp20::poll_event_ready (sequence, type);
}

#if defined(ISP_HW_V39)
FakeCamHwIsp39::FakeCamHwIsp39()
: FakeCamHwIsp20() {
}

FakeCamHwIsp39::~FakeCamHwIsp39()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp39::init(const char* sns_ent_name)
{
    return FakeCamHwIsp20::init(sns_ent_name);
}

XCamReturn
FakeCamHwIsp39::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    return FakeCamHwIsp20::prepare(width, height, mode, t_delay, g_delay);
}

XCamReturn
FakeCamHwIsp39::enqueueRawBuffer(void *rawdata, bool sync)
{
   return  FakeCamHwIsp20::enqueueRawBuffer(rawdata, sync);
}

XCamReturn
FakeCamHwIsp39::enqueueRawFile(const char *path)
{
   return  FakeCamHwIsp20::enqueueRawFile(path);
}

XCamReturn
FakeCamHwIsp39::registRawdataCb(void (*callback)(void *))
{
   return  FakeCamHwIsp20::registRawdataCb(callback);
}

XCamReturn
FakeCamHwIsp39::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
   return  FakeCamHwIsp20::rawdataPrepare(prop);
}

XCamReturn
FakeCamHwIsp39::poll_event_ready (uint32_t sequence, int type)
{
   return  FakeCamHwIsp20::poll_event_ready (sequence, type);
}
#endif

} //namspace RkCam

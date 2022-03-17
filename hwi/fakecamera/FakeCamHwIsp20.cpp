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
#include "FakeSensorHw.h"
#include "Isp20PollThread.h"
#include "rk_isp20_hw.h"
#include "Isp20_module_dbg.h"
#include "mediactl/mediactl-priv.h"
#include <linux/v4l2-subdev.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace RkCam {
FakeCamHwIsp20::FakeCamHwIsp20()
{
    ENTER_CAMHW_FUNCTION();
    _rx_memory_type = V4L2_MEMORY_DMABUF;
    _tx_memory_type = V4L2_MEMORY_DMABUF;
    EXIT_CAMHW_FUNCTION();
}

FakeCamHwIsp20::~FakeCamHwIsp20()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
FakeCamHwIsp20::init(const char* sns_ent_name)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<Isp20PollThread> isp20Pollthread;
    SmartPtr<PollThread> isp20LumaPollthread;
    SmartPtr<PollThread> isp20IsppPollthread;
    SmartPtr<BaseSensorHw> sensorHw;
    SmartPtr<LensHw> lensHw;
    std::string sensor_name(sns_ent_name);

    ENTER_CAMHW_FUNCTION();


    std::map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator it;
    if ((it = mSensorHwInfos.find(sensor_name)) == mSensorHwInfos.end()) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "can't find sensor %s", sns_ent_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }
    rk_sensor_full_info_t *s_info = it->second.ptr();
    sensorHw = new FakeSensorHw();
    mSensorDev = sensorHw;
    mSensorDev->open();
    strncpy(sns_name, sns_ent_name, sizeof(sns_name));

    if (s_info->linked_to_isp)
        _linked_to_isp = true;

    mIspCoreDev = new V4l2SubDevice(s_info->isp_info->isp_dev_path);
    mIspCoreDev->open();
    if (s_info->linked_to_isp)
        mIspCoreDev->subscribe_event(V4L2_EVENT_FRAME_SYNC);

    if (_linked_to_isp) {
        mIspLumaDev = new V4l2Device(s_info->isp_info->mipi_luma_path);
    } else
        mIspLumaDev = new V4l2Device(s_info->cif_info->mipi_luma_path);
    mIspLumaDev->open();

    _ispp_sd = new V4l2SubDevice(s_info->ispp_info->pp_dev_path);
    _ispp_sd ->open();
    LOGI_CAMHW_SUBM(FAKECAM_SUBM, "pp_dev_path: %s\n", s_info->ispp_info->pp_dev_path);

#ifndef DISABLE_PP_STATS
    mIsppStatsDev = new V4l2Device(s_info->ispp_info->pp_stats_path);
    mIsppStatsDev->open();
#endif
    mIsppParamsDev = new V4l2Device (s_info->ispp_info->pp_input_params_path);
    mIsppParamsDev->open();

    mIspStatsDev = new V4l2Device (s_info->isp_info->stats_path);
    mIspStatsDev->open();
    mIspParamsDev = new V4l2Device (s_info->isp_info->input_params_path);
    mIspParamsDev->open();

    if (!s_info->module_lens_dev_name.empty()) {
        lensHw = new LensHw(s_info->module_lens_dev_name.c_str());
        mLensDev = lensHw;
        mLensDev->open();
    }

    if(!s_info->module_ircut_dev_name.empty()) {
        mIrcutDev = new V4l2SubDevice(s_info->module_ircut_dev_name.c_str());
        mIrcutDev->open();
    }

    if (!_linked_to_isp) {
        if (strlen(s_info->cif_info->mipi_csi2_sd_path) > 0)
            _cif_csi2_sd = new V4l2SubDevice (s_info->cif_info->mipi_csi2_sd_path);
        else if (strlen(s_info->cif_info->lvds_sd_path) > 0)
            _cif_csi2_sd = new V4l2SubDevice (s_info->cif_info->lvds_sd_path);
        else
            LOGW_CAMHW_SUBM(FAKECAM_SUBM, "_cif_csi2_sd is null! \n");

        _cif_csi2_sd->open();
        _cif_csi2_sd->subscribe_event(V4L2_EVENT_FRAME_SYNC);
    }

    isp20Pollthread = new Isp20PollThread();
    isp20Pollthread->set_event_handle_dev(sensorHw);
    if(lensHw.ptr()) {
        isp20Pollthread->set_focus_handle_dev(lensHw);
        //isp20Pollthread->set_iris_handle_dev(lensHw);
    }

    mPollthread = isp20Pollthread;
    if (_linked_to_isp)
        mPollthread->set_event_device(mIspCoreDev);
    else
        mPollthread->set_event_device(_cif_csi2_sd);
    mPollthread->set_isp_stats_device(mIspStatsDev);
    mPollthread->set_isp_params_devices(mIspParamsDev, mIsppParamsDev);
    mPollthread->set_poll_callback (this);

    isp20LumaPollthread = new PollThread();
    mPollLumathread = isp20LumaPollthread;
    mPollLumathread->set_isp_luma_device(mIspLumaDev);
    mPollLumathread->set_poll_callback (this);

#ifndef DISABLE_PP
#ifndef DISABLE_PP_STATS
    isp20IsppPollthread = new PollThread();
    mPollIsppthread = isp20IsppPollthread;
    mPollIsppthread->set_ispp_stats_device(mIsppStatsDev);
    mPollIsppthread->set_poll_callback (this);
#endif
#endif

    if (s_info->flash_num) {
        mFlashLight = new FlashLightHw(s_info->module_flash_dev_name, s_info->flash_num);
        mFlashLight->init(s_info->flash_num);
    }
    if (s_info->flash_ir_num) {
        mFlashLightIr = new FlashLightHw(s_info->module_flash_ir_dev_name, s_info->flash_ir_num);
        mFlashLightIr->init(s_info->flash_ir_num);
    }

    xcam_mem_clear (_full_active_isp_params);
    xcam_mem_clear (_full_active_ispp_params);

    _state = CAM_HW_STATE_INITED;

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeCamHwIsp20::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<Isp20PollThread> isp20Pollthread;
    SmartPtr<BaseSensorHw> sensorHw;

    ENTER_CAMHW_FUNCTION();

    XCAM_ASSERT (mCalibDb);

    _hdr_mode = mode;

    Isp20Params::set_working_mode(_hdr_mode);

    std::map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator it;
    if ((it = mSensorHwInfos.find(sns_name)) == mSensorHwInfos.end()) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "can't find sensor %s", sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    rk_sensor_full_info_t *s_info = it->second.ptr();
    int isp_index = s_info->isp_info->model_idx;
    LOGI_CAMHW_SUBM(FAKECAM_SUBM, "sensor_name(%s) is linked to isp_index(%d)",
                    sns_name, isp_index);
    if (_linked_to_isp) {
        if (_hdr_mode != RK_AIQ_WORKING_MODE_NORMAL) {
            setupHdrLink(RK_AIQ_WORKING_MODE_ISP_HDR3, isp_index, true);
        } else {
            if (mNoReadBack)
                setupHdrLink(RK_AIQ_WORKING_MODE_ISP_HDR3, isp_index, false);
            else
                setupHdrLink(RK_AIQ_WORKING_MODE_ISP_HDR3, isp_index, true);
        }
    } else {
        setupHdrLink(RK_AIQ_WORKING_MODE_ISP_HDR3, isp_index, true);
        int cif_index = s_info->cif_info->model_idx;
        setupHdrLink_vidcap(_hdr_mode, cif_index, true);
    }

    sensorHw = mSensorDev.dynamic_cast_ptr<BaseSensorHw>();
    ret = sensorHw->set_working_mode(mode);
    if (ret) {
        LOGW_CAMHW_SUBM(FAKECAM_SUBM, "set sensor mode error !");
        return ret;
    }

    setExpDelayInfo(mode);
    setLensVcmCfg();
    init_mipi_devices(s_info);

    isp20Pollthread = mPollthread.dynamic_cast_ptr<Isp20PollThread>();
    isp20Pollthread->set_working_mode(mode, _linked_to_isp, false);
    isp20Pollthread->setCamHw(this);
    isp20Pollthread->set_mipi_devs(_mipi_tx_devs, _mipi_rx_devs, mIspCoreDev);

    SmartPtr<FakeSensorHw> fakeSensorHw = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    fakeSensorHw->set_mipi_tx_devs(_mipi_tx_devs);

    _ispp_module_init_ens = 0;

    ret = setupPipelineFmt();
    if (ret < 0) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "setupPipelineFmt err: %d\n", ret);
    }

    if (!_linked_to_isp)
        prepare_cif_mipi();

    isp20Pollthread->set_event_handle_dev(sensorHw);

    _state = CAM_HW_STATE_PREPARED;
    EXIT_CAMHW_FUNCTION();
    return ret;
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
        _mipi_tx_devs[i]->set_buf_sync (true);
        _mipi_rx_devs[i]->set_buf_sync (true);
    }
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

    parse_rk_rawfile(fp, &vbuf);
    fclose(fp);
    SmartPtr<FakeSensorHw> fakeSensor = mSensorDev.dynamic_cast_ptr<FakeSensorHw>();
    fakeSensor->enqueue_rawbuffer(&vbuf, true);
    EXIT_XCORE_FUNCTION();
    return ret;
}

void
FakeCamHwIsp20::parse_rk_rawdata(void *rawdata, struct rk_aiq_vbuf *vbuf)
{
    unsigned short tag = 0;
    struct _block_header header;
    uint8_t *p = (uint8_t *)rawdata;
    uint8_t *actual_raw[3];
    int actual_raw_len[3];
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
            	_finfo = *((struct _frame_info *)p);
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
         }else{
			 if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4)
                    vbuf->buf_info[0].data_addr = (uint8_t*)(_st_addr[0].laddr);
                 else if (sizeof(uint8_t*) == 8)
                    vbuf->buf_info[0].data_addr = (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);

                vbuf->buf_info[0].data_fd = 0;
			 } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                 vbuf->buf_info[0].data_fd = _st_addr[0].fd;
				 vbuf->buf_info[0].data_addr = NULL;
			 }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
          LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr=%p,fd=%d,length=%d\n",
                                       vbuf->buf_info[0].data_addr,
                                       vbuf->buf_info[0].data_fd,
                                       vbuf->buf_info[0].data_length);

         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.normal_gain_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.normal_exp_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain:0x%x,time:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);
    }else if (_rawfmt.hdr_mode == 2) {
         if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr = actual_raw[0];
            vbuf->buf_info[0].data_fd = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
         } else {
            if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
             if (sizeof(uint8_t*) == 4)
                vbuf->buf_info[0].data_addr = (uint8_t*)(_st_addr[0].laddr);
             else if (sizeof(uint8_t*) == 8)
                vbuf->buf_info[0].data_addr = (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);
                vbuf->buf_info[0].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[0].data_addr = NULL;
                vbuf->buf_info[0].data_fd = _st_addr[0].fd;
             }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[0]=%p,fd[0]=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[0]:0x%x,time[0]:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);
         if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr = actual_raw[2];//actual_raw[1]
            vbuf->buf_info[1].data_fd = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[2];//actual_raw_len[1]
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4)
                    vbuf->buf_info[1].data_addr = (uint8_t*)(_st_addr[1].laddr);
                 else if (sizeof(uint8_t*) == 8)
                    vbuf->buf_info[1].data_addr = (uint8_t*)(((uint64_t)_st_addr[1].haddr << 32) + _st_addr[1].laddr);
                 vbuf->buf_info[1].data_fd = 0;
              } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[1].data_addr = NULL;
                vbuf->buf_info[1].data_fd = _st_addr[1].fd;
              }
              vbuf->buf_info[1].data_length = _st_addr[1].size;
         }
         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[1]=%p,fd[1]=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[1]:0x%x,time[1]:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time);
    }else if (_rawfmt.hdr_mode == 3) {
         if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr = actual_raw[0];
            vbuf->buf_info[0].data_fd = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
         } else {
            if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
             if (sizeof(uint8_t*) == 4)
                vbuf->buf_info[0].data_addr = (uint8_t*)(_st_addr[0].laddr);
             else if (sizeof(uint8_t*) == 8)
                vbuf->buf_info[0].data_addr = (uint8_t*)(((uint64_t)_st_addr[0].haddr << 32) + _st_addr[0].laddr);
                vbuf->buf_info[0].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[0].data_addr = NULL;
                vbuf->buf_info[0].data_fd = _st_addr[0].fd;
             }
             vbuf->buf_info[0].data_length = _st_addr[0].size;
         }
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[0]=%p,fd[0]=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[0]:0x%x,time[0]:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);

         if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr = actual_raw[1];
            vbuf->buf_info[1].data_fd = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[1];
         } else {
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4)
                    vbuf->buf_info[1].data_addr = (uint8_t*)(_st_addr[1].laddr);
                 else if (sizeof(uint8_t*) == 8)
                    vbuf->buf_info[1].data_addr = (uint8_t*)(((uint64_t)_st_addr[1].haddr << 32) + _st_addr[1].laddr);
                 vbuf->buf_info[1].data_fd = 0;
              } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[1].data_addr = NULL;
                vbuf->buf_info[1].data_fd = _st_addr[1].fd;
              }
              vbuf->buf_info[1].data_length = _st_addr[1].size;
         }
         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[1]=%p,fd[1]=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[1]:0x%x,time[1]:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time);

         if (is_actual_rawdata) {
            vbuf->buf_info[2].data_addr = actual_raw[2];
            vbuf->buf_info[2].data_fd = 0;
            vbuf->buf_info[2].data_length = actual_raw_len[2];
         }else{
             if (_rawbuf_type == RK_AIQ_RAW_ADDR) {
                 if (sizeof(uint8_t*) == 4)
                    vbuf->buf_info[2].data_addr = (uint8_t*)(_st_addr[2].laddr);
                 else if (sizeof(uint8_t*) == 8)
                    vbuf->buf_info[2].data_addr = (uint8_t*)(((uint64_t)_st_addr[2].haddr << 32) + _st_addr[2].laddr);
                 vbuf->buf_info[2].data_fd = 0;
             } else if (_rawbuf_type == RK_AIQ_RAW_FD) {
                    vbuf->buf_info[2].data_addr = NULL;
                    vbuf->buf_info[2].data_fd = _st_addr[2].fd;
             }
             vbuf->buf_info[2].data_length = _st_addr[2].size;
         }
         vbuf->buf_info[2].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[2].exp_gain = (uint32_t)_finfo.hdr_gain_l_reg;
         vbuf->buf_info[2].exp_time = (uint32_t)_finfo.hdr_exp_l_reg;
         vbuf->buf_info[2].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[2]=%p,fd[2]=%d,,length=%d\n",
                                      vbuf->buf_info[2].data_addr,
                                      vbuf->buf_info[2].data_fd,
                                      vbuf->buf_info[2].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[2]:0x%x,time[2]:0x%x\n",
                                      vbuf->buf_info[2].exp_gain,
                                      vbuf->buf_info[2].exp_time);
    }

}

void
FakeCamHwIsp20::parse_rk_rawfile(FILE *fp, struct rk_aiq_vbuf *vbuf)
{
    unsigned short tag = 0;
    struct _block_header header;
    bool bExit = false;
    while(!bExit){
        fread(&tag, sizeof(tag), 1, fp);
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
                    vbuf->buf_info[1].data_addr = (uint8_t*)_mipi_rx_devs[1]->get_buffer_by_index(0)->get_expbuf_usrptr();
                    fread(vbuf->buf_info[1].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[1].data_length = header.block_length;
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
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.normal_gain_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.normal_exp_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain:0x%x,time:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);
    }else if (_rawfmt.hdr_mode == 2) {
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[0]=%p,fd[0]=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[0]:0x%x,time[0]:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);

         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (uint32_t)_finfo.hdr_gain_l_reg;
         vbuf->buf_info[1].exp_time = (uint32_t)_finfo.hdr_exp_l_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[1]=%p,fd[1]=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[1]:0x%x,time[1]:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time);
    }else if (_rawfmt.hdr_mode == 3) {
         vbuf->buf_info[0].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[0].exp_gain = (uint32_t)_finfo.hdr_gain_s_reg;
         vbuf->buf_info[0].exp_time = (uint32_t)_finfo.hdr_exp_s_reg;
         vbuf->buf_info[0].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[0]=%p,fd[0]=%d,,length=%d\n",
                                      vbuf->buf_info[0].data_addr,
                                      vbuf->buf_info[0].data_fd,
                                      vbuf->buf_info[0].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[0]:0x%x,time[0]:0x%x\n",
                                      vbuf->buf_info[0].exp_gain,
                                      vbuf->buf_info[0].exp_time);

         vbuf->buf_info[1].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[1].exp_gain = (uint32_t)_finfo.hdr_gain_m_reg;
         vbuf->buf_info[1].exp_time = (uint32_t)_finfo.hdr_exp_m_reg;
         vbuf->buf_info[1].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[1]=%p,fd[1]=%d,,length=%d\n",
                                      vbuf->buf_info[1].data_addr,
                                      vbuf->buf_info[1].data_fd,
                                      vbuf->buf_info[1].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[1]:0x%x,time[1]:0x%x\n",
                                      vbuf->buf_info[1].exp_gain,
                                      vbuf->buf_info[1].exp_time);

         vbuf->buf_info[2].frame_id = _rawfmt.frame_id;
         vbuf->buf_info[2].exp_gain = (uint32_t)_finfo.hdr_gain_l_reg;
         vbuf->buf_info[2].exp_time = (uint32_t)_finfo.hdr_exp_l_reg;
         vbuf->buf_info[2].valid = true;
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"data_addr[2]=%p,fd[2]=%d,,length=%d\n",
                                      vbuf->buf_info[2].data_addr,
                                      vbuf->buf_info[2].data_fd,
                                      vbuf->buf_info[2].data_length);
         LOGD_CAMHW_SUBM(FAKECAM_SUBM,"gain[2]:0x%x,time[2]:0x%x\n",
                                      vbuf->buf_info[2].exp_gain,
                                      vbuf->buf_info[2].exp_time);
    }

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
}; //namspace RkCam

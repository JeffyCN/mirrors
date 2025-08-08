#include "rk_aiq_comm.h"
#include "RawStreamProcUnit.h"
#include "CaptureRawData.h"
#include "CamHwIsp20.h"

#ifdef ANDROID_OS
#include <cutils/properties.h>
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 32
#endif
#endif

namespace RkCam {
RawStreamProcUnit::RawStreamProcUnit ()
    : _is_multi_cam_conc(false)
    , _first_trigger(true)
    , _is_1608_sensor(false)
    , dumpRkRawType(DUMP_RKRAW_DEFAULT)
{
    _raw_proc_thread = new RawProcThread(this);
    _PollCallback = NULL;
    mCamPhyId = -1;
    _rawCap = NULL;
}

RawStreamProcUnit::RawStreamProcUnit (const rk_sensor_full_info_t *s_info, bool linked_to_isp, int tx_buf_cnt)
    : _is_multi_cam_conc(false)
    , _first_trigger(true)
    , _is_1608_sensor(false)
    ,dumpRkRawType(DUMP_RKRAW_DEFAULT)
{
    _raw_proc_thread = new RawProcThread(this);
    _PollCallback = NULL;
    _rawCap = NULL;
    //short frame
    if (strlen(s_info->isp_info->rawrd2_s_path)) {
        _dev[0] = new V4l2Device (s_info->isp_info->rawrd2_s_path);//rkisp_rawrd2_s
        _dev[0]->open();
        _dev[0]->set_mem_type(V4L2_MEMORY_DMABUF);
    }
    //mid frame
    if (strlen(s_info->isp_info->rawrd0_m_path)) {
        _dev[1] = new V4l2Device (s_info->isp_info->rawrd0_m_path);//rkisp_rawrd0_m
        _dev[1]->open();
        _dev[1]->set_mem_type(V4L2_MEMORY_DMABUF);
    }
    //long frame
    if (strlen(s_info->isp_info->rawrd1_l_path)) {
        _dev[2] = new V4l2Device (s_info->isp_info->rawrd1_l_path);//rkisp_rawrd1_l
        _dev[2]->open();
        _dev[2]->set_mem_type(V4L2_MEMORY_DMABUF);
    }
    int buf_cnt = 0;
    if (tx_buf_cnt > 0) {
        buf_cnt = tx_buf_cnt;
    } else if (linked_to_isp) {
        buf_cnt = ISP_TX_BUF_NUM;
    } else if (s_info->linked_to_1608) {
        buf_cnt = VIPCAP_TX_BUF_NUM_1608;
    } else {
        buf_cnt = VIPCAP_TX_BUF_NUM;
    }
    for (int i = 0; i < 3; i++) {
        if (_dev[i].ptr()) {
            _dev[i]->set_buffer_count(buf_cnt);
            _dev[i]->set_buf_sync (true);
        }

        _dev_index[i] = i;
        _stream[i] =  new RKRawStream(_dev[i], i, ISP_POLL_RX);
        _stream[i]->setPollCallback(this);
    }
}

RawStreamProcUnit::~RawStreamProcUnit ()
{
}

XCamReturn RawStreamProcUnit::start(int mode)
{
#ifdef UseCaptureRawData
    _rawCap = new CaptureRawData(mCamPhyId);
#else
    if (getDumpRkRawType() == DUMP_RKRAW1)
        _rawCap = new DumpRkRaw1(mCamPhyId);
    else
        _rawCap = new DumpRkRaw2(mCamPhyId);
#endif
    setIspInfoToDump();

    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->setCamPhyId(mCamPhyId);
        _stream[i]->start();
    }
    _msg_queue.clear();

    // TODO: [baron] for rk1608, vi will capture data before start
    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        buf_list[i].clear ();
        cache_list[i].clear ();
    }
    _isp_hdr_fid2ready_map.clear();
    _buf_mutex.unlock();

    _raw_proc_thread->set_policy(SCHED_RR);
    _raw_proc_thread->set_priority(20);
    _raw_proc_thread->start();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RawStreamProcUnit::stop ()
{
    _msg_queue.push(0);
    _raw_proc_thread->stop();
    _msg_queue.clear();

    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopThreadOnly();
    }

    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        buf_list[i].clear ();
        cache_list[i].clear ();
    }
    _isp_hdr_fid2ready_map.clear();
    _buf_mutex.unlock();

    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map.clear();
    _sof_timestamp_map.clear();
    _mipi_trigger_mutex.unlock();

    if (_rawCap) {
        delete _rawCap;
        _rawCap = NULL;
    }

    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopDeviceOnly();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RawStreamProcUnit::prepare(int idx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // mipi rx/tx format should match to sensor.
    for (int i = 0; i < 3; i++) {
        if (!(idx & (1 << i)))
            continue;

        ret = _dev[i]->prepare();
        if (ret < 0)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM,"mipi tx:%d prepare err: %d\n", ret);

        _stream[i]->set_device_prepared(true);
    }
    return ret;
}

void
RawStreamProcUnit::set_working_mode(int mode)
{
    _working_mode = mode;

    switch (_working_mode) {
    case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
        _mipi_dev_max = 3;
        break;
    case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
        _mipi_dev_max = 2;
        break;
    default:
        _mipi_dev_max = 1;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM,"working_mode:0x%x, _mipi_dev_max=%d\n", _working_mode, _mipi_dev_max);
}

void
RawStreamProcUnit::set_rx_devices(SmartPtr<V4l2Device> mipi_rx_devs[3])
{
    for (int i = 0; i < 3; i++) {
        _dev[i] = mipi_rx_devs[i];
        _stream[i].release();
        _stream[i] =  new RKRawStream(_dev[i], i, ISP_POLL_RX);
        _stream[i]->setPollCallback(this);
    }
}

SmartPtr<V4l2Device>
RawStreamProcUnit::get_rx_device(int index)
{
    if (index > _mipi_dev_max)
        return nullptr;
    else
        return _dev[index];
}

XCamReturn
RawStreamProcUnit::set_rx_format(const struct v4l2_subdev_format& sns_sd_fmt,
                                 uint32_t sns_v4l_pix_fmt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < 3; i++) {
        if (!_dev[i].ptr()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        ret = _dev[i]->set_format(sns_sd_fmt.format.width,
                                  sns_sd_fmt.format.height,
                                  sns_v4l_pix_fmt,
                                  V4L2_FIELD_NONE,
                                  0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed", _dev[i]->get_device_name());
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            _dev[i]->get_device_name(), sns_v4l_pix_fmt,
                            sns_sd_fmt.format.width, sns_sd_fmt.format.height);
        }
    }

    return ret;
}

XCamReturn
RawStreamProcUnit::set_rx_format(const struct v4l2_subdev_selection& sns_sd_sel,
                                 uint32_t sns_v4l_pix_fmt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < 3; i++) {
        if (!_dev[i].ptr()) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        ret = _dev[i]->set_format(sns_sd_sel.r.width,
                                  sns_sd_sel.r.height,
                                  sns_v4l_pix_fmt,
                                  V4L2_FIELD_NONE,
                                  0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed", _dev[i]->get_device_name());
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set rx fmt info: fmt 0x%x, %dx%d !",
                            _dev[i]->get_device_name(), sns_v4l_pix_fmt,
                            sns_sd_sel.r.width, sns_sd_sel.r.height);
        }
    }

    return ret;
}

void
RawStreamProcUnit::set_devices(SmartPtr<V4l2SubDevice> ispdev, CamHwIsp20* handle)
{
    _isp_core_dev = ispdev;
    _camHw = handle;
}

XCamReturn
RawStreamProcUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    SmartPtr<V4l2BufferProxy> rx_buf = nullptr;

    {
        SmartLock locker (_buf_mutex);

        if (!buf_list[dev_index].is_empty()) {
            rx_buf = buf_list[dev_index].pop(-1);
            LOGV_CAMHW_SUBM(ISP20HW_SUBM,"%s dev_index:%d index:%d fd:%d, seq:%d\n",
                    __func__, dev_index, rx_buf->get_v4l2_buf_index(),
                    rx_buf->get_expbuf_fd(), rx_buf->get_sequence());
        }
        if (_PollCallback)
            _PollCallback->poll_buffer_ready (buf, dev_index);
    }

    if (dev_index == 0)
        DumpRkRawInFrameEnd(rx_buf);

    return XCAM_RETURN_NO_ERROR;
}

void
RawStreamProcUnit::set_hdr_frame_readback_infos(uint32_t frame_id, int times)
{
    if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        return;

    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map[frame_id] = times;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "rdtimes seq %d \n", frame_id);
//    trigger_isp_readback();
    _mipi_trigger_mutex.unlock();
}

void
RawStreamProcUnit::match_lumadetect_map(uint32_t sequence, sint32_t &additional_times)
{
    std::map<uint32_t, int>::iterator it_times_del;
    _mipi_trigger_mutex.lock();
    for (std::map<uint32_t, int>::iterator iter = _isp_hdr_fid2times_map.begin();
            iter != _isp_hdr_fid2times_map.end();) {
        if (iter->first < sequence) {
            it_times_del = iter++;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "del seq %d", it_times_del->first);
            iter = _isp_hdr_fid2times_map.erase(it_times_del);
        } else if (iter->first == sequence) {
            additional_times = iter->second;
            it_times_del = iter++;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "del seq %d", it_times_del->first);
            iter = _isp_hdr_fid2times_map.erase(it_times_del);
            break;
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "%s missing rdtimes for buf_seq %d, min rdtimes_seq %d !",
                            __func__, sequence, iter->first);
            additional_times = 0;
            break;
        }
    }
    _mipi_trigger_mutex.unlock();
}

void
RawStreamProcUnit::match_globaltmostate_map(uint32_t sequence, bool &isHdrGlobalTmo)
{
    std::map<uint32_t, bool>::iterator it_del;
    _mipi_trigger_mutex.lock();
    for (std::map<uint32_t, bool>::iterator iter = _hdr_global_tmo_state_map.begin();
            iter !=  _hdr_global_tmo_state_map.end();) {
        if (iter->first < sequence) {
            it_del = iter++;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "del seq %d", it_del->first);
            iter = _hdr_global_tmo_state_map.erase(it_del);
        } else if (iter->first == sequence) {
            isHdrGlobalTmo = iter->second;
            it_del = iter++;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "del seq %d", it_del->first);
            iter = _hdr_global_tmo_state_map.erase(it_del);
            break;
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "%s missing tmo state for buf_seq %d, min rdtimes_seq %d !",
                            __func__, sequence, iter->first);
            break;
        }
    }
    _mipi_trigger_mutex.unlock();
}

XCamReturn
RawStreamProcUnit::match_sof_timestamp_map(sint32_t sequence, uint64_t &timestamp)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    std::map<int, uint64_t>::iterator it;
    sint32_t search_id = sequence < 0 ? 0 : sequence;

    it = _sof_timestamp_map.find(search_id);
    if (it != _sof_timestamp_map.end()) {
        timestamp = it->second;
    } else {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM,  "can't find frameid(%d), get sof timestamp failed!\n",
                sequence);
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

void
RawStreamProcUnit::set_hdr_global_tmo_mode(uint32_t frame_id, bool mode)
{
    _mipi_trigger_mutex.lock();
    _hdr_global_tmo_state_map[frame_id] = mode;
    _mipi_trigger_mutex.unlock();
}

void
RawStreamProcUnit::notify_sof(uint64_t time, uint32_t frameid)
{
    _mipi_trigger_mutex.lock();
    while (_sof_timestamp_map.size() > 8) {
        _sof_timestamp_map.erase(_sof_timestamp_map.begin());
    }
    _sof_timestamp_map[frameid] = time;
    _mipi_trigger_mutex.unlock();
}

bool RawStreamProcUnit::raw_buffer_proc()
{
    LOGD("%s enter", __FUNCTION__);
    int ret = true;
    auto v  = _msg_queue.front();
    if (v)
        trigger_isp_readback();
    else
        ret = false;
    _msg_queue.pop();
    LOGD("%s exit", __FUNCTION__);
    return ret;
}

void
RawStreamProcUnit::send_sync_buf
(
    SmartPtr<V4l2BufferProxy> &buf_s,
    SmartPtr<V4l2BufferProxy> &buf_m,
    SmartPtr<V4l2BufferProxy> &buf_l
)
{
     _buf_mutex.lock();
     for (int i = 0; i < _mipi_dev_max; i++) {
        if (i == ISP_MIPI_HDR_S)
            cache_list[ISP_MIPI_HDR_S].push(buf_s);
        else if (i == ISP_MIPI_HDR_M)
            cache_list[ISP_MIPI_HDR_M].push(buf_m);
        else if (i == ISP_MIPI_HDR_L)
            cache_list[ISP_MIPI_HDR_L].push(buf_l);
     }
     _isp_hdr_fid2ready_map[buf_s->get_sequence()] = true;
     _buf_mutex.unlock();
     _msg_queue.push(1);
}

void
RawStreamProcUnit::trigger_isp_readback()
{
    std::map<uint32_t, bool>::iterator it_ready;
    SmartPtr<V4l2Buffer> v4l2buf[3];
    SmartPtr<V4l2BufferProxy> buf_proxy;
    uint32_t sequence = -1;
    sint32_t additional_times = -1;
    bool isHdrGlobalTmo = false;

    SmartLock locker (_buf_mutex);

    if (_isp_hdr_fid2ready_map.size() == 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s buf not ready !", __func__);
        return;
    }

    it_ready = _isp_hdr_fid2ready_map.begin();
    sequence = it_ready->first;

    if ( _working_mode != RK_AIQ_WORKING_MODE_NORMAL) {
        match_lumadetect_map(sequence, additional_times);
        if (additional_times == -1) {
        //    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s rdtimes not ready for seq %d !", __func__, sequence);
        //    return;
        additional_times = 0;//add by zyl
        }

        match_globaltmostate_map(sequence, isHdrGlobalTmo);
        //if (isHdrGlobalTmo && !_camHw->getDhazState())
        //    additional_times = 0;
    } else {
        additional_times = 0;
    }
    _isp_hdr_fid2ready_map.erase(it_ready);

    if (_camHw) {
        {
            int ret = XCAM_RETURN_NO_ERROR;

            // whether to start capturing raw files
            if (_rawCap) {
#ifdef UseCaptureRawData
                _rawCap->detect_capture_raw_status(sequence, _first_trigger);
#else
                _rawCap->detectDumpStatus(sequence);
#endif
            }

            //CaptureRawData::getInstance().detect_capture_raw_status(sequence, _first_trigger);
            //_camHw->setIsppConfig(sequence);
            for (int i = 0; i < _mipi_dev_max; i++) {
                ret = _dev[i]->get_buffer(v4l2buf[i],
                        cache_list[i].front()->get_v4l2_buf_index());
                if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Rx[%d] can not get buffer\n", i);
                    goto out;
                } else {
                    buf_proxy = cache_list[i].pop(-1);
#if 0
                    if (_first_trigger) {
                        u8 *buf = (u8 *)buf_proxy->get_v4l2_userptr();
                        struct v4l2_format format = v4l2buf[i]->get_format();

                        if (buf) {
                            for (u32 j = 0; j < format.fmt.pix.width / 2; j++)
                                *buf++ += j % 16;
                        }
                    }
#endif
                    buf_list[i].push(buf_proxy);
                    if (_dev[i]->get_mem_type() == V4L2_MEMORY_USERPTR)
                        v4l2buf[i]->set_expbuf_usrptr(buf_proxy->get_v4l2_userptr());
                    else if (_dev[i]->get_mem_type() == V4L2_MEMORY_DMABUF){
                        v4l2buf[i]->set_expbuf_fd(buf_proxy->get_expbuf_fd());
                    }else if (_dev[i]->get_mem_type() == V4L2_MEMORY_MMAP) {
                        if (_dev[i]->get_use_type() == 1)
                        {
                            memcpy((void*)v4l2buf[i]->get_expbuf_usrptr(),(void*)buf_proxy->get_v4l2_userptr(),v4l2buf[i]->get_buf().m.planes[0].length);
                            v4l2buf[i]->set_reserved(buf_proxy->get_v4l2_userptr());
                        }
                    }

                    if (_rawCap) {
#ifdef UseCaptureRawData
                       _rawCap->dynamic_capture_raw(i, sequence, buf_proxy, v4l2buf[i],_mipi_dev_max,_working_mode,_dev[0]);
#else
                       _rawCap->dumpRkRawBlock(i, sequence, buf_proxy->get_v4l2_userptr(), v4l2buf[i]->get_buf().m.planes[0].bytesused);
#endif
                    }
                   //CaptureRawData::getInstance().dynamic_capture_raw(i, sequence, buf_proxy, v4l2buf[i],_mipi_dev_max,_working_mode,_dev[0]);
                }
            }

#ifdef UseCaptureRawData
            if (_rawCap) {
                if (_rawCap->is_need_save_metadata_and_register()) {
                    rkisp_effect_params_v20 ispParams;
                    _camHw->getEffectiveIspParams(ispParams, sequence);

                    SmartPtr<BaseSensorHw> mSensorSubdev = _camHw->mSensorDev.dynamic_cast_ptr<BaseSensorHw>();
                    SmartPtr<RkAiqSensorExpParamsProxy> ExpParams = nullptr;
                    mSensorSubdev->getEffectiveExpParams(ExpParams, sequence);

                    SmartPtr<LensHw> mLensSubdev = _camHw->mLensDev.dynamic_cast_ptr<LensHw>();
                    SmartPtr<RkAiqAfInfoProxy> afParams = nullptr;
                    if (mLensSubdev.ptr())
                        mLensSubdev->getAfInfoParams(afParams, sequence);
                    _rawCap->save_metadata_and_register(sequence, ispParams, ExpParams, afParams, _working_mode);
                }
            }
#endif

            for (int i = 0; i < _mipi_dev_max; i++) {
                ret = _dev[i]->queue_buffer(v4l2buf[i]);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    buf_list[i].pop(-1);
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Rx[%d] queue buffer failed\n", i);
                    break;
                }
            }

            struct isp2x_csi_trigger tg = {
                .sof_timestamp = 0,
                .frame_timestamp = 0,
                .frame_id = sequence,
                .times = 0,
                .mode = _mipi_dev_max == 1 ? T_START_X1 :
                _mipi_dev_max == 2 ? T_START_X2 : T_START_X3,
                /* .mode = T_START_X2, */
            };

            if (_first_trigger)
                tg.times = 1;
            else
                tg.times += additional_times;

            if (tg.times > 2)
                tg.times = 2;
            if (_is_multi_cam_conc && (tg.times < 1))
                tg.times = 1;

            uint64_t sof_timestamp = 0;
            match_sof_timestamp_map(tg.frame_id, sof_timestamp);
            tg.sof_timestamp = sof_timestamp;
            tg.frame_timestamp = buf_proxy->get_timestamp () * 1000;
            // tg.times = 1;//fixed to three times readback
            LOGD_CAMHW_SUBM(ISP20HW_SUBM,
                            "camId: %d, frameId: %d: sof_ts %" PRId64 "ms, frame_ts %" PRId64 "ms, trigger readback times: %d\n",
                            mCamPhyId, sequence,
                            tg.sof_timestamp / 1000 / 1000,
                            tg.frame_timestamp / 1000 / 1000,
                            tg.times);

            if (ret == XCAM_RETURN_NO_ERROR) {
                _isp_core_dev->io_control(RKISP_CMD_TRIGGER_READ_BACK, &tg);
            } else {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s frame[%d] queue  failed, don't read back!\n",
                                __func__, sequence);
            }

            if (_rawCap){
                _buf_mutex.unlock();
#ifdef UseCaptureRawData
                _rawCap->update_capture_raw_status(_first_trigger);
#else
                _rawCap->waitDumpRawDone();
                _rawCap->waitThirdPartyDumpDone(_first_trigger);
#endif
                _buf_mutex.lock();
            }
            //CaptureRawData::getInstance().update_capture_raw_status(_first_trigger);
        }
    }

    _first_trigger = false;
out:
    return;
}

XCamReturn
RawStreamProcUnit::set_csi_mem_word_big_align(uint32_t width, uint32_t height,
        uint32_t sns_v4l_pix_fmt, int8_t sns_bpp)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGD_CAMHW_SUBM(ISP20HW_SUBM,"sensor fmt 0x%x, %dx%d, sns_bpp: %d",
                    sns_v4l_pix_fmt, width, height, sns_bpp);

    for (int i = 0; i < 3; i++) {
        if (!_dev[i].ptr()) {
            ret = XCAM_RETURN_BYPASS;
            break;
        }

        if (((width / 2 - RKMOUDLE_UNITE_EXTEND_PIXEL) *  sns_bpp / 8) & 0xf) {
            int mem_mode = CSI_MEM_WORD_BIG_ALIGN;
            int ret1 = _dev[i]->io_control (RKISP_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
            if (ret1) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set CSI_MEM_WORD_BIG_ALIGN failed !\n");
                ret = XCAM_RETURN_ERROR_IOCTL;
            } else {
                LOGD_CAMHW_SUBM(ISP20HW_SUBM,"set the memory mode of isp rx to big align");
            }
        }
    }

    return ret;
}

XCamReturn RawStreamProcUnit::setIspInfoToDump()
{
    if (!_dev[0].ptr() || !_rawCap || !_camHw)
        return XCAM_RETURN_ERROR_FAILED;

    struct ispInfo_s ispInfo;
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    if (_dev[0]->get_format (format) < 0)
        return XCAM_RETURN_ERROR_FAILED;

    ispInfo.sns_width = format.fmt.pix.width;
    ispInfo.sns_height = format.fmt.pix.height;
    ispInfo.pixelformat = format.fmt.pix.pixelformat;
    ispInfo.working_mode = _working_mode;
    ispInfo.mCamPhyId = mCamPhyId;
    ispInfo.stridePerLine = format.fmt.pix_mp.plane_fmt[0].bytesperline;

    if (strlen(_camHw->sns_name) > 0)
        sscanf(&_camHw->sns_name[6], "%s", ispInfo.sns_name);
    // memcpy(ispInfo.sns_name, _camHw->sns_name, strlen(_camHw->sns_name));
#ifndef UseCaptureRawData
    _rawCap->setIspInfo(ispInfo);
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RawStreamProcUnit::DumpRkRawInFrameEnd(SmartPtr<V4l2BufferProxy> &rx_buf)
{
    uint32_t sequence = 0;

    if(rx_buf.ptr()) {
        sequence = rx_buf->get_sequence();
    }

	SmartPtr<BaseSensorHw> mSensorSubdev = _camHw->mSensorDev.dynamic_cast_ptr<BaseSensorHw>();
	SmartPtr<RkAiqSensorExpParamsProxy> expParams = nullptr;
	mSensorSubdev->getEffectiveExpParams(expParams, sequence);
	rkisp_effect_params_v20 ispParams;
	_camHw->getEffectiveIspParams(ispParams, sequence);

#ifndef UseCaptureRawData
	if (_rawCap->isDumpInFrameEnd(sequence, ispParams, expParams) == XCAM_RETURN_NO_ERROR) {
		SmartPtr<LensHw> mLensSubdev = _camHw->mLensDev.dynamic_cast_ptr<LensHw>();
		SmartPtr<RkAiqAfInfoProxy> afParams = nullptr;
		if (mLensSubdev.ptr())
			mLensSubdev->getAfInfoParams(afParams, sequence);

		_rawCap->dumpMetadataBlock(sequence, ispParams, expParams, afParams);
#ifdef ISP_REGS_BASE
		_rawCap->dumpIspRegBlock(ISP_REGS_BASE, 0, ISP_REGS_SIZE, sequence);
#endif
		_rawCap->dumpPlatformInfoBlock();
		_rawCap->updateDumpStatus();
		_rawCap->notifyDumpRaw();
	}
#endif

    return XCAM_RETURN_NO_ERROR;
}

int8_t
RawStreamProcUnit::getDumpRkRawType()
{
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.rkraw.type", property_value, "1");
    dumpRkRawType = strtoull(property_value, nullptr, 16);
#else
    char* valueStr = getenv("rkisp_dump_rkraw_type");
    if (valueStr)
         dumpRkRawType = strtoull(valueStr, nullptr, 16);
#endif

    LOGE_CAMHW_SUBM(ISP20HW_SUBM,"dumpRkRawType: %d", dumpRkRawType);

    return dumpRkRawType;
}

}; //namspace RkCam


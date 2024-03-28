#include "CamHwIsp20.h"
#include "rk_aiq_comm.h"
#include "fake_v4l2_device.h"
#include "rkcif-config.h"

namespace RkCam {
RawStreamCapUnit::RawStreamCapUnit ()
    :_is_1608_stream(false)
    ,_skip_num(0)
    ,_state(RAW_CAP_STATE_INVALID)
{
}

RawStreamCapUnit::RawStreamCapUnit (const rk_sensor_full_info_t *s_info, bool linked_to_isp, int tx_buf_cnt)
    :_is_1608_stream(false)
    ,_skip_num(0)
    ,_state(RAW_CAP_STATE_INVALID)
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
    // FIXME: flag(start_en) is ensure 1608 sensor already open.
    bool start_en = true;
    if (s_info->linked_to_1608) {
        // Traverse the corresponding [1~4] nodes
        if (CamHwIsp20::rk1608_share_inf.us_open_cnt > 0) {
            // 1st open
            start_en = false;
        }

        // state update.
        CamHwIsp20::rk1608_share_inf.us_open_cnt++;
        if (CamHwIsp20::rk1608_share_inf.us_open_cnt > 10)
            CamHwIsp20::rk1608_share_inf.us_open_cnt = 10;
    }

    //short frame
    if (strlen(s_info->isp_info->rawrd2_s_path)) {
        if (linked_to_isp)
            _dev[0] = new V4l2Device (s_info->isp_info->rawwr2_path);//rkisp_rawwr2
        else {
            if (s_info->dvp_itf) {
                if (strlen(s_info->cif_info->stream_cif_path))
                    _dev[0] = new V4l2Device (s_info->cif_info->stream_cif_path);
                else
                    _dev[0] = new V4l2Device (s_info->cif_info->dvp_id0);
            } else {
                if (!s_info->linked_to_1608) {
                    // normal mode
                    _dev[0] = new V4l2Device (s_info->cif_info->mipi_id0);
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        _dev[0] = new V4l2Device (s_info->cif_info->mipi_id0);
                    }
                }
            }
        }
        if (_dev[0].ptr())
            _dev[0]->open();

    }
    //mid frame
    if (strlen(s_info->isp_info->rawrd0_m_path)) {
        if (linked_to_isp)
            _dev[1] = new V4l2Device (s_info->isp_info->rawwr0_path);//rkisp_rawwr0
        else {
            if (!s_info->dvp_itf) {
                if (!s_info->linked_to_1608) {
                    // normal mode.
                    _dev[1] = new V4l2Device (s_info->cif_info->mipi_id1);
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        _dev[1] = new V4l2Device (s_info->cif_info->mipi_id1);
                    }
                }
            }
        }

        if (_dev[1].ptr())
            _dev[1]->open();
    }
    //long frame
    if (strlen(s_info->isp_info->rawrd1_l_path)) {
        if (linked_to_isp)
            _dev[2] = new V4l2Device (s_info->isp_info->rawwr1_path);//rkisp_rawwr1
        else {
            if (!s_info->dvp_itf) {
                if (!s_info->linked_to_1608) {
                    // normal mode.
                    _dev[2] = new V4l2Device (s_info->cif_info->mipi_id2);//rkisp_rawwr1
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        _dev[2] = new V4l2Device (s_info->cif_info->mipi_id2);//rkisp_rawwr1
                    }
                }
            }
        }
        if (_dev[2].ptr())
            _dev[2]->open();
    }
    int buf_cnt = tx_buf_cnt;
    if (tx_buf_cnt == 0) {
        if (linked_to_isp) {
            buf_cnt = ISP_TX_BUF_NUM;
        } else if (s_info->linked_to_1608) {
            buf_cnt = VIPCAP_TX_BUF_NUM_1608;
        } else {
            buf_cnt = VIPCAP_TX_BUF_NUM;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (_dev[i].ptr()) {
            _dev[i]->set_buffer_count(buf_cnt);
            _dev[i]->set_buf_sync (true); 
            _dev_bakup[i] = _dev[i];
            _dev_index[i] = i;
            _stream[i] =  new RKRawStream(_dev[i], i, ISP_POLL_TX);
            _stream[i]->setPollCallback(this);
        } else {
            _dev_bakup[i] = NULL;
            _dev_index[i] = i;
        }
    }

    _state = RAW_CAP_STATE_INITED;
}

RawStreamCapUnit::~RawStreamCapUnit ()
{
    _state = RAW_CAP_STATE_INVALID;
}

XCamReturn RawStreamCapUnit::start(int mode)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->setCamPhyId(mCamPhyId);
        _stream[i]->start();
    }
    _state = RAW_CAP_STATE_STARTED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RawStreamCapUnit::stop ()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopThreadOnly();
    }
    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        buf_list[i].clear ();
    }
    _buf_mutex.unlock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopDeviceOnly();
    }
    _state = RAW_CAP_STATE_STOPPED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RawStreamCapUnit::prepare(int idx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    // mipi rx/tx format should match to sensor.
    for (int i = 0; i < 3; i++) {
        if (!(idx & (1 << i)))
            continue;
        ret = _dev[i]->prepare();
        if (ret < 0)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "mipi tx:%d prepare err: %d\n", ret);

        _stream[i]->set_device_prepared(true);
    }
    _state = RAW_CAP_STATE_PREPARED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return ret;
}

void
RawStreamCapUnit::prepare_cif_mipi()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter,working_mode=0x%x", __FUNCTION__, _working_mode);

    FakeV4l2Device* fake_dev = dynamic_cast<FakeV4l2Device* >(_dev[0].ptr());

    if (fake_dev) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM,"ignore fake tx");
        return;
    }

    SmartPtr<V4l2Device> tx_devs_tmp[3] =
    {
        _dev_bakup[0],
        _dev_bakup[1],
        _dev_bakup[2],
    };

    // _mipi_tx_devs
    if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        // use _mipi_tx_devs[0] only
        // id0 as normal
        // do nothing
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> normal",
                        _dev[0]->get_device_name());
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        // use _mipi_tx_devs[0] and _mipi_tx_devs[1]
        // id0 as l, id1 as s
        SmartPtr<V4l2Device> tmp = tx_devs_tmp[1];
        tx_devs_tmp[1] = tx_devs_tmp[0];
        tx_devs_tmp[0] = tmp;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> long",
                        _dev[1]->get_device_name());
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> short",
                        _dev[0]->get_device_name());
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
        // use _mipi_tx_devs[0] and _mipi_tx_devs[1]
        // id0 as l, id1 as m, id2 as s
        SmartPtr<V4l2Device> tmp = tx_devs_tmp[2];
        tx_devs_tmp[2] = tx_devs_tmp[0];
        tx_devs_tmp[0] = tmp;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> long",
                        _dev[2]->get_device_name());
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> middle",
                        _dev[1]->get_device_name());
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> short",
                        _dev[0]->get_device_name());
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "wrong hdr mode: %d\n", _working_mode);
    }
    for (int i = 0; i < 3; i++) {
        if (tx_devs_tmp[i].ptr()) {
            _dev[i] = tx_devs_tmp[i];
            _dev_index[i] = i;
            _stream[i].release();
            _stream[i] =  new RKRawStream(_dev[i], i, ISP_POLL_TX);
            _stream[i]->setPollCallback(this);
        }
    }
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
}

void
RawStreamCapUnit::set_working_mode(int mode)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter,mode=0x%x", __FUNCTION__, mode);
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
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
}

void
RawStreamCapUnit::set_tx_devices(SmartPtr<V4l2Device> mipi_tx_devs[3])
{
    for (int i = 0; i < 3; i++) {
        _dev[i] = mipi_tx_devs[i];
        _stream[i].release();
        _stream[i] =  new RKRawStream(_dev[i], i, ISP_POLL_TX);
        _stream[i]->setPollCallback(this);
    }
}

SmartPtr<V4l2Device>
RawStreamCapUnit::get_tx_device(int index)
{
    if (index > _mipi_dev_max)
        return nullptr;
    else
        return _dev[index];
}

XCamReturn
RawStreamCapUnit::set_tx_format(const struct v4l2_subdev_format& sns_sd_fmt,
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
RawStreamCapUnit::set_tx_format(const struct v4l2_subdev_selection& sns_sd_sel,
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
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            _dev[i]->get_device_name(), sns_v4l_pix_fmt,
                            sns_sd_sel.r.width, sns_sd_sel.r.height);
        }
    }

    return ret;
}

void
RawStreamCapUnit::set_devices(SmartPtr<V4l2SubDevice> ispdev, CamHwIsp20* handle, RawStreamProcUnit *proc)
{
    _isp_core_dev = ispdev;
    _camHw = handle;
    _proc_stream = proc;
}

XCamReturn
RawStreamCapUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<V4l2BufferProxy> buf_s, buf_m, buf_l;

    _buf_mutex.lock();
    buf_list[dev_index].push(buf);
    ret = sync_raw_buf(buf_s, buf_m, buf_l);
    _buf_mutex.unlock();

    if (ret == XCAM_RETURN_NO_ERROR) {
        // FIXME: rawCap only has one valid pointer(just malloc once buf), so it needs to do multiple syncs
        if (!_is_1608_stream) {
            // normal
            if (_proc_stream) {
                _proc_stream->send_sync_buf(buf_s, buf_m, buf_l);
            }
        } else {
            // 1608 mode.
            for (int idx = 0; idx < CAM_INDEX_FOR_1608; idx++) {
                if (CamHwIsp20::rk1608_share_inf.raw_proc_unit[idx]) {
                    CamHwIsp20::rk1608_share_inf.raw_proc_unit[idx]->send_sync_buf(buf_s, buf_m, buf_l);
                }
            }
        }

        if (_camHw->mHwResLintener) {
            auto vbufInfo = buf_s->get_video_info();
            SmartPtr<SubVideoBuffer> subvbuf = new SubVideoBuffer (buf_s);
            subvbuf->_buf_type = ISP_POLL_TX;
            subvbuf->set_sequence(buf_s->get_sequence());
            subvbuf->set_video_info(vbufInfo);
            SmartPtr<VideoBuffer> vbuf = subvbuf.dynamic_cast_ptr<VideoBuffer>();
            _camHw->mHwResLintener->hwResCb(vbuf);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

void RawStreamCapUnit::skip_frames(int skip_num, int32_t skip_seq)
{
    _mipi_mutex.lock();
    _skip_num = skip_num;
    _skip_to_seq = skip_seq + _skip_num;
    _mipi_mutex.unlock();
}

bool RawStreamCapUnit::check_skip_frame(int32_t buf_seq)
{
    _mipi_mutex.lock();
#if 0 // ts
    if (_skip_num > 0) {
        int64_t skip_ts_ms = _skip_start_ts / 1000 / 1000;
        int64_t buf_ts_ms = buf_ts / 1000;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "skip num  %d, start from %" PRId64 " ms,  buf ts %" PRId64 " ms",
                        _skip_num,
                        skip_ts_ms,
                        buf_ts_ms);
        if (buf_ts_ms  > skip_ts_ms) {
            _skip_num--;
            _mipi_mutex.unlock();
            return true;
        }
    }
#else

    if ((_skip_num > 0) && (buf_seq < _skip_to_seq)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "skip num  %d, skip seq %d, dest seq %d",
                        _skip_num, buf_seq, _skip_to_seq);
        _skip_num--;
        _mipi_mutex.unlock();
        return true;
    }
#endif
    _mipi_mutex.unlock();
    return false;
}

XCamReturn
RawStreamCapUnit::sync_raw_buf
(
    SmartPtr<V4l2BufferProxy> &buf_s,
    SmartPtr<V4l2BufferProxy> &buf_m,
    SmartPtr<V4l2BufferProxy> &buf_l
)
{
    uint32_t sequence_s = -1, sequence_m = -1, sequence_l = -1;

    for (int i = 0; i < _mipi_dev_max; i++) {
        if (buf_list[i].is_empty()) {
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    buf_l = buf_list[ISP_MIPI_HDR_L].front();
    if (buf_l.ptr())
        sequence_l = buf_l->get_sequence();

    buf_m = buf_list[ISP_MIPI_HDR_M].front();
    if (buf_m.ptr())
        sequence_m = buf_m->get_sequence();

    buf_s = buf_list[ISP_MIPI_HDR_S].front();

    if (buf_s.ptr()) {
        sequence_s = buf_s->get_sequence();
        if ((_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
                _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) &&
                buf_m.ptr() && buf_l.ptr() && buf_s.ptr() &&
                sequence_l == sequence_s && sequence_m == sequence_s) {

            buf_list[ISP_MIPI_HDR_S].erase(buf_s);
            buf_list[ISP_MIPI_HDR_M].erase(buf_m);
            buf_list[ISP_MIPI_HDR_L].erase(buf_l);
            if (check_skip_frame(sequence_s)) {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else if ((_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                    _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) &&
                   buf_m.ptr() && buf_s.ptr() && sequence_m == sequence_s) {
            buf_list[ISP_MIPI_HDR_S].erase(buf_s);
            buf_list[ISP_MIPI_HDR_M].erase(buf_m);
            if (check_skip_frame(sequence_s)) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            buf_list[ISP_MIPI_HDR_S].erase(buf_s);
            if (check_skip_frame(sequence_s)) {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "do nothing, sequence not match l: %d, s: %d, m: %d !!!",
                            sequence_l, sequence_s, sequence_m);
        }
        return XCAM_RETURN_NO_ERROR;
    }
end:
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
RawStreamCapUnit::reset_hardware()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (_dev[0].ptr()) {
        int32_t reset = 1;
        int32_t b_ret = 0;

        errno = 0;
        b_ret = _dev[0]->io_control(RKCIF_CMD_SET_RESET, &reset);
        if (b_ret < 0) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "device(%s) reset failed: %d (%s)!",
                    XCAM_STR(_dev[0]->get_device_name()), b_ret, strerror(errno));
            return XCAM_RETURN_ERROR_IOCTL;
        }

        LOGD_CAMHW_SUBM(SENSOR_SUBM, "device(%s) reset", XCAM_STR(_dev[0]->get_device_name()));
    }

    return ret;
}

XCamReturn
RawStreamCapUnit::set_csi_mem_word_big_align(uint32_t width, uint32_t height,
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
            int mem_mode = CSI_LVDS_MEM_WORD_HIGH_ALIGN;
            int ret1 = _dev[i]->io_control (RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
            if (ret1) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set CSI_MEM_WORD_BIG_ALIGN failed!\n");
                ret = XCAM_RETURN_ERROR_IOCTL;
            } else {
                LOGD_CAMHW_SUBM(ISP20HW_SUBM,"set the memory mode of vicap to big align");
            }
        }
    }

    return ret;
}

XCamReturn
RawStreamCapUnit::setVicapStreamMode(int mode, uint32_t *frameId, bool isSingleMode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkcif_quick_stream_param info;

    if (!_dev[0].ptr()) {
        LOGE_CAMHW("dev[0] is null pointer");
        return XCAM_RETURN_ERROR_FAILED;
    }

    info.on = mode;
    if (isSingleMode)
        info.resume_mode = RKISP_RTT_MODE_ONE_FRAME;
    else
        info.resume_mode = RKISP_RTT_MODE_MULTI_FRAME;

    if (_dev[0]->io_control(RKCIF_CMD_SET_QUICK_STREAM, &info) < 0) {
        LOGE_CAMHW("dev(%s) ioctl faile, set vicap %s faile", _dev[0]->get_device_name(), mode ? "pause" : "resume");
        ret = XCAM_RETURN_ERROR_IOCTL;
    }
    if (frameId)
        *frameId = info.frame_num;
    return ret;
}

}

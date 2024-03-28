
#include "RawStreamProcUnit.h"
#include "MediaInfo.h"

namespace RkRawStream {

int _parse_rk_rawdata(void *rawdata, rkrawstream_rkraw2_t *rkraw2)
{
    unsigned short tag = 0;
    struct _block_header header;
    uint8_t *p = (uint8_t *)rawdata;
    bool bExit = false;

    int bufsize[3] = {0};
    uint8_t *userptr[3] = {NULL};
    struct _st_addrinfo_stream _st_addr[3];
    uint64_t uptr;

    if(rawdata == NULL || rkraw2 == NULL){
        return -1;
    }
    while(!bExit){
        tag = *((unsigned short*)p);
        LOGD_RKSTREAM("_parse_rk_rawdata tag=0x%04x\n",tag);
        switch (tag)
        {
            case START_TAG:
                p = p+TAG_BYTE_LEN;
                memset(_st_addr, 0, sizeof(_st_addr));
                memset(&rkraw2->_rawfmt, 0, sizeof(struct _raw_format));
                memset(&rkraw2->_finfo, 0, sizeof(rk_aiq_frame_info_t));
                break;
            case NORMAL_RAW_TAG:
            {
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo_stream)) {
                    _st_addr[0] = *((struct _st_addrinfo_stream*)p);
                }else{
                    userptr[0] = p;
                    bufsize[0] = header.block_length;
                }
                p = p + header.block_length;

                break;
            }
            case HDR_S_RAW_TAG:
            {
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo_stream)) {
                    _st_addr[0] = *((struct _st_addrinfo_stream*)p);
                }else{
                    userptr[0] = p;
                    bufsize[0] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case HDR_M_RAW_TAG:
            {
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo_stream)) {
                    _st_addr[1] = *((struct _st_addrinfo_stream*)p);
                }else{
                    userptr[1] = p;
                    bufsize[1] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case HDR_L_RAW_TAG:
            {
                header = *((struct _block_header *)p);
                p = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo_stream)) {
                    _st_addr[2] = *((struct _st_addrinfo_stream*)p);
                }else{
                    userptr[2] = p;
                    bufsize[2] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case FORMAT_TAG:
            {
                rkraw2->_rawfmt = *((struct _raw_format *)p);
                p = p + sizeof(struct _block_header) + rkraw2->_rawfmt.size;
                break;
            }
            case STATS_TAG:
            {
                rkraw2->_finfo = *((rk_aiq_frame_info_t *)p);
                p = p + sizeof(struct _block_header) + rkraw2->_finfo.size;
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
                LOGW_RKSTREAM("Not support TAG(0x%04x)\n", tag);
                bExit = true;
                break;
            }
        }
    }

    if(_st_addr[0].fd || _st_addr[0].laddr){
        uptr = _st_addr[0].haddr;
        uptr = uptr << 32;
        uptr = uptr | _st_addr[0].laddr;

        rkraw2->plane[0].mode = 0;
        rkraw2->plane[0].addr = uptr;
        rkraw2->plane[0].fd = _st_addr[0].fd;
        rkraw2->plane[0].idx = _st_addr[0].idx;
        rkraw2->plane[0].size = _st_addr[0].size;
        rkraw2->plane[0].timestamp = _st_addr[0].timestamp;
    }
    if(userptr[0]){
        //sbuf_s->_userptr = _rawbuffer[0];
        //memcpy(_rawbuffer[0], userptr[0], bufsize[0]);
        rkraw2->plane[0].mode = 1;
        rkraw2->plane[0].addr = (uint64_t)userptr[0];
        rkraw2->plane[0].size = bufsize[0];
    }

    if(_st_addr[1].fd || _st_addr[1].laddr){
        uptr = _st_addr[1].haddr;
        uptr = uptr << 32;
        uptr = uptr | _st_addr[1].laddr;

        rkraw2->plane[1].mode = 0;
        rkraw2->plane[1].addr = uptr;
        rkraw2->plane[1].fd = _st_addr[1].fd;
        rkraw2->plane[1].idx = _st_addr[1].idx;
        rkraw2->plane[1].size = _st_addr[1].size;
        rkraw2->plane[1].timestamp = _st_addr[1].timestamp;
    }
    if(userptr[1]){
        //sbuf_m->_userptr = _rawbuffer[1];
        //memcpy(_rawbuffer[1], userptr[1], bufsize[1]);

        rkraw2->plane[1].mode = 1;
        rkraw2->plane[1].addr = (uint64_t)userptr[1];
        rkraw2->plane[1].size = bufsize[1];
    }

    if(_st_addr[2].fd || _st_addr[2].laddr){
        uptr = _st_addr[2].haddr;
        uptr = uptr << 32;
        uptr = uptr | _st_addr[2].laddr;

        rkraw2->plane[2].mode = 0;
        rkraw2->plane[2].addr = uptr;
        rkraw2->plane[2].fd = _st_addr[2].fd;
        rkraw2->plane[2].idx = _st_addr[2].idx;
        rkraw2->plane[2].size = _st_addr[2].size;
        rkraw2->plane[2].timestamp = _st_addr[2].timestamp;
    }
    if(userptr[2]){
        //sbuf_l->_userptr = _rawbuffer[2];
        //memcpy(_rawbuffer[2], userptr[2], bufsize[2]);
        rkraw2->plane[2].mode = 1;
        rkraw2->plane[2].addr = (uint64_t)userptr[2];
        rkraw2->plane[2].size = bufsize[2];
    }

    return 0;
}

RawStreamProcUnit::RawStreamProcUnit (const rk_sensor_full_info_t *s_info, uint8_t is_offline)
    : _first_trigger(true)
    , _mipi_dev_max(0)
    , _is_multi_cam_conc(false)
    , user_isp_process_done_cb(NULL)
    , user_priv_data(NULL)
    , _memory_type(V4L2_MEMORY_DMABUF)
    , _buffer_count(RKRAWSTREAM_DEF_BUFCNT)
{
    _raw_proc_thread = new RawProcThread(this);
    _PollCallback = NULL;

    bool linked_to_isp = s_info->linked_to_isp;

    _is_offline_mode = is_offline;

    strncpy(_sns_name, s_info->sensor_name.c_str(), 32);
    //short frame
    if (strlen(s_info->isp_info->rawrd2_s_path)) {
        _dev[0] = new V4l2Device (s_info->isp_info->rawrd2_s_path);//rkisp_rawrd2_s
        _dev[0]->open();
        _dev[0]->set_mem_type(_memory_type);
    }
    //mid frame
    if (strlen(s_info->isp_info->rawrd0_m_path)) {
        _dev[1] = new V4l2Device (s_info->isp_info->rawrd0_m_path);//rkisp_rawrd0_m
        _dev[1]->open();
        _dev[1]->set_mem_type(_memory_type);
    }
    //long frame
    if (strlen(s_info->isp_info->rawrd1_l_path)) {
        _dev[2] = new V4l2Device (s_info->isp_info->rawrd1_l_path);//rkisp_rawrd1_l
        _dev[2]->open();
        _dev[2]->set_mem_type(_memory_type);
    }
    for (int i = 0; i < 3; i++) {
        if (_dev[i].ptr())
            _dev[i]->set_buffer_count(_buffer_count);
        if (_dev[i].ptr())
            _dev[i]->set_buf_sync (false);
        _dev_index[i] = i;
        _stream[i] =  new RKRawStream(_dev[i], i, RKRAWSTREAM_POLL_READBACK);
        _stream[i]->setPollCallback(this);

    }

    _isp_core_dev = new V4l2SubDevice(s_info->isp_info->isp_dev_path);
    _isp_core_dev->open();

    _offline_index = 0;
    _offline_seq = 0;

    is_multi_isp_mode = s_info->isp_info->is_multi_isp_mode;

    _rawbuffer[0] = NULL;
    _rawbuffer[1] = NULL;
    _rawbuffer[2] = NULL;
}

RawStreamProcUnit::~RawStreamProcUnit ()
{
    LOGD_RKSTREAM("enter ~RawStreamProcUnit\n");
    if (_dev[0].ptr())
        _dev[0] -> close();
    if (_dev[1].ptr())
        _dev[1] -> close();
    if (_dev[2].ptr())
        _dev[2] -> close();
    LOGD_RKSTREAM("exit ~RawStreamProcUnit\n");
}

XCamReturn RawStreamProcUnit::start()
{
    XCamReturn ret;
    for (int i = 0; i < _mipi_dev_max; i++) {
        ret = _stream[i]->start();
        if (ret != XCAM_RETURN_NO_ERROR){
            LOGE_RKSTREAM("RawStreamProcUnit rxdev[%d] start error!\n", i);
        }
    }
    _msg_queue.resume_pop();
    _msg_queue.clear();
    _raw_proc_thread->start();

    _offline_index = 0;
    _offline_seq = 0;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RawStreamProcUnit::stop ()
{
    _msg_queue.pause_pop();
    _raw_proc_thread->stop();

    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopThreadOnly();
    }

    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        cache_list2[i].clear ();
    }
    _isp_hdr_fid2ready_map.clear();
    _buf_mutex.unlock();

    //_mipi_trigger_mutex.lock();
    //_isp_hdr_fid2times_map.clear();
    _sof_timestamp_map.clear();
    //_mipi_trigger_mutex.unlock();

    //if (_rawCap) {
    //    delete _rawCap;
    //    _rawCap = NULL;
   // }
    /*
    if(_is_offline_mode) {
        for (int i = 0; i < _mipi_dev_max; i++) {
            if(_rawbuffer[i]){
                free(_rawbuffer[i]);
                _rawbuffer[i] = NULL;
            }
        }
    }
    */
    for (int i = 0; i < _mipi_dev_max; i++) {
        _stream[i]->stopDeviceOnly();
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RawStreamProcUnit::prepare(uint8_t buf_memory_type, uint8_t buf_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_RKSTREAM("RawStreamProcUnit %s bufype: %d bufcnt %d\n",__func__, buf_memory_type, buf_cnt);
    // mipi rx/tx format should match to sensor.
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
            LOGE_RKSTREAM("mipi tx:%d prepare err: %d\n", i, ret);

        _stream[i]->set_device_prepared(true);
        /*
         if(_is_offline_mode) {
            printf("malloc _rawbuff, %dx%d\n", _width, _height);
            _rawbuffer[i] = (uint8_t *)malloc(_width * _height * 2);
         }
        */
    }
    return ret;
}

void
RawStreamProcUnit::set_working_mode(int mode)
{
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
    LOGD_RKSTREAM("working_mode:0x%x, _mipi_dev_max=%d\n", _working_mode, _mipi_dev_max);
}

void
RawStreamProcUnit::set_rx_format(uint32_t width, uint32_t height, uint32_t pix_fmt, int mode)
{
    struct v4l2_format format;
    uint32_t w,h,pf;
    memset(&format, 0, sizeof(format));

    LOGD_RKSTREAM("RawStreamProcUnit %s:%dx%d, 0x%x mode %d\n", __func__, width, height, pix_fmt, mode);

    for (int i = 0; i < _mipi_dev_max; i++) {
        _dev[i]->get_format (format);
        w = width? width: format.fmt.pix.width;
        h = height? height: format.fmt.pix.height;
        pf = pix_fmt? pix_fmt: format.fmt.pix.pixelformat;

        int bpp = pixFmt2Bpp(format.fmt.pix.pixelformat);
        int mem_mode = mode;
        if (is_multi_isp_mode  && mem_mode == CSI_MEM_COMPACT &&
            (((w / 2 - RKMOUDLE_UNITE_EXTEND_PIXEL) * bpp / 8) & 0xf)) {
            mem_mode = CSI_MEM_WORD_BIG_ALIGN;
            LOGE_RKSTREAM("ISP is running on rkisp-unite mode, width %d does not meet the 256 alignment,"
                          "force set raw mem_mode to CSI_MEM_WORD_BIG_ALIGN", w);
        }
        int ret1 = _dev[i]->io_control (RKISP_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
        if (ret1)
            LOGE_RKSTREAM("set CSI_MEM_WORD_LITTLE_ALIGN failed !\n");


        LOGI_RKSTREAM("RawStreamProcUnit %s:use format %dx%d, 0x%x\n", __func__, w, h, pf);
        _dev[i]->set_format(w, h, pf, V4L2_FIELD_NONE, 0);
    }
}

void
RawStreamProcUnit::setup_pipeline_fmt(uint32_t width, uint32_t height, uint32_t pixfmt)
{
    int ret;
    // set isp sink fmt, same as sensor bounds - crop
    struct v4l2_subdev_format isp_sink_fmt;

    memset(&isp_sink_fmt, 0, sizeof(isp_sink_fmt));
    isp_sink_fmt.pad = 0;
    isp_sink_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret = _isp_core_dev->getFormat(isp_sink_fmt);
    if (ret) {
        LOGE_RKSTREAM("set mIspCoreDev fmt failed !\n");
        return;
    }
    isp_sink_fmt.format.width = width;
    isp_sink_fmt.format.height = height;
    isp_sink_fmt.format.code = pixfmt;

    ret = _isp_core_dev->setFormat(isp_sink_fmt);
    if (ret) {
        LOGE_RKSTREAM("set mIspCoreDev fmt failed !\n");
        return;
    }

    LOGD_RKSTREAM("isp sink fmt info: fmt 0x%x, %dx%d !",
                    isp_sink_fmt.format.code, isp_sink_fmt.format.width, isp_sink_fmt.format.height);

    // set selection, isp needn't do the crop
    struct v4l2_subdev_selection aSelection;
    memset(&aSelection, 0, sizeof(aSelection));

    aSelection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    aSelection.pad = 0;
    aSelection.flags = 0;
    aSelection.target = V4L2_SEL_TGT_CROP;
    aSelection.r.width = width;
    aSelection.r.height = height;
    aSelection.r.left = 0;
    aSelection.r.top = 0;
    ret = _isp_core_dev->set_selection (aSelection);
    if (ret) {
        LOGE_RKSTREAM("set mIspCoreDev crop failed !\n");
        return;
    }

    LOGD_RKSTREAM("isp sink crop info: %dx%d@%d,%d !",
                    aSelection.r.width, aSelection.r.height,
                    aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src crop
    aSelection.pad = 2;
    ret = _isp_core_dev->set_selection (aSelection);
    if (ret) {
        LOGE_RKSTREAM("set mIspCoreDev source crop failed !\n");
        return;
    }
    LOGD_RKSTREAM("isp src crop info: %dx%d@%d,%d !",
                    aSelection.r.width, aSelection.r.height,
                    aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src pad fmt
    struct v4l2_subdev_format isp_src_fmt;

    memset(&isp_src_fmt, 0, sizeof(isp_src_fmt));
    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad = 2;
    ret = _isp_core_dev->getFormat(isp_src_fmt);
    if (ret) {
        LOGE_RKSTREAM("get mIspCoreDev src fmt failed !\n");
        return;
    }

    isp_src_fmt.format.width = aSelection.r.width;
    isp_src_fmt.format.height = aSelection.r.height;
    ret = _isp_core_dev->setFormat(isp_src_fmt);
    if (ret) {
        LOGE_RKSTREAM("set mIspCoreDev src fmt failed !\n");
        return;
    }

    LOGD_RKSTREAM("isp src fmt info: fmt 0x%x, %dx%d !",
                    isp_src_fmt.format.code, isp_src_fmt.format.width, isp_src_fmt.format.height);
}

/*
SmartPtr<V4l2Device>
RawStreamProcUnit::get_rx_device(int index)
{
    if (index > _mipi_dev_max)
        return nullptr;
    else
        return _dev[index];
}


void
RawStreamProcUnit::set_rx_format(const struct v4l2_subdev_selection& sns_sd_sel, uint32_t sns_v4l_pix_fmt)
{
    // set mipi tx,rx fmt
    // for cif: same as sensor fmt

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    for (int i = 0; i < 3; i++) {
        if (_dev[i].ptr())
            _dev[i]->get_format (format);
        if (format.fmt.pix.width != sns_sd_sel.r.width ||
                format.fmt.pix.height != sns_sd_sel.r.height ||
                format.fmt.pix.pixelformat != sns_v4l_pix_fmt) {
            if (_dev[i].ptr())
                _dev[i]->set_format(sns_sd_sel.r.width,
                                    sns_sd_sel.r.height,
                                    sns_v4l_pix_fmt,
                                    V4L2_FIELD_NONE,
                                    0);
        }
    }

    LOGD_RKSTREAM("set rx fmt info: fmt 0x%x, %dx%d !",
                    sns_v4l_pix_fmt, sns_sd_sel.r.width, sns_sd_sel.r.height);
}

void
RawStreamProcUnit::set_devices(SmartPtr<V4l2SubDevice> ispdev, CamHwIsp20* handle)
{
    _isp_core_dev = ispdev;
    _camHw = handle;
}
*/
XCamReturn
RawStreamProcUnit::poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index)
{
    SmartLock locker (_buf_mutex);
    rkrawstream_readback_cb_param_t param;
    param.user_data = user_priv_data;
    param.dev_index = dev_index;
    // if (!buf_list[dev_index].is_empty()) {
        // SmartPtr<V4l2BufferProxy> rx_buf = buf_list[dev_index].pop(-1);
        // LOGD_RKSTREAM("%s dev_index:%d index:%d fd:%d\n",
                        // __func__, dev_index, rx_buf->get_v4l2_buf_index(), rx_buf->get_expbuf_fd());
    // }
    if (_PollCallback){
        _PollCallback->poll_buffer_ready (buf, dev_index);
    }

    if(user_isp_process_done_cb)
        user_isp_process_done_cb(&param);

    return XCAM_RETURN_NO_ERROR;
}
/*
void
RawStreamProcUnit::set_hdr_frame_readback_infos(int frame_id, int times)
{
    if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        return;

    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map[frame_id] = times;
    LOGD_RKSTREAM( "rdtimes seq %d \n", frame_id);
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
            LOGD_RKSTREAM( "del seq %d", it_times_del->first);
            iter = _isp_hdr_fid2times_map.erase(it_times_del);
        } else if (iter->first == sequence) {
            additional_times = iter->second;
            it_times_del = iter++;
            LOGD_RKSTREAM( "del seq %d", it_times_del->first);
            iter = _isp_hdr_fid2times_map.erase(it_times_del);
            break;
        } else {
            LOGW( "%s missing rdtimes for buf_seq %d, min rdtimes_seq %d !",
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
            LOGD_RKSTREAM( "del seq %d", it_del->first);
            iter = _hdr_global_tmo_state_map.erase(it_del);
        } else if (iter->first == sequence) {
            isHdrGlobalTmo = iter->second;
            it_del = iter++;
            LOGD_RKSTREAM( "del seq %d", it_del->first);
            iter = _hdr_global_tmo_state_map.erase(it_del);
            break;
        } else {
            LOGW( "%s missing tmo state for buf_seq %d, min rdtimes_seq %d !",
                            __func__, sequence, iter->first);
            break;
        }
    }
    _mipi_trigger_mutex.unlock();
}

void
RawStreamProcUnit::set_hdr_global_tmo_mode(int frame_id, bool mode)
{
    _mipi_trigger_mutex.lock();
    _hdr_global_tmo_state_map[frame_id] = mode;
    _mipi_trigger_mutex.unlock();
}

void
RawStreamProcUnit::notify_sof(uint64_t time, int frameid)
{
    _mipi_trigger_mutex.lock();
    while (_sof_timestamp_map.size() > 8) {
        _sof_timestamp_map.erase(_sof_timestamp_map.begin());
    }
    _sof_timestamp_map[frameid] = time;
    _mipi_trigger_mutex.unlock();
}
*/

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
        LOGW(  "can't find frameid(%d), get sof timestamp failed!\n",
                sequence);
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

bool
RawStreamProcUnit::raw_buffer_proc ()
{
    LOGD_RKSTREAM("%s enter", __FUNCTION__);
    if (_msg_queue.pop(-1).ptr()) {
        trigger_isp_readback();
    } else {
        return false;
    }
    LOGD_RKSTREAM("%s exit", __FUNCTION__);
    return true;
}

void
RawStreamProcUnit::send_sync_buf2(uint8_t *rkraw_data)
{
    rkrawstream_rkraw2_t rkraw2;
    _parse_rk_rawdata(rkraw_data,  &rkraw2);
    _send_sync_buf(&rkraw2);
}

void
RawStreamProcUnit::_send_sync_buf(rkrawstream_rkraw2_t *rkraw2)
{
    SmartPtr<SimpleFdBuf> sbuf_s, sbuf_m, sbuf_l;
    sbuf_s = new SimpleFdBuf();
    sbuf_m = new SimpleFdBuf();
    sbuf_l = new SimpleFdBuf();

    /*
     * Offline frames has no index and seq,
     * so we assign them here.
     */
    if(rkraw2->plane[0].mode == 0){
        sbuf_s->_userptr = (uint8_t *)rkraw2->plane[0].addr;
        sbuf_s->_fd = rkraw2->plane[0].fd;
        sbuf_s->_index = rkraw2->plane[0].idx;
        sbuf_s->_seq = rkraw2->_rawfmt.frame_id;
        sbuf_s->_ts = rkraw2->plane[0].timestamp;
    } else {
        //memcpy(_rawbuffer[0], (uint8_t *)rkraw2->plane[0].addr, rkraw2->plane[0].size);
        //sbuf_s->_userptr = _rawbuffer[0];
        sbuf_s->_userptr = (uint8_t *)rkraw2->plane[0].addr;
        sbuf_s->_size = (uint32_t)rkraw2->plane[0].size;
        sbuf_s->_index = _offline_index;
        sbuf_s->_seq = _offline_seq;
        if (rkraw2->_rawfmt.frame_id != 0) {
            sbuf_s->_seq = rkraw2->_rawfmt.frame_id;
        }
    }

    if (_mipi_dev_max > 0) {
        if(rkraw2->plane[1].mode == 0){
            sbuf_m->_userptr = (uint8_t *)rkraw2->plane[1].addr;
            sbuf_m->_fd = rkraw2->plane[1].fd;
            sbuf_m->_index = rkraw2->plane[1].idx;
            sbuf_m->_seq = rkraw2->_rawfmt.frame_id;
            sbuf_m->_ts = rkraw2->plane[1].timestamp;
        } else {
            //memcpy(_rawbuffer[0], (uint8_t *)rkraw2->plane[0].addr, rkraw2->plane[0].size);
            //sbuf_s->_userptr = _rawbuffer[0];
            sbuf_m->_userptr = (uint8_t *)rkraw2->plane[1].addr;
            sbuf_m->_size = (uint32_t)rkraw2->plane[1].size;
            sbuf_m->_index = _offline_index;
            sbuf_m->_seq = _offline_seq;
            if (rkraw2->_rawfmt.frame_id != 0) {
                sbuf_m->_seq = rkraw2->_rawfmt.frame_id;
            }
        }
    }

    if (_mipi_dev_max > 1) {
        if(rkraw2->plane[2].mode == 0){
            sbuf_l->_userptr = (uint8_t *)rkraw2->plane[2].addr;
            sbuf_l->_fd = rkraw2->plane[2].fd;
            sbuf_l->_index = rkraw2->plane[2].idx;
            sbuf_l->_seq = rkraw2->_rawfmt.frame_id;
            sbuf_l->_ts = rkraw2->plane[2].timestamp;
        } else {
            //memcpy(_rawbuffer[0], (uint8_t *)rkraw2->plane[0].addr, rkraw2->plane[0].size);
            //sbuf_s->_userptr = _rawbuffer[0];
            sbuf_l->_userptr = (uint8_t *)rkraw2->plane[2].addr;
            sbuf_l->_size = (uint32_t)rkraw2->plane[2].size;
            sbuf_l->_index = _offline_index;
            sbuf_l->_seq = _offline_seq;
            if (rkraw2->_rawfmt.frame_id != 0) {
                sbuf_l->_seq = rkraw2->_rawfmt.frame_id;
            }
        }
    }

    _offline_index ++;
    _offline_seq ++;

    if(_offline_index == _buffer_count)
        _offline_index = 0;

    _buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (i == ISP_MIPI_HDR_S)
            cache_list2[ISP_MIPI_HDR_S].push(sbuf_s);
        else if (i == ISP_MIPI_HDR_M)
            cache_list2[ISP_MIPI_HDR_M].push(sbuf_m);
        else if (i == ISP_MIPI_HDR_L)
            cache_list2[ISP_MIPI_HDR_L].push(sbuf_l);
    }
    _isp_hdr_fid2ready_map[sbuf_s->_seq] = true;
    _buf_mutex.unlock();

    /* this means send sof event. */
    //if (_is_offline_mode) {
    //    int mode = 1;
    //    rk_aiq_uapi2_sysctl_rawReproc_genIspParams(aiq_ctx, sbuf_s->_seq, &_finfo, mode);
    //}
    SmartPtr<EmptyClass> ec = new EmptyClass();
    _msg_queue.push(ec);
}

void
RawStreamProcUnit::trigger_isp_readback()
{
    std::map<uint32_t, bool>::iterator it_ready;
    SmartPtr<V4l2Buffer> v4l2buf[3];
    SmartPtr<V4l2BufferProxy> buf_proxy;
    SmartPtr<SimpleFdBuf> simple_buf;
    uint32_t sequence = -1;
    sint32_t additional_times = -1;
    bool isHdrGlobalTmo = false;
    struct isp2x_csi_trigger tg = {
        .sof_timestamp = 0,
        .frame_timestamp = 0,
        .frame_id = sequence,
        .times = 0,
        .mode = _mipi_dev_max == 1 ? T_START_X1 :
        _mipi_dev_max == 2 ? T_START_X2 : T_START_X3,
        /* .mode = T_START_X2, */
    };
    uint64_t sof_timestamp = 0;
    SmartLock locker (_buf_mutex);

    if (_isp_hdr_fid2ready_map.size() == 0) {
        LOGE_RKSTREAM( "%s buf not ready !", __func__);
        return;
    }

    it_ready = _isp_hdr_fid2ready_map.begin();
    sequence = it_ready->first;

    //rk_aiq_uapi2_sysctl_setAllReadyIspParams(aiq_ctx, sequence);
    // if ( _working_mode != RK_AIQ_WORKING_MODE_NORMAL) {
        // match_lumadetect_map(sequence, additional_times);
        // if (additional_times == -1) {
        // //    LOGE( "%s rdtimes not ready for seq %d !", __func__, sequence);
        // //    return;
        // additional_times = 0;//add by zyl
        // }

        // match_globaltmostate_map(sequence, isHdrGlobalTmo);
        // //if (isHdrGlobalTmo && !_camHw->getDhazState())
        // //    additional_times = 0;
    // } else {
        // additional_times = 0;
    // }
    additional_times = 0;
    _isp_hdr_fid2ready_map.erase(it_ready);


            int ret = XCAM_RETURN_NO_ERROR;

            // whether to start capturing raw files
            //if (_rawCap)
            //    _rawCap->detect_capture_raw_status(sequence, _first_trigger);

            //CaptureRawData::getInstance().detect_capture_raw_status(sequence, _first_trigger);
            //_camHw->setIsppConfig(sequence);
            for (int i = 0; i < _mipi_dev_max; i++) {
                // ret = _dev[i]->get_buffer(v4l2buf[i],
                        // cache_list[i].front()->get_v4l2_buf_index());
                // if (ret != XCAM_RETURN_NO_ERROR) {
                    // LOGE( "Rx[%d] can not get buffer\n", i);
                    // goto out;
                // } else {
                    // buf_proxy = cache_list[i].pop(-1);

                    // buf_list[i].push(buf_proxy);
                    // if (_dev[i]->get_mem_type() == V4L2_MEMORY_USERPTR)
                        // v4l2buf[i]->set_expbuf_usrptr(buf_proxy->get_v4l2_userptr());
                    // else if (_dev[i]->get_mem_type() == V4L2_MEMORY_DMABUF){
                        // v4l2buf[i]->set_expbuf_fd(buf_proxy->get_expbuf_fd());
                    // }else if (_dev[i]->get_mem_type() == V4L2_MEMORY_MMAP) {
                        // if (_dev[i]->get_use_type() == 1)
                        // {
                            // memcpy((void*)v4l2buf[i]->get_expbuf_usrptr(),(void*)buf_proxy->get_v4l2_userptr(),v4l2buf[i]->get_buf().m.planes[0].length);
                            // v4l2buf[i]->set_reserved(buf_proxy->get_v4l2_userptr());
                        // }
                    // }

                ret = _dev[i]->get_buffer(v4l2buf[i],
                        cache_list2[i].front()->_index);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_RKSTREAM("RawStreamProcUnit: Rx[%d] can not get buffer", i);
                    goto out;
                } else {
                    simple_buf = cache_list2[i].pop(-1);

                    if (_memory_type == V4L2_MEMORY_USERPTR) {
                        LOGD_RKSTREAM("RawStreamProcUnit: Rx[%d] set userptrbuf, ptr 0x%lx", i, (uintptr_t)simple_buf->_userptr);
                        v4l2buf[i]->set_expbuf_usrptr((uint64_t)simple_buf->_userptr);
                    }
                    else if (_memory_type == V4L2_MEMORY_DMABUF) {
                        LOGD_RKSTREAM("RawStreamProcUnit: Rx[%d] set dmabuf, fd %d seq %d", i, simple_buf->_fd, simple_buf->_seq);
                        v4l2buf[i]->set_expbuf_fd(simple_buf->_fd);
                    }
                    else if (_memory_type == V4L2_MEMORY_MMAP) {
                        LOGD_RKSTREAM("RawStreamProcUnit: Rx[%d] copy mmapbuf, size %d\n", i, simple_buf->_size);
                        memcpy((void*)v4l2buf[i]->get_expbuf_usrptr(),(void*)simple_buf->_userptr,simple_buf->_size);
                        v4l2buf[i]->set_reserved((uintptr_t)simple_buf->_userptr);
                        sequence = simple_buf->_seq;
                    }
                }
            }

            for (int i = 0; i < _mipi_dev_max; i++) {
                ret = _dev[i]->queue_buffer(v4l2buf[i]);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_RKSTREAM( "RawStreamProcUnit: Rx[%d] queue buffer failed", i);
                    break;
                }
            }

            tg.frame_id = sequence;
            if (_first_trigger)
                tg.times = 1;
            else
                tg.times += additional_times;

            if (tg.times > 2)
                tg.times = 2;
            if (_is_multi_cam_conc && (tg.times < 1))
                tg.times = 1;

            tg.frame_timestamp = simple_buf->_ts * 1000;
            tg.sof_timestamp = tg.frame_timestamp;
            // tg.times = 1;//fixed to three times readback
            LOGI_RKSTREAM(
                            "RawStreamProcUnit: trigger_readback camId:%d frame[%d]: sof_ts %" PRId64 "ms, frame_ts %" PRId64 "ms, globalTmo(%d), readback(%d) fd %d",
                            mCamPhyId,
                            sequence,
                            tg.sof_timestamp / 1000 / 1000,
                            tg.frame_timestamp / 1000 / 1000,
                            isHdrGlobalTmo,
                            tg.times,
                            simple_buf->_fd);

            if (ret == XCAM_RETURN_NO_ERROR)
                _isp_core_dev->io_control(RKISP_CMD_TRIGGER_READ_BACK, &tg);
            else
                LOGE_RKSTREAM("RawStreamProcUnit: trigger_readback frame[%d] queue failed!",
                                sequence);
            //if (_rawCap)
            //    _rawCap->update_capture_raw_status(_first_trigger);
            //CaptureRawData::getInstance().update_capture_raw_status(_first_trigger);
    _first_trigger = false;
out:
    return;
}

}


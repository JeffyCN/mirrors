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

#include <sys/stat.h>
#include "Isp20PollThread.h"
#include "Isp20StatsBuffer.h"
#include "rkisp2-config.h"
#include "SensorHw.h"
#include "LensHw.h"
#include "Isp20_module_dbg.h"
#include <fcntl.h>

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#ifdef ANDROID_OS
#define DEFAULT_CAPTURE_RAW_PATH "/data/capture_image"
#define CAPTURE_CNT_FILENAME "/data/.capture_cnt"
#else
#define DEFAULT_CAPTURE_RAW_PATH "/tmp/capture_image"
#define CAPTURE_CNT_FILENAME "/tmp/.capture_cnt"
#endif

#define WRITE_RAW_FILE_HEADER
/*
 * #define WRITE_ISP_REG
 * #define WRITE_ISPP_REG
 */
#define ISP_REGS_BASE 0xffb50000
#define ISPP_REGS_BASE 0xffb60000

#define RAW_FILE_IDENT 0x8080
#define HEADER_LEN 128U

/*
 * Raw file structure:
 *
+------------+-----------------+-------------+-----------------+---------------------------+
|    ITEM    |    PARAMETER    |  DATA TYPE  |  LENGTH(Bytes)  |        DESCRIPTION        |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |     Identifier  |  uint16_t   |       2         |  fixed 0x8080             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |  Header length  |  uint16_t   |       2         |  fixed 128U               |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Frame index  |  uint32_t   |       4         |                           |
|            +-----------------+-------------+-----------------+---------------------------+
|            |          Width  |  uint16_t   |       2         |  image width              |
|            +-----------------+-------------+-----------------+---------------------------+
|            |         Height  |  uint16_t   |       2         |  image height             |
|            +-----------------+-------------+-----------------+---------------------------+
|            |      Bit depth  |   uint8_t   |       1         |  image bit depth          |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  0: BGGR;  1: GBRG;       |
|            |   Bayer format  |   uint8_t   |       1         |  2: GRBG;  3: RGGB;       |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: linear                |
|    FRAME   |  Number of HDR  |             |                 |  2: long + short          |
|   HEADER   |      frame      |   uint8_t   |       1         |  3: long + mid + short    |
|            +-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |  1: short                 |
|            |  Current frame  |             |                 |  2: mid                   |
|            |       type      |   uint8_t   |       1         |  3: long                  |
|            +-----------------+-------------+-----------------+---------------------------+
|            |   Storage type  |   uint8_t   |       1         |  0: packed; 1: unpacked   |
|            +-----------------+-------------+-----------------+---------------------------+
|            |    Line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |     Effective   |             |                 |                           |
|            |    line stride  |  uint16_t   |       2         |  In bytes                 |
|            +-----------------+-------------+-----------------+---------------------------+
|            |       Reserved  |   uint8_t   |      107        |                           |
+------------+-----------------+-------------+-----------------+---------------------------+
|            |                 |             |                 |                           |
|  RAW DATA  |       RAW DATA  |    RAW      |  W * H * bpp    |  RAW DATA                 |
|            |                 |             |                 |                           |
+------------+-----------------+-------------+-----------------+---------------------------+

 */

/*
 * the structure of measuure parameters from isp in meta_data file:
 *
 * "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]"
 *
 */

namespace RkCam {

const struct capture_fmt Isp20PollThread::csirx_fmts[] =
{
    /* raw */
    {
        .fourcc = V4L2_PIX_FMT_SRGGB8,
        .bayer_fmt = 3,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG8,
        .bayer_fmt = 2,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG8,
        .bayer_fmt = 1,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR8,
        .bayer_fmt = 0,
        .pcpp = 1,
        .bpp = { 8 },
    }, {
        .fourcc = V4L2_PIX_FMT_SRGGB10,
        .bayer_fmt = 3,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG10,
        .bayer_fmt = 2,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG10,
        .bayer_fmt = 1,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR10,
        .bayer_fmt = 0,
        .pcpp = 4,
        .bpp = { 10 },
    }, {
        .fourcc = V4L2_PIX_FMT_SRGGB12,
        .bayer_fmt = 3,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGRBG12,
        .bayer_fmt = 2,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SGBRG12,
        .bayer_fmt = 1,
        .pcpp = 2,
        .bpp = { 12 },
    }, {
        .fourcc = V4L2_PIX_FMT_SBGGR12,
        .bayer_fmt = 0,
        .pcpp = 2,
        .bpp = { 12 },
    },
};

const char*
Isp20PollThread::mipi_poll_type_to_str[ISP_POLL_MIPI_MAX] =
{
    "mipi_tx_poll",
    "mipi_rx_poll",
};

class MipiPollThread
    : public Thread
{
public:
    MipiPollThread (Isp20PollThread*poll, int type, int dev_index)
        : Thread (Isp20PollThread::mipi_poll_type_to_str[type])
        , _poll (poll)
        , _type (type)
        , _dev_index (dev_index)
    {}

protected:
    virtual bool loop () {
        XCamReturn ret = _poll->mipi_poll_buffer_loop (_type, _dev_index);

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
                XCAM_RETURN_BYPASS)
            return true;
        return false;
    }

private:
    Isp20PollThread *_poll;
    int _type;
    int _dev_index;
};

bool
Isp20PollThread::get_value_from_file(const char* path, int& value, uint32_t& frameId)
{
    const char *delim = " ";
    char buffer[16] = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGV_CAMHW_SUBM(ISP20POLL_SUBM, "%s read %s failed!\n", __func__, path);
        } else {
            char *p = nullptr;

            p = strtok(buffer, delim);
            if (p != nullptr) {
                value = atoi(p);
                p = strtok(nullptr, delim);
                if (p != nullptr)
                    frameId = atoi(p);
            }
        }
        close(fp);
        LOGV_CAMHW_SUBM(ISP20POLL_SUBM, "value: %d, frameId: %d\n", value, frameId);
        return true;
    }

    return false;
}


bool
Isp20PollThread::set_value_to_file(const char* path, int value, uint32_t sequence)
{
    char buffer[16] = {0};
    int fp;

    fp = open(path, O_CREAT | O_RDWR | O_SYNC, S_IRWXU | S_IRUSR | S_IXUSR | S_IROTH | S_IXOTH);
    if (fp != -1) {
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
        snprintf(buffer, sizeof(buffer), "%3d %8d\n", _capture_raw_num, sequence);
        if (write(fp, buffer, sizeof(buffer)) <= 0)
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "%s write %s failed!\n", __func__, path);
        close(fp);
        return true;
    }

    return false;
}

SmartPtr<VideoBuffer>
Isp20PollThread::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                  SmartPtr<V4l2Device> dev,
                                  int type)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<VideoBuffer> video_buf = nullptr;

    if (type == ISP_POLL_3A_STATS) {
        rkisp_effect_params_v20 ispParams = {0};
        SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;
        SmartPtr<RkAiqIrisParamsProxy> irisParams = nullptr;
        SmartPtr<RkAiqAfInfoProxy> afParams = nullptr;

        //get exp params
        _event_handle_dev->getEffectiveExpParams(expParams, buf->get_buf().sequence);

        if (_focus_handle_dev.ptr()) {
            _focus_handle_dev->getAfInfoParams(afParams, buf->get_buf().sequence);
            _focus_handle_dev->getIrisInfoParams(irisParams, buf->get_buf().sequence);

        }

        if (mCamHw) {
            if (mCamHw->getEffectiveIspParams(ispParams, buf->get_buf().sequence) != XCAM_RETURN_NO_ERROR)
                LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "sequence(%d), get ispparams failed!!!\n",
                        buf->get_buf().sequence);
        }

        video_buf = new Isp20StatsBuffer(buf, dev, _event_handle_dev, mCamHw, afParams, irisParams);

        // write metadata && isp/ispp reg for debug
        if (_capture_metadata_num > 0) {
            char file_name[32] = {0};
            int capture_cnt = 0;
            uint32_t rawFrmId = 0;

            snprintf(file_name, sizeof(file_name), "%s", CAPTURE_CNT_FILENAME);
            get_value_from_file(file_name, capture_cnt, rawFrmId);
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "rawFrmId: %d, sequence: %d, _capture_metadata_num: %d\n",
                            rawFrmId, buf->get_buf().sequence,
                            _capture_metadata_num);
            if (_is_raw_dir_exist && buf->get_buf().sequence >= rawFrmId && \
                expParams.ptr())
#ifdef WRITE_ISP_REG
                write_reg_to_file(ISP_REGS_BASE, 0x0, 0x6000, buf->get_buf().sequence);
#endif
#ifdef WRITE_ISPP_REG
            write_reg_to_file(ISPP_REGS_BASE, 0x0, 0xc94, buf->get_buf().sequence);
#endif
            write_metadata_to_file(raw_dir_path,
                                   buf->get_buf().sequence,
                                   ispMeasParams, expParams, afParams);
            _capture_metadata_num--;
            if (!_capture_metadata_num) {
                _is_raw_dir_exist = false;
                if (_capture_raw_type == CAPTURE_RAW_SYNC) {
                    _capture_image_mutex.lock();
                    _capture_image_cond.broadcast();
                    _capture_image_mutex.unlock();
                }

                LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "stop capturing raw!\n");
            }
        }
    } else
        return PollThread::new_video_buffer(buf, dev, type);
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

XCamReturn
Isp20PollThread::notify_sof (uint64_t time, int frameid)
{
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_ASSERT(_event_handle_dev.ptr());
    ret = _event_handle_dev->handle_sof(time, frameid);
    if (get_rkaiq_runtime_dbg() > 0) {
        XCAM_STATIC_FPS_CALCULATION(SOF_FPS, 60);
    }
    if (_focus_handle_dev.ptr())
        _focus_handle_dev->handle_sof(time, frameid);

    _sof_map_mutex.lock();
    while (_sof_timestamp_map.size() > 8)
	    _sof_timestamp_map.erase(_sof_timestamp_map.begin());
    _sof_timestamp_map[frameid] = time;
    _sof_map_mutex.unlock();
    EXIT_CAMHW_FUNCTION();

    return ret;
}

bool
Isp20PollThread::set_event_handle_dev(SmartPtr<BaseSensorHw> &dev)
{
    ENTER_CAMHW_FUNCTION();
    XCAM_ASSERT (dev.ptr());
    _event_handle_dev = dev;

    rk_aiq_exposure_sensor_descriptor sns_des;
    dev->get_format(&sns_des);
    sns_width = sns_des.sensor_output_width;
    sns_height = sns_des.sensor_output_height;
    pixelformat = sns_des.sensor_pixelformat;


    if (!_linked_to_isp) {
        XCamReturn ret = XCAM_RETURN_NO_ERROR;

        // get sensor crop bounds
        struct v4l2_subdev_selection sns_sd_sel;
        memset(&sns_sd_sel, 0, sizeof(sns_sd_sel));

        ret = dev->get_selection(0, V4L2_SEL_TGT_CROP_BOUNDS, sns_sd_sel);
        if (ret) {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "get_selection failed !\n");
        } else {
            if (sns_width != sns_sd_sel.r.width ||
                    sns_height != sns_sd_sel.r.height) {
                sns_width = sns_sd_sel.r.width;
                sns_height = sns_sd_sel.r.height;
            }
        }
    }

    EXIT_CAMHW_FUNCTION();
    return true;
}

bool
Isp20PollThread::set_focus_handle_dev(SmartPtr<LensHw> &dev)
{
    ENTER_CAMHW_FUNCTION();
    XCAM_ASSERT (dev.ptr());
    _focus_handle_dev = dev;
    EXIT_CAMHW_FUNCTION();
    return true;
}

bool
Isp20PollThread::setCamHw(ICamHw *dev)
{
    ENTER_CAMHW_FUNCTION();
    XCAM_ASSERT (dev);
    mCamHw = dev;
    EXIT_CAMHW_FUNCTION();
    return true;
}

Isp20PollThread::Isp20PollThread()
    : PollThread()
    , _first_trigger(true)
    , sns_width(0)
    , sns_height(0)
    , _is_raw_dir_exist(false)
    , _is_capture_raw(false)
    , _capture_raw_num(0)
    , _capture_metadata_num(0)
    , _capture_image_mutex(false)
    , _capture_image_cond(false)
    ,  _capture_raw_type(CAPTURE_RAW_ASYNC)
    , _is_multi_cam_conc(false)
    , _skip_num(0)
    , _skip_to_seq(0)
    , _need_luma_rd_info(true)
{
    for (int i = 0; i < 3; i++) {
        SmartPtr<MipiPollThread> mipi_poll = new MipiPollThread(this, ISP_POLL_MIPI_TX, i);
        XCAM_ASSERT (mipi_poll.ptr ());
        _isp_mipi_tx_infos[i].loop = mipi_poll;

        mipi_poll = new MipiPollThread(this, ISP_POLL_MIPI_RX, i);
        XCAM_ASSERT (mipi_poll.ptr ());
        _isp_mipi_rx_infos[i].loop = mipi_poll;

        _isp_mipi_tx_infos[i].stop_fds[0] = -1;
        _isp_mipi_tx_infos[i].stop_fds[1] = -1;
        _isp_mipi_rx_infos[i].stop_fds[0] = -1;
        _isp_mipi_rx_infos[i].stop_fds[1] = -1;

    }

    LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "Isp20PollThread constructed");
}

Isp20PollThread::~Isp20PollThread()
{
    stop();

    LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "~Isp20PollThread destructed");
}


XCamReturn
Isp20PollThread::start ()
{
    if (create_stop_fds_mipi()) {
        LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "create mipi stop fds failed !");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    for (int i = 0; i < _mipi_dev_max; i++) {
        _isp_mipi_tx_infos[i].loop->start ();
        _isp_mipi_rx_infos[i].loop->start ();
    }
    return PollThread::start ();
}

XCamReturn
Isp20PollThread::stop ()
{
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_isp_mipi_tx_infos[i].dev.ptr ()) {
            if (_isp_mipi_tx_infos[i].stop_fds[1] != -1) {
                char buf = 0xf;  // random value to write to flush fd.
                unsigned int size = write(_isp_mipi_tx_infos[i].stop_fds[1], &buf, sizeof(char));
                if (size != sizeof(char))
                    LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "Flush write not completed");
            }
            _isp_mipi_tx_infos[i].loop->stop ();
            /* _isp_mipi_tx_infos[i].buf_list.clear (); */
            /* _isp_mipi_tx_infos[i].cache_list.clear (); */
        }

        if (_isp_mipi_rx_infos[i].dev.ptr ()) {
            if (_isp_mipi_rx_infos[i].stop_fds[1] != -1) {
                char buf = 0xf;  // random value to write to flush fd.
                unsigned int size = write(_isp_mipi_rx_infos[i].stop_fds[1], &buf, sizeof(char));
                if (size != sizeof(char))
                    LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "Flush write not completed");
            }
            _isp_mipi_rx_infos[i].loop->stop ();
            /* _isp_mipi_rx_infos[i].buf_list.clear (); */
            /* _isp_mipi_rx_infos[i].cache_list.clear (); */
        }
    }

    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_isp_mipi_tx_infos[i].dev.ptr ()) {
            _isp_mipi_tx_infos[i].buf_list.clear ();
            _isp_mipi_tx_infos[i].cache_list.clear ();
        }
        if (_isp_mipi_rx_infos[i].dev.ptr ()) {
            _isp_mipi_rx_infos[i].buf_list.clear ();
            _isp_mipi_rx_infos[i].cache_list.clear ();
        }
    }
    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map.clear();
    _isp_hdr_fid2ready_map.clear();
    _hdr_global_tmo_state_map.clear();
    _mipi_trigger_mutex.unlock();
    _sof_map_mutex.lock();
    _sof_timestamp_map.clear();
    _sof_map_mutex.unlock();
    destroy_stop_fds_mipi ();
    _skip_num = 0;

    return PollThread::stop ();
}

void
Isp20PollThread::set_working_mode(int mode, bool linked_to_isp, bool nordbk)
{
    _working_mode = mode;
    _linked_to_isp = linked_to_isp;

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
    if (nordbk)
        _mipi_dev_max = 0;
}

void
Isp20PollThread::set_mipi_devs(SmartPtr<V4l2Device> mipi_tx_devs[3],
                               SmartPtr<V4l2Device> mipi_rx_devs[3],
                               SmartPtr<V4l2SubDevice> isp_dev)
{
    _isp_core_dev = isp_dev;
    for (int i = 0; i < 3; i++) {
        _isp_mipi_tx_infos[i].dev = mipi_tx_devs[i];
        _isp_mipi_rx_infos[i].dev = mipi_rx_devs[i];
    }
}

void
Isp20PollThread::set_hdr_frame_readback_infos(rk_aiq_luma_params_t luma_params)
{
    _mipi_trigger_mutex.lock();
    _isp_hdr_fid2times_map[luma_params.frame_id] = luma_params;
    LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "rdtimes seq %d \n", luma_params.frame_id);
    _mipi_trigger_mutex.unlock();
    trigger_readback();
}

XCamReturn
Isp20PollThread::create_stop_fds_mipi () {
    int i, status = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    destroy_stop_fds_mipi();

    for (i = 0; i < _mipi_dev_max; i++) {
        status = pipe(_isp_mipi_tx_infos[i].stop_fds);
        if (status < 0) {
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Failed to create mipi tx:%d poll stop pipe: %s",
                            i, strerror(errno));
            ret = XCAM_RETURN_ERROR_UNKNOWN;
            goto exit_error;
        }
        status = fcntl(_isp_mipi_tx_infos[0].stop_fds[0], F_SETFL, O_NONBLOCK);
        if (status < 0) {
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Fail to set event mipi tx:%d stop pipe flag: %s",
                            i, strerror(errno));
            goto exit_error;
        }

        status = pipe(_isp_mipi_rx_infos[i].stop_fds);
        if (status < 0) {
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Failed to create mipi rx:%d poll stop pipe: %s",
                            i, strerror(errno));
            ret = XCAM_RETURN_ERROR_UNKNOWN;
            goto exit_error;
        }
        status = fcntl(_isp_mipi_rx_infos[0].stop_fds[0], F_SETFL, O_NONBLOCK);
        if (status < 0) {
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Fail to set event mipi rx:%d stop pipe flag: %s",
                            i, strerror(errno));
            goto exit_error;
        }
    }

    return XCAM_RETURN_NO_ERROR;
exit_error:
    destroy_stop_fds_mipi();
    return ret;
}

void Isp20PollThread::destroy_stop_fds_mipi () {
    for (int i = 0; i < 3; i++) {
        if (_isp_mipi_tx_infos[i].stop_fds[0] != -1 ||
                _isp_mipi_tx_infos[i].stop_fds[1] != -1) {
            close(_isp_mipi_tx_infos[i].stop_fds[0]);
            close(_isp_mipi_tx_infos[i].stop_fds[1]);
            _isp_mipi_tx_infos[i].stop_fds[0] = -1;
            _isp_mipi_tx_infos[i].stop_fds[1] = -1;
        }

        if (_isp_mipi_rx_infos[i].stop_fds[0] != -1 ||
                _isp_mipi_rx_infos[i].stop_fds[1] != -1) {
            close(_isp_mipi_rx_infos[i].stop_fds[0]);
            close(_isp_mipi_rx_infos[i].stop_fds[1]);
            _isp_mipi_rx_infos[i].stop_fds[0] = -1;
            _isp_mipi_rx_infos[i].stop_fds[1] = -1;
        }
    }
}

XCamReturn
Isp20PollThread::mipi_poll_buffer_loop (int type, int dev_index)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;
    SmartPtr<V4l2Device> dev;
    int stop_fd = -1;

    if (type == ISP_POLL_MIPI_TX) {
        dev = _isp_mipi_tx_infos[dev_index].dev;
        stop_fd = _isp_mipi_tx_infos[dev_index].stop_fds[0];
    } else if (type == ISP_POLL_MIPI_RX) {
        dev = _isp_mipi_rx_infos[dev_index].dev;
        stop_fd = _isp_mipi_rx_infos[dev_index].stop_fds[0];
    } else
        return XCAM_RETURN_ERROR_UNKNOWN;

    poll_ret = dev->poll_event (PollThread::default_poll_timeout,
                                stop_fd);

    if (poll_ret == POLL_STOP_RET) {
        LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "poll %s buffer stop success !", mipi_poll_type_to_str[type]);
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret <= 0) {
        LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "mipi_dev_index %d poll %s buffer event got error(0x%x) but continue\n",
                        dev_index, mipi_poll_type_to_str[type], poll_ret);
        ::usleep (10000); // 10ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "dequeue %s buffer failed", mipi_poll_type_to_str[type]);
        return ret;
    }

    SmartPtr<V4l2BufferProxy> buf_proxy = new V4l2BufferProxy(buf, dev);
    if (type == ISP_POLL_MIPI_TX) {
        handle_tx_buf(buf_proxy, dev_index);
    } else if (type == ISP_POLL_MIPI_RX) {
        _event_handle_dev->on_dqueue(dev_index, buf_proxy);
        handle_rx_buf(buf_proxy, dev_index);
    }

    return ret;
}

void
Isp20PollThread::handle_rx_buf(SmartPtr<V4l2BufferProxy> &rx_buf, int dev_index)
{
    if (!_isp_mipi_rx_infos[dev_index].buf_list.is_empty()) {
        SmartPtr<V4l2BufferProxy> buf = _isp_mipi_rx_infos[dev_index].buf_list.pop(-1);
        LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "%s dev_index:%d index:%d fd:%d\n",
                        __func__, dev_index, buf->get_v4l2_buf_index(), buf->get_expbuf_fd());
    }
}

void Isp20PollThread::sync_tx_buf()
{
    SmartPtr<V4l2BufferProxy> buf_s, buf_m, buf_l;
    uint32_t sequence_s = -1, sequence_m = -1, sequence_l = -1;

    _mipi_buf_mutex.lock();
    for (int i = 0; i < _mipi_dev_max; i++) {
        if (_isp_mipi_tx_infos[i].buf_list.is_empty()) {
            _mipi_buf_mutex.unlock();
            return;
        }
    }

    buf_l = _isp_mipi_tx_infos[ISP_MIPI_HDR_L].buf_list.front();
    if (buf_l.ptr())
        sequence_l = buf_l->get_sequence();

    buf_m = _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.front();
    if (buf_m.ptr())
        sequence_m = buf_m->get_sequence();

    buf_s = _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.front();

    if (buf_s.ptr()) {
        sequence_s = buf_s->get_sequence();
        if ((_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
                _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) &&
                buf_m.ptr() && buf_l.ptr() && buf_s.ptr() &&
                sequence_l == sequence_s && sequence_m == sequence_s) {

            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.erase(buf_m);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_L].buf_list.erase(buf_l);
            if (check_skip_frame(sequence_s)) {
                LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_M].cache_list.push(buf_m);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_L].cache_list.push(buf_l);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "trigger readback %d", sequence_s);
            trigger_readback();
        } else if ((_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                    _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) &&
                   buf_m.ptr() && buf_s.ptr() && sequence_m == sequence_s) {
            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            _isp_mipi_tx_infos[ISP_MIPI_HDR_M].buf_list.erase(buf_m);
            if (check_skip_frame(sequence_s)) {
                LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _isp_mipi_rx_infos[ISP_MIPI_HDR_M].cache_list.push(buf_m);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "trigger readback %d", sequence_s);
            trigger_readback();
        } else if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            _isp_mipi_tx_infos[ISP_MIPI_HDR_S].buf_list.erase(buf_s);
            if (check_skip_frame(sequence_s)) {
                LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
            _isp_mipi_rx_infos[ISP_MIPI_HDR_S].cache_list.push(buf_s);
            _mipi_trigger_mutex.lock();
            _isp_hdr_fid2ready_map[sequence_s] = true;
            _mipi_trigger_mutex.unlock();
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "trigger readback %d", sequence_s);
            trigger_readback();
        } else {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "do nothing, sequence not match l: %d, s: %d, m: %d !!!",
                            sequence_l, sequence_s, sequence_m);
        }
    }
end:
    _mipi_buf_mutex.unlock();
}

void
Isp20PollThread::handle_tx_buf(SmartPtr<V4l2BufferProxy> &tx_buf, int dev_index)
{
    LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "%s dev_index:%d sequence:%d\n",
                    __func__, dev_index, tx_buf->get_sequence());
    _isp_mipi_tx_infos[dev_index].buf_list.push(tx_buf);
    sync_tx_buf();
}

void
Isp20PollThread::trigger_readback()
{
    std::map<uint32_t, bool>::iterator it_ready;
    SmartPtr<V4l2Buffer> v4l2buf[3];
    SmartPtr<V4l2BufferProxy> buf_proxy;
    uint32_t sequence = -1;
    sint32_t additional_times = -1;
    bool isHdrGlobalTmo = false;
    bool isDhazEn = false, isTmoEn = false;
    u64 ispModuleEns = 0x0;

    _mipi_trigger_mutex.lock();

    if (!_first_trigger && _cache_tx_data) {
		/*
		 * The hdrtmo needs to get the luma in advance,
		 * so cache a frame of tx data.
		 */
        if (_isp_hdr_fid2ready_map.size() < 2) {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "%s buf not ready(%d) !",
                    __func__, _isp_hdr_fid2ready_map.size());
            _mipi_trigger_mutex.unlock();
            return;
        }
    } else {
        if (_isp_hdr_fid2ready_map.size() == 0) {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "%s buf not ready(%d) !",
                    __func__, _isp_hdr_fid2ready_map.size());
            _mipi_trigger_mutex.unlock();
            return;
        }
    }
    it_ready = _isp_hdr_fid2ready_map.begin();
    sequence = it_ready->first;

    if (!_event_handle_dev->is_virtual_sensor()) {
        match_lumadetect_map(sequence, additional_times);
        if (additional_times == -1) {
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM,
                            "%s rdtimes not ready for seq %d !",
                            __func__, sequence);
            _mipi_trigger_mutex.unlock();
            return;
        }
    }

    match_globaltmostate_map(sequence, isHdrGlobalTmo);
    _isp_hdr_fid2ready_map.erase(it_ready);

    if (mCamHw) {
        int ret = XCAM_RETURN_NO_ERROR;

        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM,
                            "%s frame[%d] set isp params failed, don't read back!\n",
                            __func__, sequence);
            // drop frame, return buf to tx
            for (int i = 0; i < _mipi_dev_max; i++) {
                _isp_mipi_rx_infos[i].cache_list.pop(-1);
            }
            goto out;
        } else {
            // whether to start capturing raw files
            char file_name[32] = {0};
            snprintf(file_name, sizeof(file_name), "%s", CAPTURE_CNT_FILENAME);
            detect_capture_raw_status(file_name, sequence);

            for (int i = 0; i < _mipi_dev_max; i++) {
                ret = _isp_mipi_rx_infos[i].dev->get_buffer(v4l2buf[i],
                        _isp_mipi_rx_infos[i].cache_list.front()->get_v4l2_buf_index());
                if (ret != XCAM_RETURN_NO_ERROR) {
                    LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Rx[%d] can not get buffer\n", i);
                    goto out;
                } else {
                    buf_proxy = _isp_mipi_rx_infos[i].cache_list.pop(-1);
#if 0
                    if (_first_trigger) {
                        u8 *buf = (u8 *)buf_proxy->get_v4l2_userptr();
                        struct v4l2_format format = v4l2buf[i]->get_format();
                        u32 bytesperline = format.fmt.pix_mp.plane_fmt[0].bytesperline;

                        if (buf) {
                            for (u32 k = 0; k < 16; k++) {
                                for (u32 j = 0; j < bytesperline / 2; j++)
                                    *buf++ += (k + j) % 16;
                                buf += bytesperline / 2;
                            }
                        }
                    }
#endif
                    _isp_mipi_rx_infos[i].buf_list.push(buf_proxy);
                    if (_isp_mipi_rx_infos[i].dev->get_mem_type() == V4L2_MEMORY_USERPTR)
                        v4l2buf[i]->set_expbuf_usrptr(buf_proxy->get_v4l2_userptr());
                    else if (_isp_mipi_rx_infos[i].dev->get_mem_type() == V4L2_MEMORY_DMABUF){
                        v4l2buf[i]->set_expbuf_fd(buf_proxy->get_expbuf_fd());
                    }else if (_isp_mipi_rx_infos[i].dev->get_mem_type() == V4L2_MEMORY_MMAP) {
                        if (_isp_mipi_tx_infos[i].dev->get_use_type() == 1)
                        {
                            memcpy((void*)v4l2buf[i]->get_expbuf_usrptr(),(void*)buf_proxy->get_v4l2_userptr(),v4l2buf[i]->get_buf().m.planes[0].length);
                            v4l2buf[i]->set_reserved(buf_proxy->get_v4l2_userptr());
                        }
                    }

                    dynamic_capture_raw(i, sequence, buf_proxy, v4l2buf[i]);
                }
            }

            for (int i = 0; i < _mipi_dev_max; i++) {
                ret = _isp_mipi_rx_infos[i].dev->queue_buffer(v4l2buf[i]);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    _isp_mipi_rx_infos[i].buf_list.pop(-1);
                    LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "Rx[%d] queue buffer failed\n", i);
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

            ispModuleEns = mCamHw->getIspModuleEnState();
            if (ispModuleEns & (1LL <<  RK_ISP2X_HDRTMO_ID))
                isTmoEn = true;
            if (ispModuleEns & (1LL << RK_ISP2X_DHAZ_ID))
                isDhazEn = true;
            /* In the case of multiple sensors, read back at least twice
             * if dhaz or tmo enable
             */
            if (_is_multi_cam_conc && (isTmoEn || isDhazEn))
                tg.times = 1;

            /* Fixed read back once:
             * 1. global TMO and dhaz is off
             * 2. tmo && dhaz is off
             */
            if (isTmoEn && isHdrGlobalTmo && !isDhazEn)
                additional_times = 0;
            else if (!isTmoEn && !isDhazEn)
                additional_times = 0;

            tg.times += additional_times;

            if (_first_trigger)
                tg.times = 1;
            else if (tg.times > 2)
                tg.times = 2;

            uint64_t sof_timestamp;
            match_sof_timestamp_map(tg.frame_id, sof_timestamp);
            tg.sof_timestamp = sof_timestamp;
            tg.frame_timestamp = buf_proxy->get_timestamp () * 1000;
            // tg.times = 1;//fixed to three times readback
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM,
                            "frame[%d]: sof_ts %" PRId64 "ms, frame_ts %" PRId64 "ms, globalTmo(%d), readback(%d)\n",
                            sequence,
                            tg.sof_timestamp / 1000 / 1000,
                            tg.frame_timestamp / 1000 / 1000,
                            isHdrGlobalTmo,
                            tg.times);

            if (ret == XCAM_RETURN_NO_ERROR)
                _isp_core_dev->io_control(RKISP_CMD_TRIGGER_READ_BACK, &tg);
            else
                LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "%s frame[%d] queue  failed, don't read back!\n",
                                __func__, sequence);

            update_capture_raw_status(file_name);
        }
    }

    _first_trigger = false;
out:
    _mipi_trigger_mutex.unlock();
}

XCamReturn
Isp20PollThread::getEffectiveLumaParams(int frame_id, rk_aiq_luma_params_t& luma_params)
{
    ENTER_CAMHW_FUNCTION();
    std::map<uint32_t, rk_aiq_luma_params_t>::iterator it;
    uint32_t search_id = frame_id < 0 ? 0 : frame_id;

    if (_isp_hdr_fid2times_map.size() == 0) {
        LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "can't search id %d,  _effecting_exp_mapsize is %d\n",
                        frame_id, _isp_hdr_fid2times_map.size());
        return  XCAM_RETURN_ERROR_PARAM;
    }

    it = _isp_hdr_fid2times_map.find(search_id);

    // havn't found
    if (it == _isp_hdr_fid2times_map.end()) {
        std::map<uint32_t, rk_aiq_luma_params_t>::reverse_iterator rit;

        rit = _isp_hdr_fid2times_map.rbegin();
        do {
            if (search_id >= rit->first) {
                LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "exp-sync: can't find id %d, get latest id %d in _isp_hdr_fid2times_map\n",
                                search_id, rit->first);
                break;
            }
        } while (++rit != _isp_hdr_fid2times_map.rend());

        if (rit == _isp_hdr_fid2times_map.rend()) {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "can't find the latest effecting exposure for id %d, impossible case !", frame_id);
            return XCAM_RETURN_ERROR_PARAM;
        }

        luma_params = rit->second;
    } else {
        luma_params = it->second;
    }

    while (_isp_hdr_fid2times_map.size() > 4)
        _isp_hdr_fid2times_map.erase(_isp_hdr_fid2times_map.begin());

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void
Isp20PollThread::match_lumadetect_map(uint32_t sequence, sint32_t &additional_times)
{
	uint32_t frame_id, target_id;

    if (!_need_luma_rd_info) {
        additional_times = 0;
        return ;
    }
	
	if (_isp_hdr_fid2times_map.empty())
		LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "luma map is empty");

	while (_isp_hdr_fid2times_map.size() > 4)
		_isp_hdr_fid2times_map.erase(_isp_hdr_fid2times_map.begin());

	std::map<uint32_t, rk_aiq_luma_params_t>::iterator iter, it_times_del;
	for (iter = _isp_hdr_fid2times_map.begin(); iter != _isp_hdr_fid2times_map.end();) {
		if (_cache_tx_data) {
			/*
			 * The hdrtmo needs to get the luma in advance,
			 * so match the luma of previous frame in Lumadetect.
			 */
			target_id = iter->first - 1;
		} else {
			target_id = iter->first;
		}

		if (target_id < sequence && iter->first != 0) {
			it_times_del = iter++;
			LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "del seq %d", it_times_del->first);
			// _isp_hdr_fid2times_map.erase(it_times_del);
		} else if (target_id == sequence || iter->first == 0) {
			additional_times = iter->second.hdrProcessCnt;
			frame_id = iter->first;
			it_times_del = iter++;
			LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "find luma id(%d)-seq id(%d)", frame_id, sequence);
			// _isp_hdr_fid2times_map.erase(it_times_del);
			break;
		} else {
			LOGW_CAMHW_SUBM(ISP20POLL_SUBM,
					"%s missing rdtimes for buf_seq %d, min rdtimes_seq %d !",
					__func__, sequence, iter->first);
			additional_times = 0;
			break;
		}
	}
}

void
Isp20PollThread::match_globaltmostate_map(uint32_t sequence, bool &isHdrGlobalTmo)
{
    std::map<uint32_t, bool>::iterator it_del;
    for (std::map<uint32_t, bool>::iterator iter = _hdr_global_tmo_state_map.begin();
            iter !=  _hdr_global_tmo_state_map.end();) {
        if (iter->first < sequence) {
            it_del = iter++;
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "del seq %d", it_del->first);
            _hdr_global_tmo_state_map.erase(it_del);
        } else if (iter->first == sequence) {
            isHdrGlobalTmo = iter->second;
            it_del = iter++;
            LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "del seq %d", it_del->first);
            _hdr_global_tmo_state_map.erase(it_del);
            break;
        } else {
            LOGW_CAMHW_SUBM(ISP20POLL_SUBM, "%s missing tmo state for buf_seq %d, min rdtimes_seq %d !",
                            __func__, sequence, iter->first);
            break;
        }
    }
}

XCamReturn
Isp20PollThread::match_sof_timestamp_map(sint32_t sequence, uint64_t &timestamp)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    std::map<int, uint64_t>::iterator it;
    sint32_t search_id = sequence < 0 ? 0 : sequence;

    _sof_map_mutex.lock();
    it = _sof_timestamp_map.find(search_id);
    if (it != _sof_timestamp_map.end()) {
        timestamp = it->second;
        _sof_timestamp_map.erase(it);
    } else {
        LOGW_CAMHW_SUBM(ISP20POLL_SUBM,
                "can't find frameid(%d), get sof timestamp failed!\n",
                sequence);
        ret = XCAM_RETURN_ERROR_FAILED;
    }
    _sof_map_mutex.unlock();

    return ret;
}

void
Isp20PollThread::write_metadata_to_file(const char* dir_path,
                                        int frame_id,
                                        rkisp_effect_params_v20& ispParams,
                                        SmartPtr<RkAiqExpParamsProxy>& expParams,
                                        SmartPtr<RkAiqAfInfoProxy>& afParams)
{
#if 0 // TODO Merge
    FILE *fp = nullptr;
    char file_name[64] = {0};
    char buffer[256] = {0};
    int32_t focusCode = 0;
    int32_t zoomCode = 0;

    snprintf(file_name, sizeof(file_name), "%s/meta_data", dir_path);

    if(afParams.ptr()) {
        focusCode = afParams->data()->focusCode;
        zoomCode = afParams->data()->zoomCode;
    }

    fp = fopen(file_name, "ab+");
    if (fp != nullptr) {
        if (_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
                _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
            snprintf(buffer,
                     sizeof(buffer),
                     "frame%08d-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]-afcode[%08d_%08d]\n",
                     frame_id,
                     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                     ispParams->data()->awb_gain.rgain,
                     ispParams->data()->awb_gain.grgain,
                     ispParams->data()->awb_gain.gbgain,
                     ispParams->data()->awb_gain.bgain,
                     1,
                     focusCode,
                     zoomCode);
        } else if (_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
                   _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
            snprintf(buffer,
                     sizeof(buffer),
                     "frame%08d-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]-afcode[%08d_%08d]\n",
                     frame_id,
                     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                     expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                     ispParams->data()->awb_gain.rgain,
                     ispParams->data()->awb_gain.grgain,
                     ispParams->data()->awb_gain.gbgain,
                     ispParams->data()->awb_gain.bgain,
                     1,
                     focusCode,
                     zoomCode);
        } else {
            snprintf(buffer,
                     sizeof(buffer),
                     "frame%08d-gain[%08.5f]-time[%08.5f]-"
                     "awbGain[%08.4f_%08.4f_%08.4f_%08.4f]-dgain[%08d]-afcode[%08d_%08d]\n",
                     frame_id,
                     expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                     expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                     ispParams->data()->awb_gain.rgain,
                     ispParams->data()->awb_gain.grgain,
                     ispParams->data()->awb_gain.gbgain,
                     ispParams->data()->awb_gain.bgain,
                     1,
                     focusCode,
                     zoomCode);
        }

        fwrite((void *)buffer, strlen(buffer), 1, fp);
        fflush(fp);
        fclose(fp);
    }
#endif
}

const struct capture_fmt* Isp20PollThread::find_fmt(const uint32_t pixelformat)
{
    const struct capture_fmt *fmt;
    unsigned int i;

    for (i = 0; i < sizeof(csirx_fmts); i++) {
        fmt = &csirx_fmts[i];
        if (fmt->fourcc == pixelformat)
            return fmt;
    }

    return NULL;
}

int Isp20PollThread::detect_capture_raw_status(const char* file_name, uint32_t sequence)
{
    if (!_is_capture_raw) {
        uint32_t rawFrmId = 0;
        get_value_from_file(file_name, _capture_raw_num, rawFrmId);

        if (_capture_raw_num > 0) {
            set_value_to_file(file_name, _capture_raw_num, sequence);
            _is_capture_raw = true;
            _capture_metadata_num = _capture_raw_num;
            if (_first_trigger)
                ++_capture_metadata_num;
        }
    }

    return 0;
}

int Isp20PollThread::update_capture_raw_status(const char* file_name)
{
    if (_is_capture_raw && !_first_trigger) {
        if (_capture_raw_type == CAPTURE_RAW_AND_YUV_SYNC) {
            _capture_image_mutex.lock();
            _capture_image_cond.timedwait(_capture_image_mutex, 3000000);
            _capture_image_mutex.unlock();
        }

        if (!--_capture_raw_num) {
            set_value_to_file(file_name, _capture_raw_num);
            _is_capture_raw = false;
        }
    }

    return 0;
}

int Isp20PollThread::dynamic_capture_raw(int i, uint32_t sequence,
        SmartPtr<V4l2BufferProxy> buf_proxy,
        SmartPtr<V4l2Buffer> &v4l2buf)
{
    if (_is_capture_raw && _capture_raw_num > 0) {
        if (!_is_raw_dir_exist) {
            if (_capture_raw_type == CAPTURE_RAW_SYNC)
                creat_raw_dir(user_set_raw_dir);
            else
                creat_raw_dir(DEFAULT_CAPTURE_RAW_PATH);
        }

        if (_is_raw_dir_exist) {
            char raw_name[128] = {0};
            FILE *fp = nullptr;

            XCAM_STATIC_PROFILING_START(write_raw);
            memset(raw_name, 0, sizeof(raw_name));
            if (_mipi_dev_max == 1)
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%d_%dx%d_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         "normal");
            else if (_mipi_dev_max == 2)
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%d_%dx%d_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         i == 0 ? "short" : "long");
            else
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%d_%dx%d_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         i == 0 ? "short" : i == 1 ? "middle" : "long");

            fp = fopen(raw_name, "wb+");
            if (fp != nullptr) {
                int size = 0;
#ifdef WRITE_RAW_FILE_HEADER
                write_frame_header_to_raw(fp, i, sequence);
#endif

#if 0
                size = v4l2buf->get_buf().m.planes[0].length;
#else
                /* raw image size compatible with ISP expansion line mode */
                size = _stride_perline * sns_height;
#endif
                write_raw_to_file(fp, i, sequence,
                                  (void *)(buf_proxy->get_v4l2_userptr()),
                                  size);
                fclose(fp);
            }
            XCAM_STATIC_PROFILING_END(write_raw, 0);
        }
    }

    return 0;
}

int Isp20PollThread::calculate_stride_per_line(const struct capture_fmt& fmt,
        uint32_t& bytesPerLine)
{
    uint32_t pixelsPerLine = 0, stridePerLine = 0;
    /* The actual size stored in the memory */
    uint32_t actualBytesPerLine = 0;

    bytesPerLine = sns_width * fmt.bpp[0] / 8;

    pixelsPerLine = fmt.pcpp * DIV_ROUND_UP(sns_width, fmt.pcpp);
    actualBytesPerLine = pixelsPerLine * fmt.bpp[0] / 8;

#if 0
    /* mipi wc(Word count) must be 4 byte aligned */
    stridePerLine = 256 * DIV_ROUND_UP(actualBytesPerLine, 256);
#else
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    _isp_mipi_tx_infos[0].dev->get_format(format);
    stridePerLine = format.fmt.pix_mp.plane_fmt[0].bytesperline;
#endif

    LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "sns_width: %d, pixelsPerLine: %d, bytesPerLine: %d, stridePerLine: %d\n",
                    sns_width,
                    pixelsPerLine,
                    bytesPerLine,
                    stridePerLine);

    return stridePerLine;
}

/*
 * Refer to "Raw file structure" in the header of this file
 */
XCamReturn
Isp20PollThread::write_frame_header_to_raw(FILE *fp, int dev_index,
        int sequence)
{
    uint8_t buffer[128] = {0};
    uint32_t stridePerLine = 0, bytesPerLine = 0;
    const struct capture_fmt* fmt = nullptr;
    uint8_t mode = 0;
    uint8_t frame_type = 0, storage_type = 0;

    if (fp == nullptr)
        return XCAM_RETURN_ERROR_PARAM;

    if ((fmt = find_fmt(pixelformat)))
        stridePerLine = calculate_stride_per_line(*fmt, bytesPerLine);

    if (_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
            _working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        mode = 3;
        frame_type = dev_index == 0 ? 1 : dev_index == 1 ? 2 : 3;
    } else if (_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
               _working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        mode = 2;
        frame_type = dev_index == 0 ? 1 : 3;
    } else {
        mode = 1;
    }

    _stride_perline = stridePerLine;

    *((uint16_t* )buffer) = RAW_FILE_IDENT;   // Identifier
    *((uint16_t* )(buffer + 2)) = HEADER_LEN;     // Header length
    *((uint32_t* )(buffer + 4)) = sequence;   // Frame number
    *((uint16_t* )(buffer + 8)) = sns_width;      // Image width
    *((uint16_t* )(buffer + 10)) = sns_height;    // Image height
    *(buffer + 12) = fmt->bpp[0];         // Bit depth
    *(buffer + 13) = fmt->bayer_fmt;          // Bayer format
    *(buffer + 14) = mode;            // Number of HDR frame
    *(buffer + 15) = frame_type;          // Current frame type
    *(buffer + 16) = storage_type;        // Storage type
    *((uint16_t* )(buffer + 17)) = stridePerLine; // Line stride
    *((uint16_t* )(buffer + 19)) = bytesPerLine;  // Effective line stride

    fwrite(buffer, sizeof(buffer), 1, fp);
    fflush(fp);

    LOGV_CAMHW_SUBM(ISP20POLL_SUBM, "frame%d: image rect: %dx%d, %d bit depth, Bayer fmt: %d, "
                    "hdr frame number: %d, frame type: %d, Storage type: %d, "
                    "line stride: %d, Effective line stride: %d\n",
                    sequence, sns_width, sns_height,
                    fmt->bpp[0], fmt->bayer_fmt, mode,
                    frame_type, storage_type, stridePerLine,
                    bytesPerLine);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
Isp20PollThread::write_raw_to_file(FILE* fp, int dev_index,
                                   int sequence, void* userptr, int size)
{
    if (fp == nullptr)
        return XCAM_RETURN_ERROR_PARAM;

    fwrite(userptr, size, 1, fp);
    fflush(fp);

    if (!dev_index) {
        for (int i = 0; i < _capture_raw_num; i++)
            printf(">");
        printf("\n");

        LOGV_CAMHW_SUBM(ISP20POLL_SUBM, "write frame%d raw\n", sequence);
    }

    return XCAM_RETURN_NO_ERROR;
}

void
Isp20PollThread::write_reg_to_file(uint32_t base_addr, uint32_t offset_addr,
                                   int len, int sequence)
{

}

XCamReturn
Isp20PollThread::creat_raw_dir(const char* path)
{
    time_t now;
    struct tm* timenow;

    if (!path)
        return XCAM_RETURN_ERROR_FAILED;

    time(&now);
    timenow = localtime(&now);

    if (access(path, W_OK) == -1) {
        if (mkdir(path, 0755) < 0)
            LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "mkdir %s error(%s)!\n",
                            path, strerror(errno));
        return XCAM_RETURN_ERROR_PARAM;
    }

    snprintf(raw_dir_path, sizeof(raw_dir_path), "%s/raw_%04d-%02d-%02d_%02d-%02d-%02d",
             path,
             timenow->tm_year + 1900,
             timenow->tm_mon + 1,
             timenow->tm_mday,
             timenow->tm_hour,
             timenow->tm_min,
             timenow->tm_sec);

    LOGV_CAMHW_SUBM(ISP20POLL_SUBM, "mkdir %s for capturing %d frames raw!\n",
                    raw_dir_path, _capture_raw_num);

    if(mkdir(raw_dir_path, 0755) < 0) {
        LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "mkdir %s error(%s)!!!\n",
                        raw_dir_path, strerror(errno));
        return XCAM_RETURN_ERROR_PARAM;
    }

    _is_raw_dir_exist = true;

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn
Isp20PollThread::notify_capture_raw()
{
    SmartLock locker(_capture_image_mutex);
    _capture_image_cond.broadcast();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
Isp20PollThread::capture_raw_ctl(capture_raw_t type, int count, const char* capture_dir, char* output_dir)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    _capture_raw_type = type;
    if (_capture_raw_type == CAPTURE_RAW_SYNC) {
        if (capture_dir != nullptr)
            snprintf(user_set_raw_dir, sizeof( user_set_raw_dir),
                     "%s/capture_image", capture_dir);
        else
            strcpy(user_set_raw_dir, DEFAULT_CAPTURE_RAW_PATH);

        char cmd_buffer[32] = {0};
        snprintf(cmd_buffer, sizeof(cmd_buffer),
                 "echo %d > %s",
                 count, CAPTURE_CNT_FILENAME);
        system(cmd_buffer);

        _capture_image_mutex.lock();
        if (_capture_image_cond.timedwait(_capture_image_mutex, 30000000) != 0)
            ret = XCAM_RETURN_ERROR_TIMEOUT;
        else
            strncpy(output_dir, raw_dir_path, strlen(raw_dir_path));
        _capture_image_mutex.unlock();
    } else if (_capture_raw_type == CAPTURE_RAW_AND_YUV_SYNC) {
        LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "capture raw and yuv images simultaneously!");
    }

    return ret;
}

void
Isp20PollThread::set_hdr_global_tmo_mode(int frame_id, bool mode)
{
    _mipi_trigger_mutex.lock();
    _hdr_global_tmo_state_map[frame_id] = mode;
    _mipi_trigger_mutex.unlock();
}

void Isp20PollThread::skip_frames(int skip_num, int32_t skip_seq)
{
    _mipi_trigger_mutex.lock();
    _skip_num = skip_num;
    _skip_to_seq = skip_seq + _skip_num;
    _mipi_trigger_mutex.unlock();
}

bool Isp20PollThread::check_skip_frame(int32_t buf_seq)
{
    _mipi_trigger_mutex.lock();
#if 0 // ts
    if (_skip_num > 0) {
        int64_t skip_ts_ms = _skip_start_ts / 1000 / 1000;
        int64_t buf_ts_ms = buf_ts / 1000;
        LOGD_CAMHW_SUBM(ISP20POLL_SUBM, "skip num  %d, start from %" PRId64 " ms,  buf ts %" PRId64 " ms",
                        _skip_num,
                        skip_ts_ms,
                        buf_ts_ms);
        if (buf_ts_ms  > skip_ts_ms) {
            _skip_num--;
            _mipi_trigger_mutex.unlock();
            return true;
        }
    }
#else

    if ((_skip_num > 0) && (buf_seq < _skip_to_seq)) {
        LOGE_CAMHW_SUBM(ISP20POLL_SUBM, "skip num  %d, skip seq %d, dest seq %d",
                        _skip_num, buf_seq, _skip_to_seq);
        _skip_num--;
        _mipi_trigger_mutex.unlock();
        return true;
    }
#endif
    _mipi_trigger_mutex.unlock();
    return false;
}

}; //namspace RkCam

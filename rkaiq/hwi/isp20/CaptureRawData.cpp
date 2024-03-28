/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include "rk_aiq_comm.h"
#include "CaptureRawData.h"

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

const struct capture_fmt CaptureRawData::csirx_fmts[] =
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

CaptureRawData::CaptureRawData ()
    : sns_width(0)
    , sns_height(0)
    , pixelformat(0)
    , _stride_perline(0)
    , _is_raw_dir_exist(false)
    , _is_capture_raw(false)
    , _capture_raw_num(0)
    , _capture_metadata_num(0)
    , _capture_image_mutex(false)
    , _capture_image_cond(false)
    , _capture_raw_type(CAPTURE_RAW_ASYNC)
    , _camId(-1)
{
}

CaptureRawData::CaptureRawData(int32_t camId)
    : sns_width(0)
    , sns_height(0)
    , pixelformat(0)
    , _stride_perline(0)
    , _is_raw_dir_exist(false)
    , _is_capture_raw(false)
    , _capture_raw_num(0)
    , _capture_metadata_num(0)
    , _capture_image_mutex(false)
    , _capture_image_cond(false)
    , _capture_raw_type(CAPTURE_RAW_ASYNC)
    , _camId(camId)
{
}

CaptureRawData::~CaptureRawData ()
{
}

const struct capture_fmt* CaptureRawData::find_fmt(const uint32_t pixelformat)
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

bool
CaptureRawData::get_value_from_file(const char* path, int& value, uint32_t& frameId)
{
    const char *delim = " ";
    char buffer[16] = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "%s read %s failed!\n", __func__, path);
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
        LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "value: %d, frameId: %d\n", value, frameId);
        return true;
    }

    return false;
}


bool
CaptureRawData::set_value_to_file(const char* path, int value, uint32_t sequence)
{
    char buffer[16] = {0};
    int fp;
    if (access(path, F_OK) == -1)
        return false;
    fp = open(path, O_CREAT | O_RDWR | O_SYNC, S_IRWXU | S_IRUSR | S_IXUSR | S_IROTH | S_IXOTH);
    if (fp != -1) {
        ftruncate(fp, 0);
        lseek(fp, 0, SEEK_SET);
        snprintf(buffer, sizeof(buffer), "%3d %8u\n", _capture_raw_num, sequence);
        if (write(fp, buffer, sizeof(buffer)) <= 0) {
            LOGW_CAMHW_SUBM(CAPTURERAW_SUBM, "%s write %s failed!\n", __func__, path);
        }
        close(fp);
        return true;
    }

    return false;
}

int CaptureRawData::detect_capture_raw_status(uint32_t sequence, bool first_trigger)
{
    char file_name[64] = {0};
    snprintf(file_name, sizeof(file_name), "%s", CAPTURE_CNT_FILENAME);
    if (!_is_capture_raw) {
        uint32_t rawFrmId = 0;

        bool ret = get_value_from_file(file_name, _capture_raw_num, rawFrmId);
        if (!ret) {
            // test multi cam mode
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
            get_value_from_file(file_name, _capture_raw_num, rawFrmId);
        }

        if (_capture_raw_num > 0) {
            bool ret = set_value_to_file(file_name, _capture_raw_num, sequence);
            if (!ret) {
                // test multi cam mode
                snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
                set_value_to_file(file_name, _capture_raw_num, sequence);
            }
            _is_capture_raw = true;
            _capture_metadata_num = _capture_raw_num;
            if (first_trigger)
                ++_capture_metadata_num;
        }
    }

    return 0;
}

int CaptureRawData::update_capture_raw_status(bool first_trigger)
{
    char file_name[64] = {0};
    snprintf(file_name, sizeof(file_name), "%.63s", CAPTURE_CNT_FILENAME);
    if (_is_capture_raw && !first_trigger) {
        if (_capture_raw_type == CAPTURE_RAW_AND_YUV_SYNC) {
            _capture_image_mutex.lock();
            _capture_image_cond.timedwait(_capture_image_mutex, 3000000);
            _capture_image_mutex.unlock();
        }

        if (!--_capture_raw_num) {
            bool ret = set_value_to_file(file_name, _capture_raw_num);
            if (!ret) {
                // test multi cam mode
                snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
                set_value_to_file(file_name, _capture_raw_num);
            }
            _is_capture_raw = false;
        }
    }

    return 0;
}

int CaptureRawData::dynamic_capture_raw
(
    int i,
    uint32_t sequence,
    SmartPtr<V4l2BufferProxy> buf_proxy,
    SmartPtr<V4l2Buffer> &v4l2buf,
    int mipi_dev_max,
    int working_mode,
    SmartPtr<V4l2Device>  dev
)
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
            sns_width = v4l2buf->get_format().fmt.pix.width;
            sns_height = v4l2buf->get_format().fmt.pix.height;
            pixelformat = v4l2buf->get_format().fmt.pix.pixelformat;
            XCAM_STATIC_PROFILING_START(write_raw);
            memset(raw_name, 0, sizeof(raw_name));
            if (mipi_dev_max == 1)
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%u_%ux%u_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         "normal");
            else if (mipi_dev_max == 2)
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%u_%ux%u_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         i == 0 ? "short" : "long");
            else
                snprintf(raw_name, sizeof(raw_name),
                         "%s/frame%u_%ux%u_%s.raw",
                         raw_dir_path,
                         sequence,
                         sns_width,
                         sns_height,
                         i == 0 ? "short" : i == 1 ? "middle" : "long");

            fp = fopen(raw_name, "wb+");
            if (fp != nullptr) {
                int size = 0;
#ifdef WRITE_RAW_FILE_HEADER
                write_frame_header_to_raw(fp, i, sequence, working_mode, dev);
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

void
CaptureRawData::write_metadata_to_file(const char* dir_path,
                                        uint32_t frame_id,
                                        rkisp_effect_params_v20& ispParams,
                                        SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
                                        SmartPtr<RkAiqAfInfoProxy>& afParams,
                                        int working_mode)
{
    FILE *fp = nullptr;
    char file_name[64] = {0};
    char buffer[256] = {0};
    int32_t focusCode = 0;
    int32_t zoomCode = 0;
    /*
     * rk_aiq_isp_meas_params_v20_t* ispMeasParams =
     *     static_cast<rk_aiq_isp_meas_params_v20_t*>(ispParams->data().ptr());
     */

    snprintf(file_name, sizeof(file_name), "%s/meta_data", dir_path);

    if(afParams.ptr()) {
        focusCode = afParams->data()->focusCode;
        zoomCode = afParams->data()->zoomCode;
    }

    fp = fopen(file_name, "ab+");
    if (fp != nullptr) {
        if (working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
                working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
            if (CHECK_ISP_HW_V20()) {
#ifdef ISP_HW_V20
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params.others.awb_gain_cfg.gain_red,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                         ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V21()) {
#ifdef ISP_HW_V21
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V30()) {
#ifdef ISP_HW_V30
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_m_s-gain[%08.5f_%08.5f_%08.5f]-time[%08.5f_%08.5f_%08.5f]-"
                         "awbGain0[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.awb_gain_cfg.gain0_red,
                         ispParams.awb_gain_cfg.gain0_green_r,
                         ispParams.awb_gain_cfg.gain0_green_b,
                         ispParams.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
        } else if (working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
                   working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
            if (CHECK_ISP_HW_V20()) {
#ifdef ISP_HW_V20
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params.others.awb_gain_cfg.gain_red,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                         ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V21()) {
#ifdef ISP_HW_V21
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V30()) {
#ifdef ISP_HW_V30
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-l_s-gain[%08.5f_%08.5f]-time[%08.5f_%08.5f]-"
                         "awbGain0[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                         expParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                         ispParams.awb_gain_cfg.gain0_red,
                         ispParams.awb_gain_cfg.gain0_green_r,
                         ispParams.awb_gain_cfg.gain0_green_b,
                         ispParams.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
        } else {
            if (CHECK_ISP_HW_V20()) {
#ifdef ISP_HW_V20
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-gain[%08.5f]-time[%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                         ispParams.isp_params.others.awb_gain_cfg.gain_red,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_r,
                         ispParams.isp_params.others.awb_gain_cfg.gain_green_b,
                         ispParams.isp_params.others.awb_gain_cfg.gain_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V21()) {
#ifdef ISP_HW_V21
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-gain[%08.5f]-time[%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v21.others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V30()) {
#ifdef ISP_HW_V30
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-gain[%08.5f]-time[%08.5f]-"
                         "awbGain[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_red,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_r,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_green_b,
                         ispParams.isp_params_v3x[0].others.awb_gain_cfg.gain0_blue,
                         1,
                         focusCode,
                         zoomCode);
#endif
            } else if (CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                snprintf(buffer,
                         sizeof(buffer),
                         "frame%08u-gain[%08.5f]-time[%08.5f]-"
                         "awbGain1[%08d_%08d_%08d_%08d]-dgain[%08d]-afcode[%08d_%08d]\n",
                         frame_id,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                         expParams->data()->aecExpInfo.LinearExp.exp_real_params.integration_time,
                         ispParams.awb_gain_cfg.awb1_gain_r,
                         ispParams.awb_gain_cfg.awb1_gain_gr,
                         ispParams.awb_gain_cfg.awb1_gain_gb,
                         ispParams.awb_gain_cfg.awb1_gain_b,
                         1,
                         focusCode,
                         zoomCode);

#endif
                }
            }
        }
        }

        fwrite((void *)buffer, strlen(buffer), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int CaptureRawData::calculate_stride_per_line(const struct capture_fmt& fmt,
        uint32_t& bytesPerLine, SmartPtr<V4l2Device>  dev)
{
    uint32_t pixelsPerLine = 0, stridePerLine = 0;
    /* The actual size stored in the memory */
    // uint32_t actualBytesPerLine = 0;

    bytesPerLine = sns_width * fmt.bpp[0] / 8;

    pixelsPerLine = fmt.pcpp * DIV_ROUND_UP(sns_width, fmt.pcpp);
    // actualBytesPerLine = pixelsPerLine * fmt.bpp[0] / 8;

#if 0
    /* mipi wc(Word count) must be 4 byte aligned */
    stridePerLine = 256 * DIV_ROUND_UP(actualBytesPerLine, 256);
#else
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    dev->get_format(format);
    stridePerLine = format.fmt.pix_mp.plane_fmt[0].bytesperline;
#endif

    LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "sns_width: %d, pixelsPerLine: %d, bytesPerLine: %d, stridePerLine: %d\n",
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
CaptureRawData::write_frame_header_to_raw(FILE *fp, int dev_index,
        int sequence, int working_mode, SmartPtr<V4l2Device>  dev)
{
    uint8_t buffer[128] = {0};
    uint32_t stridePerLine = 0, bytesPerLine = 0;
    const struct capture_fmt* fmt = nullptr;
    uint8_t mode = 0;
    uint8_t frame_type = 0, storage_type = 0;

    if (fp == nullptr)
        return XCAM_RETURN_ERROR_PARAM;

    if ((fmt = find_fmt(pixelformat)))
        stridePerLine = calculate_stride_per_line(*fmt, bytesPerLine, dev);

    if (working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || \
            working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        mode = 3;
        frame_type = dev_index == 0 ? 1 : dev_index == 1 ? 2 : 3;
    } else if (working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || \
               working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
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

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "frame%d: image rect: %dx%d, %d bit depth, Bayer fmt: %d, "
                    "hdr frame number: %d, frame type: %d, Storage type: %d, "
                    "line stride: %d, Effective line stride: %d\n",
                    sequence, sns_width, sns_height,
                    fmt->bpp[0], fmt->bayer_fmt, mode,
                    frame_type, storage_type, stridePerLine,
                    bytesPerLine);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CaptureRawData::write_raw_to_file(FILE* fp, int dev_index,
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

    }

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "dev(%d) write frame%d raw\n", dev_index, sequence);
    return XCAM_RETURN_NO_ERROR;
}

void
CaptureRawData::write_reg_to_file(uint32_t base_addr, uint32_t offset_addr,
                                   int len, int sequence)
{

}

XCamReturn
CaptureRawData::creat_raw_dir(const char* path)
{
    time_t now;
    struct tm* timenow;
    struct timeval tv;
    struct timezone tz;

    if (!path)
        return XCAM_RETURN_ERROR_FAILED;

    gettimeofday(&tv, &tz);
    time(&now);
    timenow = localtime(&now);

    if (access(path, W_OK) == -1) {
        if (mkdir(path, 0755) < 0) {
            LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!\n",
                            path, strerror(errno));
            return XCAM_RETURN_ERROR_FILE;
        }
    }

    snprintf(raw_dir_path, sizeof(raw_dir_path), "%s/Cam%d-raw_%04d-%02d-%02d_%02d-%02d-%02d-%03ld",
             path,
             _camId,
             timenow->tm_year + 1900,
             timenow->tm_mon + 1,
             timenow->tm_mday,
             timenow->tm_hour,
             timenow->tm_min,
             timenow->tm_sec,
             tv.tv_usec/1000);

    LOGV_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s for capturing %d frames raw!\n",
                    raw_dir_path, _capture_raw_num);

    if(mkdir(raw_dir_path, 0755) < 0) {
        LOGE_CAMHW_SUBM(CAPTURERAW_SUBM, "mkdir %s error(%s)!!!\n",
                        raw_dir_path, strerror(errno));
        return XCAM_RETURN_ERROR_PARAM;
    }

    _is_raw_dir_exist = true;

    return XCAM_RETURN_ERROR_PARAM;
}

XCamReturn
CaptureRawData::notify_capture_raw()
{
    SmartLock locker(_capture_image_mutex);
    _capture_image_cond.broadcast();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CaptureRawData::capture_raw_ctl(capture_raw_t type, int count, const char* capture_dir, char* output_dir)
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
                 "echo %d > %s_c%d",
                 count, CAPTURE_CNT_FILENAME, _camId);
        system(cmd_buffer);

        _capture_image_mutex.lock();
        if (_capture_image_cond.timedwait(_capture_image_mutex, 30000000) != 0)
            ret = XCAM_RETURN_ERROR_TIMEOUT;
        else {
            // TODO(FIX ME): mistaken use of strncpy.
            strcpy(output_dir, raw_dir_path);
        }
        _capture_image_mutex.unlock();
    } else if (_capture_raw_type == CAPTURE_RAW_AND_YUV_SYNC) {
        LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "capture raw and yuv images simultaneously!");
    }

    return ret;
}

void CaptureRawData::save_metadata_and_register
(
    uint32_t frameId,
    rkisp_effect_params_v20& ispParams,
    SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
    SmartPtr<RkAiqAfInfoProxy>& afParams,
    int working_mode
)
{
    if (_capture_metadata_num > 0) {
        char file_name[32] = {0};
        int capture_cnt = 0;
        uint32_t rawFrmId = 0;

        snprintf(file_name, sizeof(file_name), "%s", CAPTURE_CNT_FILENAME);
        bool ret = get_value_from_file(file_name, capture_cnt, rawFrmId);
        if (!ret) {
            // test multi cam mode
            snprintf(file_name, sizeof(file_name), "%.50s_c%d", CAPTURE_CNT_FILENAME, _camId);
            get_value_from_file(file_name, capture_cnt, rawFrmId);
        }
        LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "rawFrmId: %d, sequence: %d, _capture_metadata_num: %d\n",
                        rawFrmId, frameId,
                        _capture_metadata_num);
        if (_is_raw_dir_exist && frameId >= rawFrmId && expParams.ptr()) {
#ifdef WRITE_ISP_REG
            write_reg_to_file(ISP_REGS_BASE, 0x0, 0x6000, frameId);
#endif
#ifdef WRITE_ISPP_REG
            write_reg_to_file(ISPP_REGS_BASE, 0x0, 0xc94, frameId);
#endif
            write_metadata_to_file(raw_dir_path,
                    frameId,
                    ispParams, expParams, afParams,working_mode);

            _capture_metadata_num--;
            if (!_capture_metadata_num) {
                _is_raw_dir_exist = false;
                if (_capture_raw_type == CAPTURE_RAW_SYNC) {
                    _capture_image_mutex.lock();
                    _capture_image_cond.broadcast();
                    _capture_image_mutex.unlock();
                }

                LOGD_CAMHW_SUBM(CAPTURERAW_SUBM, "stop capturing raw!\n");
            }
        }
    }
}

} //namspace RkCam

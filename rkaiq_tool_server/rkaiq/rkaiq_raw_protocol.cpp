#include "rkaiq_raw_protocol.h"

#include "multiframe_process.h"
#include "rkaiq_protocol.h"
#include "rk-camera-module.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <regex>
#include <sstream>
using namespace std;

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

static int capture_status = READY;
static int capture_mode = CAPTURE_NORMAL;
static int capture_frames = 1;
static int capture_frames_index = 0;
static uint16_t capture_check_sum;
static struct capture_info cap_info;
static uint32_t* averge_frame0;
static uint16_t* averge_frame1;
static int needSetParamFlag = 1;

extern int g_sensorMemoryMode;
extern int g_sensorSyncMode;
extern std::string g_sensor_name;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern int g_device_id;
extern uint32_t g_sensorHdrMode;

static void ExecuteCMD(const char* cmd, char* result)
{
    char buf_ps[2048];
    char ps[2048] = {0};
    FILE* ptr;
    strcpy(ps, cmd);
    if ((ptr = popen(ps, "r")) != NULL)
    {
        while (fgets(buf_ps, 2048, ptr) != NULL)
        {
            strcat(result, buf_ps);
            if (strlen(result) > 2048)
            {
                break;
            }
        }
        pclose(ptr);
        ptr = NULL;
    }
    else
    {
        printf("popen %s error\n", ps);
    }
}

static void SendMessageToPC(int sockfd, char* data, unsigned long long dataSize = 0)
{
    if (dataSize == 0)
    {
        dataSize = strlen(data);
    }
    unsigned long long packetSize = strlen("#&#^ToolServerMsg#&#^") + strlen("#&#^@`#`@`#`") + dataSize;
    unsigned long long offSet = 0;
    char* dataToSend = (char*)malloc(packetSize);
    memcpy(dataToSend + offSet, "#&#^ToolServerMsg#&#^", strlen("#&#^ToolServerMsg#&#^"));
    offSet += strlen("#&#^ToolServerMsg#&#^");
    memcpy(dataToSend + offSet, data, dataSize);
    offSet += dataSize;
    memcpy(dataToSend + offSet, "#&#^@`#`@`#`", strlen("#&#^@`#`@`#`"));
    offSet += strlen("#&#^@`#`@`#`");

    send(sockfd, dataToSend, packetSize, 0);
    free(dataToSend);
}

static int SetLHcg(int mode)
{
    if (needSetParamFlag == 0)
    {
        LOG_INFO("Online mode, not set SetLHcg\n");
        return 0;
    }

    int fd = device_open(cap_info.sd_path.device_name);
    LOG_DEBUG("SetLHcg, sensor subdev path: %s\n", cap_info.sd_path.device_name);
    if (fd < 0)
    {
        LOG_ERROR("Open %s failed.\n", cap_info.sd_path.device_name);
    }
    else
    {
        int ret = ioctl(fd, RKMODULE_SET_CONVERSION_GAIN, &mode);
        if (ret > 0)
        {
            LOG_ERROR("SetLHcg failed\n");
        }
        else
        {
            LOG_INFO("SetLHcg :%d\n", mode);
        }
    }
    close(fd);
    return 0;
}

static void InitCommandStreamingAns(CommandData_t* cmd, int ret_status)
{
    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_STREAMING;
    cmd->cmdID = 0xffff;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void InitCommandPingAns(CommandData_t* cmd, int ret_status)
{
    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_STATUS;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void InitCommandRawCapAns(CommandData_t* cmd, int ret_status)
{
    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x00; // ProcessID
    cmd->dat[1] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void RawCaptureinit(CommandData_t* cmd)
{
    char* buf = (char*)(cmd->dat);
    Capture_Reso_t* Reso = (Capture_Reso_t*)(cmd->dat + 1);

    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
    if (mi.cif.linked_sensor)
    {
        cap_info.link = link_to_vicap;
        strcpy(cap_info.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
        strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
        cap_info.dev_name = cap_info.cif_path.cif_video_path;
    }
    else if (mi.dvp.linked_sensor)
    {
        cap_info.link = link_to_dvp;
        strcpy(cap_info.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
        strcpy(cap_info.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
        cap_info.dev_name = cap_info.cif_path.cif_video_path;
    }
    else
    {
        cap_info.link = link_to_isp;
        strcpy(cap_info.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
        strcpy(cap_info.vd_path.isp_main_path, mi.isp.main_path.c_str());
        cap_info.dev_name = cap_info.vd_path.isp_main_path;
    }
    strcpy(cap_info.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
    strcpy(cap_info.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
    if (mi.lens.module_lens_dev_name.length())
    {
        strcpy(cap_info.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
    }
    else
    {
        cap_info.lens_path.lens_device_name[0] = '\0';
    }
    cap_info.dev_fd = -1;
    cap_info.subdev_fd = -1;
    cap_info.lensdev_fd = -1;
    LOG_DEBUG("cap_info.link: %d \n", cap_info.link);
    LOG_DEBUG("cap_info.dev_name: %s \n", cap_info.dev_name);
    LOG_DEBUG("cap_info.isp_media_path: %s \n", cap_info.vd_path.media_dev_path);
    LOG_DEBUG("cap_info.vd_path.isp_sd_path: %s \n", cap_info.vd_path.isp_sd_path);
    LOG_DEBUG("cap_info.sd_path.device_name: %s \n", cap_info.sd_path.device_name);
    LOG_DEBUG("cap_info.lens_path.lens_dev_name: %s \n", cap_info.lens_path.lens_device_name);

    cap_info.io = IO_METHOD_MMAP;
    cap_info.height = Reso->height;
    cap_info.width = Reso->width;
    // cap_info.format = v4l2_fourcc('B', 'G', '1', '2');
    LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info.width, cap_info.height);

    //
    int fd = device_open(cap_info.sd_path.device_name);
    LOG_DEBUG("sensor subdev path: %s\n", cap_info.sd_path.device_name);
    LOG_DEBUG("cap_info.subdev_fd: %d\n", fd);
    if (fd < 0)
    {
        LOG_ERROR("Open %s failed.\n", cap_info.sd_path.device_name);
    }
    else
    {
        rkmodule_hdr_cfg hdrCfg;
        int ret = ioctl(fd, RKMODULE_GET_HDR_CFG, &hdrCfg);
        if (ret > 0)
        {
            g_sensorHdrMode = NO_HDR;
            LOG_ERROR("Get sensor hdr mode failed, use default, No HDR\n");
        }
        else
        {
            g_sensorHdrMode = hdrCfg.hdr_mode;
            LOG_INFO("Get sensor hdr mode:%u\n", g_sensorHdrMode);
            hdrCfg.hdr_mode = 0;
            ret = ioctl(fd, RKMODULE_SET_HDR_CFG, &hdrCfg);
            LOG_INFO("Set sensor to no hdr mode, ret=%d\n", ret);

            ret = ioctl(fd, RKMODULE_GET_HDR_CFG, &hdrCfg);
            if (ret > 0)
            {
                g_sensorHdrMode = NO_HDR;
                LOG_ERROR("Get sensor hdr mode again failed, use default, No HDR\n");
            }
            else
            {
                g_sensorHdrMode = hdrCfg.hdr_mode;
                LOG_INFO("Get sensor hdr mode again:%u\n", g_sensorHdrMode);
            }
            LOG_INFO("Set sensor to no hdr mode finish\n");
        }
    }
    close(fd);
    if (mi.cif.linked_sensor)
    {
        if (g_sensorHdrMode == NO_HDR)
        {
            LOG_INFO("Get sensor mode: NO_HDR\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else if (g_sensorHdrMode == HDR_X2)
        {
            LOG_INFO("Get sensor mode: HDR_2\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
        else if (g_sensorHdrMode == HDR_X3)
        {
            LOG_INFO("Get sensor mode: HDR_3\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id2.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
    }
}

static void RawCaptureDeinit()
{
    if (cap_info.subdev_fd > 0)
    {
        device_close(cap_info.subdev_fd);
        cap_info.subdev_fd = -1;
        LOG_DEBUG("device_close(cap_info.subdev_fd)\n");
    }
    if (cap_info.dev_fd > 0)
    {
        device_close(cap_info.dev_fd);
        cap_info.dev_fd = -1;
        LOG_DEBUG("device_close(cap_info.dev_fd)\n");
    }
}

static void GetSensorPara(CommandData_t* cmd, int ret_status)
{
    struct v4l2_queryctrl ctrl;
    struct v4l2_subdev_frame_interval finterval;
    struct v4l2_subdev_format fmt;
    struct v4l2_format format;

    memset(cmd, 0, sizeof(CommandData_t));

    Sensor_Params_t* sensorParam = (Sensor_Params_t*)(&cmd->dat[1]);
    int hblank, vblank;
    int vts, hts, ret;
    float fps;
    int endianness = 0;

    cap_info.subdev_fd = device_open(cap_info.sd_path.device_name);
    LOG_DEBUG("sensor subdev path: %s\n", cap_info.sd_path.device_name);

    uint16_t sensorFormat;
    // set capture image data format
    if (g_sensorHdrMode == NO_HDR)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_NON_COMPACT_LINEAR;
        LOG_INFO("NO_HDR | sensorFormat:%d\n", sensorFormat);
    }
    else if (g_sensorHdrMode == HDR_X2)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN256;
        LOG_INFO("HDR_X2 | sensorFormat:%d\n", sensorFormat);
    }
    else if (g_sensorHdrMode == HDR_X3)
    {
        sensorFormat = PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN256;
        LOG_INFO("HDR_X3 | sensorFormat:%d\n", sensorFormat);
    }
    else
    {
        LOG_ERROR("Get sensor hdr mode failed, hdr mode:%u, use default.No HDR\n", g_sensorHdrMode);
        sensorFormat = PROC_ID_CAPTURE_RAW_NON_COMPACT_LINEAR;
        LOG_INFO("NO_HDR | sensorFormat:%d\n", sensorFormat);
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (device_getblank(cap_info.subdev_fd, &ctrl) < 0)
    {
        // todo
        sensorParam->status = RES_FAILED;
        goto end;
    }
    hblank = ctrl.minimum;
    LOG_DEBUG("get hblank: %d\n", hblank);

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (device_getblank(cap_info.subdev_fd, &ctrl) < 0)
    {
        // todo
        sensorParam->status = RES_FAILED;
        goto end;
    }
    vblank = ctrl.minimum;
    LOG_DEBUG("get vblank: %d\n", vblank);

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    if (device_getsubdevformat(cap_info.subdev_fd, &fmt) < 0)
    {
        sensorParam->status = RES_FAILED;
        goto end;
    }
    vts = vblank + fmt.format.height;
    hts = hblank + fmt.format.width;
    LOG_DEBUG("get hts: %d  vts: %d\n", hts, vts);
    cap_info.format = convert_to_v4l2fmt(&cap_info, fmt.format.code);
    cap_info.sd_path.sen_fmt = fmt.format.code;
    cap_info.sd_path.width = fmt.format.width;
    cap_info.sd_path.height = fmt.format.height;

    LOG_DEBUG("get sensor code: %d  bits: %d, cap_info.format:  %d\n", cap_info.sd_path.sen_fmt, cap_info.sd_path.bits, cap_info.format);

    /* set isp subdev fmt to bayer raw*/
    if (cap_info.link == link_to_isp)
    {
        ret = rkisp_set_ispsd_fmt(&cap_info, fmt.format.width, fmt.format.height, fmt.format.code, cap_info.width, cap_info.height, fmt.format.code);
        endianness = 1;
        LOG_DEBUG("rkisp_set_ispsd_fmt: %d endianness = %d\n", ret, endianness);

        if (ret)
        {
            LOG_ERROR("subdev choose the best fit fmt: %dx%d, 0x%08x\n", fmt.format.width, fmt.format.height, fmt.format.code);
            sensorParam->status = RES_FAILED;
            goto end;
        }
    }

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;
    if (device_getsensorfps(cap_info.subdev_fd, &finterval) < 0)
    {
        sensorParam->status = RES_FAILED;
        goto end;
    }
    fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
    LOG_DEBUG("get fps: %f\n", fps);

    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_RAW_CAPTURE;
    cmd->datLen = sizeof(Sensor_Params_t);
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x01;

    sensorParam->status = ret_status;
    sensorParam->fps = fps;
    sensorParam->hts = hts;
    sensorParam->vts = vts;
    sensorParam->bits = cap_info.sd_path.bits;
    sensorParam->endianness = endianness;
    sensorParam->sensorImageFormat = sensorFormat;

    LOG_DEBUG("sensorParam->endianness: %d\n", endianness);

    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
    LOG_DEBUG("cmd->checkSum %d\n", cmd->checkSum);

    if (cap_info.subdev_fd > 0)
    {
        device_close(cap_info.subdev_fd);
        cap_info.subdev_fd = -1;
    }
    if (cap_info.dev_fd > 0)
    {
        device_close(cap_info.dev_fd);
        cap_info.dev_fd = -1;
    }
    return;

end:
    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = PC_TO_DEVICE;
    cmd->cmdID = CMD_ID_CAPTURE_RAW_CAPTURE;
    cmd->datLen = sizeof(Sensor_Params_t);
    cmd->dat[0] = 0x01;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
    if (cap_info.subdev_fd > 0)
    {
        device_close(cap_info.subdev_fd);
        cap_info.subdev_fd = -1;
    }
}

static void SetCapConf(CommandData_t* recv_cmd, CommandData_t* cmd, int ret_status)
{
    // change raw cap format to no compact
    LOG_INFO("change raw cap format to no compact\n");

    int fd = open(cap_info.dev_name, O_RDWR, 0);
    LOG_INFO("fd: %d\n", fd);
    if (fd < 0)
    {
        LOG_ERROR("Open dev %s failed.\n", cap_info.dev_name);
    }
    else
    {
        int ret = ioctl(fd, RKCIF_CMD_GET_CSI_MEMORY_MODE, &g_sensorMemoryMode); // get original memory mode
        if (ret > 0)
        {
            LOG_ERROR("get cif node %s memory mode failed.\n", cap_info.dev_name);
        }
        else
        {
            LOG_INFO("get cif node memory mode:%d .\n", g_sensorMemoryMode);
        }

        if (g_sensorHdrMode == NO_HDR)
        {
            int value = CSI_LVDS_MEM_WORD_LOW_ALIGN;
            int ret = ioctl(fd, RKCIF_CMD_SET_CSI_MEMORY_MODE, &value); // set to no compact
            if (ret > 0)
            {
                LOG_ERROR("set cif node %s compact mode failed.\n", cap_info.dev_name);
            }
            else
            {
                LOG_INFO("cif node %s set to no compact mode.\n", cap_info.dev_name);
            }
        }
        else
        {
            LOG_INFO("cif node HDR mode, compact format as default.\n");
        }

        // set sync mode to no sync for dual camera
        int sensorfd = open(cap_info.sd_path.device_name, O_RDWR, 0);
        ret = ioctl(sensorfd, RKMODULE_GET_SYNC_MODE, &g_sensorSyncMode); // get original memory mode
        if (ret > 0)
        {
            LOG_ERROR("get cif node %s sync mode failed.\n", cap_info.sd_path.device_name);
        }
        else
        {
            LOG_INFO("get cif node sync mode:%d .\n", g_sensorSyncMode);
        }
        int value = NO_SYNC_MODE;
        ret = ioctl(sensorfd, RKMODULE_SET_SYNC_MODE, &value); // set to no sync
        if (ret > 0)
        {
            LOG_ERROR("set cif node %s sync mode failed.\n", cap_info.sd_path.device_name);
        }
        else
        {
            LOG_INFO("cif node %s set to no sync mode.\n", cap_info.sd_path.device_name);
        }
        close(sensorfd);
    }
    close(fd);
    //
    needSetParamFlag = 1;

    char result[2048] = {0};
    std::string pattern{"Isp online"};
    std::regex re(pattern);
    std::smatch results;
    ExecuteCMD("cat /proc/rkisp0-vir0", result);
    std::string srcStr = result;
    // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
    std::regex_search(srcStr, results, re);
    if (results.length() > 0)
    {
        needSetParamFlag = 0;
        LOG_INFO("Online capture raw not set param.\n");
    }
    memset(result, 0, sizeof(result));
    ExecuteCMD("cat /proc/rkisp1-vir0", result);
    srcStr = result;
    // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
    std::regex_search(srcStr, results, re);
    if (results.length() > 0)
    {
        needSetParamFlag = 0;
        LOG_INFO("Online capture raw not set param.\n");
    }
    memset(result, 0, sizeof(result));
    ExecuteCMD("cat /proc/rkisp-vir0", result);
    srcStr = result;
    // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
    std::regex_search(srcStr, results, re);
    if (results.length() > 0)
    {
        needSetParamFlag = 0;
        LOG_INFO("Online capture raw not set param.\n");
    }
    memset(result, 0, sizeof(result));
    ExecuteCMD("cat /proc/rkisp-unite", result);
    srcStr = result;
    // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
    std::regex_search(srcStr, results, re);
    if (results.length() > 0)
    {
        needSetParamFlag = 0;
        LOG_INFO("Online capture raw not set param.\n");
    }

    //
    bool focus_enable = false;
    memset(cmd, 0, sizeof(CommandData_t));
    Capture_Params_t* CapParam = (Capture_Params_t*)(recv_cmd->dat + 1);

    for (int i = 0; i < recv_cmd->datLen; i++)
    {
        LOG_DEBUG("data[%d]: 0x%x\n", i, recv_cmd->dat[i]);
    }

    cap_info.subdev_fd = device_open(cap_info.sd_path.device_name);
    if (strlen(cap_info.lens_path.lens_device_name) > 0)
    {
        focus_enable = true;
    }
    if (focus_enable)
        cap_info.lensdev_fd = device_open(cap_info.lens_path.lens_device_name);

    capture_frames = CapParam->framenumber;
    capture_frames_index = 0;
    cap_info.frame_count = CapParam->framenumber;
    cap_info.lhcg = CapParam->lhcg;
    capture_mode = CapParam->multiframe;
    capture_check_sum = 0;

    if (needSetParamFlag == 1)
    {
        LOG_INFO(" set gain        : %d\n", CapParam->gain);
        LOG_INFO(" set exposure    : %d\n", CapParam->time);
        LOG_INFO(" set lhcg        : %d\n", CapParam->lhcg);
        LOG_INFO(" set bits        : %d\n", CapParam->bits);
        LOG_INFO(" set framenumber : %d\n", CapParam->framenumber);
        LOG_INFO(" set multiframe  : %d\n", CapParam->multiframe);
        LOG_INFO(" set vblank      : %d\n", CapParam->vblank);
        LOG_INFO(" set focus       : %d\n", CapParam->focus_position);
        LOG_INFO(" sd_path subdev  : %s\n", cap_info.sd_path.device_name);

        struct v4l2_control exp;
        exp.id = V4L2_CID_EXPOSURE;
        exp.value = CapParam->time;
        struct v4l2_control gain;
        gain.id = V4L2_CID_ANALOGUE_GAIN;
        gain.value = CapParam->gain;
        struct v4l2_control vblank;
        vblank.id = V4L2_CID_VBLANK;
        vblank.value = CapParam->vblank;
        struct v4l2_control focus;

        if (focus_enable)
        {
            struct v4l2_queryctrl focus_query;
            focus_query.id = V4L2_CID_FOCUS_ABSOLUTE;
            if (device_queryctrl(cap_info.lensdev_fd, &focus_query) < 0)
            {
                LOG_ERROR(" query focus result failed to device\n");
                focus_enable = false;
            }
            focus.id = V4L2_CID_FOCUS_ABSOLUTE;
            focus.value = CapParam->focus_position;
            if ((int)CapParam->focus_position > focus_query.maximum)
                focus.value = focus_query.maximum;
            if ((int)CapParam->focus_position < focus_query.minimum)
                focus.value = focus_query.minimum;
        }

        if (device_setctrl(cap_info.subdev_fd, &vblank) < 0)
        {
            LOG_ERROR(" set vblank result failed to device\n");
            ret_status = RES_FAILED;
        }
        if (device_setctrl(cap_info.subdev_fd, &exp) < 0)
        {
            LOG_ERROR(" set exposure result failed to device\n");
            ret_status = RES_FAILED;
        }
        if (device_setctrl(cap_info.subdev_fd, &gain) < 0)
        {
            LOG_ERROR(" set gain result failed to device\n");
            ret_status = RES_FAILED;
        }
        if (focus_enable)
        {
            if (device_setctrl(cap_info.lensdev_fd, &focus) < 0)
            {
                LOG_ERROR(" set focus result failed to device\n");
                ret_status = RES_FAILED;
            }
        }
    }

    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x02;
    cmd->dat[1] = ret_status;
    for (int i = 0; i < cmd->datLen; i++)
    {
        LOG_DEBUG("data[%d]: 0x%x\n", i, cmd->dat[i]);
    }
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
    LOG_DEBUG("cmd->checkSum %d\n", cmd->checkSum);

    if (cap_info.subdev_fd > 0)
    {
        device_close(cap_info.subdev_fd);
        cap_info.subdev_fd = -1;
    }
}

static void SendRawData(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("SendRawData\n");
    assert(buffer);

    char* buf = NULL;
    int total = size;
    int packet_len = MAXPACKETSIZE;
    int send_size = 0;
    int ret_val;
    uint16_t check_sum = 0;
    buf = (char*)buffer;
    while (total > 0)
    {
        if (total < packet_len)
        {
            send_size = total;
        }
        else
        {
            send_size = packet_len;
        }
        ret_val = send(socket, buf, send_size, 0);
        total -= send_size;
        buf += ret_val;
    }

    buf = (char*)buffer;
    for (int i = 0; i < size; i++)
    {
        check_sum += buf[i];
    }

    LOG_INFO("capture raw index %d, check_sum %d capture_check_sum %d\n", index, check_sum, capture_check_sum);
    capture_check_sum = check_sum;
}

static void DoCaptureCallBack(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("DoCaptureCallBack size %d\n", size);
    int width = cap_info.width;
    int height = cap_info.height;
    LOG_DEBUG("cap_info.width %d\n", cap_info.width);
    LOG_DEBUG("cap_info.height %d\n", cap_info.height);
    if (g_sensorHdrMode == NO_HDR && size > (width * height * 2))
    {
        LOG_ERROR("DoMultiFrameCallBack size error\n");
        return;
    }
    SendRawData(socket, index, buffer, size);
    // SendRawDataWithExpInfo(socket, index, buffer, size, &cap_info.expInfo, sizeof(ExpInfo_t1));
}

static void DoCapture(int socket)
{
    LOG_DEBUG("DoCapture entry!!!!!\n");
    AutoDuration ad;
    int skip_frame = 5;

    if (capture_frames_index == 0)
    {
        for (int i = 0; i < skip_frame; i++)
        {
            if (i == 0 && cap_info.lhcg != 2)
            {
                SetLHcg(cap_info.lhcg);
            }
            read_frame(socket, i, &cap_info, nullptr);
            LOG_DEBUG("DoCapture skip frame %d ...\n", i);
        }
    }

    read_frame(socket, capture_frames_index, &cap_info, DoCaptureCallBack);
    capture_frames_index++;

    LOG_INFO("DoCapture %lld ms %lld us\n", ad.Get() / 1000, ad.Get() % 1000);
    LOG_DEBUG("DoCapture exit!!!!!\n");
}

#if DEBUG_RAW
void WriteToFile(int index, void* buffer, int size)
{
    std::string path = "/data/frame_";
    path.append(std::to_string(index));
    path.append(".raw");
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(buffer), size);
}
#endif

static void DoMultiFrameCallBack(int socket, int index, void* buffer, int size)
{
    LOG_DEBUG("DoMultiFrameCallBack index %d buffer %p size %d\n", index, buffer, size);
    AutoDuration ad;
    int width = cap_info.width;
    int height = cap_info.height;

    if (size > (width * height * 2))
    {
        LOG_ERROR(" DoMultiFrameCallBack size error\n");
        return;
    }

#if DEBUG_RAW
    WriteToFile(index, buffer, size);
#endif
    int offset = (((height / 2) + 10) * width) + (width / 2);
    DumpRawData((uint16_t*)buffer + offset, size, 2);
    if (cap_info.link == link_to_vicap)
    {
        MultiFrameAddition((uint32_t*)averge_frame0, (uint16_t*)buffer, width, height, false);
    }
    else
    {
        MultiFrameAddition((uint32_t*)averge_frame0, (uint16_t*)buffer, width, height);
    }
    DumpRawData32((uint32_t*)averge_frame0 + offset, size, 2);
    LOG_DEBUG("index %d MultiFrameAddition %lld ms %lld us\n", index, ad.Get() / 1000, ad.Get() % 1000);
    ad.Reset();
    if (index == (capture_frames - 1))
    {
        MultiFrameAverage(averge_frame0, averge_frame1, width, height, capture_frames);
#if DEBUG_RAW
        WriteToFile(88, averge_frame0, size);
        WriteToFile(89, averge_frame1, size);
#endif
        DumpRawData32((uint32_t*)averge_frame0 + offset, size, 2);
        DumpRawData((uint16_t*)averge_frame1 + offset, size, 2);
        LOG_DEBUG("index %d MultiFrameAverage %lld ms %lld us\n", index, ad.Get() / 1000, ad.Get() % 1000);
        ad.Reset();
        SendRawData(socket, index, averge_frame1, size);
        LOG_DEBUG("index %d SendRawData %lld ms %lld us\n", index, ad.Get() / 1000, ad.Get() % 1000);
    }
    else if (index == ((capture_frames >> 1) - 1))
    {
        SendRawData(socket, index, buffer, size);
        LOG_DEBUG("index %d SendRawData %lld ms %lld us\n", index, ad.Get() / 1000, ad.Get() % 1000);
    }
}

static int InitMultiFrame()
{
    uint32_t one_frame_size = cap_info.width * cap_info.height * sizeof(uint32_t);
    averge_frame0 = (uint32_t*)malloc(one_frame_size);
    memset(averge_frame0, 0, one_frame_size);
    one_frame_size = one_frame_size >> 1;
    averge_frame1 = (uint16_t*)malloc(one_frame_size);
    memset(averge_frame1, 0, one_frame_size);
    return 0;
}

static int deInitMultiFrame()
{
    if (averge_frame0 != nullptr)
    {
        free(averge_frame0);
    }
    if (averge_frame1 != nullptr)
    {
        free(averge_frame1);
    }
    averge_frame0 = nullptr;
    averge_frame1 = nullptr;
    return 0;
}

static void DoMultiFrameCapture(int socket)
{
    LOG_DEBUG("DoMultiFrameCapture entry!!!!!\n");
    AutoDuration ad;

    int skip_frame = 5;
    if (capture_frames_index == 0)
    {
        for (int i = 0; i < skip_frame; i++)
        {
            if (i == 0 && cap_info.lhcg != 2)
            {
                SetLHcg(cap_info.lhcg);
            }
            read_frame(socket, i, &cap_info, nullptr);
            LOG_DEBUG("DoCapture skip frame %d ...\n", i);
        }
    }

    if (capture_frames_index == 0)
    {
        for (int i = 0; i < (capture_frames >> 1); i++)
        {
            read_frame(socket, i, &cap_info, DoMultiFrameCallBack);
            capture_frames_index++;
        }
    }
    else if (capture_frames_index == (capture_frames >> 1))
    {
        for (int i = (capture_frames >> 1); i < capture_frames; i++)
        {
            read_frame(socket, i, &cap_info, DoMultiFrameCallBack);
            capture_frames_index++;
        }
    }

    LOG_INFO("DoMultiFrameCapture %lld ms %lld us\n", ad.Get() / 1000, ad.Get() % 1000);
    LOG_DEBUG("DoMultiFrameCapture exit!!!!!\n");
}

static void DumpCapinfo()
{
    LOG_INFO("DumpCapinfo: \n");
    LOG_INFO("    dev_name ------------- %s\n", cap_info.dev_name);
    LOG_INFO("    dev_fd --------------- %d\n", cap_info.dev_fd);
    LOG_INFO("    io ------------------- %d\n", cap_info.io);
    LOG_INFO("    width ---------------- %d\n", cap_info.width);
    LOG_INFO("    height --------------- %d\n", cap_info.height);
    LOG_INFO("    format --------------- %d\n", cap_info.format);
    LOG_INFO("    capture_buf_type ----- %d\n", cap_info.capture_buf_type);
    LOG_INFO("    out_file ------------- %s\n", cap_info.out_file);
    LOG_INFO("    frame_count ---------- %d\n", cap_info.frame_count);
}

static int StartCapture()
{
    LOG_DEBUG("enter\n");
    init_device(&cap_info);
    DumpCapinfo();
    start_capturing(&cap_info);
    if (capture_mode != CAPTURE_NORMAL)
    {
        InitMultiFrame();
    }
    LOG_DEBUG("exit\n");
    return 0;
}

static int StopCapture()
{
    LOG_DEBUG("enter\n");
    stop_capturing(&cap_info);
    uninit_device(&cap_info);
    RawCaptureDeinit();
    if (capture_mode != CAPTURE_NORMAL)
    {
        deInitMultiFrame();
    }
    LOG_DEBUG("exit\n");
    return 0;
}

static void RawCaputure(CommandData_t* cmd, int socket)
{
    LOG_DEBUG("enter\n");
    LOG_DEBUG("capture_frames %d capture_frames_index %d\n", capture_frames, capture_frames_index);
    if (capture_frames_index == 0)
    {
        StartCapture();
    }

    if (capture_mode == CAPTURE_NORMAL)
    {
        DoCapture(socket);
    }
    else
    {
        DoMultiFrameCapture(socket);
    }

    if (capture_frames_index == capture_frames)
    {
        StopCapture();
        //
        int fd = open(cap_info.dev_name, O_RDWR, 0);
        LOG_INFO("fd: %d\n", fd);
        if (fd < 0)
        {
            LOG_ERROR("Open dev %s failed.\n", cap_info.dev_name);
        }
        else
        {
            if (g_sensorHdrMode == NO_HDR)
            {
                int ret = ioctl(fd, RKCIF_CMD_SET_CSI_MEMORY_MODE, &g_sensorMemoryMode); // set to original value
                if (ret > 0)
                {
                    LOG_ERROR("set cif node %s compact mode failed.\n", cap_info.dev_name);
                }
                else
                {
                    LOG_INFO("cif node %s set to mode %d.\n", cap_info.dev_name, g_sensorMemoryMode);
                }
            }
        }
        close(fd);

        // recover sync mode for dual camera
        int sensorfd = open(cap_info.sd_path.device_name, O_RDWR, 0);
        int ret = ioctl(sensorfd, RKMODULE_SET_SYNC_MODE, &g_sensorSyncMode); // set to no sync
        if (ret > 0)
        {
            LOG_ERROR("set cif node %s sync mode failed.\n", cap_info.sd_path.device_name);
        }
        else
        {
            LOG_INFO("cif node %s set to no sync mode.\n", cap_info.sd_path.device_name);
        }
        close(sensorfd);
    }
    LOG_DEBUG("exit\n");
}

static void SendRawDataResult(CommandData_t* cmd, CommandData_t* recv_cmd)
{
    unsigned short* checksum;
    checksum = (unsigned short*)&recv_cmd->dat[1];
    strncpy((char*)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_RAW_CAPTURE;
    cmd->datLen = 2;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = 0x04;
    LOG_DEBUG("capture_check_sum %d, recieve %d\n", capture_check_sum, *checksum);
    if (capture_check_sum == *checksum)
    {
        cmd->dat[1] = RES_SUCCESS;
    }
    else
    {
        cmd->dat[1] = RES_FAILED;
        StopCapture();
    }
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    LOG_DEBUG("enter\n");

    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_DEBUG("exit\n");
}

static void DoAnswer2(int sockfd, CommandData_t* cmd, int cmd_id, uint16_t check_sum, uint32_t result)
{
    char send_data[MAXPACKETSIZE];
    LOG_DEBUG("enter\n");
    strncpy((char*)cmd->RKID, RKID_ISP_ON, sizeof(cmd->RKID));
    cmd->cmdType = CMD_TYPE_CAPTURE;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = result;
    cmd->dat[1] = check_sum & 0xFF;
    cmd->dat[2] = (check_sum >> 8) & 0xFF;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++)
    {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_DEBUG("exit\n");
}

static void OnLineSet(int sockfd, CommandData_t* cmd, uint16_t& check_sum, uint32_t& result)
{
    int recv_size = 0;
    int param_size = *(int*)cmd->dat;
    int remain_size = param_size;

    LOG_DEBUG("enter\n");
    LOG_DEBUG("expect recv param_size 0x%x\n", param_size);
    char* param = (char*)malloc(param_size);
    while (remain_size > 0)
    {
        int offset = param_size - remain_size;
        recv_size = recv(sockfd, param + offset, remain_size, 0);
        remain_size = remain_size - recv_size;
    }

    LOG_DEBUG("recv ready\n");

    for (int i = 0; i < param_size; i++)
    {
        check_sum += param[i];
    }

    LOG_DEBUG("DO Sycn Setting, CmdId: 0x%x, expect ParamSize %d\n", cmd->cmdID, param_size);
#if 0
  if (rkaiq_manager) {
    result = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);
  }
#endif
    if (param != NULL)
    {
        free(param);
    }
    LOG_DEBUG("exit\n");
}

void RKAiqRawProtocol::HandlerRawCapMessage(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret_val, ret;

    LOG_DEBUG("HandlerRawCapMessage:\n");

    // for (int i = 0; i < common_cmd->datLen; i++) {
    //   LOG_DEBUG("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
    // }

    // if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    //   LOG_DEBUG("RKID: %s\n", common_cmd->RKID);
    // } else {
    //   LOG_DEBUG("RKID: Unknow\n");
    //   return;
    // }

    if (common_cmd->cmdType == CMD_TYPE_CAPTURE)
    {
        RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_CAPTURE);
        LOG_DEBUG("cmdType: CMD_TYPE_CAPTURE\n");
    }
    else if (common_cmd->cmdType == CMD_TYPE_STREAMING)
    {
        RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_STREAMING);
        InitCommandStreamingAns(&send_cmd, RES_SUCCESS);
        send(sockfd, &send_cmd, sizeof(CommandData_t), 0);
        LOG_DEBUG("cmdType: CMD_TYPE_STREAMING\n");
        if (common_cmd->cmdID == 0xffff)
        {
            uint16_t check_sum;
            uint32_t result;
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
            OnLineSet(sockfd, common_cmd, check_sum, result);
            DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum, result ? RES_FAILED : RES_SUCCESS);
            return;
        }
    }
    else
    {
        LOG_DEBUG("cmdType: Unknow %x\n", common_cmd->cmdType);
        return;
    }

    LOG_DEBUG("cmdID: %x\n", common_cmd->cmdID);

    switch (common_cmd->cmdID)
    {
        case CMD_ID_CAPTURE_STATUS:
            LOG_DEBUG("CmdID CMD_ID_CAPTURE_STATUS in\n");
            if (common_cmd->dat[0] == KNOCK_KNOCK)
            {
                InitCommandPingAns(&send_cmd, READY);
                LOG_DEBUG("Device is READY\n");
            }
            else
            {
                // SendMessageToPC(sockfd, "Unknow CMD_ID_CAPTURE_STATUS message");
                LOG_ERROR("Unknow CMD_ID_CAPTURE_STATUS message\n");
            }
            memcpy(send_data, &send_cmd, sizeof(CommandData_t));
            send(sockfd, send_data, sizeof(CommandData_t), 0);
            LOG_DEBUG("cmdID CMD_ID_CAPTURE_STATUS out\n\n");
            break;
        case CMD_ID_CAPTURE_RAW_CAPTURE: {
            LOG_DEBUG("CmdID RAW_CAPTURE in\n");
            char* datBuf = (char*)(common_cmd->dat);

            switch (datBuf[0])
            {
                case DATA_ID_CAPTURE_RAW_STATUS:
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_STATUS in\n");
                    if (common_cmd->dat[1] == KNOCK_KNOCK)
                    {
                        if (capture_status == RAW_CAP)
                        {
                            LOG_DEBUG("capture_status BUSY\n");
                            InitCommandRawCapAns(&send_cmd, BUSY);
                        }
                        else
                        {
                            LOG_DEBUG("capture_status READY\n");
                            InitCommandRawCapAns(&send_cmd, READY);
                        }
                    }
                    else
                    {
                        // SendMessageToPC(sockfd, "Unknow DATA_ID_CAPTURE_RAW_STATUS message");
                        LOG_ERROR("Unknow DATA_ID_CAPTURE_RAW_STATUS message\n");
                    }
                    memcpy(send_data, &send_cmd, sizeof(CommandData_t));
                    send(sockfd, send_data, sizeof(CommandData_t), 0);
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_STATUS out\n");
                    break;
                case DATA_ID_CAPTURE_RAW_GET_PARAM:
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_GET_PARAM in\n");
                    RawCaptureinit(common_cmd);
                    GetSensorPara(&send_cmd, RES_SUCCESS);
                    LOG_DEBUG("send_cmd.checkSum %d\n", send_cmd.checkSum);
                    memcpy(send_data, &send_cmd, sizeof(CommandData_t));
                    ret_val = send(sockfd, send_data, sizeof(CommandData_t), 0);
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_GET_PARAM out\n");
                    break;
                case DATA_ID_CAPTURE_RAW_SET_PARAM:
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_SET_PARAM in\n");
                    SetCapConf(common_cmd, &send_cmd, READY);
                    memcpy(send_data, &send_cmd, sizeof(CommandData_t));
                    send(sockfd, send_data, sizeof(CommandData_t), 0);
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_SET_PARAM out\n");
                    break;
                case DATA_ID_CAPTURE_RAW_START: {
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_START in\n");
                    capture_status = RAW_CAP;
                    RawCaputure(&send_cmd, sockfd);
                    capture_status = AVALIABLE;
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_START out\n");
                    break;
                }
                case DATA_ID_CAPTURE_RAW_CHECKSUM:
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_CHECKSUM in\n");
                    SendRawDataResult(&send_cmd, common_cmd);
                    memcpy(send_data, &send_cmd, sizeof(CommandData_t));
                    ret_val = send(sockfd, send_data, sizeof(CommandData_t), 0);
                    LOG_DEBUG("ProcID DATA_ID_CAPTURE_RAW_CHECKSUM out\n");
                    break;
                default:
                    break;
            }
            LOG_DEBUG("CmdID RAW_CAPTURE out\n\n");
            break;
        }
        default:
            break;
    }
}

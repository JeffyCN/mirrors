#ifndef _RKAIQ_COMMON_H__
#define _RKAIQ_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <linux/v4l2-controls.h>

#include "camera_capture.h"
#include "camera_infohw.h"
#include "logger/log.h"
#include "rkaiq_cmdid.h"
#if 0
    #include "rkaiq_manager.h"
#endif
#ifdef ENABLE_RSTP_SERVER
    #include "rtsp_server.h"
#endif
#include "tcp_server.h"

typedef enum
{
    KNOCK_KNOCK = 0x80,
    READY,
    BUSY,
    RES_FAILED = 0x00,
    RES_SUCCESS
} cmdStatus;

typedef enum
{
    PACKET_TYPE_SET = 0x00,
    PACKET_TYPE_GET = 0x01,
    PACKET_TYPE_STATUS = 0x80
} packeType;

typedef enum
{
    PC_TO_DEVICE = 0x00,
    DEVICE_TO_PC
} cmdType;

#define RKAIQ_TOOL_VERSION "v0.0.1"

#define STOP_RKLUNCH_CMD "sh /oem/RkLunch-stop.sh"

#if 0
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif

#define RKID_ISP_ON "ISP_ON"
#define RKID_ISP_OFF "ISP_OFF"
#define RKID_CHECK "IGNORE"
#define RKID_SEND_FILE "SendFile"
#define RKID_OFFLINE_RAW "OffRAW"
#define RKID_GET_AWB_PARA_FILE "GetAWBp"

#pragma pack(1)
typedef struct CommandData_s
{
    uint8_t RKID[8];
    uint16_t cmdType;
    uint16_t cmdID;
    uint8_t version[8];
    uint16_t datLen;
    uint8_t dat[48];
    uint16_t checkSum;
} CommandData_t;
#pragma pack()

typedef enum
{
    CMD_TYPE_UAPI_SET = 0x00,
    CMD_TYPE_UAPI_GET = 0x01,
    CMD_TYPE_CAPTURE = 0x02,
    CMD_TYPE_DUMP_RAW = 0x03,
    CMD_TYPE_STATUS = 0x80,
    CMD_TYPE_STREAMING = 0xff,
} FuncType_e;

typedef enum
{
    CMD_ID_CAPTURE_STATUS = 0x0001,
    CMD_ID_CAPTURE_RAW_CAPTURE = 0x0002,
    CMD_ID_CAPTURE_YUV_CAPTURE = 0x0006,
    CMD_ID_CAPTURE_ONLINE_RAW_CAPTURE = 0x0007,
} RkispCmdCaptureID_e;

typedef enum
{
    CMD_ID_GET_STATUS = 0x0100,
    CMD_ID_GET_MODE = 0x0101,
    CMD_ID_START_RTSP = 0x0102,
    CMD_ID_STOP_RTSP = 0x0103,
} RkispCmdStatusID_e;

typedef enum
{
    DATA_ID_CAPTURE_RAW_STATUS = 0x00,
    DATA_ID_CAPTURE_RAW_GET_PARAM = 0x01,
    DATA_ID_CAPTURE_RAW_SET_PARAM = 0x02,
    DATA_ID_CAPTURE_RAW_START = 0x03,
    DATA_ID_CAPTURE_RAW_CHECKSUM = 0x04,
} RkispCmdRawCaptureProcID_e;

typedef enum
{
    DATA_ID_CAPTURE_YUV_STATUS = 0x00,
    DATA_ID_CAPTURE_YUV_GET_PARAM = 0x01,
    DATA_ID_CAPTURE_YUV_SET_PARAM = 0x02,
    DATA_ID_CAPTURE_YUV_START = 0x03,
    DATA_ID_CAPTURE_YUV_CHECKSUM = 0x04
} RkispCmdYuvCaptureProcID_e;

typedef enum
{
    DATA_ID_CAPTURE_ONLINE_RAW_STATUS = 0x10,
    DATA_ID_CAPTURE_ONLINE_RAW_GET_PARAM,
    DATA_ID_CAPTURE_ONLINE_RAW_SET_PARAM,
    DATA_ID_CAPTURE_ONLINE_RAW_START,
    DATA_ID_CAPTURE_ONLINE_RAW_CHECKSUM,
} RkispCmdOnlineRawCaptureProcID_e;

typedef enum
{
    RKISP_FORMAT_NV12 = 0x0,
    RKISP_FORMAT_NV16,
    RKISP_FORMAT_YUYV,
    RKISP_FORMAT_FBC0,
} RkispFmt_e;

#define RKCIF_CMD_GET_CSI_MEMORY_MODE _IOR('V', BASE_VIDIOC_PRIVATE + 0, int)
#define RKCIF_CMD_SET_CSI_MEMORY_MODE _IOW('V', BASE_VIDIOC_PRIVATE + 1, int)
#define RKCIF_CMD_GET_SCALE_BLC _IOR('V', BASE_VIDIOC_PRIVATE + 2, struct bayer_blc)
#define RKCIF_CMD_SET_SCALE_BLC _IOW('V', BASE_VIDIOC_PRIVATE + 3, struct bayer_blc)

/* cif memory mode
 * 0: raw12/raw10/raw8 8bit memory compact
 * 1: raw12/raw10 16bit memory one pixel
 *    low align for rv1126/rv1109/rk356x
 *    |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 *    | -| -| -| -|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 * 2: raw12/raw10 16bit memory one pixel
 *    high align for rv1126/rv1109/rk356x
 *    |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 *    |11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| -| -| -| -|
 *
 * note: rv1109/rv1126/rk356x dvp only support uncompact mode,
 *       and can be set low align or high align
 */

enum cif_csi_lvds_memory
{
    CSI_LVDS_MEM_COMPACT = 0,
    CSI_LVDS_MEM_WORD_LOW_ALIGN = 1,
    CSI_LVDS_MEM_WORD_HIGH_ALIGN = 2,
};

/* black level for scale image
 * The sequence of pattern00~03 is the same as the output of sensor bayer
 */

struct bayer_blc
{
    uint8_t pattern00;
    uint8_t pattern01;
    uint8_t pattern02;
    uint8_t pattern03;
};

#define BASE_VIDIOC_PRIVATE 192 /* 192-255 are private */
#define RKMODULE_GET_SYNC_MODE _IOR('V', BASE_VIDIOC_PRIVATE + 21, __u32)
#define RKMODULE_SET_SYNC_MODE _IOW('V', BASE_VIDIOC_PRIVATE + 22, __u32)

#endif

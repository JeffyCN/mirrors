#ifndef _RKAIQ_RAW_PROTOCOL_H__
#define _RKAIQ_RAW_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_common.h"

typedef enum
{
    CAPTURE_NORMAL = 0,
    CAPTUER_AVERAGE,
} captureMode;

typedef enum
{
    VIDEO_APP_OFF = 0x80,
    VIDEO_APP_ON
} videoAppStatus;

typedef enum
{
    APP_RUN_STATUS_INIT = -1,
    APP_RUN_STATUS_TUNRING = 0,
    APP_RUN_STATUS_CAPTURE = 1,
    APP_RUN_STATUS_STREAMING = 2,
} appRunStatus;

typedef enum
{
    RAW_CAP = 0x00,
    AVALIABLE
} runStaus;

#define RAW_FMT_NON_COMPACT (0x0000 << 7)
#define RAW_FMT_COMPACT (0x0001 << 7)
#define RAW_FMT_COMPACT_CHECK (0x0001 << 7)

#define RAW_FMT_LINEAR (0x0000 << 4)
#define RAW_FMT_HDR2 (0x0001 << 4)
#define RAW_FMT_HDR3 (0x0002 << 4)
#define RAW_FMT_MODE_CHECK (0x0003 << 4)

#define RAW_FMT_ALIGN0 (0x0000 << 0)
#define RAW_FMT_ALIGN64 (0x0001 << 0)
#define RAW_FMT_ALIGN128 (0x0002 << 0)
#define RAW_FMT_ALIGN256 (0x0003 << 0)
#define RAW_FMT_ALIGN512 (0x0004 << 0)
#define RAW_FMT_ALIGN_CHECK (0x000F << 0)

#define IS_RAW_FMT_NON_COMPACT(x) (((x) & (RAW_FMT_COMPACT_CHECK)) == (RAW_FMT_NON_COMPACT))
#define IS_RAW_FMT_COMPACT(x) (((x) & (RAW_FMT_COMPACT_CHECK)) == (RAW_FMT_COMPACT))

#define IS_RAW_FMT_LINEAR(x) (((x) & (RAW_FMT_MODE_CHECK)) == (RAW_FMT_LINEAR))
#define IS_RAW_FMT_HDR2(x) (((x) & (RAW_FMT_MODE_CHECK)) == (RAW_FMT_HDR2))
#define IS_RAW_FMT_HDR3(x) (((x) & (RAW_FMT_MODE_CHECK)) == (RAW_FMT_HDR3))

#define IS_RAW_FMT_ALIGN0(x) (((x) & (RAW_FMT_ALIGN_CHECK)) == (RAW_FMT_ALIGN0))
#define IS_RAW_FMT_ALIGN64(x) (((x) & (RAW_FMT_ALIGN_CHECK)) == (RAW_FMT_ALIGN64))
#define IS_RAW_FMT_ALIGN128(x) (((x) & (RAW_FMT_ALIGN_CHECK)) == (RAW_FMT_ALIGN128))
#define IS_RAW_FMT_ALIGN256(x) (((x) & (RAW_FMT_ALIGN_CHECK)) == (RAW_FMT_ALIGN256))
#define IS_RAW_FMT_ALIGN512(x) (((x) & (RAW_FMT_ALIGN_CHECK)) == (RAW_FMT_ALIGN512))

typedef enum
{
    PROC_ID_CAPTURE_RAW_FMT_STATUS = 0x00,
    // NON-Compact
    PROC_ID_CAPTURE_RAW_NON_COMPACT_LINEAR = (RAW_FMT_NON_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN0),
    PROC_ID_CAPTURE_RAW_NON_COMPACT_HDR2 = (RAW_FMT_NON_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN0),
    PROC_ID_CAPTURE_RAW_NON_COMPACT_HDR3 = (RAW_FMT_NON_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN0),
    // Compact and non-align
    PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN0 = (RAW_FMT_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN0),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN0 = (RAW_FMT_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN0),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN0 = (RAW_FMT_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN0),
    // Compact and align with 64
    PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN64 = (RAW_FMT_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN64),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN64 = (RAW_FMT_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN64),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN64 = (RAW_FMT_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN64),
    // Compact and align with 128
    PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN128 = (RAW_FMT_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN128),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN128 = (RAW_FMT_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN128),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN128 = (RAW_FMT_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN128),
    // Compact and align with 256
    PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN256 = (RAW_FMT_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN256),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN256 = (RAW_FMT_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN256),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN256 = (RAW_FMT_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN256),
    // Compact and align with 512
    PROC_ID_CAPTURE_RAW_COMPACT_LINEAR_ALIGN512 = (RAW_FMT_COMPACT | RAW_FMT_LINEAR | RAW_FMT_ALIGN512),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR2_ALIGN512 = (RAW_FMT_COMPACT | RAW_FMT_HDR2 | RAW_FMT_ALIGN512),
    PROC_ID_CAPTURE_RAW_COMPACT_HDR3_ALIGN512 = (RAW_FMT_COMPACT | RAW_FMT_HDR3 | RAW_FMT_ALIGN512),
} RkispCmdRawFmtID_e;

#pragma pack(1)
typedef struct Sensor_Params_s
{
    uint8_t status;
    uint32_t fps;
    uint32_t hts;
    uint32_t vts;
    uint32_t bits;
    uint8_t endianness;
    uint16_t sensorImageFormat;
} Sensor_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Params_s
{
    uint32_t gain;
    uint32_t time;
    uint8_t lhcg;
    uint8_t bits;
    uint16_t framenumber;
    uint8_t multiframe;
    uint32_t vblank;
    uint32_t focus_position;
} Capture_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Reso_s
{
    uint16_t width;
    uint16_t height;
} Capture_Reso_t;
#pragma pack()

#define VIDEO_RAW0 "/dev/video0"
#define SAVE_RAW0_PATH "/data/output.raw"

#define TAG_PC_TO_DEVICE RKID_ISP_OFF
#define TAG_DEVICE_TO_PC RKID_ISP_OFF

int StopProcess(const char* process, const char* str);
int WaitProcessExit(const char* process, int sec);

class RKAiqRawProtocol
{
  public:
    RKAiqRawProtocol() = default;
    virtual ~RKAiqRawProtocol() = default;
    static void HandlerRawCapMessage(int sockfd, char* buffer, int size);
};

#endif

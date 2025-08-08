#ifndef _RKAIQ_ONLINE_PROTOCOL_H__
#define _RKAIQ_ONLINE_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_common.h"

#define TAG_OL_PC_TO_DEVICE RKID_ISP_ON
#define TAG_OL_DEVICE_TO_PC RKID_ISP_ON

#pragma pack(1)
typedef struct Capture_Yuv_Params_s
{
    uint32_t gain;
    uint32_t time;
    uint8_t fmt;
    uint16_t framenumber;
} Capture_Yuv_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Sensor_Yuv_Params_s
{
    uint8_t data_id;
    uint16_t width;
    uint16_t height;
    uint8_t format;
} Sensor_Yuv_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Sensor_Online_Raw_Params_s
{
    uint8_t data_id;
    uint16_t width;
    uint16_t height;
    uint32_t bits;
    uint16_t sensorImageFormat;
} Sensor_Online_Raw_Params_t;
#pragma pack()

class RKAiqOLProtocol
{
  public:
    RKAiqOLProtocol() = default;
    virtual ~RKAiqOLProtocol() = default;
    static void HandlerOnLineMessage(int sockfd, char* buffer, int size);
};

#endif

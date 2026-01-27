#ifndef _CAMERA_CAPTURE_H_
#define _CAMERA_CAPTURE_H_

#include "camera_device.h"
#include "camera_memory.h"
#include "logger/log.h"

typedef void (*CaptureCallBack)(int, int, void*, int);

int read_frame(struct capture_info* cap_info);
int read_frame(int handler, int index, struct capture_info* cap_info, CaptureCallBack callback);
void stop_capturing(struct capture_info* cap_info);
void start_capturing(struct capture_info* cap_info);
void uninit_device(struct capture_info* cap_info);
int init_device(struct capture_info* cap_info);

#endif
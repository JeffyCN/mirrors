#ifndef _CAMERA_INFOHW_H_
#define _CAMERA_INFOHW_H_

#include "camera_device.h"
#include "camera_memory.h"
#include "mediactl-priv.h"
#include "mediactl.h"

int initCamHwInfos(struct capture_info* media_info);
int setupLink(struct capture_info* media_info, bool raw_mode);
__u32 convert_to_v4l2fmt(struct capture_info* media_info, int code);
int rkisp_set_ispsd_fmt(struct capture_info* media_info, int in_w, int in_h, int in_code, int out_w, int out_h, int out_code);

#endif

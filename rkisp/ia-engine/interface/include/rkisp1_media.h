#ifndef __RKISP_HELPER_H__
#define __RKISP_HELPER_H__

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool
rk_common_v4l2device_find_by_name (const char *name, char *ret_name);
#if 0
int
rkisp1_init_media(int width, int height);

int
rkisp1_set_media(int width, int height,
		struct media_entity *sensor_subdev,
		struct media_entity *phy_subdev,
		struct media_entity *isp_subdev);

int
rk_common_media_find_by_vnode (const char *vnode);

struct media_entity *
rk_common_media_find_subdev_by_name (int index, const char *subdev_name);

struct media_entity *
rk_common_media_get_last_enity (int index);
#endif
#endif

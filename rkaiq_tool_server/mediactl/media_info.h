#ifndef __MEDIA_INFO_H__
#define __MEDIA_INFO_H__

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "mediactl.h"
#include "tools.h"
#include "v4l2subdev.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct flag_name
    {
        __u32 flag;
        char* name;
    };

    void print_flags(const struct flag_name* flag_names, unsigned int num_entries, __u32 flags);
    void v4l2_subdev_print_format(struct media_entity* entity, unsigned int pad, enum v4l2_subdev_format_whence which);
    const char* v4l2_dv_type_to_string(unsigned int type);
    void v4l2_subdev_print_dv_timings(const struct v4l2_dv_timings* timings, const char* name);
    void v4l2_subdev_print_pad_dv(struct media_entity* entity, unsigned int pad, enum v4l2_subdev_format_whence which);
    void v4l2_subdev_print_subdev_dv(struct media_entity* entity);
    const char* media_entity_type_to_string(unsigned type);
    const char* media_entity_subtype_to_string(unsigned type);
    const char* media_pad_type_to_string(unsigned flag);
    void media_print_pad_text(struct media_entity* entity, const struct media_pad* pad);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

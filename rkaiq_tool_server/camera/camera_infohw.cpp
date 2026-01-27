#include "camera_infohw.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

__u32 convert_to_v4l2fmt(struct capture_info* media_info, int code)
{
    __u32 v4l2_fmt = 0;
    __u32 bits = 0;
    switch (code)
    {
        /* 8bit raw bayer */
        case MEDIA_BUS_FMT_SBGGR8_1X8:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB8;
            media_info->sd_path.bits = 8;
            break;
        case MEDIA_BUS_FMT_SGBRG8_1X8:
            v4l2_fmt = V4L2_PIX_FMT_SGBRG8;
            media_info->sd_path.bits = 8;
            break;
        case MEDIA_BUS_FMT_SGRBG8_1X8:
            v4l2_fmt = V4L2_PIX_FMT_SGRBG8;
            media_info->sd_path.bits = 8;
            break;
        case MEDIA_BUS_FMT_SRGGB8_1X8:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB8;
            media_info->sd_path.bits = 8;
            break;
        case MEDIA_BUS_FMT_Y8_1X8:
            v4l2_fmt = V4L2_PIX_FMT_GREY;
            media_info->sd_path.bits = 8;
            break;
        /* 10bit raw bayer */
        case MEDIA_BUS_FMT_SBGGR10_1X10:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB10;
            media_info->sd_path.bits = 10;
            break;
        case MEDIA_BUS_FMT_SGBRG10_1X10:
            v4l2_fmt = V4L2_PIX_FMT_SGBRG10;
            media_info->sd_path.bits = 10;
            break;
        case MEDIA_BUS_FMT_SGRBG10_1X10:
            v4l2_fmt = V4L2_PIX_FMT_SGRBG10;
            media_info->sd_path.bits = 10;
            break;
        case MEDIA_BUS_FMT_SRGGB10_1X10:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB10;
            media_info->sd_path.bits = 10;
            break;
        case MEDIA_BUS_FMT_Y10_1X10:
            v4l2_fmt = V4L2_PIX_FMT_Y10;
            media_info->sd_path.bits = 10;
            break;
        /* 12bit raw bayer */
        case MEDIA_BUS_FMT_SBGGR12_1X12:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB12;
            media_info->sd_path.bits = 12;
            break;
        case MEDIA_BUS_FMT_SGBRG12_1X12:
            v4l2_fmt = V4L2_PIX_FMT_SGBRG12;
            media_info->sd_path.bits = 12;
            break;
        case MEDIA_BUS_FMT_SGRBG12_1X12:
            v4l2_fmt = V4L2_PIX_FMT_SGRBG12;
            media_info->sd_path.bits = 12;
            break;
        case MEDIA_BUS_FMT_SRGGB12_1X12:
            v4l2_fmt = V4L2_PIX_FMT_SRGGB12;
            media_info->sd_path.bits = 12;
            break;
        case MEDIA_BUS_FMT_Y12_1X12:
            v4l2_fmt = V4L2_PIX_FMT_Y12;
            media_info->sd_path.bits = 12;
            break;
        default:
            LOG_ERROR("nonsupport raw bayer formats, please check sensor output fmt\n");
            break;
    }
    return v4l2_fmt;
}

int get_isp_subdevs(struct media_device* device, const char* devpath, struct capture_info* media_info)
{
    media_entity* entity = NULL;
    const char* entity_name = NULL;
    uint32_t nents, j = 0;
    const struct media_entity_desc* entity_info = NULL;

    if (!device || !media_info || !devpath)
    {
        return -1;
    }

    strncpy(media_info->vd_path.media_dev_path, (char*)devpath, sizeof(media_info->vd_path.media_dev_path));
    LOG_DEBUG("get isp subdev: %s \n", media_info->vd_path.media_dev_path);
    entity = media_get_entity_by_name(device, "rkisp_mainpath");
    if (entity)
    {
        entity_name = media_entity_get_devname(entity);
        if (entity_name)
        {
            strncpy(media_info->vd_path.isp_main_path, (char*)entity_name, sizeof(media_info->vd_path.isp_main_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity)
    {
        entity_name = media_entity_get_devname(entity);
        if (entity_name)
        {
            strncpy(media_info->vd_path.isp_sd_path, (char*)entity_name, sizeof(media_info->vd_path.isp_sd_path));
            LOG_DEBUG("isp subdev path: %s\n", media_info->vd_path.isp_sd_path);
        }
    }

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    nents = media_get_entities_count(device);
    for (j = 0; j < nents; ++j)
    {
        entity = media_get_entity(device, j);
        entity_info = media_entity_get_info(entity);
        if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR))
        {
            strncpy(media_info->sd_path.device_name, (char*)media_entity_get_devname(entity), sizeof(media_info->sd_path.device_name));
            media_info->link = link_to_isp;
            LOG_DEBUG("get isp subdev: sensor link to %d \n", media_info->link);
            strncpy(media_info->sd_path.sensor_name, entity_info->name, sizeof(media_info->sd_path.sensor_name));
            LOG_DEBUG("sensor subdev path: %s\n", media_info->sd_path.device_name);
        }
    }

    return 0;
}

int get_vicap_subdevs(struct media_device* device, const char* devpath, struct capture_info* media_info)
{
    media_entity* entity = NULL;
    const char* entity_name = NULL;
    uint32_t nents, j = 0;
    const struct media_entity_desc* entity_info = NULL;

    if (!device || !media_info || !devpath)
    {
        return -1;
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id0");
    if (entity)
    {
        entity_name = media_entity_get_devname(entity);
        if (entity_name)
        {
            strncpy(media_info->cif_path.cif_video_path, (char*)entity_name, sizeof(media_info->cif_path.cif_video_path));
            LOG_DEBUG("get vicap subdev: %s \n", media_info->cif_path.cif_video_path);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_tools_id0");
    if (entity)
    {
        entity_name = media_entity_get_devname(entity);
        if (entity_name)
        {
            strncpy(media_info->cif_path.cif_video_path, (char*)entity_name, sizeof(media_info->cif_path.cif_video_path));
            LOG_DEBUG("get vicap subdev: %s \n", media_info->cif_path.cif_video_path);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_lite_mipi_lvds");
    if (entity)
    {
        entity_name = media_entity_get_devname(entity);
        if (entity_name)
        {
            strncpy(media_info->cif_path.cif_video_path, (char*)entity_name, sizeof(media_info->cif_path.cif_video_path));
            LOG_DEBUG("get vicap subdev: %s \n", media_info->cif_path.cif_video_path);
        }
    }

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    nents = media_get_entities_count(device);
    for (j = 0; j < nents; ++j)
    {
        entity = media_get_entity(device, j);
        entity_info = media_entity_get_info(entity);
        if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR))
        {
            strncpy(media_info->sd_path.device_name, (char*)media_entity_get_devname(entity), sizeof(media_info->sd_path.device_name));
            media_info->link = link_to_vicap;
            LOG_DEBUG("get vicap subdev: sensor link to %d \n", media_info->link);
            strncpy(media_info->sd_path.sensor_name, entity_info->name, sizeof(media_info->sd_path.sensor_name));
            LOG_DEBUG("sensor subdev path: %s\n", media_info->sd_path.device_name);
        }
    }

    return 0;
}

static int rkisp_sd_set_crop(const char* ispsd, int fd, int pad, int* w, int* h)
{
    struct v4l2_subdev_selection sel;
    int ret;

    memset(&sel, 0, sizeof(sel));
    sel.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    sel.pad = pad;
    sel.r.width = *w;
    sel.r.height = *h;
    sel.r.left = 0;
    sel.r.top = 0;
    sel.target = V4L2_SEL_TGT_CROP;
    sel.flags = V4L2_SEL_FLAG_LE;
    ret = device_setsubdevcrop(fd, &sel);
    if (ret)
    {
        LOG_ERROR("subdev %s pad %d crop failed, ret = %d\n", ispsd, sel.pad, ret);
        return ret;
    }

    *w = sel.r.width;
    *h = sel.r.height;

    return 0;
}

int rkisp_sd_set_fmt(const char* ispsd, int pad, int* w, int* h, int code)
{
    struct v4l2_subdev_format fmt;
    int ret, fd;

    fd = device_open(ispsd);
    if (fd < 0)
    {
        LOG_ERROR("Open isp subdev %s failed, %s\n", ispsd, strerror(errno));
        return fd;
    }

    if (pad == 2)
    { /* Source pad */
        ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
        if (ret)
        {
            goto close;
        }
    }

    /* Get fmt and only update what we want */
    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = pad;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret = device_getsubdevformat(fd, &fmt);
    if (ret < 0)
    {
        LOG_ERROR("subdev %s get pad: %d fmt failed %s.\n", ispsd, fmt.pad, strerror(errno));
        goto close;
    }

    fmt.format.height = *h;
    fmt.format.width = *w;
    fmt.format.code = code;

    ret = device_setsubdevformat(fd, &fmt);
    if (ret < 0)
    {
        LOG_ERROR("subdev %s set pad: %d fmt failed %s.\n", ispsd, fmt.pad, strerror(errno));
        goto close;
    }

    if (fmt.format.width != *w || fmt.format.height != *h || fmt.format.code != code)
    {
        LOG_INFO("subdev %s pad %d choose the best fit fmt: %dx%d, 0x%08x\n", ispsd, pad, fmt.format.width, fmt.format.height, fmt.format.code);
    }

    *w = fmt.format.width;
    *h = fmt.format.height;
    if (pad == 0)
    {
        ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
        if (ret)
        {
            goto close;
        }
    }

close:
    close(fd);

    return ret;
}

int rkisp_set_ispsd_fmt(struct capture_info* media_info, int in_w, int in_h, int in_code, int out_w, int out_h, int out_code)
{
    const char* ispsd;
    int ret;
    if (media_info->link == link_to_isp)
    {
        ispsd = media_info->vd_path.isp_sd_path;
        LOG_INFO("link_to_isp, isp subdev path:%s\n", media_info->vd_path.isp_sd_path);
    }
    else if (media_info->link == link_to_vicap)
    {
        ispsd = media_info->cif_path.cif_video_path;
        LOG_INFO("link_to_vicap, isp subdev path:%s\n", media_info->cif_path.cif_video_path);
    }
    // TODO: check source and sink pad
    ret = rkisp_sd_set_fmt(ispsd, 0, &in_w, &in_h, in_code);
    ret |= rkisp_sd_set_fmt(ispsd, 2, &out_w, &out_h, out_code);

    return ret;
}

int setupLink(struct capture_info* media_info, bool raw_mode)
{
    media_device* device = NULL;
    media_entity* entity = NULL;
    media_pad *src_pad = NULL, *sink_pad_bridge = NULL, *sink_pad_mp = NULL;
    media_pad *sink_pad = NULL, *src_raw2_s = NULL;
    int ret;

    device = media_device_new(media_info->vd_path.media_dev_path);

    LOG_INFO("%s: setup link for raw or yuv: %d\n", media_info->vd_path.media_dev_path, raw_mode);

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity)
    {
        src_pad = (media_pad*)media_entity_get_pad(entity, 2);
        if (!src_pad)
        {
            LOG_DEBUG("get rkisp-isp-subdev source pad failed!\n");
            goto FAIL;
        }
    }

    entity = media_get_entity_by_name(device, "rkisp-bridge-ispp");
    if (entity)
    {
        sink_pad_bridge = (media_pad*)media_entity_get_pad(entity, 0);
        if (!sink_pad_bridge)
        {
            LOG_DEBUG("get rkisp-bridge-ispp sink pad failed!\n");
            goto FAIL;
        }
    }

    entity = media_get_entity_by_name(device, "rkisp_mainpath");
    if (entity)
    {
        sink_pad_mp = (media_pad*)media_entity_get_pad(entity, 0);
        if (!sink_pad_mp)
        {
            LOG_DEBUG("get rkisp_mainpath sink pad failed!\n");
            goto FAIL;
        }
    }

    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity)
    {
        sink_pad = (media_pad*)media_entity_get_pad(entity, 0);
        if (!sink_pad)
        {
            LOG_DEBUG("get rkisp-isp-subdev source pad failed!\n");
            goto FAIL;
        }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s");
    if (entity)
    {
        src_raw2_s = (media_pad*)media_entity_get_pad(entity, 0);
        if (!src_raw2_s)
        {
            LOG_DEBUG("get rkisp_rawrd2_s sink pad failed!\n");
            goto FAIL;
        }
    }

    if (raw_mode)
    {
        ret = media_setup_link(device, src_raw2_s, sink_pad, 0);
        if (ret)
        {
            LOG_ERROR("media_setup_link src_raw2_s sink_pad FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_pad, sink_pad_bridge, 0);
        if (ret)
        {
            LOG_ERROR("media_setup_link src_pad sink_pad_bridge FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_pad, sink_pad_mp, MEDIA_LNK_FL_ENABLED);
        if (ret)
        {
            LOG_ERROR("media_setup_link src_pad src_pad FAILED: %d\n", ret);
        }
    }
    else
    {
        ret = media_setup_link(device, src_pad, sink_pad_mp, 0);
        if (ret)
        {
            LOG_ERROR("media_setup_link src_pad  sink_pad_mp FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_pad, sink_pad_bridge, MEDIA_LNK_FL_ENABLED);
        if (ret)
        {
            LOG_ERROR("media_setup_link src_pad sink_pad_bridge FAILED: %d\n", ret);
        }
    }
    media_device_unref(device);
    return 0;
FAIL:
    media_device_unref(device);
    return -1;
}

int initCamHwInfos(struct capture_info* media_info)
{
    // TODO
    // (1) all sensor info
    // (2) all pipeline entity infos belonged to
    //     the sensor

    char sys_path[64], devpath[32];
    FILE* fp = NULL;
    struct media_device* device = NULL;
    uint32_t nents, j = 0, i = 0;
    const struct media_entity_desc* entity_info = NULL;
    struct media_entity* entity = NULL;
    int ret = 0;

    LOG_INFO("init start!!!!!!\n");
    while (i < MAX_MEDIA_INDEX)
    {
        snprintf(sys_path, 64, "/dev/media%d", i++);
        fp = fopen(sys_path, "r");
        if (!fp)
        {
            continue;
        }
        fclose(fp);
        device = media_device_new(sys_path);

        /* Enumerate entities, pads and links. */
        media_device_enumerate(device);

        if (strcmp(device->info.model, "rkisp0") == 0 || strcmp(device->info.model, "rkisp") == 0)
        {
            ret = get_isp_subdevs(device, sys_path, media_info);
            if (ret)
            {
                return ret;
            }
        }
        else if (strcmp(device->info.model, "rkcif") == 0 || strcmp(device->info.model, "rkcif_lite_mipi_lvds") == 0 || strcmp(device->info.model, "rkcif_mipi_lvds") == 0)
        {
            ret = get_vicap_subdevs(device, sys_path, media_info);
            if (ret)
            {
                return ret;
            }
        }
        else
        {
            goto media_unref;
        }
    media_unref:
        media_device_unref(device);
    }
    return 0;
}

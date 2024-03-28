#include <iostream>
#include <mediactl.h>
#include <unistd.h>
#include <mediactl-priv.h>
#include <string>
#include <cstring>
#include <map>

#include "v4l2subdev.h"

#include <tools.h>

#include "v4l2_device.h"

#define MAX_MEDIA_INDEX               16
#define DEV_PATH_LEN                  64
#define SENSOR_ATTACHED_FLASH_MAX_NUM 2
#define MAX_CAM_NUM                   8

#define MAX_ISP_LINKED_VICAP_CNT      4

#define ISP_TX_BUF_NUM 4
#define VIPCAP_TX_BUF_NUM 4

struct dev_pipeline{
    unsigned int entity_num = 0;
    struct media_entity *entities[7];
};

static std::map<int, std::string> map_pipeline;

static const char* fmt2str( unsigned fmt )
{
    static char retstr[5] = {0};
    memset( retstr, 0, 5 );
    retstr[0] = fmt & 0xFF;
    retstr[1] = (fmt >> 8) & 0x7F;
    retstr[2] = (fmt >> 16) & 0x7F;
    retstr[3] = (fmt >> 24) & 0x7F;
    return retstr;
}

static int get_video_fmt_info(const char *name){
    XCam::V4l2SubDevice vdev(name);
    struct v4l2_format fmt;
    struct v4l2_crop crop;
    struct v4l2_subdev_selection selection;
    memset(&crop, 0, sizeof(crop));
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = vdev.open();
    if (ret == XCAM_RETURN_NO_ERROR) {
        ret = vdev.get_format(fmt);
        if (ret == XCAM_RETURN_NO_ERROR) {
            printf("fmt:           %uX%u\npixelformat:   %s\n",fmt.fmt.pix.width,fmt.fmt.pix.height, xcam_fourcc_to_string(fmt.fmt.pix.pixelformat));
        } else if (ret == XCAM_RETURN_ERROR_IOCTL) {
            printf("fmt:  get fmt info errors\n");
        }
        ret = vdev.get_crop(crop);
        if (ret == XCAM_RETURN_NO_ERROR) {
            printf("crop:          (%u, %u)%uX%u\n", crop.c.left, crop.c.top, crop.c.width, crop.c.height);
        } else {
            printf("crop: get crop info errors\n");
        }
    }
    return 0;
}

static const char *media_entity_type_to_string(unsigned type)
{
    static const struct {
        __u32 type;
        const char *name;
    } types[] = {
        { MEDIA_ENT_T_DEVNODE, "Node" },
        { MEDIA_ENT_T_V4L2_SUBDEV, "V4L2 subdev" },
    };
    unsigned int i;
    type &= MEDIA_ENT_TYPE_MASK;
    for (i = 0; i < ARRAY_SIZE(types); i++) {
        if (types[i].type == type)
            return types[i].name;
    }
    return "Unknown";
}
static const char *media_entity_subtype_to_string(unsigned type)
{
    static const char *node_types[] = {
        "Unknown",
        "V4L",
        "FB",
        "ALSA",
        "DVB",
    };
    static const char *subdev_types[] = {
        "Unknown",
        "Sensor",
        "Flash",
        "Lens",
    };
    unsigned int subtype = type & MEDIA_ENT_SUBTYPE_MASK;
    switch (type & MEDIA_ENT_TYPE_MASK) {
    case MEDIA_ENT_T_DEVNODE:
        if (subtype >= ARRAY_SIZE(node_types))
            subtype = 0;
        return node_types[subtype];

    case MEDIA_ENT_T_V4L2_SUBDEV:
        if (subtype >= ARRAY_SIZE(subdev_types))
            subtype = 0;
        return subdev_types[subtype];
    default:
        return node_types[0];
    }
}

static const char *media_pad_type_to_string(unsigned flag)
{
static const struct {
    __u32 flag;
    const char *name;
    } flags[] = {
        { MEDIA_PAD_FL_SINK, "Sink" },
        { MEDIA_PAD_FL_SOURCE, "Source" },
    };
    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(flags); i++) {
        if (flags[i].flag & flag)
            return flags[i].name;
    }
    return "Unknown";
}

//look for vicap linked to isp
static const char *get_isp_dev_info_by_name(const char *name, struct dev_pipeline *pipeline)
{
    char sys_path[64];
    FILE *fp = NULL;
    struct media_device *device = NULL;
    bool sensor_flag = false;
    unsigned int j;
    const struct media_pad *pad = NULL;
    const struct media_link *link = NULL;
    const struct media_entity_desc *entity_info = NULL;
    media_entity *entity = NULL;
    media_entity *sensor_entity = NULL;
    //轮询media 查找所要的media
    for (int i = 0; i < MAX_MEDIA_INDEX; i++)
    {
        snprintf(sys_path, 64, "/dev/media%d", i);
        fp = fopen (sys_path, "r");
        if (!fp)
        {
            continue;
        }
        fclose(fp);
        device = media_device_new(sys_path);
        if (!device) {
            printf("device is no exit\n");
        }
        media_device_enumerate(device);
        if (0 == strcmp(device->info.driver, name)) {
            break;
        }
    }

    if (0 != strcmp(device->info.driver, name)) {
        return NULL;
    }

    entity =media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
        pipeline->entity_num = pipeline->entity_num + 1;
    }

    entity =media_get_entity_by_name(device, "rkisp_selfpath", strlen("rkisp_selfpath"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
        pipeline->entity_num = pipeline->entity_num + 1;
    }

    //look for pads of isp-subdev
    entity =media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
        pipeline->entity_num = pipeline->entity_num + 1;
    }

    for (j=0; j < device->entities_count; j++) {
        entity = &device->entities[j];
        entity_info = media_entity_get_info(entity);
        if(strncmp(media_entity_subtype_to_string(entity_info->type), "Sensor", strlen("Sensor")) == 0){
            sensor_flag = true;
            sensor_entity = entity;
            break;
        }
    }

    const char* linked_entity_name_strs [] = {
        "rkcif_dvp",
        "rkcif_lite_mipi_lvds",
        "rkcif-mipi-lvds",
        "rkcif-mipi-lvds1",
        "rkcif-mipi-lvds2",
        "rkcif-mipi-lvds3",
        "rkcif-mipi-lvds4",
        "rkcif-mipi-lvds5",
        NULL
    };

    for (int i = 0; linked_entity_name_strs[i] != NULL; i++) {
        entity = media_get_entity_by_name(device, linked_entity_name_strs[i],strlen(linked_entity_name_strs[i]));
        if (entity)
        {
            pipeline->entities[pipeline->entity_num] = entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num = pipeline->entity_num + 1;
            if(sensor_flag){
                break;
            }else{
                return entity->info.name;
            }
        }
    }

    if (sensor_flag) {
        entity =media_get_entity_by_name(device, "rkisp-csi-subdev", strlen("rkisp-csi-subdev"));
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num = pipeline->entity_num + 1;
        }

        entity =media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num = pipeline->entity_num + 1;
        } else {
            entity =media_get_entity_by_name(device, "rockchip-csi2-dphy1", strlen("rockchip-csi2-dphy1"));
            if (entity) {
                pipeline->entities[pipeline->entity_num] = entity;
                map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
                pipeline->entity_num = pipeline->entity_num + 1;
            }
        }

        pipeline->entities[pipeline->entity_num] = sensor_entity;
        map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
        pipeline->entity_num++;
    }

    return NULL;
}

//look for camera linkde to vicap
static int get_vicap_dev_info_by_name(const char *name, struct dev_pipeline *pipeline)
{
    char sys_path[64];
    FILE *fp = NULL;
    struct media_device *device = NULL;
    std::string pipeline_str = "";
    const struct media_pad *pad = NULL;
    const struct media_link *link = NULL;
    const struct media_entity_desc *entity_info = NULL;
    media_entity *entity = NULL;
    media_entity *camera_entity = NULL;

    media_entity *mipi_csi2_entity = NULL;
    const char *camera_name = NULL;
    const char *mipi_csi2_name = NULL;

    bool flag0 = false;
    bool flag1 = false;
    bool flag2 = false;
    //轮询media 查找所要的media
    for (int i = 0; i < MAX_MEDIA_INDEX; i++)
    {
        snprintf(sys_path, 64, "/dev/media%d", i);
        fp = fopen (sys_path, "r");
        if (!fp)
        {
            continue;
        }
        fclose(fp);
        device = media_device_new(sys_path);
        if (!device){
            printf("device is no exit\n");
        }
        media_device_enumerate(device);
        if (0 == strcmp(device->info.model, name)) {
            break;
        }
    }

    if (0 != strcmp(device->info.model, name)) {
        return 0;
    }

    if (0 == strcmp(name,"rkcif_dev")) {
        flag0 = true;
    } else if (0 == strcmp(name,"rkcif_lite_mipi_lvds")) {
        flag1 = true;
    } else {
        flag2 = true;
    }

    const char* dphy_name[] ={
        "rockchip-csi2-dphy0",
        "rockchip-csi2-dphy1",
        "rockchip-csi2-dphy2",
        "rockchip-csi2-dphy3",
        "rockchip-csi2-dphy4",
        "rockchip-csi2-dphy5",
        "rockchip-csi2-dphy6",
        "rockchip-csi2-dphy7",
        NULL
    };
    if (flag0) {
        return 0;
    }

    if (flag1)
    {
        return 0;
    }

    if (flag2) {
        for(int i=0; dphy_name[i] != NULL; i++){
            entity = media_get_entity_by_name(device,dphy_name[i],strlen(dphy_name[i]));
            if (entity) {
                unsigned int j;
                for (j=0; j < entity->num_links ; j++) {
                    link = media_entity_get_link(entity,j);
                    if(entity == link->sink->entity){
                        camera_entity = link->source->entity;
                    }
                }
                for(j=0;j< entity->num_links;j++) {
                    link = media_entity_get_link(entity,j);
                    if(entity == link->source->entity){
                        mipi_csi2_entity = link->sink->entity;
                    }
                }
                break;
            }
        }
        if (mipi_csi2_entity) {
            pipeline->entities[pipeline->entity_num] = mipi_csi2_entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num++;
        }
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num++;
        }
        if (camera_entity) {
            pipeline->entities[pipeline->entity_num] = camera_entity;
            map_pipeline.insert(std::pair<int, std::string>(int(pipeline->entity_num),std::string(sys_path)));
            pipeline->entity_num++;
        }
        return 0;
    }
    return -1;
}


static int pipelin_dev_info_print(struct dev_pipeline *pipeline)
{
    int i;
    struct v4l2_mbus_framefmt format;
    struct v4l2_rect rect;
    int ret;
    unsigned int j, k;

    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        if(i != int(pipeline->entity_num)-1){
            printf(" -> ");
        }
        printf("%-20s", info->name);
    }
    printf("\n\nEntity Name         Media_name     Devname             Pad  Pad_type  Fmt_code       Fmt       Crop                Crop.bounds         enable link                link entity devname\n");
    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        auto media_name_map = map_pipeline.find(i);
        std::string media_dev_name = media_name_map->second;
        printf("%-20s%-15s%-20s%-12s",entity->info.name, media_dev_name.c_str(), entity->devname,media_entity_type_to_string(info->type));
        unsigned int num_links = media_entity_get_links_count(entity);

        for (j = 0; j < info->pads; j++){
            const struct media_pad *pad = media_entity_get_pad(entity, j);
            const char *fmt_code = "";
            enum v4l2_subdev_format_whence which = V4L2_SUBDEV_FORMAT_ACTIVE;
            bool flag = false;

            if (media_entity_type(entity) == MEDIA_ENT_T_V4L2_SUBDEV){
                ret = v4l2_subdev_get_format(entity, &format, j, which);
                if(ret == 0){
                    fmt_code = v4l2_subdev_pixelcode_to_string(v4l2_mbus_pixelcode(format.code));
                }
            }
            if (j != 0) {
                printf("%-67s"," ");
            }
            printf("%-5u%-10s%-15s%-4uX%-5u",
                    j,
                    media_pad_type_to_string(pad->flags),
                    fmt_code, format.width, format.height);
            ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP, which);
            if (ret == 0) {
                printf("(%-3u,%-3u)/%4uX%-5u",rect.left, rect.top, rect.width, rect.height);
            } else {
                printf("%-20s","");
            }

            ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP_BOUNDS, which);
            if (ret == 0) {
                printf("(%-3u,%-3u)/%4uX%-5u",rect.left, rect.top, rect.width, rect.height);
            } else {
                printf("%-20s","");
            }
            for (k = 0; k < num_links; k++) {
                const struct media_link *link = media_entity_get_link(entity, k);
                const struct media_pad *source = link->source;
                const struct media_pad *sink = link->sink;
                if (link->flags & MEDIA_LNK_FL_ENABLED) {
                    if (source->entity == entity && source->index == j) {
                        if (flag) {
                            printf("%-147s", " ");
                        }
                        printf("-> %-20s:%-3u%s\n",
                            media_entity_get_info(sink->entity)->name,
                            sink->index, sink->entity->devname);
                        flag = true;
                    }
                    if (sink->entity == entity && sink->index == j) {
                        if(flag){
                            printf("%-132s", " ");
                        }
                        printf("<- %-20s:%-3u%s\n",
                            media_entity_get_info(source->entity)->name,
                            source->index, source->entity->devname);
                        flag = true;
                    }
                }
            }
            if (!flag) {
                printf("\n");
            }
        }
        printf("\n");
    }
    return 0;
}

static int pipelin_dev_info_print_two(struct dev_pipeline *pipeline)
{
    int i;
    struct v4l2_mbus_framefmt format;
    struct v4l2_rect rect;
    struct media_device *device = NULL;
    const char *sensor_name = NULL;
    FILE *fp = NULL;
    int ret;
    unsigned int j, k;
    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        if(strncmp(media_entity_subtype_to_string(info->type), "Sensor", strlen("Sensor")) == 0){
            sensor_name = info->name;
            break;
        }
    }
    if (sensor_name) {
        printf("\nSensor %s Detail Information", sensor_name);
        printf("\n=======================================================================================================================================\n");
        printf("Sensor %s Pipeline Link:\n\n", sensor_name);
    } else {
        printf("\nSensor NO FOUND Detail Information");
        printf("\n=======================================================================================================================================\n");
        printf("Sensor NO FOUND Pipeline Link:\n\n");
    }
    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        if(i != int(pipeline->entity_num)-1){
            printf(" -> ");
        }
        printf("%-20s", info->name);
        if (strcmp(info->name, "rkisp-isp-subdev") == 0)
            break;
    }
    printf("\n***************************************************************************************************************************************\n");
    printf("\nTable 1:\n");
    printf("Entity Name         Media_name     Devname             Pad  Pad_type  Fmt_code       Fmt       Crop                Crop.bounds         \n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        auto media_name_map = map_pipeline.find(i);
        std::string media_dev_name = media_name_map->second;
        printf("%-20s%-15s%-20s",entity->info.name, media_dev_name.c_str(), entity->devname);
        unsigned int num_links = media_entity_get_links_count(entity);

        for (j = 0; j < info->pads; j++){
            const struct media_pad *pad = media_entity_get_pad(entity, j);
            const char *fmt_code = "";
            enum v4l2_subdev_format_whence which = V4L2_SUBDEV_FORMAT_ACTIVE;
            if (j != 0) {
                    printf("%-55s"," ");
                }
                printf("%-5u%-10s",
                        j,
                        media_pad_type_to_string(pad->flags));
            if (media_entity_type(entity) == MEDIA_ENT_T_V4L2_SUBDEV){
                ret = v4l2_subdev_get_format(entity, &format, j, which);
                if(ret == 0){
                    fmt_code = v4l2_subdev_pixelcode_to_string(v4l2_mbus_pixelcode(format.code));
                }
                printf("%-15s%-4uX%-5u", fmt_code, format.width, format.height);
                ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP, which);
                if (ret == 0) {
                    printf("(%-3u,%-3u)/%4uX%-5u",rect.left, rect.top, rect.width, rect.height);
                } else {
                    printf("%-20s","");
                }
                ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP_BOUNDS, which);
                if (ret == 0) {
                    printf("(%-3u,%-3u)/%4uX%-5u",rect.left, rect.top, rect.width, rect.height);
                } else {
                    printf("%-20s","");
                }
            } else if (media_entity_type(entity) == MEDIA_ENT_T_DEVNODE) {
                XCam::V4l2SubDevice vdev(entity->devname);
                struct v4l2_format fmt;
                struct v4l2_crop crop;
                memset(&crop, 0, sizeof(crop));
                XCamReturn ret = XCAM_RETURN_NO_ERROR;
                ret = vdev.open();
                if (ret == XCAM_RETURN_NO_ERROR) {
                    ret = vdev.get_format(fmt);
                    if (ret == XCAM_RETURN_NO_ERROR) {
                        printf("%-15s%-4uX%-5u", xcam_fourcc_to_string(fmt.fmt.pix.pixelformat), fmt.fmt.pix.width, fmt.fmt.pix.height);
                    } else if (ret == XCAM_RETURN_ERROR_IOCTL) {
                        printf("%-25s", "");
                    }
                    ret = vdev.get_crop(crop);
                    if (ret == XCAM_RETURN_NO_ERROR) {
                        printf("(%-3u, %-3u)/%4uX%-5u", crop.c.left, crop.c.top, crop.c.width, crop.c.height);
                    } else {
                        printf("%-9s", "");
                    }
                }
            }
            printf("\n");
        }
        printf("\n");
    }
    printf("***************************************************************************************************************************************\n");
    printf("\nTable 2:\n");
    printf("Entity Name         Pad  Pad_type  Enabled Link           Devname\n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        bool flag = false;
        const struct media_entity_desc *info = media_entity_get_info(entity);
        printf("%-20s",entity->info.name);
        unsigned int num_links = media_entity_get_links_count(entity);
        for (j = 0; j < info->pads; j++){
            const struct media_pad *pad = media_entity_get_pad(entity, j);
            const char *fmt_code = "";
            enum v4l2_subdev_format_whence which = V4L2_SUBDEV_FORMAT_ACTIVE;
            bool flag = false;
            if (j != 0) {
                printf("%-20s"," ");
            }
            printf("%-5u%-10s",j,media_pad_type_to_string(pad->flags));
            for (k = 0; k < num_links; k++) {
                const struct media_link *link = media_entity_get_link(entity, k);
                const struct media_pad *source = link->source;
                const struct media_pad *sink = link->sink;
                if (link->flags & MEDIA_LNK_FL_ENABLED) {
                    if (source->entity == entity && source->index == j) {
                        if (flag) {
                            printf("%-35s", " ");
                        }
                        printf("-> %-20s%s\n",
                            media_entity_get_info(sink->entity)->name,
                            sink->entity->devname);
                        flag = true;
                    }
                    if (sink->entity == entity && sink->index == j) {
                        if(flag){
                            printf("%-35s", " ");
                        }
                        printf("<- %-20s%s\n",
                            media_entity_get_info(source->entity)->name,
                            source->entity->devname);
                        flag = true;
                    }
                }
            }
            if (!flag) {
                printf("\n");
            }
        }
        printf("\n");
    }
    printf("=======================================================================================================================================\n\n");
    return 0;
}

static int all_pipelin_info_print(struct dev_pipeline *pipeline)
{
    int i;
    const char *sensor_name = NULL;
    struct media_device *device = NULL;
    const char *device_name = NULL;
    struct media_entity *mainpath = NULL;
    FILE *fp = NULL;

    for (i = int(pipeline->entity_num)-1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        auto media_name_map = map_pipeline.find(i);
        std::string media_dev_name = media_name_map->second;
        if(i != int(pipeline->entity_num) - 1){
            printf(" -> ");
        }
        printf("%s", info->name);
        if (strcmp(info->name, "rkisp-isp-subdev") == 0){
            device_name = media_dev_name.c_str();
            if (device_name)
            {
                fp = fopen (device_name, "r");
                if (fp){
                    device = media_device_new(device_name);
                    if(device){
                        media_device_enumerate(device);
                        mainpath = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
                    }
                    fclose(fp);
                }
            }
            printf("(driver: %s  model: %s)", device->info.driver, device->info.model);
            break;
        }
    }
    if (mainpath){
        printf(" -> %s(%s)", mainpath->info.name, mainpath->devname);
    }
    printf("\n\n");
    return 0;
}

static const char *get_pipeline_info_by_cameraid(const char *name)
{
    char sys_path[64];
    FILE *fp = NULL;
    struct media_device *device = NULL;
    const struct media_entity_desc *entity_info = NULL;

    media_entity *entity = NULL;
    media_entity *camera_entity = NULL;
    char *model_name = NULL;
    bool flag = false;
    //轮询media 查找所要的media
    for (int i = 0; i < MAX_MEDIA_INDEX; i++)
    {
        snprintf(sys_path, 64, "/dev/media%d", i);
        fp = fopen (sys_path, "r");
        if (!fp)
        {
            continue;
        }
        fclose(fp);
        device = media_device_new(sys_path);
        if (!device) {
            printf("device is no exit\n");
        }
        media_device_enumerate(device);
        unsigned int j;
        for (j = 0; j < device->entities_count; j++) {
            entity = &device->entities[j];
            entity_info = media_entity_get_info(entity);
            if(strncmp(entity_info->name, name, 3) == 0 && strncmp(media_entity_subtype_to_string(entity_info->type), "Sensor", strlen("Sensor")) == 0){
                flag = 1;
                camera_entity = entity;
                model_name = device->info.model;
                break;
            }
        }
        if (flag)
            break;
    }

    if (camera_entity && model_name) {
        if (strncmp(model_name, "rkisp", 5) == 0) {
            return device->info.driver;
        }else {
            for (int i = 0; i < MAX_MEDIA_INDEX; i++)
            {
                snprintf(sys_path, 64, "/dev/media%d", i);
                fp = fopen (sys_path, "r");
                if (!fp)
                {
                    continue;
                }
                fclose(fp);
                device = media_device_new(sys_path);
                if (!device) {
                    printf("device is no exit\n");
                }
                media_device_enumerate(device);
                unsigned int j;
                for (j = 0; j < device->entities_count; j++) {
                    entity = &device->entities[j];
                    entity_info = media_entity_get_info(entity);
                    if(strcmp(entity_info->name, model_name) == 0){
                        return device->info.driver;
                    }
                }
            }
        }
    } else {
        return NULL;
    }
    return NULL;
}

static void print_help_info(){
    printf("\nmedia_enquiry tool help information\n");
    printf("\t-a                              print all media informaiton\n");
    printf("\t-b [require]|<isp-driver>       get media informaiton by isp driver name, such as \"-b rkisp0-vir0\"\n");
    printf("\t-c [require]|<camera id>        get media informaiton by isp camera id, such as \"-c m00\"\n");
    printf("\t-v [require]|<camera id>        get video fmt and crop informaiton by devname, such as \"-c /dev/video1\"\n");
    printf("\t-p                              print all pipeline links informaiton\n");
    printf("\n");
};

int main(int argc, char **argv)
{
    int c;
    int flags = 0;
    const char *name = NULL;

    const char* rkisp_driver[] = {
        "rkisp0-vir0",
        "rkisp0-vir1",
        "rkisp0-vir2",
        "rkisp0-vir3",
        "rkisp1-vir0",
        "rkisp1-vir1",
        "rkisp1-vir2",
        "rkisp1-vir3",
        "rkisp-vir0",
        "rkisp-vir1",
        "rkisp-vir2",
        "rkisp-vir3",
        "rkisp-vir4",
        "rkisp-vir5",
        "rkisp-vir6",
        "rkisp-vir7",
        NULL
    };
    c = getopt(argc, argv, "ab:c:v:hp");
    if (c == -1) {
        printf("command is error\n");
    }

    switch (c)
    {
        case 'a':
        {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            char camera_id[4];
            printf("\nAll Pipeline Links Information:");
            printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            for(int i=0; i < 8; i++){
                snprintf(camera_id, 4, "m0%d", i);
                map_pipeline.clear();
                name = get_pipeline_info_by_cameraid((const char*)camera_id);
                if (name) {
                    name = get_isp_dev_info_by_name(name, pipeline);
                    if (name != NULL) {
                        get_vicap_dev_info_by_name(name, pipeline);
                    }
                }
                if (pipeline->entity_num != 0) {
                    all_pipelin_info_print(pipeline);
                    pipeline->entity_num = 0;
                }
            }
            printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            for(int i=0; rkisp_driver[i] != NULL;i++){
                map_pipeline.clear();
                bool flag = false;
                name = get_isp_dev_info_by_name(rkisp_driver[i],pipeline);
                if (name != NULL) {
                    get_vicap_dev_info_by_name(name,pipeline);
                }
                if (pipeline->entity_num != 0) {
                    pipelin_dev_info_print_two(pipeline);
                    pipeline->entity_num = 0;
                }
            }
            free(pipeline);
        }
            break;

        case 'b':
        {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            map_pipeline.clear();
            name = get_isp_dev_info_by_name(optarg, pipeline);
            if (name != NULL){
                get_vicap_dev_info_by_name(name, pipeline);
            }
            if (pipeline->entity_num != 0) {
                pipelin_dev_info_print_two(pipeline);
                pipeline->entity_num = 0;
            } else {
                printf("%s is no found.\n", optarg);
            }
            pipeline->entity_num = 0;
            free(pipeline);
        }
            break;

        case 'c':
        {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            map_pipeline.clear();
            name = get_pipeline_info_by_cameraid(optarg);
            if (name) {
                name = get_isp_dev_info_by_name(name, pipeline);
                if (name != NULL) {
                    get_vicap_dev_info_by_name(name, pipeline);
                }
            }
            if (pipeline->entity_num != 0) {
                pipelin_dev_info_print_two(pipeline);
                pipeline->entity_num = 0;
            } else {
                printf("%s is no found.\n", optarg);
            }
            free(pipeline);
        }
            break;

        case 'v':
        {
            get_video_fmt_info(optarg);
        }
            break;

        case 'p' :
        {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            char camera_id[4];
            printf("\nAll Pipeline Links Information:");
            printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            for(int i=0; i < 8; i++){
                snprintf(camera_id, 4, "m0%d", i);
                map_pipeline.clear();
                name = get_pipeline_info_by_cameraid((const char*)camera_id);
                if (name) {
                    name = get_isp_dev_info_by_name(name, pipeline);
                    if (name != NULL) {
                        get_vicap_dev_info_by_name(name, pipeline);
                    }
                }
                if (pipeline->entity_num != 0) {
                    all_pipelin_info_print(pipeline);
                    pipeline->entity_num = 0;
                }
            }
            printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        }
            break;

        case 'h':
            print_help_info();
            break;

        default:
            print_help_info();
            break;
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mediactl.h>
#include <mediactl-priv.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdbool.h>
// #include "v4l2-mediabus.h"

#include "v4l2subdev.h"
// #include "v4l2_device.h"
#include <tools.h>

#define MAX_MEDIA_INDEX               16
#define DEV_PATH_LEN                  64
#define SENSOR_ATTACHED_FLASH_MAX_NUM 2
#define MAX_CAM_NUM                   8
#define MAX_ISP_LINKED_VICAP_CNT      4
#define ISP_TX_BUF_NUM                4
#define VIPCAP_TX_BUF_NUM             4

struct dev_pipeline {
    unsigned int entity_num;
    struct media_entity *entities[7];
};

struct map_entry {
    int key;
    char value[64];
};

struct media_device_map {
    char sys_path[64];
    struct media_device *dev;
};

static struct media_device_map g_device_map[MAX_MEDIA_INDEX] = {0};
static struct map_entry map_pipeline[7];
static int map_size = 0;

static void map_clear(void)
{
    map_size = 0;
}

static void map_insert(int key, const char *value)
{
    if (map_size < 7) {
        map_pipeline[map_size].key = key;
        strncpy(map_pipeline[map_size].value, value, 63);
        map_pipeline[map_size].value[63] = '\0';
        map_size++;
    }
}

static const char* map_find(int key)
{
    for (int i = 0; i < map_size; i++) {
        if (map_pipeline[i].key == key) {
            return map_pipeline[i].value;
        }
    }
    return NULL;
}

static const char* fmt2str(unsigned fmt)
{
    static char retstr[5] = {0};
    retstr[0] = fmt & 0xFF;
    retstr[1] = (fmt >> 8) & 0x7F;
    retstr[2] = (fmt >> 16) & 0x7F;
    retstr[3] = (fmt >> 24) & 0x7F;
    return retstr;
}

const char *
xcam_fourcc_to_string (uint32_t fourcc)
{
    static char str[5];

    memset(&(str), 0, sizeof(str));
    memcpy (str, &fourcc, 4);
    return str;
}

static void media_enquiry_init()
{
    char sys_path[64];
    FILE *fp = NULL;
    struct media_device *device = NULL;
    for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
        snprintf(sys_path, 64, "/dev/media%d", i);
        fp = fopen(sys_path, "r");
        if (!fp) {
            continue;
        }
        fclose(fp);
        device = media_device_new(sys_path);
        if (!device) {
            printf("device is no exit\n");
            continue;
        }
        media_device_enumerate(device);
        strcpy(g_device_map[i].sys_path, sys_path);
        g_device_map[i].dev = device;
    }
}

static void media_enquiry_deinit()
{
    for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
        if (g_device_map[i].dev) {
            media_device_unref(g_device_map[i].dev);
            g_device_map[i].dev = NULL;
        }
    }
}

static int get_video_fmt_info(const char *name)
{
    struct v4l2_format fmt;
    struct v4l2_crop crop;

    memset(&fmt, 0, sizeof(fmt));
    memset(&crop, 0, sizeof(crop));

    int fd = -1;
    fd = open(name, O_RDWR | O_CLOEXEC);
    if (fd >= 0) {
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl (fd, VIDIOC_G_FMT, &fmt) < 0) {
            printf("fmt:  get fmt info errors\n");
        } else {
            printf("fmt:           %uX%u\npixelformat:   %s\n",
                   fmt.fmt.pix.width, fmt.fmt.pix.height,
                   xcam_fourcc_to_string(fmt.fmt.pix.pixelformat));
        }
        if (ioctl (fd, VIDIOC_G_CROP, &crop) < 0) {
            printf("crop: get crop info errors\n");
        } else {
            printf("crop:          (%u, %u)%uX%u\n",
                   crop.c.left, crop.c.top, crop.c.width, crop.c.height);
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
    type &= MEDIA_ENT_TYPE_MASK;
    for (unsigned int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
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
        if (subtype >= sizeof(node_types)/sizeof(node_types[0]))
            subtype = 0;
        return node_types[subtype];
    case MEDIA_ENT_T_V4L2_SUBDEV:
        if (subtype >= sizeof(subdev_types)/sizeof(subdev_types[0]))
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
    for (unsigned int i = 0; i < sizeof(flags)/sizeof(flags[0]); i++) {
        if (flags[i].flag & flag)
            return flags[i].name;
    }
    return "Unknown";
}

static const char *get_isp_dev_info_by_name(const char *name, struct dev_pipeline *pipeline)
{
    const char* sys_path = NULL;
    FILE *fp = NULL;
    struct media_device *device = NULL;
    bool sensor_flag = false;
    const struct media_pad *pad = NULL;
    const struct media_link *link = NULL;
    const struct media_entity_desc *entity_info = NULL;
    struct media_entity *entity = NULL;
    struct media_entity *sensor_entity = NULL;

    for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
        if (!g_device_map[i].dev) continue;

        sys_path = g_device_map[i].sys_path;
        device = g_device_map[i].dev;
        if (strcmp(device->info.driver, name) == 0) {
            break;
        }

        device = NULL;
    }

    if (!device || strcmp(device->info.driver, name) != 0) {
        return NULL;
    }

    pipeline->entity_num = 0;

    entity = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_insert((int)pipeline->entity_num, sys_path);
        pipeline->entity_num++;
    }

    entity = media_get_entity_by_name(device, "rkisp_selfpath", strlen("rkisp_selfpath"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_insert((int)pipeline->entity_num, sys_path);
        pipeline->entity_num++;
    }

    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (entity) {
        pipeline->entities[pipeline->entity_num] = entity;
        map_insert((int)pipeline->entity_num, sys_path);
        pipeline->entity_num++;
    }

    for (unsigned int j = 0; j < device->entities_count; j++) {
        entity = &device->entities[j];
        entity_info = media_entity_get_info(entity);
        if (strncmp(media_entity_subtype_to_string(entity_info->type), "Sensor", strlen("Sensor")) == 0) {
            sensor_flag = true;
            sensor_entity = entity;
            break;
        }
    }

    const char* linked_entity_name_strs[] = {
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
        entity = media_get_entity_by_name(device, linked_entity_name_strs[i], strlen(linked_entity_name_strs[i]));
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
            if (sensor_flag) {
                break;
            } else {
                return entity->info.name;
            }
        }
    }

    if (sensor_flag) {
        entity = media_get_entity_by_name(device, "rkisp-csi-subdev", strlen("rkisp-csi-subdev"));
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
        }

        entity = media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
        } else {
            entity = media_get_entity_by_name(device, "rockchip-csi2-dphy1", strlen("rockchip-csi2-dphy1"));
            if (entity) {
                pipeline->entities[pipeline->entity_num] = entity;
                map_insert((int)pipeline->entity_num, sys_path);
                pipeline->entity_num++;
            }
        }

        pipeline->entities[pipeline->entity_num] = sensor_entity;
        map_insert((int)pipeline->entity_num, sys_path);
        pipeline->entity_num++;
    }

    return NULL;
}

static int get_vicap_dev_info_by_name(const char *name, struct dev_pipeline *pipeline)
{
    const char* sys_path = NULL;
    FILE *fp = NULL;
    struct media_device *device = NULL;
    const struct media_pad *pad = NULL;
    const struct media_link *link = NULL;
    const struct media_entity_desc *entity_info = NULL;
    struct media_entity *entity = NULL;
    struct media_entity *camera_entity = NULL;
    struct media_entity *mipi_csi2_entity = NULL;
    bool flag0 = false;
    bool flag1 = false;
    bool flag2 = false;

    for (int i = 0; i < MAX_MEDIA_INDEX; i++) {

        if (!g_device_map[i].dev) continue;

        sys_path = g_device_map[i].sys_path;
        device = g_device_map[i].dev;
        if (strcmp(device->info.model, name) == 0) {
            break;
        }

        device = NULL;
    }

    if (!device || strcmp(device->info.model, name) != 0) {
        return 0;
    }

    if (strcmp(name, "rkcif_dev") == 0) {
        flag0 = true;
    } else if (strcmp(name, "rkcif_lite_mipi_lvds") == 0) {
        flag1 = true;
    } else {
        flag2 = true;
    }

    const char* dphy_name[] = {
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

    if (flag0 || flag1) {
        return 0;
    }

    if (flag2) {
        for (int i = 0; dphy_name[i] != NULL; i++) {
            entity = media_get_entity_by_name(device, dphy_name[i], strlen(dphy_name[i]));
            if (entity) {
                for (unsigned int j = 0; j < entity->num_links; j++) {
                    link = media_entity_get_link(entity, j);
                    if (entity == link->sink->entity) {
                        camera_entity = link->source->entity;
                    }
                }
                for (unsigned int j = 0; j < entity->num_links; j++) {
                    link = media_entity_get_link(entity, j);
                    if (entity == link->source->entity) {
                        mipi_csi2_entity = link->sink->entity;
                    }
                }
                break;
            }
        }
        if (mipi_csi2_entity) {
            pipeline->entities[pipeline->entity_num] = mipi_csi2_entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
        }
        if (entity) {
            pipeline->entities[pipeline->entity_num] = entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
        }
        if (camera_entity) {
            pipeline->entities[pipeline->entity_num] = camera_entity;
            map_insert((int)pipeline->entity_num, sys_path);
            pipeline->entity_num++;
        }

        return 0;
    }

    return -1;
}

static int pipelin_dev_info_print_two(struct dev_pipeline *pipeline)
{
    struct v4l2_mbus_framefmt format;
    struct v4l2_rect rect;
    const char *sensor_name = NULL;
    int ret;

    for (int i = (int)pipeline->entity_num - 1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        if (strncmp(media_entity_subtype_to_string(info->type), "Sensor", strlen("Sensor")) == 0) {
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

    for (int i = (int)pipeline->entity_num - 1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        if (i != (int)pipeline->entity_num - 1) {
            printf(" -> ");
        }
        printf("%-20s", info->name);
        if (strcmp(info->name, "rkisp-isp-subdev") == 0)
            break;
    }

    printf("\n***************************************************************************************************************************************\n");
    printf("\nTable 1:\n");
    printf("Entity Name              Media_name     Devname             Pad  Pad_type  Fmt_code       Fmt       Crop                Crop.bounds         \n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");

    for (int i = (int)pipeline->entity_num - 1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        const char *media_dev_name = map_find(i);

        printf("%-25s%-15s%-20s", entity->info.name,
               media_dev_name ? media_dev_name : "",
               entity->devname);

        for (unsigned int j = 0; j < info->pads; j++) {
            const struct media_pad *pad = media_entity_get_pad(entity, j);
            const char *fmt_code = "";
            enum v4l2_subdev_format_whence which = V4L2_SUBDEV_FORMAT_ACTIVE;

            if (j != 0) {
                printf("%-60s", " ");
            }
            printf("%-5u%-10s", j, media_pad_type_to_string(pad->flags));

            if (media_entity_type(entity) == MEDIA_ENT_T_V4L2_SUBDEV) {
                ret = v4l2_subdev_get_format(entity, &format, j, which);
                if (ret == 0) {
                    fmt_code = v4l2_subdev_pixelcode_to_string((enum v4l2_mbus_pixelcode)(format.code));
                }
                printf("%-15s%-4uX%-5u", fmt_code, format.width, format.height);

                ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP, which);
                if (ret == 0) {
                    printf("(%-3u,%-3u)/%4uX%-5u", rect.left, rect.top, rect.width, rect.height);
                } else {
                    printf("%-20s", "");
                }

                ret = v4l2_subdev_get_selection(entity, &rect, j, V4L2_SEL_TGT_CROP_BOUNDS, which);
                if (ret == 0) {
                    printf("(%-3u,%-3u)/%4uX%-5u", rect.left, rect.top, rect.width, rect.height);
                } else {
                    printf("%-20s", "");
                }
            } else if (media_entity_type(entity) == MEDIA_ENT_T_DEVNODE) {
                struct v4l2_format fmt;
                struct v4l2_crop crop;

                memset(&crop, 0, sizeof(crop));
                int fd = -1;
                fd = open(entity->devname, O_RDWR | O_CLOEXEC);
                if (fd >= 0) {
                    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    if (ioctl (fd, VIDIOC_G_FMT, &fmt) >= 0) {
                        printf("%-15s%-4uX%-5u",
                               xcam_fourcc_to_string(fmt.fmt.pix.pixelformat),
                               fmt.fmt.pix.width, fmt.fmt.pix.height);
                    } else {
                        printf("%-25s", "");
                    }
                    if (ioctl (fd, VIDIOC_G_CROP, &crop) >= 0) {
                        printf("(%-3u, %-3u)/%4uX%-5u",
                               crop.c.left, crop.c.top, crop.c.width, crop.c.height);
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
    printf("Entity Name              Pad  Pad_type  Enabled Link           Devname\n");
    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");

    for (int i = (int)pipeline->entity_num - 1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        printf("%-25s", entity->info.name);

        for (unsigned int j = 0; j < info->pads; j++) {
            const struct media_pad *pad = media_entity_get_pad(entity, j);
            bool flag = false;

            if (j != 0) {
                printf("%-20s", " ");
            }
            printf("%-5u%-10s", j, media_pad_type_to_string(pad->flags));

            for (unsigned int k = 0; k < media_entity_get_links_count(entity); k++) {
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
                        if (flag) {
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
    const char *device_name = NULL;
    struct media_device *device = NULL;
    struct media_entity *mainpath = NULL;
    FILE *fp = NULL;

    for (int i = (int)pipeline->entity_num - 1; i >= 0; i--) {
        struct media_entity *entity = pipeline->entities[i];
        const struct media_entity_desc *info = media_entity_get_info(entity);
        const char *media_dev_name = map_find(i);

        if (i != (int)pipeline->entity_num - 1) {
            printf(" -> ");
        }
        printf("%s", info->name);

        if (strcmp(info->name, "rkisp-isp-subdev") == 0) {
            device_name = media_dev_name;
            for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
                if (strcmp(g_device_map[i].sys_path, device_name) == 0) {
                    device = g_device_map[i].dev;
                    break;
                }
            }
            if (device) {
                media_device_enumerate(device);
                mainpath = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
                printf("(driver: %s  model: %s)", device->info.driver, device->info.model);
            }
            break;
        }
    }

    if (mainpath) {
        printf(" -> %s(%s)", mainpath->info.name, mainpath->devname);
    }
    printf("\n\n");
    return 0;
}

static const char *get_pipeline_info_by_cameraid(const char *name)
{
    FILE *fp = NULL;
    struct media_device *device = NULL;
    const struct media_entity_desc *entity_info = NULL;
    struct media_entity *entity = NULL;
    struct media_entity *camera_entity = NULL;
    char *model_name = NULL;
    int flag = 0;

    for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
        if (!g_device_map[i].dev) continue;
        device = g_device_map[i].dev;

        for (unsigned int j = 0; j < device->entities_count; j++) {
            entity = &device->entities[j];
            entity_info = media_entity_get_info(entity);
            if (strncmp(entity_info->name, name, 3) == 0 &&
                strncmp(media_entity_subtype_to_string(entity_info->type), "Sensor", strlen("Sensor")) == 0) {
                flag = 1;
                camera_entity = entity;
                model_name = device->info.model;
                break;
            }
        }
        if (flag) {
            break;
        }
        device = NULL;
    }

    if (camera_entity && model_name) {
        if (strncmp(model_name, "rkisp", 5) == 0) {
            const char *driver = device->info.driver;
            return driver;
        } else {
            for (int i = 0; i < MAX_MEDIA_INDEX; i++) {
                if (!g_device_map[i].dev) continue;
                device = g_device_map[i].dev;

                for (unsigned int j = 0; j < device->entities_count; j++) {
                    entity = &device->entities[j];
                    entity_info = media_entity_get_info(entity);
                    if (strcmp(entity_info->name, model_name) == 0) {
                        const char *driver = device->info.driver;
                        return driver;
                    }
                }
            }
        }
    }

    return NULL;
}

static void print_help_info(void)
{
    printf("\nmedia_enquiry tool help information\n");
    printf("\t-a                              print all media informaiton\n");
    printf("\t-b [require]|<isp-driver>       get media informaiton by isp driver name, such as \"-b rkisp0-vir0\"\n");
    printf("\t-c [require]|<camera id>        get media informaiton by isp camera id, such as \"-c m00\"\n");
    printf("\t-v [require]|<camera id>        get video fmt and crop informaiton by devname, such as \"-c /dev/video1\"\n");
    printf("\t-p                              print all pipeline links informaiton\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    int c;
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

    opterr = 0;
    c = getopt(argc, argv, "ab:c:v:hp");

    if (c == -1) {
        printf("command is error\n");
        print_help_info();
        return 1;
    }

    media_enquiry_init();

    switch (c) {
        case 'a': {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            char camera_id[4];
            printf("\nAll Pipeline Links Information:");
            printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

            for (int i = 0; i < 8; i++) {
                snprintf(camera_id, 4, "m0%d", i);
                map_clear();
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

            for (int i = 0; rkisp_driver[i] != NULL; i++) {
                map_clear();
                name = get_isp_dev_info_by_name(rkisp_driver[i], pipeline);
                if (name != NULL) {
                    get_vicap_dev_info_by_name(name, pipeline);
                }
                if (pipeline->entity_num != 0) {
                    pipelin_dev_info_print_two(pipeline);
                    pipeline->entity_num = 0;
                }
            }
            free(pipeline);
            break;
        }

        case 'b': {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            map_clear();
            name = get_isp_dev_info_by_name(optarg, pipeline);
            if (name != NULL) {
                get_vicap_dev_info_by_name(name, pipeline);
            }
            if (pipeline->entity_num != 0) {
                pipelin_dev_info_print_two(pipeline);
            } else {
                printf("%s is no found.\n", optarg);
            }
            free(pipeline);
            break;
        }

        case 'c': {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            map_clear();
            name = get_pipeline_info_by_cameraid(optarg);
            if (name) {
                name = get_isp_dev_info_by_name(name, pipeline);
                if (name != NULL) {
                    get_vicap_dev_info_by_name(name, pipeline);
                }
            }
            if (pipeline->entity_num != 0) {
                pipelin_dev_info_print_two(pipeline);
            } else {
                printf("%s is no found.\n", optarg);
            }
            free(pipeline);
            break;
        }

        case 'v': {
            get_video_fmt_info(optarg);
            break;
        }

        case 'p': {
            struct dev_pipeline *pipeline = (struct dev_pipeline*)malloc(sizeof(*pipeline));
            char camera_id[4];
            printf("\nAll Pipeline Links Information:");
            printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

            for (int i = 0; i < 8; i++) {
                snprintf(camera_id, 4, "m0%d", i);
                map_clear();
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
            free(pipeline);
            break;
        }

        case 'h':
            print_help_info();
            break;

        default:
            print_help_info();
            break;
    }

    media_enquiry_deinit();

    return 0;
}
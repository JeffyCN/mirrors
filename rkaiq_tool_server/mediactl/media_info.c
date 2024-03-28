#include "media_info.h"

void print_flags(const struct flag_name* flag_names, unsigned int num_entries, __u32 flags)
{
    bool first = true;
    unsigned int i;

    for (i = 0; i < num_entries; i++)
    {
        if (!(flags & flag_names[i].flag))
        {
            continue;
        }
        if (!first)
        {
            printf(",");
        }
        printf("%s", flag_names[i].name);
        flags &= ~flag_names[i].flag;
        first = false;
    }

    if (flags)
    {
        if (!first)
        {
            printf(",");
        }
        printf("0x%x", flags);
    }
}

void v4l2_subdev_print_format(struct media_entity* entity, unsigned int pad, enum v4l2_subdev_format_whence which)
{
    struct v4l2_mbus_framefmt format;
    struct v4l2_fract interval = {0, 0};
    struct v4l2_rect rect;
    int ret;

    ret = v4l2_subdev_get_format(entity, &format, pad, which);
    if (ret != 0)
    {
        return;
    }

    ret = v4l2_subdev_get_frame_interval(entity, &interval, pad);
    if (ret != 0 && ret != -ENOTTY && ret != -EINVAL)
    {
        return;
    }

    printf("\t\t[fmt:%s/%ux%u", v4l2_subdev_pixelcode_to_string(format.code), format.width, format.height);

    if (interval.numerator || interval.denominator)
    {
        printf("@%u/%u", interval.numerator, interval.denominator);
    }

    if (format.field)
    {
        printf(" field:%s", v4l2_subdev_field_to_string(format.field));
    }

    if (format.colorspace)
    {
        printf(" colorspace:%s", v4l2_subdev_colorspace_to_string(format.colorspace));

        if (format.xfer_func)
        {
            printf(" xfer:%s", v4l2_subdev_xfer_func_to_string(format.xfer_func));
        }

        if (format.ycbcr_enc)
            printf(" ycbcr:%s", v4l2_subdev_ycbcr_encoding_to_string(format.ycbcr_enc));

        if (format.quantization)
            printf(" quantization:%s", v4l2_subdev_quantization_to_string(format.quantization));
    }

    ret = v4l2_subdev_get_selection(entity, &rect, pad, V4L2_SEL_TGT_CROP_BOUNDS, which);
    if (ret == 0)
        printf("\n\t\t crop.bounds:(%u,%u)/%ux%u", rect.left, rect.top, rect.width, rect.height);

    ret = v4l2_subdev_get_selection(entity, &rect, pad, V4L2_SEL_TGT_CROP, which);
    if (ret == 0)
        printf("\n\t\t crop:(%u,%u)/%ux%u", rect.left, rect.top, rect.width, rect.height);

    ret = v4l2_subdev_get_selection(entity, &rect, pad, V4L2_SEL_TGT_COMPOSE_BOUNDS, which);
    if (ret == 0)
        printf("\n\t\t compose.bounds:(%u,%u)/%ux%u", rect.left, rect.top, rect.width, rect.height);

    ret = v4l2_subdev_get_selection(entity, &rect, pad, V4L2_SEL_TGT_COMPOSE, which);
    if (ret == 0)
        printf("\n\t\t compose:(%u,%u)/%ux%u", rect.left, rect.top, rect.width, rect.height);

    printf("]\n");
}

const char* v4l2_dv_type_to_string(unsigned int type)
{
    static const struct
    {
        __u32 type;
        const char* name;
    } types[] = {
        {V4L2_DV_BT_656_1120, "BT.656/1120"},
    };

    static char unknown[20];
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(types); i++)
    {
        if (types[i].type == type)
        {
            return types[i].name;
        }
    }

    sprintf(unknown, "Unknown (%u)", type);
    return unknown;
}

static const struct flag_name bt_standards[] = {
    {V4L2_DV_BT_STD_CEA861, "CEA-861"}, {V4L2_DV_BT_STD_DMT, "DMT"}, {V4L2_DV_BT_STD_CVT, "CVT"}, {V4L2_DV_BT_STD_GTF, "GTF"}, {V4L2_DV_BT_STD_SDI, "SDI"},
};

static const struct flag_name bt_capabilities[] = {
    {V4L2_DV_BT_CAP_INTERLACED, "interlaced"},
    {V4L2_DV_BT_CAP_PROGRESSIVE, "progressive"},
    {V4L2_DV_BT_CAP_REDUCED_BLANKING, "reduced-blanking"},
    {V4L2_DV_BT_CAP_CUSTOM, "custom"},
};

static const struct flag_name bt_flags[] = {
    {V4L2_DV_FL_REDUCED_BLANKING, "reduced-blanking"},
    {V4L2_DV_FL_CAN_REDUCE_FPS, "can-reduce-fps"},
    {V4L2_DV_FL_REDUCED_FPS, "reduced-fps"},
    {V4L2_DV_FL_HALF_LINE, "half-line"},
    {V4L2_DV_FL_IS_CE_VIDEO, "CE-video"},
    {V4L2_DV_FL_FIRST_FIELD_EXTRA_LINE, "first-field-extra-line"},
    {V4L2_DV_FL_HAS_PICTURE_ASPECT, "has-picture-aspect"},
    {V4L2_DV_FL_HAS_CEA861_VIC, "has-cea861-vic"},
    {V4L2_DV_FL_HAS_HDMI_VIC, "has-hdmi-vic"},
    {V4L2_DV_FL_CAN_DETECT_REDUCED_FPS, "can-detect-reduced-fps"},
};

void v4l2_subdev_print_dv_timings(const struct v4l2_dv_timings* timings, const char* name)
{
    printf("\t\t[dv.%s:%s", name, v4l2_dv_type_to_string(timings->type));

    switch (timings->type)
    {
        case V4L2_DV_BT_656_1120: {
            const struct v4l2_bt_timings* bt = &timings->bt;
            unsigned int htotal, vtotal;

            htotal = V4L2_DV_BT_FRAME_WIDTH(bt);
            vtotal = V4L2_DV_BT_FRAME_HEIGHT(bt);

            printf(" %ux%u%s%llu (%ux%u)", bt->width, bt->height, bt->interlaced ? "i" : "p", (htotal * vtotal) > 0 ? (bt->pixelclock / (htotal * vtotal)) : 0, htotal, vtotal);

            printf(" stds:");
            print_flags(bt_standards, ARRAY_SIZE(bt_standards), bt->standards);
            printf(" flags:");
            print_flags(bt_flags, ARRAY_SIZE(bt_flags), bt->flags);

            break;
        }
    }

    printf("]\n");
}

void v4l2_subdev_print_pad_dv(struct media_entity* entity, unsigned int pad, enum v4l2_subdev_format_whence which)
{
    struct v4l2_dv_timings_cap caps;
    int ret;

    caps.pad = pad;
    ret = v4l2_subdev_get_dv_timings_caps(entity, &caps);
    if (ret != 0)
    {
        return;
    }

    printf("\t\t[dv.caps:%s", v4l2_dv_type_to_string(caps.type));

    switch (caps.type)
    {
        case V4L2_DV_BT_656_1120:
            printf(" min:%ux%u@%llu max:%ux%u@%llu", caps.bt.min_width, caps.bt.min_height, caps.bt.min_pixelclock, caps.bt.max_width, caps.bt.max_height, caps.bt.max_pixelclock);

            printf(" stds:");
            print_flags(bt_standards, ARRAY_SIZE(bt_standards), caps.bt.standards);
            printf(" caps:");
            print_flags(bt_capabilities, ARRAY_SIZE(bt_capabilities), caps.bt.capabilities);

            break;
    }

    printf("]\n");
}

void v4l2_subdev_print_subdev_dv(struct media_entity* entity)
{
    struct v4l2_dv_timings timings;
    int ret;

    ret = v4l2_subdev_query_dv_timings(entity, &timings);
    switch (ret)
    {
        case -ENOLINK:
            printf("\t\t[dv.query:no-link]\n");
            break;
        case -ENOLCK:
            printf("\t\t[dv.query:no-lock]\n");
            break;
        case -ERANGE:
            printf("\t\t[dv.query:out-of-range]\n");
            break;
        case 0:
            v4l2_subdev_print_dv_timings(&timings, "detect");
            break;
        default:
            return;
    }

    ret = v4l2_subdev_get_dv_timings(entity, &timings);
    if (ret == 0)
    {
        v4l2_subdev_print_dv_timings(&timings, "current");
    }
}

const char* media_entity_type_to_string(unsigned type)
{
    static const struct
    {
        __u32 type;
        const char* name;
    } types[] = {
        {MEDIA_ENT_T_DEVNODE, "Node"},
        {MEDIA_ENT_T_V4L2_SUBDEV, "V4L2 subdev"},
    };

    unsigned int i;

    type &= MEDIA_ENT_TYPE_MASK;

    for (i = 0; i < ARRAY_SIZE(types); i++)
    {
        if (types[i].type == type)
        {
            return types[i].name;
        }
    }

    return "Unknown";
}

const char* media_entity_subtype_to_string(unsigned type)
{
    static const char* node_types[] = {
        "Unknown", "V4L", "FB", "ALSA", "DVB",
    };
    static const char* subdev_types[] = {
        "Unknown", "Sensor", "Flash", "Lens", "Decoder", "Tuner",
    };

    unsigned int subtype = type & MEDIA_ENT_SUBTYPE_MASK;

    switch (type & MEDIA_ENT_TYPE_MASK)
    {
        case MEDIA_ENT_T_DEVNODE:
            if (subtype >= ARRAY_SIZE(node_types))
            {
                subtype = 0;
            }
            return node_types[subtype];

        case MEDIA_ENT_T_V4L2_SUBDEV:
            if (subtype >= ARRAY_SIZE(subdev_types))
            {
                subtype = 0;
            }
            return subdev_types[subtype];
        default:
            return node_types[0];
    }
}

const char* media_pad_type_to_string(unsigned flag)
{
    static const struct
    {
        __u32 flag;
        const char* name;
    } flags[] = {
        {MEDIA_PAD_FL_SINK, "Sink"},
        {MEDIA_PAD_FL_SOURCE, "Source"},
    };

    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(flags); i++)
    {
        if (flags[i].flag & flag)
        {
            return flags[i].name;
        }
    }

    return "Unknown";
}

void media_print_pad_text(struct media_entity* entity, const struct media_pad* pad)
{
    if (media_entity_type(entity) != MEDIA_ENT_T_V4L2_SUBDEV)
    {
        return;
    }

    v4l2_subdev_print_format(entity, pad->index, V4L2_SUBDEV_FORMAT_ACTIVE);
    v4l2_subdev_print_pad_dv(entity, pad->index, V4L2_SUBDEV_FORMAT_ACTIVE);

    if (pad->flags & MEDIA_PAD_FL_SOURCE)
    {
        v4l2_subdev_print_subdev_dv(entity);
    }
}

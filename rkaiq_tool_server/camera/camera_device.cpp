#include "camera_device.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

void device_close(int dev_fd)
{
    if (-1 == close(dev_fd))
    {
        errno_debug("close");
    }
    dev_fd = -1;
}

int device_open(const char* dev_name)
{
    int dev_fd;
    struct stat st;
    if (-1 == stat(dev_name, &st))
    {
        LOG_ERROR("Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        return -1;
    }
    if (!S_ISCHR(st.st_mode))
    {
        LOG_ERROR("%s is no devicen", dev_name);
        return -1;
    }
    dev_fd = open(dev_name, O_RDWR /*| O_NONBLOCK*/, 0);
    if (-1 == dev_fd)
    {
        LOG_ERROR("Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        return -1;
    }
    return dev_fd;
}

int device_querycap(int dev_fd, struct v4l2_capability* cap)
{
    int ret = xioctl(dev_fd, VIDIOC_QUERYCAP, cap);
    if (-1 == ret)
    {
        if (EINVAL == errno)
        {
            errno_debug("VIDIOC_QUERYCAP EINVAL");
        }
        else
        {
            errno_debug("VIDIOC_QUERYCAP");
        }
    }
    return ret;
}

int device_cropcap(int dev_fd, struct v4l2_cropcap* cropcap, struct v4l2_crop* crop)
{
    int ret = xioctl(dev_fd, VIDIOC_CROPCAP, cropcap);
    // if (0 == ret) {
    ret = xioctl(dev_fd, VIDIOC_S_CROP, crop);
    if (-1 == ret)
    {
        switch (errno)
        {
            case EINVAL:
                LOG_ERROR("Cropping not supported.\n");
                break;
            default:
                /* Errors ignored. */
                break;
        }
    }
    //}
    return ret;
}

int device_setformat(int dev_fd, struct v4l2_format* fmt)
{
    /* Note VIDIOC_S_FMT may change width and height. */
    int ret = xioctl(dev_fd, VIDIOC_S_FMT, fmt);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_S_FMT");
    }
    return ret;
}

int device_getformat(int dev_fd, struct v4l2_format* fmt)
{
    /* Preserve original settings as set by v4l2-ctl for example */
    int ret = xioctl(dev_fd, VIDIOC_G_FMT, fmt);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_G_FMT");
    }
    return ret;
}

int device_getsubdevformat(int dev_fd, struct v4l2_subdev_format* fmt)
{
    int ret = xioctl(dev_fd, VIDIOC_SUBDEV_G_FMT, fmt);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_SUBDEV_G_FMT");
    }
    return ret;
}

int device_setsubdevformat(int dev_fd, struct v4l2_subdev_format* fmt)
{
    int ret = xioctl(dev_fd, VIDIOC_SUBDEV_S_FMT, fmt);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_SUBDEV_G_FMT");
    }
    return ret;
}

int device_setsubdevcrop(int dev_fd, struct v4l2_subdev_selection* sel)
{
    int ret = xioctl(dev_fd, VIDIOC_SUBDEV_S_SELECTION, sel);
    if (-1 == ret)
    {
        switch (errno)
        {
            case EINVAL:
                LOG_ERROR("Cropping not supported.\n");
                break;
            default:
                /* Errors ignored. */
                break;
        }
    }
    return ret;
}

int device_getblank(int dev_fd, struct v4l2_queryctrl* ctrl)
{
    int ret = xioctl(dev_fd, VIDIOC_QUERYCTRL, ctrl);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_QUERYCTRL");
    }
    return ret;
}

int device_getsensorfps(int dev_fd, struct v4l2_subdev_frame_interval* finterval)
{
    int ret = xioctl(dev_fd, VIDIOC_SUBDEV_G_FRAME_INTERVAL, finterval);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_SUBDEV_G_FRAME_INTERVAL");
    }
    return ret;
}

int device_set3aexposure(int dev_fd, struct v4l2_ext_controls* ctrls)
{
    int ret = xioctl(dev_fd, VIDIOC_S_EXT_CTRLS, ctrls);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_S_EXT_CTRLS");
    }
    return ret;
}

int device_queryctrl(int dev_fd, struct v4l2_queryctrl* query)
{
    int ret = xioctl(dev_fd, VIDIOC_QUERYCTRL, query);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_S_CTRL");
    }
    return ret;
}

int device_setctrl(int dev_fd, struct v4l2_control* ctrl)
{
    int ret = xioctl(dev_fd, VIDIOC_S_CTRL, ctrl);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_S_CTRL");
    }
    return ret;
}

int device_streamon(int dev_fd, enum v4l2_buf_type* type)
{
    int ret = xioctl(dev_fd, VIDIOC_STREAMON, type);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_STREAMON");
    }
    return ret;
}

int device_streamoff(int dev_fd, enum v4l2_buf_type* type)
{
    int ret = xioctl(dev_fd, VIDIOC_STREAMOFF, type);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_STREAMOFF");
    }
    return ret;
}

int device_qbuf(int dev_fd, struct v4l2_buffer* buf)
{
    int ret = xioctl(dev_fd, VIDIOC_QBUF, buf);
    if (-1 == ret)
    {
        errno_debug("VIDIOC_QBUF");
    }
    return ret;
}

int device_dqbuf(int dev_fd, struct v4l2_buffer* buf)
{
    assert(buf);
    LOG_INFO("device_dqbuf begin\n");
    int ret = xioctl(dev_fd, VIDIOC_DQBUF, buf);
    LOG_INFO("device_dqbuf end\n");
    if (-1 == ret)
    {
        switch (errno)
        {
            case EAGAIN:
                return 0;
            case EIO:
            /* Could ignore EIO, see spec. */
            /* fall through */
            default:
                errno_debug("VIDIOC_DQBUF");
        }
    }
    return ret;
}

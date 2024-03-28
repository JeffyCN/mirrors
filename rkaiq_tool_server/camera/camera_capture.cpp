#include "camera_capture.h"
#include "rkaiq_common.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern uint32_t g_sensorHdrMode;
extern int g_sensorMemoryMode;
int g_frameDroppedFlag = 0;
uint g_lastCapturedSequense = 0;

void process_image(struct capture_info* cap_info, const void* p, int size)
{
    static int image_index = 0;
    LOG_DEBUG("image_index %d\n", image_index++);
    if (cap_info->out_fp)
    {
        fwrite(p, size, 1, cap_info->out_fp);
        fflush(cap_info->out_fp);
    }
}

int read_frame(struct capture_info* cap_info)
{
    struct v4l2_buffer buf;
    unsigned int i, bytesused;

    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            if (-1 == read(cap_info->dev_fd, cap_info->buffers[0].start, cap_info->buffers[0].length))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;
                    case EIO:
                    /* Could ignore EIO, see spec. */
                    /* fall through */
                    default:
                        errno_debug("read");
                }
            }
            process_image(cap_info, cap_info->buffers[0].start, cap_info->buffers[0].length);
            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = cap_info->capture_buf_type;
            buf.memory = V4L2_MEMORY_MMAP;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
            {
                struct v4l2_plane planes[FMT_NUM_PLANES];
                buf.m.planes = planes;
                buf.length = FMT_NUM_PLANES;
            }

            if (device_dqbuf(cap_info->dev_fd, &buf) == -1)
                break;

            LOG_INFO("RAW capture, sequence:%u\n", buf.sequence);
            cap_info->sequence = buf.sequence;
            assert(buf.index < cap_info->n_buffers);

            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
            {
                bytesused = buf.m.planes[0].bytesused;
            }
            else
            {
                bytesused = buf.bytesused;
            }

            process_image(cap_info, cap_info->buffers[buf.index].start, bytesused);

            device_qbuf(cap_info->dev_fd, &buf);
            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);
            buf.type = cap_info->capture_buf_type;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (device_dqbuf(cap_info->dev_fd, &buf) == -1)
                break;

            for (i = 0; i < cap_info->n_buffers; ++i)
            {
                if (buf.m.userptr == (unsigned long)cap_info->buffers[i].start && buf.length == cap_info->buffers[i].length)
                {
                    break;
                }
            }

            assert(i < cap_info->n_buffers);

            process_image(cap_info, (void*)buf.m.userptr, buf.bytesused);

            device_qbuf(cap_info->dev_fd, &buf);
            break;
    }

    return 1;
}

int read_frame(int handler, int index, struct capture_info* cap_info, CaptureCallBack callback)
{
    struct v4l2_buffer buf;
    unsigned int i, bytesused;

    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            if (-1 == read(cap_info->dev_fd, cap_info->buffers[0].start, cap_info->buffers[0].length))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;
                    case EIO:
                    /* Could ignore EIO, see spec. */
                    /* fall through */
                    default:
                        errno_debug("read");
                }
            }
            if (callback)
                callback(handler, index, cap_info->buffers[0].start, cap_info->buffers[0].length);
            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);
            struct v4l2_plane planes[FMT_NUM_PLANES];
            buf.type = cap_info->capture_buf_type;
            buf.memory = V4L2_MEMORY_MMAP;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
            {
                buf.m.planes = planes;
                buf.length = FMT_NUM_PLANES;
            }

            if (device_dqbuf(cap_info->dev_fd, &buf) == -1)
                break;

            LOG_INFO("RAW capture, sequence:%u\n", buf.sequence);
            cap_info->sequence = buf.sequence;
            if (g_lastCapturedSequense != 0 && buf.sequence - g_lastCapturedSequense > 5)
            {
                g_frameDroppedFlag = 1;
            }
            else
            {
                g_frameDroppedFlag = 0;
            }
            g_lastCapturedSequense = buf.sequence;

            assert(buf.index < cap_info->n_buffers);

            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
            {
                bytesused = buf.m.planes[0].bytesused;
            }
            else
            {
                bytesused = buf.bytesused;
            }

            if (callback)
            {
                callback(handler, index, cap_info->buffers[buf.index].start, bytesused);
            }
            memset(cap_info->buffers[buf.index].start, 0, bytesused);
            device_qbuf(cap_info->dev_fd, &buf);
            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

            buf.type = cap_info->capture_buf_type;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (device_dqbuf(cap_info->dev_fd, &buf) == -1)
                break;

            for (i = 0; i < cap_info->n_buffers; ++i)
                if (buf.m.userptr == (unsigned long)cap_info->buffers[i].start && buf.length == cap_info->buffers[i].length)
                {
                    break;
                }

            assert(i < cap_info->n_buffers);

            if (callback)
            {
                callback(handler, index, (void*)buf.m.userptr, buf.bytesused);
            }

            device_qbuf(cap_info->dev_fd, &buf);
            break;
    }

    return 1;
}

void stop_capturing(struct capture_info* cap_info)
{
    enum v4l2_buf_type type;

    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = cap_info->capture_buf_type;
            device_streamoff(cap_info->dev_fd, &type);
            break;
    }
}

void start_capturing(struct capture_info* cap_info)
{
    unsigned int i;
    enum v4l2_buf_type type;
    LOG_DEBUG("#### start_capturing, cap_info.dev_name:%s\n", cap_info->dev_name);

    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;
        case IO_METHOD_MMAP:
            LOG_DEBUG("IO_METHOD_MMAP, buffer number:%d\n", cap_info->n_buffers);
            for (i = 0; i < cap_info->n_buffers; ++i)
            {
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type = cap_info->capture_buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
                {
                    struct v4l2_plane planes[FMT_NUM_PLANES];
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                device_qbuf(cap_info->dev_fd, &buf);
            }
            type = cap_info->capture_buf_type;
            device_streamon(cap_info->dev_fd, &type);
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < cap_info->n_buffers; ++i)
            {
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type = cap_info->capture_buf_type;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long)cap_info->buffers[i].start;
                buf.length = cap_info->buffers[i].length;
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
                {
                    struct v4l2_plane planes[FMT_NUM_PLANES];
                    planes[0].m.userptr = (unsigned long)cap_info->buffers[i].start;
                    planes[0].length = cap_info->buffers[i].length;
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                device_qbuf(cap_info->dev_fd, &buf);
            }
            type = cap_info->capture_buf_type;
            device_streamon(cap_info->dev_fd, &type);
            break;
    }
}

void uninit_device(struct capture_info* cap_info)
{
    unsigned int i;
    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            free(cap_info->buffers[0].start);
            break;
        case IO_METHOD_MMAP:
            for (i = 0; i < cap_info->n_buffers; ++i)
                if (-1 == munmap(cap_info->buffers[i].start, cap_info->buffers[i].length))
                {
                    errno_debug("munmap");
                }
            break;
        case IO_METHOD_USERPTR:
            for (i = 0; i < cap_info->n_buffers; ++i)
            {
                free(cap_info->buffers[i].start);
            }
            break;
    }
    free(cap_info->buffers);
    device_close(cap_info->dev_fd);
    cap_info->dev_fd = -1;
}

int init_device(struct capture_info* cap_info)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
    int ret;

    cap_info->dev_fd = device_open(cap_info->dev_name);
    if (-1 != device_querycap(cap_info->dev_fd, &cap))
    {
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE))
        {
            LOG_ERROR("%s is no video capture device\n", cap_info->dev_name);
            return -1;
        }
    }

    if (-1 == check_io_method(cap_info->io, cap.capabilities))
    {
        return -1;
    }

    CLEAR(cropcap);
    cropcap.type = cap_info->capture_buf_type;
    crop.type = cap_info->capture_buf_type;
    // crop.c = cropcap.defrect; /* reset to default */
    crop.c.left = 0;
    crop.c.top = 0;
    crop.c.width = cap_info->width;
    crop.c.height = cap_info->height;
    if (cap_info->link == link_to_isp)
    {
        device_cropcap(cap_info->dev_fd, &cropcap, &crop);
    }

    CLEAR(fmt);
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
    {
        cap_info->capture_buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }
    else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
    {
        cap_info->capture_buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }

    if (cap_info->format)
    {
        fmt.type = cap_info->capture_buf_type;
        fmt.fmt.pix.width = cap_info->width;
        fmt.fmt.pix.height = cap_info->height;
        fmt.fmt.pix.pixelformat = cap_info->format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        ret = device_setformat(cap_info->dev_fd, &fmt);
        if (ret)
        {
            LOG_ERROR("%s set format failed\n", cap_info->dev_name);
        }
        else
        {
            LOG_INFO("%s set format success\n", cap_info->dev_name);
        }
    }
    else
    {
        device_getformat(cap_info->dev_fd, &fmt);
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
    {
        fmt.fmt.pix.bytesperline = min;
    }
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
    {
        fmt.fmt.pix.sizeimage = min;
    }
    init_io_method(cap_info, fmt.fmt.pix.sizeimage);
    return 0;
}

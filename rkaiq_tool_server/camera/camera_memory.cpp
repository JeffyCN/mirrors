#include "camera_memory.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern uint32_t g_mmapNumber;
int xioctl(int fh, int request, void* arg)
{
    int ret;
    do
    {
        ret = ioctl(fh, request, arg);
    } while (-1 == ret && EINTR == errno);
    return ret;
}

void init_read(struct capture_info* cap_info, unsigned int buffer_size)
{
    cap_info->buffers = (struct buffer*)calloc(1, sizeof(*cap_info->buffers));

    if (!cap_info->buffers)
    {
        LOG_ERROR("Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    cap_info->buffers[0].length = buffer_size;
    cap_info->buffers[0].start = malloc(buffer_size);

    if (!cap_info->buffers[0].start)
    {
        LOG_ERROR("Out of memory\\n");
        exit(EXIT_FAILURE);
    }
}

void init_mmap(struct capture_info* cap_info)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = g_mmapNumber;
    req.type = cap_info->capture_buf_type;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(cap_info->dev_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            LOG_ERROR("%s does not support "
                      "memory mappingn",
                      cap_info->dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_debug("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        LOG_ERROR("Insufficient buffer memory on %s\\n", cap_info->dev_name);
        exit(EXIT_FAILURE);
    }

    cap_info->buffers = (struct buffer*)calloc(req.count, sizeof(*cap_info->buffers));

    if (!cap_info->buffers)
    {
        LOG_ERROR("Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    LOG_DEBUG("g_mmapNumber:%d\n", g_mmapNumber);
    LOG_DEBUG("req.count:%d\n", req.count);

    for (cap_info->n_buffers = 0; cap_info->n_buffers < req.count; ++cap_info->n_buffers)
    {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];

        CLEAR(buf);
        CLEAR(planes);

        buf.type = cap_info->capture_buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = cap_info->n_buffers;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
        {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(cap_info->dev_fd, VIDIOC_QUERYBUF, &buf))
        {
            errno_debug("VIDIOC_QUERYBUF");
        }

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == cap_info->capture_buf_type)
        {
            cap_info->buffers[cap_info->n_buffers].length = buf.m.planes[0].length;
            cap_info->buffers[cap_info->n_buffers].start = mmap(NULL /* start anywhere */, buf.m.planes[0].length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, cap_info->dev_fd, buf.m.planes[0].m.mem_offset);
        }
        else
        {
            cap_info->buffers[cap_info->n_buffers].length = buf.length;
            cap_info->buffers[cap_info->n_buffers].start = mmap(NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, cap_info->dev_fd, buf.m.offset);
        }

        if (MAP_FAILED == cap_info->buffers[cap_info->n_buffers].start)
        {
            errno_debug("mmap");
        }

        memset(cap_info->buffers[cap_info->n_buffers].start, 0, cap_info->buffers[cap_info->n_buffers].length);
    }
}

void init_userp(struct capture_info* cap_info, unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = cap_info->capture_buf_type;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(cap_info->dev_fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            LOG_ERROR("%s does not support "
                      "user pointer i/on",
                      cap_info->dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_debug("VIDIOC_REQBUFS");
        }
    }

    cap_info->buffers = (struct buffer*)calloc(4, sizeof(*cap_info->buffers));

    if (!cap_info->buffers)
    {
        LOG_ERROR("Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    for (cap_info->n_buffers = 0; cap_info->n_buffers < 4; ++cap_info->n_buffers)
    {
        cap_info->buffers[cap_info->n_buffers].length = buffer_size;
        cap_info->buffers[cap_info->n_buffers].start = malloc(buffer_size);

        if (!cap_info->buffers[cap_info->n_buffers].start)
        {
            LOG_ERROR("Out of memory\\n");
            exit(EXIT_FAILURE);
        }
    }
}

int check_io_method(enum io_method io, unsigned int capabilities)
{
    switch (io)
    {
        case IO_METHOD_READ:
            if (!(capabilities & V4L2_CAP_READWRITE))
            {
                LOG_ERROR("Not support read i/o\n");
                return -1;
            }
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(capabilities & V4L2_CAP_STREAMING))
            {
                LOG_ERROR("Not support streaming i/o\n");
                return -1;
            }
            break;
    }
    return 0;
}

int init_io_method(struct capture_info* cap_info, unsigned int size)
{
    switch (cap_info->io)
    {
        case IO_METHOD_READ:
            init_read(cap_info, size);
            break;

        case IO_METHOD_MMAP:
            init_mmap(cap_info);
            break;

        case IO_METHOD_USERPTR:
            init_userp(cap_info, size);
            break;
    }
    return 0;
}

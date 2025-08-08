
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <signal.h>
#include <dirent.h>
#include <inttypes.h>

#include "rkrawstream_video_out.h"

#define xcam_mem_clear(v_stack) memset(&(v_stack), 0, sizeof(v_stack))
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1

#define BUFFER_COUNT 3

static int g_debug = 0;

#define DBG(...) do { if(g_debug) printf(__VA_ARGS__); } while(0)
#define ERR(...) do { printf(__VA_ARGS__); } while (0)

typedef struct _VI_stream_context_s {
    int                fd;
    int                width;
    int                height;
    int                format;
    enum v4l2_buf_type buf_type;
    VI_buffer_t*       buffers;
    unsigned int       n_buffers;
    int                limit_range;
    char               dev_name[64];
    bool               use_poll;
} VI_stream_context_t;

static void errno_exit(const char *dev_name, const char *s)
{
    ERR("%s: %s error %d, %s\n", dev_name, s, errno, strerror(errno));
    //exit(EXIT_FAILURE);
}

static int xioctl (int fd, unsigned long cmd, void *arg)
{
    int ret = 0;
    int tried_time = 0;

    if (fd < 0)
        return -1;

    while (1) {
        ret = ioctl (fd, cmd, arg);
        if (ret >= 0)
            break;
        if (errno != EINTR && errno != EAGAIN)
            break;
        // if (++tried_time > 5)
        //     break;
    }

    if (ret >= 0) {
        DBG ("ioctl return ok on fd(%d), cmd:0x%.8lx\n", fd, cmd);
    } else {
        DBG ("ioctl failed on fd(%d), cmd:0x%.8lx, error:%s\n",
                        fd, cmd, strerror(errno));
    }
    return ret;
}

static int open_device(const char* dev_name, bool use_poll)
{
    printf("-------- open output dev_name:%s -------------\n", dev_name);

    int fd = -1;

    if (!use_poll)
        fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);
    else
        fd = open(dev_name, O_RDWR | O_CLOEXEC | O_NONBLOCK, 0);

    if (-1 == fd) {
        ERR("Cannot open '%s': %d, %s\n",
            dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

static int close_device(int fd)
{
   return close(fd);

}

static void init_mmap(VI_stream_context_t *ctx)
{
    struct v4l2_requestbuffers req;
    int fd_tmp = -1;

    CLEAR(req);


    fd_tmp = ctx->fd;

    req.count = BUFFER_COUNT;
    req.type = ctx->buf_type;
    req.memory = V4L2_MEMORY_MMAP;

    VI_buffer_t *tmp_buffers = NULL;

    if (-1 == xioctl(fd_tmp, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            ERR("%s does not support "
                "memory mapping\n", ctx->dev_name);
            //exit(EXIT_FAILURE);
        } else {
            errno_exit(ctx->dev_name, "VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        ERR("%s: Insufficient buffer memory on %s\n", ctx->dev_name,
            ctx->dev_name);
        //exit(EXIT_FAILURE);
    }

    tmp_buffers = (VI_buffer_t*)calloc(req.count, sizeof(VI_buffer_t));

    if (!tmp_buffers) {
        ERR("%s: Out of memory\n", ctx->dev_name);
        //exit(EXIT_FAILURE);
    }

    ctx->buffers = tmp_buffers;

    for (ctx->n_buffers = 0; ctx->n_buffers < req.count; ++ctx->n_buffers) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);

        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = ctx->n_buffers;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd_tmp, VIDIOC_QUERYBUF, &buf))
            errno_exit(ctx->dev_name, "VIDIOC_QUERYBUF");

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            tmp_buffers[ctx->n_buffers].length = buf.m.planes[0].length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.planes[0].m.mem_offset);
        } else {
            tmp_buffers[ctx->n_buffers].length = buf.length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.offset);
        }

        if (MAP_FAILED == tmp_buffers[ctx->n_buffers].start)
            errno_exit(ctx->dev_name, "mmap");

        // export buf dma fd
        struct v4l2_exportbuffer expbuf;
        xcam_mem_clear (expbuf);
        expbuf.type = ctx->buf_type;
        expbuf.index = ctx->n_buffers;
        expbuf.flags = O_CLOEXEC;
        if (xioctl(fd_tmp, VIDIOC_EXPBUF, &expbuf) < 0) {
            errno_exit(ctx->dev_name, "get dma buf failed\n");
        } else {
            DBG("%s: get dma buf(%u)-fd: %d\n", ctx->dev_name, ctx->n_buffers, expbuf.fd);
        }
        tmp_buffers[ctx->n_buffers].export_fd = expbuf.fd;
    }
}

static void init_device(VI_stream_context_t *ctx)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    if (-1 == xioctl(ctx->fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            ERR("%s is no V4L2 device\n", ctx->dev_name);
            //exit(EXIT_FAILURE);
        } else {
            errno_exit(ctx->dev_name, "VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
            !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ERR("%s is not a video capture device, capabilities: %x\n",
            ctx->dev_name, cap.capabilities);
        //exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ERR("%s does not support streaming i/o\n", ctx->dev_name);
        //exit(EXIT_FAILURE);
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        if (ctx->width)
            fmt.fmt.pix.width = ctx->width;
        if (ctx->height)
            fmt.fmt.pix.height = ctx->height;
        if (ctx->format)
            fmt.fmt.pix.pixelformat = ctx->format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;
    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        if (ctx->width)
            fmt.fmt.pix_mp.width = ctx->width;
        if (ctx->height)
            fmt.fmt.pix_mp.height = ctx->height;
        if (ctx->format)
            fmt.fmt.pix_mp.pixelformat = ctx->format;
        fmt.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_FULL_RANGE;
    }

    if (ctx->width && ctx->height) {
        struct v4l2_selection in_selection;
        memset(&in_selection, 0, sizeof(in_selection));

        in_selection.target = V4L2_SEL_TGT_CROP;

        if (xioctl(ctx->fd, VIDIOC_G_SELECTION, &in_selection) == 0) {
            in_selection.r.width = ctx->width;
            in_selection.r.height = ctx->height;
            in_selection.r.left = 0;
            in_selection.r.top = 0;
            xioctl(ctx->fd, VIDIOC_S_SELECTION, &in_selection);
        }
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_S_FMT, &fmt))
        errno_exit(ctx->dev_name, "VIDIOC_S_FMT");

    init_mmap(ctx);
}

static void uninit_device(VI_stream_context_t *ctx)
{
    unsigned int i;
    if (ctx->n_buffers == 0)
        return;

    for (i = 0; i < ctx->n_buffers; ++i) {
        if (-1 == munmap(ctx->buffers[i].start, ctx->buffers[i].length))
            errno_exit(ctx->dev_name, "munmap");

        close(ctx->buffers[i].export_fd);
    }

    free(ctx->buffers);
    ctx->n_buffers = 0;
}

int8_t VI_stream_start(VI_stream_context_t *ctx)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < ctx->n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            struct v4l2_plane planes[FMT_NUM_PLANES];

            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, &buf)) {
            errno_exit(ctx->dev_name, "VIDIOC_QBUF");
            return -1;
        }
    }
    type = ctx->buf_type;
    DBG("%s:-------- stream on output -------------\n", ctx->dev_name);

    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMON, &type)) {
        errno_exit(ctx->dev_name, "VIDIOC_STREAMON");
        return -1;
    }
    
    return 0;
}

int8_t VI_stream_stop(VI_stream_context_t *ctx)
{
    enum v4l2_buf_type type;

    type = ctx->buf_type;
    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMOFF, &type)) {
        errno_exit(ctx->dev_name, "VIDIOC_STREAMOFF");
        return -1;
    }

    return 0;
}

VI_stream_context_t*
VI_stream_init(const char *vd_name, int width, int height,
               int fmt, bool user_poll)
{

    if (!vd_name) {
        return NULL;
    }

    VI_stream_context_t *ctx = NULL;
    ctx = (VI_stream_context_t *)malloc(sizeof(VI_stream_context_t));

    if (!ctx) {
        return NULL;
    }

    snprintf(ctx->dev_name, 64, "%s", vd_name);

    ctx->fd = open_device(vd_name, user_poll);
    if (ctx->fd == -1) {
        errno_exit(ctx->dev_name, "open error");
        return NULL;
    }

    ctx->width    = width;
    ctx->height   = height;
    ctx->format   = fmt;
    ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ctx->use_poll = user_poll;

    init_device(ctx);

    return ctx;
}

int8_t
VI_stream_deinit(VI_stream_context_t* ctx)
{

    if (!ctx) {
        return -1;
    }

    uninit_device(ctx);
    close_device(ctx->fd);

    free(ctx);
    return 0;
}

VI_buffer_t*
VI_streama_dequeue_buf(VI_stream_context_t* ctx)
{
    struct v4l2_buffer buf;
    VI_buffer_t *ret_buf = NULL;

    while (ctx->use_poll) {
        fd_set fds;
        struct timeval tv = { ctx->use_poll ? 2 : 0, 0 };
        int r = 0;

        FD_ZERO(&fds);
        FD_SET(ctx->fd, &fds);

        r = select(ctx->fd + 1, &fds, NULL, NULL, &tv);

        if (r == -1) {
            if (EINTR == errno)
                continue;
            ERR("select error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (r == 0) {
            ERR("select timeout\n");
            return NULL;
        }

        if (FD_ISSET(ctx->fd, &fds))
            break;
    }

    CLEAR(buf);

    buf.type = ctx->buf_type;
    buf.memory = V4L2_MEMORY_MMAP;

    struct v4l2_plane planes[FMT_NUM_PLANES];
    memset(planes, 0, sizeof(struct v4l2_plane)*FMT_NUM_PLANES);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_DQBUF, &buf)) {
        errno_exit(ctx->dev_name, "VIDIOC_DQBUF");
        return NULL;
    }

    ret_buf           = &(ctx->buffers[buf.index]);
    ret_buf->_buf     = buf;
    ret_buf->sequence = buf.sequence;

    return ret_buf;

}

int8_t
VI_streama_queue_buf(VI_stream_context_t* ctx, VI_buffer_t* buf) {

    struct v4l2_buffer v4l2_buf_s;
    struct v4l2_buffer* v4l2_buf = &v4l2_buf_s;

    struct v4l2_plane planes[FMT_NUM_PLANES];

    *v4l2_buf = buf->_buf;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == ctx->buf_type) {
        memset(planes, 0, sizeof(planes));
        v4l2_buf->m.planes = planes;
        planes[0]          = v4l2_buf->m.planes[0];
    }

    if (v4l2_buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        v4l2_buf->m.planes[0].bytesused = v4l2_buf->m.planes[0].length;
        v4l2_buf->m.planes[0].m.fd = buf->export_fd;
        v4l2_buf->m.planes[0].m.userptr = (unsigned long)buf->start;
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, v4l2_buf)) {
        errno_exit(ctx->dev_name, "VIDIOC_QBUF");
        return -1;
    }

    return 0;

}
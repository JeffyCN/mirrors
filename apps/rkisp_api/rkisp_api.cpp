#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#include <rkisp_control_loop.h>
#include <rkisp_dev_manager.h>
#include <interface/rkcamera_vendor_tags.h>
#include <mediactl.h>

#include "rkisp_api.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1 //TODO: support multi planes fmts

#define DEFAULT_WIDTH           640
#define DEFAULT_HEIGHT          480
#define DEFAULT_FCC             V4L2_PIX_FMT_NV12
#define DEFAULT_BUFFER_COUNT    4

#define IS_BAYER_RAW(mbus_code) ((mbus_code) >= MEDIA_BUS_FMT_SBGGR8_1X8 && \
                                 (mbus_code) < MEDIA_BUS_FMT_JPEG_1X8)

static int silent;

#define DBG(str, ...) do { if(!silent) printf("[DBG]%s:%d: " str, __func__, __LINE__, __VA_ARGS__); } while(0)
#define INFO(str, ...) do { printf("[INFO]%s:%d: " str, __func__, __LINE__, __VA_ARGS__); } while (0)
#define WARN(str, ...) do { printf("[WARN]%s:%d: " str, __func__, __LINE__, __VA_ARGS__); } while (0)
#define ERR(str, ...) do { fprintf(stderr, "[ERR]%s:%d: " str, __func__, __LINE__, __VA_ARGS__); } while (0)

#define FCC_TO_CHARS(fcc) \
            ((fcc) >> 0) & 0xFF, ((fcc) >> 8) & 0xFF, \
            ((fcc) >> 16) & 0xFF, ((fcc) >> 24) & 0xFF

struct rkisp_buf_priv {
    struct rkisp_api_buf pul;

    /* private data */
    int index;
};


struct control_params_3A
{
    /* used to receive current 3A settings and 3A states
     * place this at first place, so we cast ops back to base type
     */
    cl_result_callback_ops_t _result_cb_ops;
    /* used to set new setting to CL, used by _RKIspFunc.set_frame_params_func */
    rkisp_cl_frame_metadata_s _frame_metas;
    /* to manage the 3A settings, used by _frame_metas */
    CameraMetadata _settings_metadata;
    /* to manage the 3A result settings, used by metadata_result_callback */
    CameraMetadata _result_metadata;
    XCam::Mutex _meta_mutex;
};

enum {
    CAM_TYPE_RKISP1,
    CAM_TYPE_RKCIF,
    CAM_TYPE_USB,
};

struct rkisp_priv {
    struct rkisp_api_ctx ctx;

    /* Private data */
    enum v4l2_memory memory;
    enum v4l2_buf_type buf_type;
    int *dmabuf_fds;    /* From external dmabuf or export from isp driver */
    void **buf_mmap;    /* Only valid if MMAP */
    int buf_count;      /* the count actully requested from driver */
    int buf_length;

    int *req_dmabuf_fds;
    int req_buf_length; /* the count requested by app */
    int req_buf_count;

    struct rkisp_buf_priv *bufs;

    int camera_type;

    void* rkisp_engine;
    struct rkisp_media_info media_info;
    char mdev_path[64];
    camera_metadata_t* meta;
    struct control_params_3A* g_3A_control_params;
};

static const char * rkisp_get_active_sensor(const struct rkisp_priv *priv);
static int rkisp_get_media_topology(struct rkisp_priv *priv);
static int rkisp_init_engine(struct rkisp_priv *priv);
static void rkisp_deinit_engine(struct rkisp_priv *priv);
static void rkisp_stop_engine(struct rkisp_priv *priv);
static int rkisp_start_engine(struct rkisp_priv *priv);
static int rkisp_get_ae_time(struct rkisp_priv *priv, int64_t &time);
static int rkisp_get_ae_gain(struct rkisp_priv *priv, int &gain);
static int rkisp_get_meta_frame_id(struct rkisp_priv *priv, int64_t& frame_id);
static int rkisp_get_luminance_grid(struct rkisp_priv *, unsigned char *, int);
static int rkisp_get_histogram(struct rkisp_priv *, int *, int);

static int rkisp_get_fmt(const struct rkisp_api_ctx *ctx);

static int xioctl(int fh, int request, void *arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

static int rkisp_qbuf(struct rkisp_priv *priv, int buf_index)
{
    struct v4l2_plane planes[FMT_NUM_PLANES];
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = priv->buf_type;
    buf.memory = priv->memory;
    buf.index = buf_index;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type) {
        struct v4l2_plane planes[FMT_NUM_PLANES];
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;

        if (priv->memory == V4L2_MEMORY_DMABUF) {
            planes[0].m.fd = priv->dmabuf_fds[buf_index];
            planes[0].length = priv->buf_length;
        }
    } else {
        if (priv->memory == V4L2_MEMORY_DMABUF) {
            buf.m.fd = priv->dmabuf_fds[buf_index];
            buf.length = priv->buf_length;
        }
    }

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_QBUF, &buf)) {
        ERR("ERR QBUF: %d, %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

static void
rkisp_clr_dmabuf(struct rkisp_priv *priv)
{
    if (priv->dmabuf_fds)
        free(priv->dmabuf_fds);
    if (priv->req_dmabuf_fds)
        free(priv->req_dmabuf_fds);

    priv->req_dmabuf_fds = NULL;
    priv->dmabuf_fds = NULL;
    priv->req_buf_count = priv->req_buf_length = 0;
}

static int
rkisp_init_enq_allbuf(struct rkisp_priv *priv)
{
    int ret, i;

    for (i = 0; i < priv->buf_count; i++) {
        ret = rkisp_qbuf(priv, i);
        if (ret) {
            ERR("qbuf failed: %d, %s\n", errno, strerror(errno));
            return ret;
        }
    }

    return 0;
}

static int
rkisp_init_dmabuf(struct rkisp_priv *priv)
{
    struct v4l2_plane planes[FMT_NUM_PLANES];
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    int i, ret;

    CLEAR(req);
    req.count  = priv->req_buf_count;
    req.type   = priv->buf_type;
    req.memory = V4L2_MEMORY_DMABUF;

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_REQBUFS, &req)) {
        ERR("ERR REQBUFS: %d, %s\n", errno, strerror(errno));
        return -1;
    }

    CLEAR(buf);
    buf.type = priv->buf_type;
    buf.index = 0;
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type) {
        CLEAR(planes[0]);
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_QUERYBUF, &buf)) {
        ERR("ERR QUERYBUF: %d\n", errno);
        return -1;
    }
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type)
        priv->buf_length = planes[0].length;
    else
        priv->buf_length = buf.length;

    if (priv->buf_length > priv->req_buf_length) {
        ERR("ERR DMABUF size is smaller than desired, %d < %d\n",
            priv->req_buf_length, priv->buf_length);
        return -1;
    }
    priv->buf_length = priv->req_buf_length;

    priv->dmabuf_fds = (int *)calloc(req.count, sizeof(int));
    if (!priv->dmabuf_fds) {
        ERR("No memory, %d\n", errno);
        return -1;
    }

    for (i = 0; i < req.count; ++i) {
        priv->dmabuf_fds[i] = priv->req_dmabuf_fds[i];
    }
    priv->buf_count = req.count;

    return 0;
}

static void
rkisp_clr_mmap_buf(struct rkisp_priv *priv)
{
    int i;

    for (i = 0; i < priv->buf_count; i++) {
        if (priv->dmabuf_fds) {
            close(priv->dmabuf_fds[i]);
        }
        if (priv->buf_mmap) {
            munmap(priv->buf_mmap[i], priv->buf_length);
        }
    }

    if (priv->dmabuf_fds)
        free(priv->dmabuf_fds);
    if (priv->buf_mmap)
        free(priv->buf_mmap);
    priv->dmabuf_fds = NULL;
    priv->buf_mmap = NULL;
    priv->buf_length = 0;
}

static int rkisp_init_mmap(struct rkisp_priv *priv)
{
    struct v4l2_requestbuffers req;
    int i, j;

    CLEAR(req);
    req.count = priv->req_buf_count;
    req.type = priv->buf_type;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_REQBUFS, &req)) {
        ERR("ERR REQBUFS: %d, %s\n", errno, strerror(errno));
        return -1;
    }

    priv->dmabuf_fds = (int*) malloc(sizeof(int) * req.count);
    priv->buf_mmap = (void **) malloc(sizeof(void*) * req.count);
    if (!priv->dmabuf_fds || !priv->buf_mmap) {
        ERR("No memory, %d\n", errno);
        return -1;
    }

    for (i = 0; i < req.count; i++) {
        struct v4l2_plane planes[FMT_NUM_PLANES];
        struct v4l2_buffer buf;
        int offset;

        CLEAR(buf);
        CLEAR(planes);

        buf.type = priv->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (xioctl(priv->ctx.fd, VIDIOC_QUERYBUF, &buf) == -1) {
            ERR("QUERYBUF failed: %d, %s\n", errno, strerror(errno));
            goto unmap;
        }

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type) {
            priv->buf_length = planes[0].length;
            offset = planes[0].m.mem_offset;
        } else {
            priv->buf_length = buf.length;
            offset = buf.m.offset;
        }

        priv->buf_mmap[i] = mmap(NULL, priv->buf_length,
                                 PROT_READ | PROT_WRITE, MAP_SHARED,
                                 priv->ctx.fd, offset);
        if (MAP_FAILED == priv->buf_mmap[i]) {
            ERR("Mmap failed, %d, %s\n", errno, strerror(errno));
            goto unmap;
        }
    }

    for (j = 0; j < req.count; j++) {
        struct v4l2_exportbuffer expbuf;

        CLEAR(expbuf);
        expbuf.type = priv->buf_type;
        expbuf.index = j;
        expbuf.plane = 0;
        if (xioctl(priv->ctx.fd, VIDIOC_EXPBUF, &expbuf) == -1) {
            ERR("export buf failed: %d, %s\n", errno, strerror(errno));
            goto close_expfd;
        }
        priv->dmabuf_fds[j] = expbuf.fd;
        fcntl(expbuf.fd, F_SETFD, FD_CLOEXEC);
    }

    priv->buf_count = req.count;

    return 0;

close_expfd:
    while (j)
        close(priv->dmabuf_fds[--j]);
unmap:
    while (i)
        munmap(priv->buf_mmap[--i], priv->buf_length);

    free(priv->dmabuf_fds);
    free(priv->buf_mmap);
    priv->dmabuf_fds = NULL;
    priv->buf_mmap = NULL;

    return -1;
}

const struct rkisp_api_ctx*
rkisp_open_device(const char *dev_path, int uselocal3A)
{
    struct rkisp_priv *priv;
    struct v4l2_capability cap;

    priv = (struct rkisp_priv *)malloc(sizeof(*priv));
    if (!priv) {
        ERR("malloc fail, %d\n", errno);
        return NULL;
    }
    CLEAR(*priv);

    priv->ctx.fd = open(dev_path, O_RDWR | O_CLOEXEC, 0);
    if (-1 == priv->ctx.fd) {
        ERR("Cannot open '%s': %d, %s\n",
            dev_path, errno, strerror(errno));
        goto free_priv;
    }

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_QUERYCAP, &cap)) {
        ERR("%s ERR QUERYCAP, %d, %s\n", dev_path, errno, strerror(errno));
        goto err_close;
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        priv->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        priv->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    } else {
        ERR("%s is not a video capture device, capabilities: %x\n",
            dev_path, cap.capabilities);
        goto err_close;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ERR("%s does not support streaming i/o\n", dev_path);
        goto err_close;
    }

    strncpy(priv->ctx.dev_path, dev_path, sizeof(priv->ctx.dev_path));

    if (rkisp_get_media_topology(priv))
        goto err_close;

    if (uselocal3A) {
        if (priv->camera_type != CAM_TYPE_RKISP1) {
            ERR("rkcif(%s) is not supports local 3A\n", dev_path);
            goto err_close;
        }
        priv->ctx.uselocal3A = uselocal3A;
        /*
         * TODO: Signal SIGSTOP the rkisp_3A_server to stop 3A tuning.
         *       rkisp_3A_server shall clear old events after resume.
         */
        if (rkisp_init_engine(priv))
            goto err_close;
    }

    rkisp_get_fmt((struct rkisp_api_ctx*) priv);

    /* Set default value in case app does not call #{rkisp_set_buf()} */
    priv->req_buf_count = DEFAULT_BUFFER_COUNT;
    priv->memory = V4L2_MEMORY_MMAP;

    return (struct rkisp_api_ctx*) priv;

err_close:
    close(priv->ctx.fd);
    priv->ctx.fd = -1;
free_priv:
    free(priv);

    return NULL;
}

/* Any of fmt, memory type, or buf count changed shall re-request buf */
static void
rkisp_clr_buf(struct rkisp_priv *priv)
{
    struct v4l2_requestbuffers req;

    /* There is not buffer initilized by REQBUF */
    if (!priv->buf_count)
        return;

    if (priv->memory == V4L2_MEMORY_DMABUF)
        rkisp_clr_dmabuf(priv);
    else if (priv->memory  == V4L2_MEMORY_MMAP)
        rkisp_clr_mmap_buf(priv);

    CLEAR(req);
    req.count  = 0;
    req.type   = priv->buf_type;
    req.memory = priv->memory;

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_REQBUFS, &req)) {
        ERR("ERR CLR(REQ 0) BUF: %d, %s\n", errno, strerror(errno));
    }

    if (priv->bufs)
        free(priv->bufs);
    priv->bufs = NULL;

    priv->buf_count = 0;
}

static int rkisp_init_buf(struct rkisp_priv *priv)
{
    int ret;

    if (priv->buf_count) {
        ERR("BUG: REQBUF has been called, %s\n", __func__);
        return -1;
    }

    if (priv->memory == V4L2_MEMORY_MMAP)
        ret = rkisp_init_mmap(priv);
    else if (priv->memory == V4L2_MEMORY_DMABUF)
        ret = rkisp_init_dmabuf(priv);
    if (ret)
        return ret;

    /* Create local buffers */
    priv->bufs = (struct rkisp_buf_priv*) calloc(priv->buf_count,
                                                 sizeof(struct rkisp_buf_priv));
    if (!priv->bufs) {
        ERR("no memory, %d\n", errno);
        return -1;
    }

    return 0;
}

static int rkisp_get_fmt(const struct rkisp_api_ctx *ctx)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct v4l2_format fmt;

    CLEAR(fmt);
    fmt.type = priv->buf_type;
    if (-1 == xioctl(priv->ctx.fd, VIDIOC_G_FMT, &fmt)) {
        ERR("%s ERR S_FMT: %d, %s\n", priv->ctx.dev_path,
	    errno, strerror(errno));
        return -errno;
    }

    priv->ctx.width = fmt.fmt.pix.width;
    priv->ctx.height = fmt.fmt.pix.height;
    priv->ctx.fcc = fmt.fmt.pix.pixelformat;

    INFO("Get Driver default fmt: fcc %C%C%C%C [%dx%d]\n",
         FCC_TO_CHARS(priv->ctx.fcc), priv->ctx.width, priv->ctx.height);

    return 0;
}

int
rkisp_set_fmt(const struct rkisp_api_ctx *ctx, int w, int h, int fcc)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct v4l2_format fmt;

    if (priv->ctx.width == fmt.fmt.pix.width &&
        priv->ctx.height == fmt.fmt.pix.height &&
        priv->ctx.fcc == fmt.fmt.pix.pixelformat)
        return 0;

    /* Clear buffer if any. Driver requirs not buffer queued before set_fmt */
    rkisp_clr_buf(priv);

    CLEAR(fmt);
    fmt.type = priv->buf_type;
    fmt.fmt.pix.width = w;
    fmt.fmt.pix.height = h;
    fmt.fmt.pix.pixelformat = fcc;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_S_FMT, &fmt)) {
        ERR("%s ERR S_FMT: %d, %s\n", priv->ctx.dev_path,
	    errno, strerror(errno));
        return -errno;
    }

    priv->ctx.width = fmt.fmt.pix.width;
    priv->ctx.height = fmt.fmt.pix.height;
    priv->ctx.fcc = fmt.fmt.pix.pixelformat;

    if (priv->ctx.width != w || priv->ctx.height != h || priv->ctx.fcc != fcc)
        WARN("Format is not match, request: fcc %C%C%C%C [%dx%d], "
             "driver supports: fcc %C%C%C%C [%dx%d]\n", FCC_TO_CHARS(fcc), w, h,
             FCC_TO_CHARS(priv->ctx.fcc), priv->ctx.width, priv->ctx.height);

    return 0;
}

int
rkisp_set_sensor_fmt(const struct rkisp_api_ctx *ctx, int w, int h, int code)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct v4l2_subdev_format fmt;
    const char *sensor;
    int ret, fd;

    if (priv->camera_type == CAM_TYPE_USB)
        return -EINVAL;

    sensor = rkisp_get_active_sensor(priv);
    if (!sensor)
        return -EINVAL;
    fd = open(sensor, O_RDWR | O_CLOEXEC, 0);
    if (fd < 0) {
       ERR("Open sensor subdev %s failed, %s\n", sensor, strerror(errno));
       return fd;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = 0; //TODO: It's not definite that pad always be 0
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret = ioctl(fd, VIDIOC_SUBDEV_G_FMT, &fmt);
    if (ret < 0) {
        ERR("get sensor fmt failed %s.\n", strerror(errno));
        goto close;
    }

    fmt.format.height = h;
    fmt.format.width = w;
    fmt.format.code = code;

    ret = ioctl(fd, VIDIOC_SUBDEV_S_FMT, &fmt);
    if (ret < 0) {
        ERR("subdev %s set fmt failed, %s. Does sensor driver support set_fmt?\n",
            sensor, strerror(errno));
        goto close;
    }

    if (fmt.format.width != w || fmt.format.height != h ||
        fmt.format.code != code) {
        INFO("subdev %s choose the best fit fmt: %dx%d, 0x%08x\n",
             sensor, fmt.format.width, fmt.format.height, fmt.format.code);
    }

    /* If ispsd input size is larger than sensor, update pipeline with default value */
    if (priv->camera_type == CAM_TYPE_RKISP1 &&
        strlen(priv->media_info.sd_isp_path) != 0) {
        struct v4l2_subdev_format isp_fmt;
        int isp_fd;

        isp_fd = open(priv->media_info.sd_isp_path, O_RDWR | O_CLOEXEC, 0);
        if (isp_fd < 0) {
            ERR("Open isp subdev failed, %s\n", strerror(errno));
            goto close;
        }

        memset(&isp_fmt, 0, sizeof(isp_fmt));
        isp_fmt.pad = 0; //TODO isp sd sink pad always be 0?
        isp_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        ret = ioctl(isp_fd, VIDIOC_SUBDEV_G_FMT, &isp_fmt);
        if (ret < 0) {
            ERR("isp subdev get fmt failed %s.\n", strerror(errno));
            close(isp_fd);
            goto close;
        }
        close(isp_fd);

        if (isp_fmt.format.width > fmt.format.width ||
            isp_fmt.format.height > fmt.format.height ||
            isp_fmt.format.code != fmt.format.code) {
                int ispsd_out_code, width, height;

                WARN("isp subdev fmt(%dx%d) > fmt(%dx%d), or mbus code(0x%08x) != 0x%08x\n",
                     isp_fmt.format.width, isp_fmt.format.height,
                     fmt.format.width, fmt.format.height,
                     isp_fmt.format.code, fmt.format.code);
                WARN("Update isp pipeline accordeing to sensor settings, "
                     "PLEASE DOUBLE CHECK with `medai-ctl -p -d %s`\n",
                     priv->mdev_path);
                if (IS_BAYER_RAW(fmt.format.code))
                    ispsd_out_code = MEDIA_BUS_FMT_YUYV8_2X8;
                else
                    ispsd_out_code = fmt.format.code;

                width = fmt.format.width;
                height = fmt.format.height;

                ret = rkisp_set_ispsd_fmt(ctx, width, height, fmt.format.code,
                                          width, height, ispsd_out_code);
                if (ret)
                    goto close;
        }
    }

close:
    close(fd);
    return ret;
}

int
rkisp_video_set_crop(const struct rkisp_priv *priv, int x, int y, int w, int h)
{
    struct v4l2_selection sel;
    int ret;

    memset(&sel, 0, sizeof(sel));
    sel.type = priv->buf_type;
    sel.r.width = w;
    sel.r.height = h;
    sel.r.left = x;
    sel.r.top = y;
    sel.target = V4L2_SEL_TGT_CROP;
    sel.flags = 0;
    ret = ioctl(priv->ctx.fd, VIDIOC_S_SELECTION, &sel);
    if (ret) {
        ERR("set output crop(0,0/%dx%d) failed, %s\n", w, h, strerror(errno));
        return ret;
    }

    return 0;
}

static int
rkisp_sd_set_crop(const char *ispsd, int fd, int pad, int *w, int *h)
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
    ret = ioctl(fd, VIDIOC_SUBDEV_S_SELECTION, &sel);
    if (ret) {
        ERR("subdev %s pad %d crop failed, ret = %d\n", ispsd, sel.pad, ret);
        return ret;
    }

    *w = sel.r.width;
    *h = sel.r.height;

    return 0;
}

static int
rkisp_sd_set_fmt(const char *ispsd, int pad, int *w, int *h, int code)
{
    struct v4l2_subdev_format fmt;
    int ret, fd;

    fd = open(ispsd, O_RDWR | O_CLOEXEC, 0);
    if (fd < 0) {
       ERR("Open isp subdev %s failed, %s\n", ispsd, strerror(errno));
       return fd;
    }

    if (pad == 2) { /* Source pad */
        ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
        if (ret)
            goto close;
    }

    /* Get fmt and only update what we want */
    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = pad;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret = ioctl(fd, VIDIOC_SUBDEV_G_FMT, &fmt);
    if (ret < 0) {
        ERR("subdev %s get pad: %d fmt failed %s.\n",
            ispsd, fmt.pad, strerror(errno));
        goto close;
    }

    fmt.format.height = *h;
    fmt.format.width = *w;
    fmt.format.code = code;

    ret = ioctl(fd, VIDIOC_SUBDEV_S_FMT, &fmt);
    if (ret < 0) {
        ERR("subdev %s set pad: %d fmt failed %s.\n",
            ispsd, fmt.pad, strerror(errno));
        goto close;
    }

    if (fmt.format.width != *w || fmt.format.height != *h ||
        fmt.format.code != code) {
        INFO("subdev %s pad %d choose the best fit fmt: %dx%d, 0x%08x\n",
             ispsd, pad, fmt.format.width, fmt.format.height, fmt.format.code);
    }

    *w = fmt.format.width;
    *h = fmt.format.height;
    if (pad == 0) {
        ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
        if (ret)
            goto close;
    }

close:
    close(fd);

    return ret;
}

int
rkisp_set_ispsd_fmt(const struct rkisp_api_ctx *ctx,
                    int in_w, int in_h, int in_code,
                    int out_w, int out_h, int out_code)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    const char *ispsd;
    int ret;

    if (priv->camera_type != CAM_TYPE_RKISP1)
        return -EINVAL;

    ispsd = priv->media_info.sd_isp_path;
    //TODO: check source and sink pad
    ret = rkisp_sd_set_fmt(ispsd, 0, &in_w, &in_h, in_code);
    ret |= rkisp_sd_set_fmt(ispsd, 2, &out_w, &out_h, out_code);

    /* set video selection(crop) because ispsd size changed */
    ret |= rkisp_video_set_crop(priv, 0, 0, out_w, out_h);

    return ret;
}

int
rkisp_set_buf(const struct rkisp_api_ctx *ctx, int buf_count,
              const int dmabuf_fds[], int dmabuf_size)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    int i;

    if (buf_count < 2) {
        ERR("buf count shall be >= 2, current: %d\n", buf_count);
        return -1;
    }

    if (dmabuf_fds) { /* The target memory type is DMABUF */
        int size = sizeof(int) * buf_count;

        if (dmabuf_size <= 0) {
            ERR("dmabuf_size[] can't be (%d) if dmabuf_fds is not NULL\n", dmabuf_size);
            return -1;
        }
        if (priv->memory == V4L2_MEMORY_MMAP || priv->req_buf_count != buf_count)
            rkisp_clr_buf(priv);
        if (priv->req_dmabuf_fds && memcmp(dmabuf_fds, priv->req_dmabuf_fds, size))
            /* dmabuf fd different with privious fd */
            rkisp_clr_buf(priv);

        priv->req_dmabuf_fds = (int *)calloc(buf_count, sizeof(int));
        if (!priv->req_dmabuf_fds) {
            ERR("Not memory, req_dmabuf_fds size: %d\n", size);
            return -1;
        }

        memcpy(priv->req_dmabuf_fds, dmabuf_fds, size);

        priv->memory = V4L2_MEMORY_DMABUF;
        priv->req_buf_length = dmabuf_size;
        priv->req_buf_count = buf_count;
    } else { /* The target memory type is MMAP */
        if (priv->memory == V4L2_MEMORY_DMABUF ||
            priv->req_buf_count != buf_count)
            rkisp_clr_buf(priv);

        priv->memory = V4L2_MEMORY_MMAP;
        priv->req_buf_count = buf_count;
    }

    return 0;
}

int
rkisp_start_capture(const struct rkisp_api_ctx *ctx)
{
    struct rkisp_priv *priv = (struct rkisp_priv*) ctx;
    enum v4l2_buf_type type;
    int ret;

    if (priv->ctx.uselocal3A && (ret = rkisp_start_engine(priv)))
        return ret;

    /*
     * Initialize buffers if first startup or buffers are cleared.
     * Any of fmt changed, memory type changed, buffer or buf_count changed
     * could clear buffers.
     */
    if (priv->buf_count == 0 && (ret = rkisp_init_buf(priv)))
        goto deinit;

    /*
     * Previous STREAMOFF restored buffer state to
     * starting state as after calling REQBUF.
     */
    if (ret = rkisp_init_enq_allbuf(priv))
        goto deinit;

    type = priv->buf_type;
    if (ret = xioctl(priv->ctx.fd, VIDIOC_STREAMON, &type)) {
        ERR("ERR STREAMON, %d\n", errno);
        rkisp_clr_buf(priv);
        goto deinit;
    }

    return 0;

deinit:
    if (priv->ctx.uselocal3A)
        rkisp_stop_engine(priv);

    return ret;
}

const struct rkisp_api_buf*
rkisp_get_frame(const struct rkisp_api_ctx *ctx, int timeout_ms)
{
    struct rkisp_priv *priv = (struct rkisp_priv*) ctx;
    struct v4l2_plane planes[FMT_NUM_PLANES];
    struct rkisp_buf_priv* buffer;
    struct v4l2_buffer buf;

    if (timeout_ms > 0) {
        fd_set fds;
        struct timeval tv;
        int retval;

        FD_ZERO(&fds);
        FD_SET(priv->ctx.fd, &fds);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        while (timeout_ms > 0) {
            retval = select(priv->ctx.fd + 1, &fds, NULL, NULL, &tv);
            if (retval == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    ERR("select() return error: %s\n", strerror(errno));
                    return NULL;
                }
            } else if (retval) {
                /* Data ready, FD_ISSET(0, &fds) will be true. */
                break;
            } else {
                /* timeout */
                return NULL;
            }
        }
    }

    CLEAR(buf);
    buf.type = priv->buf_type;
    buf.memory = priv->memory;
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == xioctl(priv->ctx.fd, VIDIOC_DQBUF, &buf)) {
        ERR("ERR DQBUF: %d\n", errno);
        return NULL;
    }

    buffer = &priv->bufs[buf.index];
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == priv->buf_type)
        buffer->pul.size = buf.m.planes[0].bytesused;
    else
        buffer->pul.size = buf.bytesused;
    buffer->pul.next_plane = NULL;

    if (priv->memory == V4L2_MEMORY_DMABUF) {
        buffer->pul.fd = priv->dmabuf_fds[buf.index];
        buffer->pul.buf = NULL;
    } else if (priv->memory == V4L2_MEMORY_MMAP) {
        buffer->pul.fd = priv->dmabuf_fds[buf.index];
        buffer->pul.buf = priv->buf_mmap[buf.index];
    }

    buffer->index = buf.index;
    buffer->pul.timestamp = buf.timestamp;

    if (priv->ctx.uselocal3A && priv->rkisp_engine) {
        rkisp_get_ae_time(priv, buffer->pul.metadata.expo_time);
        rkisp_get_ae_gain(priv, buffer->pul.metadata.gain);
        rkisp_get_meta_frame_id(priv, buffer->pul.metadata.frame_id);
        buffer->pul.metadata.luminance_grid_count = rkisp_get_luminance_grid(priv,
           buffer->pul.metadata.luminance_grid, RKISP_MAX_LUMINANCE_GRID);
        buffer->pul.metadata.hist_bins_count = rkisp_get_histogram(priv,
           buffer->pul.metadata.hist_bins, RKISP_MAX_HISTOGRAM_BIN);
    }

    return (struct rkisp_api_buf*)buffer;
}

void
rkisp_put_frame(const struct rkisp_api_ctx *ctx,
                const struct rkisp_api_buf *buf)
{
    struct rkisp_buf_priv *buffer;

    buffer = (struct rkisp_buf_priv *) buf;
    rkisp_qbuf((struct rkisp_priv *)ctx, buffer->index);
}

void rkisp_stop_capture(const struct rkisp_api_ctx *ctx)
{
    struct rkisp_priv *priv = (struct rkisp_priv*) ctx;
    enum v4l2_buf_type type;

    if (priv->ctx.uselocal3A && priv->rkisp_engine)
        rkisp_stop_engine(priv);

    type = priv->buf_type;
    if (-1 == xioctl(priv->ctx.fd, VIDIOC_STREAMOFF, &type)) {
        ERR("ERR STREAMOFF: %d\n", errno);
    }
}

void rkisp_close_device(const struct rkisp_api_ctx *ctx)
{
    struct rkisp_priv *priv = (struct rkisp_priv*) ctx;

    rkisp_clr_buf(priv);

    if (-1 == close(priv->ctx.fd))
        ERR("ERR close, %d\n", errno);

    if (priv->ctx.uselocal3A && priv->rkisp_engine)
        rkisp_deinit_engine(priv);

    free(priv);
}

/////////////   RKISP 3A
//
//

/*
 * construct the default camera settings, including the static
 * camera capabilities, infos.
 */
static void construct_default_metas(CameraMetadata* metas)
{
    int32_t fps_range[2] = {0, 120}; /* 120fps */
    metas->update(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, fps_range, 2);
    metas->update(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, fps_range, 2);

    int64_t exptime_range_ns[2] = {0, 200 * 1000 * 1000}; /* 200ms */
    metas->update(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE, exptime_range_ns, 2);

    int32_t sensitivity_range[2] = {0, 100 * 100};
    metas->update(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE, sensitivity_range, 2);

    uint8_t ae_mode = ANDROID_CONTROL_AE_MODE_ON;
    metas->update(ANDROID_CONTROL_AE_MODE, &ae_mode, 1);

    int64_t expo_time_ns = 10 * 1000 * 1000;
    metas->update(ANDROID_SENSOR_EXPOSURE_TIME, &expo_time_ns, 1);

    uint8_t control_mode = ANDROID_CONTROL_MODE_AUTO;
    metas->update(ANDROID_CONTROL_MODE, &control_mode, 1);

    int32_t sensitivity = 1600;
    metas->update(ANDROID_SENSOR_SENSITIVITY, &sensitivity, 1);

    uint8_t ae_lock = ANDROID_CONTROL_AE_LOCK_OFF;
    metas->update(ANDROID_CONTROL_AE_LOCK, &ae_lock, 1);
}

static void
rkisp_metadata_result_cb(const struct cl_result_callback_ops *ops,
                         struct rkisp_cl_frame_metadata_s *result)
{
    struct control_params_3A* ctl_params = (struct control_params_3A*)ops;

    SmartLock lock(ctl_params->_meta_mutex);
    /* this will clone results to _result_metadata */
    ctl_params->_result_metadata = result->metas;
}

static int rkisp_get_meta_frame_id(struct rkisp_priv *priv, int64_t& frame_id) {
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID);
    if (!entry.count) {
        DBG("no RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID, %lu\n", entry.count);
        frame_id = -1;
        return -1;
    }
    frame_id = entry.data.i64[0];

    return 0;
}

/* time in ns */
static int rkisp_get_ae_time(struct rkisp_priv *priv, int64_t &time)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_EXPOSURE_TIME);
    if (!entry.count)
        return -1;

    time = entry.data.i64[0];

    return 0;
}

static int rkisp_getAeMaxExposureTime(struct rkisp_priv *priv, int64_t &time)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (entry.count != 2)
        return -1;

    time = entry.data.i64[1];

    return 0;
}

static int rkisp_get_ae_gain(struct rkisp_priv *priv, int &gain)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_SENSITIVITY);
    if (!entry.count)
        return -1;

    gain = entry.data.i32[0];

    return 0;
}

static int rkisp_getAeMaxExposureGain(struct rkisp_priv *priv, int &gain)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
    if (!entry.count)
        return -1;

    gain = entry.data.i32[1];

    return 0;
}

static int rkisp_get_histogram(struct rkisp_priv *priv, int *hist, int size)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry, count;

    SmartLock lock(ctl_params->_meta_mutex);

    count = ctl_params->_result_metadata.find(ANDROID_STATISTICS_INFO_MAX_HISTOGRAM_COUNT);
    entry = ctl_params->_result_metadata.find(ANDROID_STATISTICS_HISTOGRAM);
    if (!entry.count || !count.count)
        return -1;

    if (count.data.i32[0] > size) {
        ERR("size of array [%d] < target size [%d]\n", size, count.data.i32[0]);
        return -1;
    }

    memcpy(hist, entry.data.i32, sizeof(int) * count.data.i32[0]);

    return count.data.i32[0];
}

static int
rkisp_get_luminance_grid(struct rkisp_priv *priv, unsigned char *luma, int size)
{
    struct control_params_3A* ctl_params = priv->g_3A_control_params;
    camera_metadata_entry entry, count;

    SmartLock lock(ctl_params->_meta_mutex);

    count = ctl_params->_result_metadata.find(RKCAMERA3_PRIVATEDATA_EXP_MEANS_COUNT);
    entry = ctl_params->_result_metadata.find(RKCAMERA3_PRIVATEDATA_EXP_MEANS);
    if (!entry.count || !count.count)
        return -1;

    if (count.data.i32[0] > size) {
        ERR("size of array [%d] < target size [%d]\n", size, count.data.i32[0]);
        return -1;
    }

    memcpy(luma, entry.data.u8, sizeof(entry.data.u8[0]) * count.data.i32[0]);

    return count.data.i32[0];
}


static int init_3A_control_params(struct rkisp_priv *priv)
{
    priv->meta = allocate_camera_metadata(DEFAULT_ENTRY_CAP, DEFAULT_DATA_CAP);
    if (!priv->meta)
        return -1;

    priv->g_3A_control_params = new control_params_3A();
    if (!priv->g_3A_control_params)
        return -1;

    priv->g_3A_control_params->_result_cb_ops.metadata_result_callback = rkisp_metadata_result_cb;
    priv->g_3A_control_params->_settings_metadata = priv->meta;
    //construct_default_metas(&priv->g_3A_control_params->_settings_metadata);
    priv->g_3A_control_params->_frame_metas.id = 0;
    priv->g_3A_control_params->_frame_metas.metas = priv->g_3A_control_params->_settings_metadata.getAndLock();
    priv->g_3A_control_params->_settings_metadata.unlock(priv->g_3A_control_params->_frame_metas.metas);

    return 0;
}

static void deinit_3A_control_params(struct rkisp_priv *priv)
{
    if (priv->g_3A_control_params )
        delete priv->g_3A_control_params ;
    priv->g_3A_control_params = NULL;

    if (priv->meta)
        free_camera_metadata(priv->meta);
    priv->meta = NULL;
}

static int rkisp_init_engine(struct rkisp_priv *priv)
{
    struct rkisp_cl_prepare_params_s params = {0};
    int ret = 0;

    if (ret = init_3A_control_params(priv)) {
        ERR("Not memory, ret = %d", ret);
        return ret;
    }

    if (ret = rkisp_cl_init(&priv->rkisp_engine, NULL, (cl_result_callback_ops_t*)priv->g_3A_control_params)) {
        ERR("rkisp engine init failed!, ret = %d\n", ret);
        goto deinit_params;
    }

    params.isp_sd_node_path = priv->media_info.sd_isp_path;
    params.isp_vd_params_path = priv->media_info.vd_params_path;
    params.isp_vd_stats_path = priv->media_info.vd_stats_path;

    for (int i = 0; i < RKISP_CAMS_NUM_MAX; i++) {
        if (!priv->media_info.cams[i].link_enabled) {
            continue;
        }

        DBG("%s: link enabled %d\n", priv->media_info.cams[i].sd_sensor_path,
            priv->media_info.cams[i].link_enabled);

        params.sensor_sd_node_path = priv->media_info.cams[i].sd_sensor_path;

        if (strlen(priv->media_info.cams[i].sd_lens_path))
            params.lens_sd_node_path = priv->media_info.cams[i].sd_lens_path;
        if (strlen(priv->media_info.cams[i].sd_flash_path[0]))
            params.flashlight_sd_node_path[0] = priv->media_info.cams[i].sd_flash_path[0];

        break;
    }

    params.staticMeta = priv->g_3A_control_params->_settings_metadata.getAndLock();
    if (ret = rkisp_cl_prepare(priv->rkisp_engine, &params)) {
        ERR("rkisp engine prepare failed! ret = %d\n", ret);
        priv->g_3A_control_params->_settings_metadata.unlock(params.staticMeta);
        goto deinit_cl;
    }
    priv->g_3A_control_params->_settings_metadata.unlock(params.staticMeta);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &priv->g_3A_control_params->_frame_metas);

    return 0;

deinit_cl:
    rkisp_cl_deinit(priv->rkisp_engine);
deinit_params:
    deinit_3A_control_params(priv);

    return ret;
}

static int rkisp_start_engine(struct rkisp_priv *priv)
{
    int ret;
    ret = rkisp_cl_start(priv->rkisp_engine);
    if (ret || priv->rkisp_engine == NULL) {
        ERR("rkisp_init engine failed, engine = %p\n", priv->rkisp_engine);
        return -1;
    }
    return 0;
}

static void rkisp_stop_engine(struct rkisp_priv *priv)
{
    rkisp_cl_stop(priv->rkisp_engine);
}

static void rkisp_deinit_engine(struct rkisp_priv *priv)
{
    rkisp_cl_deinit(priv->rkisp_engine);
    deinit_3A_control_params(priv);
    priv->rkisp_engine = NULL;
}

static const char * rkisp_get_active_sensor(const struct rkisp_priv *priv)
{
    int i;

    for (i = 0; i < RKISP_CAMS_NUM_MAX; i++)
        if (priv->media_info.cams[i].link_enabled)
            break;
    if (i == RKISP_CAMS_NUM_MAX) {
        ERR("Not sensor linked for %s, make sure sensor probed correctly\n",
            priv->ctx.dev_path);
        return NULL;
    }

    return priv->media_info.cams[i].sd_sensor_path;
}

static int rkisp_get_media_topology(struct rkisp_priv *priv)
{
    char mdev_path[64];
    int i, ret;

    for (i = 0; ; i++) {
        sprintf(mdev_path, "/dev/media%d", i);
        INFO("Get media device: %s info\n", mdev_path);

        ret = rkisp_get_media_info(&priv->media_info, mdev_path);

        if (ret == -ENOENT || ret == -ENOMEM)
            break;
        else if (ret != 0) /* The device could be other media device */
            continue;

        if (!strcmp(priv->media_info.vd_main_path, priv->ctx.dev_path) ||
            !strcmp(priv->media_info.vd_self_path, priv->ctx.dev_path)) {
            INFO("Get rkisp media device: %s info, done\n", mdev_path);
            priv->camera_type = CAM_TYPE_RKISP1;
            break;
        } else if (!strcmp(priv->media_info.vd_cif_path, priv->ctx.dev_path)) {
            INFO("Get rkcif media device: %s info, done\n", mdev_path);
            priv->camera_type = CAM_TYPE_RKCIF;
            break;
        }
    }

    if (ret) {
        INFO("The %s could be a USB camera.\n", priv->ctx.dev_path);
        priv->camera_type = CAM_TYPE_USB;
        priv->mdev_path[0] = '\0';
        return 0;
    }

    strcpy(priv->mdev_path, mdev_path);

    if (!rkisp_get_active_sensor(priv)) {
        ERR("%s is RKISP1 or RKCIF device but not sensor attached\n",
            priv->ctx.dev_path);
        return -1;
    }

    return ret;
}

int
rkisp_set_manual_expo(const struct rkisp_api_ctx *ctx, int on)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    uint8_t ae_mode;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    ctl_params = priv->g_3A_control_params;
    if (on) {
        camera_metadata_entry entry;

        ae_mode = ANDROID_CONTROL_AE_MODE_OFF;
        SmartLock lock(ctl_params->_meta_mutex);

        entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_EXPOSURE_TIME);
        if (!entry.count)
            return -1;
        ctl_params->_settings_metadata.update(ANDROID_SENSOR_EXPOSURE_TIME, entry.data.i64, 1);

        entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_SENSITIVITY);
        if (!entry.count)
            return -1;
        ctl_params->_settings_metadata.update(ANDROID_SENSOR_SENSITIVITY, entry.data.i32, 1);

        ctl_params->_settings_metadata.update(ANDROID_CONTROL_AE_MODE, &ae_mode, 1);
    } else {
        ae_mode = ANDROID_CONTROL_AE_MODE_ON;
        ctl_params->_settings_metadata.update(ANDROID_CONTROL_AE_MODE, &ae_mode, 1);
    }
    ctl_params->_frame_metas.id++;
    ctl_params->_frame_metas.metas = ctl_params->_settings_metadata.getAndLock();
    ctl_params->_settings_metadata.unlock(ctl_params->_frame_metas.metas);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &ctl_params->_frame_metas);

    return 0;
}

int
rkisp_update_expo(const struct rkisp_api_ctx *ctx, int gain, int64_t expo_time_ns)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    uint8_t ae_mode = ANDROID_CONTROL_AE_MODE_OFF;
    int32_t sensitivity = gain;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    ctl_params = priv->g_3A_control_params;

    ctl_params->_settings_metadata.update(ANDROID_SENSOR_SENSITIVITY, &sensitivity, 1);
    ctl_params->_settings_metadata.update(ANDROID_CONTROL_AE_MODE, &ae_mode, 1);
    ctl_params->_settings_metadata.update(ANDROID_SENSOR_EXPOSURE_TIME, &expo_time_ns, 1);
    ctl_params->_frame_metas.id++;
    ctl_params->_frame_metas.metas = ctl_params->_settings_metadata.getAndLock();
    ctl_params->_settings_metadata.unlock(ctl_params->_frame_metas.metas);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &ctl_params->_frame_metas);

    return 0;
}

int
rkisp_set_max_expotime(const struct rkisp_api_ctx *ctx, int64_t max_expo_time_ns)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    int64_t exptime_range_ns[2] = {0};

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    ctl_params = priv->g_3A_control_params;

    exptime_range_ns[1] = max_expo_time_ns;
    ctl_params->_settings_metadata.update(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE,
                                          exptime_range_ns, 2);
    // should update new settings id
    ctl_params->_frame_metas.id++;
    ctl_params->_frame_metas.metas = ctl_params->_settings_metadata.getAndLock();
    ctl_params->_settings_metadata.unlock(ctl_params->_frame_metas.metas);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &ctl_params->_frame_metas);

    return 0;
}

int
rkisp_get_max_expotime(const struct rkisp_api_ctx *ctx, int64_t *max_expo_time)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    camera_metadata_entry entry;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (entry.count != 2)
        return -1;

    *max_expo_time = entry.data.i64[1];

    return 0;
}

int
rkisp_set_max_gain(const struct rkisp_api_ctx *ctx, int max_gain)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    int32_t sensitivity_range[2] = {0,0};

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    ctl_params = priv->g_3A_control_params;

    sensitivity_range[1] = max_gain;
    ctl_params->_settings_metadata.update(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE,
                                          sensitivity_range, 2);
    // should update new settings id
    ctl_params->_frame_metas.id++;
    ctl_params->_frame_metas.metas =
        ctl_params->_settings_metadata.getAndLock();
    ctl_params->_settings_metadata.unlock(ctl_params->_frame_metas.metas);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &ctl_params->_frame_metas);

    return 0;
}

int
rkisp_get_max_gain(const struct rkisp_api_ctx *ctx, int *max_gain)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    camera_metadata_entry entry;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    SmartLock lock(ctl_params->_meta_mutex);

    entry = ctl_params->_result_metadata.find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
    if (!entry.count)
        return -1;

    *max_gain = entry.data.i32[1];

    return 0;
}

int
rkisp_get_expo_weights(const struct rkisp_api_ctx *ctx,
                       unsigned char* weights, unsigned int size)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    if (size != RKISP_EXPO_WEIGHT_GRID) {
        ERR("The weights array size shall be %d at least, but got %d\n",
            RKISP_EXPO_WEIGHT_GRID, size);
        return -EINVAL;
    }

    //TODO: warning if streamoff
    //AEC weights are available after rkisp_cl_prepare(),
    //for our case, after rkisp_open_device().
    rkisp_get_aec_weights(weights, &size);

    return 0;
}

int
rkisp_set_expo_weights(const struct rkisp_api_ctx *ctx,
                       unsigned char* weights, unsigned int size)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    if (size != RKISP_EXPO_WEIGHT_GRID) {
        ERR("The weights array size should be %d\n", RKISP_EXPO_WEIGHT_GRID);
        return -EINVAL;
    }

    //TODO: warning if streamoff
    rkisp_set_aec_weights(weights, size);

    return 0;
}

int
rkisp_set_fps_range(const struct rkisp_api_ctx *ctx, int max_fps)
{
    struct rkisp_priv *priv = (struct rkisp_priv *) ctx;
    struct control_params_3A* ctl_params;
    int32_t fps_range[2] = {1, 120};

    if (!priv->ctx.uselocal3A || !priv->rkisp_engine)
        return -EINVAL;

    ctl_params = priv->g_3A_control_params;

    fps_range[1] = max_fps;
    ctl_params->_settings_metadata.update(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, fps_range, 2);
    ctl_params->_settings_metadata.update(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, fps_range, 2);

    // should update new settings id
    ctl_params->_frame_metas.id++;
    ctl_params->_frame_metas.metas = ctl_params->_settings_metadata.getAndLock();
    ctl_params->_settings_metadata.unlock(ctl_params->_frame_metas.metas);

    rkisp_cl_set_frame_params(priv->rkisp_engine, &ctl_params->_frame_metas);

    return 0;
}

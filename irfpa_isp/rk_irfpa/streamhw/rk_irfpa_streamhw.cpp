#include "rk_irfpa_streamhw.h"
#include "ev_cpp.h"
#include "cv4l-helpers.h"
#include <list>
#include <map>
#include <string>
extern "C" {
#include "mediactl.h"  // C语言媒体设备控制库
}

static const char *vicap_dvp_name = "stream_cif_dvp_id0";
static const char *ooc_tx_name = "rkooc-ooctx";
static const char *irfpa_rx_name = "rk-irfpaisp-rx";

typedef std::map<std::string, struct v4l2_query_ext_ctrl> ctrl_qmap;
typedef std::map<unsigned, std::string> ctrl_idmap;

typedef struct {
    rk_irfpa_streamhw_ctx_t ctx;

    bool inited = false;
    bool started = false;
    bool ready = false;
    bool exit = false;
    struct ev_loop *loop;
    ev_async exit_sig;

    cv4l_fd sensor_fd;
    cv4l_fd vicap_fd;
    cv4l_fd ooctx_fd;

    cv4l_queue ooctx_queue;
    ev_io ooctx_watcher;
    std::list<cv4l_buffer> ooctx_buffers;

    cv4l_queue vicap_queue;
    ev_io vicap_watcher;
    cv4l_buffer curbuf;

    ctrl_qmap ctrl_str2q;
    ctrl_idmap ctrl_id2str;

    cv4l_fd irfparx_fd;
    cv4l_queue irfparx_queue;
    ev_io irfparx_watcher;
    std::list<cv4l_buffer> irfparx_buffers;
    uint32_t irfparx_seq;
    v4l2_rect irfparx_crop;
    uint32_t irfparx_pixelformat;
    uint32_t irfparx_imagesize;

} streamhw_priv_t;

static bool streamhw_open_device(streamhw_priv_t *priv, struct media_device * device, const char *name, cv4l_fd &c_fd)
{
    bool ret = false;
    int type, fd = 0;
    media_entity *entity = NULL;
    const char *entity_name = NULL;

    entity = media_get_entity_by_name(device, name);
    if (!entity) {
        rk_irfpa_log_error(priv->ctx.log, "%s not found", name);
        goto FAIL;
    }
    entity_name = media_entity_get_devname(entity);
    type = media_entity_type(entity);

    c_fd.s_direct(true);

    if (type == MEDIA_ENT_T_V4L2_SUBDEV) {
        fd = c_fd.subdev_open(entity_name);
    } else {
        fd = c_fd.open(entity_name);
    }
    if (fd < 0) {
        rk_irfpa_log_error(priv->ctx.log, "Failed to open %s: %s", entity_name, strerror(errno));
        goto FAIL;
    }

    ret = true;
FAIL:
    return ret;
}

static std::string name2var(const char *name)
{
	std::string s;
	int add_underscore = 0;

	while (*name) {
		if (isalnum(*name)) {
			if (add_underscore)
				s += '_';
			add_underscore = 0;
			s += std::string(1, tolower(*name));
		}
		else if (s.length()) add_underscore = 1;
		name++;
	}
	return s;
}

static int query_ext_ctrl_ioctl(bool have_query_ext_ctrl, int fd, struct v4l2_query_ext_ctrl &qctrl)
{
	struct v4l2_queryctrl qc;
	int rc;

	if (have_query_ext_ctrl) {
		rc = ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &qctrl);
		if (rc != ENOTTY)
			return rc;
	}
	qc.id = qctrl.id;
	rc = ioctl(fd, VIDIOC_QUERYCTRL, &qc);
	if (rc == 0) {
		qctrl.type = qc.type;
		memcpy(qctrl.name, qc.name, sizeof(qctrl.name));
		qctrl.minimum = qc.minimum;
		if (qc.type == V4L2_CTRL_TYPE_BITMASK) {
			qctrl.maximum = (__u32)qc.maximum;
			qctrl.default_value = (__u32)qc.default_value;
		} else {
			qctrl.maximum = qc.maximum;
			qctrl.default_value = qc.default_value;
		}
		qctrl.step = qc.step;
		qctrl.flags = qc.flags;
		qctrl.elems = 1;
		qctrl.nr_of_dims = 0;
		memset(qctrl.dims, 0, sizeof(qctrl.dims));
		switch (qctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER64:
			qctrl.elem_size = sizeof(__s64);
			break;
		case V4L2_CTRL_TYPE_STRING:
			qctrl.elem_size = qc.maximum + 1;
			qctrl.flags |= V4L2_CTRL_FLAG_HAS_PAYLOAD;
			break;
		default:
			qctrl.elem_size = sizeof(__s32);
			break;
		}
		memset(qctrl.reserved, 0, sizeof(qctrl.reserved));
	}
	qctrl.id = qc.id;
	return rc;
}

static void streamhw_init_controls(streamhw_priv_t *priv)
{
	const unsigned next_fl = V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;
	struct v4l2_query_ext_ctrl qctrl;
	struct v4l2_query_ext_ctrl qc = {
		V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND
	};
	int rc = ioctl(priv->sensor_fd.g_fd(), VIDIOC_QUERY_EXT_CTRL, &qc);
	bool have_query_ext_ctrl = rc == 0;

	int fd = priv->sensor_fd.g_fd();

	memset(&qctrl, 0, sizeof(qctrl));
	qctrl.id = next_fl;
	while (query_ext_ctrl_ioctl(have_query_ext_ctrl, fd, qctrl) == 0) {
		if (qctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS &&
		    !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {
            std::string str = name2var(qctrl.name);

			priv->ctrl_str2q[str] = qctrl;
			priv->ctrl_id2str[qctrl.id] = str;
            rk_irfpa_log_debug(priv->ctx.log, "add sensor ctrl: %s", qctrl.name);
		}
		qctrl.id |= next_fl;
	}
}

static bool vicaphw_init(streamhw_priv_t *priv)
{
    bool ret = false;
    struct media_device *device = NULL;   ///< 媒体设备结构体指针
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    const struct media_device_info *info = NULL;
	const struct media_entity_desc *sensor_info = NULL;
    const struct media_link *link = NULL;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    // 创建并打开媒体设备
    device = media_device_new(priv->ctx.cfg.vicap_media);
    if (!device) {
        rk_irfpa_log_error(priv->ctx.log, "open VICAP media failed");
        goto FAIL;
    }

    // 枚举媒体设备上的所有实体
    media_device_enumerate(device);
    info = media_get_info(device);

    ret = streamhw_open_device(priv, device, vicap_dvp_name, priv->vicap_fd);
    if (!ret)
        goto FAIL;

    entity = media_get_entity_by_name(device, vicap_dvp_name);
    link = media_entity_get_link(entity, 0);
    if (link && link->source && link->source->entity) {
        sensor_info = media_entity_get_info(link->source->entity);
    }
    if (!sensor_info) {
        rk_irfpa_log_error(priv->ctx.log, "VICAP not link sensor");
        goto FAIL;
    }

    ret = streamhw_open_device(priv, device, sensor_info->name, priv->sensor_fd);
    if (!ret)
        goto FAIL;

    rk_irfpa_log_info(priv->ctx.log, "Sensor name: %s", sensor_info->name);
    strcpy(priv->ctx.info.sensor_name, sensor_info->name);

    streamhw_init_controls(priv);

    ret = true;
FAIL:
    media_device_unref(device);
    return ret;
}

static bool handle_vicap(streamhw_priv_t *priv)
{
	int ret;
    cv4l_buffer &buf = priv->curbuf;

	for (;;) {
		ret = priv->vicap_fd.dqbuf(buf);
        if (ret)
            return false;
		if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
			break;
		if (priv->vicap_fd.qbuf(buf))
			return false;
	}
	
    uint32_t seq =  buf.g_sequence();
    uint16_t *ptr = (uint16_t *)priv->vicap_queue.g_dataptr(buf.g_index(), 0);

    priv->ctx.res.y16 = ptr;
    priv->ctx.res.seq = seq;
    priv->ready = true;
    /*
    ret = streamhw_sensor_process(priv, ptr);
    if (ret) {
        priv->ctx.res.y16 = ptr;
        priv->ctx.res.seq = seq;
        priv->ready = true;
    } else {
        priv->vicap_fd.qbuf(buf);
    }
    */
	return 0;
}

void vicap_cb (EV_P_ ev_io *w, int revents)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ev_userdata(EV_A);
    handle_vicap(priv);
}

static bool vicaphw_prepare(streamhw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->vicap_fd.g_fmt(fmt);
    int fd, fd_flags;
    const unsigned reqbufs_count_cap = 3;

    priv->ctx.info.vi_width = fmt.g_width();
    priv->ctx.info.vi_height = fmt.g_height();
    uint32_t pf = fmt.g_pixelformat();

    if (pf == V4L2_PIX_FMT_Y14) {
        priv->ctx.info.vi_bits = 14;
        rk_irfpa_log_info(priv->ctx.log, "vicap size: %dx%d, 14bit",
                priv->ctx.info.vi_width, priv->ctx.info.vi_height);
    }


    // 初始化队列(内存映射模式)
    priv->vicap_queue.init(priv->vicap_fd.g_type(), V4L2_MEMORY_MMAP);

    // 请求缓冲区
    if (priv->vicap_queue.reqbufs(&priv->vicap_fd, reqbufs_count_cap))
        goto FAIL;

    // 获取缓冲区
    if (priv->vicap_queue.obtain_bufs(&priv->vicap_fd))
        goto FAIL;

    // 将所有缓冲区加入队列
    if (priv->vicap_queue.queue_all(&priv->vicap_fd))
        goto FAIL;


    priv->curbuf.init(priv->vicap_queue);
    // 设置非阻塞IO
    fd = priv->vicap_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    ev_io_init (&priv->vicap_watcher, vicap_cb, fd, EV_READ);
    ev_io_start (priv->loop, &priv->vicap_watcher);

    return true;
FAIL:
    rk_irfpa_log_error(priv->ctx.log, "%s error", __func__);
    return false;
}

static bool vicaphw_start(streamhw_priv_t *priv)
{
    if (!priv->inited) {
        rk_irfpa_log_error(priv->ctx.log, "%s error, not inited", __func__);
        return false;
    }

    if (priv->started)
        return true;

    rk_irfpa_log_debug(priv->ctx.log, "vicap streamon");
    int ret = priv->vicap_fd.streamon();
    if (ret) {
        rk_irfpa_log_error(priv->ctx.log, "%s error: %d", __func__, ret);
        return false;
    }
    priv->started = true;
    priv->exit = false;
    return true;
}

static void vicaphw_stop(streamhw_priv_t *priv)
{
    if (priv->started) {
        rk_irfpa_log_debug(priv->ctx.log, "vicap streamoff");
        priv->vicap_fd.streamoff();
        priv->started = false;
    }
}

static void vicaphw_deinit(streamhw_priv_t *priv)
{
    if (priv->inited) {
        rk_irfpa_log_debug(priv->ctx.log, "free and close vicap");

        ev_io_stop(priv->loop, &priv->vicap_watcher);
        priv->vicap_queue.free(&priv->vicap_fd);
        priv->vicap_fd.close();
        priv->sensor_fd.close();
        priv->inited = false;
    }
}

rk_irfpa_streamhw_ctx_t* rk_irfpa_streamhw_create_ctx(void)
{
    streamhw_priv_t *priv = new streamhw_priv_t;
    if (!priv)
        return NULL;

    return &priv->ctx;
}

static bool oochw_init(streamhw_priv_t *priv)
{
    bool ret = false;
    struct media_device *device = NULL;   ///< 媒体设备结构体指针
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    const struct media_device_info *info = NULL;

    // 创建并打开媒体设备
    device = media_device_new(priv->ctx.cfg.ooc_media);
    if (!device) {
        rk_irfpa_log_error(priv->ctx.log, "open OOC media failed");
        goto FAIL;
    }

    media_device_enumerate(device);
    info = media_get_info(device);

    ret = streamhw_open_device(priv, device, ooc_tx_name, priv->ooctx_fd);
    if (!ret)
        goto FAIL;
    ret = streamhw_open_device(priv, device, irfpa_rx_name, priv->irfparx_fd);
    if (!ret)
        goto FAIL;

    ret = true;
FAIL:
    media_device_unref(device);
    return ret;
}

static bool handle_ooctx(streamhw_priv_t *priv)
{
    int ret;
    cv4l_buffer buf(priv->ooctx_queue);

    for (;;) {
        ret = priv->ooctx_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->ooctx_fd.qbuf(buf))
            return false;
    }

    priv->ooctx_buffers.push_back(buf);
    return 0;
}

static void ooctx_cb (EV_P_ ev_io *w, int revents) {
    streamhw_priv_t *priv = (streamhw_priv_t *)ev_userdata(EV_A);
    handle_ooctx(priv);
}

static bool oochw_prepare(streamhw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->ooctx_fd.g_fmt(fmt);
    int fd, fd_flags;
    const unsigned reqbufs_count_cap = 3;

    int w = fmt.g_width();
    int h = fmt.g_height();
    priv->ctx.info.ooc_width = w;
    priv->ctx.info.ooc_height = h;
    int oocbuf_size = w * h;

    rk_irfpa_log_info(priv->ctx.log, "ooc size: %dx%d", w, h);

    priv->ctx.oocbuf = (uint8_t *)malloc(oocbuf_size);

    // 初始化队列(内存映射模式)
    priv->ooctx_queue.init(priv->ooctx_fd.g_type(), V4L2_MEMORY_MMAP);

    // 请求缓冲区
    if (priv->ooctx_queue.reqbufs(&priv->ooctx_fd, reqbufs_count_cap))
        goto FAIL;

    // 获取缓冲区
    if (priv->ooctx_queue.obtain_bufs(&priv->ooctx_fd))
        goto FAIL;

    // 查询并设置每个缓冲区
    for (unsigned i = 0; i < priv->ooctx_queue.g_buffers(); i++) {
        cv4l_buffer buf(priv->ooctx_queue);
        if (priv->ooctx_fd.querybuf(buf, i))
            goto FAIL;
        
        // 检查缓冲区大小是否与参数结构匹配
        int size =  buf.g_length();
        if (size != oocbuf_size) {
            rk_irfpa_log_error(priv->ctx.log, "ooctx size mistach %d", size);
            goto FAIL1;
        }

        // 设置缓冲区参数
        buf.s_bytesused(size);
        buf.s_index(i);

        // 添加到参数缓冲区列表
        priv->ooctx_buffers.push_back(buf);
    }

    // 设置非阻塞IO
    fd = priv->ooctx_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);


    ev_io_init (&priv->ooctx_watcher, ooctx_cb, fd, EV_WRITE);
    ev_io_start (priv->loop, &priv->ooctx_watcher);

    if (priv->ooctx_fd.streamon())
        goto FAIL;

    rk_irfpa_log_debug(priv->ctx.log, "ooc streamon");
    return true;
FAIL:
    rk_irfpa_log_error(priv->ctx.log, "%s error", __func__);
FAIL1:
    return false;
}

static void oochw_deinit(streamhw_priv_t *priv)
{
    rk_irfpa_log_debug(priv->ctx.log, "ooc streamoff and close");

    priv->ooctx_fd.streamoff();
    ev_io_stop(priv->loop, &priv->ooctx_watcher);
    priv->ooctx_queue.free(&priv->ooctx_fd);
    priv->ooctx_fd.close();
    free(priv->ctx.oocbuf);
}

static bool irfparx_update_fmt(streamhw_priv_t *priv)
{
    cv4l_fmt fmt;
    v4l2_selection sel;
    priv->irfparx_fd.g_fmt(fmt);
    priv->irfparx_fd.g_selection(sel);
    rk_irfpa_output_node_info_t *output = &priv->ctx.info.output;
    
    uint16_t w = fmt.g_width();
    uint16_t h = fmt.g_height();
    uint32_t pf = fmt.g_pixelformat();


    /*
    output->top = sel.r.top;
    output->left = sel.r.left;
    output->width = sel.r.width;
    output->height = sel.r.height;
    */
    output->top = 0;
    output->left = 0;
    output->width = w;
    output->height = h;
    output->pixelformat = pf;
    if (w != output->width || h != output->height) {
        rk_irfpa_log_error(priv->ctx.log, "%s format not support", __func__);
        return false;
    }
    output->pixelformat = pf;

    if (pf == V4L2_PIX_FMT_GREY) {
        rk_irfpa_log_warn(priv->ctx.log, "%s V4L2_PIX_FMT_GREY not support", __func__);
        priv->irfparx_imagesize = w * h;
    } else {
        priv->irfparx_imagesize = w * h * 3 / 2;
    }

    return true;
}

static bool handle_irfparx(streamhw_priv_t *priv)
{
    int ret;
    cv4l_buffer buf(priv->irfparx_queue);

    for (;;) {
        ret = priv->irfparx_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->irfparx_fd.qbuf(buf))
            return false;
    }

    priv->irfparx_buffers.push_back(buf);
    return 0;
}

static void irfparx_cb (EV_P_ ev_io *w, int revents)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ev_userdata(EV_A);
    handle_irfparx(priv);
}

static bool irfparx_start(streamhw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->irfparx_fd.g_fmt(fmt);
    int fd, fd_flags;
    int image_size = fmt.g_width() * fmt.g_height();
    
    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    priv->irfparx_queue.init(priv->irfparx_fd.g_type(), V4L2_MEMORY_MMAP);

    if (priv->irfparx_queue.reqbufs(&priv->irfparx_fd, 2))
        goto FAIL;
    
    if (priv->irfparx_queue.obtain_bufs(&priv->irfparx_fd))
        goto FAIL;
    
    for (unsigned i = 0; i < priv->irfparx_queue.g_buffers(); i++) {
        cv4l_buffer buf(priv->irfparx_queue);
        if (priv->irfparx_fd.querybuf(buf, i))
            goto FAIL;
        int size =  buf.g_length();
        if (size != priv->irfparx_imagesize) {
            rk_irfpa_log_error(priv->ctx.log, "%s image size %d error, expect %d", __func__, size,priv->irfparx_imagesize);
            goto FAIL1;
        }
        
        // 设置缓冲区参数
        buf.s_bytesused(size);
        buf.s_index(i);

        priv->irfparx_buffers.push_back(buf);
    }

    // 设置非阻塞IO
    fd = priv->irfparx_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    if (priv->irfparx_fd.streamon())
        goto FAIL;

    priv->irfparx_seq = 0;
    ev_io_init(&priv->irfparx_watcher, irfparx_cb, fd, EV_WRITE);
    ev_io_start(priv->loop, &priv->irfparx_watcher);
    return true;
FAIL:
    rk_irfpa_log_error(priv->ctx.log, "%s failed", __func__);
FAIL1:
    return false;
}

static void irfparx_deinit(streamhw_priv_t *priv)
{
    priv->irfparx_fd.streamoff();
    ev_io_stop (priv->loop, &priv->irfparx_watcher);
    priv->irfparx_queue.free(&priv->irfparx_fd);
    priv->irfparx_fd.close();
}


static void exit_sig_cb (EV_P_ ev_async *w, int revents)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ev_userdata(EV_A);
    priv->exit = true;
}

bool rk_irfpa_streamhw_init(rk_irfpa_streamhw_ctx_t *ctx)
{
    bool ret = false;
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;

    rk_irfpa_log_trace(priv->ctx.log, "enter: %s", __func__);

    if (!priv)
        goto FAIL;

    if (!priv->ctx.cfg.ooc_media) {
        rk_irfpa_log_error(priv->ctx.log, "OOC media name not set");
        goto FAIL;
    }
    if (!priv->ctx.cfg.vicap_media) {
        rk_irfpa_log_error(priv->ctx.log, "VICAP media name not set");
        goto FAIL;
    }

    priv->loop = ev_loop_new (EVBACKEND_SELECT | EVFLAG_NOENV);
    ev_set_userdata (priv->loop, priv);

    ev_async_init(&priv->exit_sig, exit_sig_cb);
    ev_async_start(priv->loop, &priv->exit_sig);

    ret = vicaphw_init(priv);
    if (!ret)
        goto FAIL;

    ret = vicaphw_prepare(priv);
    if (!ret)
        goto FAIL;

    ret = oochw_init(priv);
    if (!ret)
        goto FAIL;

    ret = oochw_prepare(priv);
    if (!ret)
        goto FAIL;

    ret = irfparx_update_fmt(priv);
    if (!ret)
        goto FAIL;
    ret = irfparx_start(priv);
    if (!ret)
        goto FAIL;

    priv->inited = true;

    rk_irfpa_log_trace(priv->ctx.log, "%s: sucess", __func__);
    return true;
FAIL:
    return false;
}

bool rk_irfpa_streamhw_start(rk_irfpa_streamhw_ctx_t *ctx)
{
    bool ret;
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    if (!priv)
        goto FAIL;

    ret = vicaphw_start(priv);
    if (!ret)
        goto FAIL;
    return true;
FAIL:
    return false;
}

bool rk_irfpa_streamhw_stop(rk_irfpa_streamhw_ctx_t *ctx)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    if (!priv)
        goto FAIL;

    ev_async_send(priv->loop, &priv->exit_sig);
    vicaphw_stop(priv);
    return true;
FAIL:
    return false;
}

bool rk_irfpa_streamhw_getFrame(rk_irfpa_streamhw_ctx_t *ctx)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    if (priv->ready) {
        priv->vicap_fd.qbuf(priv->curbuf);
        priv->ready = false;
    }

    while (1) {
        ev_run (priv->loop, EVRUN_ONCE);
        if (priv->ready)
            break;

        if (priv->exit)
            break;
    }
    return priv->ready;
}

void rk_irfpa_streamhw_deinit(rk_irfpa_streamhw_ctx_t *ctx)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    if (!priv)
        return;


    vicaphw_stop(priv);
    vicaphw_deinit(priv);
    oochw_deinit(priv);
    irfparx_deinit(priv);
    ev_loop_destroy(priv->loop);
    delete priv;
}

static bool get_control_by_name(streamhw_priv_t *priv, const char *name, int *val)
{
    std::string str(name);

    if (priv->ctrl_str2q.count(str) == 0) {
        rk_irfpa_log_error(priv->ctx.log, "no such control");
        return false;
    }

    struct v4l2_ext_control ctrl;
    struct v4l2_ext_controls ctrls;
    struct v4l2_query_ext_ctrl &qc = priv->ctrl_str2q[str];
    memset(&ctrl, 0, sizeof(ctrl));

    uint32_t status = 0;
    ctrl.id = qc.id;
    ctrl.size = qc.elems * qc.elem_size;
    ctrl.ptr = &status;

    ctrls.which = V4L2_CTRL_ID2WHICH(qc.id);
    ctrls.count = 1;
    ctrls.controls = &ctrl;
    ioctl(priv->sensor_fd.g_fd(), VIDIOC_G_EXT_CTRLS, &ctrls);

    *val = ctrl.value;
    return true;
}

static bool set_control_by_name(streamhw_priv_t *priv, const char *name, int val)
{
    std::string str(name);
    if (priv->ctrl_str2q.count(str) == 0) {
        rk_irfpa_log_error(priv->ctx.log, "no such control");
        return false;
    }

    struct v4l2_ext_control ctrl;
    struct v4l2_ext_controls ctrls;
    struct v4l2_query_ext_ctrl &qc = priv->ctrl_str2q[str];
    memset(&ctrl, 0, sizeof(ctrl));

    ctrl.id = qc.id;
    ctrl.size = qc.elems * qc.elem_size;
    ctrl.value = val;

    ctrls.which = V4L2_CTRL_ID2WHICH(qc.id);
    ctrls.count = 1;
    ctrls.controls = &ctrl;
    if (ioctl(priv->sensor_fd.g_fd(), VIDIOC_S_EXT_CTRLS, &ctrls)) {
        if (ctrls.error_idx >= ctrls.count) {
            rk_irfpa_log_error(priv->ctx.log, "Error setting controls: %s",
                    strerror(errno));
        }
        else {
            rk_irfpa_log_error(priv->ctx.log, "%s: %s",
                    priv->ctrl_id2str[ctrl.id].c_str(), strerror(errno));
        }
        return false;
    }

    return true;
}

bool rk_irfpa_streamhw_setControl(rk_irfpa_streamhw_ctx_t *ctx, const char *ctrl, int val)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    return set_control_by_name(priv, ctrl, val);
}

bool rk_irfpa_streamhw_getControl(rk_irfpa_streamhw_ctx_t *ctx, const char *ctrl, int *val)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;
    return get_control_by_name(priv, ctrl, val);
}

bool rk_irfpa_streamhw_flush_ooc(rk_irfpa_streamhw_ctx_t *ctx)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;

    while (priv->ooctx_buffers.size() == 0) {
        ev_run (priv->loop, EVRUN_ONCE);
    }

    cv4l_buffer buf = priv->ooctx_buffers.front();
    priv->ooctx_buffers.pop_front();

    int i = buf.g_index();
    void *ptr = priv->ooctx_queue.g_dataptr(i, 0);
    memcpy(ptr, priv->ctx.oocbuf, buf.g_length());

    priv->ooctx_fd.qbuf(buf);
    return true;
}

bool rk_irfpa_streamhw_irfparx(rk_irfpa_streamhw_ctx_t *ctx, uint8_t *data)
{
    streamhw_priv_t *priv = (streamhw_priv_t *)ctx;

    while (priv->irfparx_buffers.size() == 0) {
        ev_run (priv->loop, EVRUN_ONCE);
    }

    if (priv->irfparx_buffers.size() > 0) {
        cv4l_buffer buf = priv->irfparx_buffers.front();
        priv->irfparx_buffers.pop_front();

        int i = buf.g_index();
        void *ptr = priv->irfparx_queue.g_dataptr(i, 0);
        memcpy(ptr, data, buf.g_length());

        priv->irfparx_fd.qbuf(buf);
        return true;
    }
    return false;
}


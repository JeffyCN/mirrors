#include "rk_irfpa_isphw.h"
#include "ev_cpp.h"
#include "cv4l-helpers.h"
#include <dlfcn.h>
#include <linux/dma-buf.h>
#include <sys/ioctl.h>

extern "C" {
#include "mediactl.h"  // C语言媒体设备控制库
}
#include <list>

// ISP相关设备节点和实体名称定义
static const char *isp_subdev_name = "rkisp-isp-subdev";    ///< ISP子设备名称
static const char *isp_rawrd_name = "rkisp_rawrd2_s";       ///< RAW数据读取节点名称
static const char *isp_mainpath_name = "rkisp_mainpath";    ///< 主路径输出节点名称
static const char *isp_params_name = "rkisp-input-params";  ///< ISP参数输入节点名称
static const char *isp_stats_name = "rkisp-statistics";  ///< ISP统计输出节点名称

typedef struct {
    rk_irfpa_isphw_ctx_t ctx;

    bool inited = false;

    bool iir_ready = false;
    bool y8_ready = false;
    struct ev_loop *loop;

    void *iir_address[8] = {0};
    int iir_stride = 0;
    int iir_size;
    int iir_fd;
    uint8_t iir_cnt = 0;
    int iir_idx = 0;

    void* wgt_address[8] = { 0 };
    int wgt_size;
    int wgt_fd;

    void* ds_address[8] = { 0 };
    int ds_size;
    int ds_fd;

    void* tmp_iir = NULL;
    void* tmp_wgt = NULL;
    void* tmp_ds = NULL;
    // ISP子设备文件描述符和数据结构
    cv4l_fd subdev_fd;        ///< ISP子设备文件描述符
    
    // RAW数据读取相关数据结构
    cv4l_fd rawrd_fd;         ///< RAW数据读取文件描述符
    cv4l_queue rawrd_queue;        ///< RAW数据队列
    ev_io rawrd_watcher;           ///< RAW数据事件监视器(libev)
    std::list<cv4l_buffer> rawrd_buffers;  ///< RAW数据缓冲区列表

    // 主路径输出相关数据结构
    cv4l_fd mainpath_fd;       ///< 主路径文件描述符
    cv4l_queue mainpath_queue;     ///< 主路径数据队列
    ev_io mainpath_watcher;        ///< 主路径事件监视器(libev)

    // ISP参数配置相关数据结构
    cv4l_fd params_fd;         ///< 参数文件描述符
    cv4l_queue params_queue;       ///< 参数队列
    ev_io params_watcher;          ///< 事件监视器(libev)
    std::list<cv4l_buffer> params_buffers;  ///< 参数缓冲区列表

    cv4l_fd stats_fd;         ///< 参数文件描述符
    cv4l_queue stats_queue;       ///< 参数队列
    ev_io stats_watcher;          ///< 事件监视器(libev)

    struct isp33_sharp_stat sharp_stat;
} isphw_priv_t;

static void rk_irfpa_perror(isphw_priv_t* priv, const char* fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    vsnprintf(priv->ctx.error_str, 128, fmt, va);
    va_end(va);
}

static bool rk_irfpa_open_device(isphw_priv_t *priv, struct media_device * device, const char *name, cv4l_fd &c_fd)
{
    bool ret = false;
    int type, fd = 0;
    media_entity *entity = NULL;
    const char *entity_name = NULL;

    entity = media_get_entity_by_name(device, name);
    if (!entity) {
        rk_irfpa_perror(priv, "%s not found", name);
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
        rk_irfpa_perror(priv, "Failed to open %s: %s", entity_name, strerror(errno));
        goto FAIL;
    }

    ret = true;
FAIL:
    return ret;
}

static bool isphw_init(isphw_priv_t *priv)
{
    int fd = -1;
    bool ret = false;
    struct media_device *device = NULL;   ///< 媒体设备结构体指针
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    const struct media_device_info *info = NULL;
    media_pad* src_pad    = NULL;
    media_pad* sink_pad   = NULL;

    // 创建并打开媒体设备
    device = media_device_new(priv->ctx.cfg.isp_media);
    if (!device) {
        rk_irfpa_perror(priv, "Failed to open ISP media node");
        goto FAIL;
    }

    // 枚举媒体设备上的所有实体
    media_device_enumerate(device);
    info = media_get_info(device);

    // 打开ISP子设备(ISP处理核心)
    ret = rk_irfpa_open_device(priv, device, isp_subdev_name, priv->subdev_fd);
    if (!ret)
        goto FAIL;

    // 打开RAW数据读取设备(RAW图像输入)
    ret = rk_irfpa_open_device(priv, device, isp_rawrd_name, priv->rawrd_fd);
    if (!ret)
        goto FAIL;

    // 打开主路径输出设备(处理后的图像输出)
    ret = rk_irfpa_open_device(priv, device, isp_mainpath_name, priv->mainpath_fd);
    if (!ret)
        goto FAIL;

    // 打开ISP参数输入设备(ISP算法参数配置)
    ret = rk_irfpa_open_device(priv, device, isp_params_name, priv->params_fd);
    if (!ret)
        goto FAIL;

    // 打开ISP统计输出设备
    ret = rk_irfpa_open_device(priv, device, isp_stats_name, priv->stats_fd);
    if (!ret)
        goto FAIL;

    entity = media_get_entity_by_name(device, isp_subdev_name);
    sink_pad = (media_pad*)media_entity_get_pad(entity, 0);  // 获取接收pad
    entity = media_get_entity_by_name(device, isp_rawrd_name);
    src_pad = (media_pad *)media_entity_get_pad(entity, 0);  // 获取源pad
    // 建立RAW数据源到ISP接收端的链接
    media_setup_link(device, src_pad, sink_pad, MEDIA_LNK_FL_ENABLED);

    ret = true;

FAIL:
    media_device_unref(device);
    return ret;
}

/*
bool irfparx_set_result(isphw_priv_t *priv)
{
    if (priv->irfparx_buffers.size() > 0) {
        cv4l_buffer buf = priv->irfparx_buffers.front();
        priv->irfparx_buffers.pop_front();

        int i = buf.g_index();
        void *ptr = priv->irfparx_queue.g_dataptr(i, 0);
        memcpy(ptr, priv->ctx.res.y8, buf.g_length());

        priv->irfparx_fd.qbuf(buf);
    } else {
        return false;
    }
    return true;
}
*/

/**
 * @brief 设置ISP管道格式
 * 
 * 配置ISP子设备的输入和输出格式，包括分辨率、数据格式和裁剪区域。
 * 此函数确保ISP管道能够正确处理指定分辨率的图像数据。
 * 
 * @param width 图像宽度
 * @param height 图像高度
 */
static bool isp_setup_pipeline_fmt(isphw_priv_t *priv, int width, int height)
{
    int ret = false;

    // 设置ISP接收端(Sink)格式
    struct v4l2_subdev_format isp_sink_fmt;
    memset(&isp_sink_fmt, 0, sizeof(isp_sink_fmt));
    isp_sink_fmt.pad = 0;  // 接收pad编号
    isp_sink_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;  // 设置活动格式
    ret = priv->subdev_fd.subdev_g_fmt(isp_sink_fmt);  // 获取当前格式
    if (ret) {
        goto FAIL;
    }
    
    // 设置新的接收格式参数
    isp_sink_fmt.format.width = width;
    isp_sink_fmt.format.height = height;
    isp_sink_fmt.format.code = MEDIA_BUS_FMT_Y12_1X12;  // 12-bit灰度格式
    ret = priv->subdev_fd.subdev_s_fmt(isp_sink_fmt);  // 应用新格式
    if (ret) {
        goto FAIL;
    }

    // 设置裁剪区域(ISP不需要裁剪，设置为全分辨率)
    struct v4l2_subdev_selection aSelection;
    memset(&aSelection, 0, sizeof(aSelection));
    aSelection.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    aSelection.pad = 0;  // 接收pad
    aSelection.flags = 0;
    aSelection.target = V4L2_SEL_TGT_CROP;  // 裁剪目标
    aSelection.r.width = width;
    aSelection.r.height = height;
    aSelection.r.left = 0;
    aSelection.r.top = 0;
    ret = priv->subdev_fd.subdev_s_selection(aSelection);  // 应用裁剪设置
    if (ret) {
        goto FAIL;
    }

    // 设置ISP源端(输出)格式
    struct v4l2_subdev_format isp_src_fmt;
    memset(&isp_src_fmt, 0, sizeof(isp_src_fmt));
    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad = 2;  // 源pad编号
    ret = priv->subdev_fd.subdev_g_fmt(isp_src_fmt);  // 获取当前格式
    if (ret) {
        goto FAIL;
    }
    // 设置输出格式与裁剪区域一致
    isp_src_fmt.format.width = aSelection.r.width;
    isp_src_fmt.format.height = aSelection.r.height;
    ret = priv->subdev_fd.subdev_s_fmt(isp_src_fmt);  // 应用新格式
    if (ret) {
        goto FAIL;
    }

    return true;
FAIL:
    rk_irfpa_perror(priv, "%s failed", __func__);
    return false;
}

/**
 * @brief 更新RAW数据读取格式
 * 
 * 配置RAW数据读取节点的格式，包括分辨率、像素格式和内存模式。
 * 
 * @param width 图像宽度
 * @param height 图像高度
 */
static bool isp_update_rawrd_fmt(isphw_priv_t *priv, int width, int height)
{
    cv4l_fmt fmt;
    int memory_mode = 1;
    priv->rawrd_fd.g_fmt(fmt);  // 获取当前格式
    
    // 设置CSI内存模式(可能影响DMA缓冲区配置)
    int ret = v4l_ioctl(priv->rawrd_fd.g_v4l_fd(), RKISP_CMD_SET_CSI_MEMORY_MODE, &memory_mode);
    if (ret) {
        goto FAIL;
    }

    // 设置新格式参数
    fmt.s_width(width);
    fmt.s_height(height);
    fmt.s_pixelformat(V4L2_PIX_FMT_Y12);  // 12-bit灰度格式
    ret = priv->rawrd_fd.s_fmt(fmt);  // 应用新格式
    if (ret) {
        goto FAIL;
    }
    return true;
FAIL:
    rk_irfpa_perror(priv, "%s failed", __func__);
    return false;
}

/**
 * @brief 设置主路径输出格式
 * 
 * 配置主路径输出节点的格式和裁剪区域，确保输出图像符合预期规格。
 * 
 * @param width 图像宽度
 * @param height 图像高度
 */
static bool isp_setup_mainpath_fmt(isphw_priv_t *priv, int width, int height)
{
    int ret = 0;
    cv4l_fmt fmt;
    struct v4l2_selection sel;
    priv->mainpath_fd.g_fmt(fmt);  // 获取当前格式

    // 设置新格式参数
    fmt.s_width(width);
    fmt.s_height(height);
    fmt.s_bytesperline(0);  // 自动计算步长
    fmt.s_pixelformat(V4L2_PIX_FMT_GREY);  // 8-bit灰度格式
    ret = priv->mainpath_fd.s_fmt(fmt);  // 应用新格式
    if (ret) {
        goto FAIL;
    }

    // 设置裁剪区域
    sel.type = fmt.g_type();
    sel.target = V4L2_SEL_TGT_CROP;  // 裁剪目标
    sel.flags = 0;
    sel.r.left = 0;
    sel.r.top = 0;
    sel.r.width = width;
    sel.r.height = height;
    ret = priv->mainpath_fd.s_selection(sel);  // 应用裁剪设置
    if (ret) {
        goto FAIL;
    }
    return true;
FAIL:
    rk_irfpa_perror(priv, "%s failed", __func__);
    return false;
}

/**
 * @brief 更新ISP硬件状态
 * 
 * 强制ISP硬件重新枚举和更新内部状态，通常在格式变更后调用，
 * 确保硬件与软件配置同步。
 */
bool isp_update_hw_status(isphw_priv_t *priv)
{
    // 发送强制枚举命令给ISP硬件
    int ret = v4l_ioctl(priv->subdev_fd.g_v4l_fd(), RKISP_CMD_MULTI_DEV_FORCE_ENUM, NULL);
    if (ret) {
        rk_irfpa_perror(priv, "%s failed", __func__);
        return false;
    }
    return true;
}

bool isphw_configure(isphw_priv_t *priv, int width, int height)
{
    bool ret = false;

    ret = isp_setup_pipeline_fmt(priv, width, height);
    if (!ret) {
        goto FAIL;
    }

    ret = isp_update_rawrd_fmt(priv, width, height);
    if (!ret) {
        goto FAIL;
    }

    ret = isp_setup_mainpath_fmt(priv, width, height);
    if (!ret) {
        goto FAIL;
    }

    ret = isp_update_hw_status(priv);
    if (!ret) {
        goto FAIL;
    }
    ret = true;
FAIL:
    return ret;
}

/**
 * @brief 设置ISP参数缓冲区
 * 
 * 初始化ISP参数队列和缓冲区，为ISP算法参数配置做准备。
 * 分配并映射参数缓冲区内存，确保参数能够正确传递给ISP硬件。
 */
static bool isphw_start_params(isphw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->params_fd.g_fmt(fmt);
    int fd, fd_flags;
    
    // 初始化参数队列(元数据输出类型，内存映射模式)
    priv->params_queue.init(V4L2_BUF_TYPE_META_OUTPUT, V4L2_MEMORY_MMAP);

    // 请求参数缓冲区
    if (priv->params_queue.reqbufs(&priv->params_fd, 2))
        goto FAIL;
    
    // 获取缓冲区
    if (priv->params_queue.obtain_bufs(&priv->params_fd))
        goto FAIL;
    
    // 查询并设置每个缓冲区
    for (unsigned i = 0; i < priv->params_queue.g_buffers(); i++) {
        cv4l_buffer buf(priv->params_queue);
        if (priv->params_fd.querybuf(buf, i))
            goto FAIL;
        
        // 检查缓冲区大小是否与参数结构匹配
        int size =  buf.g_length();
        if (size != sizeof(isphw_params_t)) {
            rk_irfpa_perror(priv, "isp params length %d, mismatch", size);
            goto FAIL1;
        }

        // 设置缓冲区参数
        buf.s_bytesused(size);
        buf.s_index(i);

        // 添加到参数缓冲区列表
        priv->params_buffers.push_back(buf);
    }

    // 设置非阻塞IO
    fd = priv->params_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    // 初始化并启动事件监视器
    // 启动数据流
    if (priv->params_fd.streamon(V4L2_BUF_TYPE_META_OUTPUT))
        goto FAIL;

    return true;
FAIL:
    rk_irfpa_perror(priv, "%s failed", __func__);
FAIL1:
    return false;
}

/**
 * @brief 打开主路径数据流
 * 
 * 初始化主路径队列，分配缓冲区，启动数据流和事件监视。
 * 配置非阻塞IO并注册回调函数处理输出数据。
 */
static bool isphw_start_mainpath(isphw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->mainpath_fd.g_fmt(fmt);
    unsigned reqbufs_count_cap = 3;  // 请求3个缓冲区
    int fd, fd_flags;

    // 初始化主路径队列(内存映射模式)
    priv->mainpath_queue.init(priv->mainpath_fd.g_type(), V4L2_MEMORY_MMAP);

    // 请求缓冲区
    if (priv->mainpath_queue.reqbufs(&priv->mainpath_fd, reqbufs_count_cap))
        goto FAIL;

    // 获取缓冲区
    if (priv->mainpath_queue.obtain_bufs(&priv->mainpath_fd))
        goto FAIL;

    // 将所有缓冲区加入队列
    if (priv->mainpath_queue.queue_all(&priv->mainpath_fd))
        goto FAIL;

    // 设置非阻塞IO
    fd = priv->mainpath_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    // 启动数据流
    if (priv->mainpath_fd.streamon())
        goto FAIL;

    return true;
FAIL:
    return false;
}

/**
 * @brief 打开RAW数据流
 * 
 * 初始化RAW数据队列，分配缓冲区，启动数据流和事件监视。
 * 配置非阻塞IO并注册回调函数处理输入数据。
 */
static bool isphw_start_rawrd(isphw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->rawrd_fd.g_fmt(fmt);
    unsigned reqbufs_count_cap = 3;  // 请求3个缓冲区
    int fd, fd_flags;
    int image_size = fmt.g_width() * fmt.g_height() * 2;
    priv->ctx.info.width = fmt.g_width();
    priv->ctx.info.height = fmt.g_height();

    // 初始化RAW数据队列(内存映射模式)
    priv->rawrd_queue.init(priv->rawrd_fd.g_type(), V4L2_MEMORY_MMAP);

    // 请求缓冲区
    if (priv->rawrd_queue.reqbufs(&priv->rawrd_fd, reqbufs_count_cap))
        goto FAIL;

    // 获取缓冲区
    if (priv->rawrd_queue.obtain_bufs(&priv->rawrd_fd))
        goto FAIL;

    // 查询并设置每个缓冲区
    for (unsigned i = 0; i < priv->rawrd_queue.g_buffers(); i++) {
        cv4l_buffer buf(priv->rawrd_queue);
        if (priv->rawrd_fd.querybuf(buf, i))
            goto FAIL;
        
        // 检查缓冲区大小是否与预期图像大小匹配
        int size =  buf.g_length();
        if (size != image_size) {
            rk_irfpa_perror(priv, "isp rawrd length %d, mismatch!!\n", size);
            goto FAIL;
        }

        // 设置缓冲区参数
        buf.s_bytesused(size);
        buf.s_index(i);

        // 添加到RAW数据缓冲区列表
        priv->rawrd_buffers.push_back(buf);
    }

    // 设置非阻塞IO
    fd = priv->rawrd_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    // 启动数据流
    if (priv->rawrd_fd.streamon())
        goto FAIL;

    return true;
FAIL:
    return false;
}

static bool isphw_start_stats(isphw_priv_t *priv)
{
    cv4l_fmt fmt;
    priv->stats_fd.g_fmt(fmt);
    unsigned reqbufs_count_cap = 3;  // 请求3个缓冲区
    int fd, fd_flags;

    // 初始化队列(内存映射模式)
    priv->stats_queue.init(priv->stats_fd.g_type(), V4L2_MEMORY_MMAP);

    // 请求缓冲区
    if (priv->stats_queue.reqbufs(&priv->stats_fd, reqbufs_count_cap))
        goto FAIL;

    // 获取缓冲区
    if (priv->stats_queue.obtain_bufs(&priv->stats_fd))
        goto FAIL;

    // 将所有缓冲区加入队列
    if (priv->stats_queue.queue_all(&priv->stats_fd))
        goto FAIL;

    // 设置非阻塞IO
    fd = priv->stats_fd.g_fd();
    fd_flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

    // 启动数据流
    if (priv->stats_fd.streamon())
        goto FAIL;

    return true;
FAIL:
    // 错误处理:打印错误并退出
    rk_irfpa_perror(priv, "%s failed!\n", __func__);
    return false;

}

bool isphw_init_bay3d_buf(isphw_priv_t *priv)
{
    struct rkisp_bnr_buf_info _bay3dbuf = {0};
    int image_width = priv->ctx.cfg.width;
    int image_height = priv->ctx.cfg.height;

    _bay3dbuf.iir.buf_cnt = 1;
    _bay3dbuf.u.v35.ds.buf_cnt = 1;
    _bay3dbuf.u.v35.wgt.buf_cnt = 1;
    _bay3dbuf.u.v35.iir_rw_fmt = 2;

    int ret = v4l_ioctl(priv->subdev_fd.g_v4l_fd(), RKISP_CMD_INIT_BNR_BUF, &_bay3dbuf);
    if (ret) {
        rk_irfpa_perror(priv, "RKISP_CMD_INIT_BNR_BUF failed" );
        return false;
    }

    for (int i = 0; i < _bay3dbuf.iir.buf_cnt; i++) {
        int iir_fd   = _bay3dbuf.iir.buf_fd[i];
        int iir_size = _bay3dbuf.iir.buf_size;
        void *addr = (char*)mmap(NULL, iir_size, PROT_READ | PROT_WRITE, MAP_SHARED, iir_fd, 0);
        priv->iir_address[i] = addr;
        priv->iir_size = iir_size;
        priv->iir_fd = iir_fd;
        priv->iir_stride = (image_width * 2 + image_width / 4 + 15) / 16 * 16;

        int wgt_fd = _bay3dbuf.u.v35.wgt.buf_fd[i];
        int wgt_size = _bay3dbuf.u.v35.wgt.buf_size;
        addr = (char*)mmap(NULL, wgt_size, PROT_READ | PROT_WRITE, MAP_SHARED, wgt_fd, 0);
        priv->wgt_address[i] = addr;
        priv->wgt_size = wgt_size;
        int ds_fd = _bay3dbuf.u.v35.ds.buf_fd[i];
        int ds_size = _bay3dbuf.u.v35.ds.buf_size;
        addr = (char*)mmap(NULL, ds_size, PROT_READ | PROT_WRITE, MAP_SHARED, ds_fd, 0);
        priv->ds_address[i] = addr;
        priv->ds_size = ds_size;
        if (iir_size != priv->iir_stride * image_height) {
            rk_irfpa_perror(priv, "buf size err: iir[%d] fd %d, size %d", i, iir_fd, iir_size);
            return false;
        }
    }
    priv->iir_cnt = _bay3dbuf.iir.buf_cnt;
    return true;
}

static bool handle_params(isphw_priv_t *priv)
{
    int ret;
    cv4l_buffer buf(priv->params_queue);

    for (;;) {
        ret = priv->params_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->params_fd.qbuf(buf))
            return false;
    }

    priv->params_buffers.push_back(buf);
    return 0;
}

static bool handle_rawrd(isphw_priv_t *priv)
{
    int ret;
    bool last_buffer = false;
    cv4l_buffer buf(priv->rawrd_queue);

    for (;;) {
        ret = priv->rawrd_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->rawrd_fd.qbuf(buf))
            return false;
    }

    priv->rawrd_buffers.push_back(buf);
    return true;
}

static bool handle_mainpath(isphw_priv_t *priv)
{
    int ret;
    bool last_buffer = false;
    cv4l_buffer buf(priv->mainpath_queue);

    for (;;) {
        ret = priv->mainpath_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->mainpath_fd.qbuf(buf))
            return false;
    }

    uint8_t *ptr = (uint8_t *)priv->mainpath_queue.g_dataptr(buf.g_index(), 0);
    memcpy(priv->ctx.res.y8, ptr, buf.g_bytesused());
    priv->ctx.res.seq =  buf.g_sequence();

    priv->y8_ready = true;
    priv->mainpath_fd.qbuf(buf);
    return true;
}

static bool handle_stats(isphw_priv_t* priv)
{
    int ret;
    bool last_buffer = false;
    cv4l_buffer buf(priv->stats_queue);

    for (;;) {
        ret = priv->stats_fd.dqbuf(buf);
        if (ret)
            return false;
        if (!(buf.g_flags() & V4L2_BUF_FLAG_ERROR))
            break;
        if (priv->stats_fd.qbuf(buf))
            return false;
    }


    int image_width = priv->ctx.cfg.width;
    int image_height = priv->ctx.cfg.height;
    uint32_t seq =  buf.g_sequence();
    void *ptr = (void *)priv->stats_queue.g_dataptr(buf.g_index(), 0);
    struct rkisp35_stat_buffer *stats_buffer = (struct rkisp35_stat_buffer *)ptr;
    struct isp35_stat *stat = &stats_buffer->stat;
    int iir_idx = stat->buf_bay3d_iir_index;
    if (iir_idx >= 0) {
        int16_t* iir_addr = (int16_t*)priv->iir_address[iir_idx];
        uint16_t stride = priv->iir_stride / 2;

        for (int j=0; j<image_height; j++) {
            for (int i=0; i<image_width; i++) {
                priv->ctx.res.iir[i + image_width * j] = (iir_addr[i +stride * j] / 16) & 0xfff;
            }
        }
        priv->ctx.res.seq = seq;
    }
    memcpy(&priv->sharp_stat, &stat->sharp, sizeof(struct isp33_sharp_stat));
    priv->iir_idx = iir_idx;
    priv->iir_ready = true;
    priv->stats_fd.qbuf(buf);
    return true;
}

static void params_cb (EV_P_ ev_io *w, int revents)
{
    isphw_priv_t *priv = (isphw_priv_t *)ev_userdata(EV_A);
    handle_params(priv);
}

static void mainpath_cb (EV_P_ ev_io *w, int revents)
{
    isphw_priv_t *priv = (isphw_priv_t *)ev_userdata(EV_A);
    handle_mainpath(priv);
}

static void rawrd_cb (EV_P_ ev_io *w, int revents)
{
    isphw_priv_t *priv = (isphw_priv_t *)ev_userdata(EV_A);
    handle_rawrd(priv);
}

static void stats_cb (EV_P_ ev_io *w, int revents)
{
    isphw_priv_t *priv = (isphw_priv_t *)ev_userdata(EV_A);
    handle_stats(priv);
}

static void isphw_event_init(isphw_priv_t *priv)
{
    int fd;
    fd = priv->mainpath_fd.g_fd();
    ev_io_init (&priv->mainpath_watcher, mainpath_cb, fd, EV_READ);
    fd = priv->rawrd_fd.g_fd();
    ev_io_init (&priv->rawrd_watcher, rawrd_cb, fd, EV_WRITE);
    fd = priv->params_fd.g_fd();
    ev_io_init (&priv->params_watcher, params_cb, fd, EV_WRITE);
    fd = priv->stats_fd.g_fd();
    ev_io_init (&priv->stats_watcher, stats_cb, fd, EV_READ);
    ev_io_start (priv->loop, &priv->stats_watcher);
    ev_io_start (priv->loop, &priv->mainpath_watcher);
    ev_io_start (priv->loop, &priv->rawrd_watcher);
    ev_io_start (priv->loop, &priv->params_watcher);
}

static bool isphw_start(isphw_priv_t *priv)
{
    bool ret = false;
    ret = isphw_start_params(priv);
    if (!ret)
        goto FAIL;
    ret = isphw_init_bay3d_buf(priv);
    if (!ret)
        goto FAIL;
    ret = isphw_start_mainpath(priv);
    if (!ret)
        goto FAIL;
    ret = isphw_start_stats(priv);
    if (!ret)
        goto FAIL;
    ret = isphw_start_rawrd(priv);
    if (!ret)
        goto FAIL;

    ret = true;
FAIL:
    return ret;
}

bool isphw_set_config(isphw_priv_t *priv, void *cfg)
{
    // 检查是否有可用缓冲区
    if (priv->params_buffers.size() > 0) {
        // 获取第一个可用缓冲区
        cv4l_buffer buf = priv->params_buffers.front();
        priv->params_buffers.pop_front();

        // 复制参数数据到缓冲区
        int i = buf.g_index();
        void *ptr = priv->params_queue.g_dataptr(i, 0);
        memcpy(ptr, cfg, buf.g_length());

        // 将缓冲区加入队列
        priv->params_fd.qbuf(buf);
    } else {
        rk_irfpa_perror(priv, "%s: no available buffer", __func__);
        return false;
    }
    return true;
}

bool isphw_trigger_readback(isphw_priv_t *priv, uint32_t seq)
{
    struct isp2x_csi_trigger tg = { 0 };
    struct timespec timestamp;
    uint64_t ts = 0;

    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    ts = timestamp.tv_sec * 1000000000LL + timestamp.tv_nsec;

    tg.frame_id = seq;
    tg.times = 1;
    tg.mode = T_START_X1;
    tg.sof_timestamp = ts;
    tg.frame_timestamp = ts;

    int ret = v4l_ioctl(priv->subdev_fd.g_v4l_fd(), RKISP_CMD_TRIGGER_READ_BACK, &tg);
    if (ret) {
        rk_irfpa_perror(priv, "isp_subdev trigger readback error: %s\n", strerror(errno));
        return false;
    }
    return true;
}


bool isphw_queueframe(isphw_priv_t *priv, void *data)
{
    // 检查是否有可用缓冲区
    if (priv->rawrd_buffers.size() > 0) {
        // 获取第一个可用缓冲区
        cv4l_buffer buf = priv->rawrd_buffers.front();
        priv->rawrd_buffers.pop_front();

        // 复制参数数据到缓冲区
        int i = buf.g_index();
        void *ptr = priv->rawrd_queue.g_dataptr(i, 0);
        memcpy(ptr, data, buf.g_length());

        // 将缓冲区加入队列
        priv->rawrd_fd.qbuf(buf);
    } else {
        rk_irfpa_perror(priv, "%s: no available buffer", __func__);
        return false;
    }
    return true;
}

static bool isphw_run(isphw_priv_t *priv)
{
    priv->iir_ready = false;
    priv->y8_ready = false;

    while (1) {
        ev_run (priv->loop, EVRUN_ONCE);
        if (priv->iir_ready && priv->y8_ready) {
            break;
        }
    }
    return true;
}

static void isphw_deinit(isphw_priv_t *priv)
{
    if (priv->inited) {
        priv->rawrd_fd.streamoff();
        priv->mainpath_fd.streamoff();
        priv->stats_fd.streamoff();
        priv->params_fd.streamoff();
        
        ev_io_stop (priv->loop, &priv->rawrd_watcher);
        ev_io_stop (priv->loop, &priv->mainpath_watcher);
        ev_io_stop (priv->loop, &priv->stats_watcher);
        ev_io_stop (priv->loop, &priv->params_watcher);

        priv->rawrd_queue.free(&priv->rawrd_fd);
        priv->mainpath_queue.free(&priv->mainpath_fd);
        priv->stats_queue.free(&priv->stats_fd);
        priv->params_queue.free(&priv->params_fd);

        priv->rawrd_fd.close();
        priv->mainpath_fd.close();
        priv->stats_fd.close();
        priv->params_fd.close();
        priv->subdev_fd.close();

        ev_loop_destroy(priv->loop);

        priv->inited = false;
    }
}

rk_irfpa_isphw_ctx_t* rk_irfpa_isphw_create_ctx(void)
{
    isphw_priv_t *priv = new isphw_priv_t;
    if (!priv)
        return NULL;
    return &priv->ctx;
}

bool rk_irfpa_isphw_init(rk_irfpa_isphw_ctx_t *ctx)
{
    int width, height;
    bool ret;
    isphw_priv_t* priv = (isphw_priv_t*)ctx;
    isphw_params_t* params = &ctx->params;
    if (!priv)
        goto FAIL;

    if (!ctx->cfg.isp_media) {
        rk_irfpa_perror(priv, "ISP media name not set");
        goto FAIL;
    }

    priv->loop = ev_loop_new (EVBACKEND_SELECT | EVFLAG_NOENV);
    ev_set_userdata (priv->loop, priv);

    width = ctx->cfg.width;
    height = ctx->cfg.height;
    ctx->info.seq = 0;

    ret = isphw_init(priv);
    if (!ret)
        goto FAIL;

    ret = isphw_configure(priv, width, height);
    if (!ret)
        goto FAIL;

    params->frame_id = 0;
    params->module_ens = 0;
    params->module_cfg_update = 0;
    params->module_en_update = 0xffffffffffffffffLL;

    // 初始化并启动事件监视器
    isphw_event_init(priv);

    ret = isphw_start(priv);
    if (!ret)
        goto FAIL;

    ctx->res.iir = (uint16_t *)malloc(width * height * 2);
    ctx->res.y8 = (uint8_t *)malloc(width * height);


    priv->inited = true;
    return true;
FAIL:
    return false;
}

void rk_irfpa_isphw_preProcess(rk_irfpa_isphw_ctx_t* ctx, uint32_t seq) {
    if (seq < 1) {
        return;
    }
    isphw_priv_t* priv = (isphw_priv_t*)ctx;
    struct dma_buf_sync sync = { 0 };
    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
    ioctl(priv->iir_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(priv->wgt_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(priv->ds_fd, DMA_BUF_IOCTL_SYNC, &sync);

    memcpy(priv->iir_address[priv->iir_idx], priv->tmp_iir, priv->iir_size);
    memcpy(priv->wgt_address[priv->iir_idx], priv->tmp_wgt, priv->wgt_size);
    memcpy(priv->ds_address[priv->iir_idx], priv->tmp_ds, priv->ds_size);

    sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
    ioctl(priv->iir_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(priv->wgt_fd, DMA_BUF_IOCTL_SYNC, &sync);
    ioctl(priv->ds_fd, DMA_BUF_IOCTL_SYNC, &sync);
}

bool rk_irfpa_isphw_save_stats(rk_irfpa_isphw_ctx_t* ctx) {
    isphw_priv_t *priv = (isphw_priv_t *)ctx;
    memcpy(&ctx->stats.sharp, &priv->sharp_stat, sizeof(ctx->stats.sharp));
    return true;
}

bool rk_irfpa_isphw_process(rk_irfpa_isphw_ctx_t* ctx, uint16_t* data, uint32_t seq)
{
    bool ret = false;
    isphw_priv_t *priv = (isphw_priv_t *)ctx;
    isphw_params_t *params = &ctx->params;

    if (!priv)
        goto FAIL;

    if (!priv->inited) {
        rk_irfpa_perror(priv, "isp hw not started");
        goto FAIL;
    }

    params->frame_id = seq;

    ret = isphw_set_config(priv, params);
    if (!ret)
        goto FAIL;

    ret = isphw_queueframe(priv, data);
    if (!ret)
        goto FAIL;

    ret = isphw_trigger_readback(priv, seq);
    if (!ret)
        goto FAIL;

    ret = isphw_run(priv);

    params->module_cfg_update = 0;
    params->module_en_update = 0;
    ctx->info.seq ++;
    ret = true;
FAIL:
    return ret;
}

void rk_irfpa_isphw_postProcess(rk_irfpa_isphw_ctx_t* ctx) {
    isphw_priv_t* priv = (isphw_priv_t*)ctx;
    if (priv->tmp_iir == NULL) {
        priv->tmp_iir = (void*)malloc(priv->iir_size);
    }
    memcpy(priv->tmp_iir, priv->iir_address[priv->iir_idx], priv->iir_size);
    if (priv->tmp_wgt == NULL) {
        priv->tmp_wgt = (void*)malloc(priv->wgt_size);
    }
    memcpy(priv->tmp_wgt, priv->wgt_address[priv->iir_idx], priv->wgt_size);

    if (priv->tmp_ds == NULL) {
        priv->tmp_ds = (void*)malloc(priv->ds_size);
    }
    memcpy(priv->tmp_ds, priv->ds_address[priv->iir_idx], priv->ds_size);
}

void rk_irfpa_isphw_deinit(rk_irfpa_isphw_ctx_t* ctx)
{
    isphw_priv_t *priv = (isphw_priv_t *)ctx;
    if (!priv)
        return;
    if (priv->tmp_iir != NULL) {
        free(priv->tmp_iir);
    }
    if (priv->tmp_wgt != NULL) {
        free(priv->tmp_wgt);
    }
    if (priv->tmp_ds != NULL) {
        free(priv->tmp_ds);
    }
    isphw_deinit(priv);

    free(ctx->res.iir);
    free(ctx->res.y8);
    delete priv;
}

/*
bool rk_irfpa_isphw_config_tnr(rk_irfpa_isphw_ctx_t *ctx, isphw_tnr_params_t *cfg, bool enable)
{
    isphw_params_t *params = &ctx->params;
    struct isp35_bay3d_cfg *pCfg = &params->others.bay3d_cfg;
    uint64_t mask = ISP35_MODULE_BAY3D;

    bool cur_en = !!(params->module_ens & mask);
    if (cur_en != enable) {
        params->module_en_update |= mask;
    }

    if (!enable) {
        params->module_ens &=~ mask;
        return true;
    }
    params->module_ens |= mask;

    // if (memcmp(pCfg, cfg, sizeof(*cfg)) == 0) {
    //     return true;
    // }
    // memcpy(pCfg, cfg, sizeof(*cfg));
    params->module_cfg_update |= mask;

    return true;
}

bool rk_irfpa_isphw_config_ynr(rk_irfpa_isphw_ctx_t *ctx, isphw_ynr_params_t *cfg, bool enable)
{
    isphw_params_t *params = &ctx->params;
    struct isp35_ynr_cfg *pCfg = &params->others.ynr_cfg;
    uint64_t mask = ISP35_MODULE_YNR;

    bool cur_en = !!(params->module_ens & mask);
    if (cur_en != enable) {
        params->module_en_update |= mask;
    }

    if (!enable) {
        params->module_ens &=~ mask;
        return true;
    }
    params->module_ens |= mask;

    if (memcmp(pCfg, cfg, sizeof(*cfg)) == 0) {
        return true;
    }
    memcpy(pCfg, cfg, sizeof(*cfg));
    params->module_cfg_update |= mask;

    return true;
}

bool rk_irfpa_isphw_config_sharp(rk_irfpa_isphw_ctx_t *ctx, isphw_sharp_params_t *cfg, bool enable)
{
    isphw_params_t *params = &ctx->params;
    struct isp35_sharp_cfg *pCfg = &params->others.sharp_cfg;
    uint64_t mask = ISP35_MODULE_SHARP;

    bool cur_en = !!(params->module_ens & mask);
    if (cur_en != enable) {
        params->module_en_update |= mask;
    }

    if (!enable) {
        params->module_ens &=~ mask;
        return true;
    }
    params->module_ens |= mask;

    if (memcmp(pCfg, cfg, sizeof(*cfg)) == 0) {
        return true;
    }
    memcpy(pCfg, cfg, sizeof(*cfg));
    params->module_cfg_update |= mask;

    mask = ISP35_MODULE_CNR;
    params->module_ens |= mask;
    params->module_en_update |= mask;
    params->module_cfg_update |= mask;

    mask = ISP35_MODULE_ENH;
    params->module_ens |= mask;
    params->module_en_update |= mask;
    params->module_cfg_update |= mask;

    mask = ISP35_MODULE_GAIN;
    params->module_ens |= mask;
    params->module_en_update |= mask;
    params->module_cfg_update |= mask;

    struct isp35_enh_cfg *pCfg_enh = &params->others.enh_cfg;
    mask = ISP35_MODULE_ENH;
    params->module_ens |= mask;
    params->module_en_update |= mask;
    params->module_cfg_update |= mask;

    pCfg_enh->bypass = 1;

    return true;
}
*/
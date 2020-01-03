#ifndef __RKISP_API_CTX__
#define __RKISP_API_CTX__

#include <linux/videodev2.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * The context of rkisp capturing device
 *
 * @dev_path:   The file path of video device, eg:
 *                - /dev/video0
 * @fd:         The fd of open(), application could select() on it
 * @width:      The target width of frame resolution
 * @height:     The target height of frame resolution
 * @fcc:        The format in fcc defined in linux/videodev2.h, eg:
 *                - V4L2_PIX_FMT_NV12
 * @uselocal3A: Integrate 3A in this process or not.
 */
struct rkisp_api_ctx {
    int fd;
    char dev_path[256];
    int width;
    int height;
    int fcc;
    int uselocal3A;
};

/*
 * The linked buffer held frame data.
 * For particular formats(e.g. YUV422M), every frame may contains multi planes.
 *
 * @buf:        The buffer pointer if exist
 * @fd:         The fd of buffer
 * @size:       The size of this buffer/plane
 * @expo_time:  the expo time in nano second of this frame when @uselocal3A is true
 * @gain:       The gain of this frame when @uselocal3A is true
 * @frame_id:   The frame id from librkisp.so
 * @next_plane: Link to the next plane if this is multi-planes buffer
 */
struct rkisp_api_buf {
    void *buf;
    int fd;
    int size;
    struct {
        int64_t expo_time;
        int gain;
        int64_t frame_id;
    } metadata;
    struct timeval timestamp;
    struct rkisp_api_buf *next_plane;
};

/*
 * Open rkisp video device and check device capability. Required.
 *
 * @dev_path:   The video device path, e.g. /dev/video1
 * @uselocal3A: Integrate 3A in this process if true.
 *               - True if app want to use manual AE/GAIN/FPS, or to get
 *                 AE/GAIN metadata of frame
 *               - False if app does not care about AE/GAIN metadata and will
 *                 use auto 3A.
 *
 * If failed return NULL.
 */
const struct rkisp_api_ctx*
rkisp_open_device(const char *dev_path, int uselocal3A);

/*
 * Set the frame format, incluing resolution and fourcc. Optional.
 * This can be called only when stream off.
 *
 * If app does set format, the default values are
 *   - width:  640
 *   - height: 480
 *   -fcc:     V4L2_PIX_FMT_NV12
 *
 * Return 0 if success, error num if format is not supported.
 */
int
rkisp_set_fmt(const struct rkisp_api_ctx *ctx, int w, int h, int fcc);

/*
 * Request buffers bases on frame format. Optional.
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 * @buf_count:  The buffer count shall not less than 2. 4 is recommended.
 *              If cpu is heavy loading, a larger buffer may
 *              reduce buffer dropping, but normally 8 is large enough.
 * @dmabuf_fds: If app uses extenal dmabuf, set dmabuf fd array to it.
 *                - If dmabuf_fds is NULL, buffers are allocated from isp driver
 *                  and exported to user
 *                - If dmabuf_fds is not NULL, buffers are allocated from app and
 *                  import to isp driver
 *              @dmabuf_fds array size should be @buf_count
 * @dmabuf_size:The buffer size of dmabuf if @dmabuf_fds is not NULL
 *
 * Return 0 if success, error num if fail
 */
int
rkisp_set_buf(const struct rkisp_api_ctx *ctx, int buf_count,
              const int dmabuf_fds[], int dmabuf_size);

/*
 * Start streaming.
 * This setup buffers first and then starting stream.
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 *
 * Return 0 if success, error num if fail
 */
int
rkisp_start_capture(const struct rkisp_api_ctx *ctx);

/*
 * Get a frame from rkisp. It will block until there's a frame or timeout.
 * App shall return back frame buffer by #{rkisp_put_frame}
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 * @timeout_ms:    Wait at most @timeout_ms if larger than 0.
 *                 Note: It uses select() to wait for frame data. In case
 *                       app wants to poll()/select() ctx->fd on its own,
 *                       @timeout_ms shall be 0 then.
 *
 * Return linked buffer list that contains one or more planes,
 * or NULL if something wrong.
 */
const struct rkisp_api_buf*
rkisp_get_frame(const struct rkisp_api_ctx *ctx, int timeout_ms);

/*
 * Return back a frame buffer.
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 * @buf:        The linked buffer list obtained from #{rkisp_get_frame()}
 *
 */
void
rkisp_put_frame(const struct rkisp_api_ctx *ctx,
                const struct rkisp_api_buf *buf);

/*
 * Stop capture.
 * After capture stopped, #{rkisp_get_frame()} behavior is undefined.
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 */
void
rkisp_stop_capture(const struct rkisp_api_ctx *ctx);

/*
 * Close device and uninit.
 * Please make sure all buffers from #{rkisp_get_frame()} are returned back
 * by #{rkisp_put_frame()} before closing device.
 *
 * @ctx:        The context returned by #{rkisp_open_device()}
 */
void
rkisp_close_device(const struct rkisp_api_ctx *ctx);

int
rkisp_set_manual_expo(const struct rkisp_api_ctx *ctx, int on);
int
rkisp_update_expo(const struct rkisp_api_ctx *ctx, int gain, int64_t expo_time_ns);

int
rkisp_set_max_expotime(const struct rkisp_api_ctx *ctx, int64_t max_expo_time_ns);
int
rkisp_get_max_expotime(const struct rkisp_api_ctx *ctx, int64_t *max_expo_time_ns);
int
rkisp_set_max_gain(const struct rkisp_api_ctx *ctx, int max_gain);
int
rkisp_get_max_gain(const struct rkisp_api_ctx *ctx, int *max_gain);

int
rkisp_set_expo_weights(const struct rkisp_api_ctx *ctx,
                       unsigned char* weights, unsigned int size);
int
rkisp_set_fps_range(const struct rkisp_api_ctx *ctx, int max_fps);

#ifdef __cplusplus
}
#endif

#endif

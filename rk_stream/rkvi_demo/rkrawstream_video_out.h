#ifndef _RKRAWSTREAM_VIDEO_OUT_H_
#define _RKRAWSTREAM_VIDEO_OUT_H_

#include <linux/videodev2.h>
#include <stdbool.h>

typedef struct VI_buffer_s {
    struct v4l2_buffer _buf;
    void *start;
    size_t length;
    int export_fd;
    int sequence;
} VI_buffer_t;

typedef struct _VI_stream_context_s VI_stream_context_t;

VI_stream_context_t*
VI_stream_init(const char *vd_name, int width, int height, int fmt, bool user_poll);

int8_t VI_stream_deinit(VI_stream_context_t* ctx);
int8_t VI_stream_start(VI_stream_context_t *ctx);
int8_t VI_stream_stop(VI_stream_context_t *ctx);
VI_buffer_t* VI_streama_dequeue_buf(VI_stream_context_t* ctx);
int8_t VI_streama_queue_buf(VI_stream_context_t* ctx, VI_buffer_t* buf);

#endif
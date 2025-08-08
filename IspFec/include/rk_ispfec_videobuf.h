#ifndef RK_ISPFEC_VIDEOBUF_H
#define RK_ISPFEC_VIDEOBUF_H

#include <linux/dma-buf.h>
#include <stdint.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rkfec_buffer {
    uint32_t handle;
    int dmabuf_fd;
    uint64_t size;
    void* map;
};

int rk_ispfec_api_vb_init();
void rk_ispfec_api_vb_deinit(int fd);
int rk_ispfec_api_vb_alloc(int fd, uint32_t size, struct rkfec_buffer* buf);
int rk_ispfec_api_vb_free(int fd, struct rkfec_buffer* buf);

static inline int rk_ispfec_api_vb_sync_start(int fd, int write)
{
    struct dma_buf_sync sync = {
        .flags = DMA_BUF_SYNC_START | (write ? DMA_BUF_SYNC_WRITE : DMA_BUF_SYNC_READ)
    };
    return ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
}

static inline int rk_ispfec_api_vb_sync_end(int fd, int write)
{
    struct dma_buf_sync sync = {
        .flags = DMA_BUF_SYNC_END | (write ? DMA_BUF_SYNC_WRITE : DMA_BUF_SYNC_READ)
    };
    return ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
}

#ifdef __cplusplus
}
#endif

#endif  // RK_ISPFEC_VIDEOBUF_H
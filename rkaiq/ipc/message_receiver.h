#ifndef _MESSAGE_RECEIVER_H_
#define _MESSAGE_RECEIVER_H_

#include <stdint.h>

// define IPCSERVER module logs
#define LOGD_IPC(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOGE_IPC(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOGW_IPC(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOGV_IPC(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOGI_IPC(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOG1_IPC(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)
#define LOGK_IPC(...) XCAM_MODULE_LOG_KEY(XCORE_LOG_MODULE_IPC, 0xff, ##__VA_ARGS__)

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t rxstate;                    // rx state machine state
    uint8_t *rxbuf;                     // buffer for receiving data
    uint32_t rxbuf_size;                // rxbuf size
    uint32_t rxpos;                     // rxbuf pos
    uint32_t rxi;                       // Field size byte counter
    uint32_t reseti;                    // reset counter
    uint32_t magic;
    uint32_t cmd_id;
    uint32_t cmd_ret;
    uint32_t cmd_seq;
    uint32_t size1;
    uint32_t size2;
    uint32_t data_cksum;
    int send_fd;
    void *aiqctx;

    uint32_t ret;                       // return result
    char note[128];                     // return note
} receiver_t;

void message_receiver_init(receiver_t *rec);
void message_receiver_deinit(receiver_t *rec);
void message_receiver_gotdata(receiver_t *rec, uint8_t *buf, int len);

#ifdef  __cplusplus
}
#endif

#endif


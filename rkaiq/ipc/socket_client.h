#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <pthread.h>
#include <stdbool.h>
#include "message_receiver.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define IPC_CMDID_GET_3ASTATS       0x100
#define IPC_CMDID_ENQUE_RKRAW       0x101
#define IPC_CMDID_WRITE_AWBIN       0x102
#define IPC_CMDID_GET_HDRCOMPRCURVE 0x103

#define IPC_RET_OK  0xff00
#define IPC_RET_JSON_ERROR  0xff01
#define IPC_RET_UAPI_ERROR  0xff02
#define IPC_RET_AIQ_ERROR  0xff03
#define IPC_RET_UAPI_WARNING  0xff04

#pragma pack(1)
typedef struct RkToolExpToolParam_s {
    float integration_time;
    float analog_gain;
    float digital_gain;
    float isp_dgain;
    int   iso;
    int   dcg_mode;
    int   longfrm_mode;
} RkToolExpRealParam_t;

typedef struct RkToolExpSensorParam_s {
    uint32_t fine_integration_time;
    uint32_t coarse_integration_time;
    uint32_t analog_gain_code_global;
    uint32_t digital_gain_global;
    uint32_t isp_digital_gain;
} RkToolExpSensorParam_t;

typedef struct RkToolExpParam_s {
    RkToolExpRealParam_t exp_real_params; //real value
    RkToolExpSensorParam_t exp_sensor_params;//reg value
} RkToolExpParam_t;

typedef struct RkToolAwbParam_s {
    float awb_gain_r;
    float awb_gain_gr;
    float awb_gain_gb;
    float awb_gain_b;
} RkToolAwbParam_t;

#ifndef HDR_COMPR_POINT_MAX
#define HDR_COMPR_POINT_MAX 32
#endif
typedef struct rk_aiq_isp_tool_hdr_compr_curve_s {
    uint16_t version;  // 0x0100 = v1.0
    uint8_t point;
    uint8_t src_bit;
    uint8_t k_shift;
    uint16_t data_compr[HDR_COMPR_POINT_MAX];
    uint32_t data_src[HDR_COMPR_POINT_MAX];
    uint32_t slope_k[HDR_COMPR_POINT_MAX];
    uint8_t reserved[512];
} rk_aiq_isp_tool_hdr_compr_curve_t;

typedef struct rk_aiq_isp_tool_stats_s {
    uint16_t version; // 0x0100 = v1.0
    uint32_t frameID;
    RkToolExpParam_t linearExp; // from RKAiqAecStats_t::ae_exp.LinearExp.exp_real_params
    RkToolExpParam_t hdrExp[3];
    RkToolAwbParam_t awbGain;
    uint8_t reserved[240];
} rk_aiq_isp_tool_stats_t;
#pragma pack()

typedef struct {
    int cid;
    int sockfd;
    int client_fd;
    int data_fd;
    int stopfd[2];
    bool quit;
    bool connected;
    bool thread_running;
    pthread_t client_thread;
    char *recvbuf;
    void *aiqctx;
    bool enable;

    receiver_t mReceiver;
} SocketClientCtx_t;

int socket_client_start(void *aiqctx, SocketClientCtx_t *ctx, int cid);
void socket_client_exit(SocketClientCtx_t *ctx);
void socket_client_setNote(SocketClientCtx_t *ctx, uint32_t ret, char *str);
bool socket_client_getConnected(SocketClientCtx_t *ctx);

rk_aiq_isp_tool_stats_t *socket_client_get_isp_statics(void* aiqctx);
int socket_client_writeAwbIn(void* aiqctx, char* data);
int socket_client_enque_rkraw(void* aiqctx, char* data);
rk_aiq_isp_tool_hdr_compr_curve_t* socket_client_get_hdrComprCurve(void* aiqctx);

#ifdef  __cplusplus
}
#endif

#endif

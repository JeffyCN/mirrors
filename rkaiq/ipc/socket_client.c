#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stddef.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "socket_client.h"
#include "xcam_log.h"
#include "c_base/aiq_base.h"
#include "uAPI2/rk_aiq_user_api2_stats.h"
#include "uAPI2/rk_aiq_user_api2_awb.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#if defined(USE_NEWSTRUCT)
#include "hwi_c/aiq_CamHwBase.h"
#include "uAPI2_c/rk_aiq_api_private_c.h"
#endif

#define USE_IPC_SERVER

#ifdef USE_IPC_SERVER
#ifdef __ANDROID__
#define LOCALSOCKET_NAME "/dev/socket/camera_tool"
#else
#define LOCALSOCKET_NAME "/tmp/UNIX.domain"
#endif
#else
#ifdef __ANDROID__
#define LOCALSOCKET_NAME "/dev/socket/camera_tool"
#else
#define LOCALSOCKET_NAME "/tmp/camera_tool"
#endif
#endif

#define BUF_SIZE 1024

#define FD_SET_MAX(fd, max, set) do { \
    FD_SET(fd, set); \
    if (max < fd + 1) max = fd + 1; \
} while (0)


static void recv_process(SocketClientCtx_t *ctx)
{
    int maxfd;
    int ret;
    int cid = ctx->cid;
    fd_set rset, wset, xset;

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);
    maxfd = 0;

    FD_SET_MAX(ctx->stopfd[0], maxfd, &rset);
    FD_SET_MAX(ctx->data_fd, maxfd, &rset);

    LOGD_IPC("%s[%d] start select, maxfd %d ...", __func__, cid, maxfd);
    ret = select(maxfd, &rset, NULL, NULL, NULL);
    if (ret >= 0) {
        if (FD_ISSET(ctx->stopfd[0], &rset)) {
            char c;
            ssize_t ret = read(ctx->stopfd[0], &c, sizeof(c));
            ctx->quit = true;
            LOGD_IPC("%s[%d]: stop signal read %x ...", __func__, cid, c);
            return;
        }
        if (FD_ISSET(ctx->data_fd, &rset)) {
            ret = recv(ctx->data_fd, ctx->recvbuf, BUF_SIZE, 0);
            if (ret <= 0) {
                LOGI_IPC("%s[%d]: toolbridge closed!", cid, __func__);
                close(ctx->data_fd);
                ctx->data_fd = 0;
                ctx->connected = false;
            } else {
                message_receiver_gotdata(&ctx->mReceiver, (uint8_t *)ctx->recvbuf, ret);
            }
        }
    }
}

static void sleep_with_select(SocketClientCtx_t *ctx, int sec) {
    fd_set rset;
    int maxfd = 0;
    struct timeval tv = {0};
    tv.tv_sec = sec;

    FD_ZERO(&rset);
    FD_SET_MAX(ctx->stopfd[0], maxfd, &rset);
    int ret = select(maxfd, &rset, NULL, NULL, &tv);
    if (ret >= 0) {
        if (FD_ISSET(ctx->stopfd[0], &rset)) {
            char c;
            ssize_t ret = read(ctx->stopfd[0], &c, sizeof(c));
            ctx->quit = true;
            return;
        }
    }
}

static void try_connect(SocketClientCtx_t *ctx, struct sockaddr_un *addr, int alen)
{
    int cid = ctx->cid;

    LOGD_IPC("%s[%d]: try connect ...", __func__, cid);
    if(connect(ctx->sockfd, (struct sockaddr *) addr, alen) >= 0) {
        LOGI_IPC("%s[%d]: toolbridge connected!", __func__, cid);
        ctx->data_fd = ctx->sockfd;
        ctx->mReceiver.send_fd = ctx->data_fd;
        ctx->connected = true;
        return;
    }

    // wait for 3 seconds
    sleep_with_select(ctx, 3);
}

static void try_accept(SocketClientCtx_t *ctx, struct sockaddr_un *addr, int alen)
{
    int cid = ctx->cid;
    LOGD_IPC("%s[%d]: try accept %d ...", __func__, cid, ctx->sockfd);

    int connfd = accept(ctx->sockfd, NULL, NULL);
    if (connfd > 0) {
        LOGI_IPC("%s[%d]: accept connfd %d...", __func__, cid, connfd);
        ctx->data_fd = connfd;
        ctx->mReceiver.send_fd = ctx->data_fd;
        ctx->connected = true;
        return;
    }

    // wait for 3 seconds
    sleep_with_select(ctx, 3);
}

static void *ClientThreadFunc(void *p)
{
    SocketClientCtx_t *ctx = (SocketClientCtx_t *)p;
    int ret;
    int cid = ctx->cid;

    struct sockaddr_un addr;
    size_t namelen;
    socklen_t alen;
    memset (&addr, 0, sizeof (addr));

    /* unix_path_max appears to be missing on linux */
    addr.sun_family = AF_LOCAL;
    sprintf(addr.sun_path, "%s%d", LOCALSOCKET_NAME, cid);
    namelen = strlen(addr.sun_path);
    alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;
    LOGD_IPC("%s[%d]: local client sun_path %s, len %d", __func__, cid, addr.sun_path, namelen);

    ctx->sockfd = socket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (ctx->sockfd <= 0) {
        LOGE_IPC("%s[%d]: create socket error: %s", strerror(errno), __func__, cid);
        ctx->sockfd = 0;
        return NULL;
    }
    LOGI_IPC("%s[%d]: create sockfd %d...", __func__, cid, ctx->sockfd);

#ifdef USE_IPC_SERVER
    int n = 1;
    /*ignore ENOENT*/
    unlink(addr.sun_path);
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

    if (bind(ctx->sockfd, (struct sockaddr *) &addr, alen) != 0) {
        LOGE_IPC("bind local server socket error: %s", strerror(errno));
        return NULL;
    }
    LOGD_IPC("bind local server socket ok ...");
    if (listen(ctx->sockfd, 2) != 0) {
        LOGE_IPC("listen local server socket error: %s", strerror(errno));
        return NULL;
    }
    LOGD_IPC("listen local server socket ok ...");
#endif

    ctx->thread_running = true;
    while (!ctx->quit) {
        if (!ctx->connected) {
#ifdef USE_IPC_SERVER
            try_accept(ctx, &addr, alen);
#else
            try_connect(ctx, &addr, alen);
#endif
        } else {
            recv_process(ctx);
        }
    }
    ctx->thread_running = false;
    return NULL;
}

int socket_client_start(void *aiqctx, SocketClientCtx_t *ctx, int cid) {
    int ret;
    LOGD_IPC("%s[%d]: enter", __func__, cid);

    ctx->enable = true;
    ctx->connected = false;
    ctx->quit = false;
    ctx->thread_running = false;
    ctx->sockfd = 0;
    ctx->cid = cid;
    ctx->aiqctx = aiqctx;

    message_receiver_init(&ctx->mReceiver);
    ctx->mReceiver.aiqctx = aiqctx;

    ctx->recvbuf = aiq_mallocz(BUF_SIZE);
    if (ctx->recvbuf == NULL) {
        LOGE_IPC("%s[%d]: malloc recvbuf error", __func__, cid);
        return -1;
    }

    if (pipe(ctx->stopfd) < 0) {
        LOGE_IPC("%s[%d]: create stop pipe error: %s", __func__, cid, strerror(errno));
        return -1;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&ctx->client_thread, &attr, ClientThreadFunc, ctx);
    pthread_attr_destroy(&attr);
    if (ret != 0) {
        LOGE_IPC("%s[%d]: create ipc thread error: %s", __func__, cid, strerror(ret));
        return -1;
    }

    LOGD_IPC("%s[%d]: done",__func__, cid);
    return 0;
}

void socket_client_exit(SocketClientCtx_t *ctx) {
    int cid = ctx->cid;
    LOGD_IPC("%s[%d]: enter", __func__, cid);

    if (ctx->thread_running) {
        char c = 0xfe;
        ctx->quit = true;
        ssize_t size = write(ctx->stopfd[1], &c, sizeof(c));
        pthread_join(ctx->client_thread, NULL);
    }

    if (ctx->recvbuf)
        aiq_free(ctx->recvbuf);
    if (ctx->data_fd && ctx->data_fd != ctx->sockfd)
        close(ctx->data_fd);
    if (ctx->sockfd)
        close(ctx->sockfd);
    if (ctx->stopfd[0])
        close(ctx->stopfd[0]);
    if (ctx->stopfd[1])
        close(ctx->stopfd[1]);

    message_receiver_deinit(&ctx->mReceiver);
    LOGD_IPC("%s[%d]: done", __func__, cid);
}

void socket_client_setNote(SocketClientCtx_t *ctx, uint32_t ret, char *str)
{
    if (ctx == NULL)
        return;
    receiver_t *rec = &ctx->mReceiver;
    if(rec->ret == 0xff00)
        rec->ret = ret;

    if (rec->note[0] == 0) {
        if (str) {
            strncpy(rec->note, str, 127);
            if (strlen(str) > 127) {
                LOGW_IPC("note info more len 127 byte, cut of 127 char");
            }
        }
    }
}

bool socket_client_getConnected(SocketClientCtx_t *ctx)
{
    if (ctx == NULL)
        return false;

    return ctx->connected;
}

void socket_client_setEnable(SocketClientCtx_t *ctx, bool enable)
{
    if (ctx == NULL)
        return;
    ctx->enable = enable;
}

static void copyRkAiqExpParamComb_t2RkToolExpParam_t(RkAiqExpParamComb_t *in, RkToolExpParam_t *out)
{
    // copy exp_real_params
    out->exp_real_params.analog_gain      = in->exp_real_params.analog_gain;
    out->exp_real_params.dcg_mode         = in->exp_real_params.dcg_mode;
    out->exp_real_params.digital_gain     = in->exp_real_params.digital_gain;
    out->exp_real_params.integration_time = in->exp_real_params.integration_time;
    out->exp_real_params.iso              = in->exp_real_params.iso;
    out->exp_real_params.isp_dgain        = in->exp_real_params.isp_dgain;
    out->exp_real_params.longfrm_mode     = in->exp_real_params.longfrm_mode;

    // copy exp_sensor_params
    out->exp_sensor_params.analog_gain_code_global = in->exp_sensor_params.analog_gain_code_global;
    out->exp_sensor_params.coarse_integration_time = in->exp_sensor_params.coarse_integration_time;
    out->exp_sensor_params.digital_gain_global     = in->exp_sensor_params.digital_gain_global;
    out->exp_sensor_params.fine_integration_time   = in->exp_sensor_params.fine_integration_time;
    out->exp_sensor_params.isp_digital_gain        = in->exp_sensor_params.isp_digital_gain;
}

static void _getEffWbGain(void* aiqctx, uint32_t fid, RkToolAwbParam_t* out)
{
#if defined(USE_NEWSTRUCT)
    aiq_isp_effect_params_t* ispParams = NULL;

    AiqCamHw_getEffectiveIspParams(((rk_aiq_sys_ctx_t*)aiqctx)->_camHw, &ispParams, fid);
    if (ispParams) {
        struct isp32_awb_gain_cfg* in = &ispParams->awb_gain_cfg;
        out->awb_gain_gb = (in->awb1_gain_gb >> 8) + (in->awb1_gain_gb & 0xFF) / 256.0f;
        out->awb_gain_gr = (in->awb1_gain_gr >> 8) + (in->awb1_gain_gr & 0xFF) / 256.0f;
        out->awb_gain_b = (in->awb1_gain_b >> 8) + (in->awb1_gain_b & 0xFF) / 256.0f;
        out->awb_gain_r = (in->awb1_gain_r >> 8) + (in->awb1_gain_r & 0xFF) / 256.0f;
        AIQ_REF_BASE_UNREF(&ispParams->_ref_base);
    }
#endif
}

rk_aiq_isp_tool_hdr_compr_curve_t* socket_client_get_hdrComprCurve(void* aiqctx) {
    rk_aiq_isp_tool_hdr_compr_curve_t* tool_compr_curve =
        aiq_mallocz(sizeof(rk_aiq_isp_tool_hdr_compr_curve_t));
    if (tool_compr_curve == NULL) {
        LOGE_IPC("malloc tool_compr_curve failed");
        return NULL;
    }

    XCamReturn ret        = XCAM_RETURN_NO_ERROR;
    RkAiqHdrCompr_t compr = {0};
    ret                   = rk_aiq_uapi2_sysctl_getHdrComprCurve((rk_aiq_sys_ctx_t*)aiqctx, &compr);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_IPC("get hdr compr curve failed, ret: %d\n", ret);
        if (tool_compr_curve) {
            aiq_free(tool_compr_curve);
        }
        return NULL;
    } else {
        tool_compr_curve->version = 0x0100;
        tool_compr_curve->point   = compr.point;
        tool_compr_curve->src_bit = compr.src_bit;
        tool_compr_curve->k_shift = compr.k_shift;
        memcpy(tool_compr_curve->data_compr, compr.data_compr, sizeof(compr.data_compr));
        memcpy(tool_compr_curve->data_src, compr.data_src, sizeof(compr.data_src));
        memcpy(tool_compr_curve->slope_k, compr.slope_k, sizeof(compr.slope_k));
    }

    return tool_compr_curve;
}

rk_aiq_isp_tool_stats_t *socket_client_get_isp_statics(void* aiqctx)
{
    rk_aiq_isp_statistics_t aiq_stats;
    rk_aiq_isp_tool_stats_t *tool_stats = aiq_mallocz(sizeof(rk_aiq_isp_tool_stats_t));

    XCamReturn ret = rk_aiq_uapi2_stats_getIspStats(aiqctx, &aiq_stats, 300);
    LOGI_IPC("%s: frameId: %d\n",__func__, tool_stats->frameID);
    if (ret == XCAM_RETURN_NO_ERROR) {
        tool_stats->version = 0x0101;
        tool_stats->frameID = aiq_stats.frame_id;
        copyRkAiqExpParamComb_t2RkToolExpParam_t(&aiq_stats.aec_stats.ae_exp.LinearExp, &tool_stats->linearExp);
        copyRkAiqExpParamComb_t2RkToolExpParam_t(&aiq_stats.aec_stats.ae_exp.HdrExp[0], &tool_stats->hdrExp[0]);
        copyRkAiqExpParamComb_t2RkToolExpParam_t(&aiq_stats.aec_stats.ae_exp.HdrExp[1], &tool_stats->hdrExp[1]);
        copyRkAiqExpParamComb_t2RkToolExpParam_t(&aiq_stats.aec_stats.ae_exp.HdrExp[2], &tool_stats->hdrExp[2]);
    } else {
        LOGE_IPC("%s: call rk_aiq_uapi2_stats_getIspStats error",  __func__);
    }

    _getEffWbGain(aiqctx, aiq_stats.frame_id, &tool_stats->awbGain);

    return tool_stats;
}

int socket_client_writeAwbIn(void* aiqctx, char* data)
{
    static int call_cnt = 0;
    rk_aiq_uapiV2_awb_wrtIn_attr_t attr;
    memset(&attr, 0, sizeof(rk_aiq_uapiV2_awb_wrtIn_attr_t));
    attr.en = true;
    attr.mode = 1; // 1 means rgb ,0 means raw
    attr.call_cnt = call_cnt++;
    sprintf(attr.path, "/tmp");
    LOGI_IPC("%s: data: %s\n",__func__, data);
    return rk_aiq_user_api2_awb_WriteAwbIn(aiqctx, attr);
}

int socket_client_enque_rkraw(void* aiqctx, char* data)
{
    LOGI_IPC("%s: file: %s\n",__func__, data);
    rk_aiq_uapi2_sysctl_enqueueRkRawFile(aiqctx, data);
    return 0;
}

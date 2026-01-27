
#include "aiq_btnrReproc.h"

#include "isp/rk_aiq_isp_btnr42.h"
#include "algos/rk_aiq_api_types_btnr42.h"

#include <sys/ioctl.h>
#include <linux/dma-buf.h>
#include <fcntl.h>

#define BTNR_WGT_REPROC_DEBUG 0

#ifdef BTNR_WGT_REPROC_DEBUG
static int _get_int_default_frome_file(const char *path)
{
    if (!path) {
        return -1;
    }

    char level[64] = {'\0'};
    FILE* fp = fopen(path, "r");

    if (!fp)
        return -1;

    fseek(fp, 0, SEEK_SET);
    if (fp) {
        fread(level, 63, 1, fp);
        fclose(fp);
        // LOGK("MEMC FILE %s", level);
        int res = atoi(level);
        return res;
    }

    return -1;

}

static void _btnr_wgt_debug_raw(const char* path, uint8_t* data, size_t size)
{
    if (!path || !data || size <= 0) {
        return;
    }

    FILE* fp = fopen(path, "wb+");
    if (fp) {
        fwrite(data, size, 1, fp);
        fclose(fp);
    }
}
#endif

static void bayertnr_reproc_ready(AiqBtnrDsReproc_t* pBtnrDsProc)
{
    AiqV4l2SubDevice_t* ispCoreDev = (AiqV4l2SubDevice_t*)pBtnrDsProc->_isp_subdev;
    if (!ispCoreDev) {
        return;
    }

    AiqV4l2SubDevice_ioctl((AiqV4l2Device_t*)ispCoreDev, RKISP_CMD_BTNR_WGT_READY, NULL);
}

static void bayertnr_picds_filter(int enable, int widvr, int width, int heigh, unsigned char *inbuff, unsigned char *otbuff, unsigned char *d0buff, int filtmode)
{
//#define CLIP(a, min_v, max_v)             (((a) < (min_v)) ? (min_v) : (((a) > (max_v)) ? (max_v) : (a)))
//#define MIN(a,b)                          ((a) <= (b) ? (a):(b))
//#define MAX(a,b)                          ((a) >= (b) ? (a):(b))
//virt width = (((width+31)/32 + 1)/2*2 + 3)/4*4;
//virt heigh =   (heigh+31)/32;
    // unsigned char *d0buff = (unsigned char *)malloc(widvr*heigh*2);
    int minfilt_fh, minfilt_fw;
    int maxfilt_fh, maxfilt_fw;
    int fh = 7, fw = 7;

    if(!enable)
    {
        for (int i = 0; i < heigh; i++)
        {
            for (int j = 0; j < width; j++)
            {
                otbuff[i * widvr + j] = inbuff[i * widvr + j];
            }
        }

        for (int j = 0; j < width; j++)
        {
            otbuff[(heigh - 2)*widvr + j] = otbuff[(heigh - 3) * widvr + j];
            otbuff[(heigh - 1)*widvr + j] = otbuff[(heigh - 2) * widvr + j];
        }
        return ;
    }

    for (int j = 0; j < width; j++)
    {
        inbuff[(heigh - 2)*widvr + j] = inbuff[(heigh - 3) * widvr + j];
        inbuff[(heigh - 1)*widvr + j] = inbuff[(heigh - 2) * widvr + j];
    }

    if(filtmode == 0)
    {
        fw = 3;
        fh = 3;
    }
    else if(filtmode == 1)
    {
        fw = 3;
        fh = 7;
    }
    else if(filtmode == 2)
    {
        fw = 5;
        fh = 5;
    }
    else if(filtmode == 3)
    {
        fw = 7;
        fh = 7;
    }
    minfilt_fh = fh;
    minfilt_fw = fw;
    maxfilt_fh = fh;
    maxfilt_fw = fw;

    //min
    for (int i = 0; i < heigh; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int y0 = minfilt_fh / 2;
            int x0 = minfilt_fw / 2;
            int minv = 4096, y, x;

            for (int ii = 0; ii < minfilt_fh; ii++)
            {
                for (int jj = 0; jj < minfilt_fw; jj++)
                {
                    y = CLIP((i + ii - y0), 0, heigh - 1);
                    x = CLIP((j + jj - x0), 0, width - 1);

                    minv = MIN(minv, inbuff[y * widvr + x]);
                }
            }

            d0buff[i * widvr + j] = minv;
        }
    }

    //max
    for (int i = 0; i < heigh; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int y0 = maxfilt_fh / 2;
            int x0 = maxfilt_fw / 2;
            int maxv = 0, y, x;

            for (int ii = 0; ii < maxfilt_fh; ii++)
            {
                for (int jj = 0; jj < maxfilt_fw; jj++)
                {
                    y = CLIP((i + ii - y0), 0, heigh - 1);
                    x = CLIP((j + jj - x0), 0, width - 1);

                    maxv = MAX(maxv, d0buff[y * widvr + x]);
                }
            }

            otbuff[i * widvr + j] = maxv;
        }
    }
}

static bool btnrDeepLoWgtThread_func(void *user_data)
{

    AiqBtnrDsReprocThread_t* pHdlTh = (AiqBtnrDsReprocThread_t*)user_data;
    AiqBtnrDsReproc_t* pBtnrDsProc = pHdlTh->mBtnrDsProc;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGW_ANALYZER("quit %s !", __func__);
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    aiq_btnrReprocMsg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    if (!pBtnrDsProc->_btnr2_en)
        return true;

    // LOGK("run here %s:%d frame_id %d addr %p", __func__, __LINE__, msg.frame_id, msg.buf_addr);

    if (msg.buf_addr && pBtnrDsProc->_doBtnrWgtReproc_en) {
        btnr_api_attrib_t* btnr_attr = (btnr_api_attrib_t*)pBtnrDsProc->_btnr_attr;
        int filter_mode = 0;
        btnr_params_static_t* psta = NULL;
        if (btnr_attr->opMode == RK_AIQ_OP_MODE_AUTO) {
            psta = &btnr_attr->stAuto.sta;
        } else {
            psta = &btnr_attr->stMan.sta;
        }
        btnr_dloWgtFilt_mode_t dloWgtFilt_mode = psta->dLoWgtFilt_mode;
        switch (dloWgtFilt_mode)
        {
        case btnr_dLoWgtFilt3x3_mode:
            /* code */
            filter_mode = 0;
            break;
        case btnr_dLoWgtFilt3x5_mode:
            filter_mode = 1;
            break;
        case btnr_dLoWgtFilt5x5_mode:
            filter_mode = 2;
            break;
        case btnr_dLoWgtFilt7x7_mode:
            filter_mode = 3;
            break;
        default:
            break;
        }

        // filter_mode = 3;

        // g_debug_flag--;

        // if (g_debug_flag == 0 || g_debug_flag == 200) {
        //     char path[128];
        //     snprintf(path, sizeof(path), "/tmp/btnr_wgt_in_%dx%d_%d.raw", pBtnrDsProc->_ds_width, pBtnrDsProc->_ds_height, msg.frame_id);
        //     _btnr_wgt_debug_raw(path, (uint8_t*)msg.buf_addr, pBtnrDsProc->_btnr_wgt_size);
        // }

        // LOGK("run here %d %s", __LINE__, timeString());
#ifdef BTNR_WGT_REPROC_DEBUG
        bool bypass_filt = false;
        int bypass_val = _get_int_default_frome_file("/tmp/btnr_wgt_bypass");
        if (bypass_val != -1) {
            bypass_filt = (bypass_val == 0) ? false : true;
        }
#endif
        // LOGK("run here %d %s fd %d", __LINE__, timeString(), msg.buf_fd);

        int dma_fd = msg.buf_fd;

        struct dma_buf_sync sync = { 0 };

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
        ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);


        bayertnr_picds_filter(!bypass_filt, pBtnrDsProc->_ds_width_vir, pBtnrDsProc->_ds_width, pBtnrDsProc->_ds_height,
                              (unsigned char *)msg.buf_addr, (unsigned char *)msg.buf_addr, pBtnrDsProc->_filter_buffer, filter_mode);


        // if (g_debug_flag == 0 || g_debug_flag == 200) {
        //     char path[128];
        //     snprintf(path, sizeof(path), "/tmp/btnr_wgt_out_%dx%d_%d.raw", pBtnrDsProc->_ds_width, pBtnrDsProc->_ds_height, msg.frame_id);
        //     _btnr_wgt_debug_raw(path, (uint8_t*)msg.buf_addr, pBtnrDsProc->_btnr_wgt_size);
        //     g_debug_flag--;
        // }

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
        ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

        pBtnrDsProc->_last_proc_frame_id = msg.frame_id;
        // LOGK("run here %s:%d frame_id %d MODE %d", __func__, __LINE__, msg.frame_id, filter_mode);

        // uint8_t *src_p = (uint8_t*)msg.buf_addr + (pBtnrDsProc->_ds_width * (pBtnrDsProc->_ds_height - 3));
        // uint8_t *dst_p = src_p + pBtnrDsProc->_ds_width;
        // uint8_t *dst_p2 = src_p + pBtnrDsProc->_ds_width * 2;
        // memcpy(dst_p, src_p, pBtnrDsProc->_ds_width);
        // memcpy(dst_p2, src_p, pBtnrDsProc->_ds_width);
    }

    if (pBtnrDsProc->_doBtnrWgtReproc_en || msg.frame_id < (pBtnrDsProc->_last_proc_frame_id + 5))
        bayertnr_reproc_ready(pBtnrDsProc);


    // LOGK("run here fmt: %ux%u ds fmt: ds_w %u ds_w_vir %u ds_h %u",
    //     pBtnrDsProc->_width, pBtnrDsProc->_height,
    //     pBtnrDsProc->_ds_width, pBtnrDsProc->_ds_width_vir, pBtnrDsProc->_ds_height);

    // LOGK("run here %d %s", __LINE__, timeString());
    // LOGK("run here %s:%d frame_id %d", __func__, __LINE__, msg.frame_id);

    return true;
}

static XCamReturn AiqBtnrDsReprocThread_init(AiqBtnrDsReprocThread_t* pHdlTh, AiqBtnrDsReproc_t* pBtnrDsProc)
{
    if (!pHdlTh || !pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    pHdlTh->mBtnrDsProc = pBtnrDsProc;

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "btnrDsThMsg";
    msgqCfg._item_nums = 5;
    msgqCfg._item_size = sizeof(aiq_btnrReprocMsg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pHdlTh->_base = aiqThread_init("fpnSwStreamProcTh", btnrDeepLoWgtThread_func, pHdlTh);
    if (!pHdlTh->_base) {
        LOGW_ANALYZER("create thread failed %s !", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqBtnrDsReprocThread_deinit(AiqBtnrDsReprocThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);

    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqBtnrDsReprocThread_start(AiqBtnrDsReprocThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGW_CAMHW("start thread failed %s !", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqBtnrDsReprocThread_stop(AiqBtnrDsReprocThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqThread_stop(pHdlTh->_base);

    return XCAM_RETURN_NO_ERROR;
}

static bool AiqBtnrDsReprocThread_push_msg(AiqBtnrDsReprocThread_t* pHdlTh, aiq_btnrReprocMsg_t* msg)
{
    if (!pHdlTh || !msg || !pHdlTh->_base->_started)
        return false;

    int ret = aiqList_push(pHdlTh->mMsgsQueue, msg);
    if (ret) {
        LOGW_CAMHW("push msg failed %s !", __func__);
        return false;
    }

    aiqCond_broadcast(&pHdlTh->_cond);

    return true;
}

XCamReturn AiqBtnrDsReproc_pushMsg(AiqBtnrDsReproc_t* pBtnrDsProc, void *stats)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (!stats)
        return XCAM_RETURN_ERROR_PARAM;

    struct rkisp35_stat_buffer *pStats = (struct rkisp35_stat_buffer *)stats;

    if (!(pStats->meas_type & ISP35_STAT_BAY3D)) {
        return XCAM_RETURN_BYPASS;
    }

    aiq_btnrReprocMsg_t msg;
    msg.buf_fd   = pBtnrDsProc->_btnr_wgt_fd_array[pStats->stat.buf_bay3d_wgt_index];
    msg.buf_addr = pBtnrDsProc->_btnr_wgt_addr_array[pStats->stat.buf_bay3d_wgt_index];
    msg.frame_id = pStats->frame_id;

    if (!AiqBtnrDsReprocThread_push_msg(pBtnrDsProc->_procTh, &msg)) {
        LOGE_ANALYZER("push msg failed %s !", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_setCalib(AiqBtnrDsReproc_t* pBtnrDsProc, const CamCalibDbV2Context_t *calib)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (!calib)
        return XCAM_RETURN_ERROR_PARAM;

    pBtnrDsProc->_btnr_attr =
        (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(calib), bayertnr));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_setBtnrWgtBuf(AiqBtnrDsReproc_t* pBtnrDsProc, void *buf_addr, uint32_t buf_size, int *buf_fd)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (!buf_addr)
        return XCAM_RETURN_ERROR_PARAM;

    memcpy(pBtnrDsProc->_btnr_wgt_fd_array, buf_fd, sizeof(pBtnrDsProc->_btnr_wgt_fd_array));
    memcpy(pBtnrDsProc->_btnr_wgt_addr_array, buf_addr, sizeof(pBtnrDsProc->_btnr_wgt_addr_array));
    pBtnrDsProc->_btnr_wgt_size = buf_size;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqBtnrDsReproc_updateEn(AiqBtnrDsReproc_t* pBtnrDsProc, bool en)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    pBtnrDsProc->_doBtnrWgtReproc_en = en;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_init(AiqBtnrDsReproc_t* pBtnrDsProc, void *isp_subdev)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    pBtnrDsProc->_procTh = (AiqBtnrDsReprocThread_t*)aiq_mallocz(sizeof(AiqBtnrDsReprocThread_t));
    if (!pBtnrDsProc->_procTh) {
        LOGE_ANALYZER("init %s error", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pBtnrDsProc->_doBtnrWgtReproc_en = false;
    pBtnrDsProc->_last_proc_frame_id = 0;
    pBtnrDsProc->_btnr2_en           = false;

    XCamReturn ret = AiqBtnrDsReprocThread_init(pBtnrDsProc->_procTh, pBtnrDsProc);
    if (ret) {
        LOGE_ANALYZER("init %s error", __func__);
        return ret;
    }

    pBtnrDsProc->_isp_subdev = isp_subdev;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_deinit(AiqBtnrDsReproc_t* pBtnrDsProc)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (pBtnrDsProc->_filter_buffer) {
        aiq_free(pBtnrDsProc->_filter_buffer);
        pBtnrDsProc->_filter_buffer = NULL;
    }

    if (pBtnrDsProc->_procTh) {
        AiqBtnrDsReprocThread_deinit(pBtnrDsProc->_procTh);
        aiq_free(pBtnrDsProc->_procTh);
        pBtnrDsProc->_procTh = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_prepare(AiqBtnrDsReproc_t* pBtnrDsProc, uint32_t width, uint32_t height)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    pBtnrDsProc->_width = width;
    pBtnrDsProc->_height = height;
    pBtnrDsProc->_ds_width = (width + 31) / 32;
    pBtnrDsProc->_ds_width_vir = (((width + 31) / 32 + 1) / 2 * 2 + 3) / 4 * 4;
    pBtnrDsProc->_ds_height = (height + 31) / 32;

    if (pBtnrDsProc->_filter_buffer) {
        aiq_free(pBtnrDsProc->_filter_buffer);
        pBtnrDsProc->_filter_buffer = NULL;
    }
    pBtnrDsProc->_filter_buffer = aiq_mallocz(pBtnrDsProc->_ds_width_vir * pBtnrDsProc->_ds_height * 2);
    if (!pBtnrDsProc->_filter_buffer) {
        LOGE_ANALYZER("malloc filter buffer error");
        return XCAM_RETURN_ERROR_MEM;
    }

    if (pBtnrDsProc->_btnr_attr) {
        btnr_api_attrib_t* btnr_attr = (btnr_api_attrib_t*)pBtnrDsProc->_btnr_attr;
        if (btnr_attr->stAuto.sta.dLoMd_mode == btnr_dLoMdCurOrPre_mode) {
            pBtnrDsProc->_btnr2_en = true;
        } else {
            pBtnrDsProc->_btnr2_en = false;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqBtnrDsReproc_start(AiqBtnrDsReproc_t* pBtnrDsProc)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    return AiqBtnrDsReprocThread_start(pBtnrDsProc->_procTh);
}

XCamReturn AiqBtnrDsReproc_stop(AiqBtnrDsReproc_t* pBtnrDsProc)
{
    if (!pBtnrDsProc)
        return XCAM_RETURN_ERROR_PARAM;

    return AiqBtnrDsReprocThread_stop(pBtnrDsProc->_procTh);
}
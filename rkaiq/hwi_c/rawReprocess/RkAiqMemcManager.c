
#include "RkAiqMemcManager.h"
#include "hwi_c/rawReprocess/gyro_memc.h"
#include "hwi_c/aiq_CamHwBase.h"
#include <sys/ioctl.h>
#include <linux/dma-buf.h>
#include <fcntl.h>

#ifdef RKAIQ_HAVE_BAYERTNR_V42
#include "algos/rk_aiq_api_types_btnr42.h"
#endif

#define RKMEMC_DEBUG 0

static int g_no_mesh_cnt = 0;
static int g_loss_frame  = 0;

typedef enum AiqMemcParamMsgType_e
{
    RKAIQ_MEMC_MESH_INFO = 0,
    RKAIQ_MEMC_ISP_PARAM,
    RKAIQ_MEMC_MSG_MAX,
} AiqMemcParamMsgType_t;

typedef enum AiqMemcWorkingMode_e
{
    RKAIQ_MEMC_SYNC_BASE_IMU = 0,
    RKAIQ_MEMC_PRE_BASE_IMU,
    RKAIQ_MEMC_AI_BASE_IMG,
} AiqMemcWorkingMode_t;

static XCamReturn _setIspParamToDrv(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* pV4l2Buf, const rk_aiq_btnr_share_mem_info_t *mesh_info);
static bool AiqMemcIirThread_push_msg(AiqMemcIirThread_t* pHdlTh, MemcIirMsg_t* msg);

static void LutBufferManagerReleaseHwBuffers(AiqMemcManager_t *man, uint8_t isp_id) {
    if (man->mem_ctx_ != NULL && man->mem_ops_ != NULL)
        man->mem_ops_->release_mem(isp_id, man->mem_ctx_);

    man->_import_buffer = false;
}

static const rk_aiq_btnr_share_mem_info_t* LutBufferManagerGetFreeHwBuffer(AiqMemcManager_t *man, uint8_t isp_id) {
    if (man->mem_ops_ == NULL || man->mem_ctx_ == NULL) {
        return NULL;
    }

    const rk_aiq_btnr_share_mem_info_t* mem_info = (const rk_aiq_btnr_share_mem_info_t*)(
        man->mem_ops_->get_free_item(isp_id, man->mem_ctx_));
    if (mem_info) {
        *mem_info->state = MESH_BUF_WAIT2CHIP;
    }

    return mem_info;
}

static bool AiqMemcParamsThread_loop(void *user_data)
{
    AiqMemcParamsThread_t* pHdlTh = (AiqMemcParamsThread_t*)user_data;
    AiqMemcManager_t* pMemcMan      = pHdlTh->mMemcMan;

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

    AiqMemcParamsMsg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    uint32_t userId = msg.frameId;
    AiqMemcParamsArray_t msgWrapper;
    aiq_memset(&msgWrapper, 0, sizeof(msgWrapper));
    AiqMemcParamsArray_t* pMsgWrapper = NULL;
    bool bInsert                      = false;
    AiqMapItem_t* pItem               = aiqMap_get(pHdlTh->mParamsMap, (void*)(intptr_t)userId);

    if (pItem) {
        pMsgWrapper = (AiqMemcParamsArray_t*)(pItem->_pData);
        bInsert     = false;
    } else {
        bInsert     = true;
        pMsgWrapper = &msgWrapper;
    }

    pMsgWrapper->mark |= 1 << msg.type;
    if (msg.type == RKAIQ_MEMC_MESH_INFO) {
        pMsgWrapper->mesh = (const rk_aiq_btnr_share_mem_info_t *)msg.data;
    } else {
        pMsgWrapper->param_buf = (AiqV4l2Buffer_t*)msg.data;
    }

    // LOGK("run here %s:%d msg id %d type %d", __func__, __LINE__, msg.frameId, msg.type);
    AiqCamHwBase_t* pCamHw    = pMemcMan->mCamHw;
    if (!(pMsgWrapper->mark ^ 0x3)) {

        AiqV4l2Buffer_t* pV4l2Buf = pMsgWrapper->param_buf;
        const rk_aiq_btnr_share_mem_info_t *mesh_info = pMsgWrapper->mesh;
        if (pV4l2Buf && mesh_info) {
            aiqMutex_lock(&pCamHw->_stop_cond_mutex);
            if (pCamHw->_isp_stream_status != ISP_STREAM_STATUS_STREAM_OFF) {
                _setIspParamToDrv(pMemcMan, pV4l2Buf, mesh_info);
            } else {
                *(mesh_info->state) = MESH_BUF_INIT;
                AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
            }
            aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
        } else if (mesh_info) {
            *(mesh_info->state) = MESH_BUF_INIT;
            LOGE_ANR("frame id %d b3dldc mesh no ready, no set to drv line %d", userId, __LINE__);
            ret = XCAM_RETURN_ERROR_FAILED;
        } else if (pV4l2Buf) {
            AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
            LOGE_ANR("frame id %d param buf is no ready, no set to drv line %d %p", userId, __LINE__, pV4l2Buf);
            ret = XCAM_RETURN_ERROR_FAILED;
        }

        pItem               = NULL;
        bool rm             = false;
        AIQ_MAP_FOREACH(pHdlTh->mParamsMap, pItem, rm) {
            if ((uint32_t)(long)pItem->_key <= userId) {
                if ((uint32_t)(long)pItem->_key < userId) {
                    AiqMemcParamsArray_t* pMsg = (AiqMemcParamsArray_t*)(pItem->_pData);
                    if (pMsg->mesh) {
                        *(pMsg->mesh->state) = MESH_BUF_INIT;
                        LOGE_ANR("frame id %d b3dldc mesh no ready, no set to drv line %d", pMsg->frameId, __LINE__);
                        ret = XCAM_RETURN_ERROR_FAILED;
                    }
                    if (pMsg->param_buf) {
                        AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pMsg->param_buf);
                        LOGE_ANR("frame id %d param buf is no ready, no set to drv line %d %p", pMsg->frameId, __LINE__, pMsg->param_buf);
                        ret = XCAM_RETURN_ERROR_FAILED;
                    }
                }

                pItem          = aiqMap_erase_locked(pHdlTh->mParamsMap, pItem->_key);
                rm             = true;
            } else
                break;
        }
        pItem = NULL;
        rm    = false;
    } else {
        if (bInsert) aiqMap_insert(pHdlTh->mParamsMap, (void*)(intptr_t)userId, pMsgWrapper);
        if (aiqMap_size(pHdlTh->mParamsMap) > 2) {
            bool rm = false;
            pItem = NULL;
            AIQ_MAP_FOREACH(pHdlTh->mParamsMap, pItem, rm) {
                AiqMemcParamsArray_t* pMsg = (AiqMemcParamsArray_t*)(pItem->_pData);
                if (pMsg->mesh) {
                    *(pMsg->mesh->state) = MESH_BUF_INIT;
                    LOGE_ANR("frame id %d b3dldc mesh no ready, no set to drv line %d", pMsg->frameId, __LINE__);
                }
                if (pMsg->param_buf) {
                    AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pMsg->param_buf);
                    LOGE_ANR("frame id %d param buf is no ready, no set to drv line %d %p", pMsg->frameId, __LINE__, pMsg->param_buf);
                }

                pItem          = aiqMap_erase_locked(pHdlTh->mParamsMap, pItem->_key);
                rm             = true;
                if (aiqMap_size(pHdlTh->mParamsMap) == 2) break;
            }
        }
    }
    // bool rm = false;
    // AIQ_MAP_FOREACH(pHdlTh->mParamsMap, pItem, rm) {
    //     LOGK("run here %s:%d frameId %d", __func__, __LINE__, (uint32_t)(long)pItem->_key);
    // }
    return true;
}

static XCamReturn
AiqMemcParamsThread_init(AiqMemcParamsThread_t* pHdlTh, char* name,
    AiqMemcManager_t* pMemcMan)
{
    pHdlTh->mMemcMan = pMemcMan;

    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "memcParamsMsg";
    msgqCfg._item_nums = 5;
    msgqCfg._item_size = sizeof(AiqMemcParamsMsg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    AiqMapConfig_t pendingParamsCfg;
    pendingParamsCfg._name          = "memcParamsMap";
    pendingParamsCfg._key_type		= AIQ_MAP_KEY_TYPE_UINT32;
    pendingParamsCfg._item_nums     = 3;
    pendingParamsCfg._item_size     = sizeof(AiqMemcParamsArray_t);
    pHdlTh->mParamsMap              = aiqMap_init(&pendingParamsCfg);
    if (!pHdlTh->mParamsMap)
        LOGE_ANALYZER("init %s error", pendingParamsCfg._name);


    pHdlTh->_base = aiqThread_init(name, AiqMemcParamsThread_loop, pHdlTh);
    if (!pHdlTh->_base) goto fail;


    return XCAM_RETURN_NO_ERROR;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

static void AiqMemcParamsThread_deinit(AiqMemcParamsThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);
    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);

    aiqMap_deinit(pHdlTh->mParamsMap);
    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcParamsThread_start(AiqMemcParamsThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    // aiqThread_setCpuCore(pHdlTh->_base, 3);
    aiqThread_setPolicy(pHdlTh->_base, SCHED_RR);
    aiqThread_setPriority(pHdlTh->_base, 99);
    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGE_ANALYZER("start %s error", pHdlTh->_base->_name);
    }
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcParamsThread_stop(AiqMemcParamsThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqThread_stop(pHdlTh->_base);
    EXIT_ANALYZER_FUNCTION();
}

static bool AiqMemcParamsThread_push_msg(AiqMemcParamsThread_t* pHdlTh, AiqMemcParamsMsg_t* msg) {
    ENTER_ANALYZER_FUNCTION();
    if (!pHdlTh->_base->_started) {
        return false;
    }
    int ret = aiqList_push(pHdlTh->mMsgsQueue, msg);
    if (ret) {
        LOGE_ANALYZER("push %s error", pHdlTh->_base->_name);
        return false;
    }
    aiqCond_broadcast(&pHdlTh->_cond);
    return true;
}

static bool algoMemcIIrSyncCacheTh_func(void *user_data)
{
    AiqMemcIirThread_t* pHdlTh = (AiqMemcIirThread_t*)user_data;
    AiqMemcManager_t* pMemcMan = pHdlTh->mMemcMan;

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

    MemcIirMsg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    int dma_fd = pMemcMan->iir_fd[msg.iir_index];

    struct dma_buf_sync sync = { 0 };

    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);


    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
    ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

#if RKMEMC_DEBUG
    if (msg.frame_id >= 300 && msg.frame_id % 100 == 0 && msg.frame_id <= 1000) {
        if (pMemcMan->iir_address[msg.iir_index]) {
            // if (msg.frame_id < pHdlTh->mDebugFrame) {
                char f_p[128] = {0};
                sprintf(f_p, "/tmp/memc_iir_%d.raw", msg.frame_id);
                FILE *fp = fopen(f_p, "wb+");
                if (fp) {
                    fwrite(pMemcMan->iir_address[msg.iir_index], pMemcMan->iir_size, 1, fp);
                    fclose(fp);
                }
            // }
        }
    }
#endif

    return true;
}

static bool algoMemcIirTh_func(void *user_data)
{
    AiqMemcIirThread_t* pHdlTh = (AiqMemcIirThread_t*)user_data;
    AiqMemcManager_t* pMemcMan = pHdlTh->mMemcMan;

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

    MemcIirMsg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    if (msg.frame_id > pHdlTh->mNewIirId) {
        pHdlTh->mIirPkWarpEn = pMemcMan->mIirPkWarp;
        pHdlTh->mNewIirId = msg.frame_id;
        pHdlTh->mNewIirEvtCnt = 1;
#ifdef RKAIQ_MEMC_PK_WARP_DEBUG
        pHdlTh->mWarpTimeTotal += pHdlTh->mWarpTime;
        pHdlTh->mWarpTime = 0.0;
        // gettimeofday(&pHdlTh->start, NULL);
#endif
    } else if (msg.frame_id == pHdlTh->mNewIirId) {
        pHdlTh->mNewIirEvtCnt++;
    } else {
        LOGE("error IIR msg %d newIIR %d", msg.frame_id, pHdlTh->mNewIirId);
        return true;
    }

    if (pHdlTh->mIirPkWarpEn) {
#ifdef RKAIQ_MEMC_PK_WARP_DEBUG
        struct timeval start, end;
        gettimeofday(&start, NULL);
#endif
        rk_gyro_memc_param_t in;
        in = pMemcMan->_param;
        in.imu_data_num = 0;
        in.cur_frame_ptr = NULL;
        in.ref_frame_ptr = NULL;
        in.cur_frame_ptr = pMemcMan->iir_address[msg.iir_index];
        in.frame_idx = msg.frame_id;
        in.event_idx = pHdlTh->mNewIirEvtCnt - 1;
        in.effect_line_cnt = pHdlTh->mNewIirEvtCnt * pMemcMan->mAiispCfg.wr_linecnt == pMemcMan->Height ?
                            pMemcMan->Height : pHdlTh->mNewIirEvtCnt * pMemcMan->mAiispCfg.wr_linecnt;
        in.pk_ptr = (uint16_t*)(in.cur_frame_ptr + pMemcMan->pk_attr_off);

        rk_gyro_memc_pk_warp(pMemcMan->_algo_ctx, &in);

#ifdef RKAIQ_MEMC_PK_WARP_DEBUG
        gettimeofday(&end, NULL);
#endif

        if (pHdlTh->mNewIirId == msg.frame_id && pHdlTh->mNewIirEvtCnt == 4) {
#ifdef RKAIQ_MEMC_PK_WARP_DEBUG
            int64_t total = 0;
            int i = 0;
            for (i = 0; i < in.effect_line_cnt * 500; i++) {
                total += in.pk_ptr[i];
            }
            LOGK("run here %s:%d total %lld i %d", __func__, __LINE__, total, i);
            gettimeofday(&pHdlTh->end, NULL);
            LOGK_ANR("run here %s:%d id %d time %s", __func__, __LINE__, msg.frame_id, timeString());
#endif
            // aiqMutex_lock(&pMemcMan->mProcTh._iir_mutex);
            // pMemcMan->_last_iir_id = msg.frame_id;
            // aiqMutex_unlock(&pMemcMan->mProcTh._iir_mutex);
            // aiqCond_broadcast(&pMemcMan->mProcTh._iir_cond);

            AiqMemcIirThread_push_msg(&pMemcMan->mIirSyncTh, &msg);
        }
#ifdef RKAIQ_MEMC_PK_WARP_DEBUG
        double cpu_time_used = (double)((end.tv_sec - start.tv_sec) * 1000.0 + 
                                (end.tv_usec - start.tv_usec) / 1000.0);
        pHdlTh->mWarpTime += cpu_time_used;
        double avg_time = pHdlTh->mWarpTimeTotal / pHdlTh->mNewIirId;

        LOGK("run here %s:%d id %d pk warp time %f total %f avg %f",
            __func__, __LINE__, msg.frame_id, cpu_time_used, pHdlTh->mWarpTime, avg_time);
#endif
    }
    // else if (pHdlTh->mNewIirId == msg.frame_id && pHdlTh->mNewIirEvtCnt == 4) {
    //     pMemcMan->_last_iir_id = msg.frame_id;
    // }


    return true;
}

static XCamReturn
AiqMemcIirSyncThread_init(AiqMemcIirThread_t* pHdlTh, char* name,
    AiqMemcManager_t* pMemcMan)
{
    pHdlTh->mMemcMan = pMemcMan;

    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    pHdlTh->mDebugFrame = 0;
    char* valueStr = getenv("MEMC_IIR_DEBUG_FRAME");
    if (valueStr) {
        pHdlTh->mDebugFrame = strtoull(valueStr, NULL, 10);
    }

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "memcIirSyncThMsg";
    msgqCfg._item_nums = 3;
    msgqCfg._item_size = sizeof(MemcIirMsg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    pHdlTh->_base = aiqThread_init(name, algoMemcIIrSyncCacheTh_func, pHdlTh);
    if (!pHdlTh->_base) goto fail;

    return XCAM_RETURN_NO_ERROR;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn
AiqMemcIirThread_init(AiqMemcIirThread_t* pHdlTh, char* name,
    AiqMemcManager_t* pMemcMan)
{
    pHdlTh->mMemcMan = pMemcMan;

    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    pHdlTh->mDebugFrame = 0;
    char* valueStr = getenv("MEMC_IIR_DEBUG_FRAME");
    if (valueStr) {
        pHdlTh->mDebugFrame = strtoull(valueStr, NULL, 10);
    }

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "memcIirThMsg";
    msgqCfg._item_nums = 5;
    msgqCfg._item_size = sizeof(MemcIirMsg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    pHdlTh->_base = aiqThread_init(name, algoMemcIirTh_func, pHdlTh);
    if (!pHdlTh->_base) goto fail;

    pHdlTh->mIirPkWarpEn = pMemcMan->mIirPkWarp;
    pHdlTh->mWarpTimeTotal = 0.0;

    return XCAM_RETURN_NO_ERROR;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

static void AiqMemcIirThread_deinit(AiqMemcIirThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);
    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);

    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcIirThread_start(AiqMemcIirThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    // aiqThread_setCpuCore(pHdlTh->_base, 3);
    aiqThread_setPolicy(pHdlTh->_base, SCHED_RR);
    aiqThread_setPriority(pHdlTh->_base, 99);
    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGE_ANALYZER("start %s error", pHdlTh->_base->_name);
    }
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcIirThread_stop(AiqMemcIirThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqThread_stop(pHdlTh->_base);
    EXIT_ANALYZER_FUNCTION();
}

static bool AiqMemcIirThread_push_msg(AiqMemcIirThread_t* pHdlTh, MemcIirMsg_t* msg) {
    ENTER_ANALYZER_FUNCTION();
    if (!pHdlTh->_base->_started) {
        return false;
    }
    int ret = aiqList_push(pHdlTh->mMsgsQueue, msg);
    if (ret) {
        LOGE_ANALYZER("push %s error", pHdlTh->_base->_name);
        return false;
    }
    aiqCond_broadcast(&pHdlTh->_cond);
    return true;
}

#if RKMEMC_DEBUG

static void _write_mesh_bin_to_ddr(char *p, void* data, int size)
{
    FILE *fp = fopen(p, "wb+");
    if (fp) {
        fwrite(data, size, 1, fp);
        fclose(fp);
    }
}

static int _get_int_default_frome_file(const char *path)
{
    if (!path) {
        return 0;
    }

    char level[64] = {'\0'};
    FILE* fp = fopen(path, "r");

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_SET);
    if (fp) {
        fread(level, 63, 1, fp);
        fclose(fp);
        // LOGK("MEMC FILE %s", level);
        int res = atoi(level);
        return res;
    }
    return 0;

}

#endif

static bool algoMemcProcTh_func(void *user_data)
{
    AiqMemcProcThread_t* pHdlTh = (AiqMemcProcThread_t*)user_data;
    AiqMemcManager_t* pMemcMan = pHdlTh->mMemcMan;

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

    memc_callback_result_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    // struct timeval start, end;
    // gettimeofday(&start, NULL);

    rk_gyro_memc_param_t* in_param = &pMemcMan->_param;

    in_param->imu_data_num = 0;
    if (!aiqList_empty(pMemcMan->mImuDataList)) {
        AiqListItem_t* pItem   = NULL;
        AiqImuData_t*  pData   = NULL;
        bool rm                = false;
        in_param->imu_data_num = 0;

        aiqMutex_lock(&pMemcMan->mImuDataList->_mutex);
        AIQ_LIST_FOREACH(pMemcMan->mImuDataList, pItem, rm) {
            pData = (AiqImuData_t*)pItem->_pData;
            in_param->imu_data_external_list[in_param->imu_data_num].timeStamp   = (double)pData->s64Timestamp;
            in_param->imu_data_external_list[in_param->imu_data_num].dTemp       = pData->dTemp;
            in_param->imu_data_external_list[in_param->imu_data_num].gyroData[0] = pData->dGyroData[0];
            in_param->imu_data_external_list[in_param->imu_data_num].gyroData[1] = pData->dGyroData[1];
            in_param->imu_data_external_list[in_param->imu_data_num].gyroData[2] = pData->dGyroData[2];
            in_param->imu_data_external_list[in_param->imu_data_num].accData[0]  = pData->dAccData[0];
            in_param->imu_data_external_list[in_param->imu_data_num].accData[1]  = pData->dAccData[1];
            in_param->imu_data_external_list[in_param->imu_data_num].accData[2]  = pData->dAccData[2];
            pItem = aiqList_erase_item_locked(pMemcMan->mImuDataList, pItem);
            rm    = true;
            in_param->imu_data_num++;
        }
        aiqMutex_unlock(&pMemcMan->mImuDataList->_mutex);
    }

    msg.mesh_info = LutBufferManagerGetFreeHwBuffer(pMemcMan, 0);

    if (msg.mesh_info) {
        in_param->map_fix_bit_y = 4;
        in_param->map_fix_bit_x = 4;
        in_param->raw_wid = pMemcMan->Width;
        in_param->raw_hgt = pMemcMan->Height;
        if (in_param->raw_wid > 4080)
            in_param->map_fix_bit_x = 3;
        if (in_param->raw_hgt > 4080)
            in_param->map_fix_bit_y = 3;

        in_param->warp_extBound_set  = 0x0;
        in_param->rk_b3dldc_itp_mode = 1;
        in_param->b3dldc_offset = 0;

        if (pMemcMan->btnr_attr) {
            btnr_swMemc_static_t *imu_info_sta = NULL;
            if (pMemcMan->btnr_attr->opMode == RK_AIQ_OP_MODE_AUTO) {
                imu_info_sta = &pMemcMan->btnr_attr->stAuto.sta.swMemc;
            } else {
                imu_info_sta = &pMemcMan->btnr_attr->stMan.sta.swMemc;
            }
            in_param->enable_default_map = imu_info_sta->sw_btnr_swMemc_bypass;
        }

        if (msg.expParam) {
            in_param->again    = msg.expParam->aecExpInfo.LinearExp.exp_real_params.analog_gain;
            in_param->ispgain  = msg.expParam->aecExpInfo.LinearExp.exp_real_params.isp_dgain;
            in_param->dgain    = msg.expParam->aecExpInfo.LinearExp.exp_real_params.digital_gain;
            in_param->exp_time = msg.expParam->aecExpInfo.LinearExp.exp_real_params.integration_time;
        }
        in_param->rs_skew          = (double)pMemcMan->_rolling_shutter_skew / 1000.0 / 1000.0;
        in_param->camera_timestamp = (double)msg.sof_time;

        in_param->frame_idx        = msg.frame_id;
        in_param->map_mesh_fix_ptr = msg.mesh_info->addr;
        in_param->mapX_start_fix_ptr = msg.mesh_info->addr1;
        // in_param->cur_frame_ptr = (uint8_t*)AiqV4l2Buffer_getExpbufUsrptr(msg.buf_s);
        // if (pMemcMan->pre_buf_s)
        //     in_param->ref_frame_ptr = (uint8_t*)AiqV4l2Buffer_getExpbufUsrptr(pMemcMan->pre_buf_s);

        // LOGK("run here %s:%d msg fd %d mesh info %p", __func__, __LINE__, msg.mesh_info->fd, msg.mesh_info->addr);
        struct dma_buf_sync sync = { 0 };
        int dma_fd = msg.mesh_info->fd;

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_START;
        ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

#if RKMEMC_DEBUG
        // char f_p[128] = {0};
        // sprintf(f_p, "/tmp/mesh_map_%d", msg.frame_id);
        // int hsize  = (((pMemcMan->Width + 15) / 16 + 1) + 1) / 2;
        // int vsize  = (pMemcMan->Height + 7) / 8 + 1;
        // _write_mesh_bin_to_ddr(f_p, msg.mesh_info->addr, hsize * vsize * 4);
        // sprintf(f_p, "/tmp/meshX_map_%d", msg.frame_id);
        // hsize  = (((pMemcMan->Width + 15) / 16 + 1) + 1) / 2;
        // vsize  = (pMemcMan->Height + 15) / 16 + 2;
        // _write_mesh_bin_to_ddr(f_p, msg.mesh_info->addr1, hsize * vsize * 4);
#endif

        if (pMemcMan->_algo_ctx) {
            // in_param->enable_default_map = 1;
            rk_gyro_memc_proc(pMemcMan->_algo_ctx, in_param);
        }

        // gettimeofday(&end, NULL);

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_END;
        ioctl(dma_fd, DMA_BUF_IOCTL_SYNC, &sync);
        // LOGK("run here %s:%d msg fd %d mesh info %p", __func__, __LINE__, msg.mesh_info->fd, msg.mesh_info->addr);
    } else {
        g_no_mesh_cnt++;
    }

    if (msg.expParam) {
        AIQ_REF_BASE_UNREF(&msg.expParam->_base._ref_base);
    }

    if (pMemcMan->_memc_mode == RKAIQ_MEMC_PRE_BASE_IMU) {
        msg.frame_id++;
        AiqMemcManager_result_callback(pMemcMan, &msg);
    } else if (pMemcMan->_memc_mode == RKAIQ_MEMC_SYNC_BASE_IMU) {
        // proc
        if (pMemcMan->pre_buf_s) {
            msg.frame_id = AiqV4l2Buffer_getSequence(pMemcMan->pre_buf_s);
            ret = AiqMemcManager_result_callback(pMemcMan, &msg);
        } else if (msg.mesh_info) {
            *(msg.mesh_info->state) = MESH_BUF_INIT;
        }

        if (msg.frame_id - pMemcMan->_last_proc_id > 1) {
            g_loss_frame++;
        }

        // if (pMemcMan->pre_buf_s && pMemcMan->mIirPkWarp) {
        //     LOGK_CAMHW("run here %s:%d id %d iir id %d proc id %d", __func__, __LINE__, AiqV4l2Buffer_getSequence(pMemcMan->pre_buf_s), pMemcMan->_last_iir_id, pMemcMan->_last_proc_id);
        //     aiqMutex_lock(&pHdlTh->_iir_mutex);
        //     while (pMemcMan->_last_iir_id < pMemcMan->_last_proc_id && !pHdlTh->bQuit) {
        //         // LOGK_CAMHW("run here %s:%d id %d iir id %d proc id %d", __func__, __LINE__, AiqV4l2Buffer_getSequence(pMemcMan->pre_buf_s), pMemcMan->_last_iir_id, pMemcMan->_last_proc_id);
        //         aiqCond_wait(&pHdlTh->_iir_cond, &pHdlTh->_iir_mutex);
        //     }
        //     aiqMutex_unlock(&pHdlTh->_iir_mutex);
        // }

        if (pMemcMan->_proc_stream && pMemcMan->pre_buf_s != NULL) {
            if (ret == XCAM_RETURN_NO_ERROR || msg.frame_id < 3 || 1) {
                pMemcMan->_last_proc_id = AiqV4l2Buffer_getSequence(pMemcMan->pre_buf_s);
                // LOGK_ANR("run here %s:%d id %d time %s ", __func__, __LINE__, msg.frame_id, timeString());
                AiqRawStreamProcUnit_send_sync_buf(pMemcMan->_proc_stream, pMemcMan->pre_buf_s, pMemcMan->pre_buf_m,
                                                pMemcMan->pre_buf_l);
                // LOGK_ANR("run here %s:%d id %d iir id %d proc id %d", __func__, __LINE__, AiqV4l2Buffer_getSequence(pMemcMan->pre_buf_s), pMemcMan->_last_iir_id, pMemcMan->_last_proc_id);
            } else {
                LOGE_ANR("%s: frame id %d disorder", __func__, msg.frame_id);
            }
        }
    }

    // struct timeval end2;
    // gettimeofday(&end2, NULL);

    // double cpu_time_used = (double)((end.tv_sec - start.tv_sec) * 1000.0 + 
    //                         (end.tv_usec - start.tv_usec) / 1000.0);
    // double cpu_time_used2 = (double)((end2.tv_sec - start.tv_sec) * 1000.0 + 
    //        (end2.tv_usec - start.tv_usec) / 1000.0);

    // LOGK_ANR("run here %s:%d id %d time use %f %f loss mesh %d frm %d",
    //     __func__, __LINE__, msg.frame_id, cpu_time_used, cpu_time_used2, g_no_mesh_cnt, g_loss_frame);

    if (pMemcMan->pre_buf_s) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_s);
        pMemcMan->pre_buf_s = NULL;
    }
    if (pMemcMan->pre_buf_m) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_m);
        pMemcMan->pre_buf_m = NULL;
    }
    if (pMemcMan->pre_buf_l) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_l);
        pMemcMan->pre_buf_l = NULL;
    }

    pMemcMan->pre_buf_s = msg.buf_s;
    pMemcMan->pre_buf_m = msg.buf_m;
    pMemcMan->pre_buf_l = msg.buf_l;

    return true;
}

static XCamReturn
AiqMemcProcThread_init(AiqMemcProcThread_t* pHdlTh, char* name,
    AiqMemcManager_t* pMemcMan)
{
    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    aiqMutex_init(&pHdlTh->_iir_mutex);
    aiqCond_init(&pHdlTh->_iir_cond);

    pHdlTh->mMemcMan = pMemcMan;

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "memcThMsg";
    msgqCfg._item_nums = 6;
    msgqCfg._item_size = sizeof(memc_callback_result_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    msgqCfg._name      = "memcSclMsg";
    msgqCfg._item_nums = 2;
    msgqCfg._item_size = sizeof(AiqV4l2Buffer_t*);
    pHdlTh->mSclList = aiqList_init(&msgqCfg);
    if (!pHdlTh->mSclList) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        goto fail;
    }

    pHdlTh->_base = aiqThread_init(name, algoMemcProcTh_func, pHdlTh);
    if (!pHdlTh->_base) goto fail;

    return XCAM_RETURN_NO_ERROR;

fail:
    return XCAM_RETURN_ERROR_FAILED;
}

static void AiqMemcProcThread_deinit(AiqMemcProcThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);
    if (pHdlTh->mSclList) aiqList_deinit(pHdlTh->mSclList);
    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);

    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);
    aiqMutex_deInit(&pHdlTh->_iir_mutex);
    aiqCond_deInit(&pHdlTh->_iir_cond);
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcProcThread_start(AiqMemcProcThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    aiqThread_setPolicy(pHdlTh->_base, SCHED_RR);
    aiqThread_setPriority(pHdlTh->_base, 20);
    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGE_ANALYZER("%s failed", __func__);
        return;
    }
    EXIT_ANALYZER_FUNCTION();
}

static void AiqMemcProcThread_stop(AiqMemcProcThread_t* pHdlTh) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqCond_broadcast(&pHdlTh->_iir_cond);
    aiqThread_stop(pHdlTh->_base);
    EXIT_ANALYZER_FUNCTION();
}

static bool AiqMemcProcThread_push_msg(AiqMemcProcThread_t* pHdlTh,
    memc_callback_result_t* buffer) {
    if (!pHdlTh->_base->_started) return false;

    int ret = aiqList_push(pHdlTh->mMsgsQueue, buffer);
    if (ret) {
        LOGE_ANALYZER("push msg failed", __func__);
        return false;
    }

    aiqCond_broadcast(&pHdlTh->_cond);

    return true;
}

static bool AiqMemcProcThread_push_scl_buf(AiqMemcProcThread_t* pHdlTh,
                        AiqV4l2Buffer_t* buffer) {
    if (!pHdlTh->_base->_started) return false;

    if (!buffer) return false;

    // AiqV4l2Buffer_ref(buffer);

    // int ret = aiqList_push(pHdlTh->mSclList, &buffer);
    // if (ret) {
    //     LOGE_ANALYZER("push msg failed", __func__);
    //     return false;
    // }

    return true;
}

static void LutBufferManagerImportBuffer(AiqMemcManager_t *pMemcMan)
{
    // import lut buf
    assert(pMemcMan->mem_ops_ != NULL);
    rk_aiq_share_mem_config_t hw_config;
    hw_config.mem_type             = MEM_TYPE_BTNR;
    hw_config.alloc_param.width  = pMemcMan->Width;
    hw_config.alloc_param.height = pMemcMan->Height;
    hw_config.alloc_param.reserved[0] = 2;

    pMemcMan->mem_ops_->alloc_mem(0, (void*)(pMemcMan->mem_ops_), &hw_config, &pMemcMan->mem_ctx_);
    pMemcMan->_import_buffer = true;
}

XCamReturn AiqMemcManager_send_sync_buf(void* memc_ctx,
    AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m,
    AiqV4l2Buffer_t* buf_l) {

    if (!memc_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    AiqMemcManager_t* pMemcMan = (AiqMemcManager_t*)memc_ctx;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (buf_s) AiqV4l2Buffer_ref(buf_s);
    if (buf_m) AiqV4l2Buffer_ref(buf_m);
    if (buf_l) AiqV4l2Buffer_ref(buf_l);

    if (pMemcMan->mMemcEn) {
        memc_callback_result_t res;
        memset(&res, 0, sizeof(memc_callback_result_t));
        res.frame_id = AiqV4l2Buffer_getSequence(buf_s);
        res.buf_s    = buf_s;
        res.buf_m    = buf_m;
        res.buf_l    = buf_l;

        AiqMapItem_t* pItem = aiqMap_get(pMemcMan->mSofTimeMap, (void*)(intptr_t)res.frame_id);
        if (pItem) {
            res.sof_time = *(uint64_t*)pItem->_pData;
            aiqMap_erase(pMemcMan->mSofTimeMap, (void*)(intptr_t)res.frame_id);
        }

        AiqCamHwBase_t* pCamHw       = pMemcMan->mCamHw;
        AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
        if (mSensorSubdev) {
            res.expParam = mSensorSubdev->getEffectiveExpParams(mSensorSubdev, res.frame_id);
        }

        if (!AiqMemcProcThread_push_msg(&pMemcMan->mProcTh, &res)) {
            if (buf_s) AiqV4l2Buffer_unref(buf_s);
            if (buf_m) AiqV4l2Buffer_unref(buf_m);
            if (buf_l) AiqV4l2Buffer_unref(buf_l);
            ret = XCAM_RETURN_ERROR_PARAM;
        }
        return ret;

    } else if (pMemcMan->_proc_stream) {
        AiqRawStreamProcUnit_send_sync_buf(pMemcMan->_proc_stream, buf_s, buf_m, buf_l);
    } else {
        LOGE_ANALYZER("no proc unit %s !", __func__);
    }

    if (buf_s) AiqV4l2Buffer_unref(buf_s);
    if (buf_m) AiqV4l2Buffer_unref(buf_m);
    if (buf_l) AiqV4l2Buffer_unref(buf_l);

    return ret;
}

bool AiqMemcManager_push_scl_buffer(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* buffer)
{
    if (!pMemcMan) return false;

    return AiqMemcProcThread_push_scl_buf(&pMemcMan->mProcTh, buffer);
}

static const rk_aiq_btnr_share_mem_info_t*
_getDefaultMesh(AiqMemcManager_t* pMemcMan)
{

    if (!pMemcMan->_import_buffer) {
        LutBufferManagerImportBuffer(pMemcMan);
    }

    const rk_aiq_btnr_share_mem_info_t* mem_info = LutBufferManagerGetFreeHwBuffer(pMemcMan, 0);
    if (mem_info && pMemcMan->_algo_ctx) {
        rk_gyro_memc_param_t in_param;
        memset(&in_param, 0, sizeof(rk_gyro_memc_param_t));
        in_param.enable_default_map = true;
        in_param.map_fix_bit_y = 4;
        in_param.map_fix_bit_x = 4;
        in_param.raw_wid = pMemcMan->Width;
        in_param.raw_hgt = pMemcMan->Height;
        if (in_param.raw_wid > 4080)
            in_param.map_fix_bit_x = 3;
        if (in_param.raw_hgt > 4080)
            in_param.map_fix_bit_y = 3;
        in_param.map_mesh_fix_ptr = mem_info->addr;
        in_param.mapX_start_fix_ptr = mem_info->addr1;
        rk_gyro_memc_default_map(pMemcMan->_algo_ctx, &in_param);
    }

    return mem_info;

}

static XCamReturn
_setIspParamToDrv(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* pV4l2Buf, const rk_aiq_btnr_share_mem_info_t *mesh_info) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqCamHwBase_t* pCamHw    = pMemcMan->mCamHw;
    uint32_t frameId          = -1;

    if (!pV4l2Buf)
        return XCAM_RETURN_ERROR_FAILED;

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    struct isp32_isp_params_cfg* isp_params =
        (struct isp32_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp32_isp_params_cfg ori_params;
#elif defined(ISP_HW_V30)
    struct isp3x_isp_params_cfg* isp_params =
        (struct isp3x_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp3x_isp_params_cfg ori_params;
#elif defined(ISP_HW_V21)
    struct isp21_isp_params_cfg* isp_params =
        (struct isp21_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp21_isp_params_cfg ori_params;
#elif defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_params =
        (struct isp39_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp39_isp_params_cfg ori_params;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* isp_params =
        (struct isp33_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp33_isp_params_cfg ori_params;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* isp_params =
        (struct isp35_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
    struct isp35_isp_params_cfg ori_params;
#else
    struct isp2x_isp_params_cfg* isp_params =
        (struct isp2x_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#endif

    int buf_index      = AiqV4l2Buffer_getBuf(pV4l2Buf)->index;
    frameId            = isp_params->frame_id;

     uint64_t oldEns = pCamHw->_isp_module_ens;
    // assume the max valid bit is 60
    for (int i = 0; i < 60; i++) {
        if (isp_params->module_en_update & (1ULL << i)) {
            if (isp_params->module_ens & (1ULL << i))
                pCamHw->_isp_module_ens |= (1ULL << i);
            else
                pCamHw->_isp_module_ens &= ~(1ULL << i);
        }
    }

    if (oldEns == pCamHw->_isp_module_ens && pCamHw->_state == CAM_HW_STATE_STARTED)
        isp_params->module_en_update = 0;

    if (pCamHw->updateEffParams) {
#if defined(RKAIQ_HAVE_MULTIISP) && (defined(ISP_HW_V30) || defined(ISP_HW_V32) || \
            defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35))
        if (g_mIsMultiIspMode) {
            ori_params = *isp_params;
            pCamHw->updateEffParams(pCamHw, isp_params, &ori_params);
        } else
            pCamHw->updateEffParams(pCamHw, isp_params, NULL);
#else
        pCamHw->updateEffParams(pCamHw, isp_params, NULL);
#endif
    }

    struct isp35_bay3d_cfg *b3d_cfg = &(isp_params->others.bay3d_cfg);
    if (pMemcMan->mMemcEn && mesh_info &&
        (isp_params->module_cfg_update & ISP35_MODULE_BAY3D)) {
        b3d_cfg->lut_buf_fd = mesh_info->fd;
        b3d_cfg->b3dldc_last_ffff_off = pMemcMan->_param.b3dldc_offset;
        if (pMemcMan->_last_btnr_cfg_params) {
            memcpy(pMemcMan->_last_btnr_cfg_params, b3d_cfg, sizeof(struct isp35_bay3d_cfg));
        }
        // LOGK("run here %s:%d id %d fd %d off %d %p", __func__, __LINE__, frameId, b3d_cfg->lut_buf_fd, b3d_cfg->b3dldc_last_ffff_off, b3d_cfg);
    } else if (mesh_info) {
        *mesh_info->state = MESH_BUF_INIT;
        // LOGK("run here %s:%d fd %d off %d %p", __func__, __LINE__, b3d_cfg->lut_buf_fd, b3d_cfg->b3dldc_last_ffff_off, b3d_cfg);
    }

    isp_params->module_cfg_update &= ~(ISP35_MODULE_FORCE);

    if (AiqV4l2Device_qbuf(pCamHw->mIspParamsDev, pV4l2Buf, true) != 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "RKISP1: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n", buf_index,
                        errno, strerror(errno));
        AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
        if (mesh_info) {
            *mesh_info->state = MESH_BUF_INIT;
        }
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int timeout = 1;
    uint32_t buf_counts = AiqV4l2Device_getBufCnt(pCamHw->mIspParamsDev);
    uint32_t try_time   = 3;
    while (AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev) > (buf_counts - pCamHw->mDefaultDelayCnt)) {
        if (pCamHw->mIspParamsDev->poll_event(pCamHw->mIspParamsDev, timeout, -1) <= 0) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev));
            if (AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev) == buf_counts &&
                try_time > 0) {
                timeout = 30;
                try_time--;
                continue;
            } else
                break;
        }
        AiqV4l2Buffer_t* dqbuf = pCamHw->mIspParamsDev->_dequeue_buffer(pCamHw->mIspParamsDev);
        if (!dqbuf) {
            XCAM_LOG_WARNING("dequeue buffer failed");
            // return ret;
        } else {
            AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, dqbuf);
        }
    }

    pCamHw->_curIspParamsSeq = frameId;

    LOGD_CAMHW_SUBM(
        ISP20HW_SUBM,
        "Config id(%u)'s isp params, full_en 0x%llx ens 0x%llx ens_up 0x%llx, cfg_up 0x%llx", isp_params->frame_id,
        pCamHw->_isp_module_ens,
        isp_params->module_ens,
        isp_params->module_en_update,
        isp_params->module_cfg_update);
    LOGD_CAMHW_SUBM(
        ISP20HW_SUBM,
        "device(%s) queue buffer index %d, queue cnt %d, check exit status again[exit: %d]",
        XCAM_STR(AiqV4l2Device_getDevName(pCamHw->mIspParamsDev)), buf_index,
        AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev), pCamHw->_is_exit);
    if (pCamHw->_is_exit) return XCAM_RETURN_BYPASS;

    return ret;
}

XCamReturn AiqMemcManager_result_callback(AiqMemcManager_t* pMemcMan,
    memc_callback_result_t *res) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pMemcMan->_memc_mode == RKAIQ_MEMC_PRE_BASE_IMU) {
        AiqMemcParamsMsg_t paramsMsg;
        paramsMsg.frameId = res->frame_id;
        paramsMsg.type    = RKAIQ_MEMC_MESH_INFO;
        paramsMsg.data    = (void*)res->mesh_info;
        AiqMemcParamsThread_push_msg(&pMemcMan->mIspParamTh, &paramsMsg);
    } else if (pMemcMan->_memc_mode == RKAIQ_MEMC_SYNC_BASE_IMU) {
        AiqCamHwBase_t* pCamHw    = pMemcMan->mCamHw;
        AiqV4l2Buffer_t* pV4l2Buf = NULL;

        if (pMemcMan->mMemcEn) {
            aiqMutex_lock(&pMemcMan->_mFullParam_mutex);
            AiqMapItem_t* pItem = aiqMap_begin(pMemcMan->mFullParamsPendingMap);
            AiqV4l2Buffer_t* pParams = NULL;
            if (pItem) pParams = (AiqV4l2Buffer_t*)(pItem->_pData);
            if (pParams) {
                bool rm = false;
                AIQ_MAP_FOREACH(pMemcMan->mFullParamsPendingMap, pItem, rm) {
                    if ((uint32_t)(long)pItem->_key == res->frame_id) {
                        pParams  = *(AiqV4l2Buffer_t**)(pItem->_pData);
                        pV4l2Buf = pParams;
                        pItem    = aiqMap_erase_locked(pMemcMan->mFullParamsPendingMap,
                                                (void*)(intptr_t)AiqV4l2Buffer_getSequence(pV4l2Buf));
                        rm       = true;
                    } else if ((uint32_t)(long)pItem->_key < res->frame_id) {
                        pParams  = *(AiqV4l2Buffer_t**)(pItem->_pData);
                        pItem    = aiqMap_erase_locked(pMemcMan->mFullParamsPendingMap,
                                                    (void*)(intptr_t)AiqV4l2Buffer_getSequence(pParams));
                        rm       = true;
                        AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pParams);
                    } else
                        break;
                }
            }
            aiqMutex_unlock(&pMemcMan->_mFullParam_mutex);

            aiqMutex_lock(&pCamHw->_stop_cond_mutex);
            if (pCamHw->_isp_stream_status != ISP_STREAM_STATUS_STREAM_OFF) {
                if (pV4l2Buf && res->mesh_info) {
                    if (res->frame_id > 1) {
                        _setIspParamToDrv(pMemcMan, pV4l2Buf, res->mesh_info);
                    } else {
                        *(res->mesh_info->state) = MESH_BUF_INIT;
                        AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
                    }
                } else if (res->mesh_info) {
                    LOGE_ANR("frame id %d param buf is no ready, no set to drv line %d", res->frame_id, __LINE__);
                    pV4l2Buf = AiqV4l2Device_getBuf(pCamHw->mIspParamsDev, -1);
                    if (pV4l2Buf && pMemcMan->_last_btnr_cfg_params) {
#if defined(ISP_HW_V35)
                        struct isp35_isp_params_cfg* isp_params =
                            (struct isp35_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);

                        isp_params->module_en_update  = 0;
                        isp_params->module_cfg_update = ISP35_MODULE_BAY3D;
                        isp_params->module_ens = 0;
                        isp_params->frame_id = res->frame_id;

                        // if (res->mesh_info && pMemcMan->_algo_ctx) {
                        //     rk_gyro_memc_param_t in_param;
                        //     memset(&in_param, 0, sizeof(rk_gyro_memc_param_t));
                        //     in_param.enable_default_map = true;
                        //     in_param.map_fix_bit_y = 4;
                        //     in_param.map_fix_bit_x = 4;
                        //     in_param.raw_wid = pMemcMan->Width;
                        //     in_param.raw_hgt = pMemcMan->Height;
                        //     if (in_param.raw_wid > 4080)
                        //         in_param.map_fix_bit_x = 3;
                        //     if (in_param.raw_hgt > 4080)
                        //         in_param.map_fix_bit_y = 3;
                        //     in_param.map_mesh_fix_ptr = res->mesh_info->addr;
                        //     in_param.mapX_start_fix_ptr = res->mesh_info->addr1;
                        //     rk_gyro_memc_default_map(pMemcMan->_algo_ctx, &in_param);
                        // }

                        isp_params->others.bay3d_cfg = *((struct isp35_bay3d_cfg*)pMemcMan->_last_btnr_cfg_params);
                        if (res->frame_id > 1) {
                            _setIspParamToDrv(pMemcMan, pV4l2Buf, res->mesh_info);
                        } else {
                            *(res->mesh_info->state) = MESH_BUF_INIT;
                            AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
                        }
#endif
                    } else {
                        *(res->mesh_info->state) = MESH_BUF_INIT;
                    }
                    ret = XCAM_RETURN_ERROR_FAILED;
                } else if (pV4l2Buf) {
                    AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
                    LOGE_ANR("frame id %d b3dldc mesh no ready, no set to drv line %d", res->frame_id, __LINE__);
                    ret = XCAM_RETURN_ERROR_FAILED;
                }
            } else if (res->mesh_info) {
                *(res->mesh_info->state) = MESH_BUF_INIT;
            }
            aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
        }
    }
    return ret;

}

void AiqMemcManager_send_ispparam(AiqMemcManager_t* pMemcMan, AiqV4l2Buffer_t* pV4l2Buf)
{
    uint32_t seq = AiqV4l2Buffer_getSequence(pV4l2Buf);
    if (seq == 0 && pMemcMan->_first_param) {
        memc_callback_result_t mem_res;
        memset(&mem_res, 0, sizeof(memc_callback_result_t));
        pMemcMan->_first_param = false;
        mem_res.mesh_info = _getDefaultMesh(pMemcMan);
        pMemcMan->_param.b3dldc_offset = 0;
        _setIspParamToDrv(pMemcMan, pV4l2Buf, mem_res.mesh_info);
    } else {
        if (pMemcMan->_memc_mode == RKAIQ_MEMC_PRE_BASE_IMU) {
            AiqMemcParamsMsg_t msg;
            msg.frameId = seq;
            msg.type    = RKAIQ_MEMC_ISP_PARAM;
            msg.data    = pV4l2Buf;
            AiqMemcParamsThread_push_msg(&pMemcMan->mIspParamTh, &msg);
        } else if (pMemcMan->_memc_mode == RKAIQ_MEMC_SYNC_BASE_IMU) {
            aiqMutex_lock(&pMemcMan->_mFullParam_mutex);
            aiqMap_insert(pMemcMan->mFullParamsPendingMap, (void*)(intptr_t)seq, &pV4l2Buf);
            aiqMutex_unlock(&pMemcMan->_mFullParam_mutex);
        }
    }
}

void AiqMemcManager_setExpParam(void* ctx, aiq_params_base_t* result)
{
    AiqMemcManager_t *pMemcMan = (AiqMemcManager_t*)ctx;
    if (pMemcMan->mCb)
        pMemcMan->mCb->rkAiqCalcExpDone(pMemcMan->mCb->pCtx, result);
}

void AiqMemcManager_setProcStreamCtx(AiqMemcManager_t* pMemcMan, AiqRawStreamProcUnit_t* proc_stream)
{
    pMemcMan->_proc_stream = proc_stream;
}

void AiqMemcManager_pushImuData(AiqMemcManager_t* pMemcMan, AiqImuData_t *data)
{
    if (!pMemcMan->mMemcEn) {
        return;
    }
    if (aiqList_size(pMemcMan->mImuDataList) == 200) {
        AiqImuData_t erase_data;
        aiqList_get(pMemcMan->mImuDataList, &erase_data);
    }
    aiqList_push(pMemcMan->mImuDataList, data);
    // pMemcMan->mNewImuTime = data->s64Timestamp;
}

void AiqMemcManager_pushSofTime(AiqMemcManager_t* pMemcMan, uint32_t frame_id, uint64_t time)
{
    if (pMemcMan->_memc_mode == RKAIQ_MEMC_PRE_BASE_IMU) {
        memc_callback_result_t res;
        memset(&res, 0, sizeof(memc_callback_result_t));
        res.frame_id = frame_id;
        res.sof_time = time;

        AiqCamHwBase_t* pCamHw       = pMemcMan->mCamHw;
        AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
        if (mSensorSubdev) {
            res.expParam = mSensorSubdev->getEffectiveExpParams(mSensorSubdev, res.frame_id);
        }

        AiqMemcProcThread_push_msg(&pMemcMan->mProcTh, &res);
        // LOGK("run here %s:%d time %lld %lld", __func__, __LINE__, pMemcMan->mNewImuTime, res.sof_time);
    } else if (pMemcMan->_memc_mode == RKAIQ_MEMC_SYNC_BASE_IMU) {
        if (aiqMap_size(pMemcMan->mSofTimeMap) > 3) {
            AiqMapItem_t *pItem = aiqMap_begin(pMemcMan->mSofTimeMap);
            aiqMap_erase(pMemcMan->mSofTimeMap, pItem->_key);
        }
        aiqMap_insert(pMemcMan->mSofTimeMap, (void*)(intptr_t)frame_id, &time);
    }

}

void AiqMemcManager_setMemcAlgoStatus(AiqMemcManager_t* pMemcMan)
{
    if (!pMemcMan) {
        return;
    }

    btnr_api_attrib_t *btnr_attrib = pMemcMan->btnr_attr;
    if (btnr_attrib) {
        if (btnr_attrib->opMode == RK_AIQ_OP_MODE_AUTO) {
            pMemcMan->mMemcEn = btnr_attrib->stAuto.sta.swMemc.sw_btnr_swMemc_en;
            pMemcMan->_memc_mode = btnr_attrib->stAuto.sta.swMemc.sw_btnr_swMemc_mode;
        } else {
            pMemcMan->mMemcEn = btnr_attrib->stMan.sta.swMemc.sw_btnr_swMemc_en;
            pMemcMan->_memc_mode = btnr_attrib->stMan.sta.swMemc.sw_btnr_swMemc_mode;
        }
    }
}

bool AiqMemcManager_getMemcEn(AiqMemcManager_t* pMemcMan)
{
    if (!pMemcMan) {
        return false;
    }

    return pMemcMan->mMemcEn;
}

bool AiqMemcManager_getMemcIsSyncBaseImuMode(AiqMemcManager_t* pMemcMan)
{

    if (!pMemcMan) {
        return false;
    }

    if (pMemcMan->_memc_mode == RKAIQ_MEMC_SYNC_BASE_IMU) {
        return true;
    } else {
        return false;
    }
}

void AiqMemcManager_setIirAddress(AiqMemcManager_t* pMemcMan, void* iir_fd, void* iir_address, uint32_t iir_size)
{
    memcpy(pMemcMan->iir_fd, iir_fd, sizeof(pMemcMan->iir_fd));
    memcpy(pMemcMan->iir_address, iir_address, sizeof(pMemcMan->iir_address));
    pMemcMan->iir_size = iir_size;
    if (pMemcMan->iir_size <= pMemcMan->pk_attr_off) {
        LOGE_CAMHW("iir size %d is less than pk_attr_off %d, maybe some error occurred", pMemcMan->iir_size, pMemcMan->pk_attr_off);
    }
}

void AiqMemcManager_pushIirMsg(AiqMemcManager_t* pMemcMan, uint8_t iir_index, uint32_t frame_id)
{
    MemcIirMsg_t msg;
    msg.frame_id = frame_id;
    msg.iir_index = iir_index;
    LOGD_CAMHW("push iir msg frame_id %d iir_index %d time %s", frame_id, iir_index, timeString());

    // aiqList_push(pMemcMan->mIirMsgList, &msg);

    AiqMemcIirThread_push_msg(&pMemcMan->mIirProcTh, &msg);
}

void AiqMemcManager_setCalib(AiqMemcManager_t* pMemcMan, const CamCalibDbV2Context_t* calibv2)
{
    if (!pMemcMan) {
        return;
    }
    btnr_api_attrib_t *btnr_attrib = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void *)calibv2, bayertnr));
    pMemcMan->btnr_attr = btnr_attrib;
}

XCamReturn AiqMemcManager_init(AiqMemcManager_t* pMemcMan,
    const isp_drv_share_mem_ops_t *mem_ops, AiqCamHwBase_t* camHw)
{
    aiqMutex_init(&pMemcMan->_mFullParam_mutex);

    pMemcMan->mem_ops_ = mem_ops;
    pMemcMan->mem_ctx_ = NULL;
    pMemcMan->mCamHw   = camHw;

    AiqMapConfig_t pendingParamsCfg;
    pendingParamsCfg._name          = "memcFullparamMap";
    pendingParamsCfg._key_type		= AIQ_MAP_KEY_TYPE_UINT32;
    pendingParamsCfg._item_nums     = 5;
    pendingParamsCfg._item_size     = sizeof(AiqV4l2Buffer_t*);
    pMemcMan->mFullParamsPendingMap = aiqMap_init(&pendingParamsCfg);
    if (!pMemcMan->mFullParamsPendingMap)
        LOGE_ANALYZER("cId:%d init %s error", pMemcMan->mCamPhyId,
                        pendingParamsCfg._name);

    pendingParamsCfg._name          = "memcSofMap";
    pendingParamsCfg._key_type		= AIQ_MAP_KEY_TYPE_UINT32;
    pendingParamsCfg._item_nums     = 4;
    pendingParamsCfg._item_size     = sizeof(uint64_t);
    pMemcMan->mSofTimeMap = aiqMap_init(&pendingParamsCfg);
    if (!pMemcMan->mSofTimeMap)
        LOGE_ANALYZER("cId:%d init %s error", pMemcMan->mCamPhyId,
                        pendingParamsCfg._name);

    AiqListConfig_t imuList;
    imuList._item_nums = 200;
    imuList._item_size = sizeof(AiqImuData_t);
    imuList._name = "Memc:mImuDataList";
    pMemcMan->mImuDataList = aiqList_init(&imuList);


    AiqPoolConfig_t poolCfg;
    AiqPoolItem_t* pItem = NULL;
    int i                = 0;
    poolCfg._name        = "Memc:mResPool";
    poolCfg._item_nums   = 4;
    poolCfg._item_size   = CALC_SIZE_WITH_HEAD(aiq_params_base_t, void*);
    pMemcMan->mMemcResPool = aiqPool_init(&poolCfg);
    if (!pMemcMan->mMemcResPool) {
        LOGE_ANALYZER("init Memc pool failed");
        return XCAM_RETURN_ERROR_FAILED;
    }
    AIQ_POOL_FOREACH(pMemcMan->mMemcResPool, pItem) {
        aiq_params_base_t* pBase = (aiq_params_base_t*)(pItem->_pData);
        AIQ_PARAMS_BASE_INIT(pBase);
        AIQ_REF_BASE_INIT(&pBase->_ref_base, pItem, aiqPoolItem_ref, aiqPoolItem_unref);
        SET_HEAD_DATA_PTR(pBase);
        LOG1("pool:%s, %p", poolCfg._name, pBase->_data);
    }

    pMemcMan->_first_param = true;
    pMemcMan->_algo_ctx    = NULL;
    pMemcMan->pre_buf_s    = NULL;
    pMemcMan->pre_buf_m    = NULL;
    pMemcMan->pre_buf_l    = NULL;
    pMemcMan->pre_buf_scl  = NULL;

    pMemcMan->_import_buffer = false;

    pMemcMan->mIirPkWarp = true;
    pMemcMan->_last_btnr_cfg_params = NULL;
    pMemcMan->_last_btnr_cfg_params = aiq_mallocz(sizeof(struct isp35_bay3d_cfg));
    // pMemcMan->mIirPkWarp = !!_get_int_default_frome_file("/data/memc_pk_warp_en");

    AiqMemcProcThread_init(&pMemcMan->mProcTh, "AlgoMemcProcTh", pMemcMan);
    AiqMemcIirThread_init(&pMemcMan->mIirProcTh, "AlgoMemcIirTh", pMemcMan);
    AiqMemcIirSyncThread_init(&pMemcMan->mIirSyncTh, "AlgoMemcIirSyncTh", pMemcMan);
    AiqMemcParamsThread_init(&pMemcMan->mIspParamTh, "AlgoMemcParamsTh", pMemcMan);

    return XCAM_RETURN_NO_ERROR;
}

void AiqMemcManager_deinit(AiqMemcManager_t* pMemcMan)
{
    AiqMemcParamsThread_deinit(&pMemcMan->mIspParamTh);
    AiqMemcProcThread_deinit(&pMemcMan->mProcTh);
    AiqMemcIirThread_deinit(&pMemcMan->mIirProcTh);
    AiqMemcIirThread_deinit(&pMemcMan->mIirSyncTh);
    if (pMemcMan->_algo_ctx) {
        rk_gyro_memc_deinit(pMemcMan->_algo_ctx);
        pMemcMan->_algo_ctx = NULL;
    }
    aiqMap_deinit(pMemcMan->mFullParamsPendingMap);
    aiqMap_deinit(pMemcMan->mSofTimeMap);
    aiqList_deinit(pMemcMan->mImuDataList);
    aiqMutex_deInit(&pMemcMan->_mFullParam_mutex);
    aiqPool_deinit(pMemcMan->mMemcResPool);

    if (pMemcMan->_last_btnr_cfg_params) {
        aiq_free(pMemcMan->_last_btnr_cfg_params);
        pMemcMan->_last_btnr_cfg_params = NULL;
    }

}

void AiqMemcManager_prepare(AiqMemcManager_t* pMemcMan, rk_aiq_exposure_sensor_descriptor *snsDes)
{
    pMemcMan->mSnsDes = *snsDes;
    pMemcMan->Width   = snsDes->isp_acq_width;
    pMemcMan->Height  = snsDes->isp_acq_height;

    uint64_t nano_seconds_per_second = 1000000000ULL;
    pMemcMan->_rolling_shutter_skew =
        snsDes->line_length_pck * nano_seconds_per_second /
        snsDes->vt_pix_clk_freq_hz * snsDes->sensor_output_height / 1000;

    int w32 = (pMemcMan->Width + 31) / 32 * 32;
    int w128 = (pMemcMan->Width + 127) / 128 * 128;
    pMemcMan->pk_attr_off = ((w32 + w128 / 8 + 15) / 16 * 16) * pMemcMan->Height * sizeof(uint16_t);
}

void AiqMemcManager_start(AiqMemcManager_t* pMemcMan)
{
    if (!pMemcMan) {
        return;
    }

    if (pMemcMan->mMemcEn && !pMemcMan->_algo_ctx && pMemcMan->btnr_attr) {
        btnr_api_attrib_t *attr = pMemcMan->btnr_attr;
        rk_gyro_memc_param_t param;
        param.raw_wid = pMemcMan->Width;
        param.raw_hgt = pMemcMan->Height;
        param.map_fix_bit_y = 4;
        param.map_fix_bit_x = 4;
        if (param.raw_wid > 4080)
            param.map_fix_bit_x = 3;
        if (param.raw_hgt > 4080)
            param.map_fix_bit_y = 3;

        param.warp_extBound_set  = 0x0;
        param.rk_b3dldc_itp_mode = 1;

        btnr_swMeBaseImu_t *imu_info = NULL;
        if (attr->opMode == RK_AIQ_OP_MODE_AUTO) {
            imu_info = &attr->stAuto.sta.swMemc.swMeBaseImu;
        } else {
            imu_info = &attr->stMan.sta.swMemc.swMeBaseImu;
        }
        param.sensor_axes_type      = imu_info->sw_btnrMe_sensorAexs_mode;
        param.focal_length_x        = imu_info->sw_btnrMe_focalX_length;
        param.focal_length_y        = imu_info->sw_btnrMe_focalY_length;
        param.center_x              = imu_info->sw_btnrMe_centerX_offset;
        param.center_y              = imu_info->sw_btnrMe_centerY_offset;
        param.k1                    = imu_info->sw_btnrMe_sensorK_offset[0];
        param.k2                    = imu_info->sw_btnrMe_sensorK_offset[1];
        param.k3                    = imu_info->sw_btnrMe_sensorK_offset[2];
        param.p1                    = imu_info->sw_btnrMe_sensorP_offset[0];
        param.p2                    = imu_info->sw_btnrMe_sensorP_offset[1];
        param.xi                    = imu_info->sw_btnrMe_sensorXi_offset;
        param.vertical_start_idx    = imu_info->sw_btnrMe_vertical_offset;
        param.horizontal_start_idx  = imu_info->sw_btnrMe_horizontal_offset;

        param.online_prediction_mode_en = pMemcMan->_memc_mode == RKAIQ_MEMC_PRE_BASE_IMU ? true : false;

        pMemcMan->_algo_ctx = rk_gyro_memc_init(&param);
    }
    AiqMemcProcThread_start(&pMemcMan->mProcTh);
    AiqMemcIirThread_start(&pMemcMan->mIirSyncTh);
    AiqMemcIirThread_start(&pMemcMan->mIirProcTh);
    AiqMemcParamsThread_start(&pMemcMan->mIspParamTh);
}

void AiqMemcManager_stop(AiqMemcManager_t* pMemcMan)
{
    AiqMemcProcThread_stop(&pMemcMan->mProcTh);
    AiqMemcIirThread_stop(&pMemcMan->mIirProcTh);
    AiqMemcIirThread_stop(&pMemcMan->mIirSyncTh);
    AiqMemcParamsThread_stop(&pMemcMan->mIspParamTh);

#if RKMEMC_DEBUG
    if (pMemcMan->mem_ctx_) {
        drv_share_mem_ctx_t* drv_mem_ctx = (drv_share_mem_ctx_t*)pMemcMan->mem_ctx_;
        rk_aiq_lut_share_mem_info_t* mem_info_array = (rk_aiq_lut_share_mem_info_t*)(drv_mem_ctx->mem_info);
        if (mem_info_array) {
            for (int idx = 0; idx < ISP2X_MESH_BUF_NUM; idx++) {
                char f_p[128] = {0};
                sprintf(f_p, "/tmp/mesh_map_%d", mem_info_array[idx].fd);
                int hsize  = (((pMemcMan->Width + 15) / 16 + 1) + 1) / 2;
                int vsize  = (pMemcMan->Height + 7) / 8 + 1;
                _write_mesh_bin_to_ddr(f_p, mem_info_array[idx].addr, hsize * vsize * 4);
                sprintf(f_p, "/tmp/meshX_map_%d", mem_info_array[idx].fd);
                hsize  = (((pMemcMan->Width + 15) / 16 + 1) + 1) / 2;
                vsize  = (pMemcMan->Height + 15) / 16 + 2;
                _write_mesh_bin_to_ddr(f_p, mem_info_array[idx].addr1, hsize * vsize * 4);
            }
        }
    }
#endif

    if (pMemcMan->pre_buf_s) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_s);
        pMemcMan->pre_buf_s = NULL;
    }
    if (pMemcMan->pre_buf_m) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_m);
        pMemcMan->pre_buf_m = NULL;
    }
    if (pMemcMan->pre_buf_l) {
        AiqV4l2Buffer_unref(pMemcMan->pre_buf_l);
        pMemcMan->pre_buf_l = NULL;
    }

    LutBufferManagerReleaseHwBuffers(pMemcMan, 0);
    pMemcMan->_first_param = true;
}

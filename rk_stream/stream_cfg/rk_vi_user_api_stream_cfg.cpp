#include "rkrawstream_user_api.h"

#include "Stream.h"
#include "RawStreamCapUnit.h"
#include "IspStreamCapUnit.h"
#include "RawStreamProcUnit.h"
#include "MediaInfo.h"
#include "rk_aiq_types.h"
#include "xcam_mutex.h"

using namespace XCam;
using namespace RkRawStream;

struct rkraw_vi_ctx_s {
    Mutex _api_mutex;
	class MediaInfo *_mMediaInfo;
    rk_sensor_full_info_t *s_full_info;
	class RawStreamCapUnit *_mRawCapUnit;
	class IspStreamCapUnit *_mIspCapUnit;
	class RawStreamProcUnit *_mRawProcUnit;
    bool fake_sns_mode;
    const char *fake_sns_name;
};

rkraw_vi_ctx_t *rkrawstream_uapi_init()
{
	xcam_get_log_level();
	rkraw_vi_ctx_t *ctx = new rkraw_vi_ctx_t();
    ctx->_mMediaInfo = new MediaInfo();
    ctx->_mMediaInfo->initCamHwInfos();
    ctx->_mRawCapUnit = NULL;
    ctx->_mRawProcUnit = NULL;
    ctx->fake_sns_mode = false;
	return ctx;
}

void rkrawstream_uapi_fakesns_mode(rkraw_vi_ctx_t *ctx, int isp_index, const char *real_lens)
{
    ctx->_mMediaInfo->setupOffLineLink(isp_index, true);
    ctx->fake_sns_name = ctx->_mMediaInfo->offline(isp_index, real_lens);
    ctx->fake_sns_mode = true;
}

void rkrawstream_uapi_deinit(rkraw_vi_ctx_t *ctx)
{
    if (!ctx)
        return;
    {
        SmartLock locker (ctx->_api_mutex);
        if(ctx->_mRawCapUnit)
            ctx->_mRawCapUnit->stop();
        if(ctx->_mIspCapUnit)
            ctx->_mIspCapUnit->stop();
        if(ctx->_mRawProcUnit)
            ctx->_mRawProcUnit->stop();
        if(ctx->_mIspCapUnit) {
            delete ctx->_mIspCapUnit;
            ctx->_mIspCapUnit = NULL;
        }
        if(ctx->_mRawCapUnit) {
            delete ctx->_mRawCapUnit;
            ctx->_mRawCapUnit = NULL;
        }
        if(ctx->_mRawProcUnit) {
            delete ctx->_mRawProcUnit;
            ctx->_mRawProcUnit = NULL;
        }
        if(ctx->_mMediaInfo) {
            delete ctx->_mMediaInfo;
            ctx->_mMediaInfo = NULL;
        }
    }
    delete ctx;
    ctx = NULL;
}

int rkrawstream_uapi_parase_rkraw2(uint8_t *rawdata, rkrawstream_rkraw2_t *rkraw2)
{
    return _parse_rk_rawdata(rawdata, rkraw2);
}

// void rkraw_vi_uapi_get_mediainfo(rkraw_vi_ctx_t *ctx, rkraw_vi_sensor_info_t *info)
// {
    // ctx->_mMediaInfo->getCamHwEntNames(info->ent_names);
// }

// int
// rkraw_vi_uapi_getStaticMetas(rkraw_vi_ctx_t* ctx, const char* sns_ent_name, rk_aiq_static_info_t* static_info)
// {
    // if (!sns_ent_name || !static_info)
        // return XCAM_RETURN_ERROR_FAILED;

    // memcpy(static_info, ctx->_mMediaInfo->getStaticCamHwInfo(sns_ent_name), sizeof(rk_aiq_static_info_t));
    // return 0;
// }

int rkrawstream_vicap_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    if (p->sns_ent_name) {
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo(p->sns_ent_name, 0);
        if(ctx->s_full_info == NULL){
            LOGE_RKSTREAM("can't find sensor %s", p->sns_ent_name);
            return XCAM_RETURN_ERROR_SENSOR;
        }
        ctx->_mRawCapUnit = new RawStreamCapUnit(ctx->s_full_info);
    } else {
        ctx->_mRawCapUnit = new RawStreamCapUnit(p->dev0_name, p->dev1_name, p->dev2_name);
    }
    ctx->_mRawCapUnit->user_priv_data = p->user_data;
	return ret;
}

int rkrawstream_vicap_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s: %dx%d 0x%x, hdr %d, mem %d, buftype %d, bufcnt %d\n",
        __func__, p->width, p->height, p->pix_fmt, p->hdr_mode,
        p->mem_mode, p->buf_memory_type, p->buf_cnt);
	int ret = 0;
    ctx->_mRawCapUnit->set_sensor_mode(p->hdr_mode);
    ctx->_mRawCapUnit->set_sensor_format(p->width, p->height, p->pix_fmt);
    ctx->_mRawCapUnit->set_working_mode(p->hdr_mode);
    if (!ctx->s_full_info->linked_to_isp && !ctx->fake_sns_mode)
        ctx->_mRawCapUnit->prepare_cif_mipi();
    ctx->_mRawCapUnit->set_tx_format(p->width, p->height, p->pix_fmt, p->mem_mode);
    ctx->_mRawCapUnit->prepare(p->buf_memory_type, p->buf_cnt);
	return ret;
}

int rkrawstream_vicap_start(rkraw_vi_ctx_t* ctx, rkrawstream_vicap_cb_t cb)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s enter", __func__);
	int ret = 0;
    ctx->_mRawCapUnit->user_on_frame_capture_cb = cb;
	ctx->_mRawCapUnit->start();
    LOGI_RKSTREAM("%s exit", __func__);
	return ret;
}

int rkrawstream_vicap_stop(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s enter", __func__);
	int ret = 0;
	ctx->_mRawCapUnit->stop();
    LOGI_RKSTREAM("%s exit", __func__);
	return ret;
}

int rkrawstream_vicap_streamoff(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s enter", __func__);
    ctx->_mRawCapUnit->stop_device();
    LOGI_RKSTREAM("%s exit", __func__);
    return 0;
}

int rkrawstream_vicap_release_buffer(rkraw_vi_ctx_t* ctx)
{
    LOGI_RKSTREAM("%s enter", __func__);
    SmartLock locker (ctx->_api_mutex);
    ctx->_mRawCapUnit->release_buffer();
    LOGI_RKSTREAM("%s exit", __func__);
    return 0;
}

void rkrawstream_vicap_buf_take(rkraw_vi_ctx_t* ctx)
{
	ctx->_mRawCapUnit->user_takes_buf = true;
}

void rkrawstream_vicap_buf_return(rkraw_vi_ctx_t* ctx, int dev_index)
{
	ctx->_mRawCapUnit->release_user_taked_buf(dev_index);
}

void rkrawstream_vicap_setdmabuf(rkraw_vi_ctx_t* ctx, int dev_index, int buf_index, int fd)
{
    LOGI_RKSTREAM("%s dev %d, buf %d, fd %d", __func__, dev_index, buf_index, fd);
    ctx->_mRawCapUnit->set_dma_buf(dev_index, buf_index, fd);
}

int rkrawstream_readback_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    if (ctx->fake_sns_mode) {
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo((char *)ctx->fake_sns_name, 0);
    } else {
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo(p->sns_ent_name, 0);
    }
    if(ctx->s_full_info == NULL){
        LOGE_RKSTREAM("can't find sensor %s", p->sns_ent_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }
    ctx->_mRawProcUnit = new RawStreamProcUnit(ctx->s_full_info, ctx->fake_sns_mode);

    ctx->_mRawProcUnit->user_priv_data = p->user_data;
	return ret;
}

int rkrawstream_readback_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p)
{
    LOGI_RKSTREAM("%s: %dx%d 0x%x, hdr %d, mem %d, buftype %d, bufcnt %d\n",
        __func__, p->width, p->height, p->pix_fmt, p->hdr_mode,
        p->mem_mode, p->buf_memory_type, p->buf_cnt);
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    ctx->_mRawProcUnit->set_working_mode(p->hdr_mode);
    ctx->_mRawProcUnit->set_rx_format(p->width, p->height, p->pix_fmt, p->mem_mode);
    ctx->_mRawProcUnit->prepare(p->buf_memory_type, p->buf_cnt);
	return ret;
}

int rkrawstream_readback_start(rkraw_vi_ctx_t* ctx, rkrawstream_readback_cb_t cb)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s enter", __func__);
	int ret = 0;
    ctx->_mRawProcUnit->user_isp_process_done_cb = cb;
	ctx->_mRawProcUnit->start();
    LOGI_RKSTREAM("%s exit", __func__);
	return ret;
}

int rkrawstream_readback_stop(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s enter", __func__);
	int ret = 0;
	ctx->_mRawProcUnit->stop();
    LOGI_RKSTREAM("%s exit", __func__);
	return ret;
}

int rkrawstream_readback_set_buffer(rkraw_vi_ctx_t* ctx, uint8_t *rkraw_data)
{
	int ret = 0;
    LOGI_RKSTREAM("%s enter", __func__);
	ctx->_mRawProcUnit->send_sync_buf2(rkraw_data);
	return ret;
}

int rkrawstream_readback_set_rkraw2(rkraw_vi_ctx_t* ctx, rkrawstream_rkraw2_t *rkraw2)
{
	int ret = 0;
    LOGI_RKSTREAM("%s enter", __func__);
    // check for addr align
    if (rkraw2->plane[0].addr & 0x2) {
        LOGW_RKSTREAM("%s addr is not align", __func__);
    }
	ctx->_mRawProcUnit->_send_sync_buf(rkraw2);
	return ret;
}

int rkrawstream_setup_pipline_fmt(rkraw_vi_ctx_t* ctx, int width, int height, uint32_t pixfmt)
{
	int ret = 0;
	ctx->_mRawProcUnit->setup_pipeline_fmt(width, height, pixfmt);
	return ret;
}

int rkrawstream_isp_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    if (ctx->fake_sns_mode) {
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo((char*)ctx->fake_sns_name, 0);
    }
    else {
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo(p->sns_ent_name, 0);
    }
    if(ctx->s_full_info == NULL){
        LOGE_RKSTREAM("can't find sensor %s", p->sns_ent_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }
    ctx->_mIspCapUnit = new IspStreamCapUnit(ctx->s_full_info);

    ctx->_mIspCapUnit->user_priv_data = p->user_data;
	return ret;
}

int rkrawstream_isp_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
    LOGI_RKSTREAM("%s: %dx%d 0x%x, hdr %d, mem %d, buftype %d, bufcnt %d\n",
        __func__, p->width, p->height, p->pix_fmt, p->hdr_mode,
        p->mem_mode, p->buf_memory_type, p->buf_cnt);
	int ret = 0;
    ctx->_mIspCapUnit->set_isp_format(p->width, p->height);
    ctx->_mIspCapUnit->prepare(p->buf_memory_type, p->buf_cnt);
	return ret;
}

int rkrawstream_isp_start(rkraw_vi_ctx_t* ctx, rkrawstream_isp_cb_t cb)
{
    SmartLock locker (ctx->_api_mutex);
    LOGD_RKSTREAM("%s enter", __func__);
	int ret = 0;
    ctx->_mIspCapUnit->user_on_frame_capture_cb = cb;
	ctx->_mIspCapUnit->start();
    LOGD_RKSTREAM("%s exit", __func__);
	return ret;
}

int rkrawstream_isp_stop(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    LOGD_RKSTREAM("%s enter", __func__);
	int ret = 0;
	ctx->_mIspCapUnit->stop();
    LOGD_RKSTREAM("%s exit", __func__);
	return ret;
}

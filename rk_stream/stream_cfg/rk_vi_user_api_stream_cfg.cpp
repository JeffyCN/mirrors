#include "rk_vi_user_api2_stream_cfg.h"

//#include "../../RkStreamManager.h"
#include "Stream.h"
#include "RawStreamCapUnit.h"
#include "RawStreamProcUnit.h"
#include "MediaInfo.h"
#include "rk_aiq_types.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "xcam_mutex.h"

using namespace XCam;
using namespace RkRawStream;

enum mipi_stream_idx {
	MIPI_STREAM_IDX_0   = 1,
	MIPI_STREAM_IDX_1   = 2,
	MIPI_STREAM_IDX_2   = 4,
	MIPI_STREAM_IDX_ALL = 7,
};

struct rkraw_vi_ctx_s {
    Mutex _api_mutex;
	class MediaInfo *_mMediaInfo;
    rk_sensor_full_info_t *s_full_info;
	class RawStreamCapUnit *_mRawCapUnit;
	class RawStreamProcUnit *_mRawProcUnit;
};

rkraw_vi_ctx_t *rkrawstream_uapi_init_offline(const char *isp_vir, const char *real_sns)
{
	xcam_get_log_level();
	rkraw_vi_ctx_t *ctx = new rkraw_vi_ctx_t();
    ctx->_mMediaInfo = new MediaInfo();
    ctx->_mMediaInfo->initCamHwInfos();
    ctx->_mMediaInfo->offline(isp_vir, real_sns);
    ctx->_mRawCapUnit = NULL;
    ctx->_mRawProcUnit = NULL;
	return ctx;
}

rkraw_vi_ctx_t *rkrawstream_uapi_init()
{
	xcam_get_log_level();
	rkraw_vi_ctx_t *ctx = new rkraw_vi_ctx_t();
    ctx->_mMediaInfo = new MediaInfo();
    ctx->_mMediaInfo->initCamHwInfos();
    ctx->_mRawCapUnit = NULL;
    ctx->_mRawProcUnit = NULL;
	return ctx;
}

void rkrawstream_uapi_deinit(rkraw_vi_ctx_t *ctx)
{
    {
        SmartLock locker (ctx->_api_mutex);
        if(ctx->_mRawCapUnit)
            ctx->_mRawCapUnit->stop();
        if(ctx->_mRawProcUnit)
            ctx->_mRawProcUnit->stop();
        delete ctx->_mRawCapUnit;
        delete ctx->_mRawProcUnit;
        delete ctx->_mMediaInfo;
    }
    delete ctx;
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
    if(p->sns_ent_name){
        LOGI_RKSTREAM("%s:use %s", __func__, p->sns_ent_name);
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo(p->sns_ent_name, 0);
        if(ctx->s_full_info == NULL){
            LOGE_RKSTREAM("can't find sensor %s", p->sns_ent_name);
            return XCAM_RETURN_ERROR_SENSOR;
        }
        ctx->_mRawCapUnit = new RawStreamCapUnit(ctx->s_full_info);
    }else{
        LOGI_RKSTREAM("%s:use %s", __func__, p->dev0_name);
        ctx->_mRawCapUnit = new RawStreamCapUnit(p->dev0_name, p->dev1_name, p->dev2_name);
    }
	return ret;
}

int rkrawstream_vicap_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    ctx->_mRawCapUnit->set_sensor_format(p->width, p->height, 0);
    ctx->_mRawCapUnit->set_tx_format(p->width, p->height, p->pix_fmt, p->mem_mode);
    if(p->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL){
        ctx->_mRawCapUnit->prepare(MIPI_STREAM_IDX_0, p->buf_memory_type, p->buf_cnt);
        ctx->_mRawCapUnit->set_working_mode(RK_AIQ_WORKING_MODE_NORMAL);
        ctx->_mRawCapUnit->set_sensor_mode(RK_AIQ_WORKING_MODE_NORMAL);
    }else if(p->hdr_mode == RK_AIQ_WORKING_MODE_ISP_HDR2){

        ctx->_mRawCapUnit->prepare(MIPI_STREAM_IDX_0 | MIPI_STREAM_IDX_1, p->buf_memory_type, p->buf_cnt);
        ctx->_mRawCapUnit->set_working_mode(RK_AIQ_ISP_HDR_MODE_2_LINE_HDR);
        ctx->_mRawCapUnit->set_sensor_mode(RK_AIQ_ISP_HDR_MODE_2_LINE_HDR);
    }
	return ret;
}

int rkrawstream_vicap_start(rkraw_vi_ctx_t* ctx, rkraw_vi_on_frame_capture_callback cb)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    ctx->_mRawCapUnit->user_on_frame_capture_cb = cb;
	ctx->_mRawCapUnit->start();
	return ret;
}

int rkrawstream_vicap_stop(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
	ctx->_mRawCapUnit->stop();
	return ret;
}

int rkrawstream_vicap_streamoff(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    ctx->_mRawCapUnit->stop_device();
    return 0;
}

int rkrawstream_vicap_release_buffer(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
    ctx->_mRawCapUnit->release_buffer();
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
    ctx->_mRawCapUnit->set_dma_buf(dev_index, buf_index, fd);
}

int rkrawstream_readback_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    if(p->sns_ent_name){
        ctx->s_full_info = ctx->_mMediaInfo->getSensorFullInfo(p->sns_ent_name, 0);
        if(ctx->s_full_info == NULL){
            LOGE_RKSTREAM("can't find sensor %s", p->sns_ent_name);
            return XCAM_RETURN_ERROR_SENSOR;
        }
        ctx->_mRawProcUnit = new RawStreamProcUnit(ctx->s_full_info, p->use_offline);
    }

	return ret;
}

int rkrawstream_readback_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    ctx->_mRawProcUnit->set_rx_format(p->width, p->height, p->pix_fmt, p->mem_mode);
    if(p->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL){
        ctx->_mRawProcUnit->prepare(MIPI_STREAM_IDX_0, p->buf_memory_type, p->buf_cnt);
        ctx->_mRawProcUnit->set_working_mode(RK_AIQ_WORKING_MODE_NORMAL);
    }else if(p->hdr_mode == RK_AIQ_WORKING_MODE_ISP_HDR2){
        ctx->_mRawProcUnit->prepare(MIPI_STREAM_IDX_0 | MIPI_STREAM_IDX_1, p->buf_memory_type, p->buf_cnt);
        ctx->_mRawProcUnit->set_working_mode(RK_AIQ_ISP_HDR_MODE_2_LINE_HDR);
    }
	return ret;
}

int rkrawstream_readback_start(rkraw_vi_ctx_t* ctx, rkraw_vi_on_isp_process_done_callback cb)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
    ctx->_mRawProcUnit->user_isp_process_done_cb = cb;
	ctx->_mRawProcUnit->start();
	return ret;
}

int rkrawstream_readback_stop(rkraw_vi_ctx_t* ctx)
{
    SmartLock locker (ctx->_api_mutex);
	int ret = 0;
	ctx->_mRawProcUnit->stop();
	return ret;
}

int rkrawstream_readback_set_buffer(rkraw_vi_ctx_t* ctx, uint8_t *rkraw_data)
{
	int ret = 0;
	ctx->_mRawProcUnit->send_sync_buf2(rkraw_data);
	return ret;
}

int rkrawstream_readback_set_rkraw2(rkraw_vi_ctx_t* ctx, rkrawstream_rkraw2_t *rkraw2)
{
	int ret = 0;
	ctx->_mRawProcUnit->_send_sync_buf(rkraw2);
	return ret;
}

int rkrawstream_setup_pipline_fmt(rkraw_vi_ctx_t* ctx, int width, int height)
{
	int ret = 0;
	ctx->_mRawProcUnit->setup_pipeline_fmt(width, height);
	return ret;
}


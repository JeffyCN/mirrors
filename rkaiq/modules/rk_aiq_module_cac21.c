#include "rk_aiq_isp39_modules.h"
#include "common/rk_aiq_types_priv_c.h"

#define RKCAC_EDGE_DETECT_FIX_BITS 4
#define RKCAC_STRENGTH_FIX_BITS    7
static void cac21_multi_cvt(struct isp32_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta, cac_cvt_info_t *cacInfo, bool is_multi_isp);

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define CAC_PSF_BUF_NUM 2
#define BITS_PER_BYTE   8
#define BYTES_PER_WORD  4
#define BITS_PER_WORD   (BITS_PER_BYTE * BYTES_PER_WORD)

static const uint32_t IspBigModeWidthLimit = 1536;
static const uint32_t IspBigModeSizeLimit  = IspBigModeWidthLimit * 864;
static const uint32_t CacPsfCountLimit     = 408;
static const uint8_t CacChannelCount       = 2;
static const uint8_t CacScaleFactorDefault = 64;
static const uint8_t CacScaleFactorBigMode = 128;
static const uint16_t CacScaleFactor256Mode = 256;
static const uint8_t CacStrengthDistance   = 128;
static const uint8_t CacPsfKernelSize      = 7 * 5;
static const uint8_t CacPsfKernelWordSizeInMemory =
    DIV_ROUND_UP((CacPsfKernelSize - 1) * BITS_PER_BYTE, BITS_PER_WORD);
static const uint8_t CacPsfBufferCount = CAC_PSF_BUF_NUM;

static void LutBufferInit(LutBuffer* buf, const LutBufferConfig* config, const rk_aiq_cac_share_mem_info_t* mem_info)
{
    buf->Config = *config;
    buf->State = (LutBufferState)(*mem_info->state);
    buf->Fd    = mem_info->fd;
    buf->Addr  = mem_info->addr;
    buf->Size  = mem_info->size;
}

static void LutBufferManagerInit(LutBufferManager *man, const LutBufferConfig* config, const isp_drv_share_mem_ops_t* mem_ops)
{
    man->mem_ops_ = mem_ops;
    man->mem_ctx_ = NULL;
    man->config_ = *config;
}

static void LutBufferManagerImportHwBuffers(LutBufferManager *man, uint8_t isp_id) {
    assert(man->mem_ops_ != NULL);
    rk_aiq_share_mem_config_t hw_config;
    hw_config.mem_type             = MEM_TYPE_CAC;
    hw_config.alloc_param.width  = man->config_.Width;
    hw_config.alloc_param.height = man->config_.Height;
    hw_config.alloc_param.reserved[0] = 2;

    man->mem_ops_->alloc_mem(isp_id, (void*)(man->mem_ops_), &hw_config, &man->mem_ctx_);
}

static void LutBufferManagerReleaseHwBuffers(LutBufferManager *man, uint8_t isp_id) {
    if (man->mem_ctx_ != NULL && man->mem_ops_ != NULL)
        man->mem_ops_->release_mem(isp_id, man->mem_ctx_);
}

static LutBuffer* LutBufferManagerGetFreeHwBuffer(LutBufferManager *man, uint8_t isp_id) {
    if (man->mem_ops_ == NULL || man->mem_ctx_ == NULL) {
        return NULL;
    }

    const rk_aiq_cac_share_mem_info_t* mem_info = (const rk_aiq_cac_share_mem_info_t*)(
        man->mem_ops_->get_free_item(isp_id, man->mem_ctx_));
    if (mem_info != NULL) {
        LutBuffer* lut_buf = aiq_mallocz(sizeof(LutBuffer));
        if (lut_buf != NULL) {
            LutBufferInit(lut_buf, &man->config_, mem_info);
            return lut_buf;
        }
    }
    return NULL;
}

void LutBufferManagerDeinit(cac_cvt_info_t *cacInfo, LutBufferManager *man)
{
    LutBufferManagerReleaseHwBuffers(man, 0);
    if (cacInfo->is_multi_isp && cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID)
        LutBufferManagerReleaseHwBuffers(man, 1);
}

static inline bool IsIspBigMode(uint32_t width, uint32_t height, bool is_multi_sensor) {
    if (is_multi_sensor || width > IspBigModeWidthLimit || width * height > IspBigModeSizeLimit) {
        return true;
    }

    return false;
}

#if RKAIQ_HAVE_CAC_V12
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig* config, bool is_asset) {
    //is_big_mode is useless;
    config->Width     = width;
    config->Height    = height;
    config->IsBigMode = true;
    config->ScaleFactor = CacScaleFactor256Mode;
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config->LutHCount   =(width + 254) >> 8 ;
    config->LutVCount   = (height  + 254) >> 8;
    config->PsfCfgCount = config->LutHCount * config->LutVCount;
    if (is_asset)
        XCAM_ASSERT(config->PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}
#else
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig* config, bool is_asset) {
    config->Width     = width;
    config->Height    = height;
    config->IsBigMode = is_big_mode;
    if (config->IsBigMode) {
        config->ScaleFactor = CacScaleFactorBigMode;
    } else {
        config->ScaleFactor = CacScaleFactorDefault;
    }
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config->LutHCount   = is_big_mode ? (width + 126) >> 7 : (width + 62) >> 6;
    config->LutVCount   = is_big_mode ? (height + 126) >> 7 : (height + 62) >> 6;
    config->PsfCfgCount = config->LutHCount * config->LutVCount;
    if (is_asset)
        XCAM_ASSERT(config->PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}
#endif

static XCamReturn rk_aiq_cac21_update_lut(cac_cvt_info_t *cacInfo, char *sw_cacT_psfMap_path) {
    LutBufferConfig lut_config;
    LutBufferConfig full_lut_config;
    uint32_t width   = cacInfo->rawWidth;
    uint32_t height  = cacInfo->rawHeight;
    bool is_big_mode = IsIspBigMode(width, height, cacInfo->is_multi_sensor);
    char cac_map_path[RKCAC_MAX_PATH_LEN] = {0};

    memset(&lut_config, 0, sizeof(lut_config));
    memset(&full_lut_config, 0, sizeof(full_lut_config));
    if (cacInfo->is_multi_isp && cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
        CalcCacLutConfig(width, height, is_big_mode, &full_lut_config, false);
        width = width / 2 + cacInfo->multi_isp_extended_pixel;
        CalcCacLutConfig(width, height, is_big_mode, &lut_config, true);
    } else {
        CalcCacLutConfig(width, height, is_big_mode, &lut_config, true);
    }
    if (cacInfo->lut_manger_ == NULL) {
        cacInfo->lut_manger_ = aiq_mallocz(sizeof(LutBufferManager));
        if (cacInfo->lut_manger_ == NULL) {
            LOGE_ACAC("malloc failure");
            return XCAM_RETURN_ERROR_MEM;
        }

        LutBufferManagerInit(cacInfo->lut_manger_, &lut_config, cacInfo->mem_ops);
        LutBufferManagerImportHwBuffers(cacInfo->lut_manger_, 0);
        if (cacInfo->is_multi_isp && cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
            LutBufferManagerImportHwBuffers(cacInfo->lut_manger_, 1);
        }
    }
    LutBuffer* buf = LutBufferManagerGetFreeHwBuffer(cacInfo->lut_manger_, 0);
    if (buf == NULL) {
        LOGW_ACAC("No buffer available, maybe only one buffer ?!");
        return XCAM_RETURN_NO_ERROR;
    }
    cacInfo->current_lut_size = 0;
    aiq_free(cacInfo->current_lut_[0]);
    cacInfo->current_lut_[0] = buf;
    cacInfo->current_lut_size++;
    if (buf->State != kInitial) {
        LOGW_ACAC("Buffer in use, will not update lut!");
        return XCAM_RETURN_NO_ERROR;
    }
    if (cacInfo->is_multi_isp && cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
        LutBuffer* buf2 = LutBufferManagerGetFreeHwBuffer(cacInfo->lut_manger_, 1);
        if (buf2 == NULL) {
            LOGW_ACAC("No buffer available, maybe only one buffer ?!");
            return XCAM_RETURN_NO_ERROR;
        }
        aiq_free(cacInfo->current_lut_[1]);
        cacInfo->current_lut_[1] = buf2;
        cacInfo->current_lut_size++;
    }
    XCAM_ASSERT(cacInfo->current_lut_size == (uint32_t)(cacInfo->isp_unite_mode + 1));

    if (sw_cacT_psfMap_path[0] != '/') {
        strcpy(cac_map_path, cacInfo->iqpath);
        strcat(cac_map_path, "/");
    }
    strcat(cac_map_path, sw_cacT_psfMap_path);

    FILE *fp = fopen(cac_map_path, "rb");
    if (fp == NULL) {
        LOGW_ACAC("Failed to open PSF file %s", cac_map_path);
        return XCAM_RETURN_ERROR_FILE;
    }

    if (cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_NORMAL) {
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t size = lut_config.LutHCount * lut_config.LutVCount * CacPsfKernelWordSizeInMemory *
                        BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = (char*)(cacInfo->current_lut_[0]->Addr) + ch * size;
            fread(addr0, 1, size, fp);
        }
    } else if (cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
        XCAM_ASSERT(cacInfo->current_lut_size > 1);
        // Read and Split Memory
        //   a == line_size - line_offset
        //   b == line_offset
        //   c == line_offset - a = 2 * line_offset - line_size
        // For each line:
        //   read b size to left
        //   copy c from left to right
        //   read a' to right
        // - +---------------------------+
        // | |<---a---->|  |  |<---a'--->|
        // | |                 |<-c->|          |
        // v |<---b---------->|          |
        // | |          |  |  |          |
        // - +---------------------------+
        //   |<---------line_size------->|
        //
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t line_size =
            full_lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = (char*)(cacInfo->current_lut_[0]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            char* addr1 = (char*)(cacInfo->current_lut_[1]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            for (uint32_t i = 0; i < full_lut_config.LutVCount; i++) {
                fread(addr0 + (i * line_offset), 1, line_offset, fp);
                memcpy(addr1 + (i * line_offset),
                       addr0 + (i * line_offset) + line_size - line_offset,
                       2 * line_offset - line_size);
                fread(addr1 + (i * line_size) + line_offset, 1, line_size - line_offset, fp);
            }
        }
    } else if (cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        LOGE_ACAC("don't support %d mode", cacInfo->isp_unite_mode);
    }
    fclose(fp);

    return XCAM_RETURN_NO_ERROR;
}

void rk_aiq_cac21_params_cvt(void* attr, isp_params_t* isp_params,
    isp_params_t* isp_cfg_right, cac_cvt_info_t *cacInfo, common_cvt_info_t* cvtinfo)
{
    struct isp32_cac_cfg* phwcfg = &isp_params->isp_cfg->others.cac_cfg;
    cac_param_t *cac_param = (cac_param_t*)attr;
    cac_params_dyn_t* pdyn = &cac_param->dyn;
    cac_params_static_t* psta = &cac_param->sta;

    RKAiqAecExpInfo_t* ae_exp = cvtinfo->ae_exp;
    cacInfo->hdr_ratio = 1;
    if (ae_exp != NULL) {
        if (cvtinfo->frameNum == 1) {
            cacInfo->hdr_ratio = 1;
        } else if (cvtinfo->frameNum == 2) {
            cacInfo->hdr_ratio = (ae_exp->HdrExp[1].exp_real_params.analog_gain *
                                        ae_exp->HdrExp[1].exp_real_params.integration_time) /
                                       (ae_exp->HdrExp[0].exp_real_params.analog_gain *
                                        ae_exp->HdrExp[0].exp_real_params.integration_time);
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f ratio %f\n", __FUNCTION__,
                      ae_exp->HdrExp[1].exp_real_params.analog_gain, cacInfo->hdr_ratio);
        } else if (cvtinfo->frameNum == 3) {
            cacInfo->hdr_ratio = (ae_exp->HdrExp[2].exp_real_params.analog_gain *
                                        ae_exp->HdrExp[2].exp_real_params.integration_time) /
                                       (ae_exp->HdrExp[0].exp_real_params.analog_gain *
                                        ae_exp->HdrExp[0].exp_real_params.integration_time);
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__,
                      ae_exp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    cacInfo->rawWidth = cvtinfo->rawWidth;
    cacInfo->rawHeight = cvtinfo->rawHeight;
    cac21_multi_cvt(phwcfg, pdyn, psta, cacInfo, false);
    if (cacInfo->is_multi_isp && cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
        phwcfg = &(isp_params->isp_cfg + 1)->others.cac_cfg;
        cac21_multi_cvt(phwcfg, pdyn, psta, cacInfo, cacInfo->is_multi_isp);
    }
}

static void cac21_multi_cvt(struct isp32_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta, cac_cvt_info_t *cacInfo, bool is_multi_isp) {

    int index = (int)is_multi_isp;
    XCamReturn ret = rk_aiq_cac21_update_lut(cacInfo, psta->psfParam.sw_cacT_psfMap_path);
    if (ret != XCAM_RETURN_NO_ERROR) {
        // buf_fd should be valid if enabled, or will cause iommu error
        phwcfg->bypass_en = true;
        phwcfg->buf_fd        = cacInfo->current_lut_[index]->Fd;
        phwcfg->hsize         = cacInfo->current_lut_[0]->Config.LutHCount * CacPsfKernelWordSizeInMemory;
        phwcfg->vsize         = cacInfo->current_lut_[0]->Config.LutVCount * CacChannelCount;
        LOGW_ACAC("%s: update lut failed\n", __FUNCTION__);
        return;
    }

    phwcfg->center_en = psta->strgCenter.hw_cacT_strgCenter_en;
    if (cacInfo->isp_unite_mode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID && psta->strgCenter.hw_cacT_strgCenter_en) {
            uint16_t w                     = cacInfo->rawWidth / 4;
            uint16_t e                     = cacInfo->multi_isp_extended_pixel / 4;
            uint16_t x                     = psta->strgCenter.hw_cacT_strgCenter_x;
            phwcfg->center_width = x - (w / 2 - e);
    }
    else {
        phwcfg->center_width = psta->strgCenter.hw_cacT_strgCenter_x;
    }
    phwcfg->center_height = psta->strgCenter.hw_cacT_strgCenter_y;

    phwcfg->psf_sft_bit = psta->psfParam.hw_cacT_psfShift_bits;
    phwcfg->cfg_num       = cacInfo->current_lut_[0]->Config.PsfCfgCount;
    phwcfg->buf_fd        = cacInfo->current_lut_[index]->Fd;
    phwcfg->hsize         = cacInfo->current_lut_[0]->Config.LutHCount * CacPsfKernelWordSizeInMemory;
    phwcfg->vsize         = cacInfo->current_lut_[0]->Config.LutVCount * CacChannelCount;
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        phwcfg->strength[i] =
            ROUND_F(pdyn->strgInterp.hw_cacT_corrStrength_table[i] * (1 << RKCAC_STRENGTH_FIX_BITS));
        phwcfg->strength[i] =
                phwcfg->strength[i] > 2047 ? 2047 : phwcfg->strength[i];
    }
    phwcfg->clip_g_mode = (int)pdyn->chromaAberrCorr.hw_cacT_clipG_mode;
    phwcfg->neg_clip0_en = pdyn->chromaAberrCorr.hw_cacT_negClip0_en;
    phwcfg->edge_detect_en = pdyn->strgInterp.hw_cacT_edgeDetect_en;
    phwcfg->flat_thed_b =
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctB_thred * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
    phwcfg->flat_thed_r =
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctR_thred * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
    phwcfg->offset_b = CLIP(
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctB_offset * (1 << RKCAC_EDGE_DETECT_FIX_BITS)), 0, 1 << 16);
    phwcfg->offset_r = CLIP(
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctR_offset * (1 << RKCAC_EDGE_DETECT_FIX_BITS)), 0, 1 << 16);
    int expoDctB_en = (int) !pdyn->chromaAberrCorr.sw_cacT_expoDctB_en;
    int expoDctR_en = (int)!pdyn->chromaAberrCorr.sw_cacT_expoDctR_en;
    phwcfg->expo_thed_b =
        (expoDctB_en << 20) | CLIP((int)(cacInfo->hdr_ratio * pdyn->chromaAberrCorr.hw_cacT_overExpoB_thred), 0, (1 << 20) - 1);
    phwcfg->expo_thed_r =
        (expoDctR_en << 20) | CLIP((int)(cacInfo->hdr_ratio * pdyn->chromaAberrCorr.hw_cacT_overExpoR_thred), 0, (1 << 20) - 1);
    phwcfg->expo_adj_b = CLIP((int)(cacInfo->hdr_ratio * pdyn->chromaAberrCorr.hw_cacT_overExpoB_adj), 0, (1 << 20) - 1);
    phwcfg->expo_adj_r = CLIP((int)(cacInfo->hdr_ratio * pdyn->chromaAberrCorr.hw_cacT_overExpoR_adj), 0, (1 << 20) - 1);
#if 0
    LOGD_ACAC("Dump CAC config: ");
    LOGD_ACAC("by en: %d",             phwcfg->bypass_en);
    LOGD_ACAC("center en: %d",         phwcfg->center_en);
    LOGD_ACAC("center x: %d",          phwcfg->center_width);
    LOGD_ACAC("center y: %d",          phwcfg->center_height);
    LOGD_ACAC("psf shift bits: %d",    phwcfg->psf_sft_bit);
    LOGD_ACAC("psf cfg num: %d",       phwcfg->cfg_num);
    LOGD_ACAC("psf buf fd: %d",        phwcfg->buf_fd);
    LOGD_ACAC("psf hwsize: %d",        phwcfg->hsize);
    LOGD_ACAC("psf vsize: %d",          phwcfg->vsize);
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("strength %d: %d", i, phwcfg->strength[i]);
    }
    LOGD_ACAC("clip_g_mode : %d",     phwcfg->clip_g_mode);
    LOGD_ACAC("edge_detect_en : %d",  phwcfg->edge_detect_en);
    LOGD_ACAC("neg_clip0_en : %d",    phwcfg->neg_clip0_en);
    LOGD_ACAC("flat_thed_b : %d",     phwcfg->flat_thed_b);
    LOGD_ACAC("flat_thed_r : %d",     phwcfg->flat_thed_r);
    LOGD_ACAC("offset_b : %d",        phwcfg->offset_b);
    LOGD_ACAC("offset_r : %d",        phwcfg->offset_r);
    LOGD_ACAC("expo_thed_b : %d",     phwcfg->expo_thed_b);
    LOGD_ACAC("expo_thed_r : %d",     phwcfg->expo_thed_r);
    LOGD_ACAC("expo_adj_b : %d",      phwcfg->expo_adj_b);
    LOGD_ACAC("expo_adj_r : %d",      phwcfg->expo_adj_r);
#endif
}

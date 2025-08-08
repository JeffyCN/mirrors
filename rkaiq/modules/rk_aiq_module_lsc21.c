#include "rk_aiq_isp39_modules.h"
#include "xcore/xcam_obj_debug.h"

#define c_lsc_ram_d_bw     26                 // double correction factor, must be even numbers

#define c_lsc_sample_bw    (c_lsc_ram_d_bw/2) // bit width of the correction factor values stored in RAM

#define c_lsc_corr_bw       15     // bit width of the correction factor values used internal.

#define c_lsc_grad_exp      15     // fixed exponent for the x and y gradients
#define c_lsc_corr_extend   10     // extended fractal part of dx,dy of internal correction factor
// constraint : c_lsc_corr_extend <= c_lsc_grad_exp
#define c_extend_round      (1 << (c_lsc_corr_extend - 1))

// bit width difference of correction factor values between used internal and stored in RAM
#define c_corr_diff  (c_lsc_corr_bw - c_lsc_sample_bw)

#define c_dx_shift   (c_lsc_grad_exp - c_lsc_corr_extend)
#define c_dx_round   (1 << (c_dx_shift - 1))
#define c_dy_shift   (c_lsc_grad_exp - c_lsc_corr_extend - c_corr_diff)
#define c_dy_round   (1 << (c_dy_shift - 1))

static float _sampPos[17] = {
                0, 0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5,
                0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375, 1};

// #define WRITE_OTP_TABLE

static uint8_t g_cam_otp_lsc_bypass = 0;

static void computeSamplingPoint(int size, int* pos, float* pos_f)
{
    if (pos_f == NULL) {
        LOGW_ALSC("wrong position percent, set to equalSector!\n");
        pos_f = _sampPos;
    }
    pos[0] = 0;
    for (int i = 1; i < 17; i++)
    {
        pos[i] = (int)(pos_f[i] * size) - 1;
    }

}

static void computeSamplingInterval(int width, int height, int *xInterval, int *yInterval)
{
    int xpos[17];
    int ypos[17];

    computeSamplingPoint(width, xpos, NULL);
    computeSamplingPoint(height, ypos, NULL);

    for (int i = 0; i < 16; i++)
    {
        xInterval[i] = xpos[i + 1] - xpos[i];
        yInterval[i] = ypos[i + 1] - ypos[i];
    }

    xInterval[0] = xInterval[0] + 1;
    yInterval[0] = yInterval[0] + 1;
}

static void computeGradient(int *xInterval, int *yInterval, int *xGradient, int *yGradient)
{
    // gradient N = INT(2^15 / SizeN + 0.5)
    for (int i = 0; i < 16; i++)
    {
        xGradient[i] = (int)(32768.0 / (double)xInterval[i] + 0.5);
        if (xGradient[i] > 8191)
        {
            xGradient[i] = 8191;
        }

        yGradient[i] = (int)(32768.0 / (double)yInterval[i] + 0.5);
        if (yGradient[i] > 8191)
        {
            yGradient[i] = 8191;
        }
    }
}

void calculateCorrectFactor(uint16_t *table, uint16_t *correctTable, int width, int height, int *xInterval, int *yInterval)
{
    int xGrad[16], yGrad[16];
    int xblk, yblk;
    uint16_t xbase, ybase, xoffset, yoffset;
    uint16_t curSizeX, curSizeY, curGradX, curGradY;
    uint16_t luCoeff, ldCoeff, ruCoeff, rdCoeff;
    uint32_t lCoeff, rCoeff, coeff;
    uint32_t tmp, tmp2;

    computeGradient(xInterval, yInterval, xGrad, yGrad);

    for (ybase = 0, yblk = 0; yblk < 16; yblk++)
    {
        curSizeY = yInterval[yblk];
        curGradY = yGrad[yblk];
        for (xbase = 0, xblk = 0; xblk < 16; xblk++)
        {
            curSizeX = xInterval[xblk];
            curGradX = xGrad[xblk];
            luCoeff = table[yblk * 17 + xblk];
            ldCoeff = table[(yblk + 1) * 17 + xblk];
            ruCoeff = table[yblk * 17 + xblk + 1];
            rdCoeff = table[(yblk + 1) * 17 + xblk + 1];
            for (yoffset = 0; yoffset < curSizeY; yoffset++)
            {
                tmp = abs(luCoeff - ldCoeff);
                tmp = tmp * curGradY;
                tmp = (tmp + c_dy_round) >> c_dy_shift;
                tmp = tmp * yoffset;
                tmp = (tmp + c_extend_round) >> c_lsc_corr_extend;
                tmp = (tmp << (32 - c_lsc_corr_bw)) >> (32 - c_lsc_corr_bw);
                lCoeff = luCoeff << c_corr_diff;
                lCoeff = (luCoeff > ldCoeff) ? (lCoeff - tmp) : (lCoeff + tmp);

                tmp = abs(ruCoeff - rdCoeff);
                tmp = tmp * curGradY;
                tmp = (tmp + c_dy_round) >> c_dy_shift;
                tmp = tmp * yoffset;
                tmp = (tmp + c_extend_round) >> c_lsc_corr_extend;
                tmp = (tmp << (32 - c_lsc_corr_bw)) >> (32 - c_lsc_corr_bw);
                rCoeff = ruCoeff << c_corr_diff;
                rCoeff = (ruCoeff > rdCoeff) ? (rCoeff - tmp) : (rCoeff + tmp);

                coeff = lCoeff << c_lsc_corr_extend;
                tmp = abs((int)(rCoeff - lCoeff));
                tmp = tmp * curGradX;
                tmp = (tmp + c_dx_round) >> c_dx_shift;
                for (xoffset = 0; xoffset < curSizeX; xoffset++)
                {
                    tmp2 = (coeff + c_extend_round) >> c_lsc_corr_extend;
                    tmp2 = (tmp2 > ((1 << c_lsc_corr_bw) - 1)) ? ((1 << c_lsc_corr_bw) - 1) : tmp2;
                    *(correctTable + (ybase + yoffset) * width + (xbase + xoffset)) = (uint16_t)tmp2 >> c_corr_diff;
                    coeff = (lCoeff > rCoeff) ? (coeff - tmp) : (coeff + tmp);
                }
            }
            xbase += curSizeX;
        }
        ybase += curSizeY;
    }
}

static int getBayerIndex(uint8_t pattern, int row, int col)
{
    int index = 0;
    int x, y;
    int tmp = 0;

    x = row % 2;
    y = col % 2;
    tmp = (x << 1) + y;

    if (pattern == 0)
    {
        index = tmp;
    } else if (pattern == 1) {
        switch (tmp)
        {
        case 0:
            index = 1;
            break;
        case 1:
            index = 0;
            break;
        case 2:
            index = 3;
            break;
        case 3:
            index = 2;
            break;
        }
    } else if (pattern == 2) {
        switch (tmp)
        {
        case 0:
            index = 2;
            break;
        case 1:
            index = 3;
            break;
        case 2:
            index = 0;
            break;
        case 3:
            index = 1;
            break;
        }
    } else if (pattern == 3) {
        index = 3 - tmp;
    }

    return index;
}

static void getLscParameter(uint16_t *r, uint16_t *gr, uint16_t *gb, uint16_t *b, uint16_t *table, int width, int height, uint8_t bayer)
{
    int bayerIdx;
    int idx;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            idx = i * width + j;
            bayerIdx = getBayerIndex(bayer, i, j);
            switch (bayerIdx)
            {
            case 0:
                table[idx] = r[idx];
                break;
            case 1:
                table[idx] = gr[idx];
                break;
            case 2:
                table[idx] = gb[idx];
                break;
            case 3:
                table[idx] = b[idx];
                break;
            default:
                break;
            }
        }
    }
}

static void separateBayerChannel(uint16_t* src, uint16_t* disR, uint16_t* disGr, uint16_t* disGb, uint16_t* disB, int width, int height, uint8_t bayer)
{
    int index = 0;
    int bayerIdx = 0;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            index = i * width + j;
            bayerIdx = getBayerIndex(bayer, i, j);
            switch (bayerIdx)
            {
            case 0:
                *(disR++) = *(src + index);
                break;
            case 1:
                *(disGr++) = *(src + index);
                break;
            case 2:
                *(disGb++) = *(src + index);
                break;
            case 3:
                *(disB++) = *(src + index);
                break;
            default:
                break;
            }
        }
    }
}

static void writeFile(char *fileName, uint16_t *buf)
{
    FILE *fd = fopen(fileName, "wb");
    if (fd == NULL)
        printf("%s: open failed: %s\n", __func__, fileName);
    else {
        fwrite(buf, 1, LSC_DATA_TBL_SIZE * sizeof(uint16_t), fd);
        fclose(fd);
    }
}

static int lsc_meshgain_otp_resize(uint16_t width, uint16_t height, otplsc_res_cvt_t* otp_lsc,
                                    uint8_t bayer_fmt, float* xPos_f, float* yPos_f)
{
    XCAM_STATIC_PROFILING_START(lsc_meshgain_otp_resize);

    int32_t srcLscWidth     = otp_lsc->width;
    int32_t srcLscHeight    = otp_lsc->height;
    int32_t dstWidth 		= width;
    int32_t dstHeight 		= height;

    if (dstWidth == srcLscWidth && height == srcLscHeight) {
        return 1;
    }

    LOGD_ALSC("res for otp param (%dx%d) mismatch raw res (%dx%d)\n",
            srcLscWidth, srcLscHeight, dstWidth, dstHeight);

    if (dstWidth > srcLscWidth && dstHeight > srcLscHeight)
    {
        if ((dstWidth / 2 <= srcLscWidth) && (dstHeight / 2 <= srcLscHeight))
        {
            dstWidth = dstWidth / 2;
            dstHeight = dstHeight / 2;
        } else {
            LOGE_ALSC("Failed to handle: the raw resolution exceeds!\n");
            return 0;
        }
    } else if (dstWidth < srcLscWidth && dstHeight < srcLscHeight) {
        if ((dstWidth <= srcLscWidth / 2) && (dstHeight <= srcLscHeight / 2))
        {
            srcLscWidth = srcLscWidth / 2;
            srcLscHeight = srcLscHeight / 2;
        }
    }

    uint32_t maxSize        = srcLscWidth > srcLscHeight ? srcLscWidth : srcLscHeight;
    uint32_t ratio          = maxSize > 3200 ? 8 : (maxSize > 1600 ? 4 : (maxSize > 800 ? 2 : 1));
    srcLscWidth             = 2 * (int32_t)(srcLscWidth / (ratio * 2));
    srcLscHeight            = 2 * (int32_t)(srcLscHeight / (ratio * 2));
    dstWidth                = 2 * (int32_t)(dstWidth / (ratio * 2));
    dstHeight               = 2 * (int32_t)(dstHeight / (ratio * 2));

    uint16_t *rTable = aiq_mallocz(sizeof(uint16_t) * srcLscWidth * srcLscHeight);
    uint16_t *grTable = aiq_mallocz(sizeof(uint16_t) * srcLscWidth * srcLscHeight);
    uint16_t *gbTable = aiq_mallocz(sizeof(uint16_t) * srcLscWidth * srcLscHeight);
    uint16_t *bTable = aiq_mallocz(sizeof(uint16_t) * srcLscWidth * srcLscHeight);
    uint16_t *corTable = aiq_mallocz(sizeof(uint16_t) * srcLscWidth * srcLscHeight);
    uint16_t *corTable_crop = aiq_mallocz(sizeof(uint16_t) *dstWidth * dstHeight);

#ifdef WRITE_OTP_TABLE
    char fileName[32] = {0};
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_r.bin");
    writeFile(fileName, otp_lsc->lsc_r);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_b.bin");
    writeFile(fileName, otp_lsc->lsc_b);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_gr.bin");
    writeFile(fileName, otp_lsc->lsc_gr);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_gb.bin");
    writeFile(fileName, otp_lsc->lsc_gb);
#endif

    // Interpolate gain table back to full size
    int sizeX[16];
    int sizeY[16];
    computeSamplingInterval(srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otp_lsc->lsc_r, rTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otp_lsc->lsc_gr, grTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otp_lsc->lsc_gb, gbTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otp_lsc->lsc_b, bTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    getLscParameter(rTable, grTable, gbTable, bTable, corTable,
                    srcLscWidth, srcLscHeight, bayer_fmt);

    // Clipping a Gain table is centered by default
    int cropoffsetx = srcLscWidth / 2 - dstWidth / 2;
    int cropoffsety = srcLscHeight / 2 - dstHeight / 2;

    if (cropoffsetx % 2 != 0)
        cropoffsetx = cropoffsetx - 1;
    if (cropoffsety % 2 != 0)
        cropoffsety = cropoffsety -1;

    for (int i = 0; i < dstHeight; i++)
    {
        memcpy(corTable_crop + i * dstWidth,
               corTable + (cropoffsety + i)*srcLscWidth + cropoffsetx, dstWidth * 2);
    }

    aiq_free(rTable);
    aiq_free(grTable);
    aiq_free(gbTable);
    aiq_free(bTable);
    aiq_free(corTable);

    // down-sampling again according to crop size
    uint16_t *plscdataR = aiq_mallocz(sizeof(uint16_t) *dstWidth / 2 * dstHeight / 2);
    uint16_t *plscdataGr = aiq_mallocz(sizeof(uint16_t) *dstWidth / 2 * dstHeight / 2);
    uint16_t *plscdataGb = aiq_mallocz(sizeof(uint16_t) *dstWidth / 2 * dstHeight / 2);
    uint16_t *plscdataB = aiq_mallocz(sizeof(uint16_t) *dstWidth / 2 * dstHeight / 2);
    memset(plscdataR, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataGr, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataGb, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataB, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    separateBayerChannel(corTable_crop, plscdataR, plscdataGr, plscdataGb, plscdataB, dstWidth, dstHeight, bayer_fmt);

    int cPos[17];       // col sampling
    int rPos[17];       // row sampling
    computeSamplingPoint(dstWidth / 2, cPos, xPos_f);
    computeSamplingPoint(dstHeight / 2, rPos, yPos_f);
    int r, c;
    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            r = rPos[i];
            c = cPos[j];
            *(otp_lsc->lsc_r + i * 17 + j) = plscdataR[r * dstWidth / 2 + c];
            *(otp_lsc->lsc_gr + i * 17 + j) = plscdataGr[r * dstWidth / 2 + c];
            *(otp_lsc->lsc_gb + i * 17 + j) = plscdataGb[r * dstWidth / 2 + c];
            *(otp_lsc->lsc_b + i * 17 + j) = plscdataB[r * dstWidth / 2 + c];
        }
    }

    otp_lsc->width = width;
    otp_lsc->height = height;

#ifdef WRITE_OTP_TABLE
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_r.bin");
    writeFile(fileName, otp_lsc->lsc_r);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_b.bin");
    writeFile(fileName, otp_lsc->lsc_b);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_gr.bin");
    writeFile(fileName, otp_lsc->lsc_gr);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_gb.bin");
    writeFile(fileName, otp_lsc->lsc_gb);
#endif

    aiq_free(plscdataR);
    aiq_free(plscdataGr);
    aiq_free(plscdataGb);
    aiq_free(plscdataB);
    aiq_free(corTable_crop);

    LOGD_ALSC("crop otp LscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
            otp_lsc->lsc_r[0], otp_lsc->lsc_r[1], otp_lsc->lsc_r[2], otp_lsc->lsc_r[3],
            otp_lsc->lsc_gr[0], otp_lsc->lsc_gr[1], otp_lsc->lsc_gr[2], otp_lsc->lsc_gr[3],
            otp_lsc->lsc_gb[0], otp_lsc->lsc_gb[1], otp_lsc->lsc_gb[2], otp_lsc->lsc_gb[3],
            otp_lsc->lsc_b[0], otp_lsc->lsc_b[1], otp_lsc->lsc_b[2], otp_lsc->lsc_b[3]);

    XCAM_STATIC_PROFILING_END(lsc_meshgain_otp_resize, 0);
    return 1;
}

bool rk_aiq_lsc21_params_check(void* attr, common_cvt_info_t *cvtinfo)
{
    //TODO: check if sum of x_size_tbl == raw width
    //TODO: check if sum of y_size_tbl == raw height

    return true;
}

static void lsc_meshgain_otp_cvt(uint16_t* meshGain_in, uint16_t* meshGain_otp, uint16_t* meshGain_out)
{
    if (!meshGain_in || !meshGain_otp) {
        LOGE_ALSC("%s: input params is error!\n", __func__);
        return;
    }

    for (int32_t i = 0; i < LSC_DATA_TBL_SIZE; i++) {
        meshGain_out[i] = CLIPBIT(((int)meshGain_in[i] * (int)meshGain_otp[i] + 512) >> 10, 13);
    }
}

void rk_aiq_lsc21_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    int i;
    struct isp3x_lsc_cfg * pFix = &isp_params->isp_cfg->others.lsc_cfg;
    lsc_param_t *lsc_param = (lsc_param_t *) attr;
    lsc_param_dyn_t * pdyn = &lsc_param->dyn;
    lsc_param_static_t * psta = &lsc_param->sta;

    pFix->sector_16x16 = 1;

    switch (psta->sw_lscT_meshGrid_mode) {
        case lsc_usrConfig_mode: {
            uint16_t x0, x1, y0, y1;
            x0 = (uint16_t)(psta->meshGrid.posX_f[0] * (float)cvtinfo->rawWidth);
            y0 = (uint16_t)(psta->meshGrid.posY_f[0] * (float)cvtinfo->rawHeight);
            for (i=0; i<LSC_MESHGRID_SIZE; i++) {
                x1 = (uint16_t)(psta->meshGrid.posX_f[i+1] * (float)cvtinfo->rawWidth);
                y1 = (uint16_t)(psta->meshGrid.posY_f[i+1] * (float)cvtinfo->rawHeight);
                pFix->x_size_tbl[i] = x1 - x0;
                pFix->y_size_tbl[i] = y1 - y0;
                x0 = x1;
                y0 = y1;

                pFix->x_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->x_size_tbl[i] + 0.5);
                pFix->y_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->y_size_tbl[i] + 0.5);
            }
        } break;
        case lsc_vendorDefault_mode:
        case lsc_equalSector_mode: {
            for (i=0; i<LSC_MESHGRID_SIZE; i++) {
                pFix->x_size_tbl[i] = (uint16_t)(_sampPos[i+1] * (float)cvtinfo->rawWidth) - (uint16_t)(_sampPos[i] * (float)cvtinfo->rawWidth);
                pFix->y_size_tbl[i] = (uint16_t)(_sampPos[i+1] * (float)cvtinfo->rawHeight) - (uint16_t)(_sampPos[i] * (float)cvtinfo->rawHeight);
                pFix->x_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->x_size_tbl[i] + 0.5);
                pFix->y_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->y_size_tbl[i] + 0.5);
            }
        } break;
        default:
            break;
    }

    // if x/y_size_tbl changed, otp_lsc.flag should be false
    if (cvtinfo->otp_lsc.flag) {
        if (0 != memcmp(cvtinfo->otplsc_res.x_size_tbl, pFix->x_size_tbl, sizeof(pFix->x_size_tbl)))
        {
            memcpy(cvtinfo->otplsc_res.x_size_tbl, pFix->x_size_tbl, sizeof(pFix->x_size_tbl));
            cvtinfo->otplsc_res.flag = false;
        }
        if (0 != memcmp(cvtinfo->otplsc_res.y_size_tbl, pFix->y_size_tbl, sizeof(pFix->y_size_tbl)))
        {
            memcpy(cvtinfo->otplsc_res.y_size_tbl, pFix->y_size_tbl, sizeof(pFix->y_size_tbl));
            cvtinfo->otplsc_res.flag = false;
        }
    }

    if (cvtinfo->otp_lsc.flag && !cvtinfo->otplsc_res.flag) {
        cvtinfo->otplsc_res.width = cvtinfo->otp_lsc.width;
        cvtinfo->otplsc_res.height = cvtinfo->otp_lsc.height;
        memcpy(cvtinfo->otplsc_res.lsc_r, cvtinfo->otp_lsc.lsc_r, sizeof(cvtinfo->otplsc_res.lsc_r));
        memcpy(cvtinfo->otplsc_res.lsc_gr, cvtinfo->otp_lsc.lsc_gr, sizeof(cvtinfo->otplsc_res.lsc_gr));
        memcpy(cvtinfo->otplsc_res.lsc_gb, cvtinfo->otp_lsc.lsc_gb, sizeof(cvtinfo->otplsc_res.lsc_gb));
        memcpy(cvtinfo->otplsc_res.lsc_b, cvtinfo->otp_lsc.lsc_b, sizeof(cvtinfo->otplsc_res.lsc_b));
        int ret = 0;
        if (psta->sw_lscT_meshGrid_mode == lsc_usrConfig_mode) {
            ret = lsc_meshgain_otp_resize(cvtinfo->rawWidth, cvtinfo->rawHeight,
                                    &cvtinfo->otplsc_res, cvtinfo->bayer_fmt,
                                    psta->meshGrid.posX_f, psta->meshGrid.posY_f);
        } else {
            ret = lsc_meshgain_otp_resize(cvtinfo->rawWidth, cvtinfo->rawHeight, &cvtinfo->otplsc_res,
                                    cvtinfo->bayer_fmt, NULL, NULL);
        }
        cvtinfo->otplsc_res.flag = ret;
    }

#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};
    char property_value_default[PROPERTY_VALUE_MAX] = {0};
    sprintf(property_value_default, "%d", g_cam_otp_lsc_bypass);
    property_get("persist.vendor.rkisp.otplsc.bypass", property_value, property_value_default);
    g_cam_otp_lsc_bypass = strtoull(property_value, NULL, 0);
#else
    char* valueStr = getenv("persist_camera_otp_lsc_bypass");
    if (valueStr) {
        g_cam_otp_lsc_bypass = strtoull(valueStr, NULL, 0);
    }
#endif

    if (!g_cam_otp_lsc_bypass && cvtinfo->otplsc_res.flag) {
        lsc_meshgain_otp_cvt(pdyn->meshGain.hw_lscC_gainR_val, cvtinfo->otplsc_res.lsc_r, pFix->r_data_tbl);
        lsc_meshgain_otp_cvt(pdyn->meshGain.hw_lscC_gainGr_val, cvtinfo->otplsc_res.lsc_gr, pFix->gr_data_tbl);
        lsc_meshgain_otp_cvt(pdyn->meshGain.hw_lscC_gainGb_val, cvtinfo->otplsc_res.lsc_gb, pFix->gb_data_tbl);
        lsc_meshgain_otp_cvt(pdyn->meshGain.hw_lscC_gainB_val, cvtinfo->otplsc_res.lsc_b, pFix->b_data_tbl);

        LOGD_ALSC("hwi otp r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
                pFix->r_data_tbl[0], pFix->r_data_tbl[1], pFix->r_data_tbl[2], pFix->r_data_tbl[3],
                pFix->gr_data_tbl[0], pFix->gr_data_tbl[1], pFix->gr_data_tbl[2], pFix->gr_data_tbl[3],
                pFix->gb_data_tbl[0], pFix->gb_data_tbl[1], pFix->gb_data_tbl[2], pFix->gb_data_tbl[3],
                pFix->b_data_tbl[0], pFix->b_data_tbl[1], pFix->b_data_tbl[2], pFix->b_data_tbl[3]);
    } else {
        if (cvtinfo->otplsc_res.flag) {
            LOGK_ALSC("lsc otp disable by g_cam_otp_lsc_bypass! \n");
        }
        for (i=0; i<LSC_LSCTABLE_SIZE; i++) {
            pFix->r_data_tbl[i] = CLIPBIT(pdyn->meshGain.hw_lscC_gainR_val[i], 13);
            pFix->gr_data_tbl[i] = CLIPBIT(pdyn->meshGain.hw_lscC_gainGr_val[i], 13);
            pFix->gb_data_tbl[i] = CLIPBIT(pdyn->meshGain.hw_lscC_gainGb_val[i], 13);
            pFix->b_data_tbl[i] = CLIPBIT(pdyn->meshGain.hw_lscC_gainB_val[i], 13);
        }
    }

    return;
}

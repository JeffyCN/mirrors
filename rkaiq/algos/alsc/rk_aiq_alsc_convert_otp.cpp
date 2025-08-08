#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcam_std.h>
#include "xcam_log.h"
#include "rk_aiq_alsc_convert_otp.h"

// #define WRITE_OTP_TABLE 1

void convertLscTableParameter(resolution_t *cur_res, alsc_otp_grad_t *otpGrad,
                              RkAiqBayerPattern_t bayerPattern)
{
    XCAM_STATIC_PROFILING_START(convertLscTable);

    int32_t bayer 			= bayerPattern;
    int32_t srcLscWidth     = otpGrad->width;
    int32_t srcLscHeight    = otpGrad->height;
    int32_t dstWidth 		= cur_res->width;
    int32_t dstHeight 		= cur_res->height;

    LOGD_ALSC("input params: src %dx%d, dst %dx%d, bayer: %d\n",
            srcLscWidth, srcLscHeight, dstWidth,
            dstHeight, bayer);

    if (dstWidth == srcLscWidth && dstHeight == srcLscHeight) {
        return;
    }

    if (dstWidth > srcLscWidth && dstHeight > srcLscHeight)
    {
        if ((dstWidth / 2 <= srcLscWidth) && (dstHeight / 2 <= srcLscHeight))
        {
            dstWidth = dstWidth / 2;
            dstHeight = dstHeight / 2;
        } else {
            LOGE_ALSC("Failed to handle: src %dx%d, dst %dx%x, return!\n",
                    srcLscWidth, srcLscHeight, dstWidth, dstHeight);
            return;
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

    uint16_t *rTable = new uint16_t[srcLscWidth * srcLscHeight];
    uint16_t *grTable = new uint16_t[srcLscWidth * srcLscHeight];
    uint16_t *gbTable = new uint16_t[srcLscWidth * srcLscHeight];
    uint16_t *bTable = new uint16_t[srcLscWidth * srcLscHeight];
    uint16_t *corTable = new uint16_t[srcLscWidth * srcLscHeight];
    uint16_t *corTable_crop = new uint16_t[dstWidth * dstHeight];

#if WRITE_OTP_TABLE
    char fileName[32] = {0};
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_r.bin");
    writeFile(fileName, otpGrad->lsc_r);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_b.bin");
    writeFile(fileName, otpGrad->lsc_b);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_gr.bin");
    writeFile(fileName, otpGrad->lsc_gr);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/otp_lsc_gb.bin");
    writeFile(fileName, otpGrad->lsc_gb);
#endif

    // Interpolate gain table back to full size
    int sizeX[16];
    int sizeY[16];
    computeSamplingInterval(srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otpGrad->lsc_r, rTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otpGrad->lsc_gr, grTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otpGrad->lsc_gb, gbTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    calculateCorrectFactor(otpGrad->lsc_b, bTable, srcLscWidth, srcLscHeight, sizeX, sizeY);
    getLscParameter(rTable, grTable, gbTable, bTable, corTable, srcLscWidth, srcLscHeight, bayer);

    // Clipping a Gain table is centered by default
    int cropoffsetx = srcLscWidth / 2 - dstWidth / 2;
    int cropoffsety = srcLscHeight / 2 - dstHeight / 2;

    if (cropoffsetx % 2 != 0)
        cropoffsetx = cropoffsetx - 1;
    if (cropoffsety % 2 != 0)
        cropoffsety = cropoffsety -1;

    for (int i = 0; i < dstHeight; i++)
    {

        memcpy(corTable_crop + i * dstWidth, corTable + (cropoffsety + i)*srcLscWidth + cropoffsetx, dstWidth * 2);
        //*(corTable_crop + i*dstWidth + j) = *(corTable + (cropoffsety + i)*srcLscWidth + j + cropoffsetx);
    }

    delete[] rTable;
    delete[] grTable;
    delete[] gbTable;
    delete[] bTable;
    delete[] corTable;


    // down-sampling again according to crop size
    uint16_t *plscdataR = new uint16_t[dstWidth / 2 * dstHeight / 2];
    uint16_t *plscdataGr = new uint16_t[dstWidth / 2 * dstHeight / 2];
    uint16_t *plscdataGb = new uint16_t[dstWidth / 2 * dstHeight / 2];
    uint16_t *plscdataB = new uint16_t[dstWidth / 2 * dstHeight / 2];

    memset(plscdataR, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataGr, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataGb, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);
    memset(plscdataB, 0, sizeof(uint16_t)*dstWidth / 2 * dstHeight / 2);

    separateBayerChannel(corTable_crop, plscdataR, plscdataGr, plscdataGb, plscdataB, dstWidth, dstHeight, bayer);


    int cPos[17];       // col sampling
    int rPos[17];       // row sampling
    computeSamplingPoint(dstWidth / 2, cPos);
    computeSamplingPoint(dstHeight / 2, rPos);
    int r, c;
    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            r = rPos[i];
            c = cPos[j];

            *(otpGrad->lsc_r + i * 17 + j) = plscdataR[r * dstWidth / 2 + c];
            *(otpGrad->lsc_gr + i * 17 + j) = plscdataGr[r * dstWidth / 2 + c];
            *(otpGrad->lsc_gb + i * 17 + j) = plscdataGb[r * dstWidth / 2 + c];
            *(otpGrad->lsc_b + i * 17 + j) = plscdataB[r * dstWidth / 2 + c];
        }
    }

#if WRITE_OTP_TABLE
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_r.bin");
    writeFile(fileName, otpGrad->lsc_r);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_b.bin");
    writeFile(fileName, otpGrad->lsc_b);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_gr.bin");
    writeFile(fileName, otpGrad->lsc_gr);
    snprintf(fileName, sizeof(fileName) - 1, "/tmp/converted_otp_lsc_gb.bin");
    writeFile(fileName, otpGrad->lsc_gb);
#endif

    delete[] plscdataR;
    delete[] plscdataGr;
    delete[] plscdataGb;
    delete[] plscdataB;
    delete[] corTable_crop;

    LOGD_ALSC("crop otp LscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
            otpGrad->lsc_r[0],
            otpGrad->lsc_r[1],
            otpGrad->lsc_r[2],
            otpGrad->lsc_r[3],
            otpGrad->lsc_gr[0],
            otpGrad->lsc_gr[1],
            otpGrad->lsc_gr[2],
            otpGrad->lsc_gr[3],
            otpGrad->lsc_gb[0],
            otpGrad->lsc_gb[1],
            otpGrad->lsc_gb[2],
            otpGrad->lsc_gb[3],
            otpGrad->lsc_b[0],
            otpGrad->lsc_b[1],
            otpGrad->lsc_b[2],
            otpGrad->lsc_b[3]);

    XCAM_STATIC_PROFILING_END(convertLscTable, 0);
}

void computeSamplingPoint(int size, int *pos)
{
    float sampPos[17] = { 0.0, 6.25, 12.5, 18.75, 25.0, 31.25, 37.5, 43.75, 50.0,
                          56.25, 62.5, 68.75, 75.0, 81.25, 87.5, 93.75, 100.0
                        };
    float tmpVal;

    pos[0] = 0;
    for (int i = 1; i < 17; i++)
    {
        tmpVal = sampPos[i] / 100;
        pos[i] = (int)(tmpVal * size) - 1;
    }

}

void computeSamplingInterval(int width, int height, int *xInterval, int *yInterval)
{
    int xpos[17];
    int ypos[17];

    computeSamplingPoint(width, xpos);
    computeSamplingPoint(height, ypos);

    for (int i = 0; i < 16; i++)
    {
        xInterval[i] = xpos[i + 1] - xpos[i];
        yInterval[i] = ypos[i + 1] - ypos[i];
    }

    xInterval[0] = xInterval[0] + 1;
    yInterval[0] = yInterval[0] + 1;
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

void getLscParameter(uint16_t *r, uint16_t *gr, uint16_t *gb, uint16_t *b, uint16_t *table, int width, int height, int bayer)
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

void separateBayerChannel(uint16_t* src, uint16_t* disR, uint16_t* disGr, uint16_t* disGb, uint16_t* disB, int width, int height, int bayer)
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
void computeGradient(int *xInterval, int *yInterval, int *xGradient, int *yGradient)
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
int getBayerIndex(int pattern, int row, int col)
{
    int index = 0;
    int x, y;
    int tmp = 0;

    x = row % 2;
    y = col % 2;

    if (x == 0 && y == 0)
    {
        tmp = 0;
    }
    else if (x == 0 && y == 1)
    {
        tmp = 1;
    }
    else if (x == 1 && y == 0)
    {
        tmp = 2;
    }
    else if (x == 1 && y == 1)
    {
        tmp = 3;
    }

    if (pattern == BAYER_BGGR)
    {
        switch (tmp)
        {
        case 0:
            index = B_INDEX;
            break;
        case 1:
            index = GB_INDEX;
            break;
        case 2:
            index = GR_INDEX;
            break;
        case 3:
            index = R_INDEX;
            break;
        }
    }
    else if (pattern == BAYER_GBRG)
    {
        switch (tmp)
        {
        case 0:
            index = GB_INDEX;
            break;
        case 1:
            index = B_INDEX;
            break;
        case 2:
            index = R_INDEX;
            break;
        case 3:
            index = GR_INDEX;
            break;
        }
    }
    else if (pattern == BAYER_GRBG)
    {
        switch (tmp)
        {
        case 0:
            index = GR_INDEX;
            break;
        case 1:
            index = R_INDEX;
            break;
        case 2:
            index = B_INDEX;
            break;
        case 3:
            index = GB_INDEX;
            break;
        }
    }
    else if (pattern == BAYER_RGGB)
    {
        switch (tmp)
        {
        case 0:
            index = R_INDEX;
            break;
        case 1:
            index = GR_INDEX;
            break;
        case 2:
            index = GB_INDEX;
            break;
        case 3:
            index = B_INDEX;
            break;
        }
    }

    return index;
}

void writeFile(char *fileName, uint16_t *buf)
{
    FILE *fd = fopen(fileName, "wb");
    if (fd == NULL)
        printf("%s: open failed: %s\n", __func__, fileName);
    else {
        fwrite(buf, 1, LSCDATA_LEN * sizeof(uint16_t), fd);
        fclose(fd);
    }
}


void lightFallOff(uint16_t *table, float *percenttable, float percent, int width, int height)
{
    // float ratio = (float)width / (float)height;
    // float rMax = sqrt(pow((float)(16 / 2 * ratio), 2) + pow((float)(16 / 2), 2));
    // float r = 0.0, alpha = 0.0, falloff = 0.0, halfPi = 3.1415926 / 2;

    percent = percent / 100.0;

    uint16_t tmpTable[17 * 17];
    memcpy(tmpTable, table, sizeof(uint16_t) * 17 * 17);

    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            table[i * 17 + j] = (float)(table[i * 17 + j] - 1024) * percent + 1024.0;
            percenttable[i * 17 + j] = (float)table[i * 17 + j] / (float)tmpTable[i * 17 + j];
        }
    }
}

void adjustVignettingForLscOTP(uint16_t *tableB, uint16_t *tableGb, uint16_t *tableGr, uint16_t *tableR, float newVig, int width, int height)
{
    const float falloffRevert70[17 * 17] = {
        1.42857, 1.3392, 1.27634, 1.22754, 1.18984, 1.16169, 1.14214, 1.13063, 1.12683, 1.13063, 1.14214, 1.16169, 1.18984, 1.22754, 1.27634, 1.3392, 1.42857,
        1.37424, 1.29764, 1.23862, 1.19214, 1.15602, 1.12897, 1.11015, 1.09905, 1.09539, 1.09905, 1.11015, 1.12897, 1.15602, 1.19214, 1.23862, 1.29764, 1.37424,
        1.33537, 1.26382, 1.20742, 1.16268, 1.12778, 1.10158, 1.08335, 1.07259, 1.06904, 1.07259, 1.08335, 1.10158, 1.12778, 1.16268, 1.20742, 1.26382, 1.33537,
        1.30481, 1.23644, 1.18195, 1.13853, 1.10458, 1.07908, 1.06131, 1.05082, 1.04735, 1.05082, 1.06131, 1.07908, 1.10458, 1.13853, 1.18195, 1.23644, 1.30481,
        1.28095, 1.21477, 1.16169, 1.11927, 1.08606, 1.06109, 1.04368, 1.0334, 1.03, 1.0334, 1.04368, 1.06109, 1.08606, 1.11927, 1.16169, 1.21477, 1.28095,
        1.26297, 1.19832, 1.14626, 1.10458, 1.07193, 1.04735, 1.03021, 1.02009, 1.01675, 1.02009, 1.03021, 1.04735, 1.07193, 1.10458, 1.14626, 1.19832, 1.26297,
        1.25042, 1.18678, 1.13541, 1.09425, 1.06197, 1.03767, 1.02072, 1.01071, 1.0074, 1.01071, 1.02072, 1.03767, 1.06197, 1.09425, 1.13541, 1.18678, 1.25042,
        1.24299, 1.17993, 1.12897, 1.0881, 1.05605, 1.03191, 1.01508, 1.00513, 1.00184, 1.00513, 1.01508, 1.03191, 1.05605, 1.0881, 1.12897, 1.17993, 1.24299,
        1.24053, 1.17766, 1.12683, 1.08606, 1.05408, 1.03, 1.0132, 1.00328, 1, 1.00328, 1.0132, 1.03, 1.05408, 1.08606, 1.12683, 1.17766, 1.24053,
        1.24299, 1.17993, 1.12897, 1.0881, 1.05605, 1.03191, 1.01508, 1.00513, 1.00184, 1.00513, 1.01508, 1.03191, 1.05605, 1.0881, 1.12897, 1.17993, 1.24299,
        1.25042, 1.18678, 1.13541, 1.09425, 1.06197, 1.03767, 1.02072, 1.01071, 1.0074, 1.01071, 1.02072, 1.03767, 1.06197, 1.09425, 1.13541, 1.18678, 1.25042,
        1.26297, 1.19832, 1.14626, 1.10458, 1.07193, 1.04735, 1.03021, 1.02009, 1.01675, 1.02009, 1.03021, 1.04735, 1.07193, 1.10458, 1.14626, 1.19832, 1.26297,
        1.28095, 1.21477, 1.16169, 1.11927, 1.08606, 1.06109, 1.04368, 1.0334, 1.03, 1.0334, 1.04368, 1.06109, 1.08606, 1.11927, 1.16169, 1.21477, 1.28095,
        1.30481, 1.23644, 1.18195, 1.13853, 1.10458, 1.07908, 1.06131, 1.05082, 1.04735, 1.05082, 1.06131, 1.07908, 1.10458, 1.13853, 1.18195, 1.23644, 1.30481,
        1.33537, 1.26382, 1.20742, 1.16268, 1.12778, 1.10158, 1.08335, 1.07259, 1.06904, 1.07259, 1.08335, 1.10158, 1.12778, 1.16268, 1.20742, 1.26382, 1.33537,
        1.37424, 1.29764, 1.23862, 1.19214, 1.15602, 1.12897, 1.11015, 1.09905, 1.09539, 1.09905, 1.11015, 1.12897, 1.15602, 1.19214, 1.23862, 1.29764, 1.37424,
        1.42857, 1.3392, 1.27634, 1.22754, 1.18984, 1.16169, 1.14214, 1.13063, 1.12683, 1.13063, 1.14214, 1.16169, 1.18984, 1.22754, 1.27634, 1.3392, 1.42857 };

	for (int i = 0; i < 17 * 17; i++)
	{
		tableB[i] = tableB[i] < 1024 ? 1024 : tableB[i];
		tableGb[i] = tableGb[i] < 1024 ? 1024 : tableGb[i];
		tableGr[i] = tableGr[i] < 1024 ? 1024 : tableGr[i];
		tableR[i] = tableR[i] < 1024 ? 1024 : tableR[i];
	}

    // revert to 100 vig
    for (int i = 0; i < 17 * 17; i++)
    {
        tableB[i] = (float)(tableB[i] - 1024) * falloffRevert70[i] + 1024.0 +0.5;
        tableGb[i] = (float)(tableGb[i] - 1024)  * falloffRevert70[i] + 1024.0 + 0.5;
        tableGr[i] = (float)(tableGr[i] - 1024)  * falloffRevert70[i] + 1024.0 + 0.5;
        tableR[i] = (float)(tableR[i] - 1024)  * falloffRevert70[i] + 1024.0 + 0.5;
    }

    // calculate new falloff table
    float newFalloffTable[17 * 17];
    lightFallOff(tableGb, newFalloffTable, newVig, width, height);

    // apply to other channel
    for (int i = 0; i < 17 * 17; i++)
    {
        tableB[i] = (float)tableB[i] * newFalloffTable[i] + 0.5;
        tableGr[i] = (float)tableGr[i] * newFalloffTable[i] + 0.5;
        tableR[i] = (float)tableR[i] * newFalloffTable[i] + 0.5;
    }
}

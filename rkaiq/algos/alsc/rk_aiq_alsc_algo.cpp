/*
* rk_aiq_alsc_algo.cpp

* for rockchip v2.0.0
*
*  Copyright (c) 2019 Rockchip Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
/* for rockchip v2.0.0*/

#include "rk_aiq_algo_types.h"
#include "alsc/rk_aiq_alsc_algo.h"
#include "xcam_log.h"
#include "interpolation.h"
#include "RkAiqCalibDbV2Helper.h"
#include "rk_aiq_alsc_convert_otp.h"

RKAIQ_BEGIN_DECLARE


/******************************************************************************
* Test Function
******************************************************************************/
#define _TEST_LSC_VALID_ON_HARDWARE 0
#if _TEST_LSC_VALID_ON_HARDWARE
/** @brief set lsc 4 channel table to the given value*/
static void _memset_lsc(rk_aiq_lsc_cfg_t& tb, uint16_t value) {
    for (uint32_t i = 0; i < sizeof(tb.r_data_tbl)/sizeof(tb.r_data_tbl[0]); i++)
    {
        tb.r_data_tbl[i] = value;
    }
    for (uint32_t i = 0; i < sizeof(tb.gr_data_tbl)/sizeof(tb.gr_data_tbl[0]); i++)
    {
        tb.gr_data_tbl[i] = value;
    }
    for (uint32_t i = 0; i < sizeof(tb.gb_data_tbl)/sizeof(tb.gb_data_tbl[0]); i++)
    {
        tb.gb_data_tbl[i] = value;
    }
    for (uint32_t i = 0; i < sizeof(tb.b_data_tbl)/sizeof(tb.b_data_tbl[0]); i++)
    {
        tb.b_data_tbl[i] = value;
    }
}

/** @brief test function: if lsc config is valid in hardware, you will see blink on liveview.*/
static void _test_if_hw_lsc_valid(alsc_handle_t hAlsc)
{
#define ONE_TIME_STATE      1
#define THREE_TIME_STATE    3

    static int cur_state = ONE_TIME_STATE;
    int next_state = cur_state;

    switch (cur_state)
    {
        case ONE_TIME_STATE: {
            next_state = THREE_TIME_STATE;
            _memset_lsc(hAlsc->lscHwConf, 1024);
        } break;

        case THREE_TIME_STATE: {
            next_state = ONE_TIME_STATE;
            _memset_lsc(hAlsc->lscHwConf, 3072);
        } break;

        default:
            break;
    }
    cur_state = next_state;
}
#endif

static void genLscMatrixToHwConf(lsc_matrix_t *dampedLscMatrixTable, rk_aiq_lsc_cfg_t *lscHwConf, alsc_otp_grad_t *otpGrad)
{
    if (!dampedLscMatrixTable || !lscHwConf) {
        LOGE_ALSC("%s: input params is error!\n", __func__);
        return;
    }

    // apply sensor lsc otp
    for (int32_t i = 0; i < LSC_DATA_TBL_SIZE; i++) {
        lscHwConf->r_data_tbl[i]    = dampedLscMatrixTable->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i] * \
                                            (float(otpGrad->lsc_r[i]) / 1024) + 0.5;
        lscHwConf->gr_data_tbl[i]   = dampedLscMatrixTable->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i] * \
                                            (float(otpGrad->lsc_gr[i]) / 1024) + 0.5;
        lscHwConf->gb_data_tbl[i]   = dampedLscMatrixTable->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i] * \
                                            (float(otpGrad->lsc_gb[i]) / 1024) + 0.5;
        lscHwConf->b_data_tbl[i]    = dampedLscMatrixTable->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i] * \
                                            (float(otpGrad->lsc_b[i]) / 1024) + 0.5;
    }
 }

XCamReturn illuminant_index_estimation(alsc_mode_data_t& alsc_mode_data, float awbGain[2], uint32_t& illu_case_id)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    float minDist = 9999999;
    float dist;
    float& nRG = awbGain[0]; //current R/G gain
    float& nBG = awbGain[1]; //current B/G gain

    for(uint32_t index = 0; index < alsc_mode_data.illu_case_count; index++)
    {
        pIlluCase_t illu_case = alsc_mode_data.illu_case[index];
        float refRG = illu_case->alsc_cof->wbGain[0];
        float refBG = illu_case->alsc_cof->wbGain[1];
        dist = sqrt((nRG - refRG) * (nRG -  refRG) + (nBG -  refBG) * (nBG -  refBG));
        if(dist < minDist)
        {
            minDist = dist;
            illu_case_id = index;
            ret = XCAM_RETURN_NO_ERROR;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ALSC("fail to estimate illuminant!!!\n");
    }

    LOGD_ALSC( "wbGain:%f,%f, estimation illuminant  is %s(%d) \n", awbGain[0], awbGain[1],
               alsc_mode_data.illu_case[illu_case_id]->alsc_cof->name, illu_case_id);
    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return ret;
}

static XCamReturn OrderLscProfilesByVignetting
(
    pLscTableProfile_t* pLscProfiles,
    const uint32_t      cnt
) {
    uint32_t i, j;

    for (i = 0; i < (cnt - 1); ++i) {
        for (j = 0; j < (cnt - i - 1); ++j) {
            if (pLscProfiles[j]->vignetting < pLscProfiles[j + 1]->vignetting) {
                pLscTableProfile_t temp   = pLscProfiles[j];
                pLscProfiles[j]         = pLscProfiles[j + 1];
                pLscProfiles[j + 1]       = temp;
            }
        }
    }

    return (XCAM_RETURN_NO_ERROR);
}

/** @brief assume that the vignetting is from large to small*/
static XCamReturn VignSelectLscProfiles
(
    alsc_illu_case_t*   illu_case,
    const float         fVignetting,
    pLscTableProfile_t  &ceilling,
    pLscTableProfile_t  &floor
) {
    XCamReturn XCamReturn = XCAM_RETURN_NO_ERROR;
    alsc_illu_case_resolution_t& current_res = illu_case->res_group[illu_case->current_res_idx];

    if (fVignetting >= current_res.lsc_table_group[0]->vignetting) {
        floor = current_res.lsc_table_group[0];
        ceilling = floor;
        LOGD_ALSC( "select:%s \n", floor->name);
        XCamReturn = XCAM_RETURN_ERROR_OUTOFRANGE;
    } else {
        uint32_t nLast = current_res.lsc_table_count - 1;
        if (fVignetting <= current_res.lsc_table_group[nLast]->vignetting) {
            ceilling = current_res.lsc_table_group[nLast];
            floor = ceilling;
            LOGD_ALSC( "select:%s \n", ceilling->name);
            XCamReturn = XCAM_RETURN_ERROR_OUTOFRANGE;
        } else {
            uint32_t n = 0;

            /* find the segment */
            while ((n <= nLast) && (fVignetting <= current_res.lsc_table_group[n]->vignetting)) {
                n++;
            }

            if (n > 0)
                n--;

            ceilling = current_res.lsc_table_group[n];
            floor = current_res.lsc_table_group[n + 1];

        }
    }

    return (XCamReturn);
}



/******************************************************************************
 * InterpolateMatrices
 *****************************************************************************/
static XCamReturn VignInterpolateMatrices
(
    const float             fVignetting,
    pLscTableProfile_t   pLscProfile1,
    pLscTableProfile_t   pLscProfile2,
    lsc_matrix_t*          pResMatrix
) {
    XCamReturn iXCamReturn = XCAM_RETURN_ERROR_PARAM;

    if ((pLscProfile1 != NULL) && (pLscProfile2 != NULL) && (pResMatrix != NULL)) {
        float fVigA = pLscProfile1->vignetting;
        float fVigB = pLscProfile2->vignetting;

        float f1 = (fVigB - fVignetting) / (fVigB - fVigA);
        /* FLOAT f2 = ( fVigB - fVignetting ) / ( fVigB - fVigA ); */
        float f2 = 1.0f - f1;
        LOGD_ALSC( "select:%s :%f  and %s :%f", pLscProfile1->name,
                   f1, pLscProfile2->name, f2);

        /* left shift 16 */
        uint32_t f1_ = (uint32_t)(f1 * 65536.0f);
        uint32_t f2_ = (uint32_t)(f2 * 65536.0f);

        int16_t i;

        uint32_t red;
        uint32_t greenr;
        uint32_t greenb;
        uint32_t blue;

        for (i = 0; i < (LSC_DATA_TBL_SIZE); i++) {
            red     = (f1_ * (uint32_t)pLscProfile1->lsc_samples_red.uCoeff[i])
                      + (f2_ * (uint32_t)pLscProfile2->lsc_samples_red.uCoeff[i]);

            greenr  = (f1_ * (uint32_t)pLscProfile1->lsc_samples_greenR.uCoeff[i])
                      + (f2_ * (uint32_t)pLscProfile2->lsc_samples_greenR.uCoeff[i]);

            greenb  = (f1_ * (uint32_t)pLscProfile1->lsc_samples_greenB.uCoeff[i])
                      + (f2_ * (uint32_t)pLscProfile2->lsc_samples_greenB.uCoeff[i]);

            blue    = (f1_ * (uint32_t)pLscProfile1->lsc_samples_blue.uCoeff[i])
                      + (f2_ * (uint32_t)pLscProfile2->lsc_samples_blue.uCoeff[i]);

            /* with round up (add 65536/2 <=> 0.5) before right shift */
            pResMatrix->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i]    = (uint16_t)((red + (65536 >> 1)) >> 16);
            pResMatrix->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i] = (uint16_t)((greenr + (65536 >> 1)) >> 16);
            pResMatrix->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i] = (uint16_t)((greenb + (65536 >> 1)) >> 16);
            pResMatrix->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i]   = (uint16_t)((blue + (65536 >> 1)) >> 16);
        }

        iXCamReturn = XCAM_RETURN_NO_ERROR;
    }

    return (iXCamReturn);
}


/******************************************************************************
 * Damping
 *****************************************************************************/
static XCamReturn Damping
(
    float     damp,               /**< damping coefficient */
    lsc_matrix_t*  pMatrixUndamped,   /**< undamped new computed matrices */
    rk_aiq_lsc_cfg_t *lscHwConf,
    bool* converge
) {
    XCamReturn XCamReturn = XCAM_RETURN_ERROR_PARAM;

    if ((pMatrixUndamped != NULL) && (lscHwConf != NULL)) {
        /* left shift 16 */
        uint32_t f1_ = (uint32_t)(damp * 65536.0f);
        uint32_t f2_ = (uint32_t)(65536U - f1_);

        int16_t i;

        uint32_t red;
        uint32_t greenr;
        uint32_t greenb;
        uint32_t blue;
        float dis=0;
        float dis_th=0.015;
        bool speedDamp = true;
        dis = (float)(lscHwConf->r_data_tbl[0]-
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0])/
            (lscHwConf->r_data_tbl[0]+
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]);
        speedDamp = fabs(dis)<dis_th;
        dis = (float)(lscHwConf->b_data_tbl[LSC_DATA_TBL_SIZE-1]-
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1])/
            (lscHwConf->b_data_tbl[LSC_DATA_TBL_SIZE-1]+
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1]);
        speedDamp &= fabs(dis)<dis_th;
        if(speedDamp){
            damp = 0;
            f1_ = 0;
            f2_ = 65536;
        }

        for (i = 0; i < (LSC_DATA_TBL_SIZE); i++) {
            red     = (f1_ * (uint32_t)lscHwConf->r_data_tbl[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i]);

            greenr  = (f1_ * (uint32_t)lscHwConf->gr_data_tbl[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i]);

            greenb  = (f1_ * (uint32_t)lscHwConf->gb_data_tbl[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i]);

            blue    = (f1_ * (uint32_t)lscHwConf->b_data_tbl[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i]);

            /* with round up (add 65536/2 <=> 0.5) before right shift */
            lscHwConf->r_data_tbl[i]  = (uint16_t)((red    + (65536 >> 1)) >> 16);
            lscHwConf->gr_data_tbl[i] = (uint16_t)((greenr + (65536 >> 1)) >> 16);
            lscHwConf->gb_data_tbl[i] = (uint16_t)((greenb + (65536 >> 1)) >> 16);
            lscHwConf->b_data_tbl[i]  = (uint16_t)((blue   + (65536 >> 1)) >> 16);
        }
        LOGD_ALSC( "dampfactor:%f", damp);
        LOGD_ALSC( " undampedLscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[3]
                 );
        LOGD_ALSC( " dampedLscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
                   lscHwConf->r_data_tbl[0],
                   lscHwConf->r_data_tbl[1],
                   lscHwConf->r_data_tbl[2],
                   lscHwConf->r_data_tbl[3],
                   lscHwConf->gr_data_tbl[0],
                   lscHwConf->gr_data_tbl[1],
                   lscHwConf->gr_data_tbl[2],
                   lscHwConf->gr_data_tbl[3],
                   lscHwConf->gb_data_tbl[0],
                   lscHwConf->gb_data_tbl[1],
                   lscHwConf->gb_data_tbl[2],
                   lscHwConf->gb_data_tbl[3],
                   lscHwConf->b_data_tbl[0],
                   lscHwConf->b_data_tbl[1],
                   lscHwConf->b_data_tbl[2],
                   lscHwConf->b_data_tbl[3]
                 );

        *converge = speedDamp;
        XCamReturn = XCAM_RETURN_NO_ERROR;
    }

    return (XCamReturn);
}

static XCamReturn DampingandOtp
(
    float     damp,               /**< damping coefficient */
    lsc_matrix_t*  pMatrixUndamped,   /**< undamped new computed matrices */
    lsc_matrix_t*  pMatrixDamped,      /**< old matrices and XCamReturn */
    alsc_otp_grad_t *otpGrad,
    rk_aiq_lsc_cfg_t *lscHwConf,
    bool* converge
) {
    XCamReturn XCamReturn = XCAM_RETURN_ERROR_PARAM;

    if ((pMatrixUndamped != NULL) && (pMatrixDamped != NULL)) {
        /* left shift 16 */
        uint32_t f1_ = (uint32_t)(damp * 65536.0f);
        uint32_t f2_ = (uint32_t)(65536U - f1_);

        int16_t i;

        uint32_t red;
        uint32_t greenr;
        uint32_t greenb;
        uint32_t blue;
        float dis=0;
        float dis_th=0.015;
        bool speedDamp = true;
        dis = (float)(pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]-
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0])/
            (pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]+
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]);
        speedDamp = fabs(dis)<dis_th;
        dis = (float)(pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1]-
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1])/
            (pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1]+
            pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[LSC_DATA_TBL_SIZE-1]);
        speedDamp &= fabs(dis)<dis_th;
        if(speedDamp){
            damp = 0;
            f1_ = 0;
            f2_ = 65536;
        }

        for (i = 0; i < (LSC_DATA_TBL_SIZE); i++) {
            red     = (f1_ * (uint32_t)pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i]);

            greenr  = (f1_ * (uint32_t)pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i]);

            greenb  = (f1_ * (uint32_t)pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i]);

            blue    = (f1_ * (uint32_t)pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i])
                    + (f2_ * (uint32_t)pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i]);

            /* with round up (add 65536/2 <=> 0.5) before right shift */
            pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[i]    = (uint16_t)((red    + (65536 >> 1)) >> 16);
            pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[i] = (uint16_t)((greenr + (65536 >> 1)) >> 16);
            pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[i] = (uint16_t)((greenb + (65536 >> 1)) >> 16);
            pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[i]   = (uint16_t)((blue   + (65536 >> 1)) >> 16);
        }

        genLscMatrixToHwConf(pMatrixDamped, lscHwConf, otpGrad);

        LOGD_ALSC( "dampfactor:%f", damp);
        LOGD_ALSC( " undampedLscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[3],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[1],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[2],
                   pMatrixUndamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[3]
                 );
        LOGD_ALSC( " dampedLscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[1],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[2],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[3],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[1],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[2],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[3],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[1],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[2],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[3],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[1],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[2],
                   pMatrixDamped->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[3]
                 );

        *converge = speedDamp;
        XCamReturn = XCAM_RETURN_NO_ERROR;
    }

    return (XCamReturn);
}

static XCamReturn CamCalibDbGetLscProfileByName
(
    const CalibDbV2_LSC_t *calib2_lsc,
    char* name,
    pLscTableProfile_t &pLscTableProfile
){
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;

    for(int i = 0; i < calib2_lsc->tbl.tableAll_len; i++) {
        if(strcmp(calib2_lsc->tbl.tableAll[i].name, name) == 0) {
            pLscTableProfile = &calib2_lsc->tbl.tableAll[i];
            ret = XCAM_RETURN_NO_ERROR;
            break;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGD_ALSC("%s: can't find %s in tableAll\n", __func__, name);
    }
    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);

    return ret;
}

static XCamReturn GetLscResIdxByName(alsc_illu_case_t* illu_case, char* name, uint32_t &resIdx)
{
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;

    for(uint32_t i = 0; i < illu_case->res_count; i++) {
        if(strcmp(illu_case->res_group[i].resolution.name, name) == 0) {
            resIdx = i;
            illu_case->current_res_idx = i;
            ret = XCAM_RETURN_NO_ERROR;
            break;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ALSC("can't find %s in lscResName\n", name);
    }
    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);

    return ret;
}

void UpdateDominateIlluList(struct list_head* l_head, int illu, int listMaxSize)
{
    illu_node_t* pCurNode, * pNode0;
    if (listMaxSize == 0) {
        return;
    }
    int sizeList = get_list_num(l_head);
    if (sizeList < listMaxSize) {
        pCurNode = (illu_node_t*)malloc(sizeof(illu_node_t));
        pCurNode->value = illu;
        list_prepare_item(&pCurNode->node);
        list_add_tail((struct list_head*)(&pCurNode->node), l_head);
    }
    else {
        // input list
        //     |-------------------------|
        //     head<->n0<->n1<->n2<->n3
        // output list
        //     |-------------------------|
        //     n0'<->head<->n1<->n2<->n3
        //     n0'->value = illu;
        pNode0 = (illu_node_t*)(l_head->next);
        pNode0->value = illu;
        struct list_head* nodeH = l_head;
        struct list_head* node0 = nodeH->next;
        list_swap_item(nodeH, node0);

    }
}

void StableIlluEstimation(struct list_head * head, int listSize, int illuNum, uint32_t& newIllu)
{
    int sizeList = get_list_num(head);
    if (sizeList < listSize || listSize == 0) {
        return;
    }

    struct list_head* pNextNode = head->next;
    illu_node_t* pL;
    int* illuSet = (int*)malloc(illuNum * sizeof(int));
    memset(illuSet, 0, illuNum * sizeof(int));
    while (head != pNextNode)
    {
        pL = (illu_node_t*)pNextNode;
        illuSet[pL->value]++;
        pNextNode = pNextNode->next;
    }
    int max_count = 0;
    for (int i = 0; i < illuNum; i++) {
        LOGV_ALSC("illu(%d), count(%d)\n", i, illuSet[i]);
        if (illuSet[i] > max_count) {
            max_count = illuSet[i];
            newIllu = i;
        }
    }
    free(illuSet);

}


static void ClearContext(alsc_handle_t hAlsc)
{
    clear_list(&hAlsc->alscRest.dominateIlluList);

    for (uint32_t mode_id = 0; mode_id < USED_FOR_CASE_MAX; mode_id++) {
        if (hAlsc->alsc_mode[mode_id].illu_case != nullptr)
            free(hAlsc->alsc_mode[mode_id].illu_case);
    }

    if (hAlsc->illu_case) {
        for (uint32_t id = 0; id < hAlsc->illu_case_count; id++)
        {
            alsc_illu_case_t& illu_case = hAlsc->illu_case[id];
            if (illu_case.res_group) {
                for (uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
                {
                    if (illu_case.res_group[res_id].lsc_table_group) {
                        free(illu_case.res_group[res_id].lsc_table_group);
                        illu_case.res_group[res_id].lsc_table_group = NULL;
                    } else {
                        LOGI_ALSC("%s: free: res_group[%d].lsc_table_group is already NULL!\n", __func__, res_id);
                    }
                }
                free(illu_case.res_group);
                illu_case.res_group = NULL;
            } else {
                LOGI_ALSC("%s: free: illu_case.res_group is already NULL!\n", __func__);
            }
        }
        free(hAlsc->illu_case);
        hAlsc->illu_case = NULL;
        hAlsc->illu_case_count = 0;
    } else {
        LOGI_ALSC("%s: free: hAlsc->illu_case is already NULL!\n", __func__);
    }

    if (hAlsc->res_grad) {
        free(hAlsc->res_grad);
        hAlsc->res_grad = NULL;
        hAlsc->res_grad_count = 0;
    } else {
        LOGI_ALSC("%s: free: hAlsc->res_grad is already NULL!\n", __func__);
    }

}

XCamReturn convertSensorLscOTP(resolution_t *cur_res, alsc_otp_grad_t *otpGrad,
                               RkAiqBayerPattern_t bayerPattern)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!cur_res || !otpGrad)
        return XCAM_RETURN_BYPASS;

    if (!otpGrad->flag)
        return XCAM_RETURN_BYPASS;

    if ((cur_res->width >= otpGrad->width && \
         cur_res->height >= otpGrad->height) || \
        (cur_res->width <= otpGrad->width && \
         cur_res->height <= otpGrad->height)) {
        convertLscTableParameter(cur_res, otpGrad, bayerPattern);
    }

    return ret;
}

XCamReturn alscGetOtpInfo(RkAiqAlgoCom* params)
{
    if (!params)
        return XCAM_RETURN_BYPASS;

    alsc_handle_t hAlsc = (alsc_handle_t)(params->ctx->alsc_para);
    RkAiqAlgoConfigAlsc *para = (RkAiqAlgoConfigAlsc *)params;

    alsc_sw_info_t *alscSwInfo = &para->alsc_sw_info;
    if (alscSwInfo->otpInfo.flag) {
        hAlsc->otpGrad.flag = alscSwInfo->otpInfo.flag;
        hAlsc->otpGrad.width = alscSwInfo->otpInfo.width;
        hAlsc->otpGrad.height = alscSwInfo->otpInfo.height;
        hAlsc->otpGrad.table_size = alscSwInfo->otpInfo.table_size;
        memcpy(hAlsc->otpGrad.lsc_r, alscSwInfo->otpInfo.lsc_r, sizeof(hAlsc->otpGrad.lsc_r));
        memcpy(hAlsc->otpGrad.lsc_gr, alscSwInfo->otpInfo.lsc_gr, sizeof(hAlsc->otpGrad.lsc_gr));
        memcpy(hAlsc->otpGrad.lsc_gb, alscSwInfo->otpInfo.lsc_gb, sizeof(hAlsc->otpGrad.lsc_gb));
        memcpy(hAlsc->otpGrad.lsc_b, alscSwInfo->otpInfo.lsc_b, sizeof(hAlsc->otpGrad.lsc_b));
    } else {
        hAlsc->alscSwInfo.otpInfo.flag = 0;
    }

    LOGD_ALSC("otp: flag %d, WxH: %dx%d, table_size: %d\n",
            hAlsc->otpGrad.flag,
            hAlsc->otpGrad.width,
            hAlsc->otpGrad.height,
            hAlsc->otpGrad.table_size);

    LOGD_ALSC( "otp LscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
            hAlsc->otpGrad.lsc_r[0],
            hAlsc->otpGrad.lsc_r[1],
            hAlsc->otpGrad.lsc_r[2],
            hAlsc->otpGrad.lsc_r[3],
            hAlsc->otpGrad.lsc_gr[0],
            hAlsc->otpGrad.lsc_gr[1],
            hAlsc->otpGrad.lsc_gr[2],
            hAlsc->otpGrad.lsc_gr[3],
            hAlsc->otpGrad.lsc_gb[0],
            hAlsc->otpGrad.lsc_gb[1],
            hAlsc->otpGrad.lsc_gb[2],
            hAlsc->otpGrad.lsc_gb[3],
            hAlsc->otpGrad.lsc_b[0],
            hAlsc->otpGrad.lsc_b[1],
            hAlsc->otpGrad.lsc_b[2],
            hAlsc->otpGrad.lsc_b[3]);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn UpdateLscCalibPtr(alsc_handle_t hAlsc) {
    const CalibDbV2_LSC_t* calib2 = hAlsc->calibLscV2;
    const CalibDbV2_AlscCof_ill_t* alsc_cof_tmp;
    // update hAlsc->illu_case
    for(uint32_t ill_id = 0; ill_id < hAlsc->illu_case_count; ill_id++)
    {
        alsc_illu_case_t& illu_case = hAlsc->illu_case[ill_id];
        alsc_cof_tmp = &(calib2->alscCoef.illAll[ill_id]);

        illu_case.alsc_cof = alsc_cof_tmp;
        illu_case.res_count = calib2->common.resolutionAll_len;

        for(uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
        {
            alsc_illu_case_resolution_t& illu_case_res = illu_case.res_group[res_id];
            strcpy(illu_case_res.resolution.name, calib2->common.resolutionAll[res_id].name);
            illu_case_res.lsc_table_count = alsc_cof_tmp->tableUsed_len;
            //TODO: illu_case.resolution.width & height
            for (int used_id = 0; used_id < alsc_cof_tmp->tableUsed_len; used_id++)
            {
                char profile_name[64];
                memset(profile_name, 0, sizeof(profile_name));
                sprintf(profile_name, "%s_%s", illu_case_res.resolution.name,
                    alsc_cof_tmp->tableUsed[used_id].name);
                XCamReturn ret = CamCalibDbGetLscProfileByName(calib2,
                    profile_name, illu_case_res.lsc_table_group[used_id]);
                if (XCAM_RETURN_NO_ERROR != ret) {
                    LOGE_ALSC("%s: CamCalibDbGetLscProfileByName failed\n", __func__);
                    return ret;
                }
            }
            OrderLscProfilesByVignetting(illu_case_res.lsc_table_group, illu_case_res.lsc_table_count);
        }
    }

    // update hAlsc->alsc_mode_data_t
    uint32_t update_count[USED_FOR_CASE_MAX];
    memset(update_count, 0, sizeof(update_count));
    for(uint32_t ill_id = 0; ill_id < hAlsc->illu_case_count; ill_id++)
    {
        uint32_t used_case = hAlsc->illu_case[ill_id].alsc_cof->usedForCase;
        if (used_case >= USED_FOR_CASE_MAX) {
            LOGE_ALSC("%s: used_case=%d\n", __func__, used_case);
            return XCAM_RETURN_ERROR_PARAM;
        }
        uint32_t update_id = update_count[used_case];
        if (update_id >= hAlsc->alsc_mode[used_case].illu_case_count) {
            LOGE_ALSC("%s: update_id=%d\n", __func__, update_id);
            return XCAM_RETURN_ERROR_PARAM;
        }
        hAlsc->alsc_mode[used_case].illu_case[update_id] = &(hAlsc->illu_case[ill_id]);
        update_count[used_case]++;
    }


    return XCAM_RETURN_NO_ERROR;
}

/** @brief called to arrange data to fill alsc_mode_data_t from CalibDb */
static XCamReturn UpdateLscCalibPara(alsc_handle_t hAlsc)
{
    const CalibDbV2_LSC_t* calib2 = hAlsc->calibLscV2;
    const CalibDbV2_AlscCof_ill_t* alsc_cof_tmp;
    uint32_t mode_count[USED_FOR_CASE_MAX];
    memset(mode_count, 0, sizeof(mode_count));

    hAlsc->illu_case_count = calib2->alscCoef.illAll_len;
    hAlsc->illu_case = (alsc_illu_case_t*)malloc(hAlsc->illu_case_count * sizeof(alsc_illu_case_t));
    memset(hAlsc->illu_case, 0, hAlsc->illu_case_count * sizeof(alsc_illu_case_t));

    // update hAlsc->illu_case
    for(uint32_t ill_id = 0; ill_id < hAlsc->illu_case_count; ill_id++)
    {
        alsc_illu_case_t& illu_case = hAlsc->illu_case[ill_id];
        alsc_cof_tmp = &(calib2->alscCoef.illAll[ill_id]);

        illu_case.alsc_cof = alsc_cof_tmp;
        illu_case.res_count = calib2->common.resolutionAll_len;
        illu_case.res_group = (alsc_illu_case_resolution_t*)malloc
            (illu_case.res_count * sizeof(alsc_illu_case_resolution_t));
        memset(illu_case.res_group, 0, illu_case.res_count * sizeof(alsc_illu_case_resolution_t));

        for(uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
        {
            alsc_illu_case_resolution_t& illu_case_res = illu_case.res_group[res_id];
            strcpy(illu_case_res.resolution.name, calib2->common.resolutionAll[res_id].name);
            //TODO: illu_case.resolution.width & height
            illu_case_res.lsc_table_count = alsc_cof_tmp->tableUsed_len;
            illu_case_res.lsc_table_group = (pLscTableProfile_t*)malloc(
                illu_case_res.lsc_table_count * sizeof(pLscTableProfile_t));

            for (int used_id = 0; used_id < alsc_cof_tmp->tableUsed_len; used_id++)
            {
                char profile_name[64];
                memset(profile_name, 0, sizeof(profile_name));
                sprintf(profile_name, "%s_%s", illu_case_res.resolution.name,
                    alsc_cof_tmp->tableUsed[used_id].name);
                XCamReturn ret = CamCalibDbGetLscProfileByName(calib2,
                    profile_name, illu_case_res.lsc_table_group[used_id]);
                if (XCAM_RETURN_NO_ERROR != ret) {
                    LOGE_ALSC("%s: CamCalibDbGetLscProfileByName failed\n", __func__);
                    return ret;
                }
            }
            OrderLscProfilesByVignetting(illu_case_res.lsc_table_group, illu_case_res.lsc_table_count);
        }

        mode_count[alsc_cof_tmp->usedForCase]++;
    }

    // malloc hAlsc->alsc_mode_data_t
    for (uint32_t mode_id = 0; mode_id < USED_FOR_CASE_MAX; mode_id++)
    {
        hAlsc->alsc_mode[mode_id].illu_case_count = mode_count[mode_id];
        if (mode_count[mode_id] == 0) {
            hAlsc->alsc_mode[mode_id].illu_case = NULL;
        } else {
            hAlsc->alsc_mode[mode_id].illu_case = (pIlluCase_t*)malloc(
                mode_count[mode_id] * sizeof(pIlluCase_t));
        }
    }

    // update hAlsc->alsc_mode_data_t
    uint32_t update_count[USED_FOR_CASE_MAX];
    memset(update_count, 0, sizeof(update_count));
    for(uint32_t ill_id = 0; ill_id < hAlsc->illu_case_count; ill_id++)
    {
        uint32_t used_case = hAlsc->illu_case[ill_id].alsc_cof->usedForCase;
        if (used_case >= USED_FOR_CASE_MAX) {
            LOGE_ALSC("%s: used_case=%d\n", __func__, used_case);
            return XCAM_RETURN_ERROR_PARAM;
        }
        uint32_t update_id = update_count[used_case];
        if (update_id >= hAlsc->alsc_mode[used_case].illu_case_count) {
            LOGE_ALSC("%s: update_id=%d\n", __func__, update_id);
            return XCAM_RETURN_ERROR_PARAM;
        }
        hAlsc->alsc_mode[used_case].illu_case[update_id] = &(hAlsc->illu_case[ill_id]);
        update_count[used_case]++;
    }

    // molloc & calculate the grad table
    hAlsc->res_grad_count = calib2->common.resolutionAll_len;
    hAlsc->res_grad = (alsc_grad_t*)malloc(hAlsc->res_grad_count * sizeof(alsc_grad_t));
    memset(hAlsc->res_grad, 0, hAlsc->res_grad_count * sizeof(alsc_grad_t));
    for (uint32_t res_id = 0; res_id < hAlsc->res_grad_count; res_id++) {
        CalibDbV2_Lsc_Resolution_t& src = calib2->common.resolutionAll[res_id];
        alsc_grad_t& dst = hAlsc->res_grad[res_id];
        strcpy(dst.resolution.name, src.name);
        uint32_t x_size = sizeof(src.lsc_sect_size_x) / sizeof(src.lsc_sect_size_x[0]);
        uint32_t y_size = sizeof(src.lsc_sect_size_y) / sizeof(src.lsc_sect_size_y[0]);
        for (uint32_t i = 0; i < x_size; i++) {
            if (0 < src.lsc_sect_size_x[i]) {
                dst.LscXGradTbl[i] = (uint16_t)((double)(1UL << 15) / src.lsc_sect_size_x[i] + 0.5);
            } else {
                LOGE_ALSC("%s: src.lsc_sect_size_x[%d]=0\n", __func__, i);
                return XCAM_RETURN_ERROR_PARAM;
            }
        }
        for (uint32_t i = 0; i < y_size; i++) {
            if (0 < src.lsc_sect_size_y[i]) {
                dst.LscYGradTbl[i] = (uint16_t)((double)(1UL << 15) / src.lsc_sect_size_y[i] + 0.5);
            } else {
                LOGE_ALSC("%s: src.lsc_sect_size_y[%d]=0\n", __func__, i);
                return XCAM_RETURN_ERROR_PARAM;
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

#if ARCHER_DEBUG
static void print_alsc_illu_case(alsc_illu_case_t& illu_case) {
    const CalibDbV2_AlscCof_ill_t *alsc_cof = illu_case.alsc_cof;
    char tmp[256] = {0};
    COUT(alsc_cof->usedForCase);
    COUT(alsc_cof->name);
    COUT(alsc_cof->wbGain[0]);
    COUT(alsc_cof->wbGain[1]);
    for (int i = 0; i < alsc_cof->tableUsed_len; i++) {
        COUT(i);
        COUT(alsc_cof->tableUsed[i].name);
    }
    array2str(alsc_cof->gains, alsc_cof->gains_len, tmp);
    COUT2(alsc_cof->gains, tmp);
    memset(tmp, 0, sizeof(tmp));
    array2str(alsc_cof->vig, alsc_cof->vig_len, tmp);
    COUT2(alsc_cof->vig, tmp);
    ENDL;

    for (int i = 0; i < illu_case.res_count; i++) {
        alsc_illu_case_resolution_t& res = illu_case.res_group[i];
        COUT(res.resolution.name);
        COUT(res.lsc_table_count);
        for (int k = 0; k < res.lsc_table_count; k++) {
            COUT(k);
            COUT(res.lsc_table_group[k]->name);
            COUT(res.lsc_table_group[k]->resolution);
            COUT(res.lsc_table_group[k]->illumination);
            COUT(res.lsc_table_group[k]->vignetting);
        }
    }
    COUT(illu_case.current_res_idx);

    ENDL;
    ENDL;
}

static void print_alsc(alsc_handle_t hAlsc)
{
    for (int case_id = 0; case_id < USED_FOR_CASE_MAX; case_id++)
    {
        alsc_mode_data_t& mode = hAlsc->alsc_mode[case_id];
        if (mode.illu_case_count == 0) {
            continue;
        }
        for (uint32_t ill_id = 0; ill_id < mode.illu_case_count; ill_id++)
        {
            alsc_illu_case_t& illu_case = mode.illu_case[ill_id];
            print_alsc_illu_case(illu_case);
        }
    }
    COUT(hAlsc->calibLscV2->common.enable);
    COUT(hAlsc->calibLscV2->common.resolutionAll_len);
    COUT(hAlsc->calibLscV2->alscCoef.damp_enable);
    COUT(hAlsc->calibLscV2->alscCoef.illAll_len);
    COUT(hAlsc->calibLscV2->tableAll_len);
}
#endif

XCamReturn AlscAutoConfig(alsc_handle_t hAlsc)
{
    if (hAlsc == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ALSC("%s: (enter) count:%d\n", __FUNCTION__, hAlsc->count);

    uint32_t caseIndex = hAlsc->alscRest.caseIndex;
    if (caseIndex >= USED_FOR_CASE_MAX) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    alsc_mode_data_t& alsc_mode_now = hAlsc->alsc_mode[caseIndex];
    if (alsc_mode_now.illu_case == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    //1) Estimate best(nearest) illuminant case;
    uint32_t estimateIlluCaseIdx;
    int dominateIlluListSize = 15;//to do from xml;
    //float varianceLumaTh = 0.006;//to do from xml;
    ret = illuminant_index_estimation(alsc_mode_now, hAlsc->alscSwInfo.awbGain, estimateIlluCaseIdx);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    UpdateDominateIlluList(&hAlsc->alscRest.dominateIlluList, estimateIlluCaseIdx, dominateIlluListSize);
    //TODO: working mode which has only one illuminant case, like gray mode, does not need to estimate index.
    StableIlluEstimation(&hAlsc->alscRest.dominateIlluList, dominateIlluListSize,
        alsc_mode_now.illu_case_count, estimateIlluCaseIdx);

    alsc_illu_case_t* illu_case = alsc_mode_now.illu_case[estimateIlluCaseIdx];

    // 2) get resolution index;
    uint32_t resIdx;
    ret = GetLscResIdxByName(illu_case, hAlsc->cur_res.name, resIdx);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    // 3) calculate vignetting from sensor gain;
    float sensorGain = hAlsc->alscSwInfo.sensorGain;
    float fVignetting;
    interpolation(illu_case->alsc_cof->gains,
                  illu_case->alsc_cof->vig,
                  illu_case->alsc_cof->vig_len,
                  sensorGain, &fVignetting);
    /* *************
    * if (fVignetting/estimateIlluCaseIdx/resIdx changed || forceRunFlag)
    *   Select LscProfiles by vig
    *   if (forceRunFlag || pLscProfile1/2 changed)
    *       Interpolate tbl by vig
    *       update hAlsc->alscRest.pLscProfile1/2
    * **************/
    bool flag0 = fabs(hAlsc->alscRest.fVignetting - fVignetting) > DIVMIN ||
                        hAlsc->alscRest.estimateIlluCaseIdx != estimateIlluCaseIdx ||
                        hAlsc->alscRest.resIdx != resIdx ||
                        hAlsc->smartRunRes.forceRunFlag; //forceRunFlag: prepare(init or calib/res/graymode change) or updateAttr
    LOGD_ALSC("pickLscProfile: %d = fVignetting(%f->%f)/estimateIlluCaseIdx(%d->%d)/resIdx(%d->%d) changed or forceRunFlag",
                flag0, hAlsc->alscRest.fVignetting, fVignetting, hAlsc->alscRest.estimateIlluCaseIdx, estimateIlluCaseIdx,
                hAlsc->alscRest.resIdx, resIdx, hAlsc->smartRunRes.forceRunFlag);
    if (flag0) {
        hAlsc->isReCal_ = hAlsc->smartRunRes.forceRunFlag ||
                            fabs(hAlsc->alscRest.fVignetting - fVignetting) > DIVMIN;
        hAlsc->alscRest.estimateIlluCaseIdx = estimateIlluCaseIdx;
        hAlsc->alscRest.resIdx = resIdx;
        hAlsc->alscRest.fVignetting =  fVignetting;

        // 4) select vignetting section and get the lsc matrix table;
        pLscTableProfile_t pLscProfile1 = NULL;
        pLscTableProfile_t pLscProfile2 = NULL;
        ret = VignSelectLscProfiles(illu_case, fVignetting, pLscProfile1, pLscProfile2);
        if (pLscProfile1 && pLscProfile2) {
            hAlsc->isReCal_ = hAlsc->isReCal_ ||
                            strcmp(pLscProfile1->name, hAlsc->alscRest.LscProName1) ||
                            strcmp(pLscProfile2->name, hAlsc->alscRest.LscProName2);
            LOGD_ALSC("hAlsc->isReCal_: %d = forceRunFlag(%d) || fVignetting changed || pLscProfile1/2 changed", hAlsc->isReCal_, hAlsc->smartRunRes.forceRunFlag);
        } else {
            LOGE_ALSC("check %s %s pLscProfile: %p %p \n", hAlsc->cur_res.name, illu_case->alsc_cof->name, pLscProfile1, pLscProfile2);
            return XCAM_RETURN_ERROR_PARAM;
        }
        if (hAlsc->isReCal_) {
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD_ALSC("fVignetting: %f (%f .. %f)\n",  fVignetting, pLscProfile1->vignetting, pLscProfile2->vignetting);

                ret = VignInterpolateMatrices(fVignetting, pLscProfile1, pLscProfile2,
                                            &hAlsc->alscRest.undampedLscMatrixTable);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    return (ret);
                }
            } else if (ret == XCAM_RETURN_ERROR_OUTOFRANGE) {
                /* we don't need to interpolate */
                LOGD_ALSC("fVignetting: %f (%f)\n",  fVignetting, pLscProfile1->vignetting);
                memcpy(&hAlsc->alscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED],
                    &pLscProfile1->lsc_samples_red, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&hAlsc->alscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR],
                    &pLscProfile1->lsc_samples_greenR, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&hAlsc->alscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB],
                    &pLscProfile1->lsc_samples_greenB, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&hAlsc->alscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE],
                    &pLscProfile1->lsc_samples_blue, sizeof(Cam17x17UShortMatrix_t));
            }
            strcpy(hAlsc->alscRest.LscProName1, pLscProfile1->name);
            strcpy(hAlsc->alscRest.LscProName2, pLscProfile2->name);
        }
    }

    // 5) . Damping
     /* if (isReCal_ || tblConverge==0) update hwconf
     *  else keep last hwconf */

    float dampCoef = (hAlsc->calibLscV2->alscCoef.damp_enable && hAlsc->count > 1) ? hAlsc->alscSwInfo.awbIIRDampCoef : 0;
    if (!hAlsc->isReCal_ && hAlsc->smartRunRes.lscTableConverge) {
        hAlsc->isReCal_ = false;
    } else {
        if (hAlsc->otpGrad.flag && hAlsc->otpGrad.table_size > 0){
            ret = DampingandOtp(dampCoef, &hAlsc->alscRest.undampedLscMatrixTable,
                    &hAlsc->alscRest.dampedLscMatrixTable,
                    &hAlsc->otpGrad,
                    &hAlsc->lscHwConf,
                    &hAlsc->smartRunRes.lscTableConverge);
        } else {
            ret = Damping(dampCoef, &hAlsc->alscRest.undampedLscMatrixTable,
                    &hAlsc->lscHwConf,
                    &hAlsc->smartRunRes.lscTableConverge);
        }
        hAlsc->isReCal_ = true;
    }

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AlscManualConfig
(
    alsc_handle_t hAlsc
) {
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAlsc->lscHwConf.x_size_tbl, hAlsc->mCurAtt.stManual.lsc_sect_size_x,
           sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_x));
    memcpy(hAlsc->lscHwConf.y_size_tbl, hAlsc->mCurAtt.stManual.lsc_sect_size_y,
           sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_y));
    memcpy(hAlsc->lscHwConf.r_data_tbl, hAlsc->mCurAtt.stManual.r_data_tbl,
           sizeof(hAlsc->mCurAtt.stManual.r_data_tbl));
    memcpy(hAlsc->lscHwConf.gr_data_tbl, hAlsc->mCurAtt.stManual.gr_data_tbl,
           sizeof(hAlsc->mCurAtt.stManual.gr_data_tbl));
    memcpy(hAlsc->lscHwConf.gb_data_tbl, hAlsc->mCurAtt.stManual.gb_data_tbl,
           sizeof(hAlsc->mCurAtt.stManual.gb_data_tbl));
    memcpy(hAlsc->lscHwConf.b_data_tbl, hAlsc->mCurAtt.stManual.b_data_tbl,
           sizeof(hAlsc->mCurAtt.stManual.b_data_tbl));

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn AlscManualGrad(alsc_handle_t hAlsc) {
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    uint32_t x_size = sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_x) /
                      sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_x[0]);
    uint32_t y_size = sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_y) /
                      sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_y[0]);

    for (uint32_t i = 0; i < x_size; i++) {
        if (0 < hAlsc->mCurAtt.stManual.lsc_sect_size_x[i]) {
            hAlsc->lscHwConf.x_grad_tbl[i] =
                (uint16_t)((double)(1UL << 15) / hAlsc->mCurAtt.stManual.lsc_sect_size_x[i] + 0.5);
        } else {
            LOGE_ALSC("%s: src.lsc_sect_size_x[%d]=0\n", __func__, i);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }
    for (uint32_t i = 0; i < y_size; i++) {
        if (0 < hAlsc->mCurAtt.stManual.lsc_sect_size_y[i]) {
            hAlsc->lscHwConf.y_grad_tbl[i] =
                (uint16_t)((double)(1UL << 15) / hAlsc->mCurAtt.stManual.lsc_sect_size_y[i] + 0.5);
        } else {
            LOGE_ALSC("%s: src.lsc_sect_size_y[%d]=0\n", __func__, i);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;
}


void JudgeAutoRun
(
    alsc_handle_t hAlsc

) {
    bool gainStable;
    bool wbgainStable;
    float wb_th = hAlsc->smartRunCfg.wbgain_th*hAlsc->smartRunCfg.wbgain_th;
    if (fabs(hAlsc->alscSwInfo.sensorGain - hAlsc->smartRunRes.last_gain) > hAlsc->smartRunCfg.gain_th) {
        hAlsc->smartRunRes.last_gain = hAlsc->alscSwInfo.sensorGain;
        gainStable = false;
    } else {
        gainStable = true;
    }

    if ((hAlsc->smartRunRes.last_awbGain[0]-hAlsc->alscSwInfo.awbGain[0])*(hAlsc->smartRunRes.last_awbGain[0]-hAlsc->alscSwInfo.awbGain[0])
         + (hAlsc->smartRunRes.last_awbGain[1]-hAlsc->alscSwInfo.awbGain[1])*(hAlsc->smartRunRes.last_awbGain[1]-hAlsc->alscSwInfo.awbGain[1]) > wb_th) {
        hAlsc->smartRunRes.last_awbGain[0] = hAlsc->alscSwInfo.awbGain[0];
        hAlsc->smartRunRes.last_awbGain[1] = hAlsc->alscSwInfo.awbGain[1];
        wbgainStable = false;
        LOGD_ALSC("update wbgain: %f, %f\n", hAlsc->alscSwInfo.awbGain[0], hAlsc->alscSwInfo.awbGain[1]);
    } else {
        wbgainStable = true;
        hAlsc->alscSwInfo.awbGain[0] = hAlsc->smartRunRes.last_awbGain[0];
        hAlsc->alscSwInfo.awbGain[1] = hAlsc->smartRunRes.last_awbGain[1];
    }

    hAlsc->smartRunRes.res3aChg = !(wbgainStable && gainStable);
    LOGD_ALSC("awbStable(%d),aeStable(%d),lscTableConverge(%d),forceRunFlag(%d)",
        wbgainStable,gainStable,hAlsc->smartRunRes.lscTableConverge,hAlsc->smartRunRes.forceRunFlag);

/*     bool notRunflag = wbgainStable && gainStable
        && hAlsc->smartRunRes.lscTableConverge
        && !(hAlsc->smartRunRes.forceRunFlag);
    hAlsc->smartRunRes.samrtRunFlag = !notRunflag;
    LOGD_ALSC("smartRun(%d),awbStable(%d),aeStable(%d),lscTableConverge(%d),forceRunFlag(%d)",hAlsc->smartRunRes.samrtRunFlag,
        wbgainStable,gainStable,hAlsc->smartRunRes.lscTableConverge,hAlsc->smartRunRes.forceRunFlag);
    hAlsc->smartRunRes.forceRunFlag =false; */
}


XCamReturn AlscConfig
(
    alsc_handle_t hAlsc
) {
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGD_ALSC("%s: updateAtt: %d\n", __FUNCTION__, hAlsc->updateAtt);
    hAlsc->alscRest.caseIndex = USED_FOR_CASE_NORMAL;
    if((hAlsc->alscSwInfo.grayMode == true && hAlsc->alscRest.caseIndex != USED_FOR_CASE_GRAY)||
        (hAlsc->alscSwInfo.grayMode == false && hAlsc->alscRest.caseIndex == USED_FOR_CASE_GRAY)){
        hAlsc->smartRunRes.forceRunFlag = true;
        clear_list(&hAlsc->alscRest.dominateIlluList);
    }
    if(hAlsc->alscSwInfo.grayMode){
        hAlsc->alscRest.caseIndex = USED_FOR_CASE_GRAY;
    }
    if(hAlsc->updateAtt) {
        //hAlsc->mCurAtt = hAlsc->mNewAtt;
        if (hAlsc->mCurAtt.mode == RK_AIQ_LSC_MODE_AUTO) {
            AlscAutoParamFree(hAlsc);
            AlscAutoParamClone(hAlsc);
            AlscPrepare(hAlsc);
        } else if (hAlsc->mCurAtt.mode == RK_AIQ_LSC_MODE_MANUAL)
        {
            AlscManualGrad(hAlsc);
        } else {
            LOGE_ALSC("%s: hAlsc->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAlsc->mCurAtt.mode);
        }
    }
    LOGD_ALSC("%s: byPass: %d  mode:%d used for case: %d\n", __FUNCTION__,
        hAlsc->mCurAtt.byPass, hAlsc->mCurAtt.mode,hAlsc->alscRest.caseIndex);
    if(hAlsc->mCurAtt.byPass != true) {
        hAlsc->lscHwConf.lsc_en = true;
        if(hAlsc->mCurAtt.mode == RK_AIQ_LSC_MODE_AUTO) {
            JudgeAutoRun(hAlsc);
            if (hAlsc->smartRunRes.res3aChg || !hAlsc->smartRunRes.lscTableConverge
                    || hAlsc->smartRunRes.forceRunFlag) {
                AlscAutoConfig(hAlsc);
            }
        } else if(hAlsc->mCurAtt.mode == RK_AIQ_LSC_MODE_MANUAL && hAlsc->updateAtt) {
            AlscManualConfig(hAlsc);
            hAlsc->isReCal_ = true;
        } else {
            LOGE_ALSC("%s: hAlsc->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAlsc->mCurAtt.mode);
        }
#if 0
        memcpy(hAlsc->mCurAtt.stManual.lsc_sect_size_x, hAlsc->lscHwConf.x_size_tbl,
               sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_x));
        memcpy(hAlsc->mCurAtt.stManual.lsc_sect_size_y, hAlsc->lscHwConf.y_size_tbl,
               sizeof(hAlsc->mCurAtt.stManual.lsc_sect_size_y));
        memcpy(hAlsc->mCurAtt.stManual.r_data_tbl, hAlsc->lscHwConf.r_data_tbl,
               sizeof(hAlsc->mCurAtt.stManual.r_data_tbl));
        memcpy(hAlsc->mCurAtt.stManual.gr_data_tbl, hAlsc->lscHwConf.gr_data_tbl,
               sizeof(hAlsc->mCurAtt.stManual.gr_data_tbl));
        memcpy(hAlsc->mCurAtt.stManual.gb_data_tbl, hAlsc->lscHwConf.gb_data_tbl,
               sizeof(hAlsc->mCurAtt.stManual.gb_data_tbl));
        memcpy(hAlsc->mCurAtt.stManual.b_data_tbl, hAlsc->lscHwConf.b_data_tbl,
               sizeof(hAlsc->mCurAtt.stManual.b_data_tbl));
#endif
    } else {
        hAlsc->lscHwConf.lsc_en = false;
        // if api/calib set bypass=true, then hAlsc->isReCal_ = true
        hAlsc->isReCal_ = hAlsc->updateAtt || hAlsc->smartRunRes.forceRunFlag;
    }
    LOGD_ALSC("Final hAlsc->isReCal_: %d, lscTableConverge: %d\n", hAlsc->isReCal_, hAlsc->smartRunRes.lscTableConverge);
    hAlsc->updateAtt = false;
    hAlsc->smartRunRes.forceRunFlag = false;
    hAlsc->count = ((hAlsc->count + 2) > (65536)) ? 2 : (hAlsc->count + 1);
    LOGD_ALSC( "set to ic LscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
               hAlsc->lscHwConf.r_data_tbl[0],
               hAlsc->lscHwConf.r_data_tbl[1],
               hAlsc->lscHwConf.r_data_tbl[2],
               hAlsc->lscHwConf.r_data_tbl[3],
               hAlsc->lscHwConf.gr_data_tbl[0],
               hAlsc->lscHwConf.gr_data_tbl[1],
               hAlsc->lscHwConf.gr_data_tbl[2],
               hAlsc->lscHwConf.gr_data_tbl[3],
               hAlsc->lscHwConf.gb_data_tbl[0],
               hAlsc->lscHwConf.gb_data_tbl[1],
               hAlsc->lscHwConf.gb_data_tbl[2],
               hAlsc->lscHwConf.gb_data_tbl[3],
               hAlsc->lscHwConf.b_data_tbl[0],
               hAlsc->lscHwConf.b_data_tbl[1],
               hAlsc->lscHwConf.b_data_tbl[2],
               hAlsc->lscHwConf.b_data_tbl[3]
             );

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn AlscInit(alsc_handle_t *hAlsc, const CamCalibDbV2Context_t* calib2)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    *hAlsc = (alsc_context_t*)malloc(sizeof(alsc_context_t));
    //*hAlsc = new alsc_context_t(); //TODO: use this
    alsc_context_t* alsc_context = *hAlsc;
    memset(alsc_context, 0, sizeof(alsc_context_t));

    if(calib2 == NULL) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    const CalibDbV2_LSC_t *calib2_lsc =
        (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calib2, lsc_v2));
    alsc_context->calibLscV2 = calib2_lsc;

    alsc_context->alscSwInfo.sensorGain = 1.0;
    alsc_context->alscSwInfo.awbIIRDampCoef = 0;
    alsc_context->alscSwInfo.grayMode = false;
    alsc_context->alscSwInfo.varianceLuma = 255;
    alsc_context->alscSwInfo.awbConverged = false;

    alsc_context->alscRest.caseIndex = USED_FOR_CASE_NORMAL;
    alsc_context->updateAtt = false;
    alsc_context->mCurAtt.byPass = !calib2_lsc->common.enable;
    alsc_context->count = 0;
    alsc_context->mCurAtt.mode = RK_AIQ_LSC_MODE_AUTO;
    alsc_context->smartRunCfg.enable = true;
    alsc_context->smartRunCfg.wbgain_th= 0.05;
    alsc_context->smartRunCfg.gain_th= 0.2;
    alsc_context->smartRunRes.last_gain = -1;
    alsc_context->smartRunRes.last_awbGain[0]= -1;
    alsc_context->smartRunRes.last_awbGain[1]= -1;
    alsc_context->smartRunRes.forceRunFlag = true;
    //alsc_context->alscSwInfo.prepare_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;
    //ret = UpdateLscCalibPara(alsc_context);
    //print_alsc(alsc_context);
    memset(&alsc_context->otpGrad, 0, sizeof(alsc_context->otpGrad));
    INIT_LIST_HEAD(&alsc_context->alscRest.dominateIlluList);
    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);
}

XCamReturn AlscRelease(alsc_handle_t hAlsc)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ClearContext(hAlsc);

    if (hAlsc) {
        free(hAlsc);
        hAlsc = NULL;
    } else {
        LOGI_ALSC("%s: free: hAlsc is already NULL!\n", __func__);
    }

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);

}

XCamReturn AlscPrepare(alsc_handle_t hAlsc)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ClearContext(hAlsc);
    ret = UpdateLscCalibPara(hAlsc);
    if (XCAM_RETURN_NO_ERROR != ret) {
        LOGE_ALSC("%s: UpdateLscCalibPara failed\n", __FUNCTION__);
        return ret;
    }

    hAlsc->mCurAtt.byPass = !hAlsc->calibLscV2->common.enable;
    hAlsc->smartRunRes.forceRunFlag = true;
    const CalibDbV2_Lsc_Common_t& lsc_com = hAlsc->calibLscV2->common;
    const CalibDbV2_Lsc_Resolution_t* calib_lsc_res = nullptr;
    for(int i = 0; i < lsc_com.resolutionAll_len; i++) {
        if (strcmp(lsc_com.resolutionAll[i].name, hAlsc->cur_res.name) == 0) {
            calib_lsc_res = &lsc_com.resolutionAll[i];
            break;
        }
    }
    if (calib_lsc_res != nullptr) {
        memcpy(hAlsc->lscHwConf.x_size_tbl, calib_lsc_res->lsc_sect_size_x, sizeof(calib_lsc_res->lsc_sect_size_x));
        memcpy(hAlsc->lscHwConf.y_size_tbl, calib_lsc_res->lsc_sect_size_y, sizeof(calib_lsc_res->lsc_sect_size_y));
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
    }

    const alsc_grad_t* cur_grad = NULL;
    for (uint32_t i = 0; i < hAlsc->res_grad_count; i++) {
        if (0 == strcmp(hAlsc->res_grad[i].resolution.name, hAlsc->cur_res.name)) {
            cur_grad = &(hAlsc->res_grad[i]);
            break;
        }
    }
    if (cur_grad != NULL) {
        memcpy(hAlsc->lscHwConf.x_grad_tbl, cur_grad->LscXGradTbl, sizeof(cur_grad->LscXGradTbl));
        memcpy(hAlsc->lscHwConf.y_grad_tbl, cur_grad->LscYGradTbl, sizeof(cur_grad->LscYGradTbl));
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
    }

#ifdef ARCHER_DEBUG
    if (cur_grad) {
        LOGE_ALSC( "%s\n", PRT_ARRAY(cur_grad->LscXGradTbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(cur_grad->LscYGradTbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(hAlsc->lscHwConf.x_grad_tbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(hAlsc->lscHwConf.y_grad_tbl) );
    }
#endif

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AlscAutoParamClone(alsc_handle_t hAlsc)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    hAlsc->fixed_calib.common.enable            = hAlsc->mCurAtt.stAuto.common.enable;
    hAlsc->fixed_calib.common.resolutionAll     = (CalibDbV2_Lsc_Resolution_t*)hAlsc->mCurAtt.stAuto.common.resolutionAll;
    hAlsc->fixed_calib.common.resolutionAll_len = hAlsc->mCurAtt.stAuto.common.resolutionAll_len;

    hAlsc->fixed_calib.alscCoef.damp_enable = hAlsc->mCurAtt.stAuto.alscCoef.damp_enable;
    hAlsc->fixed_calib.alscCoef.illAll_len  = hAlsc->mCurAtt.stAuto.alscCoef.illAll_len;
    if (!hAlsc->fixed_calib.alscCoef.illAll_len) {
      return XCAM_RETURN_ERROR_PARAM;
    }
    if (hAlsc->fixed_calib.alscCoef.illAll == NULL)
        hAlsc->fixed_calib.alscCoef.illAll      = (CalibDbV2_AlscCof_ill_t*)malloc(
            sizeof(CalibDbV2_AlscCof_ill_t) * hAlsc->fixed_calib.alscCoef.illAll_len);

    for (int i = 0; i < hAlsc->fixed_calib.alscCoef.illAll_len; i++) {
        hAlsc->fixed_calib.alscCoef.illAll[i].usedForCase =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].usedForCase;

        memcpy(hAlsc->fixed_calib.alscCoef.illAll[i].name,
               hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].name,
               sizeof(hAlsc->fixed_calib.alscCoef.illAll[i].name));

        memcpy(hAlsc->fixed_calib.alscCoef.illAll[i].wbGain,
               hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].wbGain,
               sizeof(hAlsc->fixed_calib.alscCoef.illAll[i].wbGain));

        hAlsc->fixed_calib.alscCoef.illAll[i].tableUsed =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].tableUsed;
        hAlsc->fixed_calib.alscCoef.illAll[i].tableUsed_len =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].tableUsed_len;
        hAlsc->fixed_calib.alscCoef.illAll[i].gains =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].gains;
        hAlsc->fixed_calib.alscCoef.illAll[i].gains_len =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].gains_len;
        hAlsc->fixed_calib.alscCoef.illAll[i].vig = hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].vig;
        hAlsc->fixed_calib.alscCoef.illAll[i].vig_len =
            hAlsc->mCurAtt.stAuto.alscCoef.illAll[i].vig_len;
    }

    hAlsc->fixed_calib.tbl.tableAll = hAlsc->mCurAtt.stAuto.tbl.tableAll;
    hAlsc->fixed_calib.tbl.tableAll_len = hAlsc->mCurAtt.stAuto.tbl.tableAll_len;

    hAlsc->calibLscV2 = &hAlsc->fixed_calib;

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AlscAutoParamFree(alsc_handle_t hAlsc)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hAlsc->fixed_calib.alscCoef.illAll &&
            (hAlsc->mCurAtt.stAuto.tbl.tableAll_len !=
            hAlsc->fixed_calib.alscCoef.illAll_len)) {
        free(hAlsc->fixed_calib.alscCoef.illAll);
        hAlsc->fixed_calib.alscCoef.illAll = NULL;
    }

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AlscPreProc(alsc_handle_t hAlsc)
{

    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);

}
XCamReturn AlscProcessing(alsc_handle_t hAlsc)
{
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if _TEST_LSC_VALID_ON_HARDWARE
    _test_if_hw_lsc_valid(hAlsc);
#endif

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);
}




RKAIQ_END_DECLARE



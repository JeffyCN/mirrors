/*
* rk_aiq_lsc_algo.cpp

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
#include "lsc_common.h"
#include "xcam_log.h"
#include "interpolation.h"
#include "RkAiqHandle.h"
#include "RkAiqCalibDbV2Helper.h"
#include "lsc_convert_otp.h"

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
static void _test_if_hw_lsc_valid(LscContext_t* pLscCtx)
{
#define ONE_TIME_STATE      1
#define THREE_TIME_STATE    3

    static int cur_state = ONE_TIME_STATE;
    int next_state = cur_state;

    switch (cur_state)
    {
        case ONE_TIME_STATE: {
            next_state = THREE_TIME_STATE;
            _memset_lsc(pLscCtx->lscHwConf, 1024);
        } break;

        case THREE_TIME_STATE: {
            next_state = ONE_TIME_STATE;
            _memset_lsc(pLscCtx->lscHwConf, 3072);
        } break;

        default:
            break;
    }
    cur_state = next_state;
}
#endif

static void genLscMatrixToHwConf(lsc_matrix_t *dampedLscMatrixTable, Lsc_Hwconf_t *lscHwConf, lsc_otp_grad_t *otpGrad)
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

XCamReturn illuminant_index_estimation(lsc_mode_data_t& lsc_mode_data, float awbGain[2], uint32_t& illu_case_id)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    float minDist = 9999999;
    float dist;
    float& nRG = awbGain[0]; //current R/G gain
    float& nBG = awbGain[1]; //current B/G gain

    for(uint32_t index = 0; index < lsc_mode_data.illu_case_count; index++)
    {
        pIlluCase_t illu_case = lsc_mode_data.illu_case[index];
        float refRG = illu_case->lsc_cof->wbGain[0];
        float refBG = illu_case->lsc_cof->wbGain[1];
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
               lsc_mode_data.illu_case[illu_case_id]->lsc_cof->name, illu_case_id);
    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return ret;
}

static XCamReturn OrderLscProfilesByVignetting
(
    lscTableProfile_t* pLscProfiles,
    const uint32_t      cnt
) {
    uint32_t i, j;

    for (i = 0; i < (cnt - 1); ++i) {
        for (j = 0; j < (cnt - i - 1); ++j) {
            if (pLscProfiles[j]->vignetting < pLscProfiles[j + 1]->vignetting) {
                lscTableProfile_t temp   = pLscProfiles[j];
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
    lsc_illu_case_t*   illu_case,
    const float         fVignetting,
    lscTableProfile_t  &ceilling,
    lscTableProfile_t  &floor
) {
    XCamReturn XCamReturn = XCAM_RETURN_NO_ERROR;
    lsc_illu_case_resolution_t& current_res = illu_case->res_group[illu_case->current_res_idx];

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
    lscTableProfile_t   pLscProfile1,
    lscTableProfile_t   pLscProfile2,
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
    Lsc_Hwconf_t *lscHwConf,
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
    lsc_otp_grad_t *otpGrad,
    Lsc_Hwconf_t *lscHwConf,
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
    const lsc_params_static_t *calib2_lsc,
    char* name,
    lscTableProfile_t &pLscTableProfile
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

static XCamReturn GetLscResIdxByName(lsc_illu_case_t* illu_case, char* name, uint32_t &resIdx)
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
    illu_lsc_node_t* pCurNode, * pNode0;
    if (listMaxSize == 0) {
        return;
    }
    int sizeList = get_list_num(l_head);
    if (sizeList < listMaxSize) {
        pCurNode = (illu_lsc_node_t*)aiq_mallocz(sizeof(illu_lsc_node_t));
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
        pNode0 = (illu_lsc_node_t*)(l_head->next);
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
    illu_lsc_node_t* pL;
    int* illuSet = (int*)aiq_mallocz(illuNum * sizeof(int));
    memset(illuSet, 0, illuNum * sizeof(int));
    while (head != pNextNode)
    {
        pL = (illu_lsc_node_t*)pNextNode;
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
    aiq_free(illuSet);

}


void ClearContext(LscContext_t* pLscCtx)
{
    clear_list(&pLscCtx->lscRest.dominateIlluList);

    for (uint32_t mode_id = 0; mode_id < USED_FOR_CASE_MAX; mode_id++) {
        if (pLscCtx->lsc_mode[mode_id].illu_case != nullptr)
            aiq_free(pLscCtx->lsc_mode[mode_id].illu_case);
    }

    if (pLscCtx->illu_case) {
        for (uint32_t id = 0; id < pLscCtx->illu_case_count; id++)
        {
            lsc_illu_case_t& illu_case = pLscCtx->illu_case[id];
            if (illu_case.res_group) {
                for (uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
                {
                    if (illu_case.res_group[res_id].lsc_table_group) {
                        aiq_free(illu_case.res_group[res_id].lsc_table_group);
                        illu_case.res_group[res_id].lsc_table_group = NULL;
                    } else {
                        LOGI_ALSC("%s: free: res_group[%d].lsc_table_group is already NULL!\n", __func__, res_id);
                    }
                }
                aiq_free(illu_case.res_group);
                illu_case.res_group = NULL;
            } else {
                LOGI_ALSC("%s: free: illu_case.res_group is already NULL!\n", __func__);
            }
        }
        aiq_free(pLscCtx->illu_case);
        pLscCtx->illu_case = NULL;
        pLscCtx->illu_case_count = 0;
    } else {
        LOGI_ALSC("%s: free: pLscCtx->illu_case is already NULL!\n", __func__);
    }

    if (pLscCtx->res_grad) {
        aiq_free(pLscCtx->res_grad);
        pLscCtx->res_grad = NULL;
        pLscCtx->res_grad_count = 0;
    } else {
        LOGI_ALSC("%s: free: pLscCtx->res_grad is already NULL!\n", __func__);
    }

}

XCamReturn convertSensorLscOTP(lsc_resolution_t *cur_res, lsc_otp_grad_t *otpGrad,
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

XCamReturn lscGetOtpInfo(RkAiqAlgoCom* params)
{
    if (!params)
        return XCAM_RETURN_BYPASS;

    LscContext_t* pLscCtx = (LscContext_t*)(params->ctx);
    RkAiqAlgoConfigLsc *para = (RkAiqAlgoConfigLsc *)params;

    lsc_sw_info_t *lscSwInfo = &para->lsc_sw_info;
    if (lscSwInfo->otpInfo.flag) {
        pLscCtx->otpGrad.flag = lscSwInfo->otpInfo.flag;
        pLscCtx->otpGrad.width = lscSwInfo->otpInfo.width;
        pLscCtx->otpGrad.height = lscSwInfo->otpInfo.height;
        pLscCtx->otpGrad.table_size = lscSwInfo->otpInfo.table_size;
        memcpy(pLscCtx->otpGrad.lsc_r, lscSwInfo->otpInfo.lsc_r, sizeof(pLscCtx->otpGrad.lsc_r));
        memcpy(pLscCtx->otpGrad.lsc_gr, lscSwInfo->otpInfo.lsc_gr, sizeof(pLscCtx->otpGrad.lsc_gr));
        memcpy(pLscCtx->otpGrad.lsc_gb, lscSwInfo->otpInfo.lsc_gb, sizeof(pLscCtx->otpGrad.lsc_gb));
        memcpy(pLscCtx->otpGrad.lsc_b, lscSwInfo->otpInfo.lsc_b, sizeof(pLscCtx->otpGrad.lsc_b));
    } else {
        pLscCtx->lscSwInfo.otpInfo.flag = 0;
    }

    LOGD_ALSC("otp: flag %d, WxH: %dx%d, table_size: %d\n",
            pLscCtx->otpGrad.flag,
            pLscCtx->otpGrad.width,
            pLscCtx->otpGrad.height,
            pLscCtx->otpGrad.table_size);

    LOGD_ALSC( "otp LscMatrix r[0:3]:%d,%d,%d,%d, gr[0:3]:%d,%d,%d,%d, gb[0:3]:%d,%d,%d,%d, b[0:3]:%d,%d,%d,%d\n",
            pLscCtx->otpGrad.lsc_r[0],
            pLscCtx->otpGrad.lsc_r[1],
            pLscCtx->otpGrad.lsc_r[2],
            pLscCtx->otpGrad.lsc_r[3],
            pLscCtx->otpGrad.lsc_gr[0],
            pLscCtx->otpGrad.lsc_gr[1],
            pLscCtx->otpGrad.lsc_gr[2],
            pLscCtx->otpGrad.lsc_gr[3],
            pLscCtx->otpGrad.lsc_gb[0],
            pLscCtx->otpGrad.lsc_gb[1],
            pLscCtx->otpGrad.lsc_gb[2],
            pLscCtx->otpGrad.lsc_gb[3],
            pLscCtx->otpGrad.lsc_b[0],
            pLscCtx->otpGrad.lsc_b[1],
            pLscCtx->otpGrad.lsc_b[2],
            pLscCtx->otpGrad.lsc_b[3]);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn UpdateLscCalibPtr(LscContext_t* pLscCtx) {
    const lsc_params_static_t* calib2 = &pLscCtx->lsc_attrib->stAuto.sta;
    const LscCof_ill_t* lsc_cof_tmp;
    // update pLscCtx->illu_case
    for(uint32_t ill_id = 0; ill_id < pLscCtx->illu_case_count; ill_id++)
    {
        lsc_illu_case_t& illu_case = pLscCtx->illu_case[ill_id];
        lsc_cof_tmp = &(calib2->lscCoef.illAll[ill_id]);

        illu_case.lsc_cof = lsc_cof_tmp;
        illu_case.res_count = calib2->common.resolutionAll_len;

        for(uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
        {
            lsc_illu_case_resolution_t& illu_case_res = illu_case.res_group[res_id];
            strcpy(illu_case_res.resolution.name, calib2->common.resolutionAll[res_id].name);
            illu_case_res.lsc_table_count = lsc_cof_tmp->tableUsed_len;
            //TODO: illu_case.resolution.width & height
            for (int used_id = 0; used_id < lsc_cof_tmp->tableUsed_len; used_id++)
            {
                char profile_name[64];
                memset(profile_name, 0, sizeof(profile_name));
                sprintf(profile_name, "%s_%s", illu_case_res.resolution.name,
                    lsc_cof_tmp->tableUsed[used_id].name);
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

    // update pLscCtx->lsc_mode_data_t
    uint32_t update_count[USED_FOR_CASE_MAX];
    memset(update_count, 0, sizeof(update_count));
    for(uint32_t ill_id = 0; ill_id < pLscCtx->illu_case_count; ill_id++)
    {
        uint32_t used_case = pLscCtx->illu_case[ill_id].lsc_cof->usedForCase;
        if (used_case >= USED_FOR_CASE_MAX) {
            LOGE_ALSC("%s: used_case=%d\n", __func__, used_case);
            return XCAM_RETURN_ERROR_PARAM;
        }
        uint32_t update_id = update_count[used_case];
        if (update_id >= pLscCtx->lsc_mode[used_case].illu_case_count) {
            LOGE_ALSC("%s: update_id=%d\n", __func__, update_id);
            return XCAM_RETURN_ERROR_PARAM;
        }
        pLscCtx->lsc_mode[used_case].illu_case[update_id] = &(pLscCtx->illu_case[ill_id]);
        update_count[used_case]++;
    }


    return XCAM_RETURN_NO_ERROR;
}

/** @brief called to arrange data to fill lsc_mode_data_t from CalibDb */
static XCamReturn UpdateLscCalibPara(LscContext_t* pLscCtx)
{
    const lsc_params_static_t* calib2 = &pLscCtx->lsc_attrib->stAuto.sta;
    const LscCof_ill_t* lsc_cof_tmp;
    uint32_t mode_count[USED_FOR_CASE_MAX];
    memset(mode_count, 0, sizeof(mode_count));

    pLscCtx->illu_case_count = calib2->lscCoef.illAll_len;
    pLscCtx->illu_case = (lsc_illu_case_t*)aiq_mallocz(pLscCtx->illu_case_count * sizeof(lsc_illu_case_t));
    memset(pLscCtx->illu_case, 0, pLscCtx->illu_case_count * sizeof(lsc_illu_case_t));

    // update pLscCtx->illu_case
    for(uint32_t ill_id = 0; ill_id < pLscCtx->illu_case_count; ill_id++)
    {
        lsc_illu_case_t& illu_case = pLscCtx->illu_case[ill_id];
        lsc_cof_tmp = &(calib2->lscCoef.illAll[ill_id]);

        illu_case.lsc_cof = lsc_cof_tmp;
        illu_case.res_count = calib2->common.resolutionAll_len;
        illu_case.res_group = (lsc_illu_case_resolution_t*)aiq_mallocz
            (illu_case.res_count * sizeof(lsc_illu_case_resolution_t));
        memset(illu_case.res_group, 0, illu_case.res_count * sizeof(lsc_illu_case_resolution_t));

        for(uint32_t res_id = 0; res_id < illu_case.res_count; res_id++)
        {
            lsc_illu_case_resolution_t& illu_case_res = illu_case.res_group[res_id];
            strcpy(illu_case_res.resolution.name, calib2->common.resolutionAll[res_id].name);
            //TODO: illu_case.resolution.width & height
            illu_case_res.lsc_table_count = lsc_cof_tmp->tableUsed_len;
            illu_case_res.lsc_table_group = (lscTableProfile_t*)aiq_mallocz(
                illu_case_res.lsc_table_count * sizeof(lscTableProfile_t));

            for (int used_id = 0; used_id < lsc_cof_tmp->tableUsed_len; used_id++)
            {
                char profile_name[64];
                memset(profile_name, 0, sizeof(profile_name));
                sprintf(profile_name, "%s_%s", illu_case_res.resolution.name,
                    lsc_cof_tmp->tableUsed[used_id].name);
                XCamReturn ret = CamCalibDbGetLscProfileByName(calib2,
                    profile_name, illu_case_res.lsc_table_group[used_id]);
                if (XCAM_RETURN_NO_ERROR != ret) {
                    LOGE_ALSC("%s: CamCalibDbGetLscProfileByName failed\n", __func__);
                    return ret;
                }
            }
            OrderLscProfilesByVignetting(illu_case_res.lsc_table_group, illu_case_res.lsc_table_count);
        }

        mode_count[lsc_cof_tmp->usedForCase]++;
    }

    // malloc pLscCtx->lsc_mode_data_t
    for (uint32_t mode_id = 0; mode_id < USED_FOR_CASE_MAX; mode_id++)
    {
        pLscCtx->lsc_mode[mode_id].illu_case_count = mode_count[mode_id];
        if (mode_count[mode_id] == 0) {
            pLscCtx->lsc_mode[mode_id].illu_case = NULL;
        } else {
            pLscCtx->lsc_mode[mode_id].illu_case = (pIlluCase_t*)aiq_mallocz(
                mode_count[mode_id] * sizeof(pIlluCase_t));
        }
    }

    // update pLscCtx->lsc_mode_data_t
    uint32_t update_count[USED_FOR_CASE_MAX];
    memset(update_count, 0, sizeof(update_count));
    for(uint32_t ill_id = 0; ill_id < pLscCtx->illu_case_count; ill_id++)
    {
        uint32_t used_case = pLscCtx->illu_case[ill_id].lsc_cof->usedForCase;
        if (used_case >= USED_FOR_CASE_MAX) {
            LOGE_ALSC("%s: used_case=%d\n", __func__, used_case);
            return XCAM_RETURN_ERROR_PARAM;
        }
        uint32_t update_id = update_count[used_case];
        if (update_id >= pLscCtx->lsc_mode[used_case].illu_case_count) {
            LOGE_ALSC("%s: update_id=%d\n", __func__, update_id);
            return XCAM_RETURN_ERROR_PARAM;
        }
        pLscCtx->lsc_mode[used_case].illu_case[update_id] = &(pLscCtx->illu_case[ill_id]);
        update_count[used_case]++;
    }

    // molloc & calculate the grad table
    pLscCtx->res_grad_count = calib2->common.resolutionAll_len;
    pLscCtx->res_grad = (lsc_grad_t*)aiq_mallocz(pLscCtx->res_grad_count * sizeof(lsc_grad_t));
    memset(pLscCtx->res_grad, 0, pLscCtx->res_grad_count * sizeof(lsc_grad_t));
    for (uint32_t res_id = 0; res_id < pLscCtx->res_grad_count; res_id++) {
        Lsc_Resolution_t& src = calib2->common.resolutionAll[res_id];
        lsc_grad_t& dst = pLscCtx->res_grad[res_id];
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
static void print_lsc_illu_case(lsc_illu_case_t& illu_case) {
    const AlscCof_ill_t *lsc_cof = illu_case.lsc_cof;
    char tmp[256] = {0};
    COUT(lsc_cof->usedForCase);
    COUT(lsc_cof->name);
    COUT(lsc_cof->wbGain[0]);
    COUT(lsc_cof->wbGain[1]);
    for (int i = 0; i < lsc_cof->tableUsed_len; i++) {
        COUT(i);
        COUT(lsc_cof->tableUsed[i].name);
    }
    array2str(lsc_cof->gains, lsc_cof->gains_len, tmp);
    COUT2(lsc_cof->gains, tmp);
    memset(tmp, 0, sizeof(tmp));
    array2str(lsc_cof->vig, lsc_cof->vig_len, tmp);
    COUT2(lsc_cof->vig, tmp);
    ENDL;

    for (int i = 0; i < illu_case.res_count; i++) {
        lsc_illu_case_resolution_t& res = illu_case.res_group[i];
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

static void print_lsc(LscContext_t* pLscCtx)
{
    for (int case_id = 0; case_id < USED_FOR_CASE_MAX; case_id++)
    {
        lsc_mode_data_t& mode = pLscCtx->lsc_mode[case_id];
        if (mode.illu_case_count == 0) {
            continue;
        }
        for (uint32_t ill_id = 0; ill_id < mode.illu_case_count; ill_id++)
        {
            lsc_illu_case_t& illu_case = mode.illu_case[ill_id];
            print_lsc_illu_case(illu_case);
        }
    }
    COUT(pLscCtx->lsc_attrib->stAuto.sta.common.enable);
    COUT(pLscCtx->lsc_attrib->stAuto.sta.common.resolutionAll_len);
    COUT(pLscCtx->lsc_attrib->stAuto.sta.lscCoef.damp_enable);
    COUT(pLscCtx->lsc_attrib->stAuto.sta.lscCoef.illAll_len);
    COUT(pLscCtx->lsc_attrib->stAuto.sta.tableAll_len);
}
#endif

XCamReturn LscAutoConfig(LscContext_t* pLscCtx)
{
    if (pLscCtx == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ALSC("%s: (enter) count:%d\n", __FUNCTION__, pLscCtx->count);

    uint32_t caseIndex = pLscCtx->lscRest.caseIndex;
    if (caseIndex >= USED_FOR_CASE_MAX) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    lsc_mode_data_t& lsc_mode_now = pLscCtx->lsc_mode[caseIndex];
    if (lsc_mode_now.illu_case == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    //1) Estimate best(nearest) illuminant case;
    uint32_t estimateIlluCaseIdx;
    int dominateIlluListSize = 15;//to do from xml;
    //float varianceLumaTh = 0.006;//to do from xml;
    ret = illuminant_index_estimation(lsc_mode_now, pLscCtx->lscSwInfo.awbGain, estimateIlluCaseIdx);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
    UpdateDominateIlluList(&pLscCtx->lscRest.dominateIlluList, estimateIlluCaseIdx, dominateIlluListSize);
    //TODO: working mode which has only one illuminant case, like gray mode, does not need to estimate index.
    StableIlluEstimation(&pLscCtx->lscRest.dominateIlluList, dominateIlluListSize,
        lsc_mode_now.illu_case_count, estimateIlluCaseIdx);

    lsc_illu_case_t* illu_case = lsc_mode_now.illu_case[estimateIlluCaseIdx];

    // 2) get resolution index;
    uint32_t resIdx;
    ret = GetLscResIdxByName(illu_case, pLscCtx->cur_res.name, resIdx);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    // 3) calculate vignetting from sensor gain;
    float sensorGain = pLscCtx->lscSwInfo.sensorGain;
    float fVignetting;
    interpolation(illu_case->lsc_cof->gains,
                  illu_case->lsc_cof->vig,
                  illu_case->lsc_cof->vig_len,
                  sensorGain, &fVignetting);
    /* *************
    * if (fVignetting/estimateIlluCaseIdx/resIdx changed || forceRunFlag)
    *   Select LscProfiles by vig
    *   if (forceRunFlag || pLscProfile1/2 changed)
    *       Interpolate tbl by vig
    *       update pLscCtx->lscRest.pLscProfile1/2
    * **************/
    bool flag0 = fabs(pLscCtx->lscRest.fVignetting - fVignetting) > DIVMIN ||
                        pLscCtx->lscRest.estimateIlluCaseIdx != estimateIlluCaseIdx ||
                        pLscCtx->lscRest.resIdx != resIdx ||
                        pLscCtx->smartRunRes.forceRunFlag; //forceRunFlag: prepare(init or calib/res/graymode change) or updateAttr
    LOGD_ALSC("pickLscProfile: %d = fVignetting(%f->%f)/estimateIlluCaseIdx(%d->%d)/resIdx(%d->%d) changed or forceRunFlag",
                flag0, pLscCtx->lscRest.fVignetting, fVignetting, pLscCtx->lscRest.estimateIlluCaseIdx, estimateIlluCaseIdx,
                pLscCtx->lscRest.resIdx, resIdx, pLscCtx->smartRunRes.forceRunFlag);
    if (flag0) {
        pLscCtx->isReCal_ = pLscCtx->smartRunRes.forceRunFlag ||
                            fabs(pLscCtx->lscRest.fVignetting - fVignetting) > DIVMIN;
        pLscCtx->lscRest.estimateIlluCaseIdx = estimateIlluCaseIdx;
        pLscCtx->lscRest.resIdx = resIdx;
        pLscCtx->lscRest.fVignetting =  fVignetting;

        // 4) select vignetting section and get the lsc matrix table;
        lscTableProfile_t pLscProfile1 = NULL;
        lscTableProfile_t pLscProfile2 = NULL;
        ret = VignSelectLscProfiles(illu_case, fVignetting, pLscProfile1, pLscProfile2);
        if (pLscProfile1 && pLscProfile2) {
            pLscCtx->isReCal_ = pLscCtx->isReCal_ ||
                            strcmp(pLscProfile1->name, pLscCtx->lscRest.LscProName1) ||
                            strcmp(pLscProfile2->name, pLscCtx->lscRest.LscProName2);
            LOGD_ALSC("pLscCtx->isReCal_: %d = forceRunFlag(%d) || fVignetting changed || pLscProfile1/2 changed", pLscCtx->isReCal_, pLscCtx->smartRunRes.forceRunFlag);
        } else {
            LOGE_ALSC("check %s %s pLscProfile: %p %p \n", pLscCtx->cur_res.name, illu_case->lsc_cof->name, pLscProfile1, pLscProfile2);
            return XCAM_RETURN_ERROR_PARAM;
        }
        if (pLscCtx->isReCal_) {
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD_ALSC("fVignetting: %f (%f .. %f)\n",  fVignetting, pLscProfile1->vignetting, pLscProfile2->vignetting);

                ret = VignInterpolateMatrices(fVignetting, pLscProfile1, pLscProfile2,
                                            &pLscCtx->lscRest.undampedLscMatrixTable);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    return (ret);
                }
            } else if (ret == XCAM_RETURN_ERROR_OUTOFRANGE) {
                /* we don't need to interpolate */
                LOGD_ALSC("fVignetting: %f (%f)\n",  fVignetting, pLscProfile1->vignetting);
                memcpy(&pLscCtx->lscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED],
                    &pLscProfile1->lsc_samples_red, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&pLscCtx->lscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR],
                    &pLscProfile1->lsc_samples_greenR, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&pLscCtx->lscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB],
                    &pLscProfile1->lsc_samples_greenB, sizeof(Cam17x17UShortMatrix_t));
                memcpy(&pLscCtx->lscRest.undampedLscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE],
                    &pLscProfile1->lsc_samples_blue, sizeof(Cam17x17UShortMatrix_t));
            }
            strcpy(pLscCtx->lscRest.LscProName1, pLscProfile1->name);
            strcpy(pLscCtx->lscRest.LscProName2, pLscProfile2->name);
        }
    }

    // 5) . Damping
     /* if (isReCal_ || tblConverge==0) update hwconf
     *  else keep last hwconf */

    float dampCoef = (pLscCtx->lsc_attrib->stAuto.sta.lscCoef.damp_enable && pLscCtx->count > 1) ? pLscCtx->lscSwInfo.awbIIRDampCoef : 0;
    if (!pLscCtx->isReCal_ && pLscCtx->smartRunRes.lscTableConverge) {
        pLscCtx->isReCal_ = false;
    } else {
        if (pLscCtx->otpGrad.flag && pLscCtx->otpGrad.table_size > 0){
            ret = DampingandOtp(dampCoef, &pLscCtx->lscRest.undampedLscMatrixTable,
                    &pLscCtx->lscRest.dampedLscMatrixTable,
                    &pLscCtx->otpGrad,
                    &pLscCtx->lscHwConf,
                    &pLscCtx->smartRunRes.lscTableConverge);
        } else {
            ret = Damping(dampCoef, &pLscCtx->lscRest.undampedLscMatrixTable,
                    &pLscCtx->lscHwConf,
                    &pLscCtx->smartRunRes.lscTableConverge);
        }
        pLscCtx->isReCal_ = true;
    }

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

void JudgeAutoRun
(
    LscContext_t* pLscCtx

) {
    bool gainStable;
    bool wbgainStable;
    float wb_th = pLscCtx->smartRunCfg.wbgain_th*pLscCtx->smartRunCfg.wbgain_th;
    if (fabs(pLscCtx->lscSwInfo.sensorGain - pLscCtx->smartRunRes.last_gain) > pLscCtx->smartRunCfg.gain_th) {
        pLscCtx->smartRunRes.last_gain = pLscCtx->lscSwInfo.sensorGain;
        gainStable = false;
    } else {
        gainStable = true;
    }

    if ((pLscCtx->smartRunRes.last_awbGain[0]-pLscCtx->lscSwInfo.awbGain[0])*(pLscCtx->smartRunRes.last_awbGain[0]-pLscCtx->lscSwInfo.awbGain[0])
         + (pLscCtx->smartRunRes.last_awbGain[1]-pLscCtx->lscSwInfo.awbGain[1])*(pLscCtx->smartRunRes.last_awbGain[1]-pLscCtx->lscSwInfo.awbGain[1]) > wb_th) {
        pLscCtx->smartRunRes.last_awbGain[0] = pLscCtx->lscSwInfo.awbGain[0];
        pLscCtx->smartRunRes.last_awbGain[1] = pLscCtx->lscSwInfo.awbGain[1];
        wbgainStable = false;
        LOGD_ALSC("update wbgain: %f, %f\n", pLscCtx->lscSwInfo.awbGain[0], pLscCtx->lscSwInfo.awbGain[1]);
    } else {
        wbgainStable = true;
        pLscCtx->lscSwInfo.awbGain[0] = pLscCtx->smartRunRes.last_awbGain[0];
        pLscCtx->lscSwInfo.awbGain[1] = pLscCtx->smartRunRes.last_awbGain[1];
    }

    pLscCtx->smartRunRes.res3aChg = !(wbgainStable && gainStable);
    LOGD_ALSC("awbStable(%d),aeStable(%d),lscTableConverge(%d),forceRunFlag(%d)",
        wbgainStable,gainStable,pLscCtx->smartRunRes.lscTableConverge,pLscCtx->smartRunRes.forceRunFlag);

/*     bool notRunflag = wbgainStable && gainStable
        && pLscCtx->smartRunRes.lscTableConverge
        && !(pLscCtx->smartRunRes.forceRunFlag);
    pLscCtx->smartRunRes.samrtRunFlag = !notRunflag;
    LOGD_ALSC("smartRun(%d),awbStable(%d),aeStable(%d),lscTableConverge(%d),forceRunFlag(%d)",pLscCtx->smartRunRes.samrtRunFlag,
        wbgainStable,gainStable,pLscCtx->smartRunRes.lscTableConverge,pLscCtx->smartRunRes.forceRunFlag);
    pLscCtx->smartRunRes.forceRunFlag =false; */
}

XCamReturn LscRelease(LscContext_t* pLscCtx)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ClearContext(pLscCtx);

    if (pLscCtx) {
        aiq_free(pLscCtx);
        pLscCtx = NULL;
    } else {
        LOGI_ALSC("%s: free: pLscCtx is already NULL!\n", __func__);
    }

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);

}

XCamReturn LscPrepare(LscContext_t* pLscCtx)
{
    LOGI_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ClearContext(pLscCtx);
    ret = UpdateLscCalibPara(pLscCtx);
    if (XCAM_RETURN_NO_ERROR != ret) {
        LOGE_ALSC("%s: UpdateLscCalibPara failed\n", __FUNCTION__);
        return ret;
    }

    pLscCtx->smartRunRes.forceRunFlag = true;
    const Lsc_Common_t& lsc_com = pLscCtx->lsc_attrib->stAuto.sta.common;
    const Lsc_Resolution_t* calib_lsc_res = nullptr;
    for(int i = 0; i < lsc_com.resolutionAll_len; i++) {
        if (strcmp(lsc_com.resolutionAll[i].name, pLscCtx->cur_res.name) == 0) {
            calib_lsc_res = &lsc_com.resolutionAll[i];
            break;
        }
    }
    if (calib_lsc_res != nullptr) {
        memcpy(pLscCtx->lscHwConf.x_size_tbl, calib_lsc_res->lsc_sect_size_x, sizeof(calib_lsc_res->lsc_sect_size_x));
        memcpy(pLscCtx->lscHwConf.y_size_tbl, calib_lsc_res->lsc_sect_size_y, sizeof(calib_lsc_res->lsc_sect_size_y));
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
    }

    const lsc_grad_t* cur_grad = NULL;
    for (uint32_t i = 0; i < pLscCtx->res_grad_count; i++) {
        if (0 == strcmp(pLscCtx->res_grad[i].resolution.name, pLscCtx->cur_res.name)) {
            cur_grad = &(pLscCtx->res_grad[i]);
            break;
        }
    }
    if (cur_grad != NULL) {
        memcpy(pLscCtx->lscHwConf.x_grad_tbl, cur_grad->LscXGradTbl, sizeof(cur_grad->LscXGradTbl));
        memcpy(pLscCtx->lscHwConf.y_grad_tbl, cur_grad->LscYGradTbl, sizeof(cur_grad->LscYGradTbl));
    } else {
        ret = XCAM_RETURN_ERROR_PARAM;
    }

#ifdef ARCHER_DEBUG
    if (cur_grad) {
        LOGE_ALSC( "%s\n", PRT_ARRAY(cur_grad->LscXGradTbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(cur_grad->LscYGradTbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(pLscCtx->lscHwConf.x_grad_tbl) );
        LOGE_ALSC( "%s\n", PRT_ARRAY(pLscCtx->lscHwConf.y_grad_tbl) );
    }
#endif

    LOGI_ALSC("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn LscProcessing(LscContext_t* pLscCtx)
{
    LOG1_ALSC("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if _TEST_LSC_VALID_ON_HARDWARE
    _test_if_hw_lsc_valid(pLscCtx);
#endif

    LOG1_ALSC("%s: (exit)\n", __FUNCTION__);
    return(ret);
}




RKAIQ_END_DECLARE



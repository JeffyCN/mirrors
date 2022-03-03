/*
* rk_aiq_accm_algo.cpp

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

#include "accm/rk_aiq_accm_algo.h"
#include "xcam_log.h"
#include "interpolation.h"

RKAIQ_BEGIN_DECLARE

XCamReturn Swinfo_wbgain_init(float awbGain[2], const CalibDbV2_Ccm_Tuning_Para_t *pCalib, const char* illuName)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ACCM("%s(%d): (enter)\n", __FUNCTION__, __LINE__);

    if (pCalib == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ACCM("%s(%d): invalid input params\n", __FUNCTION__, __LINE__);
        return ret;
    }

    bool lsFound = false;

    for(int i = 0; i < pCalib->aCcmCof_len; i++) {
        if(strcmp(pCalib->aCcmCof[i].name, illuName) == 0) {
            awbGain[0] = pCalib->aCcmCof[i].awbGain[0];
            awbGain[1] = pCalib->aCcmCof[i].awbGain[1];
            lsFound = true;
            LOGD_ACCM("%s: accm lsForFirstFrame:%s", __FUNCTION__, pCalib->aCcmCof[i].name);
            break;
        }
    }
    if(pCalib->aCcmCof_len> 0 && lsFound == false) {
        awbGain[0] = pCalib->aCcmCof[0].awbGain[0];
        awbGain[1] = pCalib->aCcmCof[0].awbGain[1];
        LOGD_ACCM("%s: accm lsForFirstFrame:%s", __FUNCTION__, pCalib->aCcmCof[0].name);
    }
    LOGV_ACCM("%s: accm illunum:%d", __FUNCTION__, pCalib->aCcmCof_len);
    LOG1_ACCM( "%s(%d): (exit)\n", __FUNCTION__, __LINE__);
    return ret;
}

XCamReturn illuminant_index_estimation_ccm(int light_num, const CalibDbV2_Ccm_Accm_Cof_Para_t illAll[], float awbGain[2], int* illuminant_index)
{

    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    float minDist = 9999999;
    float dist[8];
    float nRG, nBG;
    nRG = awbGain[0];
    nBG = awbGain[1];
    *illuminant_index = 0;
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    for(int i = 0; i < light_num; i++)
    {
        dist[i] = sqrt((nRG - illAll[i].awbGain[0]) * (nRG -  illAll[i].awbGain[0])
                       + (nBG -  illAll[i].awbGain[1]) * (nBG -  illAll[i].awbGain[1]));
        if(dist[i] < minDist)
        {
            minDist = dist[i];
            *illuminant_index = i;
            ret = XCAM_RETURN_NO_ERROR;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ACCM("fail to estimate illuminant!!!\n");
    }

    LOGD_ACCM( "wbGain:%f,%f, estimation illuminant  is %s(%d) \n", awbGain[0], awbGain[1],
               illAll[*illuminant_index].name, *illuminant_index);

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn illuminant_index_candidate_ccm(int light_num, const CalibDbV2_Ccm_Accm_Cof_Para_t illAll[], float awbGain[2], char* default_illu, float prob_limit, const float weight_rb[2], float* prob)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    float dist[8];
    float nRG, nBG;
    nRG = awbGain[0];
    nBG = awbGain[1];
    float wr = weight_rb[0];
    float wb = weight_rb[1];
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    if (light_num == 0) {
        LOGE_ACCM(" Illuminant Profile does not exit  !!!\n");
        return ret;
    }
    LOGD_ACCM( "==================Illu Probability=========================\n");
    LOGD_ACCM( " wbGain: %f, %f \n",awbGain[0], awbGain[1] );
    memset(prob, 0, light_num*sizeof(float));
    if (light_num == 1) {
        prob[0] =1;
        LOGD_ACCM( " Only one illuminant profile: Illu Name is %s, prob = %f \n", illAll[0].name, prob[0]);
        ret = XCAM_RETURN_NO_ERROR;}
    else {
        float mean_dist = 0;
        int idx = 0;
        float minDist = 9999999;
        int default_illu_idx = 0;
        for(int i = 0; i < light_num; i++) {
            dist[i] = sqrt((nRG - illAll[i].awbGain[0]) * (nRG -  illAll[i].awbGain[0])*wr + (nBG -  illAll[i].awbGain[1]) * (nBG -  illAll[i].awbGain[1])*wb);
            if (strcmp( default_illu, illAll[i].name) == 0)
                default_illu_idx = i;
            if(dist[i] < minDist)
            {
                minDist = dist[i];
                idx = i;
            }
            mean_dist = mean_dist + dist[i];
        }
        if (minDist  < illAll[idx].minDist) {
            prob[idx] = 1;
            LOGD_ACCM( " Illu Name: %s(%d), dist = %f, prob = %f \n", illAll[idx].name, idx,  dist[idx], prob[idx]);
            LOGD_ACCM( "=======================================================\n");
            ret = XCAM_RETURN_NO_ERROR;
            LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
            return ret;
        }
        float sigma = 0;
        mean_dist = mean_dist/light_num;
        for(int i = 0; i < light_num; i++)
            sigma = sigma + (dist[i] - mean_dist)* (dist[i] - mean_dist);
        sigma = sigma/light_num;

        if (fabs(sigma)>DIVMIN) {
            float sum_prob = 0;
            for(int i = 0; i < light_num; i++) {
                prob[i] = exp(-0.5*dist[i]*dist[i]/sigma);
                sum_prob = sum_prob + prob[i];
            }
            // normalize prob
            float new_sum_prob = 0;
            LOGD_ACCM( " prob_limit =  %f \n",prob_limit);
            for(int i = 0; i < light_num; i++) {
                float prob_tmp;
                prob_tmp = prob[i]/sum_prob;
                LOGD_ACCM( " Illu Name: %s(%d), dist = %f, ori prob = %f \n", illAll[i].name, i,  dist[i], prob_tmp);
                if (prob_tmp < prob_limit ) {
                    prob[i] = 0;
                }
                new_sum_prob += prob[i];
            }
            LOGD_ACCM( "------------------------------------\n");
            if (fabs(new_sum_prob)<DIVMIN) {
                LOGE_ACCM(" prob_limit is too large  !!!\n");
                float max_prob = DIVMIN;
                int maxprob_idx = 0;
                for (int i = 0; i < light_num; i++) {
                    if (prob[i] > max_prob){
                        max_prob = prob[i];
                        maxprob_idx = i;
                    }
                }
                memset(prob, 0, light_num*sizeof(float));
                prob[maxprob_idx] = 1;
                LOGD_ACCM(" pick max prob illu : %s(%d), dist = %f, ori prob = %f \n", illAll[maxprob_idx].name, maxprob_idx,  dist[maxprob_idx], prob[maxprob_idx]);
            } else {
                float *prob_test = (float*)malloc(light_num*sizeof(float));
                memset(prob_test, 0, light_num*sizeof(float));
                float fsum_prob = 0;
                for(int i = 0; i < light_num; i++) {
                    prob_test[i] = prob[i]/new_sum_prob;
                    if (prob_test[i]<0.333333)
                        prob_test[i] = 0;
                    fsum_prob += prob_test[i];
                }
                if (fabs(fsum_prob)<DIVMIN)
                    fsum_prob = new_sum_prob;
                else
                    memcpy(prob,  prob_test, light_num*sizeof(float));
                for(int i = 0; i < light_num; i++) {
                    prob[i] = prob[i]/fsum_prob;
                    LOGD_ACCM( " Illu Name: %s(%d), dist = %f, prob = %f \n", illAll[i].name, i,  dist[i], prob[i]);
                }
                free(prob_test);
                prob_test = NULL;
            }
        }
        else {
            prob[default_illu_idx] = 1;
            LOGD_ACCM( " All dist values are equal, pick Default Illu: %s(%d), dist = %f, prob = %f \n", illAll[default_illu_idx].name, default_illu_idx,  dist[default_illu_idx], prob[default_illu_idx]);
        }

        ret = XCAM_RETURN_NO_ERROR;
    }
    if(ret != XCAM_RETURN_NO_ERROR)
            LOGE_ACCM(" Fail to get Illuminant Probability !!!\n");
    LOGD_ACCM( "=======================================================\n");
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return ret;
}

static XCamReturn AwbOrderCcmProfilesBySaturation
(
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t* pCcmProfiles[],
    const int32_t   cnt
) {
    int32_t i, j;

    for (i = 0; i < (cnt - 1); ++i) {
        for (j = 0; j < (cnt - i - 1); ++j) {
            if (pCcmProfiles[j]->saturation < pCcmProfiles[j + 1]->saturation) {
                const CalibDbV2_Ccm_Ccm_Matrix_Para_t* temp   = pCcmProfiles[j];
                pCcmProfiles[j]         = pCcmProfiles[j + 1];
                pCcmProfiles[j + 1]       = temp;
            }
        }
    }

    return (XCAM_RETURN_NO_ERROR);
}

XCamReturn pCcmMatrixAll_init(accm_context_t* accm_context, const CalibDbV2_Ccm_Tuning_Para_t *pCalib)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ACCM("%s(%d): (enter)\n", __FUNCTION__, __LINE__);

    if (pCalib == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        LOGE_ACCM("%s(%d): invalid input params\n", __FUNCTION__, __LINE__);
        return ret;
    }

    //Config  pCcmMatrixAll (normal and hdr)
       // 1) get and reorder para
    for(int i = 0; i < pCalib->aCcmCof_len; i++) {
        for (int j = 0; j < pCalib->aCcmCof[i].matrixUsed_len; j++) {
            char name[CCM_PROFILE_NAME];
            sprintf(name, "%s", pCalib->aCcmCof[i].matrixUsed[j]);
            const CalibDbV2_Ccm_Ccm_Matrix_Para_t* pCcmMatrixProfile = NULL;
            // get a ccm-profile from database
            ret = CamCalibDbGetCcmProfileByName(pCalib, name, &pCcmMatrixProfile);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
            // store ccm-profile in pointer array
            accm_context->pCcmMatrixAll[i][j] = pCcmMatrixProfile;
            LOGV_ACCM("CCM name  %s coef:%f,%f,%f  %f,%f,%f  %f,%f,%f  \n", name,
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[0],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[1],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[2],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[3],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[4],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[5],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[6],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[7],
                      accm_context->pCcmMatrixAll[i][j]->ccMatrix[8]);
            LOGV_ACCM("CCM name  %s off:%f,%f,%f  \n", name,
                      accm_context->pCcmMatrixAll[i][j]->ccOffsets[0],
                      accm_context->pCcmMatrixAll[i][j]->ccOffsets[1],
                      accm_context->pCcmMatrixAll[i][j]->ccOffsets[2]);

        }
        // order ccm-profiles by saturation
        ret = AwbOrderCcmProfilesBySaturation(accm_context->pCcmMatrixAll[i],
                                              pCalib->aCcmCof[i].matrixUsed_len);
        //RETURN_RESULT_IF_DIFFERENT(result, XCAM_RETURN_NO_ERROR);

    }

    LOG1_ACCM( "%s(%d): (exit)\n", __FUNCTION__, __LINE__);
    return ret;
}
static XCamReturn SatSelectCcmProfiles
(
    const float     fSaturation,
    int32_t         no_ccm,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t* pCcmProfiles[],
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t** pCcmProfile1,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t** pCcmProfile2
) {
    XCamReturn XCamReturn = XCAM_RETURN_NO_ERROR;

    if ((no_ccm == 0) || (pCcmProfiles == NULL)
            || (pCcmProfile1 == NULL) || (pCcmProfile2 == NULL)) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    if (fSaturation >= pCcmProfiles[0]->saturation) {
        *pCcmProfile1 = pCcmProfiles[0];
        *pCcmProfile2 = NULL;
        LOGV_ACCM( "select:%s \n", (*pCcmProfile1)->name);
        XCamReturn = XCAM_RETURN_ERROR_OUTOFRANGE;
    } else {
        int32_t nLast = no_ccm - 1;
        if (fSaturation <= pCcmProfiles[nLast]->saturation) {
            *pCcmProfile1 = pCcmProfiles[nLast];
            *pCcmProfile2 = NULL;
            LOGV_ACCM( "select:%s \n", (*pCcmProfile1)->name);
            XCamReturn = XCAM_RETURN_ERROR_OUTOFRANGE;
        } else {
            uint16_t n = 0;

            /* find the segment */
            while ((fSaturation <= pCcmProfiles[n]->saturation) && (n <= nLast)) {
                n++;
            }

            if (n > 0)
                n--;

            *pCcmProfile1 = pCcmProfiles[n];
            *pCcmProfile2 = pCcmProfiles[n + 1];

        }
    }

    return (XCamReturn);
}


/******************************************************************************
 * InterpolateMatrices
 *****************************************************************************/
static XCamReturn SatInterpolateMatrices
(
    const float             fSat,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t*   pCcProfileA,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t*   pCcProfileB,
    float*          pResMatrix
) {
    XCamReturn iXCamReturn = XCAM_RETURN_ERROR_PARAM;

    if ((pCcProfileA != NULL) && (pCcProfileA != NULL) && (pResMatrix != NULL)) {
        const float *pMatrixA = pCcProfileA->ccMatrix;
        const float *pMatrixB = pCcProfileB->ccMatrix;

        float fSatA = pCcProfileA->saturation;
        float fSatB = pCcProfileB->saturation;

        float f1 = ( fSatB - fSat ) / ( fSatB - fSatA ); // test: if fSat == fSatA => f1 = 1 => choose A: ok
        float f2 = 1.0f - f1;

        int i;

        for ( i = 0; i < 9; i++)
        {
            pResMatrix[i] = f1 * pMatrixA[i] + f2 * pMatrixB[i];
        }

        iXCamReturn = XCAM_RETURN_NO_ERROR;

    }

    return (iXCamReturn);
}

/******************************************************************************
 * InterpolateMatrices
 *****************************************************************************/
static XCamReturn SatInterpolateOffset
(
    const float             fSat,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t*   pCcProfileA,
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t*   pCcProfileB,
    float     *pResOffset
) {
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ( (pCcProfileA != NULL) && (pCcProfileB != NULL) && (pResOffset != NULL) )
    {
        const float *pOffsetA = pCcProfileA->ccOffsets;
        const float *pOffsetB = pCcProfileB->ccOffsets;

        float fSatA = pCcProfileA->saturation;
        float fSatB = pCcProfileB->saturation;

        float f1 = ( fSatB - fSat ) / ( fSatB - fSatA ); // test: if fSat == fSatA => f1 = 1 => choose A: ok
        float f2 = 1.0f - f1;

        pResOffset[CAM_3CH_COLOR_COMPONENT_RED]
            = f1 * pOffsetA[CAM_3CH_COLOR_COMPONENT_RED] + f2 * pOffsetB[CAM_3CH_COLOR_COMPONENT_RED];
        pResOffset[CAM_3CH_COLOR_COMPONENT_GREEN]
            = f1 * pOffsetA[CAM_3CH_COLOR_COMPONENT_GREEN] + f2 * pOffsetB[CAM_3CH_COLOR_COMPONENT_GREEN];
        pResOffset[CAM_3CH_COLOR_COMPONENT_BLUE]
            = f1 * pOffsetA[CAM_3CH_COLOR_COMPONENT_BLUE] + f2 * pOffsetB[CAM_3CH_COLOR_COMPONENT_BLUE];

        result = XCAM_RETURN_NO_ERROR;
    }

    return ( result );

}

/******************************************************************************
 * Damping
 *****************************************************************************/
static XCamReturn Damping
(
    const float         damp,                /**< damping coefficient */
    float *pMatrixUndamped,   /**< undamped new computed matrices */
    float *pMatrixDamped,     /**< old matrices and result */
    float *pOffsetUndamped,   /**< undamped new computed offsets */
    float *pOffsetDamped      /**< old offset and result */
)
{
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ( (pMatrixUndamped != NULL) && (pMatrixDamped != NULL)
            && (pOffsetUndamped != NULL) && (pOffsetDamped != NULL) )
    {
        int32_t i;
        float f = (1.0f - damp);

        /* calc. damped cc matrix */
        for( i = 0; i < 9; i++ )
        {
            pMatrixDamped[i] = (damp * pMatrixDamped[i]) + (f *  pMatrixUndamped[i]);
        }

        /* calc. damped cc offsets */
        pOffsetDamped[CAM_3CH_COLOR_COMPONENT_RED]
            = (damp * pOffsetDamped[CAM_3CH_COLOR_COMPONENT_RED])
              + (f * pOffsetUndamped[CAM_3CH_COLOR_COMPONENT_RED]);
        pOffsetDamped[CAM_3CH_COLOR_COMPONENT_GREEN]
            = (damp * pOffsetDamped[CAM_3CH_COLOR_COMPONENT_GREEN])
              + (f * pOffsetUndamped[CAM_3CH_COLOR_COMPONENT_GREEN]);
        pOffsetDamped[CAM_3CH_COLOR_COMPONENT_BLUE]
            = (damp * pOffsetDamped[CAM_3CH_COLOR_COMPONENT_BLUE])
              + (f * pOffsetUndamped[CAM_3CH_COLOR_COMPONENT_BLUE]);

        result = XCAM_RETURN_NO_ERROR;
    }
    LOGD_ACCM( "dampfactor:%f", damp);
    LOGD_ACCM( " undampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f  undampedCcOffset:%f,%f,%f  \n",
               pMatrixUndamped[0],
               pMatrixUndamped[1],
               pMatrixUndamped[2],
               pMatrixUndamped[3],
               pMatrixUndamped[4],
               pMatrixUndamped[5],
               pMatrixUndamped[6],
               pMatrixUndamped[7],
               pMatrixUndamped[8],
               pOffsetUndamped[0],
               pOffsetUndamped[1],
               pOffsetUndamped[2]
             );
    LOGD_ACCM( " dampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f  dampedCcOffset:%f,%f,%f  \n",
               pMatrixDamped[0],
               pMatrixDamped[1],
               pMatrixDamped[2],
               pMatrixDamped[3],
               pMatrixDamped[4],
               pMatrixDamped[5],
               pMatrixDamped[6],
               pMatrixDamped[7],
               pMatrixDamped[8],
               pOffsetDamped[0],
               pOffsetDamped[1],
               pOffsetDamped[2]
             );


    return ( result );
}

void Saturationadjust(float fScale, accm_handle_t hAccm )
{
    float *pccMatrixA;
    float  Matrix_tmp[9];
    float flevel1 = hAccm->accmRest.color_saturation_level;
    if (fScale < DIVMIN) {
         if(fabs((flevel1-50))>DIVMIN){
                 LOGW_ACCM("fSclae is  %f  , so saturation adjust bypass\n",  fScale);
         }
     } else {
        flevel1 = (flevel1 - 50) / 50 + 1;
        LOGV_ACCM("Satura: %f \n", flevel1);
        pccMatrixA = hAccm->accmRest.undampedCcmMatrix;
        memcpy(&Matrix_tmp, pccMatrixA, sizeof(Matrix_tmp));
        float *pccMatrixB;
        pccMatrixB = Matrix_tmp;
        /* M =  (M0 - E) * fscale + E
           M' = ycbcr2rgb[ sat_matrix * rgb2ycbcr(M) ]
           M1 = (M' - E) / fscale + E
        */
        if ( (pccMatrixA != NULL) && (pccMatrixB != NULL) )
        {
            for(int i =0; i < 9; i++)
            {
                if (i == 0 || i == 4 || i == 8){
                  pccMatrixA[i] = (pccMatrixA[i] - 1)*fScale+1;
                }
                else{
                  pccMatrixA[i] = pccMatrixA[i]*fScale;
                }
            }
            pccMatrixB[0] = 0.299 * pccMatrixA[0] + 0.587 * pccMatrixA[3] + 0.114 * pccMatrixA[6];
            pccMatrixB[1] = 0.299 * pccMatrixA[1] + 0.587 * pccMatrixA[4] + 0.114 * pccMatrixA[7];
            pccMatrixB[2] = 0.299 * pccMatrixA[2] + 0.587 * pccMatrixA[5] + 0.114 * pccMatrixA[8];
            pccMatrixB[3] = -0.1687 * pccMatrixA[0] - 0.3313 * pccMatrixA[3] + 0.5 * pccMatrixA[6];
            pccMatrixB[4] = -0.1687 * pccMatrixA[1] - 0.3313 * pccMatrixA[4] + 0.5 * pccMatrixA[7];
            pccMatrixB[5] = -0.1687 * pccMatrixA[2] - 0.3313 * pccMatrixA[5] + 0.5 * pccMatrixA[8];
            pccMatrixB[6] = 0.5 * pccMatrixA[0]  - 0.4187 * pccMatrixA[3] - 0.0813 * pccMatrixA[6];
            pccMatrixB[7] = 0.5 * pccMatrixA[1]  - 0.4187 * pccMatrixA[4] - 0.0813 * pccMatrixA[7];
            pccMatrixB[8] = 0.5 * pccMatrixA[2]  - 0.4187 * pccMatrixA[5] - 0.0813 * pccMatrixA[8];

            for(int i = 3; i < 9; i++)
            {
                 pccMatrixB[i] = flevel1 * pccMatrixB[i];
             }
            pccMatrixA[0] = 1 * pccMatrixB[0] + 0 * pccMatrixB[3] + 1.402 * pccMatrixB[6];
            pccMatrixA[1] = 1 * pccMatrixB[1] + 0 * pccMatrixB[4] + 1.402 * pccMatrixB[7];
            pccMatrixA[2] = 1 * pccMatrixB[2] + 0 * pccMatrixB[5] + 1.402 * pccMatrixB[8];
            pccMatrixA[3] = 1 * pccMatrixB[0] - 0.34414 * pccMatrixB[3]  - 0.71414 * pccMatrixB[6];
            pccMatrixA[4] = 1 * pccMatrixB[1] - 0.34414 * pccMatrixB[4]  - 0.71414 * pccMatrixB[7];
            pccMatrixA[5] = 1 * pccMatrixB[2] - 0.34414 * pccMatrixB[5]  - 0.71414 * pccMatrixB[8];
            pccMatrixA[6] = 1 * pccMatrixB[0]  + 1.772 * pccMatrixB[3] + 0 * pccMatrixB[6];
            pccMatrixA[7] = 1 * pccMatrixB[1]  + 1.772 * pccMatrixB[4] + 0 * pccMatrixB[7];
            pccMatrixA[8] = 1 * pccMatrixB[2]  + 1.772 * pccMatrixB[5] + 0 * pccMatrixB[8];


            for(int i =0; i < 9; i++)
            {
                if (i == 0 || i == 4 || i == 8){
                  pccMatrixA[i] = (pccMatrixA[i] - 1)/fScale+1;
                }
                else{
                  pccMatrixA[i] = pccMatrixA[i]/fScale;
                }
            }
        }

      }

}


XCamReturn CamCalibDbGetCcmProfileByName(const CalibDbV2_Ccm_Tuning_Para_t *calibCcm, char* name, const CalibDbV2_Ccm_Ccm_Matrix_Para_t **pCcmMatrixProfile)
{
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;

    for(int i = 0; i <calibCcm->matrixAll_len; i++) {
        if(strcmp(calibCcm->matrixAll[i].name, name) == 0) {
            *pCcmMatrixProfile = &calibCcm->matrixAll[i];
            ret = XCAM_RETURN_NO_ERROR;
            break;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ACCM("can't find %s in matrixAll \n", name);
    }
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return ret;
}

static void UpdateIlluProbList(List *l, int illu, float prob, int listMaxSize)
{
    prob_node_t *pCurNode;
    int sizeList;
    if(listMaxSize == 0) {
        return;
    }
    pCurNode = (prob_node_t*)malloc(sizeof(prob_node_t));
    pCurNode->value = illu;
    pCurNode->prob = prob;
    ListPrepareItem(pCurNode);
    ListAddTail(l, pCurNode);
    sizeList = ListNoItems(l);
    if (sizeList > listMaxSize)
    {
        prob_node_t *pDelNode;
        pDelNode = (prob_node_t *)ListRemoveHead(l);
        free(pDelNode);
    }
}


static void StableProbEstimation(List l, int listSize, int count, int illuNum, float* probSet)
{
    int sizeList = ListNoItems(&l);
    if(sizeList < listSize || listSize == 0) {
        return;
    }
    float *prob_tmp = (float*)malloc(illuNum*sizeof(float));
    memset(prob_tmp, 0, illuNum*sizeof(float));
    List *pNextNode = ListHead(&l);
    while (NULL != pNextNode)
    {
        prob_node_t *pL;
        pL = (prob_node_t*)pNextNode;
        prob_tmp[pL->value] += pL->prob;
        pNextNode = pNextNode->p_next;
    }

    for(int i=0; i<illuNum; i++){
        probSet[i] = prob_tmp[i]/(float)count;
        LOGD_ACCM("illu(%d), stable prob = %f \n", i, probSet[i]);
    }
    free(prob_tmp);
    prob_tmp = NULL;
}

static void UpdateDominateIlluList(List *l, int illu, int listMaxSize)
{
    illu_node_t *pCurNode;
    int sizeList;
    if(listMaxSize == 0) {
        return;
    }
    pCurNode = (illu_node_t*)malloc(sizeof(illu_node_t));
    pCurNode->value = illu;
    ListPrepareItem(pCurNode);
    ListAddTail(l, pCurNode);
    sizeList = ListNoItems(l);
    if (sizeList > listMaxSize)
    {
        illu_node_t *pDelNode;
        pDelNode = (illu_node_t *)ListRemoveHead(l);
        free(pDelNode);
    }
}

static void StableIlluEstimation(List l, int listSize, int illuNum, float varianceLuma, float varianceLumaTh, bool awbConverged, int preIllu, int *newIllu)
{
    int sizeList = ListNoItems(&l);
    if(sizeList < listSize || listSize == 0) {
        return;
    }
    /*if( awbConverged) {
        *newIllu = preIllu;
        LOGD_ALSC("awb is converged , reserve the last illumination(%d) \n", preIllu );
        return;
    }*/
    /*if( varianceLuma <= varianceLumaTh) {
        *newIllu = preIllu;
        LOGD_ACCM("varianceLuma %f < varianceLumaTh %f , reserve the last illumination(%d) \n", varianceLuma,varianceLumaTh,preIllu );
        return;
    }*/
    List *pNextNode = ListHead(&l);
    illu_node_t *pL;
    int *illuSet = (int*)malloc(illuNum*sizeof(int));
    memset(illuSet, 0, illuNum*sizeof(int));
    while (NULL != pNextNode)
    {
        pL = (illu_node_t*)pNextNode;
        illuSet[pL->value]++;
        pNextNode = pNextNode->p_next;
    }
    int max_count = 0;
    for(int i=0; i<illuNum; i++){
        LOGV_ACCM("illu(%d), count(%d)\n", i,illuSet[i]);
        if(illuSet[i] > max_count){
            max_count = illuSet[i];
            *newIllu = i;
        }
    }
    free(illuSet);
    LOGV_ACCM("varianceLuma %f, varianceLumaTh %f final estmination illu is %d\n", varianceLuma,varianceLumaTh,*newIllu );
}

XCamReturn AccmAutoConfig
(
    accm_handle_t hAccm
) {

    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const CalibDbV2_Ccm_Accm_Cof_Para_t* pDomIlluProfile = NULL;
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t* pCcmProfile1 = NULL;
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t* pCcmProfile2 = NULL;

    if (hAccm == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V2_t * pCcm = NULL;
    float sensorGain =  hAccm->accmSwInfo.sensorGain;
    float fSaturation;

    pCcm = hAccm->calibV2Ccm;

    if (hAccm->update || hAccm->updateAtt) {
    LOGD_ACCM("Illu Probability Estimation Enable: %d \n", pCcm->TuningPara.illu_estim.interp_enable);
    if (pCcm->TuningPara.illu_estim.interp_enable) {
        //1) estimate illuminant prob
        float probfSaturation = 0;
        memset(hAccm->accmRest.undampedCcmMatrix, 0, sizeof(hAccm->accmRest.undampedCcmMatrix));
        memset(hAccm->accmRest.undampedCcOffset, 0, sizeof(hAccm->accmRest.undampedCcOffset));
        float *prob = (float*)malloc(pCcm->TuningPara.aCcmCof_len*sizeof(float));
        ret = illuminant_index_candidate_ccm(pCcm->TuningPara.aCcmCof_len, pCcm->TuningPara.aCcmCof,
                                          hAccm->accmSwInfo.awbGain, pCcm->TuningPara.illu_estim.default_illu, pCcm->TuningPara.illu_estim.prob_limit, pCcm->TuningPara.illu_estim.weightRB, prob);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

        // calculate stable prob
        int problistsize = pCcm->TuningPara.illu_estim.frame_no*pCcm->TuningPara.aCcmCof_len;
        for (int i = 0; i < pCcm->TuningPara.aCcmCof_len; i++)
            UpdateIlluProbList(&hAccm->accmRest.problist, i, prob[i], problistsize);
        int frames = (int)hAccm->count > (pCcm->TuningPara.illu_estim.frame_no-1) ? pCcm->TuningPara.illu_estim.frame_no : hAccm->count;//todo
        //LOGE_ACCM( "  cashed frame No: %d, ListSize = %d \n", frames, ListNoItems(&hAccm->accmRest.problist));
        LOGD_ACCM( "==================stable Illu Probability=========================\n");
        StableProbEstimation(hAccm->accmRest.problist, problistsize, frames, pCcm->TuningPara.aCcmCof_len, prob);
        LOGD_ACCM( "============================================================\n");

        // 2) all illuminant do interp by fSaturation
        float undampedCcmMatrix[9];
        float undampedCcOffset[3];
        for (int i = 0; i < pCcm->TuningPara.aCcmCof_len; i++){
            if (fabs(prob[i])<DIVMIN)
                continue;
            //     (1) get IlluProfiles of Candidate illuminants, and calculate fSaturation
            pDomIlluProfile = &pCcm->TuningPara.aCcmCof[i];
            interpolation(pDomIlluProfile->gain_sat_curve.gains,
                          pDomIlluProfile->gain_sat_curve.sat,
                          4,
                          sensorGain, &fSaturation);

            //     (2) interp CCM matrix and offset
            ret = SatSelectCcmProfiles(fSaturation, pDomIlluProfile->matrixUsed_len, hAccm->pCcmMatrixAll[i],
                                        &pCcmProfile1, &pCcmProfile2);
            if (ret == XCAM_RETURN_NO_ERROR) {
                XCamReturn ret1 = XCAM_RETURN_NO_ERROR;
                if (pCcmProfile1 && pCcmProfile2)
                    LOGV_ACCM("No.%d Illu : %s interp by sat, final fSaturation: %f (%f .. %f)\n",   i,  pDomIlluProfile->name, fSaturation, pCcmProfile1->saturation, pCcmProfile2->saturation);
                ret = SatInterpolateMatrices(fSaturation, pCcmProfile1, pCcmProfile2, undampedCcmMatrix);
                ret1 = SatInterpolateOffset(fSaturation, pCcmProfile1, pCcmProfile2, undampedCcOffset);
                if ((ret != XCAM_RETURN_NO_ERROR) && (ret1 != XCAM_RETURN_NO_ERROR)) {
                   free(prob);
                   return (ret);
                }
            } else if (ret == XCAM_RETURN_ERROR_OUTOFRANGE) {
                /* we don't need to interpolate */
                LOGV_ACCM("No.%d Illu : %s, final fSaturation: %f (%f)\n",  i,  pDomIlluProfile->name, fSaturation, pCcmProfile1->saturation);
                memcpy(undampedCcmMatrix, pCcmProfile1->ccMatrix, sizeof(float)*9);
                memcpy(undampedCcOffset, pCcmProfile1->ccOffsets, sizeof(float)*3);
            } else {
                free(prob);
                return (ret);
            }
            probfSaturation = probfSaturation + fSaturation*prob[i];

            for (int j = 0; j < 9; j++)
                hAccm->accmRest.undampedCcmMatrix[j] += undampedCcmMatrix[j]*prob[i];
            for (int j = 0; j < 3; j++)
                hAccm->accmRest.undampedCcOffset[j] += undampedCcOffset[j]*prob[i];
        }
        hAccm->accmRest.fSaturation =  probfSaturation;
        hAccm->accmRest.pCcmProfile1 = pCcmProfile1;
        hAccm->accmRest.pCcmProfile2 = pCcmProfile2;

        // 3)
        free(prob);
        prob = NULL;
    }
    else {
        int dominateIlluProfileIdx;
        int dominateIlluListSize = 15;//to do from xml;
        float varianceLumaTh = 0.006;//to do from xml;
        ret = illuminant_index_estimation_ccm(pCcm->TuningPara.aCcmCof_len, pCcm->TuningPara.aCcmCof,
                                          hAccm->accmSwInfo.awbGain, &dominateIlluProfileIdx);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        UpdateDominateIlluList(&hAccm->accmRest.dominateIlluList, dominateIlluProfileIdx, dominateIlluListSize);
        StableIlluEstimation(hAccm->accmRest.dominateIlluList, dominateIlluListSize, pCcm->TuningPara.aCcmCof_len,
            hAccm->accmSwInfo.varianceLuma ,varianceLumaTh, hAccm->accmSwInfo.awbConverged,
            hAccm->accmRest.dominateIlluProfileIdx, &dominateIlluProfileIdx);

        hAccm->accmRest.dominateIlluProfileIdx = dominateIlluProfileIdx;

        // 2)
        pDomIlluProfile = &pCcm->TuningPara.aCcmCof[dominateIlluProfileIdx];
        interpolation(pDomIlluProfile->gain_sat_curve.gains,
                      pDomIlluProfile->gain_sat_curve.sat,
                      4,
                      sensorGain, &fSaturation);

        hAccm->accmRest.fSaturation =  fSaturation;

        //3)
        ret = SatSelectCcmProfiles( hAccm->accmRest.fSaturation, pDomIlluProfile->matrixUsed_len, hAccm->pCcmMatrixAll[dominateIlluProfileIdx],
                                    &pCcmProfile1, &pCcmProfile2);
        if (ret == XCAM_RETURN_NO_ERROR) {
            if (pCcmProfile1 && pCcmProfile2)
                LOGD_ACCM("final fSaturation: %f (%f .. %f)\n",   hAccm->accmRest.fSaturation, pCcmProfile1->saturation, pCcmProfile2->saturation);
            ret = SatInterpolateMatrices( hAccm->accmRest.fSaturation, pCcmProfile1, pCcmProfile2,
                                          hAccm->accmRest.undampedCcmMatrix);
            if (ret != XCAM_RETURN_NO_ERROR) {
                return (ret);
            }
            ret = SatInterpolateOffset( hAccm->accmRest.fSaturation, pCcmProfile1, pCcmProfile2,
                                        hAccm->accmRest.undampedCcOffset);
            if (ret != XCAM_RETURN_NO_ERROR) {
                return (ret);
            }
        } else if (ret == XCAM_RETURN_ERROR_OUTOFRANGE) {
            /* we don't need to interpolate */
            LOGD_ACCM("final fSaturation: %f (%f)\n",   hAccm->accmRest.fSaturation, pCcmProfile1->saturation);
            memcpy(hAccm->accmRest.undampedCcmMatrix, pCcmProfile1->ccMatrix, sizeof(float)*9);
            memcpy(hAccm->accmRest.undampedCcOffset, pCcmProfile1->ccOffsets, sizeof(float)*3);
        } else {
            return (ret);
        }
       hAccm->accmRest.pCcmProfile1 = pCcmProfile1;
       hAccm->accmRest.pCcmProfile2 = pCcmProfile2;
        }

    //4) calc scale for y_alpha_curve
    float fScale = 1.0;
#if 1
    //real use
    interpolation(pCcm->lumaCCM.gain_alphaScale_curve.gain,
                  pCcm->lumaCCM.gain_alphaScale_curve.scale,
                  9,
                  sensorGain, &fScale);
#else
    //for test, to be same with demo
    for( int i = 0; i < 9; i++)
    {
        int j = uint16_t(sensorGain);
        j = (j > (1 << 8)) ? (1 << 8) : j;

        if(j <= (1 << i))
        {
            fScale = pCcm->lumaCCM.gain_alphaScale_curve.scale[i];
            break;
        }
    }
#endif
   // 5) color inhibition adjust for api
    float flevel2;
    interpolation(hAccm->mCurAtt.stAuto.color_inhibition.sensorGain,
                  hAccm->mCurAtt.stAuto.color_inhibition.level,
                  RK_AIQ_ACCM_COLOR_GAIN_NUM,
                  sensorGain, &flevel2);
    hAccm->accmRest.color_inhibition_level = flevel2;
    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        LOGV_ACCM(" color_inhibition sensorGain[%d]: %f, color_inhibition level[%d]: %f,", i,
                  hAccm->mCurAtt.stAuto.color_inhibition.sensorGain[i], i,
                  hAccm->mCurAtt.stAuto.color_inhibition.level[i]);
    }
    if(flevel2 > 100 || flevel2 < 0) {
        LOGE_ACCM("flevel2: %f is invalid ,should be in the range [0 100]\n",  flevel2);
        ret = XCAM_RETURN_ERROR_PARAM;
        return(ret);
    }
    LOGD_ACCM("fScale: %f , color inhibition level:%f, ",  fScale, flevel2);
    fScale *= (100 - flevel2) / 100;
    LOGD_ACCM("final fScale: %f \n", fScale);

   // 6)   saturation adjust for api
    float flevel1;
    interpolation(hAccm->mCurAtt.stAuto.color_saturation.sensorGain,
                  hAccm->mCurAtt.stAuto.color_saturation.level,
                  RK_AIQ_ACCM_COLOR_GAIN_NUM,
                  sensorGain, &flevel1);
    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        LOGV_ACCM("color_saturation sensorGain[%d]: %f, color_saturation level[%d]: %f,", i,
                  hAccm->mCurAtt.stAuto.color_saturation.sensorGain[i], i,
                  hAccm->mCurAtt.stAuto.color_saturation.level[i]);
    }
    if(flevel1 > 100 || flevel1 < 0) {
        LOGE_ACCM("flevel1: %f is invalid ,should be in the range [0 100]\n",  flevel1);
        ret = XCAM_RETURN_ERROR_PARAM;
        return(ret);
    }
    hAccm->accmRest.color_saturation_level = flevel1;
    LOGD_ACCM("fSaturation: %f , color saturation level:%f\n",  fSaturation, flevel1);

    Saturationadjust( fScale,  hAccm );
    for(int i = 0; i < CCM_CURVE_DOT_NUM; i++) { //set to ic  to do bit check
        hAccm->ccmHwConf.alp_y[i] = fScale * pCcm->lumaCCM.y_alpha_curve[i];
    }
    }
    if (!hAccm->accmSwInfo.ccmConverged ||hAccm->update || hAccm->updateAtt) {
      // 7) . Damping
        ret = Damping((pCcm->TuningPara.damp_enable && hAccm->count > 1) ? hAccm->accmSwInfo.awbIIRDampCoef : 0,
                      hAccm->accmRest.undampedCcmMatrix, hAccm->accmRest.dampedCcmMatrix,
                      hAccm->accmRest.undampedCcOffset, hAccm->accmRest.dampedCcOffset);
        float d_ccm = 0;
        float d_ccoff = 0;
        for (int i = 0; i < 9; i++)
            d_ccm += fabs(hAccm->accmRest.undampedCcmMatrix[i] - hAccm->accmRest.dampedCcmMatrix[i]);
        for (int i = 0; i < 3; i++)
            d_ccoff += fabs(hAccm->accmRest.undampedCcOffset[i] - hAccm->accmRest.dampedCcOffset[i]);
        if ((d_ccm < DIVMIN)&&(d_ccoff < DIVMIN))
            hAccm->accmSwInfo.ccmConverged = true;
        else
            hAccm->accmSwInfo.ccmConverged = false;
    }
      // 8)  set to ic  to do bit check
    memcpy(hAccm->ccmHwConf.matrix, hAccm->accmRest.dampedCcmMatrix, sizeof(hAccm->accmRest.dampedCcmMatrix));
    memcpy(hAccm->ccmHwConf.offs, hAccm->accmRest.dampedCcOffset, sizeof(hAccm->accmRest.dampedCcOffset));


    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAccm->ccmHwConf.matrix, hAccm->mCurAtt.stManual.ccMatrix, sizeof(hAccm->mCurAtt.stManual.ccMatrix));
    memcpy(hAccm->ccmHwConf.offs, hAccm->mCurAtt.stManual.ccOffsets, sizeof(hAccm->mCurAtt.stManual.ccOffsets));
    memcpy(hAccm->ccmHwConf.alp_y, hAccm->mCurAtt.stManual.y_alpha_curve, sizeof(hAccm->mCurAtt.stManual.y_alpha_curve));
    hAccm->ccmHwConf.bound_bit = hAccm->mCurAtt.stManual.low_bound_pos_bit;
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn AccmConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (fabs(hAccm->accmRest.res3a_info.sensorGain - hAccm->accmSwInfo.sensorGain) > hAccm->calibV2Ccm->control.gain_tolerance) {
        hAccm->accmRest.res3a_info.gain_stable = false;
        LOGD_ACCM( "%s: update sensorGain:%f \n", __FUNCTION__, hAccm->accmSwInfo.sensorGain);
        hAccm->accmRest.res3a_info.sensorGain = hAccm->accmSwInfo.sensorGain;
    } else {
        hAccm->accmRest.res3a_info.gain_stable = true;
        LOGD_ACCM( "%s: not update sensorGain:%f \n", __FUNCTION__, hAccm->accmSwInfo.sensorGain);
        hAccm->accmSwInfo.sensorGain = hAccm->accmRest.res3a_info.sensorGain;
    }

    if (sqrt( (hAccm->accmRest.res3a_info.awbGain[0]-hAccm->accmSwInfo.awbGain[0])*(hAccm->accmRest.res3a_info.awbGain[0]-hAccm->accmSwInfo.awbGain[0])
         + (hAccm->accmRest.res3a_info.awbGain[1]-hAccm->accmSwInfo.awbGain[1])*(hAccm->accmRest.res3a_info.awbGain[1]-hAccm->accmSwInfo.awbGain[1])) > hAccm->calibV2Ccm->control.wbgain_tolerance) {
        hAccm->accmRest.res3a_info.wbgain_stable = false;
        LOGD_ACCM( "%s: update awbGain:(%f, %f) \n", __FUNCTION__,
            hAccm->accmSwInfo.awbGain[0], hAccm->accmSwInfo.awbGain[1]);
        hAccm->accmRest.res3a_info.awbGain[0] = hAccm->accmSwInfo.awbGain[0];
        hAccm->accmRest.res3a_info.awbGain[1] = hAccm->accmSwInfo.awbGain[1];
    } else {
        hAccm->accmRest.res3a_info.wbgain_stable = true;
        LOGD_ACCM( "%s: not update awbGain:(%f, %f) \n", __FUNCTION__,
            hAccm->accmSwInfo.awbGain[0], hAccm->accmSwInfo.awbGain[1]);
        hAccm->accmSwInfo.awbGain[0] = hAccm->accmRest.res3a_info.awbGain[0];
        hAccm->accmSwInfo.awbGain[1] = hAccm->accmRest.res3a_info.awbGain[1];
    }

    if (hAccm->accmRest.res3a_info.wbgain_stable && hAccm->accmRest.res3a_info.gain_stable
                   && (!hAccm->calib_update))
        hAccm->update = false;
    else
        hAccm->update = true;
    hAccm->calib_update = false;



    LOGD_ACCM("=========================================\n");

    LOGD_ACCM("%s: updateAtt: %d\n", __FUNCTION__, hAccm->updateAtt);
    if(hAccm->updateAtt) {
        hAccm->mCurAtt.mode = hAccm->mNewAtt.mode;
        hAccm->mCurAtt.byPass = hAccm->mNewAtt.byPass;

        if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO)
            hAccm->mCurAtt.stAuto = hAccm->mNewAtt.stAuto;
        else
            hAccm->mCurAtt.stManual = hAccm->mNewAtt.stManual;
    }

    LOGD_ACCM("%s: CCM update: %d\n", __FUNCTION__, hAccm->update);
    LOGD_ACCM("%s: CCM Converged: %d\n", __FUNCTION__, hAccm->accmSwInfo.ccmConverged);

    if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO){
         hAccm->mCurAtt.byPass = !(hAccm->calibV2Ccm->control.enable);
    }
    LOGD_ACCM("%s: byPass: %d  mode:%d \n", __FUNCTION__, hAccm->mCurAtt.byPass, hAccm->mCurAtt.mode);
    if(hAccm->mCurAtt.byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf.ccmEnable = true;

        if(hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO){
            if (hAccm->updateAtt || hAccm->update ||(!hAccm->accmSwInfo.ccmConverged))
                AccmAutoConfig(hAccm);
        } else if(hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_MANUAL) {
             if (hAccm->updateAtt || hAccm->update)
                AccmManualConfig(hAccm);
        }
        else {
            LOGE_ACCM("%s: hAccm->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAccm->mCurAtt.mode);
        }
        memcpy(hAccm->mCurAtt.stManual.ccMatrix, hAccm->ccmHwConf.matrix, sizeof(hAccm->ccmHwConf.matrix));
        memcpy(hAccm->mCurAtt.stManual.ccOffsets, hAccm->ccmHwConf.offs, sizeof(hAccm->ccmHwConf.offs));
        memcpy(hAccm->mCurAtt.stManual.y_alpha_curve, hAccm->ccmHwConf.alp_y, sizeof(hAccm->ccmHwConf.alp_y));

    } else {
        hAccm->ccmHwConf.ccmEnable = false;
    }
    hAccm->updateAtt = false;

    LOGV_ACCM( " set to ic ccmEnable :%d  bound_bit:%f\n", hAccm->ccmHwConf.ccmEnable, hAccm->ccmHwConf.bound_bit);

    LOGD_ACCM( " set to ic CcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f  CcOffset:%f,%f,%f  \n",
               hAccm->ccmHwConf.matrix[0],
               hAccm->ccmHwConf.matrix[1],
               hAccm->ccmHwConf.matrix[2],
               hAccm->ccmHwConf.matrix[3],
               hAccm->ccmHwConf.matrix[4],
               hAccm->ccmHwConf.matrix[5],
               hAccm->ccmHwConf.matrix[6],
               hAccm->ccmHwConf.matrix[7],
               hAccm->ccmHwConf.matrix[8],
               hAccm->ccmHwConf.offs[0],
               hAccm->ccmHwConf.offs[1],
               hAccm->ccmHwConf.offs[2]
             );

    LOGV_ACCM( " set to ic rgb2y_para: %f,%f,%f\n",
               hAccm->ccmHwConf.rgb2y_para[0],
               hAccm->ccmHwConf.rgb2y_para[1],
               hAccm->ccmHwConf.rgb2y_para[2]

             );

    LOGV_ACCM( " set to ic alp_y: %f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f  \n",
               hAccm->ccmHwConf.alp_y[0],
               hAccm->ccmHwConf.alp_y[1],
               hAccm->ccmHwConf.alp_y[2],
               hAccm->ccmHwConf.alp_y[3],
               hAccm->ccmHwConf.alp_y[4],
               hAccm->ccmHwConf.alp_y[5],
               hAccm->ccmHwConf.alp_y[6],
               hAccm->ccmHwConf.alp_y[7],
               hAccm->ccmHwConf.alp_y[8],
               hAccm->ccmHwConf.alp_y[9],
               hAccm->ccmHwConf.alp_y[10],
               hAccm->ccmHwConf.alp_y[11],
               hAccm->ccmHwConf.alp_y[12],
               hAccm->ccmHwConf.alp_y[13],
               hAccm->ccmHwConf.alp_y[14],
               hAccm->ccmHwConf.alp_y[15],
               hAccm->ccmHwConf.alp_y[16]
             );


    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);
    LOGD_ACCM( "===================================================== \n");
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;

}

/**************************************************
  * ReloadCCMCalibV2
  *      config stTool used new CalibV2 json para
***************************************************/

static XCamReturn ReloadCCMCalibV2(accm_handle_t hAccm, const CalibDbV2_Ccm_Para_V2_t *calibv2_ccm)
{
    CalibDbV2_Ccm_Para_V2_t *stCcm = &hAccm->mCurAtt.stTool;
    if (stCcm == NULL){
        LOGE_ACCM("%s: CCM stTool is NULL !!!", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    int clearillu = 0;
    if (calibv2_ccm->TuningPara.aCcmCof_len != stCcm->TuningPara.aCcmCof_len)
        clearillu = 1;
    else {
        int findillu = 0;
        for (int i = 0; i < stCcm->TuningPara.aCcmCof_len; i++){
            for (int j = 0; j < stCcm->TuningPara.aCcmCof_len; j++){
                if (strcmp(stCcm->TuningPara.aCcmCof[i].name, calibv2_ccm->TuningPara.aCcmCof[i].name) == 0){
                    findillu = 1;
                    if (! (stCcm->TuningPara.aCcmCof[i].awbGain[0] == calibv2_ccm->TuningPara.aCcmCof[i].awbGain[0])&&( stCcm->TuningPara.aCcmCof[i].awbGain[1] == calibv2_ccm->TuningPara.aCcmCof[i].awbGain[1])){
                        clearillu = 1;
                        LOGI_ACCM( "%s: awbGain in aCcmCof has been changed. \n", calibv2_ccm->TuningPara.aCcmCof[i].name);
                        break;
                    }
                }
            }
            if (findillu == 0) clearillu = 1;
            if (clearillu == 1) break;
        }
    }
    if (clearillu == 1)
        ClearList(&hAccm->accmRest.dominateIlluList);
    hAccm->mCurAtt.stTool = *calibv2_ccm;
    return (XCAM_RETURN_NO_ERROR);
}

/**********************************
*Update CCM CalibV2 Para
*      Prepare init
*      Mode change: reinit
*      Res change: continue
*      Calib change: continue
***************************************/
static XCamReturn UpdateCcmCalibV2Para(accm_handle_t hAccm)
{
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    bool config_calib = !!(hAccm->accmSwInfo.prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    const CalibDbV2_Ccm_Para_V2_t* calib_ccm = hAccm->calibV2Ccm;

    if (!config_calib)
    {
        return(ret);
    }

    hAccm->mCurAtt.mode = (rk_aiq_ccm_op_mode_t)calib_ccm->control.mode;

    ReloadCCMCalibV2(hAccm, calib_ccm);

    ret = pCcmMatrixAll_init(hAccm, &calib_ccm->TuningPara);

    hAccm->mCurAtt.byPass = !(calib_ccm->control.enable);

    hAccm->ccmHwConf.bound_bit = calib_ccm->lumaCCM.low_bound_pos_bit;
    memcpy( hAccm->ccmHwConf.rgb2y_para, calib_ccm->lumaCCM.rgb2y_para,
            sizeof(calib_ccm->lumaCCM.rgb2y_para));
    memcpy( hAccm->ccmHwConf.alp_y, calib_ccm->lumaCCM.y_alpha_curve, sizeof(hAccm->ccmHwConf.alp_y));

    // config manual ccm
    memcpy(hAccm->mCurAtt.stManual.ccMatrix, calib_ccm->manualPara.ccMatrix, sizeof(calib_ccm->manualPara.ccMatrix));
    memcpy(hAccm->mCurAtt.stManual.ccOffsets, calib_ccm->manualPara.ccOffsets, sizeof(calib_ccm->manualPara.ccOffsets));
    memcpy(hAccm->mCurAtt.stManual.y_alpha_curve,  hAccm->ccmHwConf.alp_y, sizeof( hAccm->ccmHwConf.alp_y));
    hAccm->mCurAtt.stManual.low_bound_pos_bit = hAccm->ccmHwConf.bound_bit;
    hAccm->accmSwInfo.ccmConverged = false;
    hAccm->calib_update = true;

#if 0 //awbGain get from awb module
    ret = Swinfo_wbgain_init(hAccm->accmSwInfo.awbGain, &hAccm->mCurAtt.stTool.mode_cell[currentHdrNormalMode], hAccm->lsForFirstFrame);
#endif
    ClearList(&hAccm->accmRest.problist);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}

XCamReturn AccmInit(accm_handle_t *hAccm, const CamCalibDbV2Context_t* calibv2)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    *hAccm = (accm_context_t*)malloc(sizeof(accm_context_t));
    accm_context_t* accm_context = *hAccm;
    memset(accm_context, 0, sizeof(accm_context_t));

    if(calibv2 == NULL) {
        return  XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V2_t *calib_ccm =
        (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib));
    if (calib_ccm == NULL)
        return XCAM_RETURN_ERROR_MEM;

    accm_context->accmSwInfo.sensorGain = 1.0;
    accm_context->accmSwInfo.awbIIRDampCoef = 0;
    accm_context->accmSwInfo.varianceLuma = 255;
    accm_context->accmSwInfo.awbConverged = false;
    accm_context->accmSwInfo.awbGain[0] = 1;
    accm_context->accmSwInfo.awbGain[1] = 1;

    accm_context->accmRest.res3a_info.sensorGain = 1.0;
    accm_context->accmRest.res3a_info.awbGain[0] = 1.0;
    accm_context->accmRest.res3a_info.awbGain[1] = 1.0;
    accm_context->accmRest.res3a_info.wbgain_stable = false;
    accm_context->accmRest.res3a_info.gain_stable = false;

    accm_context->count = 0;

    accm_context->accmSwInfo.prepare_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para_V2
    accm_context->calibV2Ccm = calib_ccm;
    ret = UpdateCcmCalibV2Para(accm_context);

    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
            accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[i] = 1;
            accm_context->mCurAtt.stAuto.color_inhibition.level[i] = 0;
            accm_context->mCurAtt.stAuto.color_saturation.sensorGain[i] = 1;
            accm_context->mCurAtt.stAuto.color_saturation.level[i] = 50;
        }
    accm_context->accmRest.color_inhibition_level = 0;
    accm_context->accmRest.color_saturation_level = 100;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}

XCamReturn AccmRelease(accm_handle_t hAccm)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ClearList(&hAccm->accmRest.dominateIlluList);
    ClearList(&hAccm->accmRest.problist);
    free(hAccm);
    hAccm = NULL;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);

}

// todo whm
XCamReturn AccmPrepare(accm_handle_t hAccm)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = UpdateCcmCalibV2Para(hAccm);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}


XCamReturn AccmPreProc(accm_handle_t hAccm)
{

    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);

}
XCamReturn AccmProcessing(accm_handle_t hAccm)
{
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}




RKAIQ_END_DECLARE



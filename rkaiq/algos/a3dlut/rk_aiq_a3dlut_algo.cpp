/*
* alut3d.cpp

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

#include "a3dlut/rk_aiq_a3dlut_algo.h"
#include "xcam_log.h"
#include "interpolation.h"

RKAIQ_BEGIN_DECLARE


XCamReturn lut3d_index_estimation(int lut_num, const CalibDbV2_Lut3D_LutPara_t lutAll[], float awbGain[2], int* index)
{

    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    float minDist = 9999999;
    float *dist = (float*)malloc(lut_num*sizeof(float));
    float nRG, nBG;
    nRG = awbGain[0];
    nBG = awbGain[1];
    *index = 0;
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    for(int i = 0; i < lut_num; i++)
    {
        dist[i] = (nRG - lutAll[i].awbGain[0]) * (nRG - lutAll[i].awbGain[0]) +
                  (nBG - lutAll[i].awbGain[1]) * (nBG - lutAll[i].awbGain[1]);
        if(dist[i] < minDist)
        {
            minDist = dist[i];
            *index = i;
            ret = XCAM_RETURN_NO_ERROR;
        }
    }
    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_A3DLUT("fail to estimate idx!!!\n");
    }

    LOGD_A3DLUT("wbGain:%f,%f, estimation lut is %s\n", awbGain[0], awbGain[1],
                lutAll[*index].name);

    if (dist)
        free(dist);

    LOG1_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return ret;
}

#if RKAIQ_A3DLUT_ILLU_VOTE
static void UpdateDominateIdxList(struct list_head *l, int idx, int listMaxSize)
{
    idx_node_t *pCurNode;
    if(listMaxSize == 0) {
        return;
    }
    int sizeList = get_list_num(l);
    if (sizeList < listMaxSize) {
        pCurNode = (idx_node_t*)malloc(sizeof(idx_node_t));
        pCurNode->value = idx;
        list_prepare_item(&pCurNode->p_next);
        list_add_tail((struct list_head*)(&pCurNode->p_next), l);
    } else {
        idx_node_t *pDelNode = (idx_node_t *)(l->next);
        pDelNode->value = idx;
        struct list_head* node0 = l->next;
        list_swap_item(l, node0);
    }
}

static void StableIdxEstimation(struct list_head *l, int listSize, int Num, int *newIdx)
{
    int sizeList = get_list_num(l);
    if(sizeList < listSize || listSize == 0) {
        return;
    }

    struct list_head *pNextNode = l->next;
    idx_node_t *pL;
    int *Set = (int*)malloc(Num*sizeof(int));
    memset(Set, 0, Num*sizeof(int));
    while (NULL != pNextNode)
    {
        pL = (idx_node_t*)pNextNode;
        Set[pL->value]++;
        pNextNode = pNextNode->next;
    }
    int max_count = 0;
    for(int i=0; i<Num; i++){
        LOGV_A3DLUT("Lut (%d), count(%d)\n", i, Set[i]);
        if(Set[i] > max_count){
            max_count = Set[i];
            *newIdx = i;
        }
    }
    free(Set);
}
#endif

/******************************************************************************
 * Damping
 *****************************************************************************/
static XCamReturn Damping(const float damp,                        /**< damping coefficient */
                          const rk_aiq_lut3d_hw_tbl_t* pUndamped, /**< undamped new computed lut */
                          rk_aiq_lut3d_hw_tbl_t* pDamped,    /**< old lut and result */
                          int* lutSum
) {
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ( (pUndamped != NULL) && (pDamped != NULL) )
    {
        const float f = 1 - damp;
        memset(lutSum, 0, 3*sizeof(int));

        /* calc. damped lut */
        for (int i = 0; i < LUT3D_LUT_WSIZE; i++) {
            pDamped->look_up_table_r[i] =
                (uint16_t) (damp * (float) pDamped->look_up_table_r[i] + f * (float) pUndamped->look_up_table_r[i]);
            pDamped->look_up_table_g[i] =
                (uint16_t) (damp * (float) pDamped->look_up_table_g[i] + f * (float) pUndamped->look_up_table_g[i]);
            pDamped->look_up_table_b[i] =
                (uint16_t) (damp * (float) pDamped->look_up_table_b[i] + f * (float) pUndamped->look_up_table_b[i]);
            lutSum[0] += pDamped->look_up_table_r[i];
            lutSum[1] += pDamped->look_up_table_g[i];
            lutSum[2] += pDamped->look_up_table_b[i];
        }

        result = XCAM_RETURN_NO_ERROR;
    }
    LOGD_A3DLUT( "dampfactor:%f \n", damp);
    return ( result );
}

/******************************************************************************
 * interpolation lut table
 *****************************************************************************/
static XCamReturn InterpLutbyAlp(const float alp,
                        const rk_aiq_lut3d_hw_tbl_t* pLut0,
                        const rk_aiq_lut3d_hw_tbl_t* pLutA,
                        rk_aiq_lut3d_hw_tbl_t* pLutB
) {
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ((pLutA != NULL) && (pLutB != NULL) && (pLut0 != NULL))
    {
        uint32_t alpha = uint32_t(alp * 128.0f);

        if (alpha == 0) {
            *pLutB = *pLut0;
        } else if (alpha == 128) {
            *pLutB = (rk_aiq_lut3d_hw_tbl_t) *pLutA;
        } else {
            LOGD_A3DLUT("begin lut interp by alp\n");
            const uint32_t beta = 128 - alpha;

            for (uint32_t i = 0; i < LUT3D_LUT_WSIZE; i++) {
                pLutB->look_up_table_r[i] = (uint16_t)((alpha * (uint32_t)pLutA->look_up_table_r[i] + beta * (uint32_t)pLut0->look_up_table_r[i])>>7);
                pLutB->look_up_table_g[i] = (uint16_t)((alpha * (uint32_t)pLutA->look_up_table_g[i] + beta * (uint32_t)pLut0->look_up_table_g[i])>>7);
                pLutB->look_up_table_b[i] = (uint16_t)((alpha * (uint32_t)pLutA->look_up_table_b[i] + beta * (uint32_t)pLut0->look_up_table_b[i])>>7);
            }
        }

        result = XCAM_RETURN_NO_ERROR;
    }
    return ( result );
}

/******************************************************************************
 * interpolation lut table by alp and damp
 *****************************************************************************/
static XCamReturn InterpLutbyAlpandDamp(const float alp,
                        const rk_aiq_lut3d_hw_tbl_t* pLut0,
                        const rk_aiq_lut3d_hw_tbl_t* pLutA,
                        rk_aiq_lut3d_hw_tbl_t* pLutB,
                        const float damp,
                        int* lutSum

) {
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ((pLutA != NULL) && (pLutB != NULL) && (pLut0 != NULL))
    {
        if (damp > 0.0) {
            if (alp < DIVMIN) {
                result = Damping(damp, pLut0, pLutB, lutSum);
            } else if (fabs(alp - 1) < DIVMIN) {
                result = Damping(damp, pLutA, pLutB, lutSum);
            } else {
                const float beta = damp * alp;
                const float gamma = damp * (1 - alp);
                const float f = 1 - damp;
                memset(lutSum, 0, 3*sizeof(int));

                /* calc. damped lut */
                for (uint32_t i = 0; i < LUT3D_LUT_WSIZE; i++) {
                    pLutB->look_up_table_r[i] = (uint16_t) (beta * (float) pLutA->look_up_table_r[i] +
                            gamma * (float) pLut0->look_up_table_r[i] +
                            f * (float) pLutB->look_up_table_r[i]);
                    pLutB->look_up_table_g[i] = (uint16_t) (beta * (float) pLutA->look_up_table_g[i] +
                            gamma * (float) pLut0->look_up_table_g[i] +
                            f * (float) pLutB->look_up_table_g[i]);
                    pLutB->look_up_table_b[i] = (uint16_t) (beta * (float) pLutA->look_up_table_b[i] +
                            gamma * (float) pLut0->look_up_table_b[i] +
                            f * (float) pLutB->look_up_table_b[i]);
                    lutSum[0] += pLutB->look_up_table_r[i];
                    lutSum[1] += pLutB->look_up_table_g[i];
                    lutSum[2] += pLutB->look_up_table_b[i];
                }
                result = XCAM_RETURN_NO_ERROR;
            }
        }
    }
    return ( result );
}

XCamReturn Alut3dAutoConfig
(
    alut3d_handle_t hAlut3d,
    bool forceRun
) {

    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!hAlut3d)
        return XCAM_RETURN_ERROR_PARAM;

    const CalibDbV2_Lut3D_LutPara_t* pLutProfile = NULL;
    float sensorGain =  hAlut3d->swinfo.sensorGain;
    if (forceRun || hAlut3d->updateAtt) {
        //(1) estimate idx
        int dominateProfileIdx;
        ret = lut3d_index_estimation(hAlut3d->calibV2_lut3d->ALut3D.lutAll_len, hAlut3d->calibV2_lut3d->ALut3D.lutAll, hAlut3d->swinfo.awbGain, &dominateProfileIdx);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

#if RKAIQ_A3DLUT_ILLU_VOTE
        int dominateListSize = 0;
        dominateListSize = hAlut3d->calibV2_lut3d->ALut3D.lutAll_len*2+1;
        UpdateDominateIdxList(&hAlut3d->restinfo.dominateIdxList, dominateProfileIdx, dominateListSize);
        StableIdxEstimation(&hAlut3d->restinfo.dominateIdxList, dominateListSize, hAlut3d->calibV2_lut3d->ALut3D.lutAll_len, &dominateProfileIdx);
#endif
        //(2) interpolate alpha
        pLutProfile = &hAlut3d->calibV2_lut3d->ALut3D.lutAll[dominateProfileIdx];
        hAlut3d->restinfo.pLutProfile = &hAlut3d->calibV2_lut3d->ALut3D.lutAll[dominateProfileIdx];

        float alp_tmp = 0;
        interpolation(pLutProfile->gain_alpha.gain, pLutProfile->gain_alpha.alpha, 9, sensorGain,
                      &alp_tmp);

        //(3) lut = alpha*lutfile + (1-alpha)*lut0
        uint32_t alpha = uint32_t(alp_tmp * 128.0f);
        hAlut3d->update = hAlut3d->calib_update ||
                            (dominateProfileIdx != hAlut3d->restinfo.dominateIdx) ||
                            (alpha != uint32_t(hAlut3d->restinfo.alpha * 128.0f));

        LOGD_A3DLUT("update:%d, sensorGain: %f, Alpha: %f->%f, LutIdx: %d->%d \n",
                            hAlut3d->update, sensorGain,
                            hAlut3d->restinfo.alpha, alp_tmp,
                            hAlut3d->restinfo.dominateIdx, dominateProfileIdx);
        if (hAlut3d->update) {
            if (alpha == 0)
                hAlut3d->restinfo.alpha = 0;
            else if (alpha > 128)
                hAlut3d->restinfo.alpha = 1;
            else
                hAlut3d->restinfo.alpha = alp_tmp;
            hAlut3d->restinfo.dominateIdx = dominateProfileIdx;
        }

    }

        //(4) damp
    if ((!hAlut3d->calibV2_lut3d->ALut3D.damp_en) && (hAlut3d->update)) { // dampen = 0 && (alp changed || lut changed)
        hAlut3d->swinfo.lut3dConverged = true;
        ret = InterpLutbyAlp(hAlut3d->restinfo.alpha, &hAlut3d->lut0,
                        (const rk_aiq_lut3d_hw_tbl_t *)(&hAlut3d->restinfo.pLutProfile->Table),
                        &hAlut3d->lut3d_hw_conf.tbl);
    } else if (hAlut3d->calibV2_lut3d->ALut3D.damp_en && ((hAlut3d->swinfo.count <= 1) || (hAlut3d->swinfo.invarMode == 0))) { // first frame or attr mode changed
        hAlut3d->swinfo.lut3dConverged = true;
        ret = InterpLutbyAlp(hAlut3d->restinfo.alpha, &hAlut3d->lut0,
                        (const rk_aiq_lut3d_hw_tbl_t *)(&hAlut3d->restinfo.pLutProfile->Table),
                        &hAlut3d->lut3d_hw_conf.tbl);

    } else if (hAlut3d->calibV2_lut3d->ALut3D.damp_en && (hAlut3d->swinfo.awbIIRDampCoef > 0.0) && ((!hAlut3d->swinfo.lut3dConverged) || hAlut3d->update) ) {
        ret = InterpLutbyAlpandDamp(hAlut3d->restinfo.alpha, &hAlut3d->lut0,
                        (const rk_aiq_lut3d_hw_tbl_t *)(& hAlut3d->restinfo.pLutProfile->Table),
                        &hAlut3d->lut3d_hw_conf.tbl,
                        hAlut3d->swinfo.awbIIRDampCoef,
                        hAlut3d->restinfo.lutSum);
        hAlut3d->swinfo.lut3dConverged = !(abs(hAlut3d->restinfo.lutSum[0] - hAlut3d->restinfo.lutSum_last[0]) > 0 ||
                                           abs(hAlut3d->restinfo.lutSum[1] - hAlut3d->restinfo.lutSum_last[1]) > 0 ||
                                           abs(hAlut3d->restinfo.lutSum[2] - hAlut3d->restinfo.lutSum_last[2]) > 0);

        memcpy(hAlut3d->restinfo.lutSum_last, hAlut3d->restinfo.lutSum, sizeof(int)*3);

        LOGD_A3DLUT("DampCoef = %f, damp lutB[7] = %d, lut converge: %d, count = %d\n", hAlut3d->swinfo.awbIIRDampCoef, hAlut3d->lut3d_hw_conf.tbl.look_up_table_b[7],
                    hAlut3d->swinfo.lut3dConverged, hAlut3d->swinfo.count);

    } else {
        hAlut3d->swinfo.lut3dConverged = true;
    }

    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn Alut3dManualConfig
(
    alut3d_handle_t hAlut3d
) {

    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_r, hAlut3d->mCurAtt.stManual.look_up_table_r, sizeof(unsigned short)*LUT3D_LUT_WSIZE);
    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_r, hAlut3d->mCurAtt.stManual.look_up_table_r, sizeof(unsigned short)*LUT3D_LUT_WSIZE);
    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_r, hAlut3d->mCurAtt.stManual.look_up_table_r, sizeof(unsigned short)*LUT3D_LUT_WSIZE);

    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Alut3dConfig
(
    alut3d_handle_t hAlut3d
) {

    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hAlut3d == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_A3DLUT("%s: updateAtt: %d\n", __FUNCTION__, hAlut3d->updateAtt);
#if 0
    if(hAlut3d->updateAtt) {
        hAlut3d->swinfo.invarMode = 1;
        //hAlut3d->mCurAtt = hAlut3d->mNewAtt;
    }
#endif

    LOGD_A3DLUT("%s: byPass: %d  mode:%d \n", __FUNCTION__, hAlut3d->mCurAtt.byPass, hAlut3d->mCurAtt.mode);
    if(hAlut3d->mCurAtt.byPass != true) {
        hAlut3d->lut3d_hw_conf.enable = true;
        hAlut3d->lut3d_hw_conf.bypass_en = false;

        if(hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_AUTO) {
            bool update = false;
            if (fabs(hAlut3d->restinfo.res3a_info.sensorGain - hAlut3d->swinfo.sensorGain) > hAlut3d->calibV2_lut3d->common.gain_tolerance) {
                hAlut3d->restinfo.res3a_info.gain_stable = false;
                LOGD_A3DLUT( "%s: update sensorGain:%f \n", __FUNCTION__, hAlut3d->swinfo.sensorGain);
                hAlut3d->restinfo.res3a_info.sensorGain = hAlut3d->swinfo.sensorGain;
            } else {
                hAlut3d->restinfo.res3a_info.gain_stable = true;
                LOGD_A3DLUT( "%s: not update sensorGain:%f \n", __FUNCTION__, hAlut3d->swinfo.sensorGain);
                hAlut3d->swinfo.sensorGain = hAlut3d->restinfo.res3a_info.sensorGain;
            }

            LOGD_A3DLUT("awbConverged: %d, calib_update: %d\n", hAlut3d->swinfo.awbConverged, hAlut3d->calib_update);

            if (hAlut3d->swinfo.awbConverged &&
                hAlut3d->restinfo.res3a_info.gain_stable &&
                (!hAlut3d->calib_update)) {
                update = false;
                hAlut3d->update = false;
            } else
                update = true;
            if (update || hAlut3d->updateAtt || (!hAlut3d->swinfo.lut3dConverged))
                Alut3dAutoConfig(hAlut3d, update);
        } else if(hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_MANUAL) {
            if (hAlut3d->updateAtt)
                Alut3dManualConfig(hAlut3d);
        } else {
            LOGE_A3DLUT("%s: hAlut3d->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAlut3d->mCurAtt.mode);
        }

        LOGD_A3DLUT("final lutB[7] = %d\n", hAlut3d->lut3d_hw_conf.tbl.look_up_table_b[7]);
#if 0
        memcpy(hAlut3d->mCurAtt.stManual.look_up_table_r, hAlut3d->lut3d_hw_conf.look_up_table_r,
               sizeof(hAlut3d->mCurAtt.stManual.look_up_table_r));
        memcpy( hAlut3d->mCurAtt.stManual.look_up_table_g, hAlut3d->lut3d_hw_conf.look_up_table_g,
                sizeof(hAlut3d->mCurAtt.stManual.look_up_table_g));
        memcpy(hAlut3d->mCurAtt.stManual.look_up_table_b, hAlut3d->lut3d_hw_conf.look_up_table_b,
               sizeof(hAlut3d->mCurAtt.stManual.look_up_table_b));
#endif
    } else {
        hAlut3d->lut3d_hw_conf.enable = false;
        hAlut3d->lut3d_hw_conf.bypass_en = true;
    }
    //hAlut3d->swinfo.invarMode = 0;

    LOGD_A3DLUT("%s: enable:(%d),bypass_en(%d) \n", __FUNCTION__,
                hAlut3d->lut3d_hw_conf.enable,
                hAlut3d->lut3d_hw_conf.bypass_en);
    hAlut3d->swinfo.count = ((hAlut3d->swinfo.count + 2) > (65536)) ? 2 : (hAlut3d->swinfo.count + 1);
    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

static XCamReturn UpdateLut3dCalibPara(alut3d_handle_t  hAlut3d)
{
    LOGI_A3DLUT("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool config_calib = !!(hAlut3d->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    if(!config_calib)
    {
        return(ret);
    }

    hAlut3d->lut3d_hw_conf.lut3d_lut_wsize = LUT3D_LUT_WSIZE;
    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_r, hAlut3d->calib_lut3d->look_up_table_r,
           sizeof(hAlut3d->calib_lut3d->look_up_table_r));
    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_g, hAlut3d->calib_lut3d->look_up_table_g,
           sizeof(hAlut3d->calib_lut3d->look_up_table_g));
    memcpy(hAlut3d->lut3d_hw_conf.tbl.look_up_table_b, hAlut3d->calib_lut3d->look_up_table_b,
           sizeof(hAlut3d->calib_lut3d->look_up_table_b));

    hAlut3d->mCurAtt.byPass = !(hAlut3d->calib_lut3d->enable);
    LOGI_A3DLUT("%s: (exit)  \n", __FUNCTION__);
    return(ret);
}

static XCamReturn UpdateLut3dCalibV2Para(alut3d_handle_t  hAlut3d)
{
    LOGI_A3DLUT("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool config_calib = !!(hAlut3d->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    if(!config_calib)
    {
        return(ret);
    }

    // hAlut3d->swinfo.invarMode = 1/*hAlut3d->mCurAtt.mode & hAlut3d->mNewAtt.mode*/;
    if (hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_AUTO) {
        hAlut3d->mCurAtt.byPass = !(hAlut3d->calibV2_lut3d->common.enable);
    }

    hAlut3d->swinfo.lut3dConverged = false;
    hAlut3d->calib_update = true;

    hAlut3d->lut3d_hw_conf.lut3d_lut_wsize = LUT3D_LUT_WSIZE;

#if RKAIQ_A3DLUT_ILLU_VOTE
    clear_list(&hAlut3d->restinfo.dominateIdxList);
#endif

    LOGI_A3DLUT("%s: (exit)  \n", __FUNCTION__);
    return(ret);
}

XCamReturn Alut3dInit(alut3d_handle_t *hAlut3d, const CamCalibDbV2Context_t* calibv2)
{
    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(calibv2 == NULL) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    const CalibDbV2_Lut3D_Para_V2_t *calib_lut3d =
        (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, lut3d_calib));
    if (calib_lut3d == NULL)
        return XCAM_RETURN_ERROR_MEM;

    *hAlut3d = (alut3d_context_t*)malloc(sizeof(alut3d_context_t));
    if (!*hAlut3d) return XCAM_RETURN_ERROR_MEM;

    alut3d_context_t* alut3d_contex = *hAlut3d;
    memset(alut3d_contex, 0, sizeof(alut3d_context_t));
    // initial lut 0
    uint16_t lut_idx        = 0;
    for (uint16_t i = 0; i < LUT3D_LUT_GRID_NUM; i++) {
        for (uint16_t j = 0; j < LUT3D_LUT_GRID_NUM; j++) {
            for (uint16_t k = 0; k < LUT3D_LUT_GRID_NUM; k++) {
                lut_idx                                      = i * LUT3D_LUT_GRID_NUM * LUT3D_LUT_GRID_NUM + j * LUT3D_LUT_GRID_NUM + k;
                alut3d_contex->lut0.look_up_table_r[lut_idx] = (k << 7) - (k >> 3);
                alut3d_contex->lut0.look_up_table_g[lut_idx] = (j << 9) - (j >> 3);
                alut3d_contex->lut0.look_up_table_b[lut_idx] = (i << 7) - (i >> 3);
            }
        }
    }

    alut3d_contex->swinfo.sensorGain = 1.0;
    alut3d_contex->swinfo.awbIIRDampCoef = 0;
    alut3d_contex->swinfo.awbConverged = false;
    alut3d_contex->swinfo.awbGain[0] = 1;
    alut3d_contex->swinfo.awbGain[1] = 1;
    alut3d_contex->swinfo.count = 0;
    alut3d_contex->swinfo.invarMode = 1; // 0: mode change, 1: mode unchange
    alut3d_contex->swinfo.lut3dConverged = false;

    alut3d_contex->restinfo.res3a_info.sensorGain = 1.0;
    alut3d_contex->restinfo.res3a_info.gain_stable = false;
    alut3d_contex->restinfo.alpha = 0.0;
    alut3d_contex->restinfo.lutSum[0] = 0;
    alut3d_contex->restinfo.lutSum[1] = 0;
    alut3d_contex->restinfo.lutSum[2] = 0;
    alut3d_contex->restinfo.lutSum_last[0] = 0;
    alut3d_contex->restinfo.lutSum_last[1] = 0;
    alut3d_contex->restinfo.lutSum_last[2] = 0;
#if RKAIQ_A3DLUT_ILLU_VOTE
    INIT_LIST_HEAD(&alut3d_contex->restinfo.dominateIdxList);
#endif

    alut3d_contex->calibV2_lut3d = calib_lut3d;
    alut3d_contex->mCurAtt.mode = RK_AIQ_LUT3D_MODE_AUTO;
    alut3d_contex->prepare_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;
    ret = UpdateLut3dCalibV2Para(alut3d_contex);
    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return(ret);


}

XCamReturn Alut3dRelease(alut3d_handle_t hAlut3d)
{
    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_A3DLUT_ILLU_VOTE
    clear_list(&hAlut3d->restinfo.dominateIdxList);
#endif

    free(hAlut3d);

    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return(ret);

}

XCamReturn Alut3dPrepare(alut3d_handle_t hAlut3d)
{
    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //ret = UpdateLut3dCalibPara(hAlut3d);
     ret = UpdateLut3dCalibV2Para(hAlut3d);
    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return ret;
}
XCamReturn Alut3dPreProc(alut3d_handle_t hAlut3d)
{

    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return(ret);

}
XCamReturn Alut3dProcessing(alut3d_handle_t hAlut3d)
{
    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);
    return(ret);
}



RKAIQ_END_DECLARE



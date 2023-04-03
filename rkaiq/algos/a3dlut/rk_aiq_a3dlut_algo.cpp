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
static void UpdateDominateIdxList(List *l, int idx, int listMaxSize)
{
    idx_node_t *pCurNode;
    int sizeList;
    if(listMaxSize == 0) {
        return;
    }
    pCurNode = (idx_node_t*)malloc(sizeof(idx_node_t));
    pCurNode->value = idx;
    ListPrepareItem(pCurNode);
    ListAddTail(l, pCurNode);
    sizeList = ListNoItems(l);
    if (sizeList > listMaxSize)
    {
        idx_node_t *pDelNode;
        pDelNode = (idx_node_t *)ListRemoveHead(l);
        free(pDelNode);
    }
}

static void StableIdxEstimation(List l, int listSize, int Num, int *newIdx)
{
    int sizeList = ListNoItems(&l);
    if(sizeList < listSize || listSize == 0) {
        return;
    }

    List *pNextNode = ListHead(&l);
    idx_node_t *pL;
    int *Set = (int*)malloc(Num*sizeof(int));
    memset(Set, 0, Num*sizeof(int));
    while (NULL != pNextNode)
    {
        pL = (idx_node_t*)pNextNode;
        Set[pL->value]++;
        pNextNode = pNextNode->p_next;
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
                          rk_aiq_lut3d_mlut3d_attrib_t* pUndamped, /**< undamped new computed lut */
                          rk_aiq_lut3d_cfg_t* pDamped    /**< old lut and result */
) {
    XCamReturn result = XCAM_RETURN_ERROR_PARAM;

    if ( (pUndamped != NULL) && (pDamped != NULL) )
    {
        const float f = 1 - damp;

        /* calc. damped lut */
        for (int i = 0; i < 729; i++) {
            pDamped->look_up_table_r[i] =
                (unsigned short) (damp * (float) pDamped->look_up_table_r[i] + f * (float) pUndamped->look_up_table_r[i]);
            pDamped->look_up_table_g[i] =
                (unsigned short) (damp * (float) pDamped->look_up_table_g[i] + f * (float) pUndamped->look_up_table_g[i]);
            pDamped->look_up_table_b[i] =
                (unsigned short) (damp * (float) pDamped->look_up_table_b[i] + f * (float) pUndamped->look_up_table_b[i]);
        }

        result = XCAM_RETURN_NO_ERROR;
    }
    LOGD_A3DLUT( "dampfactor:%f \n", damp);
    return ( result );
}

XCamReturn Alut3dAutoConfig
(
    alut3d_handle_t hAlut3d
) {

    LOGI_A3DLUT("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!hAlut3d)
        return XCAM_RETURN_ERROR_PARAM;

    const CalibDbV2_Lut3D_LutPara_t* pLutProfile = NULL;
    float sensorGain =  hAlut3d->swinfo.sensorGain;
    if (hAlut3d->update || hAlut3d->updateAtt) {
        //(1) estimate idx
        int dominateProfileIdx;
        ret = lut3d_index_estimation(hAlut3d->calibV2_lut3d->ALut3D.lutAll_len, hAlut3d->calibV2_lut3d->ALut3D.lutAll, hAlut3d->swinfo.awbGain, &dominateProfileIdx);
        RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

#if RKAIQ_A3DLUT_ILLU_VOTE
        int dominateListSize = 0;
        dominateListSize = hAlut3d->calibV2_lut3d->ALut3D.lutAll_len*2+1;
        UpdateDominateIdxList(&hAlut3d->restinfo.dominateIdxList, dominateProfileIdx, dominateListSize);
        StableIdxEstimation(hAlut3d->restinfo.dominateIdxList, dominateListSize, hAlut3d->calibV2_lut3d->ALut3D.lutAll_len, &dominateProfileIdx);
#endif

        hAlut3d->restinfo.dominateIdx = dominateProfileIdx;

        //(2) interpolate alpha
        pLutProfile = &hAlut3d->calibV2_lut3d->ALut3D.lutAll[dominateProfileIdx];
        hAlut3d->restinfo.pLutProfile = &hAlut3d->calibV2_lut3d->ALut3D.lutAll[dominateProfileIdx];

        float alp_tmp = 0;
        interpolation(pLutProfile->gain_alpha.gain, pLutProfile->gain_alpha.alpha, 9, sensorGain,
                      &alp_tmp);

        LOGD_A3DLUT("sensorGain: %f, Alpha:%f \n", sensorGain, alp_tmp);

        //(3) lut = alpha*lutfile + (1-alpha)*lut0
        uint32_t alpha = uint32_t(alp_tmp * 128.0f);
        if (alpha == 0) {
            hAlut3d->restinfo.alpha = 0;
            uint16_t lut_idx        = 0;
            for (uint16_t i = 0; i < 9; i++) {
                for (uint16_t j = 0; j < 9; j++) {
                    for (uint16_t k = 0; k < 9; k++) {
                        lut_idx                                         = i * 81 + j * 9 + k;
                        hAlut3d->restinfo.UndampLut.look_up_table_r[lut_idx] = (k << 7) - (k >> 3);
                        hAlut3d->restinfo.UndampLut.look_up_table_g[lut_idx] = (j << 9) - (j >> 3);
                        hAlut3d->restinfo.UndampLut.look_up_table_b[lut_idx] = (i << 7) - (i >> 3);
                    }
                }
            }

        } else if (alpha >= 128) {
            hAlut3d->restinfo.alpha = 1;
            memcpy(hAlut3d->restinfo.UndampLut.look_up_table_r, pLutProfile->Table.look_up_table_r,
                   sizeof(pLutProfile->Table.look_up_table_r));
            memcpy(hAlut3d->restinfo.UndampLut.look_up_table_g, pLutProfile->Table.look_up_table_g,
                   sizeof(pLutProfile->Table.look_up_table_g));
            memcpy(hAlut3d->restinfo.UndampLut.look_up_table_b, pLutProfile->Table.look_up_table_b,
                   sizeof(pLutProfile->Table.look_up_table_b));
        }  // if alp changed || lut changed
        else if ((alpha != uint32_t(hAlut3d->restinfo.alpha * 128.0f)) || hAlut3d->calib_update ||
                 (hAlut3d->restinfo.dominateIdx != dominateProfileIdx)) {
            LOGD_A3DLUT("begin lut interp\n");
            hAlut3d->restinfo.alpha = alp_tmp;
            const uint32_t beta     = (1 << 7) - alpha;

            uint32_t lut_idx = 0;
            for (uint32_t i = 0; i < 9; i++) {
                for (uint32_t j = 0; j < 9; j++) {
                    for (uint32_t k = 0; k < 9; k++) {
                        lut_idx = i * 81 + j * 9 + k;
                        hAlut3d->restinfo.UndampLut.look_up_table_r[lut_idx] =
                            (unsigned short)((alpha * (uint32_t)pLutProfile->Table
                                                          .look_up_table_r[lut_idx] +
                                              beta * ((k << 7) - (k >> 3)) + 64) >>
                                             7);
                        hAlut3d->restinfo.UndampLut.look_up_table_g[lut_idx] =
                            (unsigned short)((alpha * (uint32_t)pLutProfile->Table
                                                          .look_up_table_g[lut_idx] +
                                              beta * ((j << 9) - (j >> 3)) + 64) >>
                                             7);
                        hAlut3d->restinfo.UndampLut.look_up_table_b[lut_idx] =
                            (unsigned short)((alpha * (uint32_t)pLutProfile->Table
                                                          .look_up_table_b[lut_idx] +
                                              beta * ((i << 7) - (i >> 3)) + 64) >>
                                             7);
                    }
                }
            }
        }
        hAlut3d->restinfo.dominateIdx = dominateProfileIdx;
        LOGD_A3DLUT("lutB[7] = %d (%d)\n", hAlut3d->restinfo.UndampLut.look_up_table_b[7], pLutProfile->Table.look_up_table_b[7]);
    }

        //(4) damp
    if ((!hAlut3d->calibV2_lut3d->ALut3D.damp_en) && ((hAlut3d->swinfo.count <= 1) || hAlut3d->update)) {
        hAlut3d->swinfo.lut3dConverged = true;
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_r, hAlut3d->restinfo.UndampLut.look_up_table_r, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_r));
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_g, hAlut3d->restinfo.UndampLut.look_up_table_g, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_g));
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_b, hAlut3d->restinfo.UndampLut.look_up_table_b, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_b));
    } else if (hAlut3d->calibV2_lut3d->ALut3D.damp_en && ((hAlut3d->swinfo.count <= 1) || (hAlut3d->swinfo.invarMode == 0))) {

        hAlut3d->swinfo.lut3dConverged = true;
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_r, hAlut3d->restinfo.UndampLut.look_up_table_r, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_r));
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_g, hAlut3d->restinfo.UndampLut.look_up_table_g, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_g));
        memcpy(hAlut3d->lut3d_hw_conf.look_up_table_b, hAlut3d->restinfo.UndampLut.look_up_table_b, sizeof(hAlut3d->lut3d_hw_conf.look_up_table_b));

    } else if (hAlut3d->calibV2_lut3d->ALut3D.damp_en && (hAlut3d->swinfo.awbIIRDampCoef > 0.0) && ((!hAlut3d->swinfo.lut3dConverged) || hAlut3d->update) ) {

        ret = Damping(hAlut3d->swinfo.awbIIRDampCoef, &hAlut3d->restinfo.UndampLut,
                        &hAlut3d->lut3d_hw_conf);
        hAlut3d->swinfo.lut3dConverged =
            !(memcmp(hAlut3d->mCurAtt.stManual.look_up_table_r, hAlut3d->lut3d_hw_conf.look_up_table_r,
            sizeof(hAlut3d->lut3d_hw_conf.look_up_table_r)) ||
            memcmp(hAlut3d->mCurAtt.stManual.look_up_table_g, hAlut3d->lut3d_hw_conf.look_up_table_g,
            sizeof(hAlut3d->lut3d_hw_conf.look_up_table_g)) ||
            memcmp(hAlut3d->mCurAtt.stManual.look_up_table_b, hAlut3d->lut3d_hw_conf.look_up_table_b,
            sizeof(hAlut3d->lut3d_hw_conf.look_up_table_b)));
        LOGD_A3DLUT("DampCoef = %f, damp lutB[7] = %d, lut converge: %d, count = %d\n", hAlut3d->swinfo.awbIIRDampCoef, hAlut3d->lut3d_hw_conf.look_up_table_b[7],
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

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_r, hAlut3d->mCurAtt.stManual.look_up_table_r,
           sizeof(hAlut3d->mCurAtt.stManual.look_up_table_r));
    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_g, hAlut3d->mCurAtt.stManual.look_up_table_g,
           sizeof(hAlut3d->mCurAtt.stManual.look_up_table_g));
    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_b, hAlut3d->mCurAtt.stManual.look_up_table_b,
           sizeof(hAlut3d->mCurAtt.stManual.look_up_table_b));

    LOGI_A3DLUT("%s: (exit)\n", __FUNCTION__);

    return (ret);
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

    if (fabs(hAlut3d->restinfo.res3a_info.sensorGain - hAlut3d->swinfo.sensorGain) > hAlut3d->calibV2_lut3d->common.gain_tolerance) {
        hAlut3d->restinfo.res3a_info.gain_stable = false;
        LOGD_A3DLUT( "%s: update sensorGain:%f \n", __FUNCTION__, hAlut3d->swinfo.sensorGain);
        hAlut3d->restinfo.res3a_info.sensorGain = hAlut3d->swinfo.sensorGain;
    } else {
        hAlut3d->restinfo.res3a_info.gain_stable = true;
        LOGD_A3DLUT( "%s: not update sensorGain:%f \n", __FUNCTION__, hAlut3d->swinfo.sensorGain);
        hAlut3d->swinfo.sensorGain = hAlut3d->restinfo.res3a_info.sensorGain;
    }

    if (hAlut3d->swinfo.awbConverged && hAlut3d->restinfo.res3a_info.gain_stable &&
        (!hAlut3d->calib_update))
        hAlut3d->update = false;
    else
        hAlut3d->update = true;

    LOGD_A3DLUT("%s: updateAtt: %d\n", __FUNCTION__, hAlut3d->updateAtt);
    if(hAlut3d->updateAtt) {
        hAlut3d->swinfo.invarMode = hAlut3d->mCurAtt.mode & hAlut3d->mNewAtt.mode;
        hAlut3d->mCurAtt = hAlut3d->mNewAtt;
    }

    LOGD_A3DLUT("%s: byPass: %d  mode:%d \n", __FUNCTION__, hAlut3d->mCurAtt.byPass, hAlut3d->mCurAtt.mode);
    if(hAlut3d->mCurAtt.byPass != true) {
        hAlut3d->lut3d_hw_conf.enable = true;
        hAlut3d->lut3d_hw_conf.bypass_en = false;

        LOGD_A3DLUT("%s: LUT3D Cfg update: %d\n", __FUNCTION__, hAlut3d->update);

        if(hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_AUTO) {
            if (hAlut3d->updateAtt || hAlut3d->update || (!hAlut3d->swinfo.lut3dConverged))
                Alut3dAutoConfig(hAlut3d);
        } else if(hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_MANUAL) {
            if (hAlut3d->updateAtt || hAlut3d->update)
                Alut3dManualConfig(hAlut3d);
        } else {
            LOGE_A3DLUT("%s: hAlut3d->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAlut3d->mCurAtt.mode);
        }

        LOGD_A3DLUT("final lutB[7] = %d\n", hAlut3d->lut3d_hw_conf.look_up_table_b[7]);

        memcpy(hAlut3d->mCurAtt.stManual.look_up_table_r, hAlut3d->lut3d_hw_conf.look_up_table_r,
               sizeof(hAlut3d->mCurAtt.stManual.look_up_table_r));
        memcpy( hAlut3d->mCurAtt.stManual.look_up_table_g, hAlut3d->lut3d_hw_conf.look_up_table_g,
                sizeof(hAlut3d->mCurAtt.stManual.look_up_table_g));
        memcpy(hAlut3d->mCurAtt.stManual.look_up_table_b, hAlut3d->lut3d_hw_conf.look_up_table_b,
               sizeof(hAlut3d->mCurAtt.stManual.look_up_table_b));

    } else {
        hAlut3d->lut3d_hw_conf.enable = false;
        hAlut3d->lut3d_hw_conf.bypass_en = true;
    }
    hAlut3d->updateAtt = false;
    hAlut3d->calib_update = false;

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

    hAlut3d->lut3d_hw_conf.lut3d_lut_wsize = 0x2d9;
    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_r, hAlut3d->calib_lut3d->look_up_table_r,
           sizeof(hAlut3d->calib_lut3d->look_up_table_r));
    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_g, hAlut3d->calib_lut3d->look_up_table_g,
           sizeof(hAlut3d->calib_lut3d->look_up_table_g));
    memcpy(hAlut3d->lut3d_hw_conf.look_up_table_b, hAlut3d->calib_lut3d->look_up_table_b,
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

    hAlut3d->swinfo.invarMode = hAlut3d->mCurAtt.mode & hAlut3d->mNewAtt.mode;
    if (hAlut3d->mCurAtt.mode == RK_AIQ_LUT3D_MODE_AUTO) {
        hAlut3d->mCurAtt.byPass = !(hAlut3d->calibV2_lut3d->common.enable);
    }

    hAlut3d->swinfo.lut3dConverged = false;
    hAlut3d->calib_update = true;

    hAlut3d->lut3d_hw_conf.lut3d_lut_wsize = 0x2d9;

#if RKAIQ_A3DLUT_ILLU_VOTE
    ClearList(&hAlut3d->restinfo.dominateIdxList);
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
    alut3d_contex->swinfo.sensorGain = 1.0;
    alut3d_contex->swinfo.awbIIRDampCoef = 0;
    alut3d_contex->swinfo.awbConverged = false;
    alut3d_contex->swinfo.awbGain[0] = 1;
    alut3d_contex->swinfo.awbGain[1] = 1;
    alut3d_contex->swinfo.count = 0;
    alut3d_contex->swinfo.invarMode = 0; // 0: mode change, 1: mode unchange

    alut3d_contex->restinfo.res3a_info.sensorGain = 1.0;
    alut3d_contex->restinfo.res3a_info.gain_stable = false;

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
    if (hAlut3d->restinfo.dominateIdxList) {
        ClearList(&hAlut3d->restinfo.dominateIdxList);
    }
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



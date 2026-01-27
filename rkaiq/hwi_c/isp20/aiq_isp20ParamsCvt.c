/*
 *  Copyright (c) 2024 Rockchip Corporation
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

void dumpIspDrvAeMeasCfg(AiqIspParamsCvt_t* pCvt, int fd) {
    struct isp2x_rawaebig_meas_cfg* rawae0 =
        (struct isp2x_rawaebig_meas_cfg*)(pCvt->mLatestCfgArray[RK_ISP2X_RAWAE0_ID].pLatestCfgaddr);
    char log[512] = {'\0'};
    sprintf(log,
            "rawae0:\n"
            "     en:%d, sel:%d, win[%dx%d@%dx%d]",
            pCvt->mLatestCfgArray[RK_ISP2X_RAWAE0_ID].bEn, rawae0->rawae_sel, rawae0->win.h_size,
            rawae0->win.v_size, rawae0->win.h_offs, rawae0->win.v_offs);
    if (fd != -1) dprintf(fd, "%s\n", log);
    LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "%s", log);
}

XCamReturn convertAiqAeToIsp20Params(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                     const rk_aiq_isp_aec_meas_t* aec_meas) {
#ifdef ISP_HW_V20
    struct isp2x_isp_params_cfg* isp_cfg = (struct isp2x_isp_params_cfg*)pIspCfg;
#elif defined(ISP_HW_V21)
    struct isp21_isp_params_cfg* isp_cfg = (struct isp21_isp_params_cfg*)pIspCfg;
#elif defined(ISP_HW_V30)
    struct isp3x_isp_params_cfg* isp_cfg = (struct isp3x_isp_params_cfg*)pIspCfg;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    struct isp32_isp_params_cfg* isp_cfg = (struct isp32_isp_params_cfg*)pIspCfg;
#elif defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_cfg = (struct isp39_isp_params_cfg*)pIspCfg;
#else
#error("not surpport !")
#endif
    isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;
    isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE1_ID;
    isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE2_ID;
    isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;
    isp_cfg->module_ens |= 1LL << RK_ISP2X_YUVAE_ID;

    mLatestCfgArray[RK_ISP2X_RAWAE0_ID].bEn = true;
    mLatestCfgArray[RK_ISP2X_RAWAE1_ID].bEn = true;
    mLatestCfgArray[RK_ISP2X_RAWAE2_ID].bEn = true;
    mLatestCfgArray[RK_ISP2X_RAWAE3_ID].bEn = true;

    isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
    isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE0_ID;

    isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE1_ID;
    isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE1_ID;

    isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE2_ID;
    isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE2_ID;

    isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
    isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE3_ID;

    isp_cfg->module_en_update |= 1LL << RK_ISP2X_YUVAE_ID;
    isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_YUVAE_ID;

#if ISP_HW_V20 || ISP_HW_V30
    memcpy(&isp_cfg->meas.rawae0, &aec_meas->rawae0, sizeof(aec_meas->rawae0));
    memcpy(&isp_cfg->meas.rawae1, &aec_meas->rawae1, sizeof(aec_meas->rawae1));
    memcpy(&isp_cfg->meas.rawae2, &aec_meas->rawae2, sizeof(aec_meas->rawae2));
    memcpy(&isp_cfg->meas.rawae3, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    mLatestCfgArray[RK_ISP2X_RAWAE0_ID].pLatestCfgaddr = &isp_cfg->meas.rawae0;
    mLatestCfgArray[RK_ISP2X_RAWAE1_ID].pLatestCfgaddr = &isp_cfg->meas.rawae1;
    mLatestCfgArray[RK_ISP2X_RAWAE2_ID].pLatestCfgaddr = &isp_cfg->meas.rawae2;
    mLatestCfgArray[RK_ISP2X_RAWAE3_ID].pLatestCfgaddr = &isp_cfg->meas.rawae3;
#endif
#if ISP_HW_V21 || ISP_HW_V32
    memcpy(&isp_cfg->meas.rawae0, &aec_meas->rawae0, sizeof(aec_meas->rawae0));
    memcpy(&isp_cfg->meas.rawae1, &aec_meas->rawae1, sizeof(aec_meas->rawae1));
    memcpy(&isp_cfg->meas.rawae2, &aec_meas->rawae1, sizeof(aec_meas->rawae1));
    memcpy(&isp_cfg->meas.rawae3, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    mLatestCfgArray[RK_ISP2X_RAWAE0_ID].pLatestCfgaddr = &isp_cfg->meas.rawae0;
    mLatestCfgArray[RK_ISP2X_RAWAE1_ID].pLatestCfgaddr = &isp_cfg->meas.rawae1;
    mLatestCfgArray[RK_ISP2X_RAWAE2_ID].pLatestCfgaddr = &isp_cfg->meas.rawae2;
    mLatestCfgArray[RK_ISP2X_RAWAE3_ID].pLatestCfgaddr = &isp_cfg->meas.rawae3;
#endif
#if ISP_HW_V32_LITE
    memcpy(&isp_cfg->meas.rawae0, &aec_meas->rawae0, sizeof(aec_meas->rawae0));
    memcpy(&isp_cfg->meas.rawae1, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    memcpy(&isp_cfg->meas.rawae2, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    memcpy(&isp_cfg->meas.rawae3, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    mLatestCfgArray[RK_ISP2X_RAWAE0_ID].pLatestCfgaddr = &isp_cfg->meas.rawae0;
    mLatestCfgArray[RK_ISP2X_RAWAE1_ID].pLatestCfgaddr = &isp_cfg->meas.rawae1;
    mLatestCfgArray[RK_ISP2X_RAWAE2_ID].pLatestCfgaddr = &isp_cfg->meas.rawae2;
    mLatestCfgArray[RK_ISP2X_RAWAE3_ID].pLatestCfgaddr = &isp_cfg->meas.rawae3;
#endif
#if ISP_HW_V39
    memcpy(&isp_cfg->meas.rawae0, &aec_meas->rawae0, sizeof(aec_meas->rawae0));
    memcpy(&isp_cfg->meas.rawae3, &aec_meas->rawae3, sizeof(aec_meas->rawae3));
    mLatestCfgArray[RK_ISP2X_RAWAE0_ID].pLatestCfgaddr = &isp_cfg->meas.rawae0;
    mLatestCfgArray[RK_ISP2X_RAWAE3_ID].pLatestCfgaddr = &isp_cfg->meas.rawae3;
#endif

#if defined(ISP_HW_V20)
    memcpy(&isp_cfg->meas.yuvae, &aec_meas->yuvae, sizeof(aec_meas->yuvae));
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    mLatestMeasCfg.rawae3 = isp_cfg->meas.rawae3;
    mLatestMeasCfg.rawae1 = isp_cfg->meas.rawae1;
    mLatestMeasCfg.rawae2 = isp_cfg->meas.rawae2;
    mLatestMeasCfg.rawae0 = isp_cfg->meas.rawae0;
#endif
#if defined(ISP_HW_V39)
    mLatestMeasCfg.rawae3 = isp_cfg->meas.rawae3;
    mLatestMeasCfg.rawae0 = isp_cfg->meas.rawae0;
#endif
}

#include "rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_types_adehaze_algo_prvt.h"
#include "xcam_log.h"

bool
IfDehazeDataEqu
(
    int* pInput,
    int len
)
{
    bool equ = true;

    for(int i = 0; i < len - 1; i++)
        if(pInput[i] != pInput[i + 1])
            equ = false;

    return equ;
}

XCamReturn
rk_aiq_uapi_adehaze_SetToolV20
(
    CalibDbV2_dehaze_V20_t* pStore,
    CalibDbV2_dehaze_V20_t* pInput
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //dehaze
    int DehazeLen[21];
    DehazeLen[0] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    DehazeLen[1] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len;
    DehazeLen[2] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len;
    DehazeLen[3] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len;
    DehazeLen[4] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len;
    DehazeLen[5] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len;
    DehazeLen[6] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len;
    DehazeLen[7] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len;
    DehazeLen[8] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len;
    DehazeLen[9] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len;
    DehazeLen[10] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len;
    DehazeLen[11] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len;
    DehazeLen[12] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len;
    DehazeLen[13] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len;
    DehazeLen[14] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len;
    DehazeLen[15] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len;
    DehazeLen[16] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len;
    DehazeLen[17] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed_len;
    DehazeLen[18] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len;
    DehazeLen[19] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_thed_len;
    DehazeLen[20] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur_len;
    bool DehazeLenEqu = IfDehazeDataEqu(DehazeLen, 21);
    if(!DehazeLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Dehaze Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }

    if(pStore->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len != pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len) {
        //free
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.ISO);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_thed);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur);
        //malloc
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.ISO =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_thed =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    }
    //store len
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_thed_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.ISO,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dark_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_min, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.wt_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_min, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_thed,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_thed, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len);

    //enhance
    int EnhanceLen[2];
    EnhanceLen[0] = pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len;
    EnhanceLen[1] = pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len;
    bool EnhanceLenEqu = IfDehazeDataEqu(EnhanceLen, 2);
    if(!EnhanceLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Enhance Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len != pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len) {
        //free
        free(pStore->DehazeTuningPara.enhance_setting.EnhanceData.ISO);
        free(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value);
        //malloc
        pStore->DehazeTuningPara.enhance_setting.EnhanceData.ISO =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len);
        pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len);
    }
    //store len
    pStore->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len =
        pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len;
    pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len =
        pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len;
    memcpy(pStore->DehazeTuningPara.enhance_setting.EnhanceData.ISO,
           pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO, sizeof(float)*pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len);
    memcpy(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value,
           pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value, sizeof(float)*pInput->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len);

    //hist
    int HistLen[7];
    HistLen[0] = pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    HistLen[1] = pInput->DehazeTuningPara.hist_setting.HistData.hist_gratio_len;
    HistLen[2] = pInput->DehazeTuningPara.hist_setting.HistData.hist_th_off_len;
    HistLen[3] = pInput->DehazeTuningPara.hist_setting.HistData.hist_k_len;
    HistLen[4] = pInput->DehazeTuningPara.hist_setting.HistData.hist_min_len;
    HistLen[5] = pInput->DehazeTuningPara.hist_setting.HistData.hist_scale_len;
    HistLen[6] = pInput->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len;
    bool HistLenEqu = IfDehazeDataEqu(HistLen, 7);
    if(!HistLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Hist Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DehazeTuningPara.hist_setting.HistData.ISO_len != pInput->DehazeTuningPara.hist_setting.HistData.ISO_len) {
        //free
        free(pStore->DehazeTuningPara.hist_setting.HistData.ISO);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_k);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_min);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_scale);
        free(pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio);
        //malloc
        pStore->DehazeTuningPara.hist_setting.HistData.ISO =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_k =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_scale =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
        pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    }
    //store len
    pStore->DehazeTuningPara.hist_setting.HistData.ISO_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_k_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_min_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_scale_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len =
        pInput->DehazeTuningPara.hist_setting.HistData.ISO_len;
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.ISO,
           pInput->DehazeTuningPara.hist_setting.HistData.ISO, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_gratio, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_th_off, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_k,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_k, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_min,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_min, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_scale,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_scale, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio,
           pInput->DehazeTuningPara.hist_setting.HistData.cfg_gratio, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.ISO_len);

    //other
    pStore->DehazeTuningPara.Enable = pInput->DehazeTuningPara.Enable;
    pStore->DehazeTuningPara.cfg_alpha = pInput->DehazeTuningPara.cfg_alpha;
    pStore->DehazeTuningPara.dehaze_setting.en = pInput->DehazeTuningPara.dehaze_setting.en;
    pStore->DehazeTuningPara.dehaze_setting.stab_fnum = pInput->DehazeTuningPara.dehaze_setting.stab_fnum;
    pStore->DehazeTuningPara.dehaze_setting.sigma = pInput->DehazeTuningPara.dehaze_setting.sigma;
    pStore->DehazeTuningPara.dehaze_setting.wt_sigma = pInput->DehazeTuningPara.dehaze_setting.wt_sigma;
    pStore->DehazeTuningPara.dehaze_setting.air_sigma = pInput->DehazeTuningPara.dehaze_setting.air_sigma;
    pStore->DehazeTuningPara.dehaze_setting.tmax_sigma = pInput->DehazeTuningPara.dehaze_setting.tmax_sigma;
    pStore->DehazeTuningPara.enhance_setting.en = pInput->DehazeTuningPara.enhance_setting.en;
    pStore->DehazeTuningPara.hist_setting.en = pInput->DehazeTuningPara.hist_setting.en;
    pStore->DehazeTuningPara.hist_setting.hist_channel = pInput->DehazeTuningPara.hist_setting.hist_channel;
    pStore->DehazeTuningPara.hist_setting.hist_para_en = pInput->DehazeTuningPara.hist_setting.hist_para_en;

    return ret;
}

XCamReturn
rk_aiq_uapi_adehaze_SetToolV21
(
    CalibDbV2_dehaze_V21_t* pStore,
    CalibDbV2_dehaze_V21_t* pInput
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //dehaze
    int DehazeLen[22];
    DehazeLen[0] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    DehazeLen[1] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len;
    DehazeLen[2] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len;
    DehazeLen[3] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len;
    DehazeLen[4] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len;
    DehazeLen[5] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len;
    DehazeLen[6] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len;
    DehazeLen[7] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len;
    DehazeLen[8] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len;
    DehazeLen[9] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len;
    DehazeLen[10] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len;
    DehazeLen[11] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len;
    DehazeLen[12] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len;
    DehazeLen[13] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len;
    DehazeLen[14] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len;
    DehazeLen[15] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len;
    DehazeLen[16] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len;
    DehazeLen[17] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight_len;
    DehazeLen[18] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len;
    DehazeLen[19] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma_len;
    DehazeLen[20] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur_len;
    DehazeLen[21] = pInput->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre_len;
    bool DehazeLenEqu = IfDehazeDataEqu(DehazeLen, 22);
    if(!DehazeLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Dehaze Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len != pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len) {
        //free
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur);
        free(pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre);
        //maloc
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
        pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    }
    //store len
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre_len =
        pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dark_th,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dark_th, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bright_min,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.bright_min, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.wt_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.wt_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.air_min,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.air_min, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre,
           pInput->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre, sizeof(float)*pInput->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len);

    //enhance
    int EnhanceLen[3];
    EnhanceLen[0] = pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;
    EnhanceLen[1] = pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len;
    EnhanceLen[2] = pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma_len;
    bool EnhanceLenEqu = IfDehazeDataEqu(EnhanceLen, 3);
    if(!EnhanceLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Enhance Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len != pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len) {
        //free
        free(pStore->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv);
        free(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value);
        free(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma);
        //malloc
        pStore->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);
        pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);
        pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);
    }
    //store len
    pStore->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len =
        pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;
    pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len =
        pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;
    pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma_len =
        pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;
    memcpy(pStore->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv,
           pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv, sizeof(float)*pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value,
           pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value, sizeof(float)*pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma,
           pInput->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma, sizeof(float)*pInput->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len);

    //hist
    int HistLen[7];
    HistLen[0] = pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    HistLen[1] = pInput->DehazeTuningPara.hist_setting.HistData.hist_gratio_len;
    HistLen[2] = pInput->DehazeTuningPara.hist_setting.HistData.hist_th_off_len;
    HistLen[3] = pInput->DehazeTuningPara.hist_setting.HistData.hist_k_len;
    HistLen[4] = pInput->DehazeTuningPara.hist_setting.HistData.hist_min_len;
    HistLen[5] = pInput->DehazeTuningPara.hist_setting.HistData.hist_scale_len;
    HistLen[6] = pInput->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len;
    bool HistLenEqu = IfDehazeDataEqu(HistLen, 7);
    if(!HistLenEqu) {
        LOGE_ADEHAZE("%s: Input Dehaze Hist Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DehazeTuningPara.hist_setting.HistData.EnvLv_len != pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len) {
        //free
        free(pStore->DehazeTuningPara.hist_setting.HistData.EnvLv);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_k);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_min);
        free(pStore->DehazeTuningPara.hist_setting.HistData.hist_scale);
        free(pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio);
        //malloc
        pStore->DehazeTuningPara.hist_setting.HistData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_k =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_min =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.hist_scale =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
        pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio =
            (float *) malloc(sizeof(float) * pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    }
    //store len
    pStore->DehazeTuningPara.hist_setting.HistData.EnvLv_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_k_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_min_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.hist_scale_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len =
        pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.EnvLv,
           pInput->DehazeTuningPara.hist_setting.HistData.EnvLv, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_gratio,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_gratio, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_th_off,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_th_off, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_k,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_k, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_min,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_min, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.hist_scale,
           pInput->DehazeTuningPara.hist_setting.HistData.hist_scale, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);
    memcpy(pStore->DehazeTuningPara.hist_setting.HistData.cfg_gratio,
           pInput->DehazeTuningPara.hist_setting.HistData.cfg_gratio, sizeof(float)*pInput->DehazeTuningPara.hist_setting.HistData.EnvLv_len);

    //other
    pStore->DehazeTuningPara.Enable = pInput->DehazeTuningPara.Enable;
    pStore->DehazeTuningPara.cfg_alpha = pInput->DehazeTuningPara.cfg_alpha;
    pStore->DehazeTuningPara.dehaze_setting.en = pInput->DehazeTuningPara.dehaze_setting.en;
    pStore->DehazeTuningPara.dehaze_setting.air_lc_en = pInput->DehazeTuningPara.dehaze_setting.air_lc_en;
    pStore->DehazeTuningPara.dehaze_setting.pre_wet = pInput->DehazeTuningPara.dehaze_setting.pre_wet;
    pStore->DehazeTuningPara.dehaze_setting.stab_fnum = pInput->DehazeTuningPara.dehaze_setting.stab_fnum;
    pStore->DehazeTuningPara.dehaze_setting.sigma = pInput->DehazeTuningPara.dehaze_setting.sigma;
    pStore->DehazeTuningPara.dehaze_setting.wt_sigma = pInput->DehazeTuningPara.dehaze_setting.wt_sigma;
    pStore->DehazeTuningPara.dehaze_setting.air_sigma = pInput->DehazeTuningPara.dehaze_setting.air_sigma;
    pStore->DehazeTuningPara.dehaze_setting.tmax_sigma = pInput->DehazeTuningPara.dehaze_setting.tmax_sigma;
    pStore->DehazeTuningPara.enhance_setting.en = pInput->DehazeTuningPara.enhance_setting.en;
    memcpy(pStore->DehazeTuningPara.enhance_setting.enhance_curve,
           pInput->DehazeTuningPara.enhance_setting.enhance_curve, sizeof(float) * 17);
    pStore->DehazeTuningPara.hist_setting.en = pInput->DehazeTuningPara.hist_setting.en;
    pStore->DehazeTuningPara.hist_setting.hist_para_en = pInput->DehazeTuningPara.hist_setting.hist_para_en;

    return ret;
}

XCamReturn
rk_aiq_uapi_adehaze_SetAttrib(RkAiqAlgoContext *ctx,
                              adehaze_sw_V2_t attr,
                              bool need_sync)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)ctx;

    if(CHECK_ISP_HW_V20()) {
        AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.mode = attr.AdehazeAtrrV20.mode;
        if(attr.AdehazeAtrrV20.mode == DEHAZE_API_MANUAL)
            memcpy(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stManual, &attr.AdehazeAtrrV20.stManual, sizeof(rk_aiq_dehaze_M_attrib_t));
        else if(attr.AdehazeAtrrV20.mode == DEHAZE_API_TOOL) {
            ret = rk_aiq_uapi_adehaze_SetToolV20(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stTool, &attr.AdehazeAtrrV20.stTool);
            if(ret != XCAM_RETURN_NO_ERROR)
                return XCAM_RETURN_ERROR_FAILED;
        }
        else if(attr.AdehazeAtrrV20.mode == DEHAZE_API_ENHANCE_MANUAL)
            memcpy(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stEnhanceManual, &attr.AdehazeAtrrV20.stEnhanceManual, sizeof(rk_aiq_dehaze_enhance_t));
    }
    else if(CHECK_ISP_HW_V21()) {
        AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.mode = attr.AdehazeAtrrV21.mode;
        if(attr.AdehazeAtrrV21.mode == DEHAZE_API_MANUAL)
            memcpy(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stManual, &attr.AdehazeAtrrV21.stManual, sizeof(rk_aiq_dehaze_M_attrib_t));
        else if(attr.AdehazeAtrrV21.mode == DEHAZE_API_TOOL) {
            ret = rk_aiq_uapi_adehaze_SetToolV21(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stTool, &attr.AdehazeAtrrV21.stTool);
            if(ret != XCAM_RETURN_NO_ERROR)
                return XCAM_RETURN_ERROR_FAILED;
        }
        else if(attr.AdehazeAtrrV21.mode == DEHAZE_API_ENHANCE_MANUAL)
            memcpy(&AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stEnhanceManual, &attr.AdehazeAtrrV21.stEnhanceManual, sizeof(rk_aiq_dehaze_enhance_t));
    }

    return ret;
}

XCamReturn
rk_aiq_uapi_adehaze_GetAttrib(RkAiqAlgoContext *ctx, adehaze_sw_V2_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)ctx;

    attr->HWversion = AdehazeHandle->AdehazeAtrr.HWversion;
    if(CHECK_ISP_HW_V20()) {
        attr->AdehazeAtrrV20.mode = AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.mode;
        memcpy(&attr->AdehazeAtrrV20.stManual, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stManual, sizeof(rk_aiq_dehaze_M_attrib_t));
        ret = rk_aiq_uapi_adehaze_SetToolV20(&attr->AdehazeAtrrV20.stTool, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stTool);
        if(ret != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_ERROR_FAILED;
        memcpy(&attr->AdehazeAtrrV20.stEnhanceManual, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV20.stEnhanceManual, sizeof(rk_aiq_dehaze_enhance_t));
    }
    else if(CHECK_ISP_HW_V21()) {
        attr->AdehazeAtrrV21.mode = AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.mode;
        memcpy(&attr->AdehazeAtrrV21.stManual, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stManual, sizeof(rk_aiq_dehaze_M_attrib_t));
        ret = rk_aiq_uapi_adehaze_SetToolV21(&attr->AdehazeAtrrV21.stTool, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stTool);
        if(ret != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_ERROR_FAILED;
        memcpy(&attr->AdehazeAtrrV21.stEnhanceManual, &AdehazeHandle->AdehazeAtrr.AdehazeAtrrV21.stEnhanceManual, sizeof(rk_aiq_dehaze_enhance_t));
    }

    return ret;
}


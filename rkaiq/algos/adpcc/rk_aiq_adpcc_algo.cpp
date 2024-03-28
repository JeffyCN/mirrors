#include "rk_aiq_adpcc_algo.h"
#include "rk_aiq_algo_adpcc_itf.h"

#define DPCC_RK_MODE

int AdpccInterpolation(int inPara, int* inMatrixX, int* inMatrixY)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    int returnValue = 1;

    float value = 1.0;
    for(int i = 0; i < DPCC_MAX_ISO_LEVEL - 1; i++) {
        if(inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1]) {
            value = ((float)inMatrixY[i] - (float)inMatrixY[i + 1])
                    / ((float)inMatrixX[i] - (float)inMatrixX[i + 1]);
            value *=  ((float)inPara - (float)inMatrixX[i] );
            value += (float)inMatrixY[i];
            break;
        }
    }

    if(inPara < inMatrixX[0] ) {
        value = (float)inMatrixY[0];
    }

    if(inPara >= inMatrixX[DPCC_MAX_ISO_LEVEL - 1] ) {
        value = (float)inMatrixY[DPCC_MAX_ISO_LEVEL - 1];
    }

    returnValue = (int)(value + 0.5);

    LOGI_ADPCC("%s(%d): inPara %d, returnValue %d", __FUNCTION__, __LINE__, inPara, returnValue);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return returnValue;
}

AdpccResult_t html_params_init(Adpcc_html_param_t *pParams)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    Adpcc_basic_params_t *pBasic = &pParams->stBasic;
    Adpcc_bpt_params_t *pBpt = &pParams->stBpt;
    Adpcc_pdaf_params_t *pPdaf = &pParams->stPdaf;
    int iso_base = 50;

    for(int i = 0; i < DPCC_MAX_ISO_LEVEL; i++) {
        pBasic->arBasic[i].iso = iso_base * ( 1 << i);

#ifdef DPCC_RK_MODE
        //mode 0x0000
        pBasic->arBasic[i].stage1_enable = 1;
        pBasic->arBasic[i].grayscale_mode = 0;
        pBasic->arBasic[i].enable = 1;

        //output_mode 0x0004
        pBasic->arBasic[i].sw_rk_out_sel = 1;
        pBasic->arBasic[i].sw_dpcc_output_sel = 1;
        pBasic->arBasic[i].stage1_rb_3x3 = 0;
        pBasic->arBasic[i].stage1_g_3x3 = 0;
        pBasic->arBasic[i].stage1_incl_rb_center = 1;
        pBasic->arBasic[i].stage1_incl_green_center = 1;

        //set_use 0x0008
        pBasic->arBasic[i].stage1_use_fix_set = 0;
        pBasic->arBasic[i].stage1_use_set_3 = 0;
        pBasic->arBasic[i].stage1_use_set_2 = 1;
        pBasic->arBasic[i].stage1_use_set_1 = 0;

        //methods_set_1 0x000c
        pBasic->arBasic[i].sw_rk_red_blue1_en = 0;
        pBasic->arBasic[i].rg_red_blue1_enable = 1;
        pBasic->arBasic[i].rnd_red_blue1_enable = 1;
        pBasic->arBasic[i].ro_red_blue1_enable = 1;
        pBasic->arBasic[i].lc_red_blue1_enable = 0;
        pBasic->arBasic[i].pg_red_blue1_enable = 1;
        pBasic->arBasic[i].sw_rk_green1_en = 0;
        pBasic->arBasic[i].rg_green1_enable = 1;
        pBasic->arBasic[i].rnd_green1_enable = 1;
        pBasic->arBasic[i].ro_green1_enable = 1;
        pBasic->arBasic[i].lc_green1_enable = 0;
        pBasic->arBasic[i].pg_green1_enable = 1;

        //methods_set_2 0x0010
        pBasic->arBasic[i].sw_rk_red_blue2_en = 1;
        pBasic->arBasic[i].rg_red_blue2_enable = 0;
        pBasic->arBasic[i].rnd_red_blue2_enable = 0;
        pBasic->arBasic[i].ro_red_blue2_enable = 1;
        pBasic->arBasic[i].lc_red_blue2_enable = 1;
        pBasic->arBasic[i].pg_red_blue2_enable = 1;
        pBasic->arBasic[i].sw_rk_green2_en = 1;
        pBasic->arBasic[i].rg_green2_enable = 0;
        pBasic->arBasic[i].rnd_green2_enable = 0;
        pBasic->arBasic[i].ro_green2_enable = 1;
        pBasic->arBasic[i].lc_green2_enable = 1;
        pBasic->arBasic[i].pg_green2_enable = 1;

        //methods_set_3 0x0014
        pBasic->arBasic[i].sw_rk_red_blue3_en = 0;
        pBasic->arBasic[i].rg_red_blue3_enable = 1;
        pBasic->arBasic[i].rnd_red_blue3_enable = 1;
        pBasic->arBasic[i].ro_red_blue3_enable = 1;
        pBasic->arBasic[i].lc_red_blue3_enable = 1;
        pBasic->arBasic[i].pg_red_blue3_enable = 1;
        pBasic->arBasic[i].sw_rk_green3_en = 0;
        pBasic->arBasic[i].rg_green3_enable = 1;
        pBasic->arBasic[i].rnd_green3_enable = 1;
        pBasic->arBasic[i].ro_green3_enable = 1;
        pBasic->arBasic[i].lc_green3_enable = 1;
        pBasic->arBasic[i].pg_green3_enable = 1;

        //line_thresh_1 0x0018
        pBasic->arBasic[i].sw_mindis1_rb = 0;
        pBasic->arBasic[i].sw_mindis1_g = 0;
        pBasic->arBasic[i].line_thr_1_rb = 8;
        pBasic->arBasic[i].line_thr_1_g = 8;

        //line_mad_fac_1 0x001c
        pBasic->arBasic[i].sw_dis_scale_min1 = 0;
        pBasic->arBasic[i].sw_dis_scale_max1 = 0;
        pBasic->arBasic[i].line_mad_fac_1_rb = 4;
        pBasic->arBasic[i].line_mad_fac_1_g = 4;

        //pg_fac_1 0x0020
        pBasic->arBasic[i].pg_fac_1_rb = 8;
        pBasic->arBasic[i].pg_fac_1_g = 8;

        //rnd_thresh_1 0x0024
        pBasic->arBasic[i].rnd_thr_1_rb = 10;
        pBasic->arBasic[i].rnd_thr_1_g = 10;

        //rg_fac_1 0x0028
        pBasic->arBasic[i].rg_fac_1_rb = 32;
        pBasic->arBasic[i].rg_fac_1_g = 32;


        //line_thresh_2 0x002c
        pBasic->arBasic[i].sw_mindis2_rb = 20;
        pBasic->arBasic[i].sw_mindis2_g = 20;
        pBasic->arBasic[i].line_thr_2_rb = 20;
        pBasic->arBasic[i].line_thr_2_g = 20;

        //line_mad_fac_2 0x0030
        pBasic->arBasic[i].sw_dis_scale_min2 = 12;
        pBasic->arBasic[i].sw_dis_scale_max2 = 12;
        pBasic->arBasic[i].line_mad_fac_2_rb = 10;
        pBasic->arBasic[i].line_mad_fac_2_g = 10;

        //pg_fac_2 0x0034
        pBasic->arBasic[i].pg_fac_2_rb = 5;
        pBasic->arBasic[i].pg_fac_2_g = 4;

        //rnd_thresh_2 0x0038
        pBasic->arBasic[i].rnd_thr_2_rb = 0;
        pBasic->arBasic[i].rnd_thr_2_g = 0;

        //rg_fac_2 0x003c
        pBasic->arBasic[i].rg_fac_2_rb = 0;
        pBasic->arBasic[i].rg_fac_2_g = 0;


        //line_thresh_3 0x0040
        pBasic->arBasic[i].sw_mindis3_rb = 0;
        pBasic->arBasic[i].sw_mindis3_g = 0;
        pBasic->arBasic[i].line_thr_3_rb = 32;
        pBasic->arBasic[i].line_thr_3_g = 32;

        //line_mad_fac_3 0x0044
        pBasic->arBasic[i].sw_dis_scale_min3 = 0;
        pBasic->arBasic[i].sw_dis_scale_max3 = 0;
        pBasic->arBasic[i].line_mad_fac_3_rb = 4;
        pBasic->arBasic[i].line_mad_fac_3_g = 4;

        //pg_fac_3 0x0048
        pBasic->arBasic[i].pg_fac_3_rb = 10;
        pBasic->arBasic[i].pg_fac_3_g = 10;

        //rnd_thresh_3 0x004c
        pBasic->arBasic[i].rnd_thr_3_rb = 8;
        pBasic->arBasic[i].rnd_thr_3_g = 6;

        //rg_fac_3 0x0050
        pBasic->arBasic[i].rg_fac_3_rb = 4;
        pBasic->arBasic[i].rg_fac_3_g = 4;

        //ro_limits 0x0054
        pBasic->arBasic[i].ro_lim_3_rb = 2;
        pBasic->arBasic[i].ro_lim_3_g = 1;
        pBasic->arBasic[i].ro_lim_2_rb = 2;
        pBasic->arBasic[i].ro_lim_2_g = 2;
        pBasic->arBasic[i].ro_lim_1_rb = 1;
        pBasic->arBasic[i].ro_lim_1_g = 1;

        //rnd_offs 0x0058
        pBasic->arBasic[i].rnd_offs_3_rb = 2;
        pBasic->arBasic[i].rnd_offs_3_g = 2;
        pBasic->arBasic[i].rnd_offs_2_rb = 0;
        pBasic->arBasic[i].rnd_offs_2_g = 0;
        pBasic->arBasic[i].rnd_offs_1_rb = 2;
        pBasic->arBasic[i].rnd_offs_1_g = 2;
#else

        //mode 0x0000
        pBasic->arBasic[i].stage1_enable = 1;
        pBasic->arBasic[i].grayscale_mode = 0;
        pBasic->arBasic[i].enable = 1;

        //output_mode 0x0004
        pBasic->arBasic[i].sw_rk_out_sel = 0;
        pBasic->arBasic[i].sw_dpcc_output_sel = 0;
        pBasic->arBasic[i].stage1_rb_3x3 = 0;
        pBasic->arBasic[i].stage1_g_3x3 = 0;
        pBasic->arBasic[i].stage1_incl_rb_center = 1;
        pBasic->arBasic[i].stage1_incl_green_center = 1;

        //set_use 0x0008
        pBasic->arBasic[i].SetEnable.fix_set = 0;
        pBasic->arBasic[i].stage1_use_set_3 = 1;
        pBasic->arBasic[i].stage1_use_set_2 = 1;
        pBasic->arBasic[i].stage1_use_set_1 = 1;

        //methods_set_1 0x000c
        pBasic->arBasic[i].sw_rk_red_blue1_en = 0;
        pBasic->arBasic[i].rg_red_blue1_enable = 1;
        pBasic->arBasic[i].rnd_red_blue1_enable = 1;
        pBasic->arBasic[i].ro_red_blue1_enable = 1;
        pBasic->arBasic[i].lc_red_blue1_enable = 0;
        pBasic->arBasic[i].pg_red_blue1_enable = 1;
        pBasic->arBasic[i].sw_rk_green1_en = 0;
        pBasic->arBasic[i].rg_green1_enable = 1;
        pBasic->arBasic[i].rnd_green1_enable = 1;
        pBasic->arBasic[i].ro_green1_enable = 1;
        pBasic->arBasic[i].lc_green1_enable = 0;
        pBasic->arBasic[i].pg_green1_enable = 1;

        //methods_set_2 0x0010
        pBasic->arBasic[i].sw_rk_red_blue2_en = 0;
        pBasic->arBasic[i].rg_red_blue2_enable = 0;
        pBasic->arBasic[i].rnd_red_blue2_enable = 0;
        pBasic->arBasic[i].ro_red_blue2_enable = 1;
        pBasic->arBasic[i].lc_red_blue2_enable = 1;
        pBasic->arBasic[i].pg_red_blue2_enable = 1;
        pBasic->arBasic[i].sw_rk_green2_en = 0;
        pBasic->arBasic[i].rg_green2_enable = 0;
        pBasic->arBasic[i].rnd_green2_enable = 0;
        pBasic->arBasic[i].ro_green2_enable = 1;
        pBasic->arBasic[i].lc_green2_enable = 1;
        pBasic->arBasic[i].pg_green2_enable = 1;

        //methods_set_3 0x0014
        pBasic->arBasic[i].sw_rk_red_blue3_en = 0;
        pBasic->arBasic[i].rg_red_blue3_enable = 1;
        pBasic->arBasic[i].rnd_red_blue3_enable = 1;
        pBasic->arBasic[i].ro_red_blue3_enable = 1;
        pBasic->arBasic[i].lc_red_blue3_enable = 1;
        pBasic->arBasic[i].pg_red_blue3_enable = 1;
        pBasic->arBasic[i].sw_rk_green3_en = 0;
        pBasic->arBasic[i].rg_green3_enable = 1;
        pBasic->arBasic[i].rnd_green3_enable = 1;
        pBasic->arBasic[i].ro_green3_enable = 1;
        pBasic->arBasic[i].lc_green3_enable = 1;
        pBasic->arBasic[i].pg_green3_enable = 1;

        //line_thresh_1 0x0018
        pBasic->arBasic[i].sw_mindis1_rb = 0;
        pBasic->arBasic[i].sw_mindis1_g = 0;
        pBasic->arBasic[i].line_thr_1_rb = 8;
        pBasic->arBasic[i].line_thr_1_g = 8;

        //line_mad_fac_1 0x001c
        pBasic->arBasic[i].sw_dis_scale_min1 = 0;
        pBasic->arBasic[i].sw_dis_scale_max1 = 0;
        pBasic->arBasic[i].line_mad_fac_1_rb = 4;
        pBasic->arBasic[i].line_mad_fac_1_g = 4;

        //pg_fac_1 0x0020
        pBasic->arBasic[i].pg_fac_1_rb = 8;
        pBasic->arBasic[i].pg_fac_1_g = 8;

        //rnd_thresh_1 0x0024
        pBasic->arBasic[i].rnd_thr_1_rb = 10;
        pBasic->arBasic[i].rnd_thr_1_g = 10;

        //rg_fac_1 0x0028
        pBasic->arBasic[i].rg_fac_1_rb = 32;
        pBasic->arBasic[i].rg_fac_1_g = 32;


        //line_thresh_2 0x002c
        pBasic->arBasic[i].sw_mindis2_rb = 0;
        pBasic->arBasic[i].sw_mindis2_g = 0;
        pBasic->arBasic[i].line_thr_2_rb = 20;
        pBasic->arBasic[i].line_thr_2_g = 20;

        //line_mad_fac_2 0x0030
        pBasic->arBasic[i].sw_dis_scale_min2 = 0;
        pBasic->arBasic[i].sw_dis_scale_max2 = 0;
        pBasic->arBasic[i].line_mad_fac_2_rb = 10;
        pBasic->arBasic[i].line_mad_fac_2_g = 10;

        //pg_fac_2 0x0034
        pBasic->arBasic[i].pg_fac_2_rb = 4;
        pBasic->arBasic[i].pg_fac_2_g = 5;

        //rnd_thresh_2 0x0038
        pBasic->arBasic[i].rnd_thr_2_rb = 0;
        pBasic->arBasic[i].rnd_thr_2_g = 0;

        //rg_fac_2 0x003c
        pBasic->arBasic[i].rg_fac_2_rb = 0;
        pBasic->arBasic[i].rg_fac_2_g = 0;


        //line_thresh_3 0x0040
        pBasic->arBasic[i].sw_mindis3_rb = 0;
        pBasic->arBasic[i].sw_mindis3_g = 0;
        pBasic->arBasic[i].line_thr_3_rb = 32;
        pBasic->arBasic[i].line_thr_3_g = 32;

        //line_mad_fac_3 0x0044
        pBasic->arBasic[i].sw_dis_scale_min3 = 0;
        pBasic->arBasic[i].sw_dis_scale_max3 = 0;
        pBasic->arBasic[i].line_mad_fac_3_rb = 4;
        pBasic->arBasic[i].line_mad_fac_3_g = 4;

        //pg_fac_3 0x0048
        pBasic->arBasic[i].pg_fac_3_rb = 10;
        pBasic->arBasic[i].pg_fac_3_g = 10;

        //rnd_thresh_3 0x004c
        pBasic->arBasic[i].rnd_thr_3_rb = 8;
        pBasic->arBasic[i].rnd_thr_3_g = 6;

        //rg_fac_3 0x0050
        pBasic->arBasic[i].rg_fac_3_rb = 4;
        pBasic->arBasic[i].rg_fac_3_g = 4;

        //ro_limits 0x0054
        pBasic->arBasic[i].ro_lim_3_rb = 2;
        pBasic->arBasic[i].ro_lim_3_g = 1;
        pBasic->arBasic[i].ro_lim_2_rb = 2;
        pBasic->arBasic[i].ro_lim_2_g = 2;
        pBasic->arBasic[i].ro_lim_1_rb = 1;
        pBasic->arBasic[i].ro_lim_1_g = 1;

        //rnd_offs 0x0058
        pBasic->arBasic[i].rnd_offs_3_rb = 2;
        pBasic->arBasic[i].rnd_offs_3_g = 2;
        pBasic->arBasic[i].rnd_offs_2_rb = 2;
        pBasic->arBasic[i].rnd_offs_2_g = 2;
        pBasic->arBasic[i].rnd_offs_1_rb = 2;
        pBasic->arBasic[i].rnd_offs_1_g = 2;
#endif
    }

    memset(pBpt, 0x00, sizeof(Adpcc_bpt_params_t));
    memset(pPdaf, 0x00, sizeof(Adpcc_pdaf_params_t));

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;

}

AdpccResult_t dpcc_sensor_params_init(CalibDb_Dpcc_Sensor_t *pSensor, CalibDbV2_Dpcc_t *pCalib)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSensor == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pCalib == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    pSensor->en = pCalib->DpccTuningPara.Sensor_dpcc.sensor_dpcc_auto_en ? 1.0 : 0.0;
    pSensor->max_level = (float)(pCalib->DpccTuningPara.Sensor_dpcc.max_level);

    for(int i = 0; i < pCalib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO_len; i++) {
        pSensor->iso[i] = pCalib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO[i];
        pSensor->level_single[i] = (float)(pCalib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_single[i]);
        pSensor->level_multiple[i] = (float)(pCalib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_multiple[i]);
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;

}

AdpccResult_t dpcc_fast_mode_basic_params_init(CalibDb_Dpcc_Fast_Mode_t *pFast, CalibDbV2_Dpcc_t *pCalib)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pFast == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pCalib == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    pFast->fast_mode_en = pCalib->DpccTuningPara.Fast_Mode.Fast_mode_en ? 1 : 0;
    pFast->fast_mode_single_en = pCalib->DpccTuningPara.Fast_Mode.Single_enable;
    pFast->fast_mode_double_en = pCalib->DpccTuningPara.Fast_Mode.Double_enable;
    pFast->fast_mode_triple_en = pCalib->DpccTuningPara.Fast_Mode.Triple_enable;

    for(int i = 0; i < pCalib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len; i++) {
        pFast->ISO[i] = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.ISO[i];
        pFast->fast_mode_single_level[i] = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Single_level[i];
        pFast->fast_mode_double_level[i] = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Double_level[i];
        pFast->fast_mode_triple_level[i] = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Triple_level[i];
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;

}

AdpccResult_t dpcc_manual_fast_mode_init(Adpcc_onfly_cfg_t *pFast, CalibDbV2_Dpcc_t *pCalib)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pFast == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pCalib == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    pFast->mode = ADPCC_ONFLY_MODE_FAST;
    pFast->fast_mode.fast_mode_en = 1;
    if (pCalib->DpccTuningPara.Fast_Mode.Fast_mode_en) {
        pFast->fast_mode.fast_mode_single_en = pCalib->DpccTuningPara.Fast_Mode.Single_enable;
        pFast->fast_mode.fast_mode_double_en = pCalib->DpccTuningPara.Fast_Mode.Double_enable;
        pFast->fast_mode.fast_mode_triple_en = pCalib->DpccTuningPara.Fast_Mode.Triple_enable;
        pFast->fast_mode.fast_mode_single_level = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Single_level[0];
        pFast->fast_mode.fast_mode_double_level = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Double_level[0];
        pFast->fast_mode.fast_mode_triple_level = pCalib->DpccTuningPara.Fast_Mode.Fast_Data.Triple_level[0];
    } else {
        pFast->fast_mode.fast_mode_single_en = 1;
        pFast->fast_mode.fast_mode_double_en = 1;
        pFast->fast_mode.fast_mode_triple_en = 0;
        pFast->fast_mode.fast_mode_single_level = 1;
        pFast->fast_mode.fast_mode_double_level = 1;
        pFast->fast_mode.fast_mode_triple_level = 1;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;

}

AdpccResult_t dpcc_expert_mode_basic_params_init(Adpcc_basic_params_t *pBasic, CalibDbV2_Dpcc_t *pCalib)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pBasic == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pCalib == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    for(int i = 0; i < DPCC_MAX_ISO_LEVEL; i++) {
        pBasic->arBasic[i].iso = pCalib->DpccTuningPara.Expert_Mode.SetEnable.ISO[i];

        //mode 0x0000
        pBasic->arBasic[i].stage1_enable = pCalib->DpccTuningPara.Expert_Mode.stage1_Enable ? 1 : 0;
        pBasic->arBasic[i].grayscale_mode = pCalib->DpccTuningPara.Expert_Mode.grayscale_mode ? 1 : 0;
        pBasic->arBasic[i].enable = pCalib->DpccTuningPara.Enable ? 1 : 0;

        //output_mode 0x0004
        pBasic->arBasic[i].sw_rk_out_sel = pCalib->DpccTuningPara.Expert_Mode.rk_out_sel;
        pBasic->arBasic[i].sw_dpcc_output_sel = pCalib->DpccTuningPara.Expert_Mode.dpcc_out_sel ? 1 : 0;
        pBasic->arBasic[i].stage1_rb_3x3 = pCalib->DpccTuningPara.Expert_Mode.stage1_rb_3x3 ? 1 : 0;
        pBasic->arBasic[i].stage1_g_3x3 = pCalib->DpccTuningPara.Expert_Mode.stage1_g_3x3 ? 1 : 0;
        pBasic->arBasic[i].stage1_incl_rb_center = pCalib->DpccTuningPara.Expert_Mode.stage1_inc_rb_center ? 1 : 0;
        pBasic->arBasic[i].stage1_incl_green_center = pCalib->DpccTuningPara.Expert_Mode.stage1_inc_g_center ? 1 : 0;

        //set_use 0x0008
        pBasic->arBasic[i].stage1_use_fix_set = pCalib->DpccTuningPara.Expert_Mode.SetEnable.fix_set[i];
        pBasic->arBasic[i].stage1_use_set_3 = pCalib->DpccTuningPara.Expert_Mode.SetEnable.set3[i];
        pBasic->arBasic[i].stage1_use_set_2 = pCalib->DpccTuningPara.Expert_Mode.SetEnable.set2[i];
        pBasic->arBasic[i].stage1_use_set_1 = pCalib->DpccTuningPara.Expert_Mode.SetEnable.set1[i];

        //methods_set_1 0x000c
        pBasic->arBasic[i].sw_rk_red_blue1_en = pCalib->DpccTuningPara.Expert_Mode.set1.RK.RK_enable[i];
        pBasic->arBasic[i].rg_red_blue1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_red_blue1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RND.RND_enable[i];
        pBasic->arBasic[i].ro_red_blue1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RO.RO_enable[i];
        pBasic->arBasic[i].lc_red_blue1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.LC.LC_enable[i];
        pBasic->arBasic[i].pg_red_blue1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.PG.PG_enable[i];
        pBasic->arBasic[i].sw_rk_green1_en = pCalib->DpccTuningPara.Expert_Mode.set1.RK.RK_enable[i];
        pBasic->arBasic[i].rg_green1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_green1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RND.RND_enable[i];
        pBasic->arBasic[i].ro_green1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.RO.RO_enable[i];
        pBasic->arBasic[i].lc_green1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.LC.LC_enable[i];
        pBasic->arBasic[i].pg_green1_enable = pCalib->DpccTuningPara.Expert_Mode.set1.PG.PG_enable[i];

        //methods_set_2 0x0010
        pBasic->arBasic[i].sw_rk_red_blue2_en = pCalib->DpccTuningPara.Expert_Mode.set2.RK.RK_enable[i];
        pBasic->arBasic[i].rg_red_blue2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_red_blue2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RND.RND_enable[i];
        pBasic->arBasic[i].ro_red_blue2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RO.RO_enable[i];
        pBasic->arBasic[i].lc_red_blue2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.LC.LC_enable[i];
        pBasic->arBasic[i].pg_red_blue2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.PG.PG_enable[i];
        pBasic->arBasic[i].sw_rk_green2_en = pCalib->DpccTuningPara.Expert_Mode.set2.RK.RK_enable[i];
        pBasic->arBasic[i].rg_green2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_green2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RND.RND_enable[i];
        pBasic->arBasic[i].ro_green2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.RO.RO_enable[i];
        pBasic->arBasic[i].lc_green2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.LC.LC_enable[i];
        pBasic->arBasic[i].pg_green2_enable = pCalib->DpccTuningPara.Expert_Mode.set2.PG.PG_enable[i];

        //methods_set_3 0x0014
        pBasic->arBasic[i].sw_rk_red_blue3_en = pCalib->DpccTuningPara.Expert_Mode.set3.RK.RK_enable[i];
        pBasic->arBasic[i].rg_red_blue3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_red_blue3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RND.RND_enable[i];
        pBasic->arBasic[i].ro_red_blue3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RO.RO_enable[i];
        pBasic->arBasic[i].lc_red_blue3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.LC.LC_enable[i];
        pBasic->arBasic[i].pg_red_blue3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.PG.PG_enable[i];
        pBasic->arBasic[i].sw_rk_green3_en = pCalib->DpccTuningPara.Expert_Mode.set3.RK.RK_enable[i];
        pBasic->arBasic[i].rg_green3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RG.RG_enable[i];
        pBasic->arBasic[i].rnd_green3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RND.RND_enable[i];
        pBasic->arBasic[i].ro_green3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.RO.RO_enable[i];
        pBasic->arBasic[i].lc_green3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.LC.LC_enable[i];
        pBasic->arBasic[i].pg_green3_enable = pCalib->DpccTuningPara.Expert_Mode.set3.PG.PG_enable[i];

        //line_thresh_1 0x0018
        pBasic->arBasic[i].sw_mindis1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.RK.rb_sw_mindis[i];
        pBasic->arBasic[i].sw_mindis1_g = pCalib->DpccTuningPara.Expert_Mode.set1.RK.g_sw_mindis[i];
        pBasic->arBasic[i].line_thr_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.LC.rb_line_thr[i];
        pBasic->arBasic[i].line_thr_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.LC.g_line_thr[i];

        //line_mad_fac_1 0x001c
        pBasic->arBasic[i].sw_dis_scale_min1 = pCalib->DpccTuningPara.Expert_Mode.set1.RK.sw_dis_scale_min[i];
        pBasic->arBasic[i].sw_dis_scale_max1 = pCalib->DpccTuningPara.Expert_Mode.set1.RK.sw_dis_scale_max[i];
        pBasic->arBasic[i].line_mad_fac_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.LC.rb_line_mad_fac[i];
        pBasic->arBasic[i].line_mad_fac_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.LC.g_line_mad_fac[i];

        //pg_fac_1 0x0020
        pBasic->arBasic[i].pg_fac_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.PG.rb_pg_fac[i];
        pBasic->arBasic[i].pg_fac_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.PG.g_pg_fac[i];

        //rnd_thresh_1 0x0024
        pBasic->arBasic[i].rnd_thr_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.RND.rb_rnd_thr[i];
        pBasic->arBasic[i].rnd_thr_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.RND.g_rnd_thr[i];

        //rg_fac_1 0x0028
        pBasic->arBasic[i].rg_fac_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.RG.rb_rg_fac[i];
        pBasic->arBasic[i].rg_fac_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.RG.g_rg_fac[i];


        //line_thresh_2 0x002c
        pBasic->arBasic[i].sw_mindis2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.RK.rb_sw_mindis[i];
        pBasic->arBasic[i].sw_mindis2_g = pCalib->DpccTuningPara.Expert_Mode.set2.RK.g_sw_mindis[i];
        pBasic->arBasic[i].line_thr_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.LC.rb_line_thr[i];
        pBasic->arBasic[i].line_thr_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.LC.g_line_thr[i];

        //line_mad_fac_2 0x0030
        pBasic->arBasic[i].sw_dis_scale_min2 = pCalib->DpccTuningPara.Expert_Mode.set2.RK.sw_dis_scale_min[i];
        pBasic->arBasic[i].sw_dis_scale_max2 = pCalib->DpccTuningPara.Expert_Mode.set2.RK.sw_dis_scale_max[i];
        pBasic->arBasic[i].line_mad_fac_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.LC.rb_line_mad_fac[i];
        pBasic->arBasic[i].line_mad_fac_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.LC.g_line_mad_fac[i];

        //pg_fac_2 0x0034
        pBasic->arBasic[i].pg_fac_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.PG.rb_pg_fac[i];
        pBasic->arBasic[i].pg_fac_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.PG.g_pg_fac[i];

        //rnd_thresh_2 0x0038
        pBasic->arBasic[i].rnd_thr_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.RND.rb_rnd_thr[i];
        pBasic->arBasic[i].rnd_thr_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.RND.g_rnd_thr[i];

        //rg_fac_2 0x003c
        pBasic->arBasic[i].rg_fac_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.RG.rb_rg_fac[i];
        pBasic->arBasic[i].rg_fac_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.RG.g_rg_fac[i];


        //line_thresh_3 0x0040
        pBasic->arBasic[i].sw_mindis3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.RK.rb_sw_mindis[i];
        pBasic->arBasic[i].sw_mindis3_g = pCalib->DpccTuningPara.Expert_Mode.set3.RK.g_sw_mindis[i];
        pBasic->arBasic[i].line_thr_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.LC.rb_line_thr[i];
        pBasic->arBasic[i].line_thr_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.LC.g_line_thr[i];

        //line_mad_fac_3 0x0044
        pBasic->arBasic[i].sw_dis_scale_min3 = pCalib->DpccTuningPara.Expert_Mode.set3.RK.sw_dis_scale_min[i];
        pBasic->arBasic[i].sw_dis_scale_max3 = pCalib->DpccTuningPara.Expert_Mode.set3.RK.sw_dis_scale_max[i];
        pBasic->arBasic[i].line_mad_fac_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.LC.rb_line_mad_fac[i];
        pBasic->arBasic[i].line_mad_fac_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.LC.g_line_mad_fac[i];

        //pg_fac_3 0x0048
        pBasic->arBasic[i].pg_fac_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.PG.rb_pg_fac[i];
        pBasic->arBasic[i].pg_fac_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.PG.g_pg_fac[i];

        //rnd_thresh_3 0x004c
        pBasic->arBasic[i].rnd_thr_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.RND.rb_rnd_thr[i];
        pBasic->arBasic[i].rnd_thr_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.RND.g_rnd_thr[i];

        //rg_fac_3 0x0050
        pBasic->arBasic[i].rg_fac_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.RG.rb_rg_fac[i];
        pBasic->arBasic[i].rg_fac_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.RG.g_rg_fac[i];

        //ro_limits 0x0054
        pBasic->arBasic[i].ro_lim_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.RO.rb_ro_lim[i];
        pBasic->arBasic[i].ro_lim_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.RO.g_ro_lim[i];
        pBasic->arBasic[i].ro_lim_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.RO.rb_ro_lim[i];
        pBasic->arBasic[i].ro_lim_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.RO.g_ro_lim[i];
        pBasic->arBasic[i].ro_lim_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.RO.rb_ro_lim[i];
        pBasic->arBasic[i].ro_lim_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.RO.g_ro_lim[i];

        //rnd_offs 0x0058
        pBasic->arBasic[i].rnd_offs_3_rb = pCalib->DpccTuningPara.Expert_Mode.set3.RND.rb_rnd_offs[i];
        pBasic->arBasic[i].rnd_offs_3_g = pCalib->DpccTuningPara.Expert_Mode.set3.RND.g_rnd_offs[i];
        pBasic->arBasic[i].rnd_offs_2_rb = pCalib->DpccTuningPara.Expert_Mode.set2.RND.rb_rnd_offs[i];
        pBasic->arBasic[i].rnd_offs_2_g = pCalib->DpccTuningPara.Expert_Mode.set2.RND.g_rnd_offs[i];
        pBasic->arBasic[i].rnd_offs_1_rb = pCalib->DpccTuningPara.Expert_Mode.set1.RND.rb_rnd_offs[i];
        pBasic->arBasic[i].rnd_offs_1_g = pCalib->DpccTuningPara.Expert_Mode.set1.RND.g_rnd_offs[i];

    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;

}

AdpccResult_t dpcc_pdaf_params_init(Adpcc_pdaf_params_t *pPdaf, CalibDb_Dpcc_Pdaf_V20_t *pCalib)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pPdaf == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pCalib == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid input params", __FUNCTION__, __LINE__);
        return ret;
    }

    pPdaf->sw_pdaf_en = pCalib->en;

    for(int i = 0; i < 16; i++) {
        pPdaf->pdaf_point_en[i] = pCalib->point_en[i] ? 1 : 0;
        pPdaf->point[i].x = pCalib->point_x[i];
        pPdaf->point[i].y = pCalib->point_y[i];
    }

    pPdaf->pdaf_offsetx = pCalib->offsetx;
    pPdaf->pdaf_offsety  = pCalib->offsety;
    pPdaf->pdaf_wrapx = pCalib->wrapx;
    pPdaf->pdaf_wrapy = pCalib->wrapy;
    pPdaf->pdaf_wrapx_num = pCalib->wrapx_num;
    pPdaf->pdaf_wrapy_num = pCalib->wrapy_num;
    pPdaf->pdaf_forward_med = pCalib->forward_med;

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Api_Expert_mode_select(
    Adpcc_basic_cfg_params_t *pSelect,
    Adpcc_Manual_Attr_t *pstmaunal)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pstmaunal == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    //mode 0x0000
    pSelect->stage1_enable = pstmaunal->stOnfly.expert_mode.stage1_enable;
    pSelect->grayscale_mode = pstmaunal->stOnfly.expert_mode.grayscale_mode;
    pSelect->enable = pstmaunal->stOnfly.expert_mode.enable;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = pstmaunal->stOnfly.expert_mode.sw_rk_out_sel;
    pSelect->sw_dpcc_output_sel = pstmaunal->stOnfly.expert_mode.sw_dpcc_output_sel;
    pSelect->stage1_rb_3x3 = pstmaunal->stOnfly.expert_mode.stage1_rb_3x3;
    pSelect->stage1_g_3x3 = pstmaunal->stOnfly.expert_mode.stage1_g_3x3;
    pSelect->stage1_incl_rb_center = pstmaunal->stOnfly.expert_mode.stage1_incl_rb_center;
    pSelect->stage1_incl_green_center = pstmaunal->stOnfly.expert_mode.stage1_incl_green_center;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = pstmaunal->stOnfly.expert_mode.stage1_use_fix_set;
    pSelect->stage1_use_set_3 = pstmaunal->stOnfly.expert_mode.stage1_use_set_3;
    pSelect->stage1_use_set_2 = pstmaunal->stOnfly.expert_mode.stage1_use_set_2;
    pSelect->stage1_use_set_1 = pstmaunal->stOnfly.expert_mode.stage1_use_set_1;

    //methods_set_1 0x000c
    pSelect->sw_rk_red_blue1_en = pstmaunal->stOnfly.expert_mode.sw_rk_red_blue1_en;
    pSelect->rg_red_blue1_enable = pstmaunal->stOnfly.expert_mode.rg_red_blue1_enable;
    pSelect->rnd_red_blue1_enable = pstmaunal->stOnfly.expert_mode.rnd_red_blue1_enable;
    pSelect->ro_red_blue1_enable = pstmaunal->stOnfly.expert_mode.ro_red_blue1_enable;
    pSelect->lc_red_blue1_enable = pstmaunal->stOnfly.expert_mode.lc_red_blue1_enable;
    pSelect->pg_red_blue1_enable = pstmaunal->stOnfly.expert_mode.pg_red_blue1_enable;
    pSelect->sw_rk_green1_en = pstmaunal->stOnfly.expert_mode.sw_rk_green1_en;
    pSelect->rg_green1_enable = pstmaunal->stOnfly.expert_mode.rg_green1_enable;
    pSelect->rnd_green1_enable = pstmaunal->stOnfly.expert_mode.rnd_green1_enable;
    pSelect->ro_green1_enable = pstmaunal->stOnfly.expert_mode.ro_green1_enable;
    pSelect->lc_green1_enable = pstmaunal->stOnfly.expert_mode.lc_green1_enable;
    pSelect->pg_green1_enable = pstmaunal->stOnfly.expert_mode.pg_green1_enable;

    //methods_set_2 0x0010
    pSelect->sw_rk_red_blue2_en = pstmaunal->stOnfly.expert_mode.sw_rk_red_blue2_en;
    pSelect->rg_red_blue2_enable = pstmaunal->stOnfly.expert_mode.rg_red_blue2_enable;
    pSelect->rnd_red_blue2_enable = pstmaunal->stOnfly.expert_mode.rnd_red_blue2_enable;
    pSelect->ro_red_blue2_enable = pstmaunal->stOnfly.expert_mode.ro_red_blue2_enable;
    pSelect->lc_red_blue2_enable = pstmaunal->stOnfly.expert_mode.lc_red_blue2_enable;
    pSelect->pg_red_blue2_enable = pstmaunal->stOnfly.expert_mode.pg_red_blue2_enable;
    pSelect->sw_rk_green2_en = pstmaunal->stOnfly.expert_mode.sw_rk_green2_en;
    pSelect->rg_green2_enable = pstmaunal->stOnfly.expert_mode.rg_green2_enable;
    pSelect->rnd_green2_enable = pstmaunal->stOnfly.expert_mode.rnd_green2_enable;
    pSelect->ro_green2_enable = pstmaunal->stOnfly.expert_mode.ro_green2_enable;
    pSelect->lc_green2_enable = pstmaunal->stOnfly.expert_mode.lc_green2_enable;
    pSelect->pg_green2_enable = pstmaunal->stOnfly.expert_mode.pg_green2_enable;

    //methods_set_3 0x0014
    pSelect->sw_rk_red_blue3_en = pstmaunal->stOnfly.expert_mode.sw_rk_red_blue3_en;
    pSelect->rg_red_blue3_enable = pstmaunal->stOnfly.expert_mode.rg_red_blue3_enable;
    pSelect->rnd_red_blue3_enable = pstmaunal->stOnfly.expert_mode.rnd_red_blue3_enable;
    pSelect->ro_red_blue3_enable = pstmaunal->stOnfly.expert_mode.ro_red_blue3_enable;
    pSelect->lc_red_blue3_enable = pstmaunal->stOnfly.expert_mode.lc_red_blue3_enable;
    pSelect->pg_red_blue3_enable = pstmaunal->stOnfly.expert_mode.pg_red_blue3_enable;
    pSelect->sw_rk_green3_en = pstmaunal->stOnfly.expert_mode.sw_rk_green3_en;
    pSelect->rg_green3_enable = pstmaunal->stOnfly.expert_mode.rg_green3_enable;
    pSelect->rnd_green3_enable = pstmaunal->stOnfly.expert_mode.rnd_green3_enable;
    pSelect->ro_green3_enable = pstmaunal->stOnfly.expert_mode.ro_green3_enable;
    pSelect->lc_green3_enable = pstmaunal->stOnfly.expert_mode.lc_green3_enable;
    pSelect->pg_green3_enable = pstmaunal->stOnfly.expert_mode.pg_green3_enable;

    //line_thresh_1 0x0018
    pSelect->sw_mindis1_rb = pstmaunal->stOnfly.expert_mode.sw_mindis1_rb;
    pSelect->sw_mindis1_g = pstmaunal->stOnfly.expert_mode.sw_mindis1_g;
    pSelect->line_thr_1_rb = pstmaunal->stOnfly.expert_mode.line_thr_1_rb;
    pSelect->line_thr_1_g = pstmaunal->stOnfly.expert_mode.line_thr_1_g;

    //line_mad_fac_1 0x001c
    pSelect->sw_dis_scale_min1 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_min1;
    pSelect->sw_dis_scale_max1 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_max1;
    pSelect->line_mad_fac_1_rb = pstmaunal->stOnfly.expert_mode.line_mad_fac_1_rb;
    pSelect->line_mad_fac_1_g = pstmaunal->stOnfly.expert_mode.line_mad_fac_1_g;

    //pg_fac_1 0x0020
    pSelect->pg_fac_1_rb = pstmaunal->stOnfly.expert_mode.pg_fac_1_rb;
    pSelect->pg_fac_1_g = pstmaunal->stOnfly.expert_mode.pg_fac_1_g;

    //rnd_thresh_1 0x0024
    pSelect->rnd_thr_1_rb = pstmaunal->stOnfly.expert_mode.rnd_thr_1_rb;
    pSelect->rnd_thr_1_g = pstmaunal->stOnfly.expert_mode.rnd_thr_1_g;

    //rg_fac_1 0x0028
    pSelect->rg_fac_1_rb = pstmaunal->stOnfly.expert_mode.rg_fac_1_rb;
    pSelect->rg_fac_1_g = pstmaunal->stOnfly.expert_mode.rg_fac_1_g;


    //line_thresh_2 0x002c
    pSelect->sw_mindis2_rb = pstmaunal->stOnfly.expert_mode.sw_mindis2_rb;
    pSelect->sw_mindis2_g = pstmaunal->stOnfly.expert_mode.sw_mindis2_g;
    pSelect->line_thr_2_rb = pstmaunal->stOnfly.expert_mode.line_thr_2_rb;
    pSelect->line_thr_2_g = pstmaunal->stOnfly.expert_mode.line_thr_2_g;

    //line_mad_fac_2 0x0030
    pSelect->sw_dis_scale_min2 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_min2;
    pSelect->sw_dis_scale_max2 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_max2;
    pSelect->line_mad_fac_2_rb = pstmaunal->stOnfly.expert_mode.line_mad_fac_2_rb;
    pSelect->line_mad_fac_2_g = pstmaunal->stOnfly.expert_mode.line_mad_fac_2_g;

    //pg_fac_2 0x0034
    pSelect->pg_fac_2_rb = pstmaunal->stOnfly.expert_mode.pg_fac_2_rb;
    pSelect->pg_fac_2_g = pstmaunal->stOnfly.expert_mode.pg_fac_2_g;

    //rnd_thresh_2 0x0038
    pSelect->rnd_thr_2_rb = pstmaunal->stOnfly.expert_mode.rnd_thr_2_rb;
    pSelect->rnd_thr_2_g = pstmaunal->stOnfly.expert_mode.rnd_thr_2_g;

    //rg_fac_2 0x003c
    pSelect->rg_fac_2_rb = pstmaunal->stOnfly.expert_mode.rg_fac_2_rb;
    pSelect->rg_fac_2_g = pstmaunal->stOnfly.expert_mode.rg_fac_2_g;


    //line_thresh_3 0x0040
    pSelect->sw_mindis3_rb = pstmaunal->stOnfly.expert_mode.sw_mindis3_rb;
    pSelect->sw_mindis3_g = pstmaunal->stOnfly.expert_mode.sw_mindis3_g;
    pSelect->line_thr_3_rb = pstmaunal->stOnfly.expert_mode.line_thr_3_rb;
    pSelect->line_thr_3_g = pstmaunal->stOnfly.expert_mode.line_thr_3_g;

    //line_mad_fac_3 0x0044
    pSelect->sw_dis_scale_min3 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_min3;
    pSelect->sw_dis_scale_max3 = pstmaunal->stOnfly.expert_mode.sw_dis_scale_max3;
    pSelect->line_mad_fac_3_rb = pstmaunal->stOnfly.expert_mode.line_mad_fac_3_rb;
    pSelect->line_mad_fac_3_g = pstmaunal->stOnfly.expert_mode.line_mad_fac_3_g;

    //pg_fac_3 0x0048
    pSelect->pg_fac_3_rb = pstmaunal->stOnfly.expert_mode.pg_fac_3_rb;
    pSelect->pg_fac_3_g = pstmaunal->stOnfly.expert_mode.pg_fac_3_g;

    //rnd_thresh_3 0x004c
    pSelect->rnd_thr_3_rb = pstmaunal->stOnfly.expert_mode.rnd_thr_3_rb;
    pSelect->rnd_thr_3_g = pstmaunal->stOnfly.expert_mode.rnd_thr_3_g;

    //rg_fac_3 0x0050
    pSelect->rg_fac_3_rb = pstmaunal->stOnfly.expert_mode.rg_fac_3_rb;
    pSelect->rg_fac_3_g = pstmaunal->stOnfly.expert_mode.rg_fac_3_g;

    //ro_limits 0x0054
    pSelect->ro_lim_3_rb = pstmaunal->stOnfly.expert_mode.ro_lim_3_rb;
    pSelect->ro_lim_3_g = pstmaunal->stOnfly.expert_mode.ro_lim_3_g;
    pSelect->ro_lim_2_rb = pstmaunal->stOnfly.expert_mode.ro_lim_2_rb;
    pSelect->ro_lim_2_g = pstmaunal->stOnfly.expert_mode.ro_lim_2_g;
    pSelect->ro_lim_1_rb = pstmaunal->stOnfly.expert_mode.ro_lim_1_rb;
    pSelect->ro_lim_1_g = pstmaunal->stOnfly.expert_mode.ro_lim_1_g;

    //rnd_offs 0x0058
    pSelect->rnd_offs_3_rb = pstmaunal->stOnfly.expert_mode.rnd_offs_3_rb;
    pSelect->rnd_offs_3_g = pstmaunal->stOnfly.expert_mode.rnd_offs_3_g;
    pSelect->rnd_offs_2_rb = pstmaunal->stOnfly.expert_mode.rnd_offs_2_rb;
    pSelect->rnd_offs_2_g = pstmaunal->stOnfly.expert_mode.rnd_offs_2_g;
    pSelect->rnd_offs_1_rb = pstmaunal->stOnfly.expert_mode.rnd_offs_1_rb;
    pSelect->rnd_offs_1_g = pstmaunal->stOnfly.expert_mode.rnd_offs_1_g;


    LOGD_ADPCC("%s:(%d) %d %d %d %d %d", __FUNCTION__, __LINE__,
               pSelect->enable, pSelect->line_thr_2_g, pSelect->line_mad_fac_2_rb,
               pSelect->ro_lim_2_g, pSelect->rnd_offs_2_g);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Expert_mode_param_cpy(
    Adpcc_basic_cfg_params_t *pSelect,
    Adpcc_basic_params_select_t *pParams)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    //mode 0x0000
    pSelect->stage1_enable = pParams->stage1_enable;
    pSelect->grayscale_mode = pParams->grayscale_mode;
    pSelect->enable = pParams->enable;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = pParams->sw_rk_out_sel;
    pSelect->sw_dpcc_output_sel = pParams->sw_dpcc_output_sel;
    pSelect->stage1_rb_3x3 = pParams->stage1_rb_3x3;
    pSelect->stage1_g_3x3 = pParams->stage1_g_3x3;
    pSelect->stage1_incl_rb_center = pParams->stage1_incl_rb_center;
    pSelect->stage1_incl_green_center = pParams->stage1_incl_green_center;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = pParams->stage1_use_fix_set;
    pSelect->stage1_use_set_3 = pParams->stage1_use_set_3;
    pSelect->stage1_use_set_2 = pParams->stage1_use_set_2;
    pSelect->stage1_use_set_1 = pParams->stage1_use_set_1;

    //methods_set_1 0x000c
    pSelect->sw_rk_red_blue1_en = pParams->sw_rk_red_blue1_en;
    pSelect->rg_red_blue1_enable = pParams->rg_red_blue1_enable;
    pSelect->rnd_red_blue1_enable = pParams->rnd_red_blue1_enable;
    pSelect->ro_red_blue1_enable = pParams->ro_red_blue1_enable;
    pSelect->lc_red_blue1_enable = pParams->lc_red_blue1_enable;
    pSelect->pg_red_blue1_enable = pParams->pg_red_blue1_enable;
    pSelect->sw_rk_green1_en = pParams->sw_rk_green1_en;
    pSelect->rg_green1_enable = pParams->rg_green1_enable;
    pSelect->rnd_green1_enable = pParams->rnd_green1_enable;
    pSelect->ro_green1_enable = pParams->ro_green1_enable;
    pSelect->lc_green1_enable = pParams->lc_green1_enable;
    pSelect->pg_green1_enable = pParams->pg_green1_enable;

    //methods_set_2 0x0010
    pSelect->sw_rk_red_blue2_en = pParams->sw_rk_red_blue2_en;
    pSelect->rg_red_blue2_enable = pParams->rg_red_blue2_enable;
    pSelect->rnd_red_blue2_enable = pParams->rnd_red_blue2_enable;
    pSelect->ro_red_blue2_enable = pParams->ro_red_blue2_enable;
    pSelect->lc_red_blue2_enable = pParams->lc_red_blue2_enable;
    pSelect->pg_red_blue2_enable = pParams->pg_red_blue2_enable;
    pSelect->sw_rk_green2_en = pParams->sw_rk_green2_en;
    pSelect->rg_green2_enable = pParams->rg_green2_enable;
    pSelect->rnd_green2_enable = pParams->rnd_green2_enable;
    pSelect->ro_green2_enable = pParams->ro_green2_enable;
    pSelect->lc_green2_enable = pParams->lc_green2_enable;
    pSelect->pg_green2_enable = pParams->pg_green2_enable;

    //methods_set_3 0x0014
    pSelect->sw_rk_red_blue3_en = pParams->sw_rk_red_blue3_en;
    pSelect->rg_red_blue3_enable = pParams->rg_red_blue3_enable;
    pSelect->rnd_red_blue3_enable = pParams->rnd_red_blue3_enable;
    pSelect->ro_red_blue3_enable = pParams->ro_red_blue3_enable;
    pSelect->lc_red_blue3_enable = pParams->lc_red_blue3_enable;
    pSelect->pg_red_blue3_enable = pParams->pg_red_blue3_enable;
    pSelect->sw_rk_green3_en = pParams->sw_rk_green3_en;
    pSelect->rg_green3_enable = pParams->rg_green3_enable;
    pSelect->rnd_green3_enable = pParams->rnd_green3_enable;
    pSelect->ro_green3_enable = pParams->ro_green3_enable;
    pSelect->lc_green3_enable = pParams->lc_green3_enable;
    pSelect->pg_green3_enable = pParams->pg_green3_enable;

    //line_thresh_1 0x0018
    pSelect->sw_mindis1_rb = pParams->sw_mindis1_rb;
    pSelect->sw_mindis1_g = pParams->sw_mindis1_g;
    pSelect->line_thr_1_rb = pParams->line_thr_1_rb;
    pSelect->line_thr_1_g = pParams->line_thr_1_g;

    //line_mad_fac_1 0x001c
    pSelect->sw_dis_scale_min1 = pParams->sw_dis_scale_min1;
    pSelect->sw_dis_scale_max1 = pParams->sw_dis_scale_max1;
    pSelect->line_mad_fac_1_rb = pParams->line_mad_fac_1_rb;
    pSelect->line_mad_fac_1_g = pParams->line_mad_fac_1_g;

    //pg_fac_1 0x0020
    pSelect->pg_fac_1_rb = pParams->pg_fac_1_rb;
    pSelect->pg_fac_1_g = pParams->pg_fac_1_g;

    //rnd_thresh_1 0x0024
    pSelect->rnd_thr_1_rb = pParams->rnd_thr_1_rb;
    pSelect->rnd_thr_1_g = pParams->rnd_thr_1_g;

    //rg_fac_1 0x0028
    pSelect->rg_fac_1_rb = pParams->rg_fac_1_rb;
    pSelect->rg_fac_1_g = pParams->rg_fac_1_g;


    //line_thresh_2 0x002c
    pSelect->sw_mindis2_rb = pParams->sw_mindis2_rb;
    pSelect->sw_mindis2_g = pParams->sw_mindis2_g;
    pSelect->line_thr_2_rb = pParams->line_thr_2_rb;
    pSelect->line_thr_2_g = pParams->line_thr_2_g;

    //line_mad_fac_2 0x0030
    pSelect->sw_dis_scale_min2 = pParams->sw_dis_scale_min2;
    pSelect->sw_dis_scale_max2 = pParams->sw_dis_scale_max2;
    pSelect->line_mad_fac_2_rb = pParams->line_mad_fac_2_rb;
    pSelect->line_mad_fac_2_g = pParams->line_mad_fac_2_g;

    //pg_fac_2 0x0034
    pSelect->pg_fac_2_rb = pParams->pg_fac_2_rb;
    pSelect->pg_fac_2_g = pParams->pg_fac_2_g;

    //rnd_thresh_2 0x0038
    pSelect->rnd_thr_2_rb = pParams->rnd_thr_2_rb;
    pSelect->rnd_thr_2_g = pParams->rnd_thr_2_g;

    //rg_fac_2 0x003c
    pSelect->rg_fac_2_rb = pParams->rg_fac_2_rb;
    pSelect->rg_fac_2_g = pParams->rg_fac_2_g;


    //line_thresh_3 0x0040
    pSelect->sw_mindis3_rb = pParams->sw_mindis3_rb;
    pSelect->sw_mindis3_g = pParams->sw_mindis3_g;
    pSelect->line_thr_3_rb = pParams->line_thr_3_rb;
    pSelect->line_thr_3_g = pParams->line_thr_3_g;

    //line_mad_fac_3 0x0044
    pSelect->sw_dis_scale_min3 = pParams->sw_dis_scale_min3;
    pSelect->sw_dis_scale_max3 = pParams->sw_dis_scale_max3;
    pSelect->line_mad_fac_3_rb = pParams->line_mad_fac_3_rb;
    pSelect->line_mad_fac_3_g = pParams->line_mad_fac_3_g;

    //pg_fac_3 0x0048
    pSelect->pg_fac_3_rb = pParams->pg_fac_3_rb;
    pSelect->pg_fac_3_g = pParams->pg_fac_3_g;

    //rnd_thresh_3 0x004c
    pSelect->rnd_thr_3_rb = pParams->rnd_thr_3_rb;
    pSelect->rnd_thr_3_g = pParams->rnd_thr_3_g;

    //rg_fac_3 0x0050
    pSelect->rg_fac_3_rb = pParams->rg_fac_3_rb;
    pSelect->rg_fac_3_g = pParams->rg_fac_3_g;

    //ro_limits 0x0054
    pSelect->ro_lim_3_rb = pParams->ro_lim_3_rb;
    pSelect->ro_lim_3_g = pParams->ro_lim_3_g;
    pSelect->ro_lim_2_rb = pParams->ro_lim_2_rb;
    pSelect->ro_lim_2_g = pParams->ro_lim_2_g;
    pSelect->ro_lim_1_rb = pParams->ro_lim_1_rb;
    pSelect->ro_lim_1_g = pParams->ro_lim_1_g;

    //rnd_offs 0x0058
    pSelect->rnd_offs_3_rb = pParams->rnd_offs_3_rb;
    pSelect->rnd_offs_3_g = pParams->rnd_offs_3_g;
    pSelect->rnd_offs_2_rb = pParams->rnd_offs_2_rb;
    pSelect->rnd_offs_2_g = pParams->rnd_offs_2_g;
    pSelect->rnd_offs_1_rb = pParams->rnd_offs_1_rb;
    pSelect->rnd_offs_1_g = pParams->rnd_offs_1_g;


    LOGD_ADPCC("%s:(%d) %d %d %d %d %d", __FUNCTION__, __LINE__,
               pSelect->enable, pSelect->line_thr_2_g, pSelect->line_mad_fac_2_rb,
               pSelect->ro_lim_2_g, pSelect->rnd_offs_2_g);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Expert_mode_select_basic_params_by_ISO(
    Adpcc_basic_params_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    AdpccExpInfo_t *pExpInfo)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    int lowLevel = 0;
    int highLevel = 0;
    int lowIso = 0;
    int highIso = 0;
    float ratio = 0.0;
    int iso = 50;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pExpInfo == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    iso = pExpInfo->arPreResIso[pExpInfo->hdr_mode];

    for(int i = 0; i < DPCC_MAX_ISO_LEVEL - 1; i++) {
        LOGD_ADPCC("%s:(%d) iso:%d %d %d", __FUNCTION__, __LINE__,
                   iso, pParams->arBasic[i].iso, pParams->arBasic[i + 1].iso);
        if(iso >= pParams->arBasic[i].iso && iso < pParams->arBasic[i + 1].iso) {
            lowLevel = i;
            highLevel = i + 1;
            lowIso = pParams->arBasic[i].iso;
            highIso = pParams->arBasic[i + 1].iso;
            //ratio = (iso -lowIso ) / (float)(highIso - lowIso);
            ratio = 0.0;
            break;
        }
    }

    if(iso < pParams->arBasic[0].iso ) {
        lowLevel = 0;
        highLevel = 0;
        ratio = 0.0;
    }

    if(iso >= pParams->arBasic[DPCC_MAX_ISO_LEVEL - 1].iso ) {
        lowLevel = DPCC_MAX_ISO_LEVEL - 1;
        highLevel = DPCC_MAX_ISO_LEVEL - 1;
        ratio = 1.0;
    }

    LOGD_ADPCC("%s:(%d) iso:%d lowLevel:%d hightLevel:%d lowIso:%d HighIso:%d ratio:%f",
               __FUNCTION__, __LINE__, iso, lowLevel, highLevel, lowIso, highIso, ratio);

    // copy param from pParams->arBasic[lowLevel]
    ret = Expert_mode_param_cpy(pSelect, &pParams->arBasic[lowLevel]);
    if (ret != ADPCC_RET_SUCCESS) {
        LOGE_ADPCC("%s(%d): failed to copy Expert mode params", __FUNCTION__, __LINE__);
        return ret;
    }

    LOGD_ADPCC("%s:(%d) %d %d %d %d %d", __FUNCTION__, __LINE__,
               pSelect->enable, pSelect->line_thr_2_g, pSelect->line_mad_fac_2_rb,
               pSelect->ro_lim_2_g, pSelect->rnd_offs_2_g);

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

void Fast_mode_Triple_level_Setting(
    Adpcc_basic_cfg_params_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 1;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0x3;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0x3;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 2:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 1;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0x2;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0x2;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 3:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x3;
        pSelect->pg_fac_3_g = 0x3;
        break;
    case 4:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 1;
        pSelect->pg_green3_enable = 1;
        pSelect->pg_fac_3_rb = 0x2;
        pSelect->pg_fac_3_g = 0x2;
        break;
    case 5:
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 6:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 7:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x5;
        pSelect->sw_mindis3_g = 0x5;
        pSelect->sw_dis_scale_min3 = 0x3;
        pSelect->sw_dis_scale_max3 = 0x3;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 4;
        pSelect->rnd_thr_3_g = 4;
        pSelect->rnd_offs_3_rb = 2;
        pSelect->rnd_offs_3_g = 2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 2;
        pSelect->ro_lim_3_g = 2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 8:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 1;
        pSelect->sw_rk_green3_en = 1;
        pSelect->sw_mindis3_rb = 0x3;
        pSelect->sw_mindis3_g = 0x3;
        pSelect->sw_dis_scale_min3 = 0x2;
        pSelect->sw_dis_scale_max3 = 0x2;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x4;
        pSelect->rnd_thr_3_g = 0x4;
        pSelect->rnd_offs_3_rb = 0x2;
        pSelect->rnd_offs_3_g = 0x2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 9:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 0;
        pSelect->sw_rk_green3_en = 0;
        pSelect->sw_mindis3_rb = 0;
        pSelect->sw_mindis3_g = 0;
        pSelect->sw_dis_scale_min3 = 0;
        pSelect->sw_dis_scale_max3 = 0;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x4;
        pSelect->rnd_thr_3_g = 0x4;
        pSelect->rnd_offs_3_rb = 0x2;
        pSelect->rnd_offs_3_g = 0x2;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x2;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    case 10:
        pSelect->stage1_use_fix_set = 0x1;
        pSelect->sw_rk_red_blue3_en = 0;
        pSelect->sw_rk_green3_en = 0;
        pSelect->sw_mindis3_rb = 0;
        pSelect->sw_mindis3_g = 0;
        pSelect->sw_dis_scale_min3 = 0;
        pSelect->sw_dis_scale_max3 = 0;

        pSelect->rg_red_blue3_enable = 0;
        pSelect->rg_green3_enable = 0;
        pSelect->rg_fac_3_rb = 0;
        pSelect->rg_fac_3_g = 0;

        pSelect->rnd_red_blue3_enable = 1;
        pSelect->rnd_green3_enable = 1;
        pSelect->rnd_thr_3_rb = 0x3;
        pSelect->rnd_thr_3_g = 0x3;
        pSelect->rnd_offs_3_rb = 0x1;
        pSelect->rnd_offs_3_g = 0x1;

        pSelect->ro_red_blue3_enable = 1;
        pSelect->ro_green3_enable = 1;
        pSelect->ro_lim_3_rb = 0x2;
        pSelect->ro_lim_3_g = 0x1;

        pSelect->lc_red_blue3_enable = 0;
        pSelect->lc_green3_enable = 0;
        pSelect->line_thr_3_rb = 0;
        pSelect->line_thr_3_g = 0;
        pSelect->line_mad_fac_3_rb = 0;
        pSelect->line_mad_fac_3_g = 0;

        pSelect->pg_red_blue3_enable = 0;
        pSelect->pg_green3_enable = 0;
        pSelect->pg_fac_3_rb = 0;
        pSelect->pg_fac_3_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Triple_Setting(
    AdpccContext_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    int iso)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    int level = AdpccInterpolation(iso, pParams->stAuto.stFastMode.ISO, pParams->stAuto.stFastMode.fast_mode_triple_level);

    if(pParams->stAuto.stFastMode.fast_mode_triple_en != 0)
        pSelect->stage1_use_set_3 = 0x1;
    else
        pSelect->stage1_use_set_3 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Triple_level_Setting(pSelect, level);

    LOGD_ADPCC("%s(%d): Dpcc fast mode triple en:%d level:%d", __FUNCTION__, __LINE__,
               pParams->stAuto.stFastMode.fast_mode_triple_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Double_level_Setting(
    Adpcc_basic_cfg_params_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x15;
        pSelect->sw_mindis2_g = 0x20;
        pSelect->sw_dis_scale_min2 = 0x10;
        pSelect->sw_dis_scale_max2 = 0x12;

        pSelect->rg_red_blue2_enable = 1;
        pSelect->rg_green2_enable = 1;
        pSelect->rg_fac_2_rb = 0x15;
        pSelect->rg_fac_2_g = 0x20;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x9;
        pSelect->rnd_thr_2_g = 0xa;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x1;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x6;
        pSelect->line_thr_2_g = 0x8;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x4;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_2_rb = 0x6;
        pSelect->pg_fac_2_g = 0x8;
        break;
    case 2:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 1;
        pSelect->rg_green2_enable = 1;
        pSelect->rg_fac_2_rb = 0x7;
        pSelect->rg_fac_2_g = 0x10;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x5;
        pSelect->rnd_thr_2_g = 0x6;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 3:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 1;
        pSelect->rnd_green2_enable = 1;
        pSelect->rnd_thr_2_rb = 0x5;
        pSelect->rnd_thr_2_g = 0x6;
        pSelect->rnd_offs_2_rb = 0x1;
        pSelect->rnd_offs_2_g = 0x1;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 4:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x12;
        pSelect->sw_mindis2_g = 0x16;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x2;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x12;
        pSelect->line_thr_2_g = 0x16;
        pSelect->line_mad_fac_2_rb = 0x8;
        pSelect->line_mad_fac_2_g = 0x10;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x5;
        pSelect->pg_fac_2_g = 0x6;
        break;
    case 5:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x10;
        pSelect->sw_mindis2_g = 0x14;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0xc;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x7;
        pSelect->line_thr_2_g = 0xc;
        pSelect->line_mad_fac_2_rb = 0x7;
        pSelect->line_mad_fac_2_g = 0x9;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x3;
        pSelect->pg_fac_2_g = 0x4;
        break;
    case 6:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x7;
        pSelect->sw_mindis2_g = 0x10;
        pSelect->sw_dis_scale_min2 = 0x6;
        pSelect->sw_dis_scale_max2 = 0x8;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x3;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x7;
        pSelect->line_thr_2_g = 0x9;
        pSelect->line_mad_fac_2_rb = 0x5;
        pSelect->line_mad_fac_2_g = 0x7;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x3;
        pSelect->pg_fac_2_g = 0x4;
        break;
    case 7:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x5;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x5;
        pSelect->line_thr_2_g = 0x7;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x5;

        pSelect->pg_red_blue2_enable = 1;
        pSelect->pg_green2_enable = 1;
        pSelect->pg_fac_2_rb = 0x2;
        pSelect->pg_fac_2_g = 0x1;
        break;
    case 8:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x5;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x1;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x5;
        pSelect->line_thr_2_g = 0x7;
        pSelect->line_mad_fac_2_rb = 0x3;
        pSelect->line_mad_fac_2_g = 0x5;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    case 9:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x4;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x2;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 1;
        pSelect->lc_green2_enable = 1;
        pSelect->line_thr_2_rb = 0x1;
        pSelect->line_thr_2_g = 0x3;
        pSelect->line_mad_fac_2_rb = 0x2;
        pSelect->line_mad_fac_2_g = 0x2;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    case 10:
        pSelect->sw_rk_red_blue2_en = 1;
        pSelect->sw_rk_green2_en = 1;
        pSelect->sw_mindis2_rb = 0x4;
        pSelect->sw_mindis2_g = 0x8;
        pSelect->sw_dis_scale_min2 = 0x3;
        pSelect->sw_dis_scale_max2 = 0x6;

        pSelect->rg_red_blue2_enable = 0;
        pSelect->rg_green2_enable = 0;
        pSelect->rg_fac_2_rb = 0;
        pSelect->rg_fac_2_g = 0;

        pSelect->rnd_red_blue2_enable = 0;
        pSelect->rnd_green2_enable = 0;
        pSelect->rnd_thr_2_rb = 0;
        pSelect->rnd_thr_2_g = 0;
        pSelect->rnd_offs_2_rb = 0;
        pSelect->rnd_offs_2_g = 0;

        pSelect->ro_red_blue2_enable = 1;
        pSelect->ro_green2_enable = 1;
        pSelect->ro_lim_2_rb = 0x4;
        pSelect->ro_lim_2_g = 0x3;

        pSelect->lc_red_blue2_enable = 0;
        pSelect->lc_green2_enable = 0;
        pSelect->line_thr_2_rb = 0;
        pSelect->line_thr_2_g = 0;
        pSelect->line_mad_fac_2_rb = 0;
        pSelect->line_mad_fac_2_g = 0;

        pSelect->pg_red_blue2_enable = 0;
        pSelect->pg_green2_enable = 0;
        pSelect->pg_fac_2_rb = 0;
        pSelect->pg_fac_2_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Double_Setting(
    AdpccContext_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    int iso)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);


    int level = AdpccInterpolation(iso, pParams->stAuto.stFastMode.ISO, pParams->stAuto.stFastMode.fast_mode_double_level);

    if(pParams->stAuto.stFastMode.fast_mode_double_en != 0)
        pSelect->stage1_use_set_2 = 0x1;
    else
        pSelect->stage1_use_set_2 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Double_level_Setting(pSelect, level);

    LOGD_ADPCC("%s(%d): Dpcc fast mode double en:%d level:%d", __FUNCTION__, __LINE__,
               pParams->stAuto.stFastMode.fast_mode_double_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Single_level_Setting(
    Adpcc_basic_cfg_params_t *pSelect,
    int level)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    switch (level)
    {
    case 1:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x20;
        pSelect->sw_mindis1_g = 0x20;
        pSelect->sw_dis_scale_min1 = 0x12;
        pSelect->sw_dis_scale_max1 = 0x12;

        pSelect->rg_red_blue1_enable = 1;
        pSelect->rg_green1_enable = 1;
        pSelect->rg_fac_1_rb = 0x20;
        pSelect->rg_fac_1_g = 0x20;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0xa;
        pSelect->rnd_thr_1_g = 0xa;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x8;
        pSelect->line_thr_1_g = 0x8;
        pSelect->line_mad_fac_1_rb = 0x4;
        pSelect->line_mad_fac_1_g = 0x4;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x8;
        pSelect->pg_fac_1_g = 0x8;
        break;
    case 2:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 1;
        pSelect->rg_green1_enable = 1;
        pSelect->rg_fac_1_rb = 0x10;
        pSelect->rg_fac_1_g = 0x10;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0x6;
        pSelect->rnd_thr_1_g = 0x6;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 3:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 1;
        pSelect->rnd_green1_enable = 1;
        pSelect->rnd_thr_1_rb = 0x6;
        pSelect->rnd_thr_1_g = 0x6;
        pSelect->rnd_offs_1_rb = 0x1;
        pSelect->rnd_offs_1_g = 0x1;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 4:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x16;
        pSelect->sw_mindis1_g = 0x16;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x2;
        pSelect->ro_lim_1_g = 0x2;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x16;
        pSelect->line_thr_1_g = 0x16;
        pSelect->line_mad_fac_1_rb = 0x10;
        pSelect->line_mad_fac_1_g = 0x10;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x6;
        pSelect->pg_fac_1_g = 0x6;
        break;
    case 5:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x14;
        pSelect->sw_mindis1_g = 0x14;
        pSelect->sw_dis_scale_min1 = 0xc;
        pSelect->sw_dis_scale_max1 = 0xc;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0xc;
        pSelect->line_thr_1_g = 0xc;
        pSelect->line_mad_fac_1_rb = 0x9;
        pSelect->line_mad_fac_1_g = 0x9;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x5;
        pSelect->pg_fac_1_g = 0x4;
        break;
    case 6:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x10;
        pSelect->sw_mindis1_g = 0x10;
        pSelect->sw_dis_scale_min1 = 0x8;
        pSelect->sw_dis_scale_max1 = 0x8;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x9;
        pSelect->line_thr_1_g = 0x9;
        pSelect->line_mad_fac_1_rb = 0x7;
        pSelect->line_mad_fac_1_g = 0x7;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x5;
        pSelect->pg_fac_1_g = 0x4;
        break;
    case 7:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x7;
        pSelect->line_thr_1_g = 0x7;
        pSelect->line_mad_fac_1_rb = 0x5;
        pSelect->line_mad_fac_1_g = 0x5;

        pSelect->pg_red_blue1_enable = 1;
        pSelect->pg_green1_enable = 1;
        pSelect->pg_fac_1_rb = 0x3;
        pSelect->pg_fac_1_g = 0x1;
        break;
    case 8:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x7;
        pSelect->line_thr_1_g = 0x7;
        pSelect->line_mad_fac_1_rb = 0x5;
        pSelect->line_mad_fac_1_g = 0x5;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    case 9:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 1;
        pSelect->lc_green1_enable = 1;
        pSelect->line_thr_1_rb = 0x3;
        pSelect->line_thr_1_g = 0x3;
        pSelect->line_mad_fac_1_rb = 0x2;
        pSelect->line_mad_fac_1_g = 0x2;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    case 10:
        pSelect->sw_rk_red_blue1_en = 1;
        pSelect->sw_rk_green1_en = 1;
        pSelect->sw_mindis1_rb = 0x8;
        pSelect->sw_mindis1_g = 0x8;
        pSelect->sw_dis_scale_min1 = 0x6;
        pSelect->sw_dis_scale_max1 = 0x6;

        pSelect->rg_red_blue1_enable = 0;
        pSelect->rg_green1_enable = 0;
        pSelect->rg_fac_1_rb = 0;
        pSelect->rg_fac_1_g = 0;

        pSelect->rnd_red_blue1_enable = 0;
        pSelect->rnd_green1_enable = 0;
        pSelect->rnd_thr_1_rb = 0;
        pSelect->rnd_thr_1_g = 0;
        pSelect->rnd_offs_1_rb = 0;
        pSelect->rnd_offs_1_g = 0;

        pSelect->ro_red_blue1_enable = 1;
        pSelect->ro_green1_enable = 1;
        pSelect->ro_lim_1_rb = 0x3;
        pSelect->ro_lim_1_g = 0x3;

        pSelect->lc_red_blue1_enable = 0;
        pSelect->lc_green1_enable = 0;
        pSelect->line_thr_1_rb = 0;
        pSelect->line_thr_1_g = 0;
        pSelect->line_mad_fac_1_rb = 0;
        pSelect->line_mad_fac_1_g = 0;

        pSelect->pg_red_blue1_enable = 0;
        pSelect->pg_green1_enable = 0;
        pSelect->pg_fac_1_rb = 0;
        pSelect->pg_fac_1_g = 0;
        break;
    default:
        LOGE_ADPCC("%s(%d): Wrong fast mode level!!!", __FUNCTION__, __LINE__);
        break;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Fast_mode_Single_Setting(
    AdpccContext_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    int iso)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    int level = AdpccInterpolation(iso, pParams->stAuto.stFastMode.ISO, pParams->stAuto.stFastMode.fast_mode_single_level);

    if(pParams->stAuto.stFastMode.fast_mode_single_en != 0)
        pSelect->stage1_use_set_1 = 0x1;
    else
        pSelect->stage1_use_set_1 = 0x0;

    level = LIMIT_VALUE(level, FASTMODELEVELMAX, FASTMODELEVELMIN);
    Fast_mode_Single_level_Setting(pSelect, level);


    LOGD_ADPCC("%s(%d): Dpcc fast mode single en:%d level:%d", __FUNCTION__, __LINE__, pParams->stAuto.stFastMode.fast_mode_single_en, level);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

AdpccResult_t Fast_mode_select_basic_params_by_ISO(
    AdpccContext_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    AdpccExpInfo_t *pExpInfo)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;
    int iso = 50;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pExpInfo == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    iso = pExpInfo->arPreResIso[pExpInfo->hdr_mode];

    //set dpcc ctrl params
    //mode 0x0000
    pSelect->stage1_enable = 1;

    if(pParams->isBlackSensor)
        pSelect->grayscale_mode = 0x1;
    else
        pSelect->grayscale_mode = 0x0;

    pSelect->enable = 0x1;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = 1;
    pSelect->sw_dpcc_output_sel = 1;
    pSelect->stage1_rb_3x3 = 0;
    pSelect->stage1_g_3x3 = 0;
    pSelect->stage1_incl_rb_center = 1;
    pSelect->stage1_incl_green_center = 1;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = 0x0;

    //get current fast mode single level
    Fast_mode_Single_Setting(pParams, pSelect, iso);

    //get current fast mode double level
    Fast_mode_Double_Setting(pParams, pSelect, iso);

    //get current fast mode triple level
    Fast_mode_Triple_Setting(pParams, pSelect, iso);


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Api_Fast_mode_select(
    AdpccContext_t *pParams,
    Adpcc_basic_cfg_params_t *pSelect,
    Adpcc_fast_mode_attr_t *pAttr)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pAttr == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    //set dpcc ctrl params
    //mode 0x0000
    pSelect->stage1_enable = 1;

    if(pParams->isBlackSensor)
        pSelect->grayscale_mode = 0x1;
    else
        pSelect->grayscale_mode = 0x0;

    pSelect->enable = 0x1;

    //output_mode 0x0004
    pSelect->sw_rk_out_sel = 1;
    pSelect->sw_dpcc_output_sel = 1;
    pSelect->stage1_rb_3x3 = 0;
    pSelect->stage1_g_3x3 = 0;
    pSelect->stage1_incl_rb_center = 1;
    pSelect->stage1_incl_green_center = 1;

    //set_use 0x0008
    pSelect->stage1_use_fix_set = 0x0;

    //get current fast mode single level
    if(pAttr->fast_mode_single_en)
        pSelect->stage1_use_set_1 = 0x1;
    Fast_mode_Single_level_Setting(pSelect, pAttr->fast_mode_single_level);

    //get current fast mode double level
    if(pAttr->fast_mode_double_en)
        pSelect->stage1_use_set_2 = 0x1;
    Fast_mode_Double_level_Setting(pSelect, pAttr->fast_mode_double_level);

    //get current fast mode triple level
    if(pAttr->fast_mode_triple_en)
        pSelect->stage1_use_set_3 = 0x1;
    Fast_mode_Triple_level_Setting(pSelect, pAttr->fast_mode_triple_level);


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Api_select_bpt_params(
    Adpcc_bpt_params_t *pSelect,
    Adpcc_Manual_Attr_t *pstmaunal)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pstmaunal == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, &pstmaunal->stBpt, sizeof(Adpcc_bpt_params_t));

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t select_bpt_params_by_ISO(
    Adpcc_bpt_params_t *pParams,
    Adpcc_bpt_params_t *pSelect,
    AdpccExpInfo_t *pExpInfo)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pExpInfo == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, pParams, sizeof(Adpcc_bpt_params_t));

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t Api_select_pdaf_params(
    Adpcc_pdaf_params_t *pSelect,
    Adpcc_Manual_Attr_t *pstmaunal)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pstmaunal == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, &pstmaunal->stPdaf, sizeof(Adpcc_pdaf_params_t));

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t select_pdaf_params_by_ISO(
    Adpcc_pdaf_params_t *pParams,
    Adpcc_pdaf_params_t *pSelect,
    AdpccExpInfo_t *pExpInfo)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pExpInfo == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, pParams, sizeof(Adpcc_pdaf_params_t));

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

AdpccResult_t DpccExpertMode(
    Adpcc_pdaf_params_t *pParams,
    Adpcc_pdaf_params_t *pSelect,
    AdpccExpInfo_t *pExpInfo)
{
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pSelect == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ret;
    }

    if(pExpInfo == NULL) {
        ret = ADPCC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return ret;
    }

    memcpy(pSelect, pParams, sizeof(Adpcc_pdaf_params_t));

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}

int GetCurrDpccValue
(
    int           inPara,
    float           max,
    float         inMatrixX[13],
    float         inMatrixY[13]
) {
    LOG1_ADPCC( "%s:enter!", __FUNCTION__);
    float x1 = 0.0f;
    float x2 = 0.0f;
    float value1 = 0.0f;
    float value2 = 0.0f;
    float outPara = 0.0f;

    for(int i = 0; i < 12; i++)
        inMatrixY[i] > max ? inMatrixY[i] = max : inMatrixY[i] = inMatrixY[i] ;

    if(inPara < inMatrixX[0])
        outPara = inMatrixY[0];
    else if (inPara >= inMatrixX[12])
        outPara = inMatrixY[12];
    else
        for(int i = 0 ; i < 11;)
        {
            if(inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1])
            {
                x1 = inMatrixX[i];
                x2 = inMatrixX[i + 1];
                value1 = inMatrixY[i];
                value2 = inMatrixY[i + 1];
                outPara = value1 + (inPara - x1) * (value1 - value2) / (x1 - x2);
                break;
            }
            else
                i++;
        }

    int out = (int)(outPara + 0.5);

    return out;
    LOG1_ADPCC( "%s:exit!", __FUNCTION__);
}

void Api_Sensor_dpcc_process(AdpccContext_t *pAdpccCtx)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    pAdpccCtx->SenDpccRes.enable = pAdpccCtx->stManual.stSensorDpcc.en;
    pAdpccCtx->SenDpccRes.cur_single_dpcc = pAdpccCtx->stManual.stSensorDpcc.single_level;
    pAdpccCtx->SenDpccRes.cur_multiple_dpcc = pAdpccCtx->stManual.stSensorDpcc.double_level;
    pAdpccCtx->SenDpccRes.total_dpcc = pAdpccCtx->stManual.stSensorDpcc.max_level;

    if (pAdpccCtx->SenDpccRes.enable == false) {
        LOGD_ADPCC("%s(%d):sensor dpcc setting off!!", __FUNCTION__, __LINE__);
    } else {
        LOGD_ADPCC("%s(%d):sensor dpcc setting on!!", __FUNCTION__, __LINE__);
    }

    LOGD_ADPCC("%s(%d):Api sensor dpcc cur_s_dpcc:%d cur_m_dpcc:%d total_dpcc:%d!!", __FUNCTION__, __LINE__,
               pAdpccCtx->SenDpccRes.cur_single_dpcc, pAdpccCtx->SenDpccRes.cur_multiple_dpcc, pAdpccCtx->SenDpccRes.total_dpcc);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

void Sensor_dpcc_process(AdpccContext_t *pAdpccCtx)
{


    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    int iso = 0;
    if(pAdpccCtx->stExpInfo.hdr_mode == 0)
        iso = pAdpccCtx->stExpInfo.arProcResIso[0];
    else if(pAdpccCtx->stExpInfo.hdr_mode == 1)
        iso = pAdpccCtx->stExpInfo.arProcResIso[1];
    else if(pAdpccCtx->stExpInfo.hdr_mode == 2)
        iso = pAdpccCtx->stExpInfo.arProcResIso[2];

    float sensor_dpcc_level_single = GetCurrDpccValue(iso, pAdpccCtx->stAuto.stSensorDpcc.max_level,
                                     pAdpccCtx->stAuto.stSensorDpcc.iso, pAdpccCtx->stAuto.stSensorDpcc.level_single);
    float sensor_dpcc_level_multi = GetCurrDpccValue(iso, pAdpccCtx->stAuto.stSensorDpcc.max_level,
                                    pAdpccCtx->stAuto.stSensorDpcc.iso, pAdpccCtx->stAuto.stSensorDpcc.level_multiple);

    pAdpccCtx->SenDpccRes.enable = pAdpccCtx->stAuto.stSensorDpcc.en;
    pAdpccCtx->SenDpccRes.cur_single_dpcc = (int)(sensor_dpcc_level_single + 0.5);
    pAdpccCtx->SenDpccRes.cur_multiple_dpcc = (int)(sensor_dpcc_level_multi + 0.5);
    pAdpccCtx->SenDpccRes.total_dpcc = (int)(pAdpccCtx->stAuto.stSensorDpcc.max_level + 0.5);

    if (pAdpccCtx->SenDpccRes.enable == false) {
        LOGD_ADPCC("%s(%d):sensor dpcc setting off!!", __FUNCTION__, __LINE__);
    } else {
        LOGD_ADPCC("%s(%d):sensor dpcc setting on!!", __FUNCTION__, __LINE__);
    }

    LOGD_ADPCC("%s(%d):ISO:%d sensor dpcc cur_s_dpcc:%d cur_m_dpcc:%d total_dpcc:%d!!", __FUNCTION__, __LINE__, iso,
               pAdpccCtx->SenDpccRes.cur_single_dpcc, pAdpccCtx->SenDpccRes.cur_multiple_dpcc, pAdpccCtx->SenDpccRes.total_dpcc);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);

}

AdpccResult_t AdpccReloadPara(AdpccContext_t *pAdpccCtx, CamCalibDbV2Context_t *pCalibDb)
{
    LOG1_ADPCC(" %s(%d): enter!", __FUNCTION__, __LINE__);
    LOGD_ADPCC(" %s(%d): Adpcc Reload Para, prepare type is %d!", __FUNCTION__, __LINE__, pAdpccCtx->prepare_type);

    //init fix param for algo
    CalibDbV2_Dpcc_t* calibv2_adpcc_calib =
        (CalibDbV2_Dpcc_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adpcc_calib));
    memcpy(&pAdpccCtx->stDpccCalib, calibv2_adpcc_calib, sizeof(CalibDbV2_Dpcc_t));//reload iq
    memcpy(&pAdpccCtx->stTool, calibv2_adpcc_calib, sizeof(CalibDbV2_Dpcc_t));
    dpcc_expert_mode_basic_params_init(&pAdpccCtx->stAuto.stBasicParams, &pAdpccCtx->stDpccCalib);
    dpcc_fast_mode_basic_params_init(&pAdpccCtx->stAuto.stFastMode, &pAdpccCtx->stDpccCalib);
    dpcc_pdaf_params_init(&pAdpccCtx->stAuto.stPdafParams, &pAdpccCtx->stDpccCalib.DpccTuningPara.Dpcc_pdaf);
    dpcc_sensor_params_init(&pAdpccCtx->stAuto.stSensorDpcc, &pAdpccCtx->stDpccCalib);

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;
}

AdpccResult_t AdpccInit(AdpccContext_t **ppAdpccCtx, CamCalibDbV2Context_t *pCalibDb)
{
    AdpccContext_t * pAdpccCtx;

    LOG1_ADPCC(" %s(%d): enter!", __FUNCTION__, __LINE__);

    pAdpccCtx = (AdpccContext_t *)malloc(sizeof(AdpccContext_t));
    if(pAdpccCtx == NULL) {
        LOGE_ADPCC("%s(%d): invalid inputparams", __FUNCTION__, __LINE__);
        return ADPCC_RET_NULL_POINTER;
    }

    memset(pAdpccCtx, 0x00, sizeof(AdpccContext_t));
    pAdpccCtx->eState = ADPCC_STATE_INITIALIZED;

    *ppAdpccCtx = pAdpccCtx;

    pAdpccCtx->eMode = ADPCC_OP_MODE_AUTO;

#if 1
    //init fix param for algo
    CalibDbV2_Dpcc_t* calibv2_adpcc_calib =
        (CalibDbV2_Dpcc_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adpcc_calib));
    memcpy(&pAdpccCtx->stDpccCalib, calibv2_adpcc_calib, sizeof(CalibDbV2_Dpcc_t));//load iq
    memcpy(&pAdpccCtx->stTool, calibv2_adpcc_calib, sizeof(CalibDbV2_Dpcc_t));
    dpcc_expert_mode_basic_params_init(&pAdpccCtx->stAuto.stBasicParams, &pAdpccCtx->stDpccCalib);
    dpcc_fast_mode_basic_params_init(&pAdpccCtx->stAuto.stFastMode, &pAdpccCtx->stDpccCalib);
    dpcc_pdaf_params_init(&pAdpccCtx->stAuto.stPdafParams, &pAdpccCtx->stDpccCalib.DpccTuningPara.Dpcc_pdaf);
    dpcc_sensor_params_init(&pAdpccCtx->stAuto.stSensorDpcc, &pAdpccCtx->stDpccCalib);
    dpcc_manual_fast_mode_init(&pAdpccCtx->stManual.stOnfly, &pAdpccCtx->stDpccCalib);
#else
    //static init
    html_params_init(&pAdpccCtx->stParams);
    memcpy(&pAdpccCtx->stAuto.stBasicParams, &pAdpccCtx->stParams.stBasic, sizeof(Adpcc_basic_params_t));
    memcpy(&pAdpccCtx->stAuto.stBptParams, &pAdpccCtx->stParams.stBpt, sizeof(Adpcc_bpt_params_t));
    memcpy(&pAdpccCtx->stAuto.stPdafParams, &pAdpccCtx->stParams.stPdaf, sizeof(Adpcc_pdaf_params_t));
#endif

    for(int i = 0; i < 3; i++) {
        pAdpccCtx->PreAe.arProcResIso[i] = 50;
        pAdpccCtx->PreAe.arProcResAGain[i] = 1.0;
        pAdpccCtx->PreAe.arProcResDGain[i] = 1.0;
        pAdpccCtx->PreAe.arProcResTime[i] = 0.01;
    }

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;

}

AdpccResult_t AdpccRelease(AdpccContext_t *pAdpccCtx)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    if(pAdpccCtx == NULL) {
        LOGE_ADPCC("%s(%d): null pointer", __FUNCTION__, __LINE__);
        return ADPCC_RET_NULL_POINTER;
    }

    memset(pAdpccCtx, 0x00, sizeof(AdpccContext_t));
    free(pAdpccCtx);

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;
}

AdpccResult_t AdpccConfig(AdpccContext_t *pAdpccCtx, AdpccConfig_t* pAdpccConfig)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);

    if(pAdpccCtx == NULL) {
        LOGE_ADPCC("%s(%d): null pointer", __FUNCTION__, __LINE__);
        return ADPCC_RET_INVALID_PARM;
    }

    if(pAdpccConfig == NULL) {
        LOGE_ADPCC("%s(%d): null pointer", __FUNCTION__, __LINE__);
        return ADPCC_RET_INVALID_PARM;
    }

    //pAdpccCtx->eMode = pAdpccConfig->eMode;
    //pAdpccCtx->eState = pAdpccConfig->eState;

    LOG1_ADPCC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;
}

AdpccResult_t AdpccReConfig(AdpccContext_t *pAdpccCtx, AdpccConfig_t* pAdpccConfig)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    //need todo what?

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;
}

AdpccResult_t AdpccPreProcess(AdpccContext_t *pAdpccCtx)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    //need todo what?

    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ADPCC_RET_SUCCESS;
}

AdpccResult_t AdpccProcess(AdpccContext_t *pAdpccCtx, AdpccExpInfo_t *pExpInfo, AdpccProcResult_t* pAdpccResult)
{
    LOG1_ADPCC("%s(%d): enter!", __FUNCTION__, __LINE__);
    AdpccResult_t ret = ADPCC_RET_SUCCESS;

    if(pAdpccCtx == NULL) {
        LOGE_ADPCC("%s(%d): null pointer", __FUNCTION__, __LINE__);
        return ADPCC_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ADPCC_RET_INVALID_PARM;
    }

    memcpy(&pAdpccCtx->stExpInfo, pExpInfo, sizeof(AdpccExpInfo_t));

    if(pAdpccCtx->eMode == ADPCC_OP_MODE_AUTO) {
        LOGD_ADPCC("%s(%d): Adpcc Auto mode!!!", __FUNCTION__, __LINE__);
        bool fast_enable = pAdpccCtx->stAuto.stFastMode.fast_mode_en == 0 ? false : true;

        if(fast_enable == false)
            ret = Expert_mode_select_basic_params_by_ISO(&pAdpccCtx->stAuto.stBasicParams, &pAdpccResult->stBasic, pExpInfo);
        else
            ret = Fast_mode_select_basic_params_by_ISO(pAdpccCtx, &pAdpccResult->stBasic, pExpInfo);

        ret = select_bpt_params_by_ISO(&pAdpccCtx->stAuto.stBptParams, &pAdpccResult->stBpt, pExpInfo);
        ret = select_pdaf_params_by_ISO(&pAdpccCtx->stAuto.stPdafParams, &pAdpccResult->stPdaf, pExpInfo);

        //sensor dpcc
        if(pAdpccCtx->stAuto.stSensorDpcc.en != 0 )
            Sensor_dpcc_process(pAdpccCtx);

    } else if(pAdpccCtx->eMode == ADPCC_OP_MODE_MANUAL) {
        //TODO
        LOGD_ADPCC("%s(%d): Adpcc Manual mode!!!", __FUNCTION__, __LINE__);
        if(pAdpccCtx->stManual.stOnfly.mode == ADPCC_ONFLY_MODE_FAST)
            ret = Api_Fast_mode_select(pAdpccCtx, &pAdpccResult->stBasic, &pAdpccCtx->stManual.stOnfly.fast_mode);
        else
            ret = Api_Expert_mode_select(&pAdpccResult->stBasic, &pAdpccCtx->stManual);

        ret = Api_select_bpt_params(&pAdpccResult->stBpt, &pAdpccCtx->stManual);
        ret = Api_select_pdaf_params(&pAdpccResult->stPdaf, &pAdpccCtx->stManual);

        //sensor dpcc api
        if(pAdpccCtx->stManual.stSensorDpcc.en)
            Api_Sensor_dpcc_process(pAdpccCtx);
    }
    /*else if(pAdpccCtx->eMode == ADPCC_OP_MODE_TOOL) {
        LOGD_ADPCC("%s(%d): Adpcc Tool mode!!!\n", __FUNCTION__, __LINE__);
        dpcc_expert_mode_basic_params_init(&pAdpccCtx->stAuto.stBasicParams, &pAdpccCtx->stTool);
        dpcc_fast_mode_basic_params_init(&pAdpccCtx->stAuto.stFastMode, &pAdpccCtx->stTool);
        dpcc_pdaf_params_init(&pAdpccCtx->stAuto.stPdafParams, &pAdpccCtx->stTool.DpccTuningPara.Dpcc_pdaf);
        dpcc_sensor_params_init(&pAdpccCtx->stAuto.stSensorDpcc, &pAdpccCtx->stTool);
        memset(&pAdpccCtx->stAuto.stPdafParams, 0x00, sizeof(pAdpccCtx->stAuto.stPdafParams));

        bool fast_enable = pAdpccCtx->stAuto.stFastMode.fast_mode_en == 0 ? false : true;
        if(fast_enable == false)
            ret = Expert_mode_select_basic_params_by_ISO(&pAdpccCtx->stAuto.stBasicParams, &pAdpccCtx->stAuto.stBasicSelect, pExpInfo);
        else
            ret = Fast_mode_select_basic_params_by_ISO(pAdpccCtx, &pAdpccCtx->stAuto.stBasicSelect, pExpInfo);

        ret = select_bpt_params_by_ISO(&pAdpccCtx->stAuto.stBptParams, &pAdpccCtx->stAuto.stBptSelect, pExpInfo);
        ret = select_pdaf_params_by_ISO(&pAdpccCtx->stAuto.stPdafParams, &pAdpccCtx->stAuto.stPdafSelect, pExpInfo);

        //sensor dpcc
        if(pAdpccCtx->stAuto.stSensorDpcc.en != 0 )
            Sensor_dpcc_process(pAdpccCtx);
    }*/
    else
        LOGE_ADPCC("%s(%d): Wrong api mode in dpcc!!!", __FUNCTION__, __LINE__);


    LOG1_ADPCC("%s(%d): exit!", __FUNCTION__, __LINE__);
    return ret;
}



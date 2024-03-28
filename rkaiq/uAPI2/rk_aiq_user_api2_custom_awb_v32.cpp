/*
 *  Copyright (c) 2021 Rockchip Corporation
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

//#include "RkAiqCalibDbV2Helper.h"


#include "rk_aiq_user_api2_custom_awb_type_v32.h"

RKAIQ_BEGIN_DECLARE


/* awb config of AIQ framework */
#define WRITE_DEBUG_LOG false

typedef struct rk_aiq_rkAwb_config_s {
    int Working_mode;//values look up in rk_aiq_working_mode_t definiton
    int RawWidth;
    int RawHeight;
    rk_aiq_isp_awb_meas_cfg_v32_t  awbHwConfig;
} rk_aiq_rkAwb_config_t;
static XCamReturn initCustomAwbHwConfigGw(rk_aiq_customAwb_hw_cfg_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_customAwb_hw_cfg_t));
    awbHwConfig->awbEnable             =    1;
    awbHwConfig->lightNum      =    4;
    awbHwConfig->windowSet[0]         =    0;
    awbHwConfig->windowSet[1]          =    0;
    awbHwConfig->windowSet[2]         =    3840;
    awbHwConfig->windowSet[3]         =    2160;
    awbHwConfig->maxR          =    230*16;
    awbHwConfig->maxG          =    230*16;
    awbHwConfig->maxB          =    230*16;
    awbHwConfig->maxY          =    230*16;
    awbHwConfig->minR          =    3*16;;
    awbHwConfig->minG          =    3*16;;
    awbHwConfig->minB          =    3*16;;
    awbHwConfig->minY          =    3*16;;
    awbHwConfig->multiwindow_en  =    0;
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}


static XCamReturn initAwbHwFullConfigGw(rk_aiq_isp_awb_meas_cfg_v32_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_isp_awb_meas_cfg_v32_t));
    awbHwConfig->awbEnable             =    1;
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]    =    0;
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]    =    0;
    awbHwConfig->lscBypEnable    =    0;
    awbHwConfig->blkStatisticsEnable    =    1;
    awbHwConfig->blkMeasureMode     =    RK_AIQ_AWB_BLK_STAT_MODE_REALWP_V201;
    awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)7;
    awbHwConfig->blkStatisticsWithLumaWeightEn  =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =    0;
    awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->lightNum      =    4;
    awbHwConfig->windowSet[0]         =    0;
    awbHwConfig->windowSet[1]          =    0;
    awbHwConfig->windowSet[2]         =    3840;
    awbHwConfig->windowSet[3]         =    2160;
    awbHwConfig->dsMode      =    (rk_aiq_down_scale_mode_t)1;
    awbHwConfig->maxR          =    230*16;;
    awbHwConfig->maxG          =    230*16;;
    awbHwConfig->maxB          =    230*16;;
    awbHwConfig->maxY          =    230*16;;
    awbHwConfig->minR          =    3*16;;
    awbHwConfig->minG          =    3*16;;
    awbHwConfig->minB          =    3*16;;
    awbHwConfig->minY          =    3*16;;
    awbHwConfig->pre_wbgain_inv_r  =    256;
    awbHwConfig->pre_wbgain_inv_g  =    256;
    awbHwConfig->pre_wbgain_inv_b  =    256;
    awbHwConfig->multiwindow_en  =    0;
    awbHwConfig->blk_rtdw_measure_en  =    0;
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static void initCustomAwbRes(rk_aiq_customeAwb_results_t* customAwb, rk_aiq_rkAwb_config_t* pConfig)
{
    customAwb->IsConverged = false;
    customAwb->awb_gain_algo.rgain =1.0;
    customAwb->awb_gain_algo.bgain =1.0;
    customAwb->awb_gain_algo.grgain =1.0;
    customAwb->awb_gain_algo.gbgain =1.0;
    customAwb->awb_smooth_factor = 0.5;
    rk_aiq_customAwb_hw_cfg_t  *awbHwConfig = &customAwb->awbHwConfig;
    const rk_aiq_isp_awb_meas_cfg_v32_t  *awbHwConfigFull = &pConfig->awbHwConfig;
    awbHwConfig->awbEnable   = awbHwConfigFull->awbEnable;
    awbHwConfig->multiwindow_en   = awbHwConfigFull->multiwindow_en;
    memcpy(awbHwConfig->multiwindow, awbHwConfigFull->multiwindow,sizeof(awbHwConfigFull->multiwindow));
    awbHwConfig->frameChoose   = (rk_aiq_customAwb_Raw_Select_Mode_e)awbHwConfigFull->frameChoose;
    memcpy(awbHwConfig->windowSet,awbHwConfigFull->windowSet,sizeof(awbHwConfigFull->windowSet));
    awbHwConfig->lightNum   = awbHwConfigFull->lightNum;
    awbHwConfig->maxR   = awbHwConfigFull->maxR;
    awbHwConfig->minR   = awbHwConfigFull->minR;
    awbHwConfig->maxG   = awbHwConfigFull->maxG;
    awbHwConfig->minG   = awbHwConfigFull->minG;
    awbHwConfig->maxB   = awbHwConfigFull->maxB;
    awbHwConfig->minB   = awbHwConfigFull->minB;
    awbHwConfig->maxY   = awbHwConfigFull->maxY;
    awbHwConfig->minY   = awbHwConfigFull->minY;

}

static XCamReturn calcInputBitIs12Bit( bool *inputBitIs12Bit, int frameChoose,int working_mode,const AblcProc_V32_t *ablc_res_v32)
{
    //call after frameChoose calculation
    //inputShiftEnable is true for 20bit ipnut ,for CUSTOM_AWB_INPUT_BAYERNR + (hdr case  or dgainInAwbGain) is enbale case
    bool selectBayerNrData = (frameChoose == CUSTOM_AWB_INPUT_BAYERNR);
    bool dgainEn = ablc_res_v32 && ablc_res_v32->isp_ob_predgain > 1;
    if (selectBayerNrData
        && (dgainEn ||(rk_aiq_working_mode_t)working_mode != RK_AIQ_WORKING_MODE_NORMAL)) {
        *inputBitIs12Bit = false;
    }else{
        *inputBitIs12Bit = true;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn calcInputRightShift212Bit( bool *inputShiftEnable, int frameChoose,int working_mode,const AblcProc_V32_t *ablc_res_v32)
{
    //call after frameChoose calculation
    //inputShiftEnable is true for 20bit ipnut ,for CUSTOM_AWB_INPUT_BAYERNR + (hdr case  or dgainInAwbGain) is enbale case
    bool selectBayerNrData = (frameChoose == CUSTOM_AWB_INPUT_BAYERNR);
    bool dgainEn = ablc_res_v32 && ablc_res_v32->isp_ob_predgain > 1;
    if (selectBayerNrData
        && (dgainEn ||(rk_aiq_working_mode_t)working_mode != RK_AIQ_WORKING_MODE_NORMAL)) {
        *inputShiftEnable = true;
    }else{
        *inputShiftEnable = false;
    }
    return XCAM_RETURN_NO_ERROR;
}


int32_t  awb_window_init(void* ctx,  rk_aiq_rkAwb_config_t *rkCfg)
{
    rkCfg->awbHwConfig.windowSet[0]=0;
    rkCfg->awbHwConfig.windowSet[1]=0;
    rkCfg->awbHwConfig.windowSet[2]=rkCfg->RawWidth;
    rkCfg->awbHwConfig.windowSet[3]=rkCfg->RawHeight;
    return 0;
}
int32_t  frame_choose_init(void* ctx, rk_aiq_rkAwb_config_t *rkCfg)
{

    rkCfg->awbHwConfig.frameChoose = CUSTOM_AWB_INPUT_BAYERNR;
    return 0;
}

int32_t  awb_window_check(const rk_aiq_rkAwb_config_t *rkCfg, unsigned short *windowSet)
{

    if(windowSet[0] > rkCfg->RawWidth ||
            windowSet[0] + windowSet[2] > rkCfg->RawWidth ||
            windowSet[1] > rkCfg->RawHeight ||
            windowSet[1] + windowSet[3] > rkCfg->RawHeight ) {
        LOGW_AWB("windowSet[%d,%d,%d,%d]is invaild!!! Reset to full window\n", windowSet[0],
                 windowSet[1], windowSet[2], windowSet[3]);
        windowSet[0]=0;
        windowSet[1]=0;
        windowSet[2]=rkCfg->RawWidth;
        windowSet[3]=rkCfg->RawHeight;
    }

    return 0;
}

static
void _rkAwbStats2CustomAwbStats(rk_aiq_customAwb_stats_t* customAwb,
                              rk_aiq_awb_stat_res_v32_t* rkAwb)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    memcpy(customAwb->light,rkAwb->light,sizeof(customAwb->light));
    memcpy(customAwb->WpNo2,rkAwb->WpNo2,sizeof(customAwb->WpNo2));
    memcpy(customAwb->blockResult,rkAwb->blockResult,sizeof(customAwb->blockResult));
    memcpy(customAwb->excWpRangeResult,rkAwb->excWpRangeResult,sizeof(customAwb->excWpRangeResult));
    memcpy(customAwb->WpNoHist,rkAwb->WpNoHist,sizeof(customAwb->WpNoHist));
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
}

static void _customAwbHw2rkAwbHwCfg( const rk_aiq_customeAwb_results_t* customAwbProcRes, rk_aiq_isp_awb_meas_cfg_v32_t *awbHwConfigFull)
{
    const rk_aiq_customAwb_hw_cfg_t  *awbHwConfig = &customAwbProcRes->awbHwConfig;
    awbHwConfigFull->awbEnable   = awbHwConfig->awbEnable;
    awbHwConfigFull->multiwindow_en   = awbHwConfig->multiwindow_en;
    memcpy(awbHwConfigFull->multiwindow, awbHwConfig->multiwindow,sizeof(awbHwConfig->multiwindow));
    awbHwConfigFull->frameChoose   = awbHwConfig->frameChoose;
    memcpy(awbHwConfigFull->windowSet,awbHwConfig->windowSet,sizeof(awbHwConfig->windowSet));
    awbHwConfigFull->lightNum   = awbHwConfig->lightNum;
    awbHwConfigFull->maxR   = awbHwConfig->maxR;
    awbHwConfigFull->minR   = awbHwConfig->minR;
    awbHwConfigFull->maxG   = awbHwConfig->maxG;
    awbHwConfigFull->minG   = awbHwConfig->minG;
    awbHwConfigFull->maxB   = awbHwConfig->maxB;
    awbHwConfigFull->minB   = awbHwConfig->minB;
    awbHwConfigFull->maxY   = awbHwConfig->maxY;
    awbHwConfigFull->minY   = awbHwConfig->minY;

}

static
void _customAwbRes2rkAwbRes( RkAiqAlgoProcResAwb* rkAwbProcRes,
                          rk_aiq_customeAwb_results_t* customAwbProcRes, const rk_aiq_isp_awb_meas_cfg_v32_t &awbHwConfig)
{

    rkAwbProcRes->awbConverged = customAwbProcRes->IsConverged;
    memcpy(rkAwbProcRes->awb_gain_algo, &customAwbProcRes->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));
    rkAwbProcRes->awb_smooth_factor = customAwbProcRes->awb_smooth_factor;
    *rkAwbProcRes->awb_hw32_para=  awbHwConfig;
}

static XCamReturn customAwbStatsRelease( rk_aiq_customAwb_stats_t *customStats)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_customAwb_stats_t *awbStat = customStats->next;
    rk_aiq_customAwb_stats_t *awbStat2;
    while(awbStat!=nullptr){
        awbStat2 = awbStat->next;
        //LOGE_AWB_SUBM(0xff, "%s free awbStat %p ", __func__,awbStat);
        free(awbStat);
        awbStat = awbStat2;
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}


static XCamReturn  ConfigBlc2(const AblcProc_V32_t *ablc,  const rk_aiq_wb_gain_t &awb_gain_algo,
    rk_aiq_wb_gapin_aplly_pos_e wbgainApplyPosition,int working_mode,
    rk_aiq_isp_awb_meas_cfg_v32_t* awb_hw32_para)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    awb_hw32_para->blc.enable = false;
    memset(awb_hw32_para->blc.blc,0,sizeof(awb_hw32_para->blc.blc));
    if(awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_DRC ||
        (awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_BAYERNR && working_mode!= RK_AIQ_WORKING_MODE_NORMAL )){
        //don't support to use blc2
        return(ret);
    }
    awb_hw32_para->blc.enable = true;
    float offset[AWB_CHANNEL_MAX]={0,0,0,0};
    // 1 interpolation

    // 2 blc2 recalc base on ablc
    // short int rawAwbPieplBlc[AWB_CHANNEL_MAX]={0,0,0,0};
    short int blc1[AWB_CHANNEL_MAX]={0,0,0,0};
    short ob =0;
    float dgain2 = 1.0;
    if(ablc){
        ob = ablc->isp_ob_offset;
        float isp_ob_predgain = ablc->isp_ob_predgain;
        if(isp_ob_predgain >1){
            dgain2 =  isp_ob_predgain;
        }
        if(ablc->blc1_enable){
            blc1[AWB_CHANNEL_R] = ablc->blc1_r;
            blc1[AWB_CHANNEL_GR] = ablc->blc1_gr;
            blc1[AWB_CHANNEL_GB] = ablc->blc1_gb;
            blc1[AWB_CHANNEL_B] = ablc->blc1_b;
        }
    }
    /*if (awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_DRC) {
        //update by dgain* wbgain0*wbgain1*offset
        ob2 = ob;//add ob2 to make total ob equal to 0 ,since blc0 +ob,blc1-ob,blc2-ob;
        for(int i=0; i<AWB_CHANNEL_MAX; i++){
            awb_hw32_para->blc.blc[i] = offset[i] * dgain2 * stat3aAwbGainOut2[i] + ob2 +0.5;
        }
    }else */if(awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_BAYERNR) {
        //update by dgain* wbgain0*offset +blc1
        // wbgain0 only used for hdr mode
        if(wbgainApplyPosition == IN_AWBGAIN0){
            float stat3aAwbGainOut2[AWB_CHANNEL_MAX];
            stat3aAwbGainOut2[AWB_CHANNEL_R] = awb_gain_algo.rgain;
            stat3aAwbGainOut2[AWB_CHANNEL_GR] = awb_gain_algo.grgain;
            stat3aAwbGainOut2[AWB_CHANNEL_GB] = awb_gain_algo.gbgain;
            stat3aAwbGainOut2[AWB_CHANNEL_B] = awb_gain_algo.bgain;
            for(int i=0; i<AWB_CHANNEL_MAX; i++){
                awb_hw32_para->blc.blc[i] = (offset[i] + blc1[i]) * dgain2 * stat3aAwbGainOut2[i] + 0.5;
            }
        }else{
            for(int i=0; i<AWB_CHANNEL_MAX; i++){
                awb_hw32_para->blc.blc[i] = (offset[i] + blc1[i]) * dgain2 + 0.5;
            }
        }
    }else {//select raw
        //update by offset +blc1
        for(int i=0; i<AWB_CHANNEL_MAX; i++){
            awb_hw32_para->blc.blc[i] = offset[i] + blc1[i] + 0.5;
        }
    }
    LOGV_AWB("offset =(%f,%f,%f,%f),  blc2=(%d,%d,%d,%d)", offset[0], offset[1],offset[2], offset[3],
        awb_hw32_para->blc.blc[0],awb_hw32_para->blc.blc[1],awb_hw32_para->blc.blc[2],awb_hw32_para->blc.blc[3]);

    return(ret);

}

//call after blc2 calc
static XCamReturn  ConfigOverexposureValue(const AblcProc_V32_t *ablc,
    float hdrmge_gain0_1, int working_mode, rk_aiq_isp_awb_meas_cfg_v32_t* awb_hw32_para)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    float dgain = 1.0;
    int max_blc = 0;
    short ob = 0;
    int overexposure_value = 254;
    if (ablc)
        ob = ablc->isp_ob_offset;
    if (awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_DRC) {
        awb_hw32_para->overexposure_value = overexposure_value;
    }else if(awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_BAYERNR) {
        if(ablc && ablc->isp_ob_predgain > 1){
            dgain =  ablc->isp_ob_predgain;
        }
        if(working_mode != RK_AIQ_WORKING_MODE_NORMAL){
            //hdr mode,awbgain0 will reset to full range
            LOGV_AWB("hdrmge_gain0_1 %f",hdrmge_gain0_1);
            awb_hw32_para->overexposure_value = overexposure_value * hdrmge_gain0_1 * dgain;
        }else{
            //need check
             short int blc[AWB_CHANNEL_MAX]={0,0,0,0};
             if(ablc && ablc->enable){
                 blc[AWB_CHANNEL_R] = ablc->blc_r *dgain- ob;
                 blc[AWB_CHANNEL_GR] = ablc->blc_gr*dgain - ob;
                 blc[AWB_CHANNEL_GB] = ablc->blc_gb*dgain - ob;
                 blc[AWB_CHANNEL_B] = ablc->blc_b*dgain - ob;
            }
             for(int i=0;i<AWB_CHANNEL_MAX;i++){
                 if(blc[i]<0){
                     blc[i] = 0;
                 }
                 blc[i] += awb_hw32_para->blc.blc[i]+ ob;
             }
             for(int i=0;i<AWB_CHANNEL_MAX;i++){
                 if(blc[i]>max_blc){
                     max_blc = blc[i];
                 }
             }
             if(max_blc<0){
                max_blc = 0;
             }
            awb_hw32_para->overexposure_value = overexposure_value * dgain -max_blc/16;
        }

    }else{
        //raw
        //need check
        short int blc[AWB_CHANNEL_MAX]={0,0,0,0};
        if(ablc && ablc->enable){
            blc[AWB_CHANNEL_R] = ablc->blc_r - ob;
            blc[AWB_CHANNEL_GR] = ablc->blc_gr - ob;
            blc[AWB_CHANNEL_GB] = ablc->blc_gb - ob;
            blc[AWB_CHANNEL_B] = ablc->blc_b - ob;
       }
        for(int i=0;i<AWB_CHANNEL_MAX;i++){
            if(blc[i]<0){
                blc[i] = 0;
            }
            blc[i] += awb_hw32_para->blc.blc[i] + ob;
        }
        for(int i=0;i<AWB_CHANNEL_MAX;i++){
            if(blc[i]>max_blc){
                max_blc = blc[i];
            }
        }
        if(max_blc<0){
           max_blc = 0;
        }
        awb_hw32_para->overexposure_value = overexposure_value - max_blc/16;
    }
    return(ret);

}

//call afte ablc_res_v32 got
void ConfigWbgainBaseOnBlc(const AblcProc_V32_t *blc,rk_aiq_wb_gapin_aplly_pos_e wbgainApplyPosition,
    rk_aiq_wb_gain_t *awb_gain_algo)
{
    float stat3aAwbGainOut2[AWB_CHANNEL_MAX];
    stat3aAwbGainOut2[AWB_CHANNEL_R] = awb_gain_algo->rgain;
    stat3aAwbGainOut2[AWB_CHANNEL_GR] = awb_gain_algo->grgain;
    stat3aAwbGainOut2[AWB_CHANNEL_GB] = awb_gain_algo->gbgain;
    stat3aAwbGainOut2[AWB_CHANNEL_B] = awb_gain_algo->bgain;

    short int mainPieplineBLC[AWB_CHANNEL_MAX];
    if(blc==nullptr){
        return;
    }
    memset(mainPieplineBLC, 0, sizeof(mainPieplineBLC[0])*AWB_CHANNEL_MAX);
    if(blc->enable) {
        mainPieplineBLC[AWB_CHANNEL_R] += blc->blc_r;
        mainPieplineBLC[AWB_CHANNEL_GR] += blc->blc_gr;
        mainPieplineBLC[AWB_CHANNEL_B] += blc->blc_b;
        mainPieplineBLC[AWB_CHANNEL_GB] += blc->blc_gb;
    }
    if(wbgainApplyPosition == IN_AWBGAIN1 && blc->blc1_enable) {
        mainPieplineBLC[AWB_CHANNEL_R] += blc->blc1_r;
        mainPieplineBLC[AWB_CHANNEL_GR] += blc->blc1_gr;
        mainPieplineBLC[AWB_CHANNEL_B] += blc->blc1_b;
        mainPieplineBLC[AWB_CHANNEL_GB] += blc->blc1_gb;
    }
    float maxg1 = 0;
    for(int i = 0; i < AWB_CHANNEL_MAX; i++) {
        if(mainPieplineBLC[i]>4094){
               mainPieplineBLC[i] = 4094;
               LOGE_AWB("mainPieplineBLC[%d] = %d is too large!!!!",i,mainPieplineBLC[i]);
           }
        if(mainPieplineBLC[i] > 0) {
            stat3aAwbGainOut2[i] *= 4095.0 / (4095 - mainPieplineBLC[i]); //max_value=4095
        }
        if(maxg1 < stat3aAwbGainOut2[i]) {
            maxg1 = stat3aAwbGainOut2[i];
        }
    }
    if(maxg1 > 8) { //max_wbgain=8.0
        float scale = 8 / maxg1;
        for(int i = 0; i < 4; i++) {
            stat3aAwbGainOut2[i] *= scale;
        }
        LOGW_AWB("max wbgain is %f, maybe error case", maxg1);
    }
    awb_gain_algo->rgain = stat3aAwbGainOut2[AWB_CHANNEL_R];
    awb_gain_algo->grgain = stat3aAwbGainOut2[AWB_CHANNEL_GR];
    awb_gain_algo->gbgain = stat3aAwbGainOut2[AWB_CHANNEL_GB];
    awb_gain_algo->bgain = stat3aAwbGainOut2[AWB_CHANNEL_B];
    LOGD_AWB("wbggain :(%f,%f,%f,%f)\n",awb_gain_algo->rgain,
        awb_gain_algo->grgain,awb_gain_algo->gbgain,
        awb_gain_algo->bgain);

}

//call after stat3aAwbGainOut2 calc
static XCamReturn  ConfigPreWbgain2(rk_aiq_isp_awb_meas_cfg_v32_t* awb_hw32_para, const rk_aiq_wb_gain_t &stat3aAwbGainOut,
    rk_aiq_wb_gapin_aplly_pos_e   wbgainApplyPosition )
{
    //call after wbgain  and frameChoose calculation
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    awb_hw32_para->pre_wbgain_inv_b = (1<<RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT);//0X100
    awb_hw32_para->pre_wbgain_inv_g = (1<<RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT);
    awb_hw32_para->pre_wbgain_inv_r = (1<<RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT);

    //pre_wbgain_inv_b is updating  for awb0-gain enable+select = bnr/hdrc case
    if(awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_DRC ||
        (wbgainApplyPosition == IN_AWBGAIN0 && awb_hw32_para->frameChoose == CUSTOM_AWB_INPUT_BAYERNR)){
        if(stat3aAwbGainOut.rgain< DIVMIN
            ||stat3aAwbGainOut.grgain< DIVMIN
            || stat3aAwbGainOut.bgain< DIVMIN){
            LOGE_AWB("%s  stat3aAwbGainOut[%f,%f,%f,%f] must be bigger than %f!\n", __FUNCTION__,
               stat3aAwbGainOut.rgain,
               stat3aAwbGainOut.grgain,
               stat3aAwbGainOut.gbgain,
               stat3aAwbGainOut.bgain,DIVMIN);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        awb_hw32_para->pre_wbgain_inv_r = awb_hw32_para->pre_wbgain_inv_r / stat3aAwbGainOut.rgain +0.5;
        awb_hw32_para->pre_wbgain_inv_g = awb_hw32_para->pre_wbgain_inv_g / stat3aAwbGainOut.grgain+0.5;
        awb_hw32_para->pre_wbgain_inv_b = awb_hw32_para->pre_wbgain_inv_b/ stat3aAwbGainOut.bgain+0.5;
    }
    return(ret);
}


static void WriteMeasureResult(rk_aiq_awb_stat_res_v32_t &awb_measure_result, int log_level ,int camera_id = 0)
{
#if WRITE_DEBUG_LOG
    if(log_level <XCORE_LOG_LEVEL_LOW1) {
        return;
    }
    FILE * fid;
    char fName[100];
    sprintf(fName, "./tmp/MeasureResultByCustom_camera%d.txt",camera_id);
    fid = fopen(fName, "w");
    if (fid == NULL) {
        return;
    }
    fprintf(fid, "WpNoHist:       ");
    for (int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        fprintf(fid, "%6d,", awb_measure_result.WpNoHist[i]);
    }
    fprintf(fid, "\n");
    {
        for (int i = 0; i < RK_AIQ_AWB_MAX_WHITEREGIONS_NUM_V32; i++)
        {
            fprintf(fid, " illuminant %dth:\n", i);
            fprintf(fid, "WPNUM2[%d]=    %7d\n", i, awb_measure_result.WpNo2[i]);
            fprintf(fid, "SUM_RGain_NOR[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[0].RgainValue);
            fprintf(fid, "SUM_BGain_NOR[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[0].BgainValue);
            fprintf(fid, "WPNUM_NOR[%d]=     %7lld\n", i, awb_measure_result.light[i].xYType[0].WpNo);
            fprintf(fid, "SUM_RGain_BIG[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[1].RgainValue);
            fprintf(fid, "SUM_BGain_BIG[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[1].BgainValue);
            fprintf(fid, "WPNUM_BIG[%d]=     %7lld\n", i, awb_measure_result.light[i].xYType[1].WpNo);


        }
        fprintf(fid, "blockresult[15][15]:\n");
        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_VERHOR * RK_AIQ_AWB_GRID_NUM_VERHOR; i++)
        {

            fprintf(fid, "blk[%2d,%2d]:     blk_sum_r=%7lld,        blk_sum_g=%7lld,        blk_sum_b=%7lld,   blk_sum_num=%7lld\n"
                    , i / 15, i % 15, awb_measure_result.blockResult[i].Rvalue,
                    awb_measure_result.blockResult[i].Gvalue, awb_measure_result.blockResult[i].Bvalue,
                    awb_measure_result.blockResult[i].WpNo);

        }
        fprintf(fid, "\n");
    }
    //new measurement
    {
        fprintf(fid, " excWpRangeResult:\n");
        for (int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++)
        {
            fprintf(fid, " %d:\n", i);
            fprintf(fid, "    SUM_RGain[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].RgainValue);
            fprintf(fid, "    SUM_RGain[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].BgainValue);
            fprintf(fid, "   WPNUM_NOR[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].WpNo);
        }
    }
    fclose(fid);
#endif
}


static void WriteDataForThirdParty(const rk_aiq_isp_awb_meas_cfg_v32_t &wpDetectPara,int log_level ,int camera_id=0)
{
#if WRITE_DEBUG_LOG

    if(log_level < XCORE_LOG_LEVEL_LOW1){
        return;
    }
    char fName[100];
    sprintf(fName, "./tmp/awbHwConfigByCustom_%d.txt",camera_id);
    FILE* fp = fopen(fName, "wb");
    if(fp != NULL)
    {

        fprintf(fp, "awbHwConfig->awbEnable             =    %d;//rawawb enable\n", wpDetectPara.awbEnable);
        fprintf(fp, "awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//xy detect  enable for all light\n", wpDetectPara.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//xy detect  enable for all light\n", wpDetectPara.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]    =    %d;\n", wpDetectPara.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]    =    %d;\n", wpDetectPara.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->lscBypEnable    =    %d;\n", wpDetectPara.lscBypEnable);

        fprintf(fp, "awbHwConfig->blkStatisticsEnable    =    %d;\n", wpDetectPara.blkStatisticsEnable);
        fprintf(fp, "awbHwConfig->blkMeasureMode     =    (rk_aiq_awb_blk_stat_mode_v201_t)%d;\n", wpDetectPara.blkMeasureMode);
        fprintf(fp, "awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)%d;\n", wpDetectPara.xyRangeTypeForBlkStatistics);
        fprintf(fp, "awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)%d;\n", wpDetectPara.illIdxForBlkStatistics);
        fprintf(fp, "awbHwConfig->blkStatisticsWithLumaWeightEn  =    %d;\n", wpDetectPara.blkStatisticsWithLumaWeightEn);

        fprintf(fp, "awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =    %d;\n", wpDetectPara.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =    %d;\n", wpDetectPara.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)%d;\n", wpDetectPara.xyRangeTypeForWpHist);

        fprintf(fp, "awbHwConfig->threeDyuvIllu[0]       =    %d;\n", wpDetectPara.threeDyuvIllu[0]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[1]       =    %d;\n", wpDetectPara.threeDyuvIllu[1]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[2]       =    %d;\n", wpDetectPara.threeDyuvIllu[2]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[3]       =    %d;\n", wpDetectPara.threeDyuvIllu[3]);



        fprintf(fp, "awbHwConfig->lightNum      =    %d; //measurment light num ,0~7\n", wpDetectPara.lightNum                                );
        fprintf(fp, "awbHwConfig->windowSet[0]         =    %d;//crop h_offs mod 2 == 0\n", wpDetectPara.windowSet[0]                                );
        fprintf(fp, "awbHwConfig->windowSet[1]          =    %d;//crop v_offs mod 2 == 0\n", wpDetectPara.windowSet[1]                                );
        fprintf(fp, "awbHwConfig->windowSet[2]         =    %d;//crop h_size,8x8:mod 8==0,4x4:mod 4==0\n", wpDetectPara.windowSet[2]    );
        fprintf(fp, "awbHwConfig->windowSet[3]         =    %d;//crop v_size,8x8:mod 8==0,4x4:mod 4==0\n", wpDetectPara.windowSet[3]    );

        /*if (wpDetectPara.windowSet[2]>2560||wpDetectPara.windowSet[3]>2560)
        Ds_rate = 8;*/
        int tmp;
        switch (wpDetectPara.dsMode)
        {
        case RK_AIQ_AWB_DS_4X4:
            tmp = 0;
            break;
        default:
            tmp = 1;
        }
        fprintf(fp, "awbHwConfig->dsMode      =    (rk_aiq_down_scale_mode_t)%d;//downscale,1:8x8,0:4x4\n",    tmp                            );
        fprintf(fp, "awbHwConfig->maxR          =    %d;\n",    wpDetectPara.maxR        );
        fprintf(fp, "awbHwConfig->maxG          =    %d;\n",    wpDetectPara.maxG        );
        fprintf(fp, "awbHwConfig->maxB          =    %d;\n",    wpDetectPara.maxB        );
        fprintf(fp, "awbHwConfig->maxY          =    %d;\n",    wpDetectPara.maxY        );
        fprintf(fp, "awbHwConfig->minR          =    %d;\n",    wpDetectPara.minR        );
        fprintf(fp, "awbHwConfig->minG          =    %d;\n",    wpDetectPara.minG        );
        fprintf(fp, "awbHwConfig->minB          =    %d;\n",    wpDetectPara.minB        );
        fprintf(fp, "awbHwConfig->minY          =    %d;\n",    wpDetectPara.minY        );
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[0]    =    %d;//uv detect white point region vertex and inverse slope\n", wpDetectPara.uvRange_param[0].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[3]);

        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[0]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[0]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[1]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[1]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[2]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[2]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[3]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[3]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[4]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[4]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[5]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[5]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[6]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[6]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[7]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[7]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[8]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[8]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[9]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[9]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[10]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[10]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[11]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[11]);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[0]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[1]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[2]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[3]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[4]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[5]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[0]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[1]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[2]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[3]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[4]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[5]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[0]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[1]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[2]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[0]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[1]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[2]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].disP1P2  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].disP1P2);
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[0]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[0] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[1]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[1] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[2]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[2] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[0]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[0]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[1]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[1]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[2]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[2]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[3]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[3]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[4]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[4]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[5]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[5]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeX[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeX[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeY[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeY[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeX[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeX[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeY[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeY[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_r  =    %d;\n", wpDetectPara.pre_wbgain_inv_r);
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_g  =    %d;\n", wpDetectPara.pre_wbgain_inv_g);
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_b  =    %d;\n", wpDetectPara.pre_wbgain_inv_b);
        //add new
        fprintf(fp, "awbHwConfig->multiwindow_en  =    %d;\n", wpDetectPara.multiwindow_en); // multiwindow auto white balance measure enable
        fprintf(fp, "awbHwConfig->multiwindow[0][0]  =    %d;\n", wpDetectPara.multiwindow[0][0]); // multiwindow0 row start
        fprintf(fp, "awbHwConfig->multiwindow[0][1]  =    %d;\n", wpDetectPara.multiwindow[0][1]); // multiwindow0 column start
        fprintf(fp, "awbHwConfig->multiwindow[0][2]  =    %d;\n", wpDetectPara.multiwindow[0][2]); // multiwindow0 row size
        fprintf(fp, "awbHwConfig->multiwindow[0][3]  =    %d;\n", wpDetectPara.multiwindow[0][3]); // multiwindow0 column size
        fprintf(fp, "awbHwConfig->multiwindow[1][0]  =    %d;\n", wpDetectPara.multiwindow[1][0]); // multiwindow1 row start
        fprintf(fp, "awbHwConfig->multiwindow[1][1]  =    %d;\n", wpDetectPara.multiwindow[1][1]); // multiwindow1 column start
        fprintf(fp, "awbHwConfig->multiwindow[1][2]  =    %d;\n", wpDetectPara.multiwindow[1][2]); // multiwindow1 row size
        fprintf(fp, "awbHwConfig->multiwindow[1][3]  =    %d;\n", wpDetectPara.multiwindow[1][3]); // multiwindow1 column size
        fprintf(fp, "awbHwConfig->multiwindow[2][0]  =    %d;\n", wpDetectPara.multiwindow[2][0]); // multiwindow2 row start
        fprintf(fp, "awbHwConfig->multiwindow[2][1]  =    %d;\n", wpDetectPara.multiwindow[2][1]); // multiwindow2 column start
        fprintf(fp, "awbHwConfig->multiwindow[2][2]  =    %d;\n", wpDetectPara.multiwindow[2][2]); // multiwindow2 row size
        fprintf(fp, "awbHwConfig->multiwindow[2][3]  =    %d;\n", wpDetectPara.multiwindow[2][3]); // multiwindow2 column size
        fprintf(fp, "awbHwConfig->multiwindow[3][0]  =    %d;\n", wpDetectPara.multiwindow[3][0]); // multiwindow3 row start
        fprintf(fp, "awbHwConfig->multiwindow[3][1]  =    %d;\n", wpDetectPara.multiwindow[3][1]); // multiwindow3 column start
        fprintf(fp, "awbHwConfig->multiwindow[3][2]  =    %d;\n", wpDetectPara.multiwindow[3][2]); // multiwindow3 row size
        fprintf(fp, "awbHwConfig->multiwindow[3][3]  =    %d;\n", wpDetectPara.multiwindow[3][3]); // multiwindow3 column size
        fprintf(fp, "awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[0].measureEnable); //white points region0 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[0].domain       ); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[0].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[0].xu[0]        ); //left x/u boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[0].xu[1]        ); //right x/u boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[0].yv[0]        ); //up y/v boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[0].yv[1]        ); //down y/v boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[0].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[1].measureEnable); //white points region1 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[1].domain       ); //white points region1 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[1].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[1].xu[0]        ); //left x/u boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[1].xu[1]        ); //right x/u boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[1].yv[0]        ); //up y/v boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[1].yv[1]        ); //down y/v boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[1].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[2].measureEnable); //white points region2 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[2].domain       ); //white points region2 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[2].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[2].xu[0]        ); //left x/u boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[2].xu[1]        ); //right x/u boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[2].yv[0]        ); //up y/v boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[2].yv[1]        ); //down y/v boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[2].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[3].measureEnable); //white points region3 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[3].domain       ); //white points region3 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[3].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[3].xu[0]        ); //left x/u boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[3].xu[1]        ); //right x/u boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[3].yv[0]        ); //up y/v boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[3].yv[1]        ); //down y/v boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[3].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[4].measureEnable); //white points region4 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[4].domain       ); //white points region4 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[4].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[4].xu[0]        ); //left x/u boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[4].xu[1]        ); //right x/u boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[4].yv[0]        ); //up y/v boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[4].yv[1]        ); //down y/v boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[4].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[5].measureEnable); //white points region5 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[5].domain       ); //white points region5 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[5].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[5].xu[0]        ); //left x/u boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[5].xu[1]        ); //right x/u boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[5].yv[0]        ); //up y/v boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[5].yv[1]        ); //down y/v boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[5].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[6].measureEnable);  //white points region6 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[6].domain       ); //white points region6 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[6].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[6].xu[0]        ); //left x/u boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[6].xu[1]        ); //right x/u boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[6].yv[0]        ); //up y/v boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[6].yv[1]        ); //down y/v boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].weightInculde   =     %d;\n", wpDetectPara.excludeWpRange[6].weightInculde); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        for (int i = 0; i < RK_AIQ_AWBWP_WEIGHT_CURVE_DOT_NUM; i++) {
            fprintf(fp, "awbHwConfig->wpDiffwei_y[%d]  =%d;\n", i, wpDetectPara.wpDiffwei_y[i]);
        }
        for (int i = 0; i < RK_AIQ_AWBWP_WEIGHT_CURVE_DOT_NUM; i++) {
            fprintf(fp, "awbHwConfig->wpDiffwei_w[%d]  =%d;\n", i, wpDetectPara.wpDiffwei_w[i]);
        }
        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            fprintf(fp, "awbHwConfig->blkWeight[%d]  =%d;\n", i, wpDetectPara.blkWeight[i]);
        }
        fprintf(fp, "awbHwConfig->blk_rtdw_measure_en  =    %d;\n", wpDetectPara.blk_rtdw_measure_en );
        fprintf(fp, "awbHwConfig->inputShiftEnable     =    %d;\n", wpDetectPara.inputShiftEnable);
        fprintf(fp, "awbHwConfig->overexposure_value     =    %d; //not the final value\n", wpDetectPara.overexposure_value);
        fprintf(fp, "awbHwConfig->inputBitIs12Bit     =    %d;\n", wpDetectPara.inputBitIs12Bit);
        fprintf(fp, "awbHwConfig->frameChoose     =    %d;\n", wpDetectPara.frameChoose);

        if (wpDetectPara.frameChoose == CUSTOM_AWB_INPUT_DRC) {
            fprintf(fp, "//g_awb_para.sw_drc2awb_sel_en = %d;\n", 1);
        }
        else {
            fprintf(fp, "//g_awb_para.sw_drc2awb_sel_en = %d;\n", 0);
            if (wpDetectPara.frameChoose == CUSTOM_AWB_INPUT_BAYERNR) {
                fprintf(fp, "//g_awb_para.sw_bnr2awb_sel_en = %d;\n", 1);
            }
            else {
                fprintf(fp, "//g_awb_para.sw_bnr2awb_sel_en = %d;\n", 0);
                fprintf(fp, "//g_awb_para.sw_3a_rawawb_sel = %d;\n", wpDetectPara.frameChoose);
            }
        }
        fprintf(fp, "awbHwConfig->blc.enable = %d;\n", wpDetectPara.blc.enable);
        fprintf(fp, "awbHwConfig->blc.blc[0] = %d;\n", wpDetectPara.blc.blc[0]);
        fprintf(fp, "awbHwConfig->blc.blc[1] = %d;\n", wpDetectPara.blc.blc[1]);
        fprintf(fp, "awbHwConfig->blc.blc[2] = %d;\n", wpDetectPara.blc.blc[2]);
        fprintf(fp, "awbHwConfig->blc.blc[3] = %d;\n", wpDetectPara.blc.blc[3]);
        fclose(fp);

    }
#endif
}


RKAIQ_END_DECLARE

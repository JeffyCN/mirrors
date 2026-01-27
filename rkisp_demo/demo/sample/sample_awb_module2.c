/*
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
#if defined(ISP_HW_V33)||defined(ISP_HW_V39) || defined(ISP_HW_V35)
#include "sample_comm.h"
#define safe_free(x) if(NULL!=(x))\
                           free(x); x=NULL;

void sample_print_awb_info(const void *arg)
{
    printf ("enter AWB modult test!\n");
}


/*
******************************
*
* ImgProc level API Sample Func
*
******************************
*/


static int sample_set_wbmode_manual0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_setWBMode(ctx, OP_MANUAL);
    return 0;
}

static int sample_set_wbmode_auto0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_setWBMode(ctx, OP_AUTO);
    return 0;
}

static int sample_get_wbmode0(const rk_aiq_sys_ctx_t* ctx)
{
    opMode_t mode;
    rk_aiq_uapi2_getWBMode(ctx, &mode);
    printf("get WBMode=%d\n\n", mode);
    return 0;
}

static int sample_lock_awb0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_lockAWB(ctx);
    return 0;
}

static int sample_unlock_awb0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi2_unlockAWB(ctx);
    return 0;
}

static int sample_set_mwb_gain0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_gain_t gain;
    gain.rgain = 0.5;
    gain.grgain = 0.5;
    gain.gbgain = 0.5;
    gain.bgain = 0.5;
    rk_aiq_uapi2_setMWBGain(ctx, &gain);
    return 0;
}

static int sample_get_mwb_gain0(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_gain_t gain;
    rk_aiq_uapi2_getWBGain(ctx, &gain);
    printf("get WBGain=[%f %f %f %f]\n", gain.rgain, gain.grgain, gain.gbgain, gain.bgain);
    return 0;
}


static int sample_set_wbgainBypass(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.byPass=true;
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_set_wbmode_manual(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_op_mode_t mode  = RK_AIQ_OP_MODE_MANUAL;
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.opMode=mode;
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_set_wbmode_auto(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_op_mode_t mode  = RK_AIQ_OP_MODE_AUTO;
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.opMode=mode;
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_get_wbmode(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    printf("get WBMode=%d\n\n", attr.opMode);
    return 0;
}

static int sample_lock_awb(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_user_api2_awb_Lock(ctx);
    return 0;
}

static int sample_unlock_awb(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_user_api2_awb_Unlock(ctx);
    return 0;
}

static int sample_set_mwb_scene(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCtrl_t attr;
    wb_mwb_scene_t scene = mwb_scene_twilight;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.opMode=RK_AIQ_OP_MODE_MANUAL;
    attr.manualPara.mode = mwb_mode_scene;
    attr.manualPara.cfg.scene_mode = scene;
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_get_mwb_scene(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    printf("get MWBScene=%d\n\n", attr.manualPara.cfg.scene_mode);
    return 0;
}

static int sample_set_mwb_gain(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_gain_t gain;
    gain.rgain = 0.5;
    gain.grgain = 0.5;
    gain.gbgain = 0.5;
    gain.bgain = 0.5;

    awb_gainCtrl_t attr;
    float  wbgain[4] = {gain.rgain,gain.grgain, gain.gbgain, gain.bgain};
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.opMode=RK_AIQ_OP_MODE_MANUAL;
    attr.manualPara.mode = mwb_mode_wbgain;
    memcpy(attr.manualPara.cfg.manual_wbgain,wbgain,sizeof(wbgain));
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_get_wb_gain(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_querry_info_t wb_querry_info;
    rk_aiq_user_api2_awb_QueryWBInfo(ctx, &wb_querry_info);
    printf("get WBGain=[%f %f %f %f]\n",
         wb_querry_info.gain.rgain, wb_querry_info.gain.grgain, wb_querry_info.gain.gbgain, wb_querry_info.gain.bgain);
    return 0;
}

static int sample_set_mwb_ct(const rk_aiq_sys_ctx_t* ctx)
{
    awb_cct_t cct;
    cct.cct = 6000;
    cct.ccri=0;
    awb_gainCtrl_t attr;
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx,&attr );
    attr.opMode=RK_AIQ_OP_MODE_MANUAL;
    attr.manualPara.mode = mwb_mode_cct;
    attr.manualPara.cfg.cct = cct;
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx,&attr );
    return 0;
}

static int sample_awb_getCct(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_querry_info_t wb_querry_info;
    //get
    rk_aiq_user_api2_awb_QueryWBInfo(ctx, &wb_querry_info);
    printf("\t CCRI = %f, CCT = %f\n\n", wb_querry_info.cctGloabl.CCRI, wb_querry_info.cctGloabl.CCT);

    return 0;
}


static int sample_query_wb_info(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_wb_querry_info_t wb_querry_info;
    rk_aiq_user_api2_awb_QueryWBInfo(ctx, &wb_querry_info);
    printf("Query AWB Info\n\n");
    printf("\t AWBGain = [%f %f %f %f]\n", wb_querry_info.gain.rgain,
           wb_querry_info.gain.grgain,
           wb_querry_info.gain.gbgain,
           wb_querry_info.gain.bgain);
    printf("\t cctGloabl: CCT = %f, CCRI = %f \n", wb_querry_info.cctGloabl.CCT, wb_querry_info.cctGloabl.CCRI);
    printf("\t awbConverged: %s \n", (wb_querry_info.awbConverged ? "true" : "false"));
    printf("\t LVValue: %d \n", wb_querry_info.LVValue);
    return 0;
}
static int sample_awb_SetWbAttrib(const rk_aiq_sys_ctx_t* ctx)
{
    awb_api_attrib_t attr;
    memset(&attr, 0, sizeof(awb_api_attrib_t));

    //get
    rk_aiq_user_api2_awb_GetAttrib(ctx, &attr);
    //modify
    //attr.wbGainCtrl.opMode=RK_AIQ_OP_MODE_MANUAL;
    attr.wbGainCtrl.manualPara.mode = mwb_mode_wbgain;
    if (attr.wbGainCtrl.manualPara.cfg.manual_wbgain[0] == 1.0) {
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[0]=0.5;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[1]=0.5;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[2]=0.5;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[3]=0.5;
    } else {
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[0]=1;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[1]=1;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[2]=1;
        attr.wbGainCtrl.manualPara.cfg.manual_wbgain[3]=1;
    }

    //set
    rk_aiq_user_api2_awb_SetAttrib(ctx, &attr);
    printf("%s\n\n",__func__);

    return 0;
}

static int sample_awb_SetWbGainCtrlAttrib(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCtrl_t attr;
    memset(&attr, 0, sizeof(awb_gainCtrl_t));

    //get
    rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx, &attr);
    //modify
    attr.opMode=RK_AIQ_OP_MODE_MANUAL;
    attr.manualPara.mode = mwb_mode_wbgain;
    if (attr.manualPara.cfg.manual_wbgain[0] == 1.0) {
        attr.manualPara.cfg.manual_wbgain[0]=0.5;
        attr.manualPara.cfg.manual_wbgain[1]=0.5;
        attr.manualPara.cfg.manual_wbgain[2]=0.5;
        attr.manualPara.cfg.manual_wbgain[3]=0.5;
    } else {
        attr.manualPara.cfg.manual_wbgain[0]=1;
        attr.manualPara.cfg.manual_wbgain[1]=1;
        attr.manualPara.cfg.manual_wbgain[2]=1;
        attr.manualPara.cfg.manual_wbgain[3]=1;
    }
    //set
    rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx, &attr);
    printf("%s\n\n",__func__);

    return 0;
}
static int sample_awb_SetAwbStatsAttrib(const rk_aiq_sys_ctx_t* ctx)
{
    awb_Stats_t attr;
    memset(&attr, 0, sizeof(awb_Stats_t));
     //get
    rk_aiq_user_api2_awb_GetAwbStatsAttrib(ctx, &attr);
    //modify
    if(attr.mainWin.hw_awbCfg_nonROI_en==false){
        attr.mainWin.hw_awbCfg_nonROI_en=true;
        attr.mainWin.nonROI[0].hw_awbCfg_nonROI_x = 0;
        attr.mainWin.nonROI[0].hw_awbCfg_nonROI_y= 0;
        attr.mainWin.nonROI[0].hw_awbCfg_nonROI_width = 100;
        attr.mainWin.nonROI[0].hw_awbCfg_nonROI_height = 100;
    }else{
        attr.mainWin.hw_awbCfg_nonROI_en=false;
    }
    //set
    rk_aiq_user_api2_awb_SetAwbStatsAttrib(ctx, &attr);
    printf("%s\n\n",__func__);

    return 0;
}

static int sample_awb_SetAwbGnCalcStepAttrib(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCalcStep_t attr;
    memset(&attr, 0, sizeof(awb_gainCalcStep_t));
     //get
    rk_aiq_user_api2_awb_GetAwbGnCalcStepAttrib(ctx, &attr);
    //modify
    if(attr.gnCalc_method==awb_gan_calc_method_auto){
        attr.gnCalc_method=awb_gan_calc_method_zone_mean;
    }else{
        attr.gnCalc_method=awb_gan_calc_method_auto;
    }
    //set
    rk_aiq_user_api2_awb_SetAwbGnCalcStepAttrib(ctx, &attr);
    printf("%s\n\n",__func__);
    return 0;
}

static int sample_awb_SetAwbGnCalcOthAttrib(const rk_aiq_sys_ctx_t* ctx)
{
    awb_gainCalcOth_t attr;
    memset(&attr, 0, sizeof(awb_gainCalcOth_t));
     //get
    rk_aiq_user_api2_awb_GetAwbGnCalcOthAttrib(ctx, &attr);
    //modify
    if(attr.tolerance.tolerance_en==false){
        attr.tolerance.tolerance_en = true;
    }else{
        attr.tolerance.tolerance_en = false;
    }
    //set
    rk_aiq_user_api2_awb_SetAwbGnCalcOthAttrib(ctx, &attr);
    printf("%s\n\n",__func__);
    return 0;
}


static int sample_awb_printflog(const rk_aiq_sys_ctx_t* ctx)
{

    rk_tool_awb_stat_res_full_t awb_measure_result;
    rk_tool_awb_strategy_result_t strategy_result;
    //for(int i=0;i<100;i++)
    {
		memset(&awb_measure_result, 0, sizeof(awb_measure_result));
		rk_aiq_user_api2_awb_getAlgoSta(ctx, &awb_measure_result);
		memset(&strategy_result, 0, sizeof(strategy_result));
		rk_aiq_user_api2_awb_getStrategyResult(ctx, &strategy_result);
		printf("------------%d---------------\n", strategy_result.count);
		printf("\n");
		printf("wbgns6(smooth)(rggb):(%f,%f,%f,%f),ganCalcM(%d),wbgnCvg(%d)\n",strategy_result.stat3aAwbGainOut[AWB_CHANNEL_R],
				 strategy_result.stat3aAwbGainOut[AWB_CHANNEL_GR], strategy_result.stat3aAwbGainOut[AWB_CHANNEL_GB],
				 strategy_result.stat3aAwbGainOut[AWB_CHANNEL_B],strategy_result.gnCalc_method, strategy_result.awbConverged);
		 printf("iso(%f),fLv(%f), WpT(%d),df(%1.2f,%1.2f), runItv(%d),toler(%f) \n",
				 strategy_result.aec_iso, strategy_result.fLVValue, strategy_result.WPType,
				 strategy_result.wbGainDampFactor, strategy_result.wbGainDampFactor2,
				 strategy_result.runInterval, strategy_result.tolerance);
		printf("WpNum:t(%d,%f),n(%d,%f),b(%d,%f),e(%d)\n",strategy_result.WPTotalNUM,
				 strategy_result.WPTotalNUMProp,awb_measure_result.WpNo[0], strategy_result.wpNorNumRat,
				 awb_measure_result.WpNo[1],strategy_result.wpBigNumRat,awb_measure_result.extraLightResult.WpNo);
 		 printf("wbgns5 (wbGainOffset) :(%f,%f,%f,%f)\n", strategy_result.wbGainOffset[AWB_CHANNEL_R],
				 strategy_result.wbGainOffset[AWB_CHANNEL_GR], strategy_result.wbGainOffset[AWB_CHANNEL_GB],
				 strategy_result.wbGainOffset[AWB_CHANNEL_B]);
		 printf("wbgns4 (wbGainAdjust) :(%f,%f,%f,%f)\n", strategy_result.wbGainAdjust[AWB_CHANNEL_R],
				 strategy_result.wbGainAdjust[AWB_CHANNEL_GR], strategy_result.wbGainAdjust[AWB_CHANNEL_GB],
				 strategy_result.wbGainAdjust[AWB_CHANNEL_B]);
		 printf("wbgns3 (wbGainClip) :(%f,%f,%f,%f)\n", strategy_result.wbGainClip[AWB_CHANNEL_R],
				 strategy_result.wbGainClip[AWB_CHANNEL_GR], strategy_result.wbGainClip[AWB_CHANNEL_GB],
				 strategy_result.wbGainClip[AWB_CHANNEL_B]);
        printf("wbgns1 (mix wpT1 and wpT3 ) :(%f,%f,%f,%f),updated (%d), wgt_wpT3(%f)\n", strategy_result.wbGainIntpStrategy[AWB_CHANNEL_R],
				 strategy_result.wbGainIntpStrategy[AWB_CHANNEL_GR], strategy_result.wbGainIntpStrategy[AWB_CHANNEL_GB],
				 strategy_result.wbGainIntpStrategy[AWB_CHANNEL_B], strategy_result.updateFlag, strategy_result.wbWeightType3);
		printf("wpgnT3:(%f,%f,%f,%f)\n", strategy_result.wbGainType3[AWB_CHANNEL_R], strategy_result.wbGainType3[AWB_CHANNEL_GR],
				strategy_result.wbGainType3[AWB_CHANNEL_GB], strategy_result.wbGainType3[AWB_CHANNEL_B]);
		printf("wpgnT1:(%f,%f,%f,%f)\n", strategy_result.wbGainType1[AWB_CHANNEL_R], strategy_result.wbGainType1[AWB_CHANNEL_GR],
				strategy_result.wbGainType1[AWB_CHANNEL_GB], strategy_result.wbGainType1[AWB_CHANNEL_B]);
		if(strategy_result.wbWeightSgc>-1){
			printf("wbgnSgc for WpT1:(%f,%f,%f,%f) ,wgtSgc(%f)[lv(%f),wpnum(%f),dis(%f),grad(%f)],wgtlastWbgn(%d),\n",
			strategy_result.wbGainSgc[0], strategy_result.wbGainSgc[1],
			strategy_result.wbGainSgc[2], strategy_result.wbGainSgc[3],
			strategy_result.wbWeightSgc,strategy_result.sinColorResult.wgt_lv, strategy_result.sinColorResult.wgt_wpnum,
			strategy_result.sinColorResult.wgt_meandis,strategy_result.sinColorResult.wgt_clrGrad,strategy_result.sgcGainEqu2Tem);
			printf("sgc illEst: %d  (%d,%d,%d,%d,%d,%d,%d,%d),colorEst : %d\n", strategy_result.sinColorResult.illEst,
						strategy_result.sinColorResult.voteResult[0], strategy_result.sinColorResult.voteResult[1], strategy_result.sinColorResult.voteResult[2],
						strategy_result.sinColorResult.voteResult[3], strategy_result.sinColorResult.voteResult[4], strategy_result.sinColorResult.voteResult[5],
						strategy_result.sinColorResult.voteResult[6], strategy_result.sinColorResult.voteResult[7], strategy_result.sinColorResult.colorEst);
		}
		printf("wbGnRef for WpT1 :(%f,%f,%f,%f),wgtRef(%f)\n", strategy_result.wbGainTep[AWB_CHANNEL_R], strategy_result.wbGainTep[AWB_CHANNEL_GR],
				 strategy_result.wbGainTep[AWB_CHANNEL_GB], strategy_result.wbGainTep[AWB_CHANNEL_B],strategy_result.wgtWbGnRef2T1);
		printf("wbGnExt for wpT3 (%f,%f,%f,%f), wgtExt(%f),dIdx[%d,%d],idx0Wgt(%f)\n",awb_measure_result.extraLightResult.gain[0],
			awb_measure_result.extraLightResult.gain[1],awb_measure_result.extraLightResult.gain[2],
			awb_measure_result.extraLightResult.gain[3],strategy_result.extraWp_wgt,
			strategy_result.extraWp_wbGainUsed[0],strategy_result.extraWp_wbGainUsed[1],
			strategy_result.extraWp_domainWgt);
		printf("wgtPrfNgt (%f)\n",  strategy_result.wgtPrfNgt);

		if (1 ) {
				char str1[500];
				sprintf(str1, "%s", "WpNoHist:       ");
				for (int p = 0; p < RK_AIQ_AWBWP_WEIGHT_CURVE_DOT_NUM - 1; p++) {
					char str2[100];
					sprintf(str2, "%6u,",  awb_measure_result.WpNoHist[p]);
					strcat(str1, str2);
				}
				printf("%s\n", str1);

		}
		for (int i = 0; i < strategy_result.lightNum; i++)
		{
			printf("%s  \tres_wbgn :(%f,%f,%f,%f) \n",strategy_result.illInf[i].illName,
						strategy_result.illInf[i].gainValue[0], strategy_result.illInf[i].gainValue[1],
						strategy_result.illInf[i].gainValue[2], strategy_result.illInf[i].gainValue[3]
					);
			printf("     prob_total(%f),dis(%f),lv(%f),wpnum(%f),lgtSrcWgt(%f)\n", strategy_result.illInf[i].prob_total, strategy_result.illInf[i].prob_dis,
						strategy_result.illInf[i].prob_LV, strategy_result.illInf[i].prob_WPNO,
						strategy_result.illInf[i].weight);
			printf("     prefwbgn:(%f,%f,%f,%f),prefWgt(%f)\n", strategy_result.illInf[i].spatialGainValue[0], strategy_result.illInf[i].spatialGainValue[1],
						strategy_result.illInf[i].spatialGainValue[2], strategy_result.illInf[i].spatialGainValue[3], strategy_result.illInf[i].preferWgt);
			int m=0;
			printf("     nor_wbgn:(%f,%f,%f,%f) wpnum(%d)\n",
					awb_measure_result.light[i].xYType[m].gain[0],
					awb_measure_result.light[i].xYType[m].gain[1],
					awb_measure_result.light[i].xYType[m].gain[2],
					awb_measure_result.light[i].xYType[m].gain[3],
					awb_measure_result.light[i].xYType[m].WpNo);
			m=1;
			printf("     big_wbgn:(%f,%f,%f,%f) wpnum(%d),bigWp_wgt(%f)\n",
					awb_measure_result.light[i].xYType[m].gain[0],
					awb_measure_result.light[i].xYType[m].gain[1],
					awb_measure_result.light[i].xYType[m].gain[2],
					awb_measure_result.light[i].xYType[m].gain[3],
					awb_measure_result.light[i].xYType[m].WpNo,
					strategy_result.illInf[i].bigWp_wgt);

		}
	}
    return  0;
}


static int sample_awb_getStrategyResult(const rk_aiq_sys_ctx_t* ctx)
{
    rk_tool_awb_strategy_result_t attr;
    memset(&attr, 0, sizeof(attr));
    rk_aiq_user_api2_awb_getStrategyResult(ctx, &attr);
    return 0;
}

static int sample_awb_WriteAwbIn(const rk_aiq_sys_ctx_t* ctx)
{
    static int call_cnt = 0;
    rk_aiq_uapiV2_awb_wrtIn_attr_t attr;
    memset(&attr, 0, sizeof(rk_aiq_uapiV2_awb_wrtIn_attr_t));
    attr.en = true;
    attr.mode = 1; // 1 means rgb ,0 means raw
    attr.call_cnt = call_cnt++;
    sprintf(attr.path, "/tmp");
    rk_aiq_user_api2_awb_WriteAwbIn(ctx, attr);
    printf("Write awb input \n\n");

    return 0;
}
static int sample_awb_setFFWbgain(const rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapiV2_awb_ffwbgain_attr_t attr;
    memset(&attr, 0, sizeof(rk_aiq_uapiV2_awb_ffwbgain_attr_t));
    rk_aiq_wb_gain_t wbgain = {1.0, 1.0, 1.0, 1.0};
    attr.wggain = wbgain;
    rk_aiq_user_api2_awb_SetFFWbgainAttrib(ctx, attr);
    printf("setFFWbgain \n\n");

    return 0;
}


static int sample_awb_SetNNres(const rk_aiq_sys_ctx_t* ctx)
{
    awb_ai_res_t attr;
    attr.confidence = 1;
    attr.converged = 1;// ae converged for qs sensor
    rk_aiq_wb_gain_t wbgain = {1.0, 1.0, 1.0, 1.0};
    attr.awb_gain=wbgain;
    rk_aiq_user_api2_awb_SetNNres(ctx, &attr);
    printf("SetNNres \n\n");

    return 0;
}

static void sample_awb_usage()
{
    printf("Usage : \n");
    printf("  ImgProc API: \n");
    printf("\t 0) AWB: setWBMode-OP_MANUAL.\n");
    printf("\t 1) AWB: setWBMode-OP_AUTO.\n");
    printf("\t 2) AWB: getWBMode.\n");
    printf("\t 3) AWB: lockAWB.\n");
    printf("\t 4) AWB: unlockAWB.\n");
    printf("\t 7) AWB: setMWBGain.\n");
    printf("\t 8) AWB: getWBGain.\n");

    printf("\t b) AWB: setWbGainOffset.\n");
    printf("\t c) AWB: getWbGainOffset.\n");

#if ISP_HW_V32||ISP_HW_V32_LITE||ISP_HW_V39
    printf("\t f) AWB: setAwbPreWbgain.\n");
    printf("\t g) AWB: Slave2Main.\n");
#endif
    printf("\n");
    printf("  Module API: \n");
    printf("\t A) AWB: set Awb AllAttr & Sync.\n");
    printf("\t B) AWB: get Awb AllAttr & Sync.\n");
    printf("\t C) AWB: printLog.\n");
    printf("\t E) AWB: get CCT.\n");
    printf("\t F) AWB: Query Awb Info.\n");
    printf("\t G) AWB: Lock.\n");
    printf("\t I) AWB: Unlock.\n");
    printf("\t J) AWB: set Mode Manual & Sync.\n");
    printf("\t L) AWB: set Mode Auto & Sync.\n");
    printf("\t N) AWB: set Manual attr & Sync.\n");
    printf("\t R) AWB: set sample_awb_SetNNres\n");
    printf("\t S) AWB: set WbGainOffset & Sync.\n");
#if ISP_HW_V32|| ISP_HW_V39|| ISP_HW_V33
    printf("\t Y) AWB: WriteAwbIn.\n");
#endif
    printf("\t Z) AWB: setFFWbgain.\n");
    printf("\n");
    printf("\t h) AWB: help.\n");
    printf("\t q) AWB: return to main sample screen.\n");

}

XCamReturn sample_awb_module(const void *arg)
{
    int key = -1;
    CLEAR();
    rk_aiq_wb_scene_t scene;
    rk_aiq_wb_gain_t gain;
    rk_aiq_wb_cct_t ct;
    opMode_t mode;
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }
    if (ctx == NULL) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_awb_usage ();
    do {
printf("\t please press the key: ");
        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf ("\n");

        switch (key)
        {
        case 'h':
            CLEAR();
            sample_awb_usage ();
            break;
        case '0':
            sample_set_wbmode_manual0(ctx);
            printf("setWBMode manual\n\n");
            break;
        case '1':
            sample_set_wbmode_auto0(ctx);
            printf("setWBMode auto\n\n");
            break;
        case '2':
            sample_get_wbmode0(ctx);
            break;
        case '3':
            sample_lock_awb0(ctx);
            printf("lockAWB\n\n");
            break;
        case '4':
            sample_unlock_awb0(ctx);
            printf("unlockAWB\n\n");
            break;
        case '7':
            sample_set_mwb_gain0(ctx);
            printf("setMWBGain\n\n");
            break;
        case '8':
            sample_get_mwb_gain0(ctx);
            break;
        case 'A':
            sample_set_wbgainBypass(ctx);
            break;
        case 'B':
            sample_awb_printflog(ctx);
            break;
         case 'C':
            sample_set_mwb_ct(ctx);
            sample_awb_getCct(ctx);
            break;
        case 'D':
            sample_awb_getCct(ctx);
            break;
        case 'E':
            sample_query_wb_info(ctx);
            break;
        case 'F':
            sample_lock_awb(ctx);
            break;
        case 'G':
            sample_unlock_awb(ctx);
            break;
        case 'H':
            sample_set_wbmode_manual(ctx);
            sample_get_wbmode(ctx);
            break;
        case 'I':
            sample_set_wbmode_auto(ctx);
            sample_get_wbmode(ctx);
            break;
        case 'J':
            sample_set_mwb_scene(ctx);
            sample_get_mwb_scene(ctx);
            break;
         case 'K':
            sample_set_mwb_gain(ctx);
            sample_get_wb_gain(ctx);
            break;
          case 'L':
            sample_awb_SetWbAttrib(ctx);
            sample_get_wb_gain(ctx);
            break;
          case 'M':
            sample_awb_SetWbGainCtrlAttrib(ctx);
            sample_get_wb_gain(ctx);
            break;
           case 'N':
            sample_awb_SetAwbStatsAttrib(ctx);
            sample_get_wb_gain(ctx);
            break;
            case 'O':
            sample_awb_SetAwbGnCalcStepAttrib(ctx);
            sample_get_wb_gain(ctx);
            break;
            case 'P':
            sample_awb_SetAwbGnCalcOthAttrib(ctx);
            break;
        case 'Y':
#if ISP_HW_V32|| ISP_HW_V39|| ISP_HW_V33
            sample_awb_WriteAwbIn(ctx);
#endif
            break;
        case 'Z':
            sample_awb_setFFWbgain(ctx);
            break;
        case 'R':
            sample_awb_SetNNres(ctx);
            break;
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');
    return XCAM_RETURN_NO_ERROR;
}

#endif


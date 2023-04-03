#include "awb_xml2json.h"
#include "RkAiqCalibApi.h"

void CalibV2AwbV21Free(CamCalibDbV2Context_t *calibV2)
{
    CalibDbV2_Wb_Para_V21_t* wb_v21 =
            (CalibDbV2_Wb_Para_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, wb_v21));
    if(wb_v21 == NULL){
          return;
    }
    CalibDbV2_Wb_Awb_Ext_Com_Para_t *commV21 = &wb_v21->autoExtPara;
    CalibDbV2_Wb_Awb_Para_V21_t* autoParaV21 =  &wb_v21->autoPara;
    free(commV21->lightSourceForFirstFrame);
    free(commV21->wbGainClip.cct);
    free(commV21->wbGainClip.cri_bound_up);
    free(commV21->wbGainClip.cri_bound_low);
    free(commV21->singleColorProces.colorBlock);
    for(int i=0;i<commV21->singleColorProces.lsUsedForEstimation_len;i++){
           free(commV21->singleColorProces.lsUsedForEstimation[i].name);
    }
    free(commV21->singleColorProces.lsUsedForEstimation);
    free(commV21->tolerance.toleranceValue);
    free(commV21->tolerance.lumaValue);
    free(commV21->runInterval.intervalValue);
    free(commV21->runInterval.lumaValue);
    for(int i=0;i<commV21->wbGainAdjust.lutAll_len;i++){
        free(commV21->wbGainAdjust.lutAll[i].ct_lut_out);
        free(commV21->wbGainAdjust.lutAll[i].cri_lut_out);
    }
    free(commV21->wbGainAdjust.lutAll);
    free(commV21->weightForNightGainCalc);
    free(commV21->division.wpNumTh.high);
    free(commV21->division.wpNumTh.low);
    free(commV21->division.wpNumTh.lumaValue);
    free(commV21->xyRegionStableSelection.wpNumTh.lumaValue);
    free(commV21->xyRegionStableSelection.wpNumTh.forBigType);
    free(commV21->xyRegionStableSelection.wpNumTh.forExtraType);
    for(int i=0;i<autoParaV21->lsUsedForYuvDet_len;i++){
        free(autoParaV21->lsUsedForYuvDet[i]);
    }
    free(autoParaV21->lsUsedForYuvDet);
    for(int i=0;i<autoParaV21->lightSources_len;i++){
        free(autoParaV21->lightSources[i].name);
    }
    free(autoParaV21->lightSources);
    free(autoParaV21->limitRange.lumaValue);
    free(autoParaV21->limitRange.maxB);
    free(autoParaV21->limitRange.maxR);
    free(autoParaV21->limitRange.maxG);
    free(autoParaV21->limitRange.maxY);
    free(autoParaV21->limitRange.minB);
    free(autoParaV21->limitRange.minR);
    free(autoParaV21->limitRange.minG);
    free(autoParaV21->limitRange.minY);
}
void convertCalib2calibV21(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2)
{
    const CalibDb_Awb_Calib_Para_V201_t *awb_calib_v21 = NULL;

    list_head *awb_calib_para_list =
        (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, awb_calib_para_v201);

    if (awb_calib_para_list)
        GetAwbProfileFromAwbCalibV201ListByIdx(awb_calib_para_list, 0, &awb_calib_v21);
    else
        return;

    const CalibDb_Awb_Adjust_Para_t *awb_adjust = NULL;

    list_head *awb_adjust_list =
        (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, awb_adjust_para);

    if (awb_adjust_list)
        GetAwbProfileFromAwbAdjustListByIdx(awb_adjust_list, 0, &awb_adjust);
    else
        return;

    CalibDbV2_Wb_Para_V21_t* wb_v21 =
            (CalibDbV2_Wb_Para_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, wb_v21));
    memset(wb_v21, 0, sizeof(CalibDbV2_Wb_Para_V21_t));
    CalibDbV2_Wb_Awb_Ext_Com_Para_t *commV21 = &wb_v21->autoExtPara;
    CalibDbV2_Wb_Awb_Para_V21_t* autoParaV21 =  &wb_v21->autoPara;
    commV21->lightSourceForFirstFrame= (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    commV21->wbGainClip.cct_len = awb_calib_v21->cct_clip_cfg.grid_num;
    commV21->wbGainClip.cri_bound_up_len = awb_calib_v21->cct_clip_cfg.grid_num;
    commV21->wbGainClip.cri_bound_low_len = awb_calib_v21->cct_clip_cfg.grid_num;
    commV21->wbGainClip.cct = (float*)malloc(sizeof(float) * commV21->wbGainClip.cct_len);
    commV21->wbGainClip.cri_bound_up = (float*)malloc(sizeof(float) * commV21->wbGainClip.cct_len);
    commV21->wbGainClip.cri_bound_low = (float*)malloc(sizeof(float) * commV21->wbGainClip.cct_len);
    commV21->singleColorProces.colorBlock_len = awb_calib_v21->sSelColorNUM;
    commV21->singleColorProces.lsUsedForEstimation_len = awb_calib_v21->sIllEstNum;
    commV21->singleColorProces.colorBlock = (CalibDbV2_Awb_Sgc_Cblk_t*)malloc(sizeof(CalibDbV2_Awb_Sgc_Cblk_t)*commV21->singleColorProces.colorBlock_len);
    commV21->singleColorProces.lsUsedForEstimation = (CalibDbV2_Awb_Sgc_Ls_t*)malloc(sizeof(CalibDbV2_Awb_Sgc_Ls_t)*commV21->singleColorProces.lsUsedForEstimation_len);
    for(int i=0;i<commV21->singleColorProces.lsUsedForEstimation_len;i++){
           commV21->singleColorProces.lsUsedForEstimation[i].name = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    commV21->tolerance.lumaValue_len = awb_adjust->tolerance.num;
    commV21->tolerance.toleranceValue_len = awb_adjust->tolerance.num;
    commV21->tolerance.toleranceValue = (float*)malloc(sizeof(float)*commV21->tolerance.toleranceValue_len);
    commV21->tolerance.lumaValue= (float*)malloc(sizeof(float)*commV21->tolerance.lumaValue_len);
    commV21->runInterval.lumaValue_len = awb_adjust->runInterval.num;
    commV21->runInterval.intervalValue_len= awb_adjust->runInterval.num;
    commV21->runInterval.intervalValue = (float*)malloc(sizeof(float)*commV21->runInterval.intervalValue_len);
    commV21->runInterval.lumaValue= (float*)malloc(sizeof(float)*commV21->runInterval.lumaValue_len);
    commV21->wbGainAdjust.lutAll_len = awb_adjust->cct_lut_cfg_num;
    commV21->wbGainAdjust.lutAll = (CalibDbV2_Awb_Cct_Lut_Cfg_Lv_t*)malloc(sizeof(CalibDbV2_Awb_Cct_Lut_Cfg_Lv_t)*commV21->wbGainAdjust.lutAll_len);
    for(int i=0;i<commV21->wbGainAdjust.lutAll_len;i++){
        commV21->wbGainAdjust.lutAll[i].ct_lut_out_len = awb_adjust->cct_lut_cfg[0].ct_grid_num *  awb_adjust->cct_lut_cfg[0].cri_grid_num;
        commV21->wbGainAdjust.lutAll[i].ct_lut_out = (float*)malloc(sizeof(float)*commV21->wbGainAdjust.lutAll[i].ct_lut_out_len);
        commV21->wbGainAdjust.lutAll[i].cri_lut_out_len = awb_adjust->cct_lut_cfg[0].ct_grid_num *  awb_adjust->cct_lut_cfg[0].cri_grid_num;
        commV21->wbGainAdjust.lutAll[i].cri_lut_out = (float*)malloc(sizeof(float)*commV21->wbGainAdjust.lutAll[i].cri_lut_out_len);
    }
    commV21->weightForNightGainCalc_len = 4;
    commV21->weightForNightGainCalc = (unsigned char*)malloc(sizeof(unsigned char)*commV21->weightForNightGainCalc_len);
    autoParaV21->lsUsedForYuvDet_len = 7;
    autoParaV21->lsUsedForYuvDet = (char**)malloc(sizeof(char*)*autoParaV21->lsUsedForYuvDet_len);
    for(int i=0;i<autoParaV21->lsUsedForYuvDet_len;i++){
        autoParaV21->lsUsedForYuvDet[i] = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    autoParaV21->lightSources_len = 7;
    autoParaV21->lightSources = (CalibDbV2_Awb_Light_V21_t*)malloc(sizeof(CalibDbV2_Awb_Light_V21_t)*autoParaV21->lightSources_len);
    for(int i=0;i<autoParaV21->lightSources_len;i++){
        autoParaV21->lightSources[i].name = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet_len = 2;
    autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet =
        (CalibDbV2_Awb_Luma_Weight_Lv_t*)malloc(sizeof(CalibDbV2_Awb_Luma_Weight_Lv_t)*autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet_len);
    for(int i=0;i<autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet_len;i++){
        autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet_len = 3;
        autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet =
            (CalibDbV2_Awb_Lum_Wgt_Lv_Rto_t*)malloc(sizeof(CalibDbV2_Awb_Lum_Wgt_Lv_Rto_t)*autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet_len);
    }
    autoParaV21->limitRange.lumaValue = (float*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(float));
    autoParaV21->limitRange.maxB= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.maxR= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.maxG= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.maxY= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.minB= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.minR= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.minG= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV21->limitRange.minY= (unsigned short*)malloc(awb_calib_v21->limitRange.lumaNum *sizeof(unsigned short));
    commV21->division.wpNumTh.high = (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV21->division.wpNumTh.low= (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV21->division.wpNumTh.lumaValue= (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV21->xyRegionStableSelection.wpNumTh.lumaValue = (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    commV21->xyRegionStableSelection.wpNumTh.forBigType= (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    commV21->xyRegionStableSelection.wpNumTh.forExtraType= (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    //coppy value
    CalibDbV2_Wb_Para_t *control = &wb_v21->control;
    control->byPass = 0;
    control->mode = CALIB_WB_MODE_AUTO;
    CalibDbV2_Wb_Mwb_Para_t *manualPara =  &wb_v21->manualPara;
    manualPara->mode = CALIB_MWB_MODE_SCENE;
    manualPara->cfg.scene = CALIB_WB_SCENE_CLOUDY_DAYLIGHT;
    manualPara->cfg.cct.CCT = 5000;
    manualPara->cfg.cct.CCRI = 0;
    memcpy(manualPara->cfg.mwbGain, awb_calib_v21->standardGainValue[0],sizeof(manualPara->cfg.mwbGain));
    autoParaV21->hdrPara.frameChoose = awb_calib_v21->hdrFrameChoose;
    autoParaV21->hdrPara.frameChooseMode= (CalibDbV2_Awb_Hdr_Fr_Ch_Mode_t)awb_calib_v21->hdrFrameChooseMode;
    autoParaV21->lscBypassEnable = awb_calib_v21->lscBypEnable;
    autoParaV21->uvDetectionEnable = awb_calib_v21->uvDetectionEnable;
    autoParaV21->xyDetectionEnable= awb_calib_v21->xyDetectionEnable;
    autoParaV21->yuvDetectionEnable = awb_calib_v21->yuvDetectionEnable;
    for(int i=0;i<autoParaV21->lsUsedForYuvDet_len;i++){
       strcpy(autoParaV21->lsUsedForYuvDet[i],awb_calib_v21->lsUsedForYuvDet[i]);
    }
    autoParaV21->blkStatisticsEnable = awb_calib_v21->blkStatisticsEnable;
    autoParaV21->downScaleMode = (CalibDbV2_Awb_Down_Scale_Mode_t)awb_calib_v21->dsMode;
    autoParaV21->blkMeasureMode = (CalibDbV2_Awb_Blk_Stat_V21_t)awb_calib_v21->blkMeasureMode;
    autoParaV21->mainWindow.mode = (CalibDb_Window_Mode_t)awb_calib_v21->measeureWindow.mode;
    float window[4] ={0,0,1,1};
    bool flag = false;
    if(autoParaV21->mainWindow.mode != CALIB_AWB_WINDOW_CFG_AUTO){
         char resName[CALD_AWB_RES_NAME];
        for(int i=0;i<awb_calib_v21->measeureWindow.resNum;i++){
            char* p;
            float width = 0;
            float height =0;
            strcpy(resName,awb_calib_v21->measeureWindow.resName[i]);
            p = strtok(resName, "x");
            if(p==NULL){
                printf("parse resName %s error0,\n",awb_calib_v21->measeureWindow.resName[i]);
                continue;
            }
            width = atoi(p);
            p = strtok(NULL, " ");
            if(p==NULL){
                printf("parse resName %s error1,\n",awb_calib_v21->measeureWindow.resName[i]);
                continue;
            }
            height = atoi(p);
            if (height*width>0.001){
                if((float)awb_calib_v21->measeureWindow.window[i][0]/width > window[0]){
                    window[0] = awb_calib_v21->measeureWindow.window[i][0]/width;
                    flag = true;
                }
                if((float)awb_calib_v21->measeureWindow.window[i][1]/height > window[1]){
                    window[1] = awb_calib_v21->measeureWindow.window[i][1]/height;
                    flag = true;
                }
                if((float)awb_calib_v21->measeureWindow.window[i][2]/width < window[2]){
                    window[2] = awb_calib_v21->measeureWindow.window[i][2]/width;
                    flag = true;
                }
                if((float)awb_calib_v21->measeureWindow.window[i][3]/height < window[3]){
                    window[3] = awb_calib_v21->measeureWindow.window[i][3]/height;
                    flag = true;
                }
            }else{
                printf("parse resName %s error2,\n",awb_calib_v21->measeureWindow.resName[i]);
            }
        }
    }
    if (flag){
        printf("please check parse mainWindow.window (ratio) is %f,%f,%f,%f\n",window[0],window[1],window[2],window[3]);

    }
    memcpy(autoParaV21->mainWindow.window, window,sizeof(window));

    autoParaV21->limitRange.lumaValue_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.lumaValue,awb_calib_v21->limitRange.lumaValue,autoParaV21->limitRange.lumaValue_len*sizeof(float));
    autoParaV21->limitRange.maxB_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.maxB,awb_calib_v21->limitRange.maxB,autoParaV21->limitRange.maxB_len*sizeof(unsigned short));
    autoParaV21->limitRange.maxR_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.maxR,awb_calib_v21->limitRange.maxR,autoParaV21->limitRange.maxR_len*sizeof(unsigned short));
    autoParaV21->limitRange.maxG_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.maxG,awb_calib_v21->limitRange.maxG,autoParaV21->limitRange.maxG_len*sizeof(unsigned short));
    autoParaV21->limitRange.maxY_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.maxY,awb_calib_v21->limitRange.maxY,autoParaV21->limitRange.maxY_len*sizeof(unsigned short));
    autoParaV21->limitRange.minB_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.minB,awb_calib_v21->limitRange.minB,autoParaV21->limitRange.minB_len*sizeof(unsigned short));
    autoParaV21->limitRange.minR_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.minR,awb_calib_v21->limitRange.minR,autoParaV21->limitRange.minR_len*sizeof(unsigned short));
    autoParaV21->limitRange.minG_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.minG,awb_calib_v21->limitRange.minG,autoParaV21->limitRange.minG_len*sizeof(unsigned short));
    autoParaV21->limitRange.minY_len= awb_calib_v21->limitRange.lumaNum;
    memcpy(autoParaV21->limitRange.minY,awb_calib_v21->limitRange.minY,autoParaV21->limitRange.minY_len*sizeof(unsigned short));
    memcpy(autoParaV21->rgb2TcsPara.rotationMat, awb_calib_v21->rgb2tcs_param.rotationMat, sizeof(autoParaV21->rgb2TcsPara.rotationMat));
    memcpy(autoParaV21->rgb2TcsPara.pseudoLuminanceWeight, awb_calib_v21->rgb2tcs_param.pseudoLuminanceWeight, sizeof(autoParaV21->rgb2TcsPara.pseudoLuminanceWeight));
    memcpy(autoParaV21->rgb2RotationYuvMat, awb_calib_v21->rgb2RYuv_matrix,sizeof(autoParaV21->rgb2RotationYuvMat));
    for(int i=0;i<CALD_AWB_EXCRANGE_NUM_MAX;i++){
        autoParaV21->extraWpRange[i].domain = (CalibDbV2_Awb_Ext_Range_Dom_t)awb_calib_v21->excludeWpRange[i].domain;
        if(awb_calib_v21->excludeWpRange[i].mode != CALIB_AWB_EXCLUDE_WP_MODE
            && awb_calib_v21->excludeWpRange[i].mode != CALIB_AWB_ETR_LIGHT_SOUR_MODE ){
            autoParaV21->extraWpRange[i].mode = CALIB_AWB_EXCLUDE_WP_MODE;
        }else{
            autoParaV21->extraWpRange[i].mode = (CalibDbV2_Awb_Ext_Range_Mode_t)awb_calib_v21->excludeWpRange[i].mode;
        }
        autoParaV21->extraWpRange[i].region[0] = awb_calib_v21->excludeWpRange[i].xu[0];
        autoParaV21->extraWpRange[i].region[1] = awb_calib_v21->excludeWpRange[i].xu[1];
        autoParaV21->extraWpRange[i].region[2] = awb_calib_v21->excludeWpRange[i].yv[0];
        autoParaV21->extraWpRange[i].region[3] = awb_calib_v21->excludeWpRange[i].yv[1];
    }
    autoParaV21->wpDiffBlkWeiEnable = awb_calib_v21->blkWeightEnable;
    for(int i=0;i<CALD_AWB_GRID_NUM_TOTAL;i++){
        autoParaV21->wpDiffBlkWeight[i] = (awb_calib_v21->blkWeight[i] * 2) > 63 ? 63 : awb_calib_v21->blkWeight[i] * 2;
    }
    autoParaV21->wpDiffLumaWeight.enable = awb_calib_v21->wpDiffWeiEnable;
    memcpy(autoParaV21->wpDiffLumaWeight.wpDiffwei_y, awb_calib_v21->wpDiffwei_y, sizeof(autoParaV21->wpDiffLumaWeight.wpDiffwei_y));
    memcpy(autoParaV21->wpDiffLumaWeight.perfectBin, awb_calib_v21->perfectBin, sizeof(autoParaV21->wpDiffLumaWeight.perfectBin));
    autoParaV21->wpDiffLumaWeight.wpDiffWeiEnableTh.wpDiffWeiNoTh= awb_calib_v21->wpDiffNoTh;
    autoParaV21->wpDiffLumaWeight.wpDiffWeiEnableTh.wpDiffWeiLvValueTh= awb_calib_v21->wpDiffLvValueTh;
    for(int i=0;i < autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet_len; i++){
       autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].LvValue = awb_calib_v21->wpDiffweiSet_w_LvValueTh[i];
       for(int j=0;j<autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet_len;j++){
            autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet[j].ratioValue =  awb_calib_v21->wpDiffWeiRatioTh[j];
            if(i==1){
                memcpy(autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet[j].weight, awb_calib_v21->wpDiffweiSet_w_HigLV[j],
                   sizeof(autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet[j].weight));
            }else{
                memcpy(autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet[j].weight, awb_calib_v21->wpDiffweiSet_w_LowLV[j],
                   sizeof(autoParaV21->wpDiffLumaWeight.wpDiffWeightLvSet[i].ratioSet[j].weight));
            }
       }
    }
    for(int i=0;i<autoParaV21->lightSources_len;i++){
        strcpy(autoParaV21->lightSources[i].name, awb_calib_v21->lightName[i]);
        autoParaV21->lightSources[i].doorType = (CalibDbV2_Awb_DoorType_t)awb_calib_v21->doorType[i];
        memcpy(autoParaV21->lightSources[i].standardGainValue, awb_calib_v21->standardGainValue[i],sizeof(autoParaV21->lightSources[i].standardGainValue));
        memcpy(autoParaV21->lightSources[i].uvRegion.u, awb_calib_v21->uvRange_param[i].pu_region,sizeof(autoParaV21->lightSources[i].uvRegion.u));
        memcpy(autoParaV21->lightSources[i].uvRegion.v, awb_calib_v21->uvRange_param[i].pv_region,sizeof(autoParaV21->lightSources[i].uvRegion.v));
        autoParaV21->lightSources[i].xyRegion.normal[0] = awb_calib_v21->xyRangeLight[i].NorrangeX[0];
        autoParaV21->lightSources[i].xyRegion.normal[1] = awb_calib_v21->xyRangeLight[i].NorrangeX[1];
        autoParaV21->lightSources[i].xyRegion.normal[2] = awb_calib_v21->xyRangeLight[i].NorrangeY[0];
        autoParaV21->lightSources[i].xyRegion.normal[3] = awb_calib_v21->xyRangeLight[i].NorrangeY[1];
        autoParaV21->lightSources[i].xyRegion.big[0] = awb_calib_v21->xyRangeLight[i].SperangeX[0];
        autoParaV21->lightSources[i].xyRegion.big[1] = awb_calib_v21->xyRangeLight[i].SperangeX[1];
        autoParaV21->lightSources[i].xyRegion.big[2] = awb_calib_v21->xyRangeLight[i].SperangeY[0];
        autoParaV21->lightSources[i].xyRegion.big[3] = awb_calib_v21->xyRangeLight[i].SperangeY[1];
        memcpy(autoParaV21->lightSources[i].rtYuvRegion.thcurve_u, awb_calib_v21->yuv3D2Range_param[i].thcurve_u,
            sizeof(awb_calib_v21->yuv3D2Range_param[i].thcurve_u));
        memcpy(autoParaV21->lightSources[i].rtYuvRegion.thcure_th, awb_calib_v21->yuv3D2Range_param[i].thcure_th,
            sizeof(awb_calib_v21->yuv3D2Range_param[i].thcure_th));
        memcpy(autoParaV21->lightSources[i].rtYuvRegion.lineVector, awb_calib_v21->yuv3D2Range_param[i].line,
            sizeof(awb_calib_v21->yuv3D2Range_param[i].line));
        autoParaV21->lightSources[i].rtYuvRegion.disP1P2 = 15;

        memcpy(autoParaV21->lightSources[i].staWeight, awb_adjust->awb_light_info[i].staWeight, sizeof(autoParaV21->lightSources[i].staWeight));
        autoParaV21->lightSources[i].dayGainLvThSet[0] = awb_adjust->awb_light_info[i].spatialGain_LV_THL;
        autoParaV21->lightSources[i].dayGainLvThSet[1] = awb_adjust->awb_light_info[i].spatialGain_LV_THH;
        memcpy(autoParaV21->lightSources[i].defaultDayGainLow,awb_calib_v21->spatialGain_L, sizeof(awb_calib_v21->spatialGain_L));
        memcpy(autoParaV21->lightSources[i].defaultDayGainHigh,awb_calib_v21->spatialGain_H, sizeof(awb_calib_v21->spatialGain_H));
        autoParaV21->lightSources[i].xyType2Enable = awb_adjust->awb_light_info[i].xyType2Enable;
    }

    strcpy(commV21->lightSourceForFirstFrame, awb_adjust->lsForFirstFrame);
    memcpy(commV21->tolerance.lumaValue, awb_adjust->tolerance.LV,sizeof(float)*commV21->tolerance.lumaValue_len);
    memcpy(commV21->tolerance.toleranceValue, awb_adjust->tolerance.value,sizeof(float)*commV21->tolerance.toleranceValue_len);
    memcpy(commV21->runInterval.lumaValue, awb_adjust->runInterval.LV,sizeof(float)*commV21->runInterval.lumaValue_len);
    memcpy(commV21->runInterval.intervalValue, awb_adjust->runInterval.value,sizeof(float)*commV21->runInterval.intervalValue_len);
    commV21->dampFactor.dFStep = awb_adjust->dFStep;
    commV21->dampFactor.dFMin = awb_adjust->dFMin;
    commV21->dampFactor.dFMax = awb_adjust->dFMax;
    commV21->dampFactor.lvIIRsize = awb_adjust->LvIIRsize;
    commV21->dampFactor.lvVarTh = awb_adjust->LvVarTh;
    commV21->wbGainAdjust.enable = awb_adjust->wbGainAdjustEn;
    for(int i=0; i<commV21->wbGainAdjust.lutAll_len;i++){
        commV21->wbGainAdjust.lutAll[i].lumaValue = awb_adjust->cct_lut_cfg[i].lv;
        commV21->wbGainAdjust.lutAll[i].ct_grid_num = awb_adjust->cct_lut_cfg[0].ct_grid_num;
        commV21->wbGainAdjust.lutAll[i].cri_grid_num = awb_adjust->cct_lut_cfg[0].cri_grid_num;
        memcpy(commV21->wbGainAdjust.lutAll[i].ct_in_range, awb_adjust->cct_lut_cfg[0].ct_range,sizeof(commV21->wbGainAdjust.lutAll[i].ct_in_range));
        memcpy(commV21->wbGainAdjust.lutAll[i].cri_in_range, awb_adjust->cct_lut_cfg[0].cri_range,sizeof(commV21->wbGainAdjust.lutAll[i].cri_in_range));
        memcpy(commV21->wbGainAdjust.lutAll[i].ct_lut_out,awb_adjust->cct_lut_cfg[i].ct_lut_out,sizeof(float)*commV21->wbGainAdjust.lutAll[i].ct_lut_out_len);
        memcpy(commV21->wbGainAdjust.lutAll[i].cri_lut_out,awb_adjust->cct_lut_cfg[i].cri_lut_out,sizeof(float)*commV21->wbGainAdjust.lutAll[i].cri_lut_out_len);
    }
    commV21->wbGainDaylightClip.enable= awb_calib_v21->wbGainDaylightClipEn;
    commV21->wbGainDaylightClip.outdoor_cct_min = awb_calib_v21->cct_clip_cfg.outdoor_cct_min;
    commV21->wbGainClip.enable= awb_calib_v21->wbGainClipEn;
    memcpy(commV21->wbGainClip.cct, awb_calib_v21->cct_clip_cfg.cct,sizeof(float)*commV21->wbGainClip.cct_len);
    memcpy(commV21->wbGainClip.cri_bound_low, awb_calib_v21->cct_clip_cfg.cri_bound_low,sizeof(float)*commV21->wbGainClip.cri_bound_low_len);
    memcpy(commV21->wbGainClip.cri_bound_up, awb_calib_v21->cct_clip_cfg.cri_bound_up,sizeof(float)*commV21->wbGainClip.cri_bound_up_len);
    commV21->division.lumaValThLow= awb_adjust->LV_THL;
    commV21->division.lumaValThLow2= awb_adjust->LV_THL2;
    commV21->division.lumaValThHigh= awb_adjust->LV_THH;
    commV21->division.lumaValThHigh2= awb_adjust->LV_THH2;
    commV21->division.wpNumTh.lumaValue_len = awb_adjust->WP_TH.num;
    memcpy(commV21->division.wpNumTh.lumaValue, awb_adjust->WP_TH.lumaValue,sizeof(float)*commV21->division.wpNumTh.lumaValue_len);
    commV21->division.wpNumTh.high_len = awb_adjust->WP_TH.num;
    memcpy(commV21->division.wpNumTh.high, awb_adjust->WP_TH.WP_THH,sizeof(float)*commV21->division.wpNumTh.lumaValue_len);
    commV21->division.wpNumTh.low_len = awb_adjust->WP_TH.num;
    memcpy(commV21->division.wpNumTh.low, awb_adjust->WP_TH.WP_THL,sizeof(float)*commV21->division.wpNumTh.lumaValue_len);
    memcpy(commV21->defaultNightGain,awb_calib_v21->temporalDefaultGain, sizeof(commV21->defaultNightGain));
    memcpy(commV21->lumaValueMatrix,awb_adjust->LVMatrix, sizeof(commV21->lumaValueMatrix));
    memcpy(commV21->defaultNightGainWeight,awb_adjust->tempWeight, sizeof(commV21->defaultNightGainWeight));
    commV21->probCalcDis.proDis_THH = awb_adjust->proDis_THH;
    commV21->probCalcDis.proDis_THL = awb_adjust->proDis_THL;
    commV21->probCalcLv.outdoorLumaValThHigh = awb_adjust->proLV_Outdoor_THH;
    commV21->probCalcLv.outdoorLumaValThLow = awb_adjust->proLV_Outdoor_THL;
    commV21->probCalcWp.wpNumPercTh= awb_adjust->wpNumPercTh;
    commV21->probCalcWp.wpNumPercTh2= 0.2;
    commV21->converged.varThforDamp = awb_adjust->convergedVarTh;
    commV21->converged.varThforUnDamp= awb_adjust->convergedVarTh;
    commV21->xyRegionStableSelection.enable = true;
    commV21->xyRegionStableSelection.wpNumTh.lumaValue_len= awb_adjust->wpNumTh.num;
    memcpy(commV21->xyRegionStableSelection.wpNumTh.lumaValue,awb_adjust->wpNumTh.lumaValue,sizeof(float)*awb_adjust->wpNumTh.num);
    commV21->xyRegionStableSelection.wpNumTh.forBigType_len= awb_adjust->wpNumTh.num;
    memcpy(commV21->xyRegionStableSelection.wpNumTh.forBigType,awb_adjust->wpNumTh.wpNumThForBigType,sizeof(float)*awb_adjust->wpNumTh.num);
    commV21->xyRegionStableSelection.wpNumTh.forExtraType_len= awb_adjust->wpNumTh.num;
    memcpy(commV21->xyRegionStableSelection.wpNumTh.forExtraType,awb_adjust->wpNumTh.wpNumThForExtraType,sizeof(float)*awb_adjust->wpNumTh.num);
    commV21->xyRegionStableSelection.xyTypeListSize = awb_adjust->xyTypeListSize;
    commV21->xyRegionStableSelection.varianceLumaTh = awb_adjust->varianceLumaTh;
    memcpy(commV21->weightForNightGainCalc, awb_adjust->temporalGainSetWeight, commV21->weightForNightGainCalc_len*sizeof(unsigned char));
    commV21->singleColorProces.enable = true;
    for(int i=0; i<commV21->singleColorProces.colorBlock_len; i++){
        commV21->singleColorProces.colorBlock[i].index = awb_calib_v21->sIndSelColor[i];
        commV21->singleColorProces.colorBlock[i].meanC= awb_calib_v21->sMeanCh[0][i];
        commV21->singleColorProces.colorBlock[i].meanH= awb_calib_v21->sMeanCh[1][i];
    }
    for(int i=0; i<commV21->singleColorProces.lsUsedForEstimation_len; i++){
        strcpy(commV21->singleColorProces.lsUsedForEstimation[i].name, awb_calib_v21->sNameIllEst[i]);
        commV21->singleColorProces.lsUsedForEstimation[i].RGain= awb_calib_v21->srGain[i];
        commV21->singleColorProces.lsUsedForEstimation[i].BGain= awb_calib_v21->sbGain[i];
    }
    commV21->singleColorProces.alpha= awb_calib_v21->sAlpha;
    memcpy(commV21->lineRgBg,awb_calib_v21->lineRgBg,sizeof(commV21->lineRgBg));
    memcpy(commV21->lineRgProjCCT,awb_calib_v21->lineRgProjCCT,sizeof(commV21->lineRgProjCCT));
    commV21->chrAdpttAdj.enable = awb_adjust->ca_enable;
    commV21->chrAdpttAdj.laCalcFactor = awb_adjust->ca_LACalcFactor;
    memcpy(commV21->chrAdpttAdj.targetGain, awb_calib_v21->ca_targetGain, sizeof(commV21->chrAdpttAdj.targetGain));
    commV21->remosaicCfg.enable = awb_adjust->remosaic_cfg.enable;
    commV21->remosaicCfg.applyInvWbGainEnable= true;
    memcpy(commV21->remosaicCfg.sensorWbGain, awb_adjust->remosaic_cfg.sensor_awb_gain, sizeof(commV21->remosaicCfg.sensorWbGain));
    memcpy(&commV21->wbGainOffset, &awb_adjust->wbGainOffset, sizeof(commV21->wbGainOffset));

}


void CalibV2AwbV20Free(CamCalibDbV2Context_t *calibV2)
{
    CalibDbV2_Wb_Para_V20_t* wb_v20 =
            (CalibDbV2_Wb_Para_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, wb_v20));
    if(wb_v20 == NULL)
          return;

    CalibDbV2_Wb_Awb_Ext_Com_Para_t *commV20 = &wb_v20->autoExtPara;
    CalibDbV2_Wb_Awb_Para_V20_t* autoParaV20 =  &wb_v20->autoPara;
    free(commV20->lightSourceForFirstFrame);
    free(commV20->wbGainClip.cct);
    free(commV20->wbGainClip.cri_bound_up);
    free(commV20->wbGainClip.cri_bound_low);
    free(commV20->singleColorProces.colorBlock);
    for(int i=0;i<commV20->singleColorProces.lsUsedForEstimation_len;i++){
           free(commV20->singleColorProces.lsUsedForEstimation[i].name);
    }
    free(commV20->singleColorProces.lsUsedForEstimation);
    free(commV20->tolerance.toleranceValue);
    free(commV20->tolerance.lumaValue);
    free(commV20->runInterval.intervalValue);
    free(commV20->runInterval.lumaValue);
    for(int i=0;i<commV20->wbGainAdjust.lutAll_len;i++){
        free(commV20->wbGainAdjust.lutAll[i].ct_lut_out);
        free(commV20->wbGainAdjust.lutAll[i].cri_lut_out);
    }
    free(commV20->wbGainAdjust.lutAll);
    free(commV20->weightForNightGainCalc);
    free(commV20->division.wpNumTh.high);
    free(commV20->division.wpNumTh.low);
    free(commV20->division.wpNumTh.lumaValue);
    free(commV20->xyRegionStableSelection.wpNumTh.lumaValue);
    free(commV20->xyRegionStableSelection.wpNumTh.forBigType);
    free(commV20->xyRegionStableSelection.wpNumTh.forExtraType);
    for(int i=0;i<autoParaV20->lsUsedForYuvDet_len;i++){
        free(autoParaV20->lsUsedForYuvDet[i]);
    }
    free(autoParaV20->lsUsedForYuvDet);
    for(int i=0;i<autoParaV20->lightSources_len;i++){
        free(autoParaV20->lightSources[i].name);
    }
    free(autoParaV20->lightSources);
    free(autoParaV20->limitRange.lumaValue);
    free(autoParaV20->limitRange.maxB);
    free(autoParaV20->limitRange.maxR);
    free(autoParaV20->limitRange.maxG);
    free(autoParaV20->limitRange.maxY);
    free(autoParaV20->limitRange.minB);
    free(autoParaV20->limitRange.minR);
    free(autoParaV20->limitRange.minG);
    free(autoParaV20->limitRange.minY);
}

void convertCalib2calibV20(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2)
{
    //malloc
    const CalibDb_Awb_Calib_Para_V200_t *awb_calib_v20 = NULL;

    list_head *awb_calib_para_list =
        (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, awb_calib_para_v200);

#ifdef RKAIQ_ENABLE_PARSER_V1
    if (awb_calib_para_list)
        GetAwbProfileFromAwbCalibV200ListByIdx(awb_calib_para_list, 0, &awb_calib_v20);
#else
        return;
#endif

    const CalibDb_Awb_Adjust_Para_t *awb_adjust = NULL;

    list_head *awb_adjust_list =
        (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, awb_adjust_para);

    if (awb_adjust_list)
        GetAwbProfileFromAwbAdjustListByIdx(awb_adjust_list, 0, &awb_adjust);
    else
        return;

    CalibDbV2_Wb_Para_V20_t* wb_v20 =
            (CalibDbV2_Wb_Para_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, wb_v20));
    memset(wb_v20, 0, sizeof(CalibDbV2_Wb_Para_V20_t));
    CalibDbV2_Wb_Awb_Ext_Com_Para_t *commV20 = &wb_v20->autoExtPara;
    CalibDbV2_Wb_Awb_Para_V20_t* autoParaV20 =  &wb_v20->autoPara;
    commV20->lightSourceForFirstFrame= (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    commV20->wbGainClip.cct_len = awb_calib_v20->cct_clip_cfg.grid_num;
    commV20->wbGainClip.cri_bound_up_len = awb_calib_v20->cct_clip_cfg.grid_num;
    commV20->wbGainClip.cri_bound_low_len = awb_calib_v20->cct_clip_cfg.grid_num;
    commV20->wbGainClip.cct = (float*)malloc(sizeof(float) * commV20->wbGainClip.cct_len);
    commV20->wbGainClip.cri_bound_up = (float*)malloc(sizeof(float) * commV20->wbGainClip.cct_len);
    commV20->wbGainClip.cri_bound_low = (float*)malloc(sizeof(float) * commV20->wbGainClip.cct_len);
    commV20->singleColorProces.colorBlock_len = awb_calib_v20->sSelColorNUM;
    commV20->singleColorProces.lsUsedForEstimation_len = awb_calib_v20->sIllEstNum;
    commV20->singleColorProces.colorBlock = (CalibDbV2_Awb_Sgc_Cblk_t*)malloc(sizeof(CalibDbV2_Awb_Sgc_Cblk_t)*commV20->singleColorProces.colorBlock_len);
    commV20->singleColorProces.lsUsedForEstimation = (CalibDbV2_Awb_Sgc_Ls_t*)malloc(sizeof(CalibDbV2_Awb_Sgc_Ls_t)*commV20->singleColorProces.lsUsedForEstimation_len);
     for(int i=0;i<commV20->singleColorProces.lsUsedForEstimation_len;i++){
           commV20->singleColorProces.lsUsedForEstimation[i].name = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    commV20->tolerance.lumaValue_len = awb_adjust->tolerance.num;
    commV20->tolerance.toleranceValue_len = awb_adjust->tolerance.num;
    commV20->tolerance.toleranceValue = (float*)malloc(sizeof(float)*commV20->tolerance.toleranceValue_len);
    commV20->tolerance.lumaValue= (float*)malloc(sizeof(float)*commV20->tolerance.lumaValue_len);
    commV20->runInterval.lumaValue_len = awb_adjust->runInterval.num;
    commV20->runInterval.intervalValue_len= awb_adjust->runInterval.num;
    commV20->runInterval.intervalValue = (float*)malloc(sizeof(float)*commV20->runInterval.intervalValue_len);
    commV20->runInterval.lumaValue= (float*)malloc(sizeof(float)*commV20->runInterval.lumaValue_len);
    commV20->wbGainAdjust.lutAll_len = awb_adjust->cct_lut_cfg_num;
    commV20->wbGainAdjust.lutAll = (CalibDbV2_Awb_Cct_Lut_Cfg_Lv_t*)malloc(sizeof(CalibDbV2_Awb_Cct_Lut_Cfg_Lv_t)*commV20->wbGainAdjust.lutAll_len);
    for(int i=0;i<commV20->wbGainAdjust.lutAll_len;i++){
        commV20->wbGainAdjust.lutAll[i].ct_lut_out_len = awb_adjust->cct_lut_cfg[0].ct_grid_num *  awb_adjust->cct_lut_cfg[0].cri_grid_num;
        commV20->wbGainAdjust.lutAll[i].ct_lut_out = (float*)malloc(sizeof(float)*commV20->wbGainAdjust.lutAll[i].ct_lut_out_len);
        commV20->wbGainAdjust.lutAll[i].cri_lut_out_len = awb_adjust->cct_lut_cfg[0].ct_grid_num *  awb_adjust->cct_lut_cfg[0].cri_grid_num;
        commV20->wbGainAdjust.lutAll[i].cri_lut_out = (float*)malloc(sizeof(float)*commV20->wbGainAdjust.lutAll[i].cri_lut_out_len);
    }
    commV20->weightForNightGainCalc_len = 4;
    commV20->weightForNightGainCalc = (unsigned char*)malloc(sizeof(unsigned char)*commV20->weightForNightGainCalc_len);
    autoParaV20->lsUsedForYuvDet_len = 7;
    autoParaV20->lsUsedForYuvDet = (char**)malloc(sizeof(char*)*autoParaV20->lsUsedForYuvDet_len);
    for(int i=0;i<autoParaV20->lsUsedForYuvDet_len;i++){
        autoParaV20->lsUsedForYuvDet[i] = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    autoParaV20->lightSources_len = 7;
    autoParaV20->lightSources = (CalibDbV2_Awb_Light_V20_t*)malloc(sizeof(CalibDbV2_Awb_Light_V20_t)*autoParaV20->lightSources_len);
    for(int i=0;i<autoParaV20->lightSources_len;i++){
        autoParaV20->lightSources[i].name = (char*)malloc(sizeof(char)*CALD_AWB_ILLUMINATION_NAME);
    }
    autoParaV20->limitRange.lumaValue = (float*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(float));
    autoParaV20->limitRange.maxB= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.maxR= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.maxG= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.maxY= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.minB= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.minR= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.minG= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    autoParaV20->limitRange.minY= (unsigned short*)malloc(awb_calib_v20->limitRange.lumaNum *sizeof(unsigned short));
    commV20->division.wpNumTh.high = (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV20->division.wpNumTh.low= (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV20->division.wpNumTh.lumaValue= (float *)malloc(awb_adjust->WP_TH.num*sizeof(float));
    commV20->xyRegionStableSelection.wpNumTh.lumaValue = (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    commV20->xyRegionStableSelection.wpNumTh.forBigType= (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    commV20->xyRegionStableSelection.wpNumTh.forExtraType= (float *)malloc(awb_adjust->wpNumTh.num*sizeof(float));
    //coppy value
    CalibDbV2_Wb_Para_t *control = &wb_v20->control;
    control->byPass = 0;
    control->mode = CALIB_WB_MODE_AUTO;
    CalibDbV2_Wb_Mwb_Para_t *manualPara =  &wb_v20->manualPara;
    manualPara->mode = CALIB_MWB_MODE_SCENE;
    manualPara->cfg.scene =  CALIB_WB_SCENE_CLOUDY_DAYLIGHT;
    manualPara->cfg.cct.CCT = 5000;
    manualPara->cfg.cct.CCRI = 0;
    memcpy(manualPara->cfg.mwbGain, awb_calib_v20->standardGainValue[0],sizeof(manualPara->cfg.mwbGain));
    autoParaV20->hdrPara.frameChoose = awb_calib_v20->hdrFrameChoose;
    autoParaV20->hdrPara.frameChooseMode= (CalibDbV2_Awb_Hdr_Fr_Ch_Mode_t)awb_calib_v20->hdrFrameChooseMode;
    autoParaV20->lscBypassEnable = awb_calib_v20->lscBypEnable;
    autoParaV20->uvDetectionEnable = awb_calib_v20->uvDetectionEnable;
    autoParaV20->xyDetectionEnable= awb_calib_v20->xyDetectionEnable;
    autoParaV20->yuvDetectionEnable = awb_calib_v20->yuvDetectionEnable;
    for(int i=0;i<autoParaV20->lsUsedForYuvDet_len;i++){
       strcpy(autoParaV20->lsUsedForYuvDet[i],awb_calib_v20->lsUsedForYuvDet[i]);
    }
    autoParaV20->yuvDetRef_u = awb_calib_v20->yuv3DRange_param[0].ref_u;
    autoParaV20->downScaleMode = (CalibDbV2_Awb_Down_Scale_Mode_t)awb_calib_v20->dsMode;
    autoParaV20->blkMeasureMode = (CalibDbV2_Awb_Blk_Stat_V20_t)awb_calib_v20->blkMeasureMode;
    autoParaV20->mainWindow.mode = (CalibDb_Window_Mode_t)awb_calib_v20->measeureWindow.mode;
    float window[4] ={0,0,1,1};
    memcpy(autoParaV20->mainWindow.window, window,sizeof(window));
    bool flag = false;
    if(autoParaV20->mainWindow.mode != CALIB_AWB_WINDOW_CFG_AUTO){
         char resName[CALD_AWB_RES_NAME];
        for(int i=0;i<awb_calib_v20->measeureWindow.resNum;i++){
            char* p;
            float width = 0;
            float height =0;
            strcpy(resName,awb_calib_v20->measeureWindow.resName[i]);
            p = strtok(resName, "x");
            if(p==NULL){
                printf("parse resName %s error0,\n",awb_calib_v20->measeureWindow.resName[i]);
                continue;
            }
            width = atoi(p);
            p = strtok(NULL, " ");
            if(p==NULL){
                printf("parse resName %s error1,\n",awb_calib_v20->measeureWindow.resName[i]);
                continue;
            }
            height = atoi(p);
            if (height*width>0.001){
                if((float)awb_calib_v20->measeureWindow.window[i][0]/width > window[0]){
                    window[0] = awb_calib_v20->measeureWindow.window[i][0]/width;
                    flag = true;
                }
                if((float)awb_calib_v20->measeureWindow.window[i][1]/height > window[1]){
                    window[1] = awb_calib_v20->measeureWindow.window[i][1]/height;
                    flag = true;
                }
                if((float)awb_calib_v20->measeureWindow.window[i][2]/width < window[2]){
                    window[2] = awb_calib_v20->measeureWindow.window[i][2]/width;
                    flag = true;
                }
                if((float)awb_calib_v20->measeureWindow.window[i][3]/height < window[3]){
                    window[3] = awb_calib_v20->measeureWindow.window[i][3]/height;
                    flag = true;
                }
            }else{
                printf("parse resName %s error2,\n",awb_calib_v20->measeureWindow.resName[i]);
            }
        }
    }
    if (flag){
        printf("please check parse mainWindow.window (ratio) is %f,%f,%f,%f\n",window[0],window[1],window[2],window[3]);

    }
    autoParaV20->limitRange.lumaValue_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.lumaValue,awb_calib_v20->limitRange.lumaValue,autoParaV20->limitRange.lumaValue_len*sizeof(float));
    autoParaV20->limitRange.maxB_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.maxB,awb_calib_v20->limitRange.maxB,autoParaV20->limitRange.maxB_len*sizeof(unsigned short));
    autoParaV20->limitRange.maxR_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.maxR,awb_calib_v20->limitRange.maxR,autoParaV20->limitRange.maxR_len*sizeof(unsigned short));
    autoParaV20->limitRange.maxG_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.maxG,awb_calib_v20->limitRange.maxG,autoParaV20->limitRange.maxG_len*sizeof(unsigned short));
    autoParaV20->limitRange.maxY_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.maxY,awb_calib_v20->limitRange.maxY,autoParaV20->limitRange.maxY_len*sizeof(unsigned short));
    autoParaV20->limitRange.minB_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.minB,awb_calib_v20->limitRange.minB,autoParaV20->limitRange.minB_len*sizeof(unsigned short));
    autoParaV20->limitRange.minR_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.minR,awb_calib_v20->limitRange.minR,autoParaV20->limitRange.minR_len*sizeof(unsigned short));
    autoParaV20->limitRange.minG_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.minG,awb_calib_v20->limitRange.minG,autoParaV20->limitRange.minG_len*sizeof(unsigned short));
    autoParaV20->limitRange.minY_len= awb_calib_v20->limitRange.lumaNum;
    memcpy(autoParaV20->limitRange.minY,awb_calib_v20->limitRange.minY,autoParaV20->limitRange.minY_len*sizeof(unsigned short));
    memcpy(autoParaV20->rgb2TcsPara.rotationMat, awb_calib_v20->rgb2tcs_param.rotationMat, sizeof(autoParaV20->rgb2TcsPara.rotationMat));
    memcpy(autoParaV20->rgb2TcsPara.pseudoLuminanceWeight, awb_calib_v20->rgb2tcs_param.pseudoLuminanceWeight, sizeof(autoParaV20->rgb2TcsPara.pseudoLuminanceWeight));
    autoParaV20->multiWindow.enable = awb_calib_v20->multiwindow_en;
    autoParaV20->multiWindow.multiwindowMode = (CalibDbV2_Awb_Mul_Win_Mode_t)awb_adjust->multiwindowMode;
    for(int i=0;i<CALD_AWB_WINDOW_NUM_MAX;i++){
        for(int j=0;j<4;j++){
            autoParaV20->multiWindow.window[i][j]= awb_calib_v20->multiwindow[i][j];
        }
    }
    for(int i=0;i<CALD_AWB_EXCRANGE_NUM_MAX;i++){
        autoParaV20->extraWpRange[i].domain = (CalibDbV2_Awb_Ext_Range_Dom_t)awb_calib_v20->excludeWpRange[i].domain;
        if(awb_calib_v20->excludeWpRange[i].mode != CALIB_AWB_EXCLUDE_WP_MODE
            && awb_calib_v20->excludeWpRange[i].mode != CALIB_AWB_ETR_LIGHT_SOUR_MODE ){
            autoParaV20->extraWpRange[i].mode = CALIB_AWB_EXCLUDE_WP_MODE;
        }else{
            autoParaV20->extraWpRange[i].mode = (CalibDbV2_Awb_Ext_Range_Mode_t)awb_calib_v20->excludeWpRange[i].mode;
        }
        autoParaV20->extraWpRange[i].region[0] = awb_calib_v20->excludeWpRange[i].xu[0];
        autoParaV20->extraWpRange[i].region[1] = awb_calib_v20->excludeWpRange[i].xu[1];
        autoParaV20->extraWpRange[i].region[2] = awb_calib_v20->excludeWpRange[i].yv[0];
        autoParaV20->extraWpRange[i].region[3] = awb_calib_v20->excludeWpRange[i].yv[1];
    }
    for(int i=0;i<autoParaV20->lightSources_len;i++){
        strcpy(autoParaV20->lightSources[i].name, awb_calib_v20->lightName[i]);
        autoParaV20->lightSources[i].doorType = (CalibDbV2_Awb_DoorType_t)awb_calib_v20->doorType[i];
        memcpy(autoParaV20->lightSources[i].standardGainValue, awb_calib_v20->standardGainValue[i],sizeof(autoParaV20->lightSources[i].standardGainValue));
        memcpy(autoParaV20->lightSources[i].uvRegion.u, awb_calib_v20->uvRange_param[i].pu_region,sizeof(autoParaV20->lightSources[i].uvRegion.u));
        memcpy(autoParaV20->lightSources[i].uvRegion.v, awb_calib_v20->uvRange_param[i].pv_region,sizeof(autoParaV20->lightSources[i].uvRegion.v));
        autoParaV20->lightSources[i].xyRegion.normal[0] = awb_calib_v20->xyRangeLight[i].NorrangeX[0];
        autoParaV20->lightSources[i].xyRegion.normal[1] = awb_calib_v20->xyRangeLight[i].NorrangeX[1];
        autoParaV20->lightSources[i].xyRegion.normal[2] = awb_calib_v20->xyRangeLight[i].NorrangeY[0];
        autoParaV20->lightSources[i].xyRegion.normal[3] = awb_calib_v20->xyRangeLight[i].NorrangeY[1];
        autoParaV20->lightSources[i].xyRegion.big[0] = awb_calib_v20->xyRangeLight[i].SperangeX[0];
        autoParaV20->lightSources[i].xyRegion.big[1] = awb_calib_v20->xyRangeLight[i].SperangeX[1];
        autoParaV20->lightSources[i].xyRegion.big[2] = awb_calib_v20->xyRangeLight[i].SperangeY[0];
        autoParaV20->lightSources[i].xyRegion.big[3] = awb_calib_v20->xyRangeLight[i].SperangeY[1];
        autoParaV20->lightSources[i].xyRegion.small[0] = awb_calib_v20->xyRangeLight[i].SmalrangeX[0];
        autoParaV20->lightSources[i].xyRegion.small[1] = awb_calib_v20->xyRangeLight[i].SmalrangeX[1];
        autoParaV20->lightSources[i].xyRegion.small[2] = awb_calib_v20->xyRangeLight[i].SmalrangeY[0];
        autoParaV20->lightSources[i].xyRegion.small[3] = awb_calib_v20->xyRangeLight[i].SmalrangeY[1];
        autoParaV20->lightSources[i].yuvRegion.b_uv = awb_calib_v20->yuv3DRange_param[i].b_uv;
        autoParaV20->lightSources[i].yuvRegion.slope_inv_neg_uv= awb_calib_v20->yuv3DRange_param[i].slope_inv_neg_uv;
        autoParaV20->lightSources[i].yuvRegion.slope_factor_uv= awb_calib_v20->yuv3DRange_param[i].slope_factor_uv;
        autoParaV20->lightSources[i].yuvRegion.slope_ydis= awb_calib_v20->yuv3DRange_param[i].slope_ydis;
        autoParaV20->lightSources[i].yuvRegion.b_ydis= awb_calib_v20->yuv3DRange_param[i].b_ydis;
        autoParaV20->lightSources[i].yuvRegion.ref_v= awb_calib_v20->yuv3DRange_param[i].ref_v;
        memcpy(autoParaV20->lightSources[i].yuvRegion.dis, awb_calib_v20->yuv3DRange_param[i].dis, sizeof(autoParaV20->lightSources[i].yuvRegion.dis));
        memcpy(autoParaV20->lightSources[i].yuvRegion.th, awb_calib_v20->yuv3DRange_param[i].th, sizeof(autoParaV20->lightSources[i].yuvRegion.th));
        memcpy(autoParaV20->lightSources[i].staWeight, awb_adjust->awb_light_info[i].staWeight, sizeof(autoParaV20->lightSources[i].staWeight));
        autoParaV20->lightSources[i].dayGainLvThSet[0] = awb_adjust->awb_light_info[i].spatialGain_LV_THL;
        autoParaV20->lightSources[i].dayGainLvThSet[1] = awb_adjust->awb_light_info[i].spatialGain_LV_THH;
        memcpy(autoParaV20->lightSources[i].defaultDayGainLow,awb_calib_v20->spatialGain_L, sizeof(awb_calib_v20->spatialGain_L));
        memcpy(autoParaV20->lightSources[i].defaultDayGainHigh,awb_calib_v20->spatialGain_H, sizeof(awb_calib_v20->spatialGain_H));
        autoParaV20->lightSources[i].xyType2Enable = awb_adjust->awb_light_info[i].xyType2Enable;
    }

    strcpy(commV20->lightSourceForFirstFrame, awb_adjust->lsForFirstFrame);
    memcpy(commV20->tolerance.lumaValue, awb_adjust->tolerance.LV,sizeof(float)*commV20->tolerance.lumaValue_len);
    memcpy(commV20->tolerance.toleranceValue, awb_adjust->tolerance.value,sizeof(float)*commV20->tolerance.toleranceValue_len);
    memcpy(commV20->runInterval.lumaValue, awb_adjust->runInterval.LV,sizeof(float)*commV20->runInterval.lumaValue_len);
    memcpy(commV20->runInterval.intervalValue, awb_adjust->runInterval.value,sizeof(float)*commV20->runInterval.intervalValue_len);
    commV20->dampFactor.dFStep = awb_adjust->dFStep;
    commV20->dampFactor.dFMin = awb_adjust->dFMin;
    commV20->dampFactor.dFMax = awb_adjust->dFMax;
    commV20->dampFactor.lvIIRsize = awb_adjust->LvIIRsize;
    commV20->dampFactor.lvVarTh = awb_adjust->LvVarTh;
    commV20->wbGainAdjust.enable = awb_adjust->wbGainAdjustEn;
    for(int i=0; i<commV20->wbGainAdjust.lutAll_len;i++){
        commV20->wbGainAdjust.lutAll[i].lumaValue = awb_adjust->cct_lut_cfg[i].lv;
        commV20->wbGainAdjust.lutAll[i].ct_grid_num = awb_adjust->cct_lut_cfg[0].ct_grid_num;
        commV20->wbGainAdjust.lutAll[i].cri_grid_num = awb_adjust->cct_lut_cfg[0].cri_grid_num;
        memcpy(commV20->wbGainAdjust.lutAll[i].ct_in_range, awb_adjust->cct_lut_cfg[0].ct_range,sizeof(commV20->wbGainAdjust.lutAll[i].ct_in_range));
        memcpy(commV20->wbGainAdjust.lutAll[i].cri_in_range, awb_adjust->cct_lut_cfg[0].cri_range,sizeof(commV20->wbGainAdjust.lutAll[i].cri_in_range));
        memcpy(commV20->wbGainAdjust.lutAll[i].ct_lut_out,awb_adjust->cct_lut_cfg[i].ct_lut_out,sizeof(float)*commV20->wbGainAdjust.lutAll[i].ct_lut_out_len);
        memcpy(commV20->wbGainAdjust.lutAll[i].cri_lut_out,awb_adjust->cct_lut_cfg[i].cri_lut_out,sizeof(float)*commV20->wbGainAdjust.lutAll[i].cri_lut_out_len);
    }
    commV20->wbGainDaylightClip.enable= awb_calib_v20->wbGainDaylightClipEn;
    commV20->wbGainDaylightClip.outdoor_cct_min = awb_calib_v20->cct_clip_cfg.outdoor_cct_min;
    commV20->wbGainClip.enable= awb_calib_v20->wbGainClipEn;
    memcpy(commV20->wbGainClip.cct, awb_calib_v20->cct_clip_cfg.cct,sizeof(float)*commV20->wbGainClip.cct_len);
    memcpy(commV20->wbGainClip.cri_bound_low, awb_calib_v20->cct_clip_cfg.cri_bound_low,sizeof(float)*commV20->wbGainClip.cri_bound_low_len);
    memcpy(commV20->wbGainClip.cri_bound_up, awb_calib_v20->cct_clip_cfg.cri_bound_up,sizeof(float)*commV20->wbGainClip.cri_bound_up_len);
    commV20->division.lumaValThLow= awb_adjust->LV_THL;
    commV20->division.lumaValThLow2= awb_adjust->LV_THL2;
    commV20->division.lumaValThHigh= awb_adjust->LV_THH;
    commV20->division.lumaValThHigh2= awb_adjust->LV_THH2;
    commV20->division.wpNumTh.lumaValue_len = awb_adjust->WP_TH.num;
    memcpy(commV20->division.wpNumTh.lumaValue, awb_adjust->WP_TH.lumaValue,sizeof(float)*commV20->division.wpNumTh.lumaValue_len);
    commV20->division.wpNumTh.high_len = awb_adjust->WP_TH.num;
    memcpy(commV20->division.wpNumTh.high, awb_adjust->WP_TH.WP_THH,sizeof(float)*commV20->division.wpNumTh.lumaValue_len);
    commV20->division.wpNumTh.low_len = awb_adjust->WP_TH.num;
    memcpy(commV20->division.wpNumTh.low, awb_adjust->WP_TH.WP_THL,sizeof(float)*commV20->division.wpNumTh.lumaValue_len);
    memcpy(commV20->defaultNightGain,awb_calib_v20->temporalDefaultGain, sizeof(commV20->defaultNightGain));
    memcpy(commV20->lumaValueMatrix,awb_adjust->LVMatrix, sizeof(commV20->lumaValueMatrix));
    memcpy(commV20->defaultNightGainWeight,awb_adjust->tempWeight, sizeof(commV20->defaultNightGainWeight));
    commV20->probCalcDis.proDis_THH = awb_adjust->proDis_THH;
    commV20->probCalcDis.proDis_THL = awb_adjust->proDis_THL;
    commV20->probCalcLv.outdoorLumaValThHigh = awb_adjust->proLV_Outdoor_THH;
    commV20->probCalcLv.outdoorLumaValThLow = awb_adjust->proLV_Outdoor_THL;
    commV20->probCalcWp.wpNumPercTh= awb_adjust->wpNumPercTh;
    commV20->probCalcWp.wpNumPercTh2= 0.2;
    commV20->converged.varThforDamp = awb_adjust->convergedVarTh;
    commV20->converged.varThforUnDamp= awb_adjust->convergedVarTh;
    commV20->xyRegionStableSelection.enable = true;
    commV20->xyRegionStableSelection.wpNumTh.lumaValue_len= awb_adjust->wpNumTh.num;
    memcpy(commV20->xyRegionStableSelection.wpNumTh.lumaValue,awb_adjust->wpNumTh.lumaValue,sizeof(float)*awb_adjust->wpNumTh.num);
    commV20->xyRegionStableSelection.wpNumTh.forBigType_len= awb_adjust->wpNumTh.num;
    memcpy(commV20->xyRegionStableSelection.wpNumTh.forBigType,awb_adjust->wpNumTh.wpNumThForBigType,sizeof(float)*awb_adjust->wpNumTh.num);
    commV20->xyRegionStableSelection.wpNumTh.forExtraType_len= awb_adjust->wpNumTh.num;
    memcpy(commV20->xyRegionStableSelection.wpNumTh.forExtraType,awb_adjust->wpNumTh.wpNumThForExtraType,sizeof(float)*awb_adjust->wpNumTh.num);
    commV20->xyRegionStableSelection.xyTypeListSize = awb_adjust->xyTypeListSize;
    commV20->xyRegionStableSelection.varianceLumaTh = awb_adjust->varianceLumaTh;
    memcpy(commV20->weightForNightGainCalc, awb_adjust->temporalGainSetWeight, commV20->weightForNightGainCalc_len*sizeof(unsigned char));
    commV20->singleColorProces.enable = true;
    for(int i=0; i<commV20->singleColorProces.colorBlock_len; i++){
        commV20->singleColorProces.colorBlock[i].index = awb_calib_v20->sIndSelColor[i];
        commV20->singleColorProces.colorBlock[i].meanC= awb_calib_v20->sMeanCh[0][i];
        commV20->singleColorProces.colorBlock[i].meanH= awb_calib_v20->sMeanCh[1][i];
    }
    for(int i=0; i<commV20->singleColorProces.lsUsedForEstimation_len; i++){
        strcpy(commV20->singleColorProces.lsUsedForEstimation[i].name, awb_calib_v20->sNameIllEst[i]);
        commV20->singleColorProces.lsUsedForEstimation[i].RGain= awb_calib_v20->srGain[i];
        commV20->singleColorProces.lsUsedForEstimation[i].BGain= awb_calib_v20->sbGain[i];
    }
    commV20->singleColorProces.alpha= awb_calib_v20->sAlpha;
    memcpy(commV20->lineRgBg,awb_calib_v20->lineRgBg,sizeof(commV20->lineRgBg));
    memcpy(commV20->lineRgProjCCT,awb_calib_v20->lineRgProjCCT,sizeof(commV20->lineRgProjCCT));
    commV20->chrAdpttAdj.enable = awb_adjust->ca_enable;
    commV20->chrAdpttAdj.laCalcFactor = awb_adjust->ca_LACalcFactor;
    memcpy(commV20->chrAdpttAdj.targetGain, awb_calib_v20->ca_targetGain, sizeof(commV20->chrAdpttAdj.targetGain));
    commV20->remosaicCfg.enable = awb_adjust->remosaic_cfg.enable;
    commV20->remosaicCfg.applyInvWbGainEnable= false;
    memcpy(commV20->remosaicCfg.sensorWbGain, awb_adjust->remosaic_cfg.sensor_awb_gain, sizeof(commV20->remosaicCfg.sensorWbGain));
    memcpy(&commV20->wbGainOffset, &awb_adjust->wbGainOffset, sizeof(commV20->wbGainOffset));

}
void convertCalib2calibV2(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2)
{

    convertCalib2calibV20(calib, calibV2);
    convertCalib2calibV21(calib, calibV2);

}




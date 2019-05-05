#include <ebase/types.h>
//#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/utl_fixfloat.h>
#include <calib_xml/calibdb.h>
#include <base/log.h>

#include "cam_ia10_engine.h"
#include "cam_ia10_engine_isp_modules.h"
#include "linux/v4l2-controls.h"
#include "linux/media/rk-isp-config.h"
#include <common/cam_types.h>
#include <cam_types.h>
#include <map>
#include <string>

static std::map<string, CalibDb*> g_CalibDbHandlesMap;

CamIA10Engine::CamIA10Engine():
    aecContext(NULL),
    aecDesc(NULL),
    aecParams(NULL),
    awbContext(NULL),
    awbDesc(NULL),
    awbParams(NULL),
    afContext(NULL),
    afDesc(NULL),
    afParams(NULL),
    mSensorEntityName(NULL),
    mIspVer(0),
    mXMLIspOutputType(0),
    mOTPInfo(NULL)
{
    init();
    /*
       mStats.sensor_mode.pixel_clock_freq_mhz = 180;
       mStats.sensor_mode.pixel_periods_per_line = 2688;
       mStats.sensor_mode.isp_input_width = 2592;
       mStats.sensor_mode.isp_input_height = 1944;
       */
}

CamIA10Engine::~CamIA10Engine() {
    //LOGD("%s: E", __func__);
    deinit();
    //LOGD("%s: x", __func__);
}

RESULT CamIA10Engine::restart() {
    deinit();
    init();
    return initStatic(g_aiqb_data_file, mSensorEntityName, mIspVer);
}

RESULT CamIA10Engine::init() {
    hCamCalibDb = NULL;
    mWdrEnabledState = BOOL_FALSE;
    mLightMode = LIGHT_MODE_DAY;
    mInitDynamic = false;
    mFrameId = 0;
    mStatisticsUpdated = BOOL_FALSE;
    memset(&dCfg, 0x00, sizeof(struct CamIA10_DyCfg));
    memset(&dCfgShd, 0x00, sizeof(struct CamIA10_DyCfg));
    memset(&lastAwbResult, 0, sizeof(lastAwbResult));
    memset(&curAwbResult, 0, sizeof(curAwbResult));
    memset(&curAecResult, 0, sizeof(curAecResult));
    memset(&lastAecResult, 0, sizeof(lastAecResult));
    memset(&adpfCfg, 0, sizeof(adpfCfg));
    memset(&awbcfg, 0, sizeof(awbcfg));
    memset(&aecCfg, 0, sizeof(aecCfg));
    memset(&mStats, 0, sizeof(mStats));
    memset(&awdrCfg, 0, sizeof(awdrCfg));

    memset(&mAECHalCfg, 0, sizeof(mAECHalCfg));
    memset(&mAWBHalCfg, 0, sizeof(mAWBHalCfg));
    memset(&mAFCHalCfg, 0, sizeof(mAFCHalCfg));
    memset(&mAECHalCfgShd, 0, sizeof(mAECHalCfgShd));
    memset(&mAWBHalCfgShd, 0, sizeof(mAWBHalCfgShd));
    memset(&mAFCHalCfgShd, 0, sizeof(mAFCHalCfgShd));

    hCamCalibDb = NULL;
    hAwb =	NULL;
    hAdpf = NULL;
    hAf = NULL;
    hAwdr = NULL;
    lastAecResult.regIntegrationTime = -1;
    lastAecResult.regGain = -1;
    lastAecResult.coarse_integration_time = -1;
    lastAecResult.analog_gain_code_global = -1;

    //init for Hdr
    for(int i=0;i<3;i++){
        lastAecResult.RegHdrGains[i]=-1;
        lastAecResult.RegHdrTime[i]=-1;
    }

    return 0;
}

RESULT CamIA10Engine::deinit() {
    if (hAwb) {
        if (awbDesc) {
            awbDesc->free_results(awbContext, NULL, 0);
            awbDesc->destroy_context(awbContext);
        }
        hAwb = NULL;
    }

    if (hAf) {
        if (afDesc) {
            afDesc->free_results(afContext, NULL, 0);
            afDesc->destroy_context(afContext);
        }

        hAf = NULL;
    }

    if (aecDesc) {
        aecDesc->free_results(aecContext, NULL, 0);
        aecDesc->destroy_context(aecContext);
    }

    if (hAdpf) {
        AdpfRelease(hAdpf);
        hAdpf = NULL;
    }

    if (hAwdr) {
        AwdrRelease(hAwdr);
        hAwdr = NULL;
    }

    hCamCalibDb = NULL;
    mStatisticsUpdated = BOOL_FALSE;
    mInitDynamic = false;
    mFrameId = 0;

    return 0;
}

RESULT CamIA10Engine::initStatic
(
 char* aiqb_data_file,
 const char* sensor_entity_name,
 int isp_ver
 ) {
    RESULT result = RET_FAILURE;
    CamCalibDbMetaData_t dbMeta;
    if (!hCamCalibDb) {
        std::string iq_file_str(aiqb_data_file);
        std::map<string, CalibDb*>::iterator it;
        it = g_CalibDbHandlesMap.find(iq_file_str);
        if (it != g_CalibDbHandlesMap.end()) {
            CalibDb* calibdb_p = it->second;
            hCamCalibDb = calibdb_p->GetCalibDbHandle();
            LOGD("use cached calibdb for %s !", aiqb_data_file);
        } else {
            CalibDb* calibdb_p = new CalibDb();
            if (calibdb_p->CreateCalibDb(aiqb_data_file)) {
                LOGD("create calibdb from %s success.", aiqb_data_file);
                hCamCalibDb = calibdb_p->GetCalibDbHandle();
                g_CalibDbHandlesMap[iq_file_str] = calibdb_p;
            } else {
                LOGE("create calibdb from %s failed", aiqb_data_file);
                delete calibdb_p;
                goto init_fail;
            }
        }
    }

    strcpy(g_aiqb_data_file, aiqb_data_file);
    mSensorEntityName = sensor_entity_name;
    mIspVer = isp_ver;

    result = CamCalibDbGetMetaData(hCamCalibDb, &dbMeta);
    if (result != RET_SUCCESS) {
        LOGE("get xml db meta failed");
        goto init_fail;
    }

    result = CamCalibDbGetOTPGlobal(hCamCalibDb, &mOTPInfo);
    if (result != RET_SUCCESS) {
        LOGE("get xml db meta failed");
        goto init_fail;
    }

    mXMLIspOutputType = dbMeta.isp_output_type;

    result = initAEC();
    if (result != RET_SUCCESS)
        goto init_fail;

    result = initAWB();
    if (result != RET_SUCCESS)
        goto init_fail;

    result = initAF();
    if (result != RET_SUCCESS)
        goto init_fail;

    return RET_SUCCESS;
init_fail:
    return result;
}

//#include "unistd.h"

RESULT CamIA10Engine::initDynamic(struct CamIA10_DyCfg* cfg) {
    RESULT result = RET_SUCCESS;
    mInitDynamic = true;
    dCfg = *cfg;

    LOGD("init dynamic af mode: %d, shdmode: %d", dCfg.afc_cfg.mode, dCfgShd.afc_cfg.mode);

    // check if BW sensor or raw used as BW mode
    if (cfg->sensor_mode.is_bw_sensor) {
        if (mXMLIspOutputType == isp_color_output_type)
            LOGE("xml color type is not allowed for bw sensor !");
    }

    result = updateAeConfig(cfg);
    if (result != RET_SUCCESS) {
        LOGE("update AE dynamic config failed");
        goto initDynamic_end;
    }

    result = updateAwbConfig(cfg);
    if (result != RET_SUCCESS) {
        LOGE("update AWB dynamic config failed");
        goto initDynamic_end;
    }

    result = updateAfConfig(cfg);
    if (result != RET_SUCCESS) {
        LOGE("update AF dynamic config failed");
        goto initDynamic_end;
    }

    adpfCfg.data.db.width = cfg->sensor_mode.isp_input_width;
    adpfCfg.data.db.height = cfg->sensor_mode.isp_input_height;
    adpfCfg.data.db.hCamCalibDb  = hCamCalibDb;
    if (!hAdpf) {
        result = AdpfInit(&hAdpf, &adpfCfg);
    } else {
        result = AdpfConfigure(hAdpf, &adpfCfg);
        if (result != RET_SUCCESS)
            goto initDynamic_end;
    }
    //mLightMode = cfg->LightMode;

    //ALOGE("%s: 222 lightmode:%d!", __func__, cfg->LightMode);

    if (!hAwdr) {
        awdrCfg.hCamCalibDb = hCamCalibDb;
        result = AwdrInit(&hAwdr, &awdrCfg);
        if (result != RET_SUCCESS)
            goto initDynamic_end;
    } else {
        result = AwdrConfigure(hAwdr, &awdrCfg);
        if (result != RET_SUCCESS)
            goto initDynamic_end;
    }

    dCfgShd = dCfg;

initDynamic_end:
    return result;
}

#define AWB_GAIN_REG_VALUE_MAX (UTL_FIX_MAX_U0208)
#define AWB_GAIN_REG_VALUE_MIN (UTL_FIX_MIN_U0208)

RESULT HalAwbGains2CamerIcGains
(
 AwbGains_t*      pAwbGains,
 CamerIcGains_t*  pCamerIcGains
 ) {
    RESULT result = RET_SUCCESS;

    LOGV( "%s: (enter)\n", __FUNCTION__);

    if ((pAwbGains != NULL) && (pCamerIcGains != NULL)) {
        if(pAwbGains->fRed > AWB_GAIN_REG_VALUE_MAX)
            pAwbGains->fRed = AWB_GAIN_REG_VALUE_MAX;
        if(pAwbGains->fRed < AWB_GAIN_REG_VALUE_MIN)
            pAwbGains->fRed = AWB_GAIN_REG_VALUE_MIN;

        if(pAwbGains->fGreenR > AWB_GAIN_REG_VALUE_MAX)
            pAwbGains->fGreenR = AWB_GAIN_REG_VALUE_MAX;
        if(pAwbGains->fGreenR < AWB_GAIN_REG_VALUE_MIN)
            pAwbGains->fGreenR = AWB_GAIN_REG_VALUE_MIN;

        if(pAwbGains->fGreenB > AWB_GAIN_REG_VALUE_MAX)
            pAwbGains->fGreenB = AWB_GAIN_REG_VALUE_MAX;
        if(pAwbGains->fGreenB < AWB_GAIN_REG_VALUE_MIN)
            pAwbGains->fGreenB = AWB_GAIN_REG_VALUE_MIN;

        if(pAwbGains->fBlue > AWB_GAIN_REG_VALUE_MAX)
            pAwbGains->fBlue = AWB_GAIN_REG_VALUE_MAX;
        if(pAwbGains->fBlue < AWB_GAIN_REG_VALUE_MIN)
            pAwbGains->fBlue = AWB_GAIN_REG_VALUE_MIN;

        pCamerIcGains->Red      = UtlFloatToFix_U0208(pAwbGains->fRed);
        pCamerIcGains->GreenR   = UtlFloatToFix_U0208(pAwbGains->fGreenR);
        pCamerIcGains->GreenB   = UtlFloatToFix_U0208(pAwbGains->fGreenB);
        pCamerIcGains->Blue     = UtlFloatToFix_U0208(pAwbGains->fBlue);
    } else {
        result = RET_NULL_POINTER;
    }

    LOGV( "%s: (exit %d)\n", __FUNCTION__, result);

    return (result);
}

/******************************************************************************
 * AwbXtalk2CamerIcXtalk()
 *****************************************************************************/
RESULT HalAwbXtalk2CamerIcXtalk
(
 Cam3x3FloatMatrix_t* pAwbXTalkMatrix,
 CamerIc3x3Matrix_t*  pXTalkMatrix
 ) {
    RESULT result = RET_SUCCESS;

    LOGV( "%s: (enter)\n", __FUNCTION__);

    if ((pAwbXTalkMatrix != NULL) && (pXTalkMatrix != NULL)) {
        int32_t i;
        for (i = 0UL; i < 9; i++) {
            pXTalkMatrix->Coeff[i] = UtlFloatToFix_S0407(pAwbXTalkMatrix->fCoeff[i]);
        }
    } else {
        result = RET_NULL_POINTER;
    }

    LOGV( "%s: (exit %d)\n", __FUNCTION__, result);

    return (result);
}

/******************************************************************************
 * AwbXTalkOffset2CamerIcXTalkOffset()
 *****************************************************************************/
RESULT HalAwbXTalkOffset2CamerIcXTalkOffset
(
 Cam1x3FloatMatrix_t*     pAwbXTalkOffset,
 CamerIcXTalkOffset_t*    pCamerIcXTalkOffset
 ) {
    RESULT result = RET_SUCCESS;

    LOGV( "%s: (enter)\n", __FUNCTION__);

    if ((pAwbXTalkOffset != NULL) && (pCamerIcXTalkOffset != NULL)) {
        if ((pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_RED] > 2047.0f)
            || (pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_RED] < -2048.0f)
            || (pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_GREEN] > 2047.0f)
            || (pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_GREEN] < -2048.0f)
            || (pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_BLUE] > 2047.0f)
            || (pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_BLUE] < -2048.0f)) {
            result = RET_OUTOFRANGE;
        } else {
            pCamerIcXTalkOffset->Red      = UtlFloatToFix_S1200(pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_RED]);
            pCamerIcXTalkOffset->Green    = UtlFloatToFix_S1200(pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_GREEN]);
            pCamerIcXTalkOffset->Blue     = UtlFloatToFix_S1200(pAwbXTalkOffset->fCoeff[CAM_3CH_COLOR_COMPONENT_BLUE]);
        }
    } else {
        result = RET_NULL_POINTER;
    }

    LOGV( "%s: (exit %d)\n", __FUNCTION__, result);

    return (result);
}

RESULT HalCamerIcAwbMeasure2AwbMeasure
(
 CamerIcAwbMeasuringResult_t*   pCamericMeasResult,
 AwbMeasuringResult_t*          pAwbMeasuringResult

 ) {
    RESULT result = RET_SUCCESS;

    LOGV( "%s: (enter)\n", __FUNCTION__);
    pAwbMeasuringResult->NoWhitePixel = pCamericMeasResult->NoWhitePixel;
    pAwbMeasuringResult->MeanY__G = (float)pCamericMeasResult->MeanY__G;
    pAwbMeasuringResult->MeanCb__B = (float)pCamericMeasResult->MeanCb__B;
    pAwbMeasuringResult->MeanCr__R = (float)pCamericMeasResult->MeanCr__R;
    LOGV( "%s: (exit %d)\n", __FUNCTION__, result);

    return (result);
}

RESULT CamIA10Engine::updateAeConfig(struct CamIA10_DyCfg* cfg) {
    RESULT result = RET_SUCCESS;

    struct HAL_AecCfg* set, *shd;
    set = &dCfg.aec_cfg;
    shd = &dCfgShd.aec_cfg;

    uint16_t out_width, out_height, out_hOff, out_vOff;
    if (set->win.left_hoff == 0 && set->win.top_voff == 0 &&
        set->win.right_width == 0 && set->win.bottom_height == 0) {
        out_width = cfg->sensor_mode.isp_input_width;
        out_height = cfg->sensor_mode.isp_input_height;
        out_hOff = 0;
        out_vOff = 0;
    } else {
        out_width = set->win.right_width;
        out_height = set->win.bottom_height;
        out_hOff = set->win.left_hoff;
        out_vOff = set->win.top_voff;
    }

    set->win.left_hoff = out_hOff;
    set->win.top_voff = out_vOff;
    set->win.right_width = out_width;
    set->win.bottom_height = out_height;

    aecCfg.LinePeriodsPerField =
        dCfg.sensor_mode.line_periods_per_field;
    aecCfg.PixelClockFreqMHZ =
        dCfg.sensor_mode.pixel_clock_freq_mhz == 0 ?
        16.8 : dCfg.sensor_mode.pixel_clock_freq_mhz;
    aecCfg.PixelPeriodsPerLine =
        dCfg.sensor_mode.pixel_periods_per_line == 0 ?
        1312 : dCfg.sensor_mode.pixel_periods_per_line;

    LOGD("config aec sensor mode, HTS: %f, VTS: %f, PCLK: %f",
         aecCfg.PixelPeriodsPerLine,
         aecCfg.LinePeriodsPerField,
         aecCfg.PixelClockFreqMHZ);

    if ((set->win.left_hoff != shd->win.left_hoff) ||
        (set->win.top_voff != shd->win.top_voff) ||
        (set->win.right_width != shd->win.right_width) ||
        (set->win.bottom_height != shd->win.bottom_height) ||
        (set->meter_mode != shd->meter_mode) ||
        (set->mode != shd->mode) ||
        (set->flk != shd->flk) ||
        (set->ae_bias != shd->ae_bias)||
        (set->frame_time_ms_min != shd->frame_time_ms_min)||
        (set->frame_time_ms_max != shd->frame_time_ms_max)||
        mLightMode != cfg->LightMode) {
        //cifisp_histogram_mode mode = CIFISP_HISTOGRAM_MODE_RGB_COMBINED;
        cam_ia10_isp_hst_update_stepSize(
                                         aecCfg.HistMode,
                                         aecCfg.GridWeights.uCoeff,
                                         set->win.right_width,
                                         set->win.bottom_height,
                                         mIspVer,
                                         &(aecCfg.StepSize));

        //LOGD("aec set win:%dx%d",
        //  set->win.right_width,set->win.bottom_height);

        if (set->flk == HAL_AE_FLK_OFF)
            aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_OFF;
        else if (set->flk == HAL_AE_FLK_50)
            aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_100HZ;
        else if (set->flk == HAL_AE_FLK_60)
            aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_120HZ;
        else
            aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_100HZ;

        if (set->meter_mode == HAL_AE_METERING_MODE_CENTER) {
#if 0
            unsigned char gridWeights[25];
            //cifisp_histogram_mode mode = CIFISP_HISTOGRAM_MODE_RGB_COMBINED;

            gridWeights[0] = 0x00;    //weight_00to40
            gridWeights[1] = 0x00;
            gridWeights[2] = 0x01;
            gridWeights[3] = 0x00;
            gridWeights[4] = 0x00;

            gridWeights[5] = 0x00;    //weight_01to41
            gridWeights[6] = 0x02;
            gridWeights[7] = 0x02;
            gridWeights[8] = 0x02;
            gridWeights[9] = 0x00;

            gridWeights[10] = 0x00;    //weight_02to42
            gridWeights[11] = 0x04;
            gridWeights[12] = 0x08;
            gridWeights[13] = 0x04;
            gridWeights[14] = 0x00;

            gridWeights[15] = 0x00;    //weight_03to43
            gridWeights[16] = 0x02;
            gridWeights[17] = 0x00;
            gridWeights[18] = 0x02;
            gridWeights[19] = 0x00;

            gridWeights[20] = 0x00;    //weight_04to44
            gridWeights[21] = 0x00;
            gridWeights[22] = 0x00;
            gridWeights[23] = 0x00;
            gridWeights[24] = 0x00;
            memcpy(aecCfg.GridWeights.uCoeff, gridWeights, sizeof(gridWeights));
#else
            //do nothing ,just use the xml setting
#endif
        } else if (set->meter_mode == HAL_AE_METERING_MODE_AVERAGE) {
            memset(aecCfg.GridWeights.uCoeff, 0x01, sizeof(aecCfg.GridWeights.uCoeff));
        } else
            ALOGE("%s:not support %d metering mode!", __func__, set->meter_mode);

        //set ae bias
        {
            CamCalibAecGlobal_t* pAecGlobal;
            CamCalibDbGetAecGlobal(hCamCalibDb, &pAecGlobal);
            float SetPoint = pAecGlobal->SetPoint ;
            if(cfg->LightMode == LIGHT_MODE_NIGHT && pAecGlobal->NightSetPoint != 0)
                SetPoint = pAecGlobal->NightSetPoint;

            aecCfg.SetPoint = (SetPoint) +
                (set->ae_bias / 100.0f) * MAX(10, SetPoint / (1 - pAecGlobal->ClmTolerance / 100.0f) / 10.0f) ;

            mLightMode = cfg->LightMode;
#if 1
            if (set->frame_time_ms_min != 0 && set->frame_time_ms_max != 0) {
                aecCfg.FpsSetEnable = true;
                aecCfg.isFpsFix = false;
                int ecmCnt = sizeof(aecCfg.EcmTimeDot.fCoeff) / sizeof (float);

                for (int i = 1; i < ecmCnt - 3; i++) {
                    aecCfg.EcmTimeDot.fCoeff[i] = (float)set->frame_time_ms_min / 1000;
                }
                for (int i = ecmCnt - 3; i < ecmCnt; i++) {
                    aecCfg.EcmTimeDot.fCoeff[i] = (float)set->frame_time_ms_max / 1000;
                }
                aecCfg.MinFrameDuration = (float)set->frame_time_ms_min / 1000;
                aecCfg.MaxFrameDuration = (float)set->frame_time_ms_max / 1000;
                LOGD("sensor param (%d)=[%f-%f-%f-%f-%f-%f] vts: %f, vtsMax: %d, pclk: %f, hts: %f\n",
                     ecmCnt,
                     aecCfg.EcmTimeDot.fCoeff[0],
                     aecCfg.EcmTimeDot.fCoeff[1],
                     aecCfg.EcmTimeDot.fCoeff[2],
                     aecCfg.EcmTimeDot.fCoeff[3],
                     aecCfg.EcmTimeDot.fCoeff[4],
                     aecCfg.EcmTimeDot.fCoeff[5],
                     aecCfg.LinePeriodsPerField,
                     mStats.sensor_mode.line_periods_per_field,
                     aecCfg.PixelClockFreqMHZ,
                     aecCfg.PixelPeriodsPerLine);
            }
#endif
        }
        switch (set->mode) {
            case HAL_AE_OPERATION_MODE_MANUAL:
               aecCfg.AecMode = AEC_MODE_MANUAL;
               aecCfg.ManExpoSecs = (float)set->frame_time_ms_min / 1000.0f;
               aecCfg.ManGains = set->manual_gains;
               break;
            case HAL_AE_OPERATION_MODE_AUTO:
               aecCfg.AecMode = AEC_MODE_AUTO;
               break;
            default: // just support auto and manual mode now
               aecCfg.AecMode = AEC_MODE_AUTO;
        }

        if (cfg->aaa_locks & HAL_3A_LOCKS_EXPOSURE) {
            // TODO
            LOGD("lock ae");
        }

        if (aecDesc != NULL) {
            aecDesc->update_ae_params(aecContext, &aecCfg);
        }//AecUpdateConfig(&aecCfg);
    }
    *shd = *set;
    return result;
}

RESULT CamIA10Engine::updateAwbConfig(struct CamIA10_DyCfg* cfg) {
    RESULT result = RET_SUCCESS;

    //init awb static
    if (!hAwb) {
        AwbInstanceConfig_t awbInstance;
        if (awbDesc) {
            //LOGD("%s(%d):awb init!", __func__, getpid());
            result = awbDesc->update_awb_params(awbContext, &awbInstance);
        } //result = AwbInit(&awbInstance);

        if (cfg->awb_cfg.mode != HAL_WB_AUTO ||
            mXMLIspOutputType == isp_gray_output_type) {
            // set ie mode to gray
            char prfName[10];
            int i, no;
            CamAwb_V10_IlluProfile_t* pIlluProfile = NULL;
            awbcfg.Mode = AWB_MODE_MANUAL;
            if (mXMLIspOutputType == isp_gray_output_type) {
                strcpy(prfName, "BW");
            } else if (cfg->awb_cfg.mode == HAL_WB_INCANDESCENT) {
                strcpy(prfName, "A");
            } else if (cfg->awb_cfg.mode == HAL_WB_DAYLIGHT) {
                strcpy(prfName, "D65");
            } else if (cfg->awb_cfg.mode == HAL_WB_FLUORESCENT) {
                strcpy(prfName, "F11_TL84");
            } else if (cfg->awb_cfg.mode == HAL_WB_SUNSET) {
                strcpy(prfName, "HORIZON"); //not support now
            } else if (cfg->awb_cfg.mode == HAL_WB_CLOUDY_DAYLIGHT) {
                strcpy(prfName, "F2_CWF");
            } else if (cfg->awb_cfg.mode == HAL_WB_CANDLE) {
                strcpy(prfName, "U30"); //not support now
            } else
                LOGE("%s:not support this awb mode %d !", __func__, cfg->awb_cfg.mode);

            // get number of availabe illumination profiles from database
            result = CamCalibDbGetNoOfAwb_V10_Illuminations(hCamCalibDb, &no);
            // run over all illumination profiles
            for (i = 0; i < no; i++) {
                // get an illumination profile from database
                result = CamCalibDbGetAwb_V10_IlluminationByIdx(hCamCalibDb, i, &pIlluProfile);
                if (strstr(pIlluProfile->name, prfName)) {
                    awbcfg.idx = i;
                    break;
                }
            }

            if (i == no)
                LOGE("%s:can't find %s profile!", __func__, prfName);
        }
        if (result != RET_SUCCESS)
            goto updateAwbConfig_end;
        else
            hAwb = awbInstance.hAwb;

        awbcfg.width = cfg->sensor_mode.isp_input_width;
        awbcfg.height = cfg->sensor_mode.isp_input_height;
        awbcfg.awbWin.h_offs = 0;
        awbcfg.awbWin.v_offs = 0;
        awbcfg.awbWin.h_size = cfg->sensor_mode.isp_input_width;
        awbcfg.awbWin.v_size = cfg->sensor_mode.isp_input_height;

        if (awbDesc) {
            result = awbDesc->update_awb_params(awbContext, &awbcfg);
        } //result = AwbConfigure(hAwb, &awbcfg);

        if (result != RET_SUCCESS) {
            LOGE("%s:awb config failure!", __func__);
            hAwb = NULL;
            goto updateAwbConfig_end;

        }

        AwbRunningOutputResult_t outresult;
        memset(&outresult, 0, sizeof(AwbRunningOutputResult_t));
        if (awbDesc) {
            XCamAwbParam param;
            param.mode = awbcfg.Mode == AWB_MODE_MANUAL ?
                XCAM_AWB_MODE_MANUAL : XCAM_AWB_MODE_AUTO;
            //result = awbDesc->set_stats(awbContext, NULL);
            result = awbDesc->analyze_awb(awbContext, &param);
        } //result =  AwbRun(hAwb, NULL, &outresult);

        if (result == RET_SUCCESS) {
            //convert result
            memset(&curAwbResult, 0, sizeof(curAwbResult));
            convertAwbResult2Cameric(&outresult, &curAwbResult);
        } else {
            hAwb = NULL;
            goto updateAwbConfig_end;
        }
    } else {
        awbcfg.width = cfg->sensor_mode.isp_input_width;
        awbcfg.height = cfg->sensor_mode.isp_input_height;
        if (cfg->awb_cfg.win.right_width && cfg->awb_cfg.win.bottom_height) {
            LOGD("%s: cfg awb win[%dx%d]", __FUNCTION__, cfg->awb_cfg.win.right_width, cfg->awb_cfg.win.bottom_height);
            awbcfg.awbWin.h_offs = cfg->awb_cfg.win.left_hoff;
            awbcfg.awbWin.v_offs = cfg->awb_cfg.win.top_voff;
            awbcfg.awbWin.h_size = cfg->awb_cfg.win.right_width;
            awbcfg.awbWin.v_size = cfg->awb_cfg.win.bottom_height;
        } else {
            awbcfg.awbWin.h_offs = 0;
            awbcfg.awbWin.v_offs = 0;
            awbcfg.awbWin.h_size = cfg->sensor_mode.isp_input_width;
            awbcfg.awbWin.v_size = cfg->sensor_mode.isp_input_height;
        }
        LOGD("%s: update awb win[%dx%d]", __FUNCTION__, awbcfg.awbWin.h_size, awbcfg.awbWin.v_size);
        //mode change ?
        if (cfg->awb_cfg.mode != dCfgShd.awb_cfg.mode) {
            LOGI("@%s %d: AwbMode changed from %d to %d", __FUNCTION__, __LINE__, dCfgShd.awb_cfg.mode, cfg->awb_cfg.mode);
            memset(&lastAwbResult, 0x00, sizeof(lastAwbResult));
            if (cfg->awb_cfg.mode != HAL_WB_AUTO ||
                mXMLIspOutputType == isp_gray_output_type) {
                char prfName[10];
                int i, no;
                CamAwb_V10_IlluProfile_t* pIlluProfile = NULL;
                awbcfg.Mode = AWB_MODE_MANUAL;
                //get index
                //A:3400k
                //D65:6500K artificial daylight
                //CWF:4150K cool whtie fluorescent
                //TL84: 4000K
                //D50: 5000k
                //D75:7500K
                //HORIZON:2300K
                //SNOW :6800k
                //CANDLE:1850K
                if (cfg->awb_cfg.mode == HAL_WB_INCANDESCENT) {
                    strcpy(prfName, "BW");
                } else if (cfg->awb_cfg.mode == HAL_WB_INCANDESCENT) {
                    strcpy(prfName, "A");
                } else if (cfg->awb_cfg.mode == HAL_WB_DAYLIGHT) {
                    strcpy(prfName, "D65");
                } else if (cfg->awb_cfg.mode == HAL_WB_FLUORESCENT) {
                    strcpy(prfName, "F11_TL84");
                } else if (cfg->awb_cfg.mode == HAL_WB_SUNSET) {
                    strcpy(prfName, "HORIZON"); //not support now
                } else if (cfg->awb_cfg.mode == HAL_WB_CLOUDY_DAYLIGHT) {
                    strcpy(prfName, "F2_CWF");
                } else if (cfg->awb_cfg.mode == HAL_WB_CANDLE) {
                    strcpy(prfName, "U30"); //not support now
                } else
                    LOGE("%s:not support this awb mode %d !", __func__, cfg->awb_cfg.mode);

                // get number of availabe illumination profiles from database
                result = CamCalibDbGetNoOfAwb_V10_Illuminations(hCamCalibDb, &no);
                // run over all illumination profiles
                for (i = 0; i < no; i++) {
                    // get an illumination profile from database
                    result = CamCalibDbGetAwb_V10_IlluminationByIdx(hCamCalibDb, i, &pIlluProfile);
                    if (strstr(pIlluProfile->name, prfName)) {
                        awbcfg.idx = i;
                        break;
                    }
                }

                if (i == no) {
                    LOGE("%s:can't find %s profile!", __func__, prfName);
                } else {
                    if (awbDesc) {
                        result = awbDesc->update_awb_params(awbContext, &awbcfg);
                    }
                }
            } else {
                //from manual to auto
                initAWB();
                if (awbDesc) {
                    result = awbDesc->update_awb_params(awbContext, &awbcfg);
                }
            }
        }
        /*
        //awb locks
        if ((cfg->aaa_locks & HAL_3A_LOCKS_WB)
        && (dCfg.awb_cfg.mode == HAL_WB_AUTO)) {
        AwbTryLock(hAwb);
        } else if (dCfg.aaa_locks & HAL_3A_LOCKS_WB)
        AwbUnLock(hAwb);
        */
    }
updateAwbConfig_end:
    return result;
}

RESULT CamIA10Engine::updateAfConfig(struct CamIA10_DyCfg* cfg) {
    RESULT result = RET_SUCCESS;

    struct HAL_AfcCfg* afset;
    afset = &dCfg.afc_cfg;

    if (afDesc) {
        afDesc->configure_af(afContext,
                             cfg->sensor_mode.isp_input_width,
                             cfg->sensor_mode.isp_input_height);
    }
#if 0
    // AF
    struct HAL_AfcCfg* afset;
    struct HAL_AfcCfg* afshd;
    unsigned int isp_ref_width;
    unsigned int isp_ref_height;

    afset = &dCfg.afc_cfg;
    afshd = &dCfgShd.afc_cfg;
    if (afset->mode == HAL_AF_MODE_NOT_SET) {
        dCfgShd = dCfg;
        goto updateAfConfig_end;
    }

    if (cfg->sensor_mode.isp_output_width != 0) {
        isp_ref_width = cfg->sensor_mode.isp_output_width;
        isp_ref_height = cfg->sensor_mode.isp_output_height;
    } else {
        isp_ref_width = cfg->sensor_mode.isp_input_width;
        isp_ref_height = cfg->sensor_mode.isp_input_height;
    }

    {
        //start af
        if (afset->mode != afshd->mode) {
            if (afshd->mode != HAL_AF_MODE_NOT_SET &&
                afshd->mode != HAL_AF_MODE_FIXED)
                AfStop(hAf);

            if (afset->mode == HAL_AF_MODE_CONTINUOUS_VIDEO ||
                afset->mode == HAL_AF_MODE_CONTINUOUS_PICTURE) {
                result = AfStart(hAf, AFM_FSS_ADAPTIVE_RANGE);
                if (result != RET_SUCCESS) {
                    ALOGE("%s: AfStart failure!", __func__);
                    goto updateAfConfig_end;
                }
            } else if (afset->mode == HAL_AF_MODE_AUTO) {
                result = AfOneShot(hAf, AFM_FSS_ADAPTIVE_RANGE);
                if (result != RET_SUCCESS) {
                    ALOGE("%s: AfOneShot failure!", __func__);
                    goto updateAfConfig_end;
                }
            } else if (afset->mode == HAL_AF_MODE_FIXED ||
                       afset->mode == HAL_AF_MODE_NOT_SET) {
                result = AfStop(hAf);
                if (result != RET_SUCCESS) {
                    ALOGE("%s: AfStop failure!", __func__);
                    goto updateAfConfig_end;
                }
            }

            afshd->mode = afset->mode;
        } else if ((afset->mode == HAL_AF_MODE_AUTO) &&
                   (afset->oneshot_trigger == BOOL_TRUE)) {
            afset->oneshot_trigger = BOOL_FALSE;
            result = AfOneShot(hAf, AFM_FSS_ADAPTIVE_RANGE);
            if (result != RET_SUCCESS) {
                ALOGE("%s: AfOneShot failure!", __func__);
                goto updateAfConfig_end;
            }
        }

        if ((afset->win_num != afshd->win_num) ||
            (memcmp(&afset->win_a, &afshd->win_a, sizeof(HAL_Window))) ||
            (memcmp(&afset->win_b, &afshd->win_b, sizeof(HAL_Window))) ||
            (memcmp(&afset->win_c, &afshd->win_c, sizeof(HAL_Window)))) {
            AfConfig_t afcCfg;

            afcCfg.Window_Num = afset->win_num;
            if (afset->win_num >= 1) {
                mapHalWinToIsp(afset->win_a.right_width - afset->win_a.left_hoff,
                               afset->win_a.bottom_height - afset->win_a.top_voff,
                               afset->win_a.left_hoff,
                               afset->win_a.top_voff,
                               isp_ref_width,
                               isp_ref_height,
                               afcCfg.WindowA.h_size,
                               afcCfg.WindowA.v_size,
                               afcCfg.WindowA.h_offs,
                               afcCfg.WindowA.v_offs);
            }
            if (afset->win_num >= 2) {
                mapHalWinToIsp(afset->win_b.right_width - afset->win_b.left_hoff,
                               afset->win_b.bottom_height - afset->win_b.top_voff,
                               afset->win_b.left_hoff,
                               afset->win_b.top_voff,
                               isp_ref_width,
                               isp_ref_height,
                               afcCfg.WindowB.h_size,
                               afcCfg.WindowB.v_size,
                               afcCfg.WindowB.h_offs,
                               afcCfg.WindowB.v_offs);
            }
            if (afset->win_num >= 3) {
                mapHalWinToIsp(afset->win_c.right_width - afset->win_c.left_hoff,
                               afset->win_c.bottom_height - afset->win_c.top_voff,
                               afset->win_c.left_hoff,
                               afset->win_c.top_voff,
                               isp_ref_width,
                               isp_ref_height,
                               afcCfg.WindowC.h_size,
                               afcCfg.WindowC.v_size,
                               afcCfg.WindowC.h_offs,
                               afcCfg.WindowC.v_offs);
            }

            result = AfReConfigure(hAf, &afcCfg);
            if (result != RET_SUCCESS) {
                ALOGE("%s: AfReConfigure failure! result %d", __func__, result);
                goto updateAfConfig_end;
            }

            afshd->win_num = afset->win_num;
            afshd->win_a = afset->win_a;
            afshd->win_b = afset->win_b;
            afshd->win_c = afset->win_c;
        }
    }

    if ((dCfgShd.aaa_locks & HAL_3A_LOCKS_FOCUS) != (dCfg.aaa_locks & HAL_3A_LOCKS_FOCUS)) {
        if (dCfg.aaa_locks & HAL_3A_LOCKS_FOCUS)
            AfTryLock(hAf);
        else
            AfUnLock(hAf);
    }
#endif
    return result;
}

RESULT CamIA10Engine::setStatistics(struct CamIA10_Stats* stats) {
    LOGD("setStatistics(%d)", mFrameId);
    if (mFrameId >= 1)
        mStatisticsUpdated = BOOL_TRUE;
    mStats = *stats;
    mFrameId++;
    return RET_SUCCESS;
}

RESULT CamIA10Engine::runAe(XCamAeParam *param, AecResult_t* result, bool first)
{
    RESULT ret = RET_SUCCESS;

    if (!first) {
        int lastTime = lastAecResult.regIntegrationTime;
        int lastGain = lastAecResult.regGain;

        //zlj add for Hdr result
        int lastTime_L = lastAecResult.RegHdrTime[0];
        int lastGain_L = lastAecResult.RegHdrGains[0];
        int lastTime_M = lastAecResult.RegHdrTime[1];
        int lastGain_M = lastAecResult.RegHdrGains[1];
        int lastTime_S = lastAecResult.RegHdrTime[2];
        int lastGain_S = lastAecResult.RegHdrGains[2];

        if (!(mStats.meas_type & CAMIA10_AEC_MASK) ||
            !(mStats.meas_type & CAMIA10_HST_MASK)) {
            return RET_FAILURE;
        }
        dumpAe();

        if(!lastAecResult.IsHdrExp){
            // run ae every frame
            aecParams = param;
            if (aecDesc != NULL) {
                mStats.aec.sensor_metadata.coarse_integration_time =
                  dCfg.sensor_mode.exp_time_seconds;
                mStats.aec.sensor_metadata.analog_gain_code_global =
                  dCfg.sensor_mode.gains;
                mStats.aec.sensor_metadata.regIntegrationTime =
                  dCfg.sensor_mode.exp_time;
                mStats.aec.sensor_metadata.regGain =
                  dCfg.sensor_mode.gain;
                aecDesc->set_stats(aecContext, &mStats.aec);
                if (!(dCfg.aaa_locks & HAL_3A_LOCKS_EXPOSURE))
                    aecDesc->analyze_ae(aecContext, param);
            }
        }else{ //add check exposure value between AE & 1608 Embedded data
            LOGD( "runAEC - 1608 Time_L=%d,Gain_L=%d,Time_M=%d,Gain_M=%d,Time_S=%d,Gain_S=%d\n",
                  mStats.aec.sensor.exp_time_l,
                  mStats.aec.sensor.gain_l,
                  mStats.aec.sensor.exp_time,
                  mStats.aec.sensor.gain,
                  mStats.aec.sensor.exp_time_s,
                  mStats.aec.sensor.gain_s);

            LOGD( "runAEC - aec Time_L=%d,Gain_L=%d,Time_M=%d,Gain_M=%d,Time_S=%d,Gain_S=%d\n",
                  lastTime_L,
                  lastGain_L,
                  lastTime_M,
                  lastGain_M,
                  lastTime_S,
                  lastGain_S);

            if ((lastTime_L == -1 && lastGain_L== -1 && lastTime_M== -1 && lastGain_M== -1 &&
                 lastTime_S == -1 && lastGain_S== -1)||
                (lastTime_L == (int)mStats.aec.sensor.exp_time_l &&
                 lastGain_L == (int)mStats.aec.sensor.gain_l &&
                 lastTime_S == (int)mStats.aec.sensor.exp_time_s &&
                 lastGain_S == (int)mStats.aec.sensor.gain_s)) {
                aecParams = param;
                if (aecDesc != NULL) {
                    aecDesc->set_stats(aecContext, &mStats.aec);
                    if (!(dCfg.aaa_locks & HAL_3A_LOCKS_EXPOSURE))
                        aecDesc->analyze_ae(aecContext, param);
                }
            }
        }
    } else {
        if (aecDesc != NULL) {
            aecDesc->set_stats(aecContext, NULL);
            aecDesc->update_ae_params(aecContext, &aecCfg);
            aecDesc->analyze_ae(aecContext, NULL);
        }
    }
    getAECResults(result);

    return 0;
}

void CamIA10Engine::dumpAe()
{
#if 0
    int lastTime = lastAecResult.regIntegrationTime;
    int lastGain = lastAecResult.regGain;

    ALOGI("   ");
    if (!mInitDynamic) {
        ALOGI("cccccc check type (%d) runAEC - check exp time=[%d-%d], sensor=[%d-%d]\n",
              mStats.meas_type,
              lastTime, mStats.sensor_mode.exp_time,
              lastGain, mStats.sensor_mode.gain
             );
    } else {
        ALOGI("cccccc check type (%d) runAEC - check exp time=[%d-%d], sensor=[%d-%d]\n",
              mStats.meas_type,
              lastTime, dCfg.sensor_mode.exp_time,
              lastGain, dCfg.sensor_mode.gain
             );
    }

    unsigned char* expmean = mStats.aec.exp_mean;
    for (int i=0; i<25; i+=5) {
        ALOGI("--runAEC-EXPO=[%d-%d-%d-%d-%d]",
              expmean[i], expmean[i+1], expmean[i+2], expmean[i+3], expmean[i+4]);
    }

    unsigned int* hist = mStats.aec.hist_bins;
    for (int i=0; i<16; i+=4) {
        ALOGI("--runAEC-hist=[%d-%d-%d-%d]",
              hist[i], hist[i+1], hist[i+2], hist[i+3]);
    }
#endif

}

RESULT CamIA10Engine::runAwb(XCamAwbParam *param, CamIA10_AWB_Result_t* result, bool first)
{
    RESULT ret = RET_SUCCESS;
    AwbRunningInputParams_t MeasResult;
    AwbRunningOutputResult_t retOuput;

    memset(&MeasResult, 0, sizeof(AwbRunningInputParams_t));
    memset(&retOuput, 0, sizeof(AwbRunningOutputResult_t));
    awbParams = param;
    HalCamerIcAwbMeasure2AwbMeasure(&(mStats.awb), &(MeasResult.MesureResult));
    for (int i = 0; i < AWB_HIST_NUM_BINS; i++)
        MeasResult.HistBins[i] = mStats.aec.hist_bins[i];

    if(!lastAecResult.IsHdrExp){
        MeasResult.fGain = dCfg.sensor_mode.gains;
        MeasResult.fIntegrationTime = dCfg.sensor_mode.exp_time_seconds;
    }else{
        MeasResult.fGain = lastAecResult.analog_gain_code_global/lastAecResult.DCG_Ratio;
        MeasResult.fIntegrationTime = lastAecResult.coarse_integration_time;
        LOGD("%s:fgain=%f,fintegrationTime=%f,DCG_raio=%f\n",__FUNCTION__,
             lastAecResult.analog_gain_code_global,
             lastAecResult.coarse_integration_time,
             lastAecResult.DCG_Ratio);
    }

    MeasResult.Gains = mStats.effct_awb_gains;
    MeasResult.CtMatrix = mStats.effect_CtMatrix;
    MeasResult.CtOffset = mStats.effect_CtOffset;

    if (!first && !(mStats.meas_type & CAMIA10_AWB_MEAS_MASK))
        return RET_FAILURE;

    dumpAwb();

    if (awbDesc) {
        ret = awbDesc->set_stats(awbContext, first ? NULL : &MeasResult);
        if (first)
            awbDesc->update_awb_params(awbContext, &awbcfg);
        if (!(dCfg.aaa_locks & HAL_3A_LOCKS_WB))
            ret = awbDesc->analyze_awb(awbContext, param);
        ret = awbDesc->get_results(awbContext, &retOuput);
    }

    if (ret == RET_SUCCESS) {
        //convert result
        memset(&curAwbResult, 0, sizeof(curAwbResult));
        convertAwbResult2Cameric(&retOuput, &curAwbResult);
    }
    curAwbResult.err_code = retOuput.err_code;

    //getAWBResults(result);
    return ret;
}

void CamIA10Engine::dumpAwb()
{
#if 0
    ALOGI("");
    ALOGI("--AWB Statistics");
    unsigned int* hist = (unsigned int*)MeasResult.HistBins;
    for (int i=0; i<AWB_HIST_NUM_BINS; i+=4) {
        ALOGI("--runAwb-hist=[%d-%d-%d-%d]",
              hist[i], hist[i+1], hist[i+2], hist[i+3]);
    }
    ALOGI("  NoWhitePixel: %d", MeasResult.MesureResult.NoWhitePixel);
    ALOGI("  MeanCr__R: %f", MeasResult.MesureResult.MeanCr__R);
    ALOGI("  MeanY__G: %f", MeasResult.MesureResult.MeanY__G);
    ALOGI("  MeanCb__B: %f", MeasResult.MesureResult.MeanCb__B);
#endif
}

RESULT CamIA10Engine::runAf(XCamAfParam *param, XCam3aResultFocus* result, bool first)
{
    afParams = param;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct HAL_AfcCfg* set;
    struct HAL_AfcCfg* shd;

    set = &dCfg.afc_cfg;
    shd = &dCfgShd.afc_cfg;
    if (set->mode == HAL_AF_MODE_NOT_SET) {
        LOGE("af mode not set");
        return RET_FAILURE;
    }

    if (!first && !(mStats.meas_type & CAMIA10_AFC_MASK))
        return RET_FAILURE;

    if (shd->mode != HAL_AF_MODE_NOT_SET &&
        shd->mode != HAL_AF_MODE_FIXED) {
        if (hAf != NULL) {
            if (afDesc) {
                XCamAfParam param;
                param.focus_mode = XCamAfOperationMode(set->mode);
                param.focus_rect_cnt = set->win_num;
                param.focus_rect[0].left_hoff = set->win_a.left_hoff;
                param.focus_rect[0].top_voff = set->win_a.top_voff;
                param.focus_rect[0].right_width = set->win_a.right_width;
                param.focus_rect[0].bottom_height = set->win_a.bottom_height;

                param.focus_rect[1].left_hoff = set->win_b.left_hoff;
                param.focus_rect[1].top_voff = set->win_b.top_voff;
                param.focus_rect[1].right_width = set->win_b.right_width;
                param.focus_rect[1].bottom_height = set->win_b.bottom_height;

                param.focus_rect[2].left_hoff = set->win_c.left_hoff;
                param.focus_rect[2].top_voff = set->win_c.top_voff;
                param.focus_rect[2].right_width = set->win_c.right_width;
                param.focus_rect[2].bottom_height = set->win_c.bottom_height;

                param.trigger_new_search = set->oneshot_trigger;
                ret = afDesc->set_stats(afContext, &mStats.af);

                if (!(dCfg.aaa_locks & HAL_3A_LOCKS_FOCUS))
                    ret = afDesc->analyze_af(afContext, &param);
            }//AfProcessFrame(hAf, &mStats.af);

            if ((ret != RET_SUCCESS) && (ret != RET_CANCELED))
                ALOGE( "%s AfProcessFrame: %d", __func__, ret );
        } else {
            LOGE("af handle is null");
        }
    }

    getAFResults(result);
    return 0;
}

void CamIA10Engine::setExternalAEHandlerDesc(XCamAEDescription* desc) {
    aecDesc = desc;
    if (aecDesc != NULL) {
        XCam3AContext *context = NULL;
        aecDesc->create_context(&context);
        aecContext = context;
    } else {
        LOGD("Camera ia engine got a null ae handler description.");
    }
}

void CamIA10Engine::setExternalAWBHandlerDesc(XCamAWBDescription* desc) {
    awbDesc = desc;
    if (awbDesc != NULL) {
        XCam3AContext *context = NULL;
        awbDesc->create_context(&context);
        awbContext = context;
    } else {
        LOGD("Camera ia engine got a null awb handler description.");
    }
}

void CamIA10Engine::setExternalAFHandlerDesc(XCamAFDescription* desc) {
    afDesc = desc;
    if (afDesc != NULL) {
        XCam3AContext *context = NULL;
        afDesc->create_context(&context);
        afContext = context;
    } else {
        ALOGI("Camera ia engine got a null af handler description.");
    }
}

RESULT CamIA10Engine::initAWB() {
    //init awbcfg
    const CamerIcAwbMeasuringConfig_t MeasConfig = {
        .MaxY           = 200U,
        .RefCr_MaxR     = 128U,
        .MinY_MaxG      =  30U,
        .RefCb_MaxB     = 128U,
        .MaxCSum        =  20U,
        .MinC           =  20U
    };
    awbcfg.framerate = 0;
    awbcfg.Mode = AWB_MODE_AUTO;
    awbcfg.idx = 1;
    awbcfg.damp = BOOL_TRUE;
    awbcfg.MeasMode = CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR;

    awbcfg.fStableDeviation  = 0.1f;     // 10 %
    awbcfg.fRestartDeviation = 0.2f;     // 20 %

    awbcfg.MeasMode          = CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR;
    awbcfg.MeasConfig        = MeasConfig;
    awbcfg.Flags             = AWB_WORKING_FLAG_USE_DAMPING | AWB_WORKING_FLAG_USE_CC_OFFSET;
    awbcfg.hCamCalibDb = hCamCalibDb;
    if (mIspVer > 0)
        awbcfg.validHistBinsNum = 32;
    else
        awbcfg.validHistBinsNum = 16;
    return RET_SUCCESS;
}

RESULT CamIA10Engine::initAF() {
    LOGD("%s:%d", __func__, __LINE__);
    AfInstanceConfig_t afInstance;
    AfConfig_t afcCfg;
    RESULT result = RET_SUCCESS;
    float distanceDot[7] = {0,16,32,40,48,56,64};
    float vcmDot[7] = {0.2,0.24,0.34,0.4,0.66,1.0,3.0};

    if (afDesc) {
        result = afDesc->update_af_params(afContext, &afInstance);
    } else {
        LOGE("af handler description is null");
    }

    if (result != RET_SUCCESS) {
        LOGE("%s: afInit failure! result %d", __func__, result);
        goto end;
    }
    hAf = afInstance.hAf;

#if 1
    CamCalibAfGlobal_t* pAfGlobal;
    result = CamCalibDbGetAfGlobal(hCamCalibDb, &pAfGlobal);
    if (result != RET_SUCCESS) {
        ALOGE("fail to get pAfGlobal, ret: %d", result);
        return result;
    }

    dCfg.afc_cfg.mode = HAL_AF_MODE_AUTO;
    dCfg.afc_cfg.type.contrast_af = pAfGlobal->contrast_af.enable;
    dCfg.afc_cfg.type.laser_af = pAfGlobal->laser_af.enable;
    dCfg.afc_cfg.type.pdaf = pAfGlobal->pdaf.enable;
    dCfg.afc_cfg.win_num = pAfGlobal->Window_Num;//1;

    dCfg.afc_cfg.win_a.left_hoff = pAfGlobal->WindowA.h_offs;//512;
    dCfg.afc_cfg.win_a.right_width = pAfGlobal->WindowA.h_offs + pAfGlobal->WindowA.h_size;//1024;
    dCfg.afc_cfg.win_a.top_voff = pAfGlobal->WindowA.v_offs;//512;
    dCfg.afc_cfg.win_a.bottom_height = pAfGlobal->WindowA.v_offs + pAfGlobal->WindowA.v_size;//1024;

    afcCfg.Afss = AfSearchStrategy_t(pAfGlobal->contrast_af.Afss);

    afcCfg.AfType.contrast_af = pAfGlobal->contrast_af.enable;
    afcCfg.AfType.laser_af = pAfGlobal->laser_af.enable;
    afcCfg.AfType.pdaf = pAfGlobal->pdaf.enable;
    memcpy(afcCfg.LaserAf.distanceDot, pAfGlobal->laser_af.distanceDot, sizeof(afcCfg.LaserAf.distanceDot));
    memcpy(afcCfg.LaserAf.vcmDot, pAfGlobal->laser_af.vcmDot, sizeof(afcCfg.LaserAf.vcmDot));

    afcCfg.Window_Num = pAfGlobal->Window_Num;
    memcpy(&afcCfg.WindowA, &pAfGlobal->WindowA, sizeof(afcCfg.WindowA));
    memcpy(&afcCfg.WindowB, &pAfGlobal->WindowB, sizeof(afcCfg.WindowB));
    memcpy(&afcCfg.WindowC, &pAfGlobal->WindowC, sizeof(afcCfg.WindowC));

	afcCfg.ContrastAf.TrigThers = pAfGlobal->contrast_af.TrigThers;
	afcCfg.ContrastAf.TrigValue = pAfGlobal->contrast_af.TrigValue;
	afcCfg.ContrastAf.TrigFrames = pAfGlobal->contrast_af.TrigFrames;
	afcCfg.ContrastAf.TrigAntiFlash = pAfGlobal->contrast_af.TrigAntiFlash ? BOOL_TRUE : BOOL_FALSE;

	afcCfg.ContrastAf.StableThers = pAfGlobal->contrast_af.StableThers;
	afcCfg.ContrastAf.StableValue = pAfGlobal->contrast_af.StableValue;
	afcCfg.ContrastAf.StableFrames = pAfGlobal->contrast_af.StableFrames;
	afcCfg.ContrastAf.StableTime = pAfGlobal->contrast_af.StableTime;

	afcCfg.ContrastAf.OutFocusValue = pAfGlobal->contrast_af.OutFocusValue;
	afcCfg.ContrastAf.OutFocusLuma = pAfGlobal->contrast_af.OutFocusLuma;
	afcCfg.ContrastAf.OutFocusPos = pAfGlobal->contrast_af.OutFocusPos;

    afcCfg.ContrastAf.FullSteps = pAfGlobal->contrast_af.FullSteps;
    afcCfg.ContrastAf.FullRangeTbl = pAfGlobal->contrast_af.FullRangeTbl;
    afcCfg.ContrastAf.FullDir = AfSearchDir_t(pAfGlobal->contrast_af.FullDir);
    afcCfg.ContrastAf.AdaptiveSteps = pAfGlobal->contrast_af.AdaptiveSteps;
    afcCfg.ContrastAf.AdaptRangeTbl = pAfGlobal->contrast_af.AdaptRangeTbl;
    afcCfg.ContrastAf.AdaptiveDir = AfSearchDir_t(pAfGlobal->contrast_af.AdaptiveDir);

	afcCfg.ContrastAf.FinishThersMain = pAfGlobal->contrast_af.FinishThersMain;
	afcCfg.ContrastAf.FinishThersSub = pAfGlobal->contrast_af.FinishThersSub;
	afcCfg.ContrastAf.FinishThersOffset = pAfGlobal->contrast_af.FinishThersOffset;


	LOGD("Afss: %d ", afcCfg.Afss);
	LOGD("AfType.contrast_af: %d laser_af: %d pdaf: %d ",
		afcCfg.AfType.contrast_af,
		afcCfg.AfType.laser_af,
		afcCfg.AfType.pdaf);
	LOGD("Window_Num: %d WindowA:offs:%d %d size:%dx%d ",
		afcCfg.Window_Num,
		afcCfg.WindowA.h_offs,
		afcCfg.WindowA.v_offs,
		afcCfg.WindowA.h_size,
		afcCfg.WindowA.v_size);
	LOGD("TrigThers:%f TrigValue:%d TrigFrames:%d TrigAntiFlash:%d",
		afcCfg.ContrastAf.TrigThers,
		afcCfg.ContrastAf.TrigValue,
		afcCfg.ContrastAf.TrigFrames,
		afcCfg.ContrastAf.TrigAntiFlash);
	LOGD("StableThers:%f StableValue:%d StableFrames:%d StableTime:%d",
		afcCfg.ContrastAf.StableThers,
		afcCfg.ContrastAf.StableValue,
		afcCfg.ContrastAf.StableFrames,
		afcCfg.ContrastAf.StableTime);
	LOGD("OutFocusValue:%d OutFocusLuma:%f OutFocusPos:%d",
		afcCfg.ContrastAf.OutFocusValue,
		afcCfg.ContrastAf.OutFocusLuma,
		afcCfg.ContrastAf.OutFocusPos);
	LOGD("FullDir:%d AdaptiveDir:%d ",
		afcCfg.ContrastAf.FullDir,
		afcCfg.ContrastAf.AdaptiveDir);
	LOGD("FinishThersMain:%f FinishThersSub:%f FinishThersOffset:%d",
		afcCfg.ContrastAf.FinishThersMain,
		afcCfg.ContrastAf.FinishThersSub,
		afcCfg.ContrastAf.FinishThersOffset);
#else
    dCfg.afc_cfg.mode = HAL_AF_MODE_AUTO;
    dCfg.afc_cfg.type.contrast_af = true;
    dCfg.afc_cfg.type.laser_af = false;
    dCfg.afc_cfg.type.pdaf = false;
    dCfg.afc_cfg.win_num = 1;
    dCfg.afc_cfg.win_a.left_hoff = 0;
    dCfg.afc_cfg.win_a.right_width = 0;
    dCfg.afc_cfg.win_a.top_voff = 0;
    dCfg.afc_cfg.win_a.bottom_height = 0;

    afcCfg.AfType.contrast_af = true;
    afcCfg.AfType.laser_af = false;
    afcCfg.AfType.pdaf = false;

    memcpy(afcCfg.LaserAf.distanceDot, distanceDot, sizeof(afcCfg.LaserAf.distanceDot));
    memcpy(afcCfg.LaserAf.vcmDot, vcmDot, sizeof(afcCfg.LaserAf.vcmDot));

#endif

    if (afDesc) {
        result = afDesc->update_af_params(afContext, &afcCfg);
    }
    if (result != RET_SUCCESS) {
        LOGE("%s: AfConfigure failure! result %d", __func__, result);
        goto end;
    }

    dCfgShd.afc_cfg = dCfg.afc_cfg;
    LOGD("initAF af mode: %d, shdmode: %d", dCfg.afc_cfg.mode, dCfgShd.afc_cfg.mode);
end:
    return result;

}
/*
   RESULT CamHwItfFocusSet(void* handle, const uint32_t AbsStep) {
   ALOGE("%s", __func__);
   return 0;
   }

   RESULT CamHwItfFocusGet(void* handle, uint32_t* pAbsStep) {
   ALOGE("%s", __func__);
   return 0;
   }

   RESULT CamHwItfInitMotoDrive(void* handle) {
   ALOGE("%s", __func__);
   return 0;
   }
   RESULT CamHwItfSetupMotoDrive(void* handle, uint32_t* pMaxStep) {
   ALOGE("%s", __func__);
   return 0;
   }

   RESULT CamIA10Engine::initAF() {
   struct IsiSensor_s gCamHwLtfConfig = {
   .pIsiMdiInitMotoDriveMds = CamHwItfInitMotoDrive,
   .pIsiMdiSetupMotoDrive = CamHwItfSetupMotoDrive,
   .pIsiMdiFocusSet = CamHwItfFocusSet,
   .pIsiMdiFocusGet = CamHwItfFocusGet,
   };

//LOGD("%s:%d", __func__, __LINE__);

//init afcCfg
afcCfg.hSensor = 0;
afcCfg.hSubSensor = 0;
afcCfg.Afss = AFM_FSS_ADAPTIVE_RANGE;

IsiMdiSetup(&gCamHwLtfConfig);

return RET_SUCCESS;
}
*/
void CamIA10Engine::mapSensorExpToHal
(
 int sensorGain,
 int sensorInttime,
 float& halGain,
 float& halInttime
 ) {

    float gainRange[] = {
        1,  1.9375, 16,  16, 1, 0, 15,
        2,  3.875,  8,  0, 1, 16, 31,
        4,  7.75,   4,  -32, 1, 48, 63,
        8,  15.5,   2,  -96, 1, 112, 127
    };

    float* pgainrange = NULL;
    uint32_t size = 0;

    if (aecCfg.pGainRange == NULL || aecCfg.GainRange_size <= 0) {
        pgainrange = gainRange;
        size = sizeof(gainRange) / sizeof(float);
    } else {
        pgainrange = aecCfg.pGainRange;
        size = aecCfg.GainRange_size;
    }

    int *revert_gain_array = (int *)malloc((size/7*2) * sizeof(int));
    if(revert_gain_array == NULL) {
        ALOGE("%s: malloc fail", __func__);
        return;
    }

    for(uint32_t i=0; i<(size/7); i++) {
        revert_gain_array[i*2 + 0] = (int)((pgainrange[i*7 + 0] * pgainrange[i*7 + 2] - pgainrange[i*7 + 3]) / pgainrange[i*7 + 4] + 0.5);
        revert_gain_array[i*2 + 1] = (int)((pgainrange[i*7 + 1] * pgainrange[i*7 + 2] - pgainrange[i*7 + 3]) / pgainrange[i*7 + 4] + 0.5);
    }


    // AG = (((C1 * analog gain - C0) / M0) + 0.5f
    float C1, C0, M0, minReg, maxReg, minHalGain, maxHalGain;
    float ag = sensorGain;
    uint32_t i = 0;
    for(i=0; i<(size/7); i++) {
        if (ag >= revert_gain_array[i*2+0] && ag <= revert_gain_array[i*2+1]) {
            C1 = pgainrange[i*7 + 2];
            C0 = pgainrange[i*7 + 3];
            M0 = pgainrange[i*7 + 4];
            minReg = pgainrange[i*7 + 5];
            maxReg = pgainrange[i*7 + 6];
#if 0
            LOGD("%s: gain(%f) c1:%f c0%f m0:%f min:%f max:%f",
                 __func__, ag, C1, C0, M0, minReg, maxReg);
#endif
            break;
        }
    }

    if(i > (size/7)) {
        ALOGE("GAIN OUT OF RANGE: lasttime-gain: %d-%d", sensorInttime, sensorGain);
        C1 = 16;
        C0 = 0;
        M0 = 1;
        minReg = 16;
        maxReg = 255;
    }

    halGain = ((float)sensorGain * M0 + C0) / C1;
    minHalGain = ((float)minReg * M0 + C0) / C1;
    maxHalGain = ((float)maxReg * M0 + C0) / C1;
    if (halGain < minHalGain)
        halGain = minHalGain;
    if (halGain > maxHalGain)
        halGain = maxHalGain;

#if 0
    halInttime = sensorInttime * aecCfg.PixelPeriodsPerLine /
        (aecCfg.PixelClockFreqMHZ * 1000000);
#else
    float timeC0 = aecCfg.TimeFactor[0];
    float timeC1 = aecCfg.TimeFactor[1];
    float timeC2 = aecCfg.TimeFactor[2];
    float timeC3 = aecCfg.TimeFactor[3];

    halInttime = ((sensorInttime - timeC0 * aecCfg.LinePeriodsPerField - timeC1) / timeC2 - timeC3) *
        aecCfg.PixelPeriodsPerLine / (aecCfg.PixelClockFreqMHZ * 1000000);

    if(revert_gain_array != NULL) {
        free(revert_gain_array);
        revert_gain_array = NULL;
    }

#endif

}
void CamIA10Engine::mapHalExpToSensor
(
 float hal_gain,
 float hal_time,
 int& sensor_gain,
 int& sensor_time
 ) {

    float gainRange[] = {
        1,  1.9375, 16,  16, 1, 0, 15,
        2,  3.875,  8,  0, 1, 16, 31,
        4,  7.75,   4,  -32, 1, 48, 63,
        8,  15.5,   2,  -96, 1, 112, 127
    };

    float* pgainrange;
    uint32_t size = 0;

    if (aecCfg.pGainRange == NULL || aecCfg.GainRange_size <= 0) {
        pgainrange = gainRange;
        size = sizeof(gainRange) / sizeof(float);
    } else {
        pgainrange = aecCfg.pGainRange;
        size = aecCfg.GainRange_size;
    }

    // AG = (((C1 * analog gain - C0) / M0) + 0.5f
    float C1 = -1, C0, M0, minReg, maxReg;
    float ag = hal_gain;
    unsigned int i;

    for (i=0; i<size; i+=7) {
        if (ag >= pgainrange[i] && ag <= pgainrange[i+1]) {
            C1 = pgainrange[i+2];
            C0 = pgainrange[i+3];
            M0 = pgainrange[i+4];
            minReg = pgainrange[i+5];
            maxReg = pgainrange[i+6];
            break;
        }
    }

    if (C1 == -1) {
        ALOGE("GAIN OUT OF RANGE: lasttime-gain: %f-%f", hal_time, hal_gain);
        C1 = 16;
        C0 = 0;
        M0 = 1;
        minReg = 16;
        maxReg = 255;
    }

    sensor_gain = (int)((C1 * hal_gain - C0) / M0 + 0.5f);
    if (sensor_gain < minReg)
        sensor_gain = minReg;
    if (sensor_gain > maxReg)
        sensor_gain = maxReg;

#if 0
    sensor_time = (int)(hal_time * aecCfg.PixelClockFreqMHZ * 1000000 /
                        aecCfg.PixelPeriodsPerLine + 0.5);
#else
    float timeC0 = aecCfg.TimeFactor[0];
    float timeC1 = aecCfg.TimeFactor[1];
    float timeC2 = aecCfg.TimeFactor[2];
    float timeC3 = aecCfg.TimeFactor[3];
    sensor_time = (int)(timeC0 * aecCfg.LinePeriodsPerField + timeC1 +
                        timeC2 * ((hal_time * aecCfg.PixelClockFreqMHZ * 1000000 / aecCfg.PixelPeriodsPerLine) + timeC3));
#endif

}

RESULT CamIA10Engine::initAEC() {
    RESULT ret = RET_FAILURE;

    CamCalibAecGlobal_t* pAecGlobal;
    ret = CamCalibDbGetAecGlobal(hCamCalibDb, &pAecGlobal);
    if (ret != RET_SUCCESS) {
        LOGD("fail to get pAecGlobal, ret: %d", ret);
        return ret;
    }

    aecCfg.SetPoint       = pAecGlobal->SetPoint;
    aecCfg.ClmTolerance   = pAecGlobal->ClmTolerance;
    aecCfg.DampOverStill  = pAecGlobal->DampOverStill;
    aecCfg.DampUnderStill = pAecGlobal->DampUnderStill;
    aecCfg.DampOverVideo  = pAecGlobal->DampOverVideo;
    aecCfg.DampUnderVideo = pAecGlobal->DampUnderVideo;
    aecCfg.DampingMode    = AEC_DAMPING_MODE_STILL_IMAGE;
    aecCfg.SemMode        = AEC_SCENE_EVALUATION_DISABLED;
    aecCfg.AOE_Enable     = pAecGlobal->AOE_Enable;
    aecCfg.AOE_Max_point  = pAecGlobal->AOE_Max_point;
    aecCfg.AOE_Min_point  = pAecGlobal->AOE_Min_point;
    aecCfg.AOE_Y_Max_th   = pAecGlobal->AOE_Y_Max_th;
    aecCfg.AOE_Y_Min_th   = pAecGlobal->AOE_Y_Min_th;
    aecCfg.AOE_Step_Dec   = pAecGlobal->AOE_Step_Dec;
    aecCfg.AOE_Step_Inc   = pAecGlobal->AOE_Step_Inc;
    aecCfg.DON_Night_Trigger        = pAecGlobal->DON_Night_Trigger;
    aecCfg.DON_Night_Mode           = pAecGlobal->DON_Night_Mode;
    aecCfg.DON_Day2Night_Fac_th     = pAecGlobal->DON_Day2Night_Fac_th;
    aecCfg.DON_Night2Day_Fac_th     = pAecGlobal->DON_Night2Day_Fac_th;
    aecCfg.DON_Bouncing_th          = pAecGlobal->DON_Bouncing_th;
    aecCfg.meas_mode    = (AecMeasuringMode_t)(pAecGlobal->CamerIcIspExpMeasuringMode);
    aecCfg.FpsSetEnable = pAecGlobal->FpsSetEnable;
    aecCfg.isFpsFix = pAecGlobal->isFpsFix;
    aecCfg.GainRange_size = pAecGlobal->GainRange.array_size;
    aecCfg.pGainRange = pAecGlobal->GainRange.pGainRange;
    aecCfg.IntervalAdjStgy.enable = pAecGlobal->InterAdjustStrategy.enable;
    aecCfg.IntervalAdjStgy.dluma_high_th = pAecGlobal->InterAdjustStrategy.dluma_high_th;
    aecCfg.IntervalAdjStgy.dluma_low_th = pAecGlobal->InterAdjustStrategy.dluma_low_th;
    aecCfg.IntervalAdjStgy.trigger_frame = pAecGlobal->InterAdjustStrategy.trigger_frame;
    memcpy (&aecCfg.HdrCtrl,&pAecGlobal->HdrCtrl,sizeof(pAecGlobal->HdrCtrl));//zlj
    if (mIspVer > 0) {
        aecCfg.Valid_GridWeights_Num = 81;
        aecCfg.Valid_GridWeights_W = 9;
        aecCfg.Valid_HistBins_Num = 32;
    } else {
        aecCfg.Valid_GridWeights_Num = 25;
        aecCfg.Valid_GridWeights_W = 5;
        aecCfg.Valid_HistBins_Num = 16;
    }
    //zlj add for 1608 HDR stats
    if (mSensorEntityName && strstr(mSensorEntityName, "1608") &&
        aecCfg.HdrCtrl.Enable) {
        LOGD("sensor is attached to rk1608, use HDR ae !");
        aecCfg.Valid_HdrGridWeights_Num = 225;
        aecCfg.Valid_HdrGridWeights_W = 15;
        aecCfg.Valid_HdrHistBins_Num = 256;
        aecCfg.IsHdrAeMode = true;
    }
    aecCfg.FpsSetEnable = true;
    aecCfg.isFpsFix = false;
    memcpy(aecCfg.TimeFactor, pAecGlobal->TimeFactor, sizeof(pAecGlobal->TimeFactor));
    char* pweight = (char*)pAecGlobal->GridWeights.pWeight;
    LOGD("weights before transform...");
    for ( int i = 0; i < 81; i+=9 )
        LOGD("%02d -> %02d: %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d", i, i+8,
             pweight[i], pweight[i+1],pweight[i+2],pweight[i+3],pweight[i+4],
             pweight[i+5],pweight[i+6],pweight[i+7],pweight[i+8]);
    if (mIspVer > 0)
        memcpy(aecCfg.GridWeights.uCoeff, pAecGlobal->GridWeights.pWeight, pAecGlobal->GridWeights.ArraySize);
    else {
        cam_ia10_isp_map_hstw_9x9_to_5x5(pAecGlobal->GridWeights.pWeight, aecCfg.GridWeights.uCoeff);
        pweight = (char*)aecCfg.GridWeights.uCoeff;
        LOGD("weights after transform...");
        for ( int i = 0; i < 25; i+=5 )
            LOGD("%02d -> %02d: %02d, %02d, %02d, %02d, %02d", i, i+4,
                 pweight[i], pweight[i+1],pweight[i+2],pweight[i+3],pweight[i+4]);
    }

    memcpy(aecCfg.FpsFixTimeDot.fCoeff, pAecGlobal->FpsFixTimeDot.fCoeff, sizeof(pAecGlobal->FpsFixTimeDot.fCoeff));

    aecCfg.StepSize = 0;
    aecCfg.HistMode = (CamerIcIspHistMode_t)(pAecGlobal->CamerIcIspHistMode);

	//oyyf
	if( mLightMode <= LIGHT_MODE_MIN || mLightMode >= LIGHT_MODE_MAX){
  		mLightMode= LIGHT_MODE_DAY;
  	}


	int no_DySetpoint = 0;
	ret = CamCalibDbGetNoOfDySetpoint(hCamCalibDb, pAecGlobal, &no_DySetpoint);
	if (ret != RET_SUCCESS) {
		ALOGD("%s: Getting number of DySetpoint profile from calib database failed (%d)\n",
			 __FUNCTION__, ret);
		return (ret);
	}

	if(no_DySetpoint){
		for(int i=0; i<no_DySetpoint && i<LIGHT_MODE_MAX ; i++){
		   CamCalibAecDynamicSetpoint_t* pDySetpointProfile = NULL;
		   ret = CamCalibDbGetDySetpointByIdx(hCamCalibDb, pAecGlobal, i, &pDySetpointProfile);
		   if (ret != RET_SUCCESS) {
			 ALOGD("%s: Getting idx(%d) DySetpoint profile from calib database failed (%d)\n",
				   __FUNCTION__, i, ret);
			 return (ret);
		   }
		   DCT_ASSERT(NULL != pDySetpointProfile);
		   aecCfg.pDySetpoint[i] = pDySetpointProfile;
		}
	}

	int no_ExpSeparate = 0;
    ret = CamCalibDbGetNoOfExpSeparate(hCamCalibDb, pAecGlobal, &no_ExpSeparate);
    if (ret != RET_SUCCESS) {
      ALOGD("%s: Getting number of DySetpoint profile from calib database failed (%d)\n",
            __FUNCTION__, ret);
      return (ret);
    }

    if(no_ExpSeparate){
    	for(int i=0; i<no_ExpSeparate && i<LIGHT_MODE_MAX ; i++){
    		CamCalibAecExpSeparate_t* pExpSeparateProfile = NULL;
    		ret = CamCalibDbGetExpSeparateByIdx(hCamCalibDb, pAecGlobal, i, &pExpSeparateProfile);
    		if (ret != RET_SUCCESS) {
    	      ALOGD("%s: Getting idx(%d) ExpSeparate profile from calib database failed (%d)\n",
    	            __FUNCTION__, i, ret);
    	      return (ret);
    	    }
    	    DCT_ASSERT(NULL != pExpSeparateProfile);
    		aecCfg.pExpSeparate[i] = pExpSeparateProfile;
    	}

		if(mLightMode > no_ExpSeparate - 1)
			mLightMode = LIGHT_MODE_DAY;

		memcpy(aecCfg.EcmTimeDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmTimeDot.fCoeff, sizeof(aecCfg.EcmTimeDot.fCoeff));
    	memcpy(aecCfg.EcmGainDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmGainDot.fCoeff, sizeof(aecCfg.EcmGainDot.fCoeff));
    	memcpy(aecCfg.EcmLTimeDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmLTimeDot.fCoeff, sizeof(aecCfg.EcmLTimeDot.fCoeff));//zlj
    	memcpy(aecCfg.EcmLGainDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmLGainDot.fCoeff, sizeof(aecCfg.EcmLGainDot.fCoeff));//zlj
    	memcpy(aecCfg.EcmSTimeDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmSTimeDot.fCoeff, sizeof(aecCfg.EcmSTimeDot.fCoeff));//zlj
    	memcpy(aecCfg.EcmSGainDot.fCoeff, aecCfg.pExpSeparate[mLightMode]->ecmSGainDot.fCoeff, sizeof(aecCfg.EcmSGainDot.fCoeff));//zlj
    }

    aecCfg.LightMode = mLightMode;

    if (aecDesc != NULL) {
        XCamAeParam aeParam;
        aeParam.mode  = XCAM_AE_MODE_AUTO;
        LOGD("aecCfg histmode: %d\n", aecCfg.HistMode);
        ret = aecDesc->update_ae_params(aecContext, &aecCfg);
    } //AecInit(&aecCfg);

    return ret;
}

RESULT CamIA10Engine::runAEC(HAL_AecCfg* config) {
    RESULT ret = RET_SUCCESS;

    int lastTime = lastAecResult.regIntegrationTime;
    int lastGain = lastAecResult.regGain;

#if 0
    LOGD("   ");
    if (!mInitDynamic) {
        ALOGI("cccccc check type (%d) runAEC - check exp time=[%d-%d], sensor=[%d-%d]\n",
              mStats.meas_type,
              lastTime, mStats.sensor_mode.exp_time,
              lastGain, mStats.sensor_mode.gain
             );
    } else {
        ALOGI("cccccc check type (%d) runAEC - check exp time=[%d-%d], sensor=[%d-%d]\n",
              mStats.meas_type,
              lastTime, dCfg.sensor_mode.exp_time,
              lastGain, dCfg.sensor_mode.gain
             );
    }

    unsigned char* expmean = mStats.aec.exp_mean;
    for (int i=0; i<25; i+=5) {
        ALOGI("--runAEC-EXPO=[%d-%d-%d-%d-%d]",
              expmean[i], expmean[i+1], expmean[i+2], expmean[i+3], expmean[i+4]);
    }

    unsigned int* hist = mStats.aec.hist_bins;
    for (int i=0; i<16; i+=4) {
        ALOGI("--runAEC-hist=[%d-%d-%d-%d]",
              hist[i], hist[i+1], hist[i+2], hist[i+3]);
    }
#endif

    if (!mInitDynamic) {
        //start dynamic config
        struct HAL_AecCfg* set, *shd;
        mAECHalCfg = *config;
        set = &mAECHalCfg;
        shd = &mAECHalCfgShd;

        if ((set->win.left_hoff != shd->win.left_hoff) ||
            (set->win.top_voff != shd->win.top_voff) ||
            (set->win.right_width != shd->win.right_width) ||
            (set->win.bottom_height != shd->win.bottom_height) ||
            (set->meter_mode != shd->meter_mode) ||
            (set->mode != shd->mode) ||
            (set->flk != shd->flk) ||
            (set->ae_bias != shd->ae_bias) ||
            (set->frame_time_ms_min != shd->frame_time_ms_min) /*||
                                                                 mLightMode != cfg->LightMode*/) {
            //cifisp_histogram_mode mode = CIFISP_HISTOGRAM_MODE_RGB_COMBINED;
            cam_ia10_isp_hst_update_stepSize(
                                             aecCfg.HistMode,
                                             aecCfg.GridWeights.uCoeff,
                                             mStats.sensor_mode.isp_input_width,
                                             mStats.sensor_mode.isp_input_height,
                                             mIspVer,
                                             &(aecCfg.StepSize));

            //LOGD("aec set win:%dx%d",
            //  set->win.right_width,set->win.bottom_height);

            aecCfg.LinePeriodsPerField =
                mStats.sensor_mode.line_periods_per_field == 0 ?
                2228: mStats.sensor_mode.line_periods_per_field;

            aecCfg.PixelClockFreqMHZ =
                mStats.sensor_mode.pixel_clock_freq_mhz == 0 ?
                180 : mStats.sensor_mode.pixel_clock_freq_mhz;
            aecCfg.PixelPeriodsPerLine =
                mStats.sensor_mode.pixel_periods_per_line == 0 ?
                2688 : mStats.sensor_mode.pixel_periods_per_line;

            if (set->flk == HAL_AE_FLK_OFF)
                aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_OFF;
            else if (set->flk == HAL_AE_FLK_50)
                aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_100HZ;
            else if (set->flk == HAL_AE_FLK_60)
                aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_120HZ;
            else
                aecCfg.EcmFlickerSelect = AEC_EXPOSURE_CONVERSION_FLICKER_100HZ;

            if (set->meter_mode == HAL_AE_METERING_MODE_CENTER) {
#if 0
                unsigned char gridWeights[25];
                //cifisp_histogram_mode mode = CIFISP_HISTOGRAM_MODE_RGB_COMBINED;

                gridWeights[0] = 0x00;	  //weight_00to40
                gridWeights[1] = 0x00;
                gridWeights[2] = 0x01;
                gridWeights[3] = 0x00;
                gridWeights[4] = 0x00;

                gridWeights[5] = 0x00;	  //weight_01to41
                gridWeights[6] = 0x02;
                gridWeights[7] = 0x02;
                gridWeights[8] = 0x02;
                gridWeights[9] = 0x00;

                gridWeights[10] = 0x00;    //weight_02to42
                gridWeights[11] = 0x04;
                gridWeights[12] = 0x08;
                gridWeights[13] = 0x04;
                gridWeights[14] = 0x00;

                gridWeights[15] = 0x00;    //weight_03to43
                gridWeights[16] = 0x02;
                gridWeights[17] = 0x00;
                gridWeights[18] = 0x02;
                gridWeights[19] = 0x00;

                gridWeights[20] = 0x00;    //weight_04to44
                gridWeights[21] = 0x00;
                gridWeights[22] = 0x00;
                gridWeights[23] = 0x00;
                gridWeights[24] = 0x00;
                memcpy(aecCfg.GridWeights.uCoeff, gridWeights, sizeof(gridWeights));
#else
                //do nothing ,just use the xml setting
#endif
            } else if (set->meter_mode == HAL_AE_METERING_MODE_AVERAGE) {
                memset(aecCfg.GridWeights.uCoeff, 0x01, sizeof(aecCfg.GridWeights.uCoeff));
            } else
                ALOGE("%s:not support %d metering mode!", __func__, set->meter_mode);

            //set ae bias
            {
                CamCalibAecGlobal_t* pAecGlobal;
                CamCalibDbGetAecGlobal(hCamCalibDb, &pAecGlobal);
                float SetPoint = pAecGlobal->SetPoint ;
                /*
                   if(cfg->LightMode == LIGHT_MODE_NIGHT && pAecGlobal->NightSetPoint != 0)
                   SetPoint = pAecGlobal->NightSetPoint;

                   aecCfg.SetPoint = (SetPoint) +
                   (set->ae_bias / 100.0f) * MAX(10, SetPoint / (1 - pAecGlobal->ClmTolerance / 100.0f) / 10.0f) ;

                   mLightMode = cfg->LightMode;
                   */

                //if (set->frame_time_us_min != -1 && set->frame_time_us_max != -1)
                {
                    if (set->frame_time_ms_min != -1 && set->frame_time_ms_max != -1) {
                        aecCfg.FpsSetEnable = true;
                        aecCfg.isFpsFix = true;
                        int ecmCnt = sizeof(aecCfg.EcmTimeDot.fCoeff) / sizeof (float);

                        for (int i = 1; i < ecmCnt - 3; i++) {
                            aecCfg.EcmTimeDot.fCoeff[i] = (float)set->frame_time_ms_min / 1000;
                        }
                        for (int i = ecmCnt - 3; i < ecmCnt; i++) {
                            aecCfg.EcmTimeDot.fCoeff[i] = (float)set->frame_time_ms_max / 1000;
                        }
                        LOGD("sensor param (%d)=[%f-%f-%f-%f-%f-%f] vts: %f, vtsMax: %d, pclk: %f, hts: %f\n",
                             ecmCnt,
                             aecCfg.EcmTimeDot.fCoeff[0],
                             aecCfg.EcmTimeDot.fCoeff[1],
                             aecCfg.EcmTimeDot.fCoeff[2],
                             aecCfg.EcmTimeDot.fCoeff[3],
                             aecCfg.EcmTimeDot.fCoeff[4],
                             aecCfg.EcmTimeDot.fCoeff[5],
                             aecCfg.LinePeriodsPerField,
                             mStats.sensor_mode.line_periods_per_field,
                             aecCfg.PixelClockFreqMHZ,
                             aecCfg.PixelPeriodsPerLine);
                    }
                }
            }

            //AecStop();
            if ((set->mode != HAL_AE_OPERATION_MODE_MANUAL)/* &&
                                                              !(cfg->aaa_locks & HAL_3A_LOCKS_EXPOSURE)*/) {
                if (aecDesc != NULL) {
                    aecDesc->update_ae_params(aecContext, &aecCfg);
                } //AecUpdateConfig(&aecCfg);
                //AecStart();
                //get init result
                if (aecDesc != NULL) {
                    aecDesc->set_stats(aecContext, &mStats.aec);

                    XCamAeParam aeParam;
                    aeParam.mode  = XCAM_AE_MODE_AUTO;
                    aecDesc->analyze_ae(aecContext, &aeParam);
                } //ret = AecRun(NULL, NULL);

            }

            *shd = *set;
        }
        if (!mStatisticsUpdated) {
            LOGD("------------INITIALIZE  STATIC NOT READY");
            return ret;
        }
        //end of dynamic config

        if ((mStats.meas_type & (CAMIA10_AEC_MASK | CAMIA10_HST_MASK)) == 0) {
            return ret;
        }

        if ((lastTime == -1 && lastGain == -1) ||
            (lastTime == mStats.sensor_mode.exp_time && lastGain == mStats.sensor_mode.gain)) {
            if (aecDesc != NULL) {
                aecDesc->set_stats(aecContext, &mStats.aec);

                XCamAeParam aeParam;
                aeParam.mode  = XCAM_AE_MODE_AUTO;
                aecDesc->analyze_ae(aecContext, &aeParam);
            }
        }

    } else {
        if ((lastTime == -1 && lastGain == -1)
            || (lastTime == dCfg.sensor_mode.exp_time && lastGain == dCfg.sensor_mode.gain)) {
            if (aecDesc != NULL) {
                aecDesc->set_stats(aecContext, &mStats.aec);

                XCamAeParam aeParam;
                aeParam.mode  = XCAM_AE_MODE_AUTO;
                aecDesc->analyze_ae(aecContext, &aeParam);
            } //AecRun(&mStats.aec, NULL);

        }
    }

    return ret;
}

RESULT CamIA10Engine::getAECResults(AecResult_t* result) {
    struct HAL_AecCfg* set, *shd;

    if (mInitDynamic) {
        set = &dCfg.aec_cfg;
        shd = &dCfgShd.aec_cfg;
    } else {
        set = &mAECHalCfg;
        shd = &mAECHalCfgShd;
    }

    /*if ((set->win.h_offs != shd->win.h_offs) ||
      (set->win.v_offs != shd->win.v_offs) ||
      (set->win.h_size != shd->win.h_size) ||
      (set->win.v_size != shd->win.v_size)) {*/
    //if (true) {
    if (aecDesc != NULL) {
        aecDesc->get_results(aecContext, result);
    } //AecGetResults(result);

    // TODO: need this now, getAECResults will be called twice in one 3a loop,
    // should return the same result each time in one loop
    result->actives |= CAMIA10_AEC_MASK;
    if(!result->IsHdrExp){
        if (lastAecResult.coarse_integration_time != result->coarse_integration_time
            || lastAecResult.analog_gain_code_global != result->analog_gain_code_global
            ||lastAecResult.regIntegrationTime != result->regIntegrationTime
            || lastAecResult.regGain != result->regGain) {
            result->actives |= CAMIA10_AEC_MASK;
            lastAecResult.coarse_integration_time = result->coarse_integration_time;
            lastAecResult.analog_gain_code_global = result->analog_gain_code_global;
            lastAecResult.regIntegrationTime = result->regIntegrationTime;
            lastAecResult.regGain = result->regGain;
            lastAecResult.gainFactor = result->gainFactor;
            lastAecResult.gainBias = result->gainBias;
            lastAecResult.IsHdrExp=result->IsHdrExp;
            memcpy(lastAecResult.exp_smooth_results,
                   result->exp_smooth_results,
                   sizeof(result->exp_smooth_results));
            //*shd = *set;
        }
    }else{
        if ((lastAecResult.RegHdrGains[0]!=result->RegHdrGains[0])
            || (lastAecResult.RegHdrTime[0]!=result->RegHdrTime[0])
            || (lastAecResult.RegHdrGains[1]!=result->RegHdrGains[1])
            || (lastAecResult.RegHdrTime[1]!=result->RegHdrTime[1])
            || (lastAecResult.RegHdrGains[2]!=result->RegHdrGains[2])
            || (lastAecResult.RegHdrTime[2]!=result->RegHdrTime[2])) {
            result->actives |= CAMIA10_AEC_MASK;
            lastAecResult.coarse_integration_time = result->coarse_integration_time;
            lastAecResult.analog_gain_code_global = result->analog_gain_code_global;
            lastAecResult.regIntegrationTime = result->regIntegrationTime;
            lastAecResult.regGain = result->regGain;
            lastAecResult.gainFactor = result->gainFactor;
            lastAecResult.gainBias = result->gainBias;
            lastAecResult.RegHdrGains[0]=result->RegHdrGains[0];
            lastAecResult.RegHdrGains[1]=result->RegHdrGains[1];
            lastAecResult.RegHdrGains[2]=result->RegHdrGains[2];
            lastAecResult.RegHdrTime[0]=result->RegHdrTime[0];
            lastAecResult.RegHdrTime[1]=result->RegHdrTime[1];
            lastAecResult.RegHdrTime[2]=result->RegHdrTime[2];
            lastAecResult.IsHdrExp=result->IsHdrExp;
            lastAecResult.DCG_Ratio=result->DCG_Ratio;
            //*shd = *set;
        }
    }

    result->actives |= CAMIA10_HST_MASK;
    result->meas_mode = aecCfg.meas_mode;
    result->meas_win.h_offs = set->win.left_hoff;
    result->meas_win.v_offs = set->win.top_voff;
    result->meas_win.h_size = set->win.right_width;
    result->meas_win.v_size = set->win.bottom_height;

    if(lastAecResult.auto_adjust_fps != result->auto_adjust_fps || (result->actives & AEC_AFPS_MASK) ) {
        lastAecResult.auto_adjust_fps = result->auto_adjust_fps;
        result->actives |= CAMIA10_AEC_AFPS_MASK;
    }

    //LOGD("set offset: %d-%d, size: %d-%d", set->win.left_hoff, set->win.top_voff, set->win.right_width, set->win.bottom_height);
    //LOGD("ret offset: %d-%d, size: %d-%d", result->meas_win.h_offs, result->meas_win.v_offs, result->meas_win.h_size, result->meas_win.v_size);
    //LOGD("sensor_mode size: %d-%d", dCfg.sensor_mode.isp_input_width, dCfg.sensor_mode.isp_input_height);
    //ALOGI("AEC time-gain=[%f-%f], regtime-gain=[%d-%d]",result->coarse_integration_time, result->analog_gain_code_global,result->regIntegrationTime ,result->regGain);

    return RET_SUCCESS;
}

void CamIA10Engine::convertAwbResult2Cameric
(
 AwbRunningOutputResult_t* awbResult,
 CamIA10_AWB_Result_t* awbCamicResult
 ) {
    if (!awbResult || !awbCamicResult)
        return;
    awbCamicResult->actives = awbResult->validParam;
    HalAwbGains2CamerIcGains(
                             &awbResult->WbGains,
                             &awbCamicResult->awbGains
                            );

    HalAwbXtalk2CamerIcXtalk
        (
         &awbResult->CcMatrix,
         &awbCamicResult->CcMatrix
        );

    HalAwbXTalkOffset2CamerIcXTalkOffset
        (
         &awbResult->CcOffset,
         &awbCamicResult->CcOffset
        );
    awbCamicResult->LscMatrixTable = awbResult->LscMatrixTable;
    awbCamicResult->SectorConfig   = awbResult->SectorConfig;
    awbCamicResult->MeasMode    =  awbResult->MeasMode;
    awbCamicResult->MeasConfig    =  awbResult->MeasConfig;
    awbCamicResult->awbWin      =  awbResult->awbWin;
    awbCamicResult->DoorType    =  awbResult->DoorType;
    awbCamicResult->converged =  awbResult->converged;
    awbCamicResult->GainsAlgo =  awbResult->WbGains;
    memcpy(awbCamicResult->CtMatrixAlgo.fCoeff, awbResult->CcMatrix.fCoeff,
           sizeof(awbResult->CcMatrix.fCoeff));
    // order: r,g,b
    memcpy(&awbCamicResult->CtOffsetAlgo, awbResult->CcOffset.fCoeff,
           sizeof(awbResult->CcOffset.fCoeff));
}

void CamIA10Engine::updateAwbResults
(
 CamIA10_AWB_Result_t* old,
 CamIA10_AWB_Result_t* newCfg,
 CamIA10_AWB_Result_t* update
 ) {
    if (!old || !newCfg || !update)
        return;
    /*
       newCfg->awbGains.Red = 400;
       newCfg->awbGains.GreenB = 200;
       newCfg->awbGains.GreenR = 200;
       newCfg->awbGains.Blue = 250;
       */
    update->GainsAlgo = newCfg->GainsAlgo;
    update->CtMatrixAlgo = newCfg->CtMatrixAlgo;
    update->CtOffsetAlgo = newCfg->CtOffsetAlgo;

    if (newCfg->actives & AWB_RECONFIG_GAINS) {
        if ((newCfg->awbGains.Blue != old->awbGains.Blue)
            || (newCfg->awbGains.Red != old->awbGains.Red)
            || (newCfg->awbGains.GreenR != old->awbGains.GreenR)
            || (newCfg->awbGains.GreenB != old->awbGains.GreenB)) {
            update->actives |= AWB_RECONFIG_GAINS;
            update->awbGains = newCfg->awbGains;
        } else {
            update->actives |= AWB_RECONFIG_GAINS;
            update->awbGains = newCfg->awbGains;
        }
    }

    //LOGD("AWBRET=[%d-%d-%d-%d]", newCfg->awbGains.Red, newCfg->awbGains.GreenR, newCfg->awbGains.GreenB, newCfg->awbGains.Blue);

    //TODO:AWB_RECONFIG_CCMATRIX & AWB_RECONFIG_CCOFFSET should
    //update concurrently,now alogorithm ensure this
    if (newCfg->actives & AWB_RECONFIG_CCMATRIX) {
        int i = 0;

        for (; ((i < 9) &&
                (newCfg->CcMatrix.Coeff[i] == old->CcMatrix.Coeff[i]))
             ; i++);
        if (i != 9) {
            update->actives |= AWB_RECONFIG_CCMATRIX;
            update->CcMatrix = newCfg->CcMatrix;
        } else
            update->CcMatrix = newCfg->CcMatrix;
    } else
        update->CcMatrix = newCfg->CcMatrix;

    if (newCfg->actives & AWB_RECONFIG_CCOFFSET) {
        if ((newCfg->CcOffset.Blue) != (old->CcOffset.Blue)
            || (newCfg->CcOffset.Red) != (old->CcOffset.Red)
            || (newCfg->CcOffset.Green) != (old->CcOffset.Green)) {
            update->actives |= AWB_RECONFIG_CCOFFSET;
            update->CcOffset = newCfg->CcOffset;
        } else
            update->CcOffset = newCfg->CcOffset;
    } else
        update->CcOffset = newCfg->CcOffset;

    //TODO:AWB_RECONFIG_LSCMATRIX & AWB_RECONFIG_LSCSECTOR should
    //update concurrently,now alogorithm ensure this
    if (newCfg->actives & AWB_RECONFIG_LSCMATRIX) {
        update->actives |= AWB_RECONFIG_LSCMATRIX;
        update->LscMatrixTable = newCfg->LscMatrixTable;
    } else
        update->LscMatrixTable = newCfg->LscMatrixTable;

    if (newCfg->actives & AWB_RECONFIG_LSCSECTOR) {
        update->actives |= AWB_RECONFIG_LSCSECTOR;
        update->SectorConfig = newCfg->SectorConfig;
    } else
        update->SectorConfig = newCfg->SectorConfig;

    //TODO:AWB_RECONFIG_MEASMODE & AWB_RECONFIG_MEASCFG & AWB_RECONFIG_AWBWIN
    //should update concurrently,now alogorithm ensure this
    if (newCfg->actives & AWB_RECONFIG_MEASMODE) {
        update->actives |= AWB_RECONFIG_MEASMODE;
        update->MeasMode = newCfg->MeasMode;
    } else
        update->MeasMode = newCfg->MeasMode;

    if (newCfg->actives & AWB_RECONFIG_MEASCFG) {
        update->actives |= AWB_RECONFIG_MEASCFG;
        update->MeasConfig = newCfg->MeasConfig;
    } else
        update->MeasConfig = newCfg->MeasConfig;

    if (newCfg->actives & AWB_RECONFIG_AWBWIN) {
        update->actives |= AWB_RECONFIG_AWBWIN;
        update->awbWin = newCfg->awbWin;
    } else
        update->awbWin = newCfg->awbWin;
    /* fixed */
    update->DoorType = newCfg->DoorType;
    update->converged = newCfg->converged;
    //LOGD("%s:%d,update awb config actives %d \n",__func__,__LINE__,update->actives );
}

RESULT CamIA10Engine::runAWB(HAL_AwbCfg* awbHalCfg) {
    RESULT result = RET_SUCCESS;
    //convert statics to awb algorithm
    AwbRunningInputParams_t MeasResult;
    AwbRunningOutputResult_t retOuput;

    memset(&MeasResult, 0, sizeof(AwbRunningInputParams_t));
    memset(&retOuput, 0, sizeof(AwbRunningOutputResult_t));
    //start dynamic config
    if (!mInitDynamic) {
        mAWBHalCfg = *awbHalCfg;
        if (!hAwb) {
            AwbInstanceConfig_t awbInstance;
            awbInstance.isp_ver = mIspVer;
            if (awbDesc) {
                result = awbDesc->update_awb_params(awbContext, &awbInstance);
            } //result = AwbInit(&awbInstance);

            if (awbHalCfg->mode != HAL_WB_AUTO) {
                char prfName[10];
                int i, no;
                CamAwb_V10_IlluProfile_t* pIlluProfile = NULL;
                awbcfg.Mode = AWB_MODE_MANUAL;
                if (awbHalCfg->mode == HAL_WB_INCANDESCENT) {
                    strcpy(prfName, "A");
                } else if (awbHalCfg->mode == HAL_WB_DAYLIGHT) {
                    strcpy(prfName, "D65");
                } else if (awbHalCfg->mode == HAL_WB_FLUORESCENT) {
                    strcpy(prfName, "F11_TL84");
                } else if (awbHalCfg->mode == HAL_WB_SUNSET) {
                    strcpy(prfName, "HORIZON"); //not support now
                } else if (awbHalCfg->mode == HAL_WB_CLOUDY_DAYLIGHT) {
                    strcpy(prfName, "F2_CWF");
                } else if (awbHalCfg->mode == HAL_WB_CANDLE) {
                    strcpy(prfName, "U30"); //not support now
                } else
                    LOGE("%s:not support this awb mode %d !", __func__, awbHalCfg->mode);

                // get number of availabe illumination profiles from database
                result = CamCalibDbGetNoOfAwb_V10_Illuminations(hCamCalibDb, &no);
                // run over all illumination profiles
                for (i = 0; i < no; i++) {
                    // get an illumination profile from database
                    result = CamCalibDbGetAwb_V10_IlluminationByIdx(hCamCalibDb, i, &pIlluProfile);
                    if (strstr(pIlluProfile->name, prfName)) {
                        awbcfg.idx = i;
                        break;
                    }
                }

                if (i == no)
                    LOGE("%s:can't find %s profile!", __func__, prfName);
            }
            if (result != RET_SUCCESS)
                return result;
            else
                hAwb = awbInstance.hAwb;

            awbcfg.width = mStats.sensor_mode.isp_input_width;
            awbcfg.height = mStats.sensor_mode.isp_input_height;
            awbcfg.awbWin.h_offs = 0;
            awbcfg.awbWin.v_offs = 0;
            awbcfg.awbWin.h_size = mStats.sensor_mode.isp_input_width;//HAL_WIN_REF_WIDTH;
            awbcfg.awbWin.v_size = mStats.sensor_mode.isp_input_height;//HAL_WIN_REF_HEIGHT;
            //awbcfg.awbWin.h_size = HAL_WIN_REF_WIDTH;
            //awbcfg.awbWin.v_size = HAL_WIN_REF_HEIGHT;

            if (awbDesc) {
                result = awbDesc->update_awb_params(awbContext, &awbcfg);
            } //result = AwbConfigure(hAwb, &awbcfg);

            if (result != RET_SUCCESS) {
                LOGE("%s:awb config failure!", __func__);
                hAwb = NULL;
                goto updateAwbConfig_end;

            }

            AwbRunningOutputResult_t outresult;
            memset(&outresult, 0, sizeof(AwbRunningOutputResult_t));
            if (awbDesc) {
                XCamAwbParam param;
                param.mode = XCAM_AWB_MODE_AUTO;
                //result = awbDesc->set_stats(awbContext, NULL);
                result = awbDesc->analyze_awb(awbContext, &param);
            } //result =  AwbRun(hAwb, NULL, &outresult);

            if (result == RET_SUCCESS) {
                //convert result
                memset(&curAwbResult, 0, sizeof(curAwbResult));
                convertAwbResult2Cameric(&outresult, &curAwbResult);
            } else {
                LOGE("%s:awb run failure!", __func__);
                hAwb = NULL;
                goto updateAwbConfig_end;
            }
        } else {
            if (awbHalCfg->win.right_width && awbHalCfg->win.bottom_height) {
                awbcfg.awbWin.h_offs = awbHalCfg->win.left_hoff;
                awbcfg.awbWin.v_offs = awbHalCfg->win.top_voff;
                awbcfg.awbWin.h_size = awbHalCfg->win.right_width;
                awbcfg.awbWin.v_size = awbHalCfg->win.bottom_height;
            } else {
                awbcfg.awbWin.h_offs = 0;
                awbcfg.awbWin.v_offs = 0;
                awbcfg.awbWin.h_size = mStats.sensor_mode.isp_input_width;//HAL_WIN_REF_WIDTH;
                awbcfg.awbWin.v_size = mStats.sensor_mode.isp_input_height;//HAL_WIN_REF_HEIGHT;
            }
            //mode change ?
            if (awbHalCfg->mode != mAWBHalCfg.mode) {
                memset(&lastAwbResult, 0x00, sizeof(lastAwbResult));
                if (awbHalCfg->mode != HAL_WB_AUTO) {
                    char prfName[10];
                    int i, no;
                    CamAwb_V10_IlluProfile_t* pIlluProfile = NULL;
                    awbcfg.Mode = AWB_MODE_MANUAL;
                    //get index
                    //A:3400k
                    //D65:6500K artificial daylight
                    //CWF:4150K cool whtie fluorescent
                    //TL84: 4000K
                    //D50: 5000k
                    //D75:7500K
                    //HORIZON:2300K
                    //SNOW :6800k
                    //CANDLE:1850K
                    if (awbHalCfg->mode == HAL_WB_INCANDESCENT) {
                        strcpy(prfName, "A");
                    } else if (awbHalCfg->mode == HAL_WB_DAYLIGHT) {
                        strcpy(prfName, "D65");
                    } else if (awbHalCfg->mode == HAL_WB_FLUORESCENT) {
                        strcpy(prfName, "F11_TL84");
                    } else if (awbHalCfg->mode == HAL_WB_SUNSET) {
                        strcpy(prfName, "HORIZON"); //not support now
                    } else if (awbHalCfg->mode == HAL_WB_CLOUDY_DAYLIGHT) {
                        strcpy(prfName, "F2_CWF");
                    } else if (awbHalCfg->mode == HAL_WB_CANDLE) {
                        strcpy(prfName, "U30"); //not support now
                    } else
                        LOGE("%s:not support this awb mode %d !", __func__, awbHalCfg->mode);

                    // get number of availabe illumination profiles from database
                    result = CamCalibDbGetNoOfAwb_V10_Illuminations(hCamCalibDb, &no);
                    // run over all illumination profiles
                    for (i = 0; i < no; i++) {
                        // get an illumination profile from database
                        result = CamCalibDbGetAwb_V10_IlluminationByIdx(hCamCalibDb, i, &pIlluProfile);
                        if (strstr(pIlluProfile->name, prfName)) {
                            awbcfg.idx = i;
                            break;
                        }
                    }

                    if (i == no) {
                        LOGE("%s:can't find %s profile!", __func__, prfName);
                    } else {
                        if (awbDesc) {
                            result = awbDesc->update_awb_params(awbContext, &awbcfg);
                        }
                    }
                } else {
                    //from manual to auto
                    initAWB();
                    if (awbDesc) {
                        result = awbDesc->update_awb_params(awbContext, &awbcfg);
                    }
                }
            }
            /*
            //awb locks
            if ((cfg->aaa_locks & HAL_3A_LOCKS_WB)
            && (dCfg.awb_cfg.mode == HAL_WB_AUTO)) {
            AwbTryLock(hAwb);
            } else if (dCfg.aaa_locks & HAL_3A_LOCKS_WB)
            AwbUnLock(hAwb);
            */
        }
        mAWBHalCfg = *awbHalCfg;
        if (!mStatisticsUpdated) {
            return result;
        }
    }
    //end of dynamic config

    HalCamerIcAwbMeasure2AwbMeasure(&(mStats.awb), &(MeasResult.MesureResult));
    for (int i = 0; i < AWB_HIST_NUM_BINS; i++)
        MeasResult.HistBins[i] = mStats.aec.hist_bins[i];
    MeasResult.fGain = lastAecResult.analog_gain_code_global;
    MeasResult.fIntegrationTime = lastAecResult.coarse_integration_time;
    if (awbDesc) {
        XCamAwbParam param;
        param.mode = XCAM_AWB_MODE_AUTO;
        result = awbDesc->set_stats(awbContext, &MeasResult);
        result = awbDesc->analyze_awb(awbContext, &param);
        result = awbDesc->get_results(awbContext, &retOuput);
    } //result =  AwbRun(hAwb, &MeasResult, &retOuput);

    if (result == RET_SUCCESS) {
        //convert result
        memset(&curAwbResult, 0, sizeof(curAwbResult));
        convertAwbResult2Cameric(&retOuput, &curAwbResult);
    }

    curAwbResult.err_code = retOuput.err_code;

updateAwbConfig_end:
    return result;
}

RESULT CamIA10Engine::getAWBResults(CamIA10_AWB_Result_t* result) {
    updateAwbResults(&lastAwbResult, &curAwbResult, result);
    lastAwbResult = curAwbResult;
    result->err_code = curAwbResult.err_code;
    return RET_SUCCESS;
}

uint32_t CamIA10Engine::calcAfmTenengradShift(const uint32_t MaxPixelCnt) {
    uint32_t tgrad  = MaxPixelCnt;
    uint32_t tshift = 0U;

    while ( tgrad > (128*128) )
    {
        ++tshift;
        tgrad >>= 1;
    }

    return ( tshift );
}

uint32_t CamIA10Engine::calcAfmLuminanceShift(const uint32_t MaxPixelCnt) {
    uint32_t lgrad  = MaxPixelCnt;
    uint32_t lshift = 0U;

    while ( lgrad > 65793)
    {
        ++lshift;
        lgrad >>= 1;
    }

    return ( lshift );
}

RESULT CamIA10Engine::runAF(HAL_AfcCfg* config) {
    /*
       RESULT ret = -1;
       RESULT result = RET_SUCCESS;
       struct HAL_AfcCfg* set;

       if (mInitDynamic) {
       set = &dCfg.afc_cfg;
       } else {
       set = config;
       mAFCHalCfg = *config;
       }
       if (set->mode == HAL_AF_MODE_NOT_SET) {
       return RET_FAILURE;
       }

    //init af
    if (!hAf) {
    AfInstanceConfig_t afInstance;
    AfConfig_t afcCfg;
    if (afDesc) {
    result = afDesc->update_af_params(afContext, &afInstance);
    } //result = AfInit(&afInstance);

    if (result != RET_SUCCESS)
    return ret;
    else
    hAf = afInstance.hAf;

    afcCfg.hSensor = (void*)1;
    afcCfg.hSubSensor = (void*)1;
    afcCfg.Afss = AFM_FSS_ADAPTIVE_RANGE;

    if (afDesc) {
    result = afDesc->update_af_params(afContext, &afcCfg);
    } //result = AfConfigure(hAf, &afcCfg);

    if (result != RET_SUCCESS) {
    LOGE("%s:af config failure! result %d", __func__, result);
    hAf = NULL;
    return ret;
    }
    }

    if (afDesc) {
    XCamAfParam param;
    param.focus_mode = XCamAfOperationMode(set->mode);
    param.focus_rect_cnt = set->win_num;
    param.focus_rect[0].left_hoff = set->win_a.left_hoff;
    param.focus_rect[0].top_voff = set->win_a.top_voff;
    param.focus_rect[0].right_width = set->win_a.right_width;
    param.focus_rect[0].bottom_height = set->win_a.bottom_height;

    param.focus_rect[1].left_hoff = set->win_b.left_hoff;
    param.focus_rect[1].top_voff = set->win_b.top_voff;
    param.focus_rect[1].right_width = set->win_b.right_width;
    param.focus_rect[1].bottom_height = set->win_b.bottom_height;

    param.focus_rect[2].left_hoff = set->win_c.left_hoff;
    param.focus_rect[2].top_voff = set->win_c.top_voff;
    param.focus_rect[2].right_width = set->win_c.right_width;
    param.focus_rect[2].bottom_height = set->win_c.bottom_height;

    param.trigger_new_search = set->oneshot_trigger;
    result = afDesc->set_stats(afContext, &mStats.af);
    result = afDesc->analyze_af(afContext, &param);
    }

    return ret;
    */
    RESULT result = RET_SUCCESS;
    struct HAL_AfcCfg* set;
    struct HAL_AfcCfg* shd;

    set = &dCfg.afc_cfg;
    shd = &dCfgShd.afc_cfg;
    if (set->mode == HAL_AF_MODE_NOT_SET) {
        return RET_FAILURE;
    }

    if (shd->mode != HAL_AF_MODE_NOT_SET &&
        shd->mode != HAL_AF_MODE_FIXED) {
        if (hAf != NULL) {
            if (afDesc) {
                XCamAfParam param;
                param.focus_mode = XCamAfOperationMode(set->mode);
                param.focus_rect_cnt = set->win_num;
                param.focus_rect[0].left_hoff = set->win_a.left_hoff;
                param.focus_rect[0].top_voff = set->win_a.top_voff;
                param.focus_rect[0].right_width = set->win_a.right_width;
                param.focus_rect[0].bottom_height = set->win_a.bottom_height;

                param.focus_rect[1].left_hoff = set->win_b.left_hoff;
                param.focus_rect[1].top_voff = set->win_b.top_voff;
                param.focus_rect[1].right_width = set->win_b.right_width;
                param.focus_rect[1].bottom_height = set->win_b.bottom_height;

                param.focus_rect[2].left_hoff = set->win_c.left_hoff;
                param.focus_rect[2].top_voff = set->win_c.top_voff;
                param.focus_rect[2].right_width = set->win_c.right_width;
                param.focus_rect[2].bottom_height = set->win_c.bottom_height;

                param.trigger_new_search = set->oneshot_trigger;
                result = afDesc->set_stats(afContext, &mStats.af);
                result = afDesc->analyze_af(afContext, &param);
            }//AfProcessFrame(hAf, &mStats.af);

            if ((result != RET_SUCCESS) && (result != RET_CANCELED))
                ALOGE( "%s AfProcessFrame: %d", __func__, result );
        }
    }
    return result;
}

RESULT CamIA10Engine::getAFResults(XCam3aResultFocus* result) {

    if (afDesc) {
        afDesc->get_results(afContext, result);
    }
#if 0
    uint32_t max_pixel_cnt;
    uint32_t tshift = 0U;
    uint32_t lshift = 0U;
    struct HAL_AfcCfg* set;
    CamerIcAfmOutputResult_t afm_results;
    uint32_t pixel_cnt;
    RESULT ret;

    set = &dCfg.afc_cfg;
    if (set->mode == HAL_AF_MODE_NOT_SET) {
        return RET_FAILURE;
    }

    ret = AfGetResult(hAf, &afm_results);
    if (ret != RET_SUCCESS) {
        return ret;
    }

    result->Window_Num = afm_results.Window_Num;
    if( result->Window_Num >= 1) {
        result->WindowA = afm_results.WindowA;
        pixel_cnt = ((result->WindowA.h_size * result->WindowA.v_size) >> 1);
        max_pixel_cnt = pixel_cnt;
    }

    if( result->Window_Num >= 2) {
        result->WindowB = afm_results.WindowB;
        pixel_cnt = (( result->WindowB.h_size * result->WindowB.v_size ) >> 1);
        if( max_pixel_cnt < pixel_cnt ) {
            max_pixel_cnt = pixel_cnt;
        }
    }

    if( result->Window_Num >= 3) {
        result->WindowC = afm_results.WindowC;
        pixel_cnt  = (( result->WindowC.h_size * result->WindowC.v_size ) >> 1);
        if( max_pixel_cnt < pixel_cnt ) {
            max_pixel_cnt = pixel_cnt;
        }
    }

    tshift = calcAfmTenengradShift(max_pixel_cnt);
    lshift = calcAfmLuminanceShift(max_pixel_cnt);
    result->VarShift = lshift << 16 | tshift;
    /* LOGD("%s: win num: %d max_pixel_cnt: %d win: (%d,%d,%d,%d), lshift: 0x%x 0x%x\n",
       __func__, result->Window_Num, max_pixel_cnt,
       result->WindowA.h_offs,
       result->WindowA.v_offs,
       result->WindowA.h_size,
       result->WindowA.v_size,
       lshift, result->VarShift);*/
    result->Thres = 0x4U;
    result->LensePos = afm_results.NextLensePos;
#endif
    return RET_SUCCESS;
}

RESULT CamIA10Engine::initADPF() {
    RESULT result = RET_FAILURE;


    return result;
}

RESULT CamIA10Engine::runADPF() {
    RESULT ret = RET_FAILURE;
    if (!mInitDynamic) {
        if (hAdpf == NULL) {
            adpfCfg.data.db.width = mStats.sensor_mode.isp_input_width;
            adpfCfg.data.db.height = mStats.sensor_mode.isp_input_height;
            adpfCfg.data.db.hCamCalibDb  = hCamCalibDb;

            ret = AdpfInit(&hAdpf, &adpfCfg);
        } else {
            ret = AdpfConfigure(hAdpf, &adpfCfg);
            if (ret != RET_SUCCESS)
                return ret;
        }

        if (!mStatisticsUpdated) {
            return ret;
        }
    }
    ret = AdpfRun(hAdpf, lastAecResult.analog_gain_code_global, mLightMode);

    return RET_SUCCESS;
}

RESULT CamIA10Engine::getADPFResults(AdpfResult_t* result) {
    RESULT ret = RET_FAILURE;

    ret = AdpfGetResult(hAdpf, result);

    return ret;
}

RESULT CamIA10Engine::initAWDR() {
    RESULT result = RET_FAILURE;


    return result;
}

RESULT CamIA10Engine::runAWDR() {
    RESULT ret = RET_FAILURE;

    ret = AwdrRun(hAwdr, lastAecResult.analog_gain_code_global);

    return RET_SUCCESS;
}

RESULT CamIA10Engine::getAWDRResults(AwdrResult_t* result) {
    RESULT ret = RET_FAILURE;
    ret = AwdrGetResult(hAwdr, result);

    return ret;
}

RESULT CamIA10Engine::runManIspForOTP(struct CamIA10_Results* result) {
    result->otp_info_avl = BOOL_FALSE;
    if (mOTPInfo) {
        result->otp_info = *mOTPInfo;
        result->otp_info_avl = BOOL_TRUE;
    }

    return RET_SUCCESS;
}

RESULT CamIA10Engine::runManIspForPreIsp(struct CamIA10_Results* result) {
    RESULT ret = RET_SUCCESS;

    if (mSensorEntityName && strstr(mSensorEntityName, "1608") &&
        aecCfg.HdrCtrl.Enable) {
        struct HAL_ISP_cfg_s manCfg;
        struct HAL_ISP_goc_cfg_s goc_cfg;

        memset(&manCfg, 0, sizeof(manCfg));

        memcpy(goc_cfg.gamma_y,
               mStats.cifisp_preisp_goc_curve,
               sizeof(goc_cfg.gamma_y));
        goc_cfg.mode = HAL_ISP_GAMMA_SEG_MODE_LOGARITHMIC;
        goc_cfg.used_cnt = CIFISP_PREISP_GOC_CURVE_SIZE;
        manCfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_SETTING;
        manCfg.updated_mask |= HAL_ISP_GOC_MASK;
        manCfg.goc_cfg = &goc_cfg;
        if (true/*manCfg.updated_mask & HAL_ISP_GOC_MASK*/) {
            ret = cam_ia10_isp_goc_config
                (
                 hCamCalibDb,
                 manCfg.enabled[HAL_ISP_GOC_ID],
                 manCfg.goc_cfg,
                 &(result->goc),
                 BOOL_FALSE,
                 mIspVer
                );

            if (ret != RET_SUCCESS)
                ALOGE("%s:config GOC failed !", __FUNCTION__);
            result->active |= CAMIA10_GOC_MASK;
        }
    }

    return ret;
}

RESULT CamIA10Engine::runManIspForBW(struct CamIA10_Results* result) {
    RESULT ret = RET_SUCCESS;
    struct HAL_ISP_cfg_s manCfg;

    memset(&manCfg, 0, sizeof(manCfg));

    // check if BW sensor or raw used as BW mode
    if (dCfg.sensor_mode.is_bw_sensor) {
        // bypass demosaic
        // TODO: now engine has a bug, can't control bdm enable/disable
        // by this field, now bypass bdm in isp_controller.cpp forcely
        result->bdm.enabled = BOOL_TRUE;
    } else {
        if (mXMLIspOutputType == isp_gray_output_type) {
            struct HAL_ISP_ie_cfg_s ie_cfg;

            ie_cfg.mode = HAL_EFFECT_MONO;
            manCfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_SETTING;
            manCfg.updated_mask |= HAL_ISP_IE_MASK;
            manCfg.ie_cfg = &ie_cfg;
            ret = cam_ia10_isp_ie_config
                (
                 manCfg.enabled[HAL_ISP_IE_ID],
                 manCfg.ie_cfg,
                 &(result->ie)
                );

            if (ret != RET_SUCCESS)
                ALOGE("%s:config IE failed !", __FUNCTION__);
            result->active |= CAMIA10_IE_MASK;
        }
    }

    return ret;
}

RESULT CamIA10Engine::runManISP(struct HAL_ISP_cfg_s* manCfg, struct CamIA10_Results* result) {
    RESULT ret = RET_SUCCESS;
    int width = dCfg.sensor_mode.isp_input_width;
    int height = dCfg.sensor_mode.isp_input_height;

    if (!mInitDynamic) {
        width = mStats.sensor_mode.isp_input_width;
        height = mStats.sensor_mode.isp_input_height;
    }

    //may override other awb related modules, so need place it first.
    if (manCfg->updated_mask & HAL_ISP_AWB_MEAS_MASK) {
        CamerIcAwbMeasConfig_t awb_meas_result = {BOOL_FALSE, 0, 0, 0};
        awb_meas_result.awb_meas_mode_result = &(result->awb.MeasMode);
        awb_meas_result.awb_meas_result = &(result->awb.MeasConfig);
        awb_meas_result.awb_win = &(result->awb.awbWin);
        ret = cam_ia10_isp_awb_meas_config
            (
             manCfg->enabled[HAL_ISP_AWB_MEAS_ID],
             manCfg->awb_cfg,
             &(awb_meas_result)
            );

        if ((manCfg->awb_cfg) && (!manCfg->enabled[HAL_ISP_AWB_MEAS_ID])
            && (manCfg->awb_cfg->illuIndex >= 0)) {
            //to get default LSC,CC,awb gain settings correspoding to illu
            awbcfg.Mode = AWB_MODE_MANUAL;
            //LOGD("%s:illu index %d",__func__,manCfg->awb_cfg->illuIndex);
            awbcfg.idx = manCfg->awb_cfg->illuIndex;
            if (awbDesc) {
                ret = awbDesc->update_awb_params(awbContext, &awbcfg);
            }

            runAWB();
            getAWBResults(&result->awb);
        }

        if (ret != RET_SUCCESS)
            ALOGE("%s:config AWB Meas failed !", __FUNCTION__);
        result->active |= CAMIA10_AWB_MEAS_MASK;
        result->awb_meas_enabled = awb_meas_result.enabled;
    }

    if (manCfg->updated_mask & HAL_ISP_BPC_MASK) {
        ret =  cam_ia10_isp_dpcc_config
            (
             manCfg->enabled[HAL_ISP_BPC_ID],
             manCfg->dpcc_cfg,
             hCamCalibDb,
             width,
             height,
             &(result->dpcc)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config DPCC failed !", __FUNCTION__);
        result->active |= CAMIA10_BPC_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_BLS_MASK) {
        ret = cam_ia10_isp_bls_config
            (
             manCfg->enabled[HAL_ISP_BLS_ID],
             hCamCalibDb,
             width,
             height,
             manCfg->bls_cfg,
             &(result->bls)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config BLS failed !", __FUNCTION__);
        result->active |= CAMIA10_BLS_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_SDG_MASK) {
        ret = cam_ia10_isp_sdg_config
            (
             manCfg->enabled[HAL_ISP_SDG_ID],
             manCfg->sdg_cfg,
             &(result->sdg)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config SDG failed !", __FUNCTION__);
        result->active |= CAMIA10_SDG_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_HST_MASK) {
        ret = cam_ia10_isp_hst_config
            (
             manCfg->enabled[HAL_ISP_HST_ID],
             manCfg->hst_cfg,
             width,
             height,
             mIspVer,
             &(result->hst)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config hst failed !", __FUNCTION__);
        result->active |= CAMIA10_HST_MASK;
        result->aec.actives |= CAMIA10_HST_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_LSC_MASK) {
        CamerIcLscConfig_t lsc_result = {BOOL_FALSE, 0, 0};
        lsc_result.lsc_result = &(result->awb.LscMatrixTable);
        lsc_result.lsc_seg_result = &(result->awb.SectorConfig);
        ret = cam_ia10_isp_lsc_config
            (
             manCfg->enabled[HAL_ISP_LSC_ID],
             manCfg->lsc_cfg,
             width,
             height,
             mIspVer,
             &(lsc_result)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config LSC failed !", __FUNCTION__);
        result->active |= CAMIA10_LSC_MASK;
        result->lsc_enabled = lsc_result.enabled;
    }

    if (manCfg->updated_mask & HAL_ISP_AWB_GAIN_MASK) {
        CameraIcAwbGainConfig_t awb_result = {BOOL_FALSE, 0};
        awb_result.awb_gain_result = &(result->awb.awbGains);
        ret = cam_ia10_isp_awb_gain_config
            (
             manCfg->enabled[HAL_ISP_AWB_GAIN_ID],
             manCfg->awb_gain_cfg,
             &(awb_result)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config AWB Gain failed !", __FUNCTION__);
        result->active |= CAMIA10_AWB_GAIN_MASK;
        result->awb_gains_enabled = awb_result.enabled;
    }

    if (manCfg->updated_mask & HAL_ISP_FLT_MASK) {
        ret = cam_ia10_isp_flt_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_FLT_ID],
             manCfg->flt_cfg,
             width,
             height,
             &(result->flt)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config FLT failed !", __FUNCTION__);
        result->active |= CAMIA10_FLT_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_BDM_MASK) {
        ret = cam_ia10_isp_bdm_config
            (
             manCfg->enabled[HAL_ISP_BDM_ID],
             manCfg->bdm_cfg,
             &(result->bdm)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config BDM failed !", __FUNCTION__);
        result->active |= CAMIA10_BDM_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_CTK_MASK) {
        CameraIcCtkConfig_t ctk_result = {BOOL_FALSE, 0, 0};
        ctk_result.ctk_matrix_result = &(result->awb.CcMatrix);
        ctk_result.ctk_offset_result = &(result->awb.CcOffset);
        ret = cam_ia10_isp_ctk_config
            (
             manCfg->enabled[HAL_ISP_CTK_ID],
             manCfg->ctk_cfg,
             &(ctk_result)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config CTK failed !", __FUNCTION__);
        result->active |= CAMIA10_CTK_MASK;
        result->ctk_enabled = ctk_result.enabled;
    }


    if (manCfg->updated_mask & HAL_ISP_CPROC_MASK) {
        ret = cam_ia10_isp_cproc_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_CPROC_ID],
             manCfg->cproc_cfg,
             &(result->cproc)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config CPROC failed !", __FUNCTION__);
        result->active |= CAMIA10_CPROC_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_IE_MASK) {
        ret = cam_ia10_isp_ie_config
            (
             manCfg->enabled[HAL_ISP_IE_ID],
             manCfg->ie_cfg,
             &(result->ie)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config IE failed !", __FUNCTION__);
        result->active |= CAMIA10_IE_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_AEC_MASK) {
        CameraIcAecConfig_t aec_result = {BOOL_FALSE, 0, 0};
        aec_result.aec_meas_mode = (int*)(&(result->aec.meas_mode));
        aec_result.meas_win = &(result->aec.meas_win);
        ret = cam_ia10_isp_aec_config
            (
             manCfg->enabled[HAL_ISP_AEC_ID],
             manCfg->aec_cfg,
             &(aec_result)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config AEC Meas failed !", __FUNCTION__);
        result->active |= CAMIA10_AEC_MASK;
        result->aec_enabled = aec_result.enabled;

        if ((manCfg->aec_cfg) && (!aec_result.enabled) &&
            ((manCfg->aec_cfg->exp_time > 0.01) || (manCfg->aec_cfg->exp_gain > 0.01))) {
            mapHalExpToSensor
                (
                 manCfg->aec_cfg->exp_gain,
                 manCfg->aec_cfg->exp_time,
                 result->aec.regGain,
                 result->aec.regIntegrationTime
                );
            //FIXME: for some reason, kernel report error manual ae time and gain values to HAL
            //if aec is disabled , so here is just a workaround
            result->aec_enabled = BOOL_TRUE;
            result->aec.actives |= CAMIA10_AEC_MASK;
        }

    }

    /*TODOS*/
    if (manCfg->updated_mask & HAL_ISP_WDR_MASK) {
        ret = cam_ia10_isp_wdr_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_WDR_ID],
             manCfg->wdr_cfg,
             &(result->wdr)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config WDR failed !", __FUNCTION__);
        result->active |= CAMIA10_WDR_MASK;
        if (manCfg->enabled[HAL_ISP_WDR_ID] == HAL_ISP_ACTIVE_FALSE) {
            // stop awdr
            awdrCfg.mode = AWDR_MODE_CONTROL_BY_MANUAL;
            //mWdrEnabledState = BOOL_FALSE;
        } else {
            awdrCfg.mode = AWDR_MODE_CONTROL_BY_GAIN;
            //mWdrEnabledState = BOOL_TRUE;
        }

        mWdrEnabledState = result->wdr.enabled;
        LOGD("-----set wdr state: %d", mWdrEnabledState);
    }


    if (manCfg->updated_mask & HAL_ISP_GOC_MASK) {
        ret = cam_ia10_isp_goc_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_GOC_ID],
             manCfg->goc_cfg,
             &(result->goc),
             mWdrEnabledState,
             mIspVer
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config GOC failed !", __FUNCTION__);
        result->active |= CAMIA10_GOC_MASK;
    }

    if (manCfg->updated_mask & HAL_ISP_DPF_MASK) {
        CameraIcDpfConfig_t dpfConfig;
        ret = cam_ia10_isp_dpf_config
            (
             manCfg->enabled[HAL_ISP_DPF_ID],
             manCfg->dpf_cfg,
             &(dpfConfig)
            );

        if (ret != RET_SUCCESS)
            ALOGE("%s:config DPF failed !", __FUNCTION__);
        result->active |= CAMIA10_DPF_MASK;
        result->adpf_enabled = dpfConfig.enabled;
    }

    if (manCfg->updated_mask & HAL_ISP_DPF_STRENGTH_MASK) {
        CameraIcDpfStrengthConfig_t dpfStrengConfig;
        ret = cam_ia10_isp_dpf_strength_config
            (
             manCfg->enabled[HAL_ISP_DPF_STRENGTH_ID],
             manCfg->dpf_strength_cfg,
             &(dpfStrengConfig)
            );

        result->adpf.DynInvStrength.WeightB = dpfStrengConfig.b;
        result->adpf.DynInvStrength.WeightG = dpfStrengConfig.g;
        result->adpf.DynInvStrength.WeightR = dpfStrengConfig.r;
        if (ret != RET_SUCCESS)
            ALOGE("%s:config DPF strength failed !", __FUNCTION__);
        result->active |= CAMIA10_DPF_STRENGTH_MASK;
        result->adpf_strength_enabled = dpfStrengConfig.enabled;
    }

    if (manCfg->updated_mask & HAL_ISP_AFC_MASK) {

    }

	if (manCfg->updated_mask & HAL_ISP_DEMOSAICLP_MASK) {
		ret = cam_ia10_isp_demosaicLp_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_DEMOSAICLP_ID],
             manCfg->demosaicLP_cfg,
             width,
             height,
             &(result->rkDemosaicLP)
            );
		if (ret != RET_SUCCESS)
            ALOGE("%s:config demosaiclp failed !", __FUNCTION__);
		result->active |= CAMIA10_DEMOSAICLP_MASK;
    }

	if (manCfg->updated_mask & HAL_ISP_RK_IESHARP_MASK) {
		ret = cam_ia10_isp_rkIEsharp_config
            (
             hCamCalibDb,
             manCfg->enabled[HAL_ISP_RKIESHARP_ID],
             manCfg->rkIEsharp_cfg,
             width,
             height,
             &(result->rkIEsharp)
            );
		if (ret != RET_SUCCESS)
            ALOGE("%s:config demosaiclp failed !", __FUNCTION__);
		result->active |= CAMIA10_RKIESHARP_MASK;
    }

    runManIspForBW(result);
    runManIspForPreIsp(result);
    runManIspForOTP(result);

    return ret;
}

shared_ptr<CamIA10EngineItf> getCamIA10EngineItf(void) {
    return shared_ptr<CamIA10EngineItf>(new CamIA10Engine());
}

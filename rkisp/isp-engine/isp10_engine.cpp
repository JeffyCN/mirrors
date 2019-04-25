#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include "isp10_engine.h"
//#include "camHalTrace.h"
#include <ebase/utl_fixfloat.h>
//#include "CamIsp10DevHwItf.h"

using namespace std;

static bool AecMeasuringMode_to_cifisp_exp_meas_mode(
    AecMeasuringMode_t in, enum cifisp_exp_meas_mode* out) {
  switch (in) {
    case AEC_MEASURING_MODE_1:
      *out = CIFISP_EXP_MEASURING_MODE_0;
      return true;
    case AEC_MEASURING_MODE_2:
      *out = CIFISP_EXP_MEASURING_MODE_1;
      return true;
    default:
      return false;
  }
}

static int setExposure(int m_cam_fd_overlay, unsigned int exposure, unsigned int gain, unsigned int gain_percent) {
  int ret;
  struct v4l2_ext_control exp_gain[3];
  struct v4l2_ext_controls ctrls;

  exp_gain[0].id = V4L2_CID_EXPOSURE;

  exp_gain[0].value = exposure;
  exp_gain[1].id = V4L2_CID_GAIN;
  exp_gain[1].value = gain;
  exp_gain[2].id = RK_V4L2_CID_GAIN_PERCENT;
  exp_gain[2].value = gain_percent;

  ctrls.count = 3;
  ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
  ctrls.controls = exp_gain;
  ctrls.reserved[0] = 0;
  ctrls.reserved[1] = 0;

  ret = ioctl(m_cam_fd_overlay, /*VIDIOC_S_CTRL*/VIDIOC_S_EXT_CTRLS, &ctrls);

  if (ret < 0) {
    LOGE("ERR(%s-%d):set of  AE seting to sensor config failed! err: %s\n",
         __func__,
         m_cam_fd_overlay,
         strerror(errno));
    return ret;
  }

  return ret;
}

static int SetAutoAdjustFps(int m_cam_fd_overlay, bool auto_adjust_fps) {
#if RKISP
    return 0;
#else
  int ret = 0;
  struct v4l2_control ctrl;

  ctrl.id = RK_V4L2_CID_AUTO_FPS;
  ctrl.value = auto_adjust_fps;
/*
  if((m_fps.numerator != 0 || m_fps.denominator != 0) && auto_adjust_fps)
  {
	LOGE("(%s) !!!!!!!!Warning: hal set fixfps(%d/%d), but IQ xml set auto_adjust_fps(%d). AE must have problem !!!!!!!!\n",
         __func__, m_fps.denominator, m_fps.numerator, auto_adjust_fps);
  }
*/
  ret = ioctl(m_cam_fd_overlay, VIDIOC_S_CTRL, &ctrl);

  if (ret < 0) {
    LOGE("ERR(%s):set of  AE seting auto adjust fps to sensor config failed! err: %s\n",
         __func__,
         strerror(errno));
  }
  return ret;
#endif
}

Isp10Engine::Isp10Engine():
  IspEngine(),
  mISP3AThread(new ISP3AThread(this)) {
  //LOGD("%s: E", __func__);

  memset(&mIspCfg, 0, sizeof(mIspCfg));
  mBlsNeededUpdate = BOOL_TRUE;
  mBlsEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mSdgNeededUpdate = BOOL_TRUE;
  mSdgEnabled = HAL_ISP_ACTIVE_FALSE;
  mFltNeededUpdate = BOOL_TRUE;
  mFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mGocNeededUpdate = BOOL_TRUE;
  mGocEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mCprocNeededUpdate = BOOL_FALSE;
  mCprocEnabled = HAL_ISP_ACTIVE_FALSE;
  mIeNeededUpdate = BOOL_FALSE;
  mIeEnabled = HAL_ISP_ACTIVE_FALSE;
  mDpccNeededUpdate = BOOL_TRUE;
  mDpccEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mBdmNeededUpdate = BOOL_TRUE;
  mBdmEnabled = HAL_ISP_ACTIVE_DEFAULT;

  /*following modules will be initialized by AWB algorithm*/
  mLscNeededUpdate = BOOL_FALSE;
  mLscEnabled = HAL_ISP_ACTIVE_FALSE;
  mAwbGainNeededUpdate = BOOL_FALSE;
  mAwbEnabled = HAL_ISP_ACTIVE_FALSE;
  mCtkNeededUpdate = BOOL_FALSE;
  mCtkEnabled = HAL_ISP_ACTIVE_FALSE;
  mAwbMeNeededUpdate = BOOL_FALSE;
  mAwbMeEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by AEC algorithm*/
  mAecNeededUpdate = BOOL_FALSE;
  mAecEnabled = HAL_ISP_ACTIVE_FALSE;
  mHstNeededUpdate = BOOL_FALSE;
  mHstEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by ADPF algorithm*/
  mDpfNeededUpdate = BOOL_FALSE;
  mDpfEnabled = HAL_ISP_ACTIVE_FALSE;
  mDpfStrengthNeededUpdate = BOOL_FALSE;
  mDpfStrengthEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by AFC algorithm*/
  mAfcNeededUpdate = BOOL_FALSE;
  mAfcEnabled = HAL_ISP_ACTIVE_FALSE;

  mWdrNeededUpdate = BOOL_FALSE;
  mWdrEnabled = HAL_ISP_ACTIVE_FALSE;

  mDemosaicLPNeededUpdate = BOOL_FALSE;
  mDemosaicLPEnable = HAL_ISP_ACTIVE_FALSE;

  mrkIEsharpNeededUpdate = BOOL_FALSE;
  mrkIEsharpEnable = HAL_ISP_ACTIVE_FALSE;

  m3DnrNeededUpdate = BOOL_FALSE;
  m3DnrEnabled = HAL_ISP_ACTIVE_FALSE;

  mNew3DnrNeededUpdate = BOOL_FALSE;
  mNew3DnrEnabled = HAL_ISP_ACTIVE_FALSE;

  mCamIAEngine = getCamIA10EngineItf();

  //LOGD("%s: x", __func__);
}
Isp10Engine::~Isp10Engine() {

}

bool Isp10Engine::init(const char* tuningFile,
                       const char* ispDev,
                       int isp_ver,
                       int devFd) {
  int i;
  bool ret = false;
  struct CamIA10_Results ia_results;
  struct CamIsp10ConfigSet isp_cfg;
  mDevFd = devFd;
  mIspVer = isp_ver;

  osMutexLock(&mApiLock);
  if (mInitialized == 0) {
    if (mCamIAEngine == NULL) {
      ALOGE("%s: getCamIA10EngineItf failed!",
            __func__);
      goto init_exit;
    }

    LOGD("%s:tuningFile %s", __func__, tuningFile);
    if (mCamIAEngine->initStatic((char*)tuningFile, ispDev, isp_ver) != RET_SUCCESS) {
      ALOGE("%s: initstatic failed", __func__);
      osMutexUnlock(&mApiLock);
      deInit();
      osMutexLock(&mApiLock);
      ret = false;
      goto init_exit;
    }
	LOGD("initStatic success");
/*
    if (!initISPStream(ispDev)) {
      ALOGE("%s: initISPStream failed but continue", __func__);
      osMutexUnlock(&mApiLock);
      deInit();
      osMutexLock(&mApiLock);
      goto init_exit;
    }
*/
	LOGD("initISPStream bypass");

#if 0
    if ((mCamIA_DyCfg.aec_cfg.win.right_width == 0) ||
        (mCamIA_DyCfg.aec_cfg.win.bottom_height == 0)) {
      mCamIA_DyCfg.aec_cfg.win.left_hoff = 512;
      mCamIA_DyCfg.aec_cfg.win.top_voff = 512;
      mCamIA_DyCfg.aec_cfg.win.right_width = 1024;
      mCamIA_DyCfg.aec_cfg.win.bottom_height = 1024;
    }
#endif
    for (i = 0; i < CAM_ISP_NUM_OF_STAT_BUFS; i++) {
      mIspStats[i] = (struct cifisp_stat_buffer*)mIspStatBuf[i];
    }
    mIspIoctl = new V4l2Isp10Ioctl(mIspFd);
/*
    memset(&ia_results, 0, sizeof(struct CamIA10_Results));
    runIA(&mCamIA_DyCfg, NULL, &ia_results);
    runISPManual(&ia_results, BOOL_FALSE)    ;
    convertIAResults(&isp_cfg, &ia_results);
    applyIspConfig(&isp_cfg);
*/
  }
  mInitialized++;
  ret = true;
  LOGD("%s: int succeed %s", __func__, tuningFile);

init_exit:
  osMutexUnlock(&mApiLock);
  return ret;

}

bool Isp10Engine::deInit() {
  osMutexLock(&mApiLock);
  if ((mInitialized > 0) && (--mInitialized == 0)) {
    struct CamIsp10ConfigSet isp_cfg;
    isp_cfg.active_configs = 0xffffffff;
    memset(isp_cfg.enabled, 0, sizeof(isp_cfg.enabled));
    applyIspConfig(&isp_cfg);
    if (mIspIoctl) {
      delete mIspIoctl;
      mIspIoctl = NULL;
    }
  }
  osMutexUnlock(&mApiLock);
  if (mInitialized == 0)
    return IspEngine::deInit();

  return true;
}

bool Isp10Engine::start() {
	LOGD("Isp10Engine start run to 1x");
	bool ret = true;
	ret = IspEngine::start();

	LOGD("%s: run ISP3ATh\n", __func__);
	if (RET_SUCCESS != mISP3AThread->run("ISP3ATh", OSLAYER_THREAD_PRIO_HIGH)) {
	  ALOGE("%s: ISP3ATh thread start failed", __func__);
	  stop();
	  ret = false;
	} else {
	  LOGD("%s: ISP3ATh start\n", __func__);
	}

	return ret;
}

bool Isp10Engine::stop() {
	LOGD("Isp10Engine stop run to 1x");
	bool ret = true;
	mISP3AThread->requestExitAndWait();
	ret =  IspEngine::stop();
	return ret;
}

bool Isp10Engine::applyIspConfig(struct CamIsp10ConfigSet* isp_cfg) {
  if (isp_cfg->active_configs & ISP_BPC_MASK) {
    mIspCfg.dpcc_config = isp_cfg->configs.dpcc_config;
    mIspCfg.enabled[HAL_ISP_BPC_ID] =
        isp_cfg->enabled[HAL_ISP_BPC_ID];
  }

  if (isp_cfg->active_configs & ISP_BLS_MASK) {
    mIspCfg.bls_config = isp_cfg->configs.bls_config;
    mIspCfg.enabled[HAL_ISP_BLS_ID] =
        isp_cfg->enabled[HAL_ISP_BLS_ID];
  }

  if (isp_cfg->active_configs & ISP_SDG_MASK) {
    mIspCfg.sdg_config = isp_cfg->configs.sdg_config;
    mIspCfg.enabled[HAL_ISP_SDG_ID] =
        isp_cfg->enabled[HAL_ISP_SDG_ID];
  }

  if (isp_cfg->active_configs & ISP_HST_MASK) {
    mIspCfg.hst_config = isp_cfg->configs.hst_config;
    mIspCfg.enabled[HAL_ISP_HST_ID] =
        isp_cfg->enabled[HAL_ISP_HST_ID];
  }

  if (isp_cfg->active_configs & ISP_LSC_MASK) {
    mIspCfg.lsc_config = isp_cfg->configs.lsc_config;
    mIspCfg.enabled[HAL_ISP_LSC_ID] =
        isp_cfg->enabled[HAL_ISP_LSC_ID];
  }

  if (isp_cfg->active_configs & ISP_AWB_MEAS_MASK) {
    mIspCfg.awb_meas_config = isp_cfg->configs.awb_meas_config;
    mIspCfg.enabled[HAL_ISP_AWB_MEAS_ID] =
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID];
  }

  if (isp_cfg->active_configs & ISP_AWB_GAIN_MASK) {
    mIspCfg.awb_gain_config = isp_cfg->configs.awb_gain_config;
    mIspCfg.enabled[HAL_ISP_AWB_GAIN_ID] =
        isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID];
  }
  if (isp_cfg->active_configs & ISP_FLT_MASK) {
    mIspCfg.flt_config = isp_cfg->configs.flt_config;
    mIspCfg.flt_denoise_level =
        isp_cfg->configs.flt_denoise_level;
    mIspCfg.flt_sharp_level =
        isp_cfg->configs.flt_sharp_level;
    mIspCfg.enabled[HAL_ISP_FLT_ID] =
        isp_cfg->enabled[HAL_ISP_FLT_ID];
  }
  if (isp_cfg->active_configs & ISP_BDM_MASK) {
    mIspCfg.bdm_config = isp_cfg->configs.bdm_config;
    mIspCfg.enabled[HAL_ISP_BDM_ID] =
        isp_cfg->enabled[HAL_ISP_BDM_ID];
  }

  if (isp_cfg->active_configs & ISP_CTK_MASK) {
    mIspCfg.ctk_config = isp_cfg->configs.ctk_config;
    mIspCfg.enabled[HAL_ISP_CTK_ID] =
        isp_cfg->enabled[HAL_ISP_CTK_ID];
  }

  if (isp_cfg->active_configs & ISP_GOC_MASK) {
    mIspCfg.goc_config = isp_cfg->configs.goc_config;
    mIspCfg.enabled[HAL_ISP_GOC_ID] =
        isp_cfg->enabled[HAL_ISP_GOC_ID];
  }

  if (isp_cfg->active_configs & ISP_CPROC_MASK) {
    LOGV("%s:apply cproc config!enabled %d",
            __func__, isp_cfg->enabled[HAL_ISP_CPROC_ID]
           );
    mIspCfg.cproc_config = isp_cfg->configs.cproc_config;
    mIspCfg.enabled[HAL_ISP_CPROC_ID] =
        isp_cfg->enabled[HAL_ISP_CPROC_ID];
  }

  if (isp_cfg->active_configs & ISP_AEC_MASK) {
    mIspCfg.aec_config = isp_cfg->configs.aec_config;
    mIspCfg.enabled[HAL_ISP_AEC_ID] =
        isp_cfg->enabled[HAL_ISP_AEC_ID];
  }

  if (isp_cfg->active_configs & ISP_AFC_MASK) {
    mIspCfg.afc_config = isp_cfg->configs.afc_config;
    mIspCfg.enabled[HAL_ISP_AFC_ID] =
        isp_cfg->enabled[HAL_ISP_AFC_ID];
  }

  if (isp_cfg->active_configs & ISP_IE_MASK) {
    LOGV("%s:apply ie config,enabled %d!", __func__,
            isp_cfg->enabled[HAL_ISP_IE_ID]);
    mIspCfg.ie_config = isp_cfg->configs.ie_config;
    mIspCfg.enabled[HAL_ISP_IE_ID] =
        isp_cfg->enabled[HAL_ISP_IE_ID];
  }

  if (isp_cfg->active_configs & ISP_DPF_MASK) {
    mIspCfg.dpf_config = isp_cfg->configs.dpf_config;
    mIspCfg.enabled[HAL_ISP_DPF_ID] =
        isp_cfg->enabled[HAL_ISP_DPF_ID];
  }

  if (isp_cfg->active_configs & ISP_DPF_STRENGTH_MASK) {
    mIspCfg.dpf_strength_config = isp_cfg->configs.dpf_strength_config;
    mIspCfg.enabled[HAL_ISP_DPF_STRENGTH_ID] =
        isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID];
  }

  if (isp_cfg->active_configs & ISP_WDR_MASK) {
	mIspCfg.wdr_config = isp_cfg->configs.wdr_config;
	mIspCfg.enabled[HAL_ISP_WDR_ID] =
	  isp_cfg->enabled[HAL_ISP_WDR_ID];
  }

  if (isp_cfg->active_configs & ISP_DSP_3DNR_MASK) {
  	mIspCfg.Dsp3DnrSetConfig = isp_cfg->configs.Dsp3DnrSetConfig;
  }
  
  if (isp_cfg->active_configs & ISP_NEW_DSP_3DNR_MASK) {
  	mIspCfg.NewDsp3DnrSetConfig = isp_cfg->configs.NewDsp3DnrSetConfig;
  }
  
  if (isp_cfg->active_configs & ISP_RK_DEMOSAICLP_MASK) {
    mIspCfg.demosaicLp_config = isp_cfg->configs.demosaicLp_config;
	mIspCfg.enabled[HAL_ISP_DEMOSAICLP_ID] =
	  isp_cfg->enabled[HAL_ISP_DEMOSAICLP_ID];
  }
  
  if (isp_cfg->active_configs & ISP_RK_IESHARP_MASK) {
    mIspCfg.rkIESharp_config = isp_cfg->configs.rkIESharp_config;
	mIspCfg.enabled[HAL_ISP_RKIESHARP_ID] =
	  isp_cfg->enabled[HAL_ISP_RKIESHARP_ID];
  }
  
  return true;
}

bool Isp10Engine::convertIspStats(
    struct cifisp_stat_buffer* isp_stats,
    struct CamIA10_Stats* ia_stats) {
  unsigned int i;

  if (isp_stats->meas_type & CIFISP_STAT_AUTOEXP) {
    ia_stats->meas_type |= CAMIA10_AEC_MASK;
    memcpy(ia_stats->aec.exp_mean,
           isp_stats->params.ae.exp_mean,
           sizeof(ia_stats->aec.exp_mean));
    /*
    LOGD("> AE Measurement:\n");
    for (i = 0; i < CIFISP_AE_MEAN_MAX; i += 5) {
      LOGD(">     Exposure means %d-%d: %d, %d, %d, %d, %d\n",i, i+4,
        isp_stats->params.ae.exp_mean[i],
        isp_stats->params.ae.exp_mean[i + 1],
        isp_stats->params.ae.exp_mean[i + 2],
        isp_stats->params.ae.exp_mean[i + 3],
        isp_stats->params.ae.exp_mean[i + 4]);

    }*/
  }

  if (isp_stats->meas_type & CIFISP_STAT_HIST) {
    ia_stats->meas_type |= CAMIA10_HST_MASK;
    memcpy(ia_stats->aec.hist_bins,
           isp_stats->params.hist.hist_bins,
           sizeof(ia_stats->aec.hist_bins));
    /*
    for (int i=0; i<CIFISP_HIST_BIN_N_MAX; i+=4) {
      LOGD("histogram > %d-%d-%d-%d: %d-%d-%d-%d \n",
          i, i+1, i+2, i+3,
          isp_stats->params.hist.hist_bins[i],
          isp_stats->params.hist.hist_bins[i+1],
          isp_stats->params.hist.hist_bins[i+2],
          isp_stats->params.hist.hist_bins[i+3]);
    }
    */
  }

  if (isp_stats->meas_type & CIFISP_STAT_EMB_DATA) {
    cifisp_preisp_hdr_ae_embeded_type_t *preisp_hdr_ae_stats =
           (cifisp_preisp_hdr_ae_embeded_type_t *)isp_stats->params.emd.data;

    ia_stats->meas_type |= CAMIA10_AEC_MASK | CAMIA10_HST_MASK;
    ia_stats->aec.is_hdr_stats = true;

    ia_stats->aec.lgmean = preisp_hdr_ae_stats->result.lgmean;
    ia_stats->aec.sensor.exp_time_l =
            preisp_hdr_ae_stats->result.reg_exp_time[0];
    ia_stats->aec.sensor.exp_time =
            preisp_hdr_ae_stats->result.reg_exp_time[1];
    ia_stats->aec.sensor.exp_time_s =
            preisp_hdr_ae_stats->result.reg_exp_time[2];
    ia_stats->aec.sensor.gain_l =
            preisp_hdr_ae_stats->result.reg_exp_gain[0];
    ia_stats->aec.sensor.gain =
            preisp_hdr_ae_stats->result.reg_exp_gain[1];
    ia_stats->aec.sensor.gain_s =
            preisp_hdr_ae_stats->result.reg_exp_gain[2];
    for (int i = 0; i < CIFISP_PREISP_HDRAE_MAXFRAMES; i++) {
        memcpy((char*)ia_stats->aec.oneframe[i].hdr_hist_bins,
               (char*)preisp_hdr_ae_stats->result.oneframe[i].hist_meas.hist_bin,
               sizeof(ia_stats->aec.oneframe[i].hdr_hist_bins));
        memcpy((char*)ia_stats->aec.oneframe[i].hdr_exp_mean,
               (char*)preisp_hdr_ae_stats->result.oneframe[i].mean_meas.y_meas,
               sizeof(ia_stats->aec.oneframe[i].hdr_exp_mean));
    }

    ia_stats->aec.fDRIndex.NormalIndex =
            preisp_hdr_ae_stats->result.DRIndexRes.fNormalIndex;
    ia_stats->aec.fDRIndex.LongIndex =
            preisp_hdr_ae_stats->result.DRIndexRes.fLongIndex;
    ia_stats->aec.fOEMeasRes.OE_Pixel =
            preisp_hdr_ae_stats->result.OEMeasRes.OE_Pixel;
    ia_stats->aec.fOEMeasRes.SumHistPixel =
            preisp_hdr_ae_stats->result.OEMeasRes.SumHistPixel;
    ia_stats->aec.fOEMeasRes.SframeMaxLuma =
            preisp_hdr_ae_stats->result.OEMeasRes.SframeMaxLuma;
    memcpy(ia_stats->cifisp_preisp_goc_curve,
           preisp_hdr_ae_stats->cifisp_preisp_goc_curve,
           sizeof(ia_stats->cifisp_preisp_goc_curve));
  }

  if (isp_stats->meas_type & CIFISP_STAT_AWB) {
    ia_stats->meas_type |= CAMIA10_AWB_MEAS_MASK;
    if (mIspCfg.awb_meas_config.awb_mode == CIFISP_AWB_MODE_YCBCR) {
      ia_stats->awb.NoWhitePixel = isp_stats->params.awb.awb_mean[0].cnt;
#if RKISP
        ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_y_or_g;
        ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_cb_or_b;
        ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_cr_or_r;
#else
      ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_y;
      ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_cb;
      ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_cr;
#endif
    } else if (mIspCfg.awb_meas_config.awb_mode == CIFISP_AWB_MODE_RGB) {
        ia_stats->awb.NoWhitePixel = isp_stats->params.awb.awb_mean[0].cnt;

#if RKISP
        ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_y_or_g;
        ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_cb_or_b;
        ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_cr_or_r;
#else
      ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_g;
      ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_b;
      ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_r;
#endif
    } else {
      memset(&ia_stats->awb, 0, sizeof(ia_stats->awb));
    }
#if 0
    LOGD("AWB mode:%d, RGBorYUV=[%d-%d-%d]",
            mIspCfg.awb_meas_config.awb_mode,
            isp_stats->params.awb.awb_mean[0].mean_y_or_g,
            isp_stats->params.awb.awb_mean[0].mean_cb_or_b,
            isp_stats->params.awb.awb_mean[0].mean_cr_or_r);
#endif
  }

  if (isp_stats->meas_type & CIFISP_STAT_AFM_FIN) {
    ia_stats->meas_type |= CAMIA10_AFC_MASK;
    ia_stats->af.cameric.SharpnessA = isp_stats->params.af.window[0].sum;
    ia_stats->af.cameric.LuminanceA = isp_stats->params.af.window[0].lum;
    ia_stats->af.cameric.SharpnessB = isp_stats->params.af.window[1].sum;
    ia_stats->af.cameric.LuminanceB = isp_stats->params.af.window[1].lum;
    ia_stats->af.cameric.SharpnessC = isp_stats->params.af.window[2].sum;
    ia_stats->af.cameric.LuminanceC = isp_stats->params.af.window[2].lum;

    for (int i=0; i<CIFISP_AFM_MAX_WINDOWS; i++) {
      LOGD("Af window[%d]: lum=0x%08x, sum=0x%08x\n", i,
          isp_stats->params.af.window[i].lum,
          isp_stats->params.af.window[i].sum);
    }
  }

  return true;
}

bool Isp10Engine::configureISP(const void* config) {
  bool ret = IspEngine::configureISP(config);
  if (ret) {
    if (mCamIA_DyCfg.uc == UC_RAW) {
      struct CamIA10_Results ia_results;
      struct CamIsp10ConfigSet isp_cfg;

      memset(&ia_results, 0, sizeof(struct CamIA10_Results));
      memset(&isp_cfg, 0, sizeof(struct CamIsp10ConfigSet));
      //run isp manual config??will override the 3A results
      if (!runISPManual(&ia_results, BOOL_TRUE))
        ALOGE("%s:run ISP manual failed!", __func__);
      convertIAResults(&isp_cfg, &ia_results);
      applyIspConfig(&isp_cfg);
    }
  }

  return ret;
}

/*
 * should keep in mind that struct in CamIsp10ConfigSet
 * and CamIA10_Results may have same filed name, even the
 * same filed type, but in fact, these two struct are
 * different, struct CamIA10_Results is the result from 3A
 * algorithm while struct CamIsp10ConfigSet represents the
 * driver side. so we should be very careful to copy something
 * from struct CamIA10_Results to struct CamIsp10ConfigSet,
 * because some fileds such as hist weights in struct
 * CamIA10_Results may be bigger than latter.
 */
bool Isp10Engine::convertIAResults(
    struct CamIsp10ConfigSet* isp_cfg,
    struct CamIA10_Results* ia_results) {
  unsigned int i;

  if (isp_cfg == NULL)
    return false;

  isp_cfg->active_configs = 0;
  if (ia_results) {
    if ((ia_results->active & CAMIA10_AEC_MASK)
        || (ia_results->active & CAMIA10_HST_MASK)
        || (ia_results->active & CAMIA10_AEC_AFPS_MASK)) {
      if (ia_results->aec.actives & CAMIA10_AEC_MASK) {
        /*ae enable or manual exposure*/
#if 0
        if ((ia_results->aec_enabled) ||
            ((ia_results->aec.regIntegrationTime > 0) ||
             (ia_results->aec.regGain > 0))) {
          int newTime = ia_results->aec.regIntegrationTime;
          int newGain = ia_results->aec.regGain;

          LOGV("set exposure time: %d, gain: %d, pcf: %f, pppl: %d",
                  newTime, newGain, mCamIA_DyCfg.sensor_mode.pixel_clock_freq_mhz,
                  mCamIA_DyCfg.sensor_mode.pixel_periods_per_line);
	  setExposure(mDevFd, newTime, newGain, 100);
        }
#endif
        AecMeasuringMode_to_cifisp_exp_meas_mode(
            ia_results->aec.meas_mode,
            &isp_cfg->configs.aec_config.mode);
        isp_cfg->configs.aec_config.autostop =
            CIFISP_EXP_CTRL_AUTOSTOP_0;

        isp_cfg->configs.aec_config.meas_window.h_size = ia_results->aec.meas_win.h_size;
        isp_cfg->configs.aec_config.meas_window.v_size = ia_results->aec.meas_win.v_size;
        isp_cfg->configs.aec_config.meas_window.h_offs = ia_results->aec.meas_win.h_offs;
        isp_cfg->configs.aec_config.meas_window.v_offs = ia_results->aec.meas_win.v_offs;

		if(ia_results->aec.actives & CAMIA10_AEC_AFPS_MASK)
	    {
	        SetAutoAdjustFps(mDevFd, ia_results->aec.auto_adjust_fps);
	    }
        isp_cfg->active_configs |= ISP_AEC_MASK;
        isp_cfg->enabled[HAL_ISP_AEC_ID] = ia_results->aec_enabled;
        LOGV("%s:aec mode : %d", __func__,
                ia_results->aec.meas_mode);
      }
      if (
          ia_results->aec.actives & CAMIA10_HST_MASK) {
        isp_cfg->active_configs |= ISP_HST_MASK;
        isp_cfg->enabled[HAL_ISP_HST_ID] = ia_results->hst.enabled;
        isp_cfg->configs.hst_config.mode = (cifisp_histogram_mode)
                                           (ia_results->hst.mode);

        isp_cfg->configs.hst_config.meas_window.h_size = ia_results->hst.Window.width;
        isp_cfg->configs.hst_config.meas_window.v_size = ia_results->hst.Window.height;
        isp_cfg->configs.hst_config.meas_window.h_offs = ia_results->hst.Window.hOffset;
        isp_cfg->configs.hst_config.meas_window.v_offs = ia_results->hst.Window.vOffset;

        memcpy(isp_cfg->configs.hst_config.hist_weight,
               ia_results->hst.Weights, sizeof(isp_cfg->configs.hst_config.hist_weight));
        LOGV("step size: %d, w-h: %d-%d\n",
                ia_results->aec.stepSize, isp_cfg->configs.hst_config.meas_window.h_size,
                isp_cfg->configs.hst_config.meas_window.v_size);
        isp_cfg->configs.hst_config.histogram_predivider = ia_results->hst.StepSize;

      }
    }
    if (ia_results->active & CAMIA10_AWB_GAIN_MASK) {
      isp_cfg->configs.awb_gain_config.gain_blue =
          ia_results->awb.awbGains.Blue;
      isp_cfg->configs.awb_gain_config.gain_green_b =
          ia_results->awb.awbGains.GreenB;
      isp_cfg->configs.awb_gain_config.gain_green_r =
          ia_results->awb.awbGains.GreenR;
      isp_cfg->configs.awb_gain_config.gain_red =
          ia_results->awb.awbGains.Red;
      isp_cfg->active_configs |= ISP_AWB_GAIN_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID] =
          ia_results->awb_gains_enabled;
      LOGD("AWB GAIN : enabled %d,BGbGrR(%d,%d,%d,%d)",
              isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID],
              isp_cfg->configs.awb_gain_config.gain_blue,
              isp_cfg->configs.awb_gain_config.gain_green_b,
              isp_cfg->configs.awb_gain_config.gain_green_r,
              isp_cfg->configs.awb_gain_config.gain_red);
    }

    if (ia_results->active & CAMIA10_CTK_MASK) {
      //if (ia_results->awb.actives & AWB_RECONFIG_CCMATRIX)
      {
        isp_cfg->configs.ctk_config.coeff0 =
            ia_results->awb.CcMatrix.Coeff[0];
        isp_cfg->configs.ctk_config.coeff1 =
            ia_results->awb.CcMatrix.Coeff[1];
        isp_cfg->configs.ctk_config.coeff2 =
            ia_results->awb.CcMatrix.Coeff[2];
        isp_cfg->configs.ctk_config.coeff3 =
            ia_results->awb.CcMatrix.Coeff[3];
        isp_cfg->configs.ctk_config.coeff4 =
            ia_results->awb.CcMatrix.Coeff[4];
        isp_cfg->configs.ctk_config.coeff5 =
            ia_results->awb.CcMatrix.Coeff[5];
        isp_cfg->configs.ctk_config.coeff6 =
            ia_results->awb.CcMatrix.Coeff[6];
        isp_cfg->configs.ctk_config.coeff7 =
            ia_results->awb.CcMatrix.Coeff[7];
        isp_cfg->configs.ctk_config.coeff8 =
            ia_results->awb.CcMatrix.Coeff[8];
        isp_cfg->active_configs |= ISP_CTK_MASK;
        isp_cfg->enabled[HAL_ISP_CTK_ID] =
            ia_results->ctk_enabled;
        LOGD("AWB CTK COEFF: enabled %d",
                isp_cfg->enabled[HAL_ISP_CTK_ID]);
        for (int i = 0; i < 9; i++)
          LOGD("-->COEFF[%d]:%d", i,
                  ia_results->awb.CcMatrix.Coeff[i]);
      }

      //if (ia_results->awb.actives & AWB_RECONFIG_CCOFFSET)
      {
        isp_cfg->configs.ctk_config.ct_offset_b =
            ia_results->awb.CcOffset.Blue;
        isp_cfg->configs.ctk_config.ct_offset_g =
            ia_results->awb.CcOffset.Green;
        isp_cfg->configs.ctk_config.ct_offset_r =
            ia_results->awb.CcOffset.Red;
        isp_cfg->active_configs |= ISP_CTK_MASK;
        isp_cfg->enabled[HAL_ISP_CTK_ID] =
            ia_results->ctk_enabled;
        LOGD("AWB CTK OFFSET: BGR(%d,%d,%d)",
                isp_cfg->configs.ctk_config.ct_offset_b,
                isp_cfg->configs.ctk_config.ct_offset_g,
                isp_cfg->configs.ctk_config.ct_offset_r);
      }
    }

    if (ia_results->active & CAMIA10_LSC_MASK) {
      //if (ia_results->awb.actives & AWB_RECONFIG_LSCMATRIX)
      {
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.r_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[0].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.gr_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[1].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.gb_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[2].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.b_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[3].uCoeff[i];
        isp_cfg->active_configs |= ISP_LSC_MASK;
        isp_cfg->enabled[HAL_ISP_LSC_ID] = ia_results->lsc_enabled;
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_LSCSECTOR)
      {
        for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.x_grad_tbl[i] =
              ia_results->awb.SectorConfig.LscXGradTbl[i];
        for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.x_size_tbl[i] =
              ia_results->awb.SectorConfig.LscXSizeTbl[i];
        for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.y_grad_tbl[i] =
              ia_results->awb.SectorConfig.LscYGradTbl[i];
        for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.y_size_tbl[i] =
              ia_results->awb.SectorConfig.LscYSizeTbl[i];
        isp_cfg->active_configs |= ISP_LSC_MASK;
        isp_cfg->enabled[HAL_ISP_LSC_ID] = ia_results->lsc_enabled;
      }
      //TODO: set size
      isp_cfg->configs.lsc_config.config_width =
          mCamIA_DyCfg.sensor_mode.isp_input_width;
      isp_cfg->configs.lsc_config.config_height =
          mCamIA_DyCfg.sensor_mode.isp_input_height;

      LOGV("AWB LSC: size(%dx%d),enabled %d",
              isp_cfg->configs.lsc_config.config_width,
              isp_cfg->configs.lsc_config.config_height,
              isp_cfg->enabled[HAL_ISP_LSC_ID]);
    }

    if (ia_results->active & CAMIA10_AWB_MEAS_MASK) {

      //if (ia_results->awb.actives & AWB_RECONFIG_MEASMODE)
      {
        if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_INVALID)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_MANUAL;
        else if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_YCBCR;
        else if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_RGB)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_RGB;
        else
          ALOGE("%s:%d,erro awb measure mode %d", __func__, __LINE__, ia_results->awb.MeasMode);
        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        LOGV("AWB MeasMode : %d,enabled: %d ",
                isp_cfg->configs.awb_meas_config.awb_mode,
                isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID]);
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_MEASCFG)
      {
        isp_cfg->configs.awb_meas_config.max_csum =
            ia_results->awb.MeasConfig.MaxCSum;
        isp_cfg->configs.awb_meas_config.max_y =
            ia_results->awb.MeasConfig.MaxY;
        isp_cfg->configs.awb_meas_config.min_y =
            ia_results->awb.MeasConfig.MinY_MaxG;
        isp_cfg->configs.awb_meas_config.min_c =
            ia_results->awb.MeasConfig.MinC;
        isp_cfg->configs.awb_meas_config.awb_ref_cr =
            ia_results->awb.MeasConfig.RefCr_MaxR;
        isp_cfg->configs.awb_meas_config.awb_ref_cb =
            ia_results->awb.MeasConfig.RefCb_MaxB;
        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        LOGV("AWB MEASCFG :");
        LOGV("-->max_csum:%d,max_y:%d,min_y:%d,MinC:%d,awb_ref_cr:%d,awb_ref_cb:%d",
                isp_cfg->configs.awb_meas_config.max_csum,
                isp_cfg->configs.awb_meas_config.max_y,
                isp_cfg->configs.awb_meas_config.min_y,
                isp_cfg->configs.awb_meas_config.min_c,
                isp_cfg->configs.awb_meas_config.awb_ref_cr,
                isp_cfg->configs.awb_meas_config.awb_ref_cb
               );
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_AWBWIN)
      {
        isp_cfg->configs.awb_meas_config.awb_wnd.h_size = ia_results->awb.awbWin.h_size;
        isp_cfg->configs.awb_meas_config.awb_wnd.v_size = ia_results->awb.awbWin.v_size;
        isp_cfg->configs.awb_meas_config.awb_wnd.h_offs = ia_results->awb.awbWin.h_offs;
        isp_cfg->configs.awb_meas_config.awb_wnd.v_offs = ia_results->awb.awbWin.v_offs;

        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        LOGV("AWB WINDOW:");
        LOGV("-->awb win:size:%dx%d(off:%dx%d)",
                isp_cfg->configs.awb_meas_config.awb_wnd.h_size,
                isp_cfg->configs.awb_meas_config.awb_wnd.v_size,
                isp_cfg->configs.awb_meas_config.awb_wnd.h_offs,
                isp_cfg->configs.awb_meas_config.awb_wnd.v_offs);
      }
      //TODO:
      isp_cfg->configs.awb_meas_config.frames = /*CIFISP_AWB_MAX_FRAMES*/0;
      LOGV("AWB FRAMES:%d", isp_cfg->configs.awb_meas_config.frames);
      // set enable_ymax_cmp default as true asked by cxf
      isp_cfg->configs.awb_meas_config.enable_ymax_cmp = true;
    }
    if (ia_results->active &
        (CAMIA10_DPF_MASK | CAMIA10_DPF_STRENGTH_MASK)) {

      if (ia_results->active & CAMIA10_DPF_MASK) {
        // enum cifisp_dpf_gain_usage defined in rkisp driver head in not
        // the same as in isp10 driver(old isp driver for RV1108) head
        #if defined(RKISP)
        isp_cfg->configs.dpf_config.gain.mode =
            (enum cifisp_dpf_gain_usage)(ia_results->adpf.DpfMode.GainUsage - 1);
        #else
        isp_cfg->configs.dpf_config.gain.mode =
            (enum cifisp_dpf_gain_usage)(ia_results->adpf.DpfMode.GainUsage);
        #endif
        isp_cfg->configs.dpf_config.gain.nf_b_gain =
            ia_results->adpf.NfGains.Blue;
        isp_cfg->configs.dpf_config.gain.nf_gr_gain =
            ia_results->adpf.NfGains.GreenR;
        isp_cfg->configs.dpf_config.gain.nf_gb_gain =
            ia_results->adpf.NfGains.GreenB;
        isp_cfg->configs.dpf_config.gain.nf_r_gain =
            ia_results->adpf.NfGains.Red;

        for (i = 0; i < CIFISP_DPF_MAX_NLF_COEFFS; i++) {
          isp_cfg->configs.dpf_config.nll.coeff[i] =
              ia_results->adpf.Nll.NllCoeff[i];
        }
        isp_cfg->configs.dpf_config.nll.scale_mode =
            (enum cifisp_dpf_nll_scale_mode)ia_results->adpf.Nll.xScale;

        isp_cfg->configs.dpf_config.g_flt.gb_enable =
            ia_results->adpf.DpfMode.ProcessGreenBPixel;
        isp_cfg->configs.dpf_config.g_flt.gr_enable =
            ia_results->adpf.DpfMode.ProcessGreenRPixel;
        isp_cfg->configs.dpf_config.rb_flt.r_enable =
            ia_results->adpf.DpfMode.ProcessRedPixel;
        isp_cfg->configs.dpf_config.rb_flt.b_enable =
            ia_results->adpf.DpfMode.ProcessBluePixel;
        isp_cfg->configs.dpf_config.rb_flt.fltsize =
            (enum cifisp_dpf_rb_filtersize)ia_results->adpf.DpfMode.RBFilterSize;
        for (i = 0; i < CIFISP_DPF_MAX_SPATIAL_COEFFS; i++) {
          isp_cfg->configs.dpf_config.g_flt.spatial_coeff[i] =
              ia_results->adpf.DpfMode.SpatialG.WeightCoeff[i];
          isp_cfg->configs.dpf_config.rb_flt.spatial_coeff[i] =
              ia_results->adpf.DpfMode.SpatialRB.WeightCoeff[i];
        }

        LOGV("%s: Gain: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                __func__,
                ia_results->adpf.DpfMode.GainUsage,
                ia_results->adpf.NfGains.Blue,
                ia_results->adpf.NfGains.GreenB,
                ia_results->adpf.NfGains.GreenR,
                ia_results->adpf.NfGains.Red);
        isp_cfg->active_configs |= ISP_DPF_MASK;
        isp_cfg->enabled[HAL_ISP_DPF_ID] =
            ia_results->adpf_enabled;
      }

      if (ia_results->active & CAMIA10_DPF_STRENGTH_MASK) {
        isp_cfg->configs.dpf_strength_config.b =
            ia_results->adpf.DynInvStrength.WeightB;
        isp_cfg->configs.dpf_strength_config.g =
            ia_results->adpf.DynInvStrength.WeightG;
        isp_cfg->configs.dpf_strength_config.r =
            ia_results->adpf.DynInvStrength.WeightR;
        isp_cfg->active_configs |= ISP_DPF_STRENGTH_MASK;
        isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID] =
            ia_results->adpf_strength_enabled;
      }
    }

    if (ia_results->active & CAMIA10_BPC_MASK) {
      isp_cfg->configs.dpcc_config.mode  = ia_results->dpcc.isp_dpcc_mode;
      isp_cfg->configs.dpcc_config.output_mode = ia_results->dpcc.isp_dpcc_output_mode;
      isp_cfg->configs.dpcc_config.set_use = ia_results->dpcc.isp_dpcc_set_use;
      isp_cfg->configs.dpcc_config.ro_limits = ia_results->dpcc.isp_dpcc_ro_limits;
      isp_cfg->configs.dpcc_config.rnd_offs = ia_results->dpcc.isp_dpcc_rnd_offs;
      isp_cfg->configs.dpcc_config.methods[0].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_1;
      isp_cfg->configs.dpcc_config.methods[0].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_1;
      isp_cfg->configs.dpcc_config.methods[0].method  =
          ia_results->dpcc.isp_dpcc_methods_set_1;
      isp_cfg->configs.dpcc_config.methods[0].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_1;
      isp_cfg->configs.dpcc_config.methods[0].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_1;
      isp_cfg->configs.dpcc_config.methods[0].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_1;

      isp_cfg->configs.dpcc_config.methods[1].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_2;
      isp_cfg->configs.dpcc_config.methods[1].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_2;
      isp_cfg->configs.dpcc_config.methods[1].method  =
          ia_results->dpcc.isp_dpcc_methods_set_2;
      isp_cfg->configs.dpcc_config.methods[1].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_2;
      isp_cfg->configs.dpcc_config.methods[1].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_2;
      isp_cfg->configs.dpcc_config.methods[1].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_2;

      isp_cfg->configs.dpcc_config.methods[2].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_3;
      isp_cfg->configs.dpcc_config.methods[2].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_3;
      isp_cfg->configs.dpcc_config.methods[2].method  =
          ia_results->dpcc.isp_dpcc_methods_set_3;
      isp_cfg->configs.dpcc_config.methods[2].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_3;
      isp_cfg->configs.dpcc_config.methods[2].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_3;
      isp_cfg->configs.dpcc_config.methods[2].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_3;

      isp_cfg->enabled[HAL_ISP_BPC_ID] = ia_results->dpcc.enabled;
      isp_cfg->active_configs |=  ISP_BPC_MASK;
    }

    if (ia_results->active & CAMIA10_BLS_MASK) {
      /*  not support AUTO mode now,just support fixed subtraction*/
      isp_cfg->configs.bls_config.enable_auto =
          false;
      isp_cfg->configs.bls_config.bls_samples =
          0;
      //isp_cfg->configs.bls_config.bls_window1 =
      //  ;
      memset(&isp_cfg->configs.bls_config.bls_window1,
             0, sizeof(struct cifisp_window));
      //isp_cfg->configs.bls_config.bls_window2 =
      //  ;
      memset(&isp_cfg->configs.bls_config.bls_window2,
             0, sizeof(struct cifisp_window));
      isp_cfg->configs.bls_config.en_windows =
          0;

      /* red */
      isp_cfg->configs.bls_config.fixed_val.r =
          ia_results->bls.isp_bls_a_fixed;
      /* greenR*/
      isp_cfg->configs.bls_config.fixed_val.gr =
          ia_results->bls.isp_bls_b_fixed;
      /* greenB*/
      isp_cfg->configs.bls_config.fixed_val.gb =
          ia_results->bls.isp_bls_c_fixed;
      /* blue*/
      isp_cfg->configs.bls_config.fixed_val.b =
          ia_results->bls.isp_bls_d_fixed;

      LOGV("BLS: RGrGbB:%d,%d,%d,%d",
              isp_cfg->configs.bls_config.fixed_val.r,
              isp_cfg->configs.bls_config.fixed_val.gr,
              isp_cfg->configs.bls_config.fixed_val.gb,
              isp_cfg->configs.bls_config.fixed_val.b);

      isp_cfg->enabled[HAL_ISP_BLS_ID] = ia_results->bls.enabled;
      isp_cfg->active_configs |=  ISP_BLS_MASK;
    }

    if (ia_results->active & CAMIA10_SDG_MASK) {
      int i = 0;
      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_r.gamma_y[i] =
            ia_results->sdg.red[i];

      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_g.gamma_y[i] =
            ia_results->sdg.green[i];

      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_b.gamma_y[i] =
            ia_results->sdg.blue[i];

      isp_cfg->configs.sdg_config.xa_pnts.gamma_dx0 = 0;
      isp_cfg->configs.sdg_config.xa_pnts.gamma_dx1 = 0;

      for (i = 0; i < (CIFISP_DEGAMMA_CURVE_SIZE - 1) ; i++) {
        if (i < (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2)
          isp_cfg->configs.sdg_config.xa_pnts.gamma_dx0 |=
              (uint32_t)(ia_results->sdg.segment[i]) << (i * 4);
        else {
          int index = i - (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2;
          isp_cfg->configs.sdg_config.xa_pnts.gamma_dx1 |=
              (uint32_t)(ia_results->sdg.segment[i]) << (index * 4);
        }
      }

      isp_cfg->enabled[HAL_ISP_SDG_ID] = ia_results->sdg.enabled;
      isp_cfg->active_configs |=  ISP_SDG_MASK;
    }

    if (ia_results->active & CAMIA10_FLT_MASK) {
      isp_cfg->configs.flt_config.chr_h_mode =
          ia_results->flt.chr_h_mode;
      isp_cfg->configs.flt_config.mode =
          (cifisp_flt_mode)(ia_results->flt.mode);
      isp_cfg->configs.flt_config.grn_stage1 =
          ia_results->flt.grn_stage1;
      isp_cfg->configs.flt_config.chr_v_mode =
          ia_results->flt.chr_v_mode;
      isp_cfg->configs.flt_config.thresh_bl0 =
          ia_results->flt.thresh_bl0;
      isp_cfg->configs.flt_config.thresh_bl1 =
          ia_results->flt.thresh_bl1;
      isp_cfg->configs.flt_config.thresh_sh0 =
          ia_results->flt.thresh_sh0;
      isp_cfg->configs.flt_config.thresh_sh1 =
          ia_results->flt.thresh_sh1;
      isp_cfg->configs.flt_config.lum_weight =
          ia_results->flt.lum_weight;
      isp_cfg->configs.flt_config.fac_sh1 =
          ia_results->flt.fac_sh1;
      isp_cfg->configs.flt_config.fac_sh0 =
          ia_results->flt.fac_sh0;
      isp_cfg->configs.flt_config.fac_mid =
          ia_results->flt.fac_mid;
      isp_cfg->configs.flt_config.fac_bl0 =
          ia_results->flt.fac_bl0;
      isp_cfg->configs.flt_config.fac_bl1 =
          ia_results->flt.fac_bl1;
      isp_cfg->configs.flt_denoise_level =
          ia_results->flt.denoise_level;
      isp_cfg->configs.flt_sharp_level =
          ia_results->flt.sharp_level;
      LOGV("FLT--> \n,\
				chr_h_mode:0x%x\n \
				mode	  :0x%x\n \
				grn_stage1:0x%x\n \
				chr_v_mode:0x%x\n \
				thresh_bl0:0x%x\n \
				thresh_bl1:0x%x\n \
				thresh_sh0:0x%x\n \
				thresh_sh1:0x%x\n \
				lum_weight:0x%x\n \
				fac_sh1	  :0x%x\n \
				fac_sh0	  :0x%x\n \
				fac_mid	  :0x%x\n \
				fac_bl0	  :0x%x\n \
				fac_bl1	  :0x%x\n",
              isp_cfg->configs.flt_config.chr_h_mode,
              isp_cfg->configs.flt_config.mode,
              isp_cfg->configs.flt_config.grn_stage1,
              isp_cfg->configs.flt_config.chr_v_mode,
              isp_cfg->configs.flt_config.thresh_bl0,
              isp_cfg->configs.flt_config.thresh_bl1,
              isp_cfg->configs.flt_config.thresh_sh0,
              isp_cfg->configs.flt_config.thresh_sh1,
              isp_cfg->configs.flt_config.lum_weight,
              isp_cfg->configs.flt_config.fac_sh1,
              isp_cfg->configs.flt_config.fac_sh0,
              isp_cfg->configs.flt_config.fac_mid,
              isp_cfg->configs.flt_config.fac_bl0,
              isp_cfg->configs.flt_config.fac_bl1);
      isp_cfg->enabled[HAL_ISP_FLT_ID] = ia_results->flt.enabled;
      isp_cfg->active_configs |=  ISP_FLT_MASK;
    }

    if (ia_results->active & CAMIA10_BDM_MASK) {
      isp_cfg->configs.bdm_config.demosaic_th =
          ia_results->bdm.demosaic_th;

      isp_cfg->enabled[HAL_ISP_BDM_ID] = ia_results->bdm.enabled;
      isp_cfg->active_configs |=  ISP_BDM_MASK;
    }

    if (ia_results->active & CAMIA10_GOC_MASK) {
      if (ia_results->goc.mode  == CAMERIC_ISP_SEGMENTATION_MODE_LOGARITHMIC)
        isp_cfg->configs.goc_config.mode =
            CIFISP_GOC_MODE_LOGARITHMIC;
      else if (ia_results->goc.mode  == CAMERIC_ISP_SEGMENTATION_MODE_EQUIDISTANT)
        isp_cfg->configs.goc_config.mode =
            CIFISP_GOC_MODE_EQUIDISTANT;
      else
        ALOGE("%s: not support %d goc mode.",
              __func__, ia_results->goc.mode);
      for (int i = 0; i < CIFISP_GAMMA_OUT_MAX_SAMPLES; i++) {
        isp_cfg->configs.goc_config.gamma_y[i] =
            ia_results->goc.gamma_y.GammaY[i];
      }

      isp_cfg->enabled[HAL_ISP_GOC_ID] = ia_results->goc.enabled;
      isp_cfg->active_configs |=  ISP_GOC_MASK;
    }

    if (ia_results->active & CAMIA10_CPROC_MASK) {
      isp_cfg->configs.cproc_config.brightness =
          ia_results->cproc.brightness;
      isp_cfg->configs.cproc_config.contrast =
          ia_results->cproc.contrast;
      isp_cfg->configs.cproc_config.sat =
          ia_results->cproc.saturation;
      isp_cfg->configs.cproc_config.hue =
          ia_results->cproc.hue;
      isp_cfg->configs.cproc_config.c_out_range =
          ia_results->cproc.ChromaOut;
      isp_cfg->configs.cproc_config.y_in_range =
          ia_results->cproc.LumaIn;
      isp_cfg->configs.cproc_config.y_out_range =
          ia_results->cproc.LumaOut;

      isp_cfg->enabled[HAL_ISP_CPROC_ID] = ia_results->cproc.enabled;
      isp_cfg->active_configs |=  ISP_CPROC_MASK;
    }

    if (ia_results->active & CAMIA10_IE_MASK) {
      isp_cfg->enabled[HAL_ISP_IE_ID] = ia_results->ie.enabled;
      isp_cfg->active_configs |=  ISP_IE_MASK;

      switch (ia_results->ie.mode) {
        case CAMERIC_IE_MODE_GRAYSCALE:
          isp_cfg->configs.ie_config.effect =
              V4L2_COLORFX_BW;
          break;
        case CAMERIC_IE_MODE_NEGATIVE:
          isp_cfg->configs.ie_config.effect =
              V4L2_COLORFX_NEGATIVE;
          break;
        case CAMERIC_IE_MODE_SEPIA: {
          isp_cfg->configs.ie_config.effect =
              V4L2_COLORFX_SEPIA;
          /*
          isp_cfg->configs.ie_config.color_sel =
            ia_results->ie.;
          isp_cfg->configs.ie_config.eff_mat_1 =
            ;
          isp_cfg->configs.ie_config.eff_mat_2 =
            ;
          isp_cfg->configs.ie_config.eff_mat_3 =
            ;
          isp_cfg->configs.ie_config.eff_mat_4 =
            ;
          isp_cfg->configs.ie_config.eff_mat_5 =
            ;
          isp_cfg->configs.ie_config.eff_tint =
            ;
          */
        }
        break;
        case CAMERIC_IE_MODE_EMBOSS: {
          isp_cfg->configs.ie_config.effect =
              V4L2_COLORFX_EMBOSS;
          isp_cfg->configs.ie_config.eff_mat_1 =
              (uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[0])
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[1]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[2]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[3]) << 0xc);
          isp_cfg->configs.ie_config.eff_mat_2 =
              (uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[4])
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[5]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[6]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[7]) << 0xc);
          isp_cfg->configs.ie_config.eff_mat_3 =
              (ia_results->ie.ModeConfig.Emboss.coeff[8]);
          /*not used for this effect*/
          isp_cfg->configs.ie_config.eff_mat_4 =
              0;
          isp_cfg->configs.ie_config.eff_mat_5 =
              0;
          isp_cfg->configs.ie_config.color_sel =
              0;
          isp_cfg->configs.ie_config.eff_tint =
              0;
        }
        break;
        case CAMERIC_IE_MODE_SKETCH: {
          isp_cfg->configs.ie_config.effect =
              V4L2_COLORFX_SKETCH;
          isp_cfg->configs.ie_config.eff_mat_3 =
              ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[0]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[1]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[2]) << 0xc);
          /*not used for this effect*/
          isp_cfg->configs.ie_config.eff_mat_4 =
              (uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[3])
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[4]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[5]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[6]) << 0xc);
          isp_cfg->configs.ie_config.eff_mat_5 =
              (uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[7])
              | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[8]) << 0x4);

          /*not used for this effect*/
          isp_cfg->configs.ie_config.eff_mat_1 = 0;
          isp_cfg->configs.ie_config.eff_mat_2 = 0;
          isp_cfg->configs.ie_config.color_sel =
              0;
          isp_cfg->configs.ie_config.eff_tint =
              0;
        }
        break;
        case CAMERIC_IE_MODE_SHARPEN: {
          /* TODO: can't find related mode in v4l2_colorfx*/
          //isp_cfg->configs.ie_config.effect =
          //  V4L2_COLORFX_EMBOSS;
          isp_cfg->configs.ie_config.eff_mat_3 =
              ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[0]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[1]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[2]) << 0xc);
          isp_cfg->configs.ie_config.eff_mat_4 =
              (uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[3])
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[4]) << 0x4)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[5]) << 0x8)
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[6]) << 0xc);
          isp_cfg->configs.ie_config.eff_mat_5 =
              (uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[7])
              | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[8]) << 0x4);
		  
          /*not used for this effect*/
          isp_cfg->configs.ie_config.eff_mat_1 =
              0;
          isp_cfg->configs.ie_config.eff_mat_2 =
              0;
          isp_cfg->configs.ie_config.color_sel =
              0;
          isp_cfg->configs.ie_config.eff_tint =
              0;
        }
        break;
        default: {
          ALOGE("%s: set ie mode failed %d", __FUNCTION__,
                ia_results->ie.mode);
          if (ia_results->ie.enabled == BOOL_TRUE)
            isp_cfg->active_configs &=  ~ISP_IE_MASK;
        }
      }

    }

    /*  TODOS */
    if (ia_results->active & CAMIA10_AFC_MASK) {
       isp_cfg->active_configs |= ISP_AFC_MASK;
       isp_cfg->enabled[HAL_ISP_AFC_ID] = ia_results->afc_meas_enabled;
       for (int i=0; i < ia_results->af.afc_config.num_afm_win; i++) {
           isp_cfg->configs.afc_config.afm_win[i].h_offs = ia_results->af.afc_config.afm_win[i].h_offset;
           isp_cfg->configs.afc_config.afm_win[i].v_offs = ia_results->af.afc_config.afm_win[i].v_offset;
           isp_cfg->configs.afc_config.afm_win[i].h_size = ia_results->af.afc_config.afm_win[i].width;
           isp_cfg->configs.afc_config.afm_win[i].v_size = ia_results->af.afc_config.afm_win[i].height;
       }

       isp_cfg->configs.afc_config.num_afm_win = ia_results->af.afc_config.num_afm_win;
       isp_cfg->configs.afc_config.thres = ia_results->af.afc_config.thres;
       isp_cfg->configs.afc_config.var_shift = ia_results->af.afc_config.var_shift;
/*
       LOGD("%s: set afm_win[0]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
                isp_cfg->configs.afc_config.afm_win[0].h_offs,
               isp_cfg->configs.afc_config.afm_win[0].v_offs,
               isp_cfg->configs.afc_config.afm_win[0].h_size,
               isp_cfg->configs.afc_config.afm_win[0].v_size);
       LOGD("%s: set afm_win[1]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
                isp_cfg->configs.afc_config.afm_win[1].h_offs,
               isp_cfg->configs.afc_config.afm_win[1].v_offs,
               isp_cfg->configs.afc_config.afm_win[1].h_size,
               isp_cfg->configs.afc_config.afm_win[1].v_size);
       LOGD("%s: set afm_win[2]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
                isp_cfg->configs.afc_config.afm_win[2].h_offs,
               isp_cfg->configs.afc_config.afm_win[2].v_offs,
               isp_cfg->configs.afc_config.afm_win[2].h_size,
               isp_cfg->configs.afc_config.afm_win[2].v_size);
*/
    }

	if (ia_results->active & CAMIA10_WDR_MASK) {
	  //CameraIcWdrConfig_t to struct cifisp_wdr_config
	  int regi = 0, i = 0;
	  isp_cfg->enabled[HAL_ISP_WDR_ID] = ia_results->wdr.enabled;
	  isp_cfg->active_configs |=  ISP_WDR_MASK;
	  if (ia_results->wdr.mode == CAMERIC_WDR_MODE_BLOCK)
		isp_cfg->configs.wdr_config.mode =
			CIFISP_WDR_MODE_BLOCK;
	  else
		isp_cfg->configs.wdr_config.mode =
			CIFISP_WDR_MODE_GLOBAL;
	  //TODO
	  /*offset 0x2a00*/
	  isp_cfg->configs.wdr_config.c_wdr[0] = 0x00000812;
	  /* offset 0x2a04 - 0x2a10*/

	  for (regi = 1; regi < 5; regi++) {
		isp_cfg->configs.wdr_config.c_wdr[regi] = 0;
		for (int i = 0; i < 8; i++)
		  isp_cfg->configs.wdr_config.c_wdr[regi] |=
			  (uint32_t)(ia_results->wdr.segment \
						 [i + (regi - 1) * 8 ]) << (4 * i);
	  }

	  /*offset 0x2a14 - 0x2a94*/
	  for (regi = 5; regi < 38; regi++)
		isp_cfg->configs.wdr_config.c_wdr[regi] =
			((uint32_t)(ia_results->wdr.wdr_block_y[regi - 5]) << 16) |
			(uint32_t)(ia_results->wdr.wdr_global_y[regi - 5]);
	  /*offset 0x2a98 - 0x2a9c*/
	  isp_cfg->configs.wdr_config.c_wdr[38] = 0x0;
	  isp_cfg->configs.wdr_config.c_wdr[39] = 0x0;
#if 0
      /*offset 0x2b50 - 0x2b6c*/
      isp_cfg->configs.wdr_config.c_wdr[40] = 0x00030cf0;
      isp_cfg->configs.wdr_config.c_wdr[41] = 0x000140d3;
      isp_cfg->configs.wdr_config.c_wdr[42] = 0x000000cd;
      isp_cfg->configs.wdr_config.c_wdr[43] = 0x0ccc00ee;
      isp_cfg->configs.wdr_config.c_wdr[44] = 0x00000036;
      isp_cfg->configs.wdr_config.c_wdr[45] = 0x000000b7;
      isp_cfg->configs.wdr_config.c_wdr[46] = 0x00000012;
      isp_cfg->configs.wdr_config.c_wdr[47] = 0x0;
#else
	  isp_cfg->configs.wdr_config.c_wdr[40] =
		  ((uint32_t)(ia_results->wdr.wdr_pym_cc) << 16) |
		  ((uint32_t)(ia_results->wdr.wdr_epsilon) << 8) |
		  ((uint32_t)(ia_results->wdr.wdr_lvl_en) << 4) ;

	  isp_cfg->configs.wdr_config.c_wdr[41] =
		  ((uint32_t)(ia_results->wdr.wdr_gain_max_clip_enable) << 16) |
		  ((uint32_t)(ia_results->wdr.wdr_gain_max_value) << 8) |
		  ((uint32_t)(ia_results->wdr.wdr_bavg_clip) << 6) |
		  ((uint32_t)(ia_results->wdr.wdr_nonl_segm) << 5) |
		  ((uint32_t)(ia_results->wdr.wdr_nonl_open) << 4) |
		  ((uint32_t)(ia_results->wdr.wdr_nonl_mode1) << 3) |
		  ((uint32_t)(ia_results->wdr.wdr_flt_sel) << 1) |
		  ia_results->wdr.mode ;

	  isp_cfg->configs.wdr_config.c_wdr[42] = ia_results->wdr.wdr_gain_off1;
	  isp_cfg->configs.wdr_config.c_wdr[43] =
		  ((uint32_t)(ia_results->wdr.wdr_bestlight) << 16) |
		  (uint32_t)(ia_results->wdr.wdr_noiseratio);
	  isp_cfg->configs.wdr_config.c_wdr[44] = ia_results->wdr.wdr_coe0;
	  isp_cfg->configs.wdr_config.c_wdr[45] =  ia_results->wdr.wdr_coe1;
	  isp_cfg->configs.wdr_config.c_wdr[46] =  ia_results->wdr.wdr_coe2;
	  isp_cfg->configs.wdr_config.c_wdr[47] =  ia_results->wdr.wdr_coe_off;
#endif
	  for (i = 0; i < 48; i++)
		LOGD( "WDR[%d] = 0x%x", i, isp_cfg->configs.wdr_config.c_wdr[i]);

	  LOGD( "WDR = 0x%x", ia_results->wdr.wdr_gain_max_value);
	}

	
	if (ia_results->active & CAMIA10_DSP_3DNR_MASK) {
	  isp_cfg->active_configs |= ISP_DSP_3DNR_MASK;
	  isp_cfg->configs.Dsp3DnrSetConfig = ia_results->adpf.Dsp3DnrResult;

	  LOGD( "ConvertIA luma_sp:%d %d luma_te:%d %d chrm_sp:%d %d chrm_te:%d %d shp:%d %d noise:(%d/%d)",
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_sp_nr_en,
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_sp_nr_level,
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_te_nr_en,
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_te_nr_level,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_sp_nr_en,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_sp_nr_level,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_te_nr_en,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_te_nr_level,
			  isp_cfg->configs.Dsp3DnrSetConfig.shp_en,
			  isp_cfg->configs.Dsp3DnrSetConfig.shp_level,
			  isp_cfg->configs.Dsp3DnrSetConfig.noise_coef_num,
			  isp_cfg->configs.Dsp3DnrSetConfig.noise_coef_den);

	  LOGD( "ConvertIA setting luma:%d 0x%x  chrm:%d 0x%x shp:%d 0x%x",
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_default,
			  isp_cfg->configs.Dsp3DnrSetConfig.luma_w2,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_default,
			  isp_cfg->configs.Dsp3DnrSetConfig.chrm_w2,
			  isp_cfg->configs.Dsp3DnrSetConfig.shp_default,
			  isp_cfg->configs.Dsp3DnrSetConfig.src_shp_w2);
	}

	if (ia_results->active & CAMIA10_NEW_DSP_3DNR_MASK) 
	{
	  isp_cfg->active_configs |= ISP_NEW_DSP_3DNR_MASK;
	  isp_cfg->configs.NewDsp3DnrSetConfig = ia_results->adpf.NewDsp3DnrResult;

	  LOGD( "ConvertIA setting 3dnr_en:%d dpc_en:%d ynr_en:%d tnr_en:%d iir_en:%d uvnr_en:%d shrp_en:%d",
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_3dnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_dpc,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_ynr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_tnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_iir,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_uvnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_sharp);
		  
	  LOGD( "ConvertIA setting ynr_time_weight:%d ynr_spat_weight:%d uvnr:%d sharp:%d ",
			  isp_cfg->configs.NewDsp3DnrSetConfig.ynr_time_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.ynr_spat_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.uvnr_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.sharp_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_dpc);
	}

	if (ia_results->active & CAMIA10_DEMOSAICLP_MASK) 
	{
		isp_cfg->enabled[HAL_ISP_DEMOSAICLP_ID] = (bool_t)ia_results->rkDemosaicLP.lp_en;
		isp_cfg->active_configs |= ISP_RK_DEMOSAICLP_MASK;
		isp_cfg->configs.demosaicLp_config.hp_filter_en = ia_results->rkDemosaicLP.hp_filter_en;
		isp_cfg->configs.demosaicLp_config.rb_filter_en = ia_results->rkDemosaicLP.rb_filter_en;
		isp_cfg->configs.demosaicLp_config.use_old_lp = ia_results->rkDemosaicLP.use_old_lp;
		memcpy(isp_cfg->configs.demosaicLp_config.lu_divided ,
				ia_results->rkDemosaicLP.lu_divided,
				sizeof(ia_results->rkDemosaicLP.lu_divided));

		memcpy(isp_cfg->configs.demosaicLp_config.thgrad_divided ,
				ia_results->rkDemosaicLP.thgrad_divided,
				sizeof(ia_results->rkDemosaicLP.thgrad_divided));

		memcpy(isp_cfg->configs.demosaicLp_config.thdiff_divided ,
				ia_results->rkDemosaicLP.thdiff_divided,
				sizeof(ia_results->rkDemosaicLP.thdiff_divided));

		memcpy(isp_cfg->configs.demosaicLp_config.thcsc_divided ,
				ia_results->rkDemosaicLP.thcsc_divided,
				sizeof(ia_results->rkDemosaicLP.thcsc_divided));

		memcpy(isp_cfg->configs.demosaicLp_config.thvar_divided ,
				ia_results->rkDemosaicLP.thvar_divided,
				sizeof(ia_results->rkDemosaicLP.thvar_divided));
		
		isp_cfg->configs.demosaicLp_config.th_grad = ia_results->rkDemosaicLP.th_grad;
		isp_cfg->configs.demosaicLp_config.th_diff = ia_results->rkDemosaicLP.th_diff;
		isp_cfg->configs.demosaicLp_config.th_csc = ia_results->rkDemosaicLP.th_csc;
		isp_cfg->configs.demosaicLp_config.th_var = ia_results->rkDemosaicLP.th_var;
		isp_cfg->configs.demosaicLp_config.th_grad_en = ia_results->rkDemosaicLP.th_grad_en;
		isp_cfg->configs.demosaicLp_config.th_diff_en = ia_results->rkDemosaicLP.th_diff_en;
		isp_cfg->configs.demosaicLp_config.th_csc_en = ia_results->rkDemosaicLP.th_csc_en;
		isp_cfg->configs.demosaicLp_config.th_var_en = ia_results->rkDemosaicLP.th_var_en;
		isp_cfg->configs.demosaicLp_config.flat_level_sel = ia_results->rkDemosaicLP.flat_level_sel;
		isp_cfg->configs.demosaicLp_config.pattern_level_sel = ia_results->rkDemosaicLP.pattern_level_sel;
		isp_cfg->configs.demosaicLp_config.edge_level_sel = ia_results->rkDemosaicLP.edge_level_sel;
		isp_cfg->configs.demosaicLp_config.similarity_th = ia_results->rkDemosaicLP.similarity_th;
		isp_cfg->configs.demosaicLp_config.thgrad_r_fct = ia_results->rkDemosaicLP.thgrad_r_fct;
		isp_cfg->configs.demosaicLp_config.thdiff_r_fct = ia_results->rkDemosaicLP.thdiff_r_fct;
		isp_cfg->configs.demosaicLp_config.thvar_r_fct = ia_results->rkDemosaicLP.thvar_r_fct;
		isp_cfg->configs.demosaicLp_config.thgrad_b_fct = ia_results->rkDemosaicLP.thgrad_b_fct;
		isp_cfg->configs.demosaicLp_config.thdiff_b_fct = ia_results->rkDemosaicLP.thdiff_b_fct;
		isp_cfg->configs.demosaicLp_config.thvar_b_fct = ia_results->rkDemosaicLP.thvar_b_fct;			
	}
	
	if (ia_results->active & CAMIA10_RKIESHARP_MASK) {
		isp_cfg->enabled[HAL_ISP_RKIESHARP_ID] = (bool_t)ia_results->rkIEsharp.iesharpen_en;
		isp_cfg->active_configs |= ISP_RK_IESHARP_MASK;
		isp_cfg->configs.rkIESharp_config.coring_thr = ia_results->rkIEsharp.coring_thr; 
		isp_cfg->configs.rkIESharp_config.full_range = ia_results->rkIEsharp.full_range; 
		isp_cfg->configs.rkIESharp_config.switch_avg = ia_results->rkIEsharp.switch_avg; 
		memcpy(isp_cfg->configs.rkIESharp_config.yavg_thr,
				ia_results->rkIEsharp.yavg_thr,
				sizeof(ia_results->rkIEsharp.yavg_thr));
		memcpy(isp_cfg->configs.rkIESharp_config.delta1,
				ia_results->rkIEsharp.delta1,
				sizeof(ia_results->rkIEsharp.delta1));
		memcpy(isp_cfg->configs.rkIESharp_config.delta2,
				ia_results->rkIEsharp.delta2,
				sizeof(ia_results->rkIEsharp.delta2));
		memcpy(isp_cfg->configs.rkIESharp_config.maxnumber,
				ia_results->rkIEsharp.maxnumber,
				sizeof(ia_results->rkIEsharp.maxnumber));
		memcpy(isp_cfg->configs.rkIESharp_config.minnumber,
				ia_results->rkIEsharp.minnumber,
				sizeof(ia_results->rkIEsharp.minnumber));
		memcpy(isp_cfg->configs.rkIESharp_config.gauss_flat_coe,
				ia_results->rkIEsharp.gauss_flat_coe,
				sizeof(ia_results->rkIEsharp.gauss_flat_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.gauss_noise_coe,
				ia_results->rkIEsharp.gauss_noise_coe,
				sizeof(ia_results->rkIEsharp.gauss_noise_coe));
		
		memcpy(isp_cfg->configs.rkIESharp_config.gauss_other_coe,
				ia_results->rkIEsharp.gauss_other_coe,
				sizeof(ia_results->rkIEsharp.gauss_other_coe));
		
		memcpy(isp_cfg->configs.rkIESharp_config.uv_gauss_flat_coe,
				ia_results->rkIEsharp.uv_gauss_flat_coe,
				sizeof(ia_results->rkIEsharp.uv_gauss_flat_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.uv_gauss_noise_coe,
				ia_results->rkIEsharp.uv_gauss_noise_coe,
				sizeof(ia_results->rkIEsharp.uv_gauss_noise_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.uv_gauss_other_coe,
				ia_results->rkIEsharp.uv_gauss_other_coe,
				sizeof(ia_results->rkIEsharp.uv_gauss_other_coe));
		
   		memcpy(isp_cfg->configs.rkIESharp_config.grad_seq, 
						ia_results->rkIEsharp.p_grad,
						sizeof(ia_results->rkIEsharp.p_grad));
		
		memcpy(isp_cfg->configs.rkIESharp_config.sharp_factor,
						ia_results->rkIEsharp.sharp_factor,
						sizeof(ia_results->rkIEsharp.sharp_factor));
		
		memcpy(isp_cfg->configs.rkIESharp_config.line1_filter_coe,
						ia_results->rkIEsharp.line1_filter_coe,
						sizeof(ia_results->rkIEsharp.line1_filter_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.line2_filter_coe,
						ia_results->rkIEsharp.line2_filter_coe,
						sizeof(ia_results->rkIEsharp.line2_filter_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.line3_filter_coe,
						ia_results->rkIEsharp.line3_filter_coe,
						sizeof(ia_results->rkIEsharp.line3_filter_coe));

		memcpy(isp_cfg->configs.rkIESharp_config.lap_mat_coe,
				ia_results->rkIEsharp.lap_mat_coe,
				sizeof(ia_results->rkIEsharp.lap_mat_coe));
	}
  }

  return true;
}

bool Isp10Engine::initISPStream(const char* ispDev) {
  unsigned int i;

  IspEngine::initISPStream(ispDev);

  return true;
}

bool Isp10Engine::getSensorModedata
(
    struct isp_supplemental_sensor_mode_data* drvCfg,
    CamIA10_SensorModeData* iaCfg
) {
  //LOGD("-------getSensorModedata----------W-H: %d-%d", drvCfg->isp_input_width, drvCfg->isp_input_height);

  //iaCfg->isp_input_width = drvCfg->isp_input_width;
  //iaCfg->isp_input_height = drvCfg->isp_input_height;
  iaCfg->pixel_clock_freq_mhz = drvCfg->vt_pix_clk_freq_hz / 1000000.0f;
  iaCfg->horizontal_crop_offset = drvCfg->crop_horizontal_start;
  iaCfg->vertical_crop_offset = drvCfg->crop_vertical_start;
  iaCfg->cropped_image_width = drvCfg->crop_horizontal_end - drvCfg->crop_horizontal_start + 1;
  iaCfg->cropped_image_height = drvCfg->crop_vertical_end - drvCfg->crop_vertical_start + 1;
  iaCfg->pixel_periods_per_line = drvCfg->line_length_pck;
  iaCfg->line_periods_per_field = drvCfg->frame_length_lines;
  iaCfg->sensor_output_height = drvCfg->sensor_output_height;
  iaCfg->sensor_output_width = drvCfg->sensor_output_width;
  iaCfg->isp_input_width = drvCfg->isp_input_width;
  iaCfg->isp_input_height = drvCfg->isp_input_height;
  iaCfg->isp_output_width = drvCfg->isp_output_width;
  iaCfg->isp_output_height = drvCfg->isp_output_height;
  iaCfg->fine_integration_time_min = drvCfg->fine_integration_time_min;
  iaCfg->fine_integration_time_max_margin = drvCfg->line_length_pck - drvCfg->fine_integration_time_max_margin;
  iaCfg->coarse_integration_time_min = drvCfg->coarse_integration_time_min;
  iaCfg->coarse_integration_time_max_margin = drvCfg->coarse_integration_time_max_margin;
  iaCfg->exp_time = drvCfg->exp_time;
  iaCfg->gain = drvCfg->gain;
  iaCfg->exp_time_seconds = drvCfg->exp_time_seconds;
  iaCfg->gains = drvCfg->gains;
  iaCfg->exposure_valid_frame = drvCfg->exposure_valid_frame[0];
  iaCfg->is_bw_sensor = drvCfg->is_bw_sensor;

  //LOGD("%s:iaCfg->pixel_clock_freq_mhz %f",__func__,iaCfg->pixel_clock_freq_mhz );
  //LOGD("%s:iaCfg->gain %d",__func__,iaCfg->gain );
  //LOGD("%s:iaCfg->exp_time %d",__func__,iaCfg->exp_time );
  return true;
}

void Isp10Engine::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
) {
#if 0
  struct v4l2_buffer_metadata_s* v4l2Meta =
      (struct v4l2_buffer_metadata_s*)drvMeta;
  struct cifisp_isp_metadata* ispMetaData =
      (struct cifisp_isp_metadata*)v4l2Meta->isp;
  halMeta->timStamp = v4l2Meta->frame_t.vs_t;
  if (ispMetaData) {
    LOGV("%s:drv exp time gain %d %d",
            __func__,
            ispMetaData->meas_stat.sensor_mode.exp_time,
            ispMetaData->meas_stat.sensor_mode.gain
           );
    if (mCamIAEngine.get())
      mCamIAEngine->mapSensorExpToHal
      (
          ispMetaData->meas_stat.sensor_mode.gain,
          ispMetaData->meas_stat.sensor_mode.exp_time,
          halMeta->exp_gain,
          halMeta->exp_time
      );
    else
      ALOGW("%s:mCamIAEngine has been desroyed!", __func__);
    halMeta->awb.wb_gain.gain_blue =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_blue);
    halMeta->awb.wb_gain.gain_green_b =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_green_b);
    halMeta->awb.wb_gain.gain_green_r =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_green_r);
    halMeta->awb.wb_gain.gain_red =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_red);
    halMeta->dpf_strength.b =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.b);
    halMeta->dpf_strength.g =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.g);
    halMeta->dpf_strength.r =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.r);
    /* FIXME: should be get flt info from drv meta data.*/
    halMeta->flt.denoise_level =
        mIspCfg.flt_denoise_level;
    halMeta->flt.sharp_level =
        mIspCfg.flt_sharp_level;
    memcpy(halMeta->enabled, mIspCfg.enabled, sizeof(mIspCfg.enabled));

  }
#endif
}

bool Isp10Engine::threadLoop() {
  unsigned int buf_index;
  struct cifisp_stat_buffer* buffer = NULL;
  struct v4l2_buffer v4l2_buf;
  struct CamIA10_Stats ia_stat;
  struct CamIA10_DyCfg ia_dcfg;
  struct CamIA10_Results ia_results;
  struct CamIsp10ConfigSet isp_cfg;

  memset(&ia_dcfg, 0, sizeof(ia_dcfg));
  memset(&ia_stat, 0, sizeof(ia_stat));
  memset(&ia_results, 0, sizeof(ia_results));
  memset(&isp_cfg, 0, sizeof(isp_cfg));
  //LOGD("%s: enter",__func__);

  if (!getMeasurement(v4l2_buf)) {
    ALOGE("%s: getMeasurement failed", __func__);
    return true;//false;
  }

  if (v4l2_buf.index >= CAM_ISP_NUM_OF_STAT_BUFS) {
    ALOGE("%s: v4l2_buf index: %d is invalidate!", __func__, v4l2_buf.index);
    return true;//false;
  }

  convertIspStats(mIspStats[v4l2_buf.index], &ia_stat);

  //get sensor mode data
  buffer = (struct cifisp_stat_buffer*)(mIspStats[v4l2_buf.index]);
#if RK_ISP10
  getSensorModedata(&(buffer->sensor_mode), &(mCamIA_DyCfg.sensor_mode));
#endif
  releaseMeasurement(&v4l2_buf);

  osMutexLock(&mApiLock);
  ia_dcfg = mCamIA_DyCfg;
  mCamIA_DyCfg.afc_cfg.oneshot_trigger = BOOL_FALSE;
  osMutexUnlock(&mApiLock);
  runIA(&ia_dcfg, &ia_stat, &ia_results);

  //run isp manual config??will override the 3A results
  if (!runISPManual(&ia_results, BOOL_TRUE))
    ALOGE("%s:run ISP manual failed!", __func__);

  convertIAResults(&isp_cfg, &ia_results);

  applyIspConfig(&isp_cfg);

  return true;
}


#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "isp_engine.h"
//#include "camHalTrace.h"

using namespace std;

#define CAMERA_ISP_DEV_NAME   "/dev/video1"
#define TUNNING_FILE_PATH  "/data/tunning.xml"

IspEngine::IspEngine():
    mInitialized(0)
{
    mStartCnt = 0;
    mStreaming = false;
    mIspFd = -1;
    mIspVer = 0;
    memset(&mCamIA_DyCfg, 0x00, sizeof(struct CamIA10_DyCfg));
    osMutexInit(&mApiLock);
}
IspEngine::~IspEngine()
{
    deInit();
    osMutexDestroy(&mApiLock);
}

bool IspEngine::init(const char* tuningFile,
                     const char* ispDev,
                     int isp_ver,
                     int devFd)
{
    return true;
}
bool IspEngine::deInit()
{
    osMutexLock(&mApiLock);

    if (mIspFd >= 0)
    {
        int ret;
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (mStreaming)
        {

            LOGD("%s: going to STREAMOFF", __func__);
            ret = ioctl(mIspFd, VIDIOC_STREAMOFF, &type);
            if (ret < 0)
            {
                ALOGE("%s: Failed to stop stream", __func__);
            }
        }

        for (int i = 0; i < CAM_ISP_NUM_OF_STAT_BUFS; i++)
        {
            if (mIspStatBuf[i])
            {
                munmap(mIspStatBuf[i], mIspStatBufSize);
                mIspStatBuf[i] = NULL;
            }
        }

        if (mIspFd >= 0)
            close(mIspFd);

        mIspStatBufSize = 0;
        mIspFd = -1;
        //mInitialized = false;
    }
    mCamIAEngine.reset();
    osMutexUnlock(&mApiLock);

    return true;

}

int IspEngine::setStatistics(struct CamIA10_Stats* ia_stats)
{
    mCamIAEngine->setStatistics(ia_stats);

	return 0;
}

int IspEngine::updateDynamicConfig(struct CamIA10_DyCfg* ia_dcfg)
{
    mCamIA_DyCfg = *ia_dcfg;

    mCamIAEngine->initDynamic(ia_dcfg);

    return 0;
}

int IspEngine::runAe(XCamAeParam *param, AecResult_t* result, bool first)
{
    int ret = mCamIAEngine->runAe(param, result, first);
    if (ret != 0)
        return ret;
    mCamIAEngine->runADPF();
    mCamIAEngine->runAWDR();

    return 0;
}

int IspEngine::runAwb(XCamAwbParam *param, CamIA10_AWB_Result_t* result, bool first)
{
    return mCamIAEngine->runAwb(param, result, first);
}

int IspEngine::runAf(XCamAfParam *param, XCam3aResultFocus* result, bool first)
{
    return mCamIAEngine->runAf(param, result, first);
}

bool IspEngine::getIAResult(struct CamIA10_Results* ia_results) {
    if (!ia_results) {
        return -1;
    }

    ia_results->active = 0;
    if (mCamIAEngine->getAECResults(&ia_results->aec) == RET_SUCCESS)
    {
        ia_results->active |= CAMIA10_AEC_MASK;
        ia_results->hst.enabled = BOOL_TRUE;
        //copy aec hst result to struct hst, may be override by manual settings after
        ia_results->hst.mode = CAMERIC_ISP_HIST_MODE_RGB_COMBINED;
        ia_results->hst.Window.width =
            ia_results->aec.meas_win.h_size;
        ia_results->hst.Window.height =
            ia_results->aec.meas_win.v_size;
        ia_results->hst.Window.hOffset =
            ia_results->aec.meas_win.h_offs;
        ia_results->hst.Window.vOffset =
            ia_results->aec.meas_win.v_offs;
        ia_results->hst.StepSize =
            ia_results->aec.stepSize;

        if (ia_results->aec.aoe_enable)
        {
            ia_results->hst.Weights[0] = 0;
            ia_results->hst.Weights[1] = 0;
            ia_results->hst.Weights[2] = 0;
            ia_results->hst.Weights[3] = 0;
            ia_results->hst.Weights[4] = 0;

            ia_results->hst.Weights[5] = 0;
            ia_results->hst.Weights[6] = 0;
            ia_results->hst.Weights[7] = 10;
            ia_results->hst.Weights[8] = 0;
            ia_results->hst.Weights[9] = 0;

            ia_results->hst.Weights[10] = 0;
            ia_results->hst.Weights[11] = 10;
            ia_results->hst.Weights[12] = 10;
            ia_results->hst.Weights[13] = 10;
            ia_results->hst.Weights[14] = 0;

            ia_results->hst.Weights[15] = 0;
            ia_results->hst.Weights[16] = 0;
            ia_results->hst.Weights[17] = 10;
            ia_results->hst.Weights[18] = 0;
            ia_results->hst.Weights[19] = 0;

            ia_results->hst.Weights[20] = 0;
            ia_results->hst.Weights[21] = 0;
            ia_results->hst.Weights[22] = 0;
            ia_results->hst.Weights[23] = 0;
            ia_results->hst.Weights[24] = 0;
        }
        else
        {
            /*
             * aec weights nums from aec algorithm may be greater than
             * hist hweights nums that will be set to ISP
             */
            memcpy(ia_results->hst.Weights,
                   ia_results->aec.GridWeights, sizeof(ia_results->hst.Weights));
        }


        ia_results->aec_enabled = BOOL_TRUE;
    }

    memset(&ia_results->awb, 0, sizeof(ia_results->awb));
    if (mCamIAEngine->getAWBResults(&ia_results->awb) == RET_SUCCESS)
    {
        if (ia_results->awb.actives & AWB_RECONFIG_GAINS)
            ia_results->active |= CAMIA10_AWB_GAIN_MASK;
        if ((ia_results->awb.actives & AWB_RECONFIG_CCMATRIX)
                || (ia_results->awb.actives & AWB_RECONFIG_CCOFFSET))
            ia_results->active |= CAMIA10_CTK_MASK;
        if ((ia_results->awb.actives & AWB_RECONFIG_LSCMATRIX)
                || (ia_results->awb.actives & AWB_RECONFIG_LSCSECTOR))
            ia_results->active |= CAMIA10_LSC_MASK;
        if ((ia_results->awb.actives & AWB_RECONFIG_MEASMODE)
                || (ia_results->awb.actives & AWB_RECONFIG_MEASCFG)
                || (ia_results->awb.actives & AWB_RECONFIG_AWBWIN))
            ia_results->active |= CAMIA10_AWB_MEAS_MASK;
        ia_results->awb_gains_enabled = BOOL_TRUE;
        ia_results->awb_meas_enabled = BOOL_TRUE;
        ia_results->lsc_enabled = BOOL_TRUE;
        ia_results->ctk_enabled = BOOL_TRUE;
    }

    if (mCamIAEngine->getADPFResults(&ia_results->adpf) == RET_SUCCESS)
    {
        if (ia_results->adpf.actives & ADPF_MASK)
        {
            ia_results->active |= CAMIA10_DPF_MASK;
            ia_results->adpf_enabled = BOOL_TRUE;
        }
        if (ia_results->adpf.actives & ADPF_STRENGTH_MASK)
        {
            ia_results->active |= CAMIA10_DPF_STRENGTH_MASK;
            ia_results->adpf_strength_enabled = BOOL_TRUE;
        }

        if (ia_results->adpf.actives & ADPF_DENOISE_SHARP_LEVEL_MASK)
        {
            flt_cfg.denoise_level = ia_results->adpf.denoise_level;
            flt_cfg.sharp_level = ia_results->adpf.sharp_level;
            flt_cfg.light_mode = mCamIA_DyCfg.LightMode;
            mFltEnabled = HAL_ISP_ACTIVE_SETTING;
            mFltNeededUpdate = BOOL_TRUE;
            runISPManual(ia_results, BOOL_FALSE);
            ia_results->flt.enabled = ia_results->adpf.FltEnable;
            ia_results->active |= CAMIA10_FLT_MASK;
        }

        if (ia_results->adpf.actives & ADPF_DEMOSAIC_TH_MASK)
        {
            bdm_cfg.demosaic_th = ia_results->adpf.demosaic_th;
            mBdmEnabled = HAL_ISP_ACTIVE_SETTING;
            mBdmNeededUpdate = BOOL_TRUE;
            runISPManual(ia_results, BOOL_FALSE);
            ia_results->bdm.enabled = BOOL_TRUE;
            ia_results->active |= CAMIA10_BDM_MASK;
        }

        if (ia_results->adpf.actives & ADPF_DSP_3DNR_MASK)
        {
            ia_results->active |= CAMIA10_DSP_3DNR_MASK;
        }

		if ((ia_results->adpf.actives & ADPF_NEW_DSP_3DNR_MASK) &&
	        (mNew3DnrEnabled == HAL_ISP_ACTIVE_DEFAULT)) {
	        ia_results->active |= CAMIA10_NEW_DSP_3DNR_MASK;
	    }
		
		if (ia_results->adpf.actives & ADPF_DEMOSAICLP_MASK)
        {
        	ia_results->rkDemosaicLP.lp_en = ia_results->adpf.RKDemosaicLpResult.lp_en;
			ia_results->rkDemosaicLP.rb_filter_en = ia_results->adpf.RKDemosaicLpResult.rb_filter_en;
			ia_results->rkDemosaicLP.hp_filter_en = ia_results->adpf.RKDemosaicLpResult.hp_filter_en;
			ia_results->rkDemosaicLP.th_grad = ia_results->adpf.RKDemosaicLpResult.th_grad;
			ia_results->rkDemosaicLP.th_diff = ia_results->adpf.RKDemosaicLpResult.th_diff;
			ia_results->rkDemosaicLP.th_csc = ia_results->adpf.RKDemosaicLpResult.th_csc;
			ia_results->rkDemosaicLP.th_var = ia_results->adpf.RKDemosaicLpResult.th_var;
			ia_results->rkDemosaicLP.th_var_en = ia_results->adpf.RKDemosaicLpResult.th_var_en;
			ia_results->rkDemosaicLP.th_csc_en = ia_results->adpf.RKDemosaicLpResult.th_csc_en;
			ia_results->rkDemosaicLP.th_diff_en = ia_results->adpf.RKDemosaicLpResult.th_diff_en;
			ia_results->rkDemosaicLP.th_grad_en = ia_results->adpf.RKDemosaicLpResult.th_grad_en;
			ia_results->rkDemosaicLP.use_old_lp = ia_results->adpf.RKDemosaicLpResult.use_old_lp;
			ia_results->rkDemosaicLP.similarity_th = ia_results->adpf.RKDemosaicLpResult.similarity_th;
			ia_results->rkDemosaicLP.flat_level_sel = ia_results->adpf.RKDemosaicLpResult.flat_level_sel;
			ia_results->rkDemosaicLP.pattern_level_sel = ia_results->adpf.RKDemosaicLpResult.pattern_level_sel;
			ia_results->rkDemosaicLP.edge_level_sel = ia_results->adpf.RKDemosaicLpResult.edge_level_sel;
			ia_results->rkDemosaicLP.thgrad_r_fct = ia_results->adpf.RKDemosaicLpResult.thgrad_r_fct;
			ia_results->rkDemosaicLP.thdiff_r_fct = ia_results->adpf.RKDemosaicLpResult.thdiff_r_fct;
			ia_results->rkDemosaicLP.thvar_r_fct = ia_results->adpf.RKDemosaicLpResult.thvar_r_fct;
			ia_results->rkDemosaicLP.thgrad_b_fct = ia_results->adpf.RKDemosaicLpResult.thgrad_b_fct;
			ia_results->rkDemosaicLP.thdiff_b_fct = ia_results->adpf.RKDemosaicLpResult.thdiff_b_fct;
			ia_results->rkDemosaicLP.thvar_b_fct = ia_results->adpf.RKDemosaicLpResult.thvar_b_fct;
			memcpy(ia_results->rkDemosaicLP.lu_divided,
					ia_results->adpf.RKDemosaicLpResult.lu_divided,
					sizeof(ia_results->rkDemosaicLP.lu_divided));
			memcpy(ia_results->rkDemosaicLP.thgrad_divided,
					ia_results->adpf.RKDemosaicLpResult.thgrad_divided,
					sizeof(ia_results->rkDemosaicLP.thgrad_divided));
			memcpy(ia_results->rkDemosaicLP.thdiff_divided,
					ia_results->adpf.RKDemosaicLpResult.thdiff_divided,
					sizeof(ia_results->rkDemosaicLP.thdiff_divided));
			memcpy(ia_results->rkDemosaicLP.thcsc_divided,
					ia_results->adpf.RKDemosaicLpResult.thcsc_divided,
					sizeof(ia_results->rkDemosaicLP.thcsc_divided));
			memcpy(ia_results->rkDemosaicLP.thvar_divided,
					ia_results->adpf.RKDemosaicLpResult.thvar_divided,
					sizeof(ia_results->rkDemosaicLP.thvar_divided));
			
            ia_results->active |= CAMIA10_DEMOSAICLP_MASK;		
		}

		if (ia_results->adpf.actives & ADPF_RKIESHARP_MASK)
        {
        	ia_results->rkIEsharp.iesharpen_en = ia_results->adpf.RKIESharpResult.iesharpen_en;	
			ia_results->rkIEsharp.coring_thr = ia_results->adpf.RKIESharpResult.coring_thr;	
			ia_results->rkIEsharp.full_range = ia_results->adpf.RKIESharpResult.full_range;	
			ia_results->rkIEsharp.switch_avg = ia_results->adpf.RKIESharpResult.switch_avg;	
			memcpy(ia_results->rkIEsharp.yavg_thr,
					ia_results->adpf.RKIESharpResult.yavg_thr,
					sizeof(ia_results->rkIEsharp.yavg_thr));
			memcpy(ia_results->rkIEsharp.delta1,
					ia_results->adpf.RKIESharpResult.delta1,
					sizeof(ia_results->rkIEsharp.delta1));
			memcpy(ia_results->rkIEsharp.delta2,
					ia_results->adpf.RKIESharpResult.delta2,
					sizeof(ia_results->rkIEsharp.delta2));
			memcpy(ia_results->rkIEsharp.maxnumber,
					ia_results->adpf.RKIESharpResult.maxnumber,
					sizeof(ia_results->rkIEsharp.maxnumber));
			memcpy(ia_results->rkIEsharp.minnumber,
					ia_results->adpf.RKIESharpResult.minnumber,
					sizeof(ia_results->rkIEsharp.minnumber));
			memcpy(ia_results->rkIEsharp.gauss_flat_coe,
					ia_results->adpf.RKIESharpResult.gauss_flat_coe,
					sizeof(ia_results->rkIEsharp.gauss_flat_coe));
			memcpy(ia_results->rkIEsharp.gauss_noise_coe,
					ia_results->adpf.RKIESharpResult.gauss_noise_coe,
					sizeof(ia_results->rkIEsharp.gauss_noise_coe));
			memcpy(ia_results->rkIEsharp.gauss_other_coe,
					ia_results->adpf.RKIESharpResult.gauss_other_coe,
					sizeof(ia_results->rkIEsharp.gauss_other_coe));
			memcpy(ia_results->rkIEsharp.uv_gauss_flat_coe,
					ia_results->adpf.RKIESharpResult.uv_gauss_flat_coe,
					sizeof(ia_results->rkIEsharp.uv_gauss_flat_coe));
			memcpy(ia_results->rkIEsharp.uv_gauss_noise_coe,
					ia_results->adpf.RKIESharpResult.uv_gauss_noise_coe,
					sizeof(ia_results->rkIEsharp.uv_gauss_noise_coe));
			memcpy(ia_results->rkIEsharp.uv_gauss_other_coe,
					ia_results->adpf.RKIESharpResult.uv_gauss_other_coe,
					sizeof(ia_results->rkIEsharp.uv_gauss_other_coe));		
			memcpy(ia_results->rkIEsharp.p_grad,
					ia_results->adpf.RKIESharpResult.p_grad,
					sizeof(ia_results->rkIEsharp.p_grad));
			memcpy(ia_results->rkIEsharp.sharp_factor,
					ia_results->adpf.RKIESharpResult.sharp_factor,
					sizeof(ia_results->rkIEsharp.sharp_factor));
			memcpy(ia_results->rkIEsharp.line1_filter_coe,
					ia_results->adpf.RKIESharpResult.line1_filter_coe,
					sizeof(ia_results->rkIEsharp.line1_filter_coe));
			memcpy(ia_results->rkIEsharp.line2_filter_coe,
					ia_results->adpf.RKIESharpResult.line2_filter_coe,
					sizeof(ia_results->rkIEsharp.line2_filter_coe));
			memcpy(ia_results->rkIEsharp.line3_filter_coe,
					ia_results->adpf.RKIESharpResult.line3_filter_coe,
					sizeof(ia_results->rkIEsharp.line3_filter_coe));
			memcpy(ia_results->rkIEsharp.lap_mat_coe,
					ia_results->adpf.RKIESharpResult.lap_mat_coe,
					sizeof(ia_results->rkIEsharp.lap_mat_coe));
			
			
            ia_results->active |= CAMIA10_RKIESHARP_MASK;		
		}

    }

    if (mCamIAEngine->getAWDRResults(&ia_results->awdr) == RET_SUCCESS)
    {
        if (ia_results->awdr.actives & AWDR_WDR_MAXGAIN_LEVEL_MASK)
        {
        	ia_results->wdr.enabled = ia_results->awdr.wdr_enable;
			ia_results->wdr.mode = (CameraIcWdrMode_t)(ia_results->awdr.mode);
			memcpy(ia_results->wdr.segment,
					ia_results->awdr.wdr_dx,
					sizeof(ia_results->wdr.segment));

			memcpy(ia_results->wdr.wdr_block_y,
					ia_results->awdr.wdr_block_dy,
					sizeof(ia_results->wdr.wdr_block_y));

			memcpy(ia_results->wdr.wdr_global_y,
					ia_results->awdr.wdr_global_dy,
					sizeof(ia_results->wdr.wdr_global_y));

			ia_results->wdr.wdr_noiseratio = ia_results->awdr.wdr_noiseratio;
			ia_results->wdr.wdr_bestlight = ia_results->awdr.wdr_bestlight;
			ia_results->wdr.wdr_gain_off1 = ia_results->awdr.wdr_gain_off1;
			ia_results->wdr.wdr_pym_cc = ia_results->awdr.wdr_pym_cc;
			ia_results->wdr.wdr_epsilon = ia_results->awdr.wdr_epsilon;
			ia_results->wdr.wdr_lvl_en = ia_results->awdr.wdr_lvl_en;
			ia_results->wdr.wdr_flt_sel = ia_results->awdr.wdr_flt_sel;
			ia_results->wdr.wdr_gain_max_clip_enable = ia_results->awdr.wdr_gain_max_clip_enable;
			ia_results->wdr.wdr_gain_max_value = ia_results->awdr.wdr_gain_max_value;
			ia_results->wdr.wdr_bavg_clip = ia_results->awdr.wdr_bavg_clip;
			ia_results->wdr.wdr_nonl_segm = ia_results->awdr.wdr_nonl_segm;
			ia_results->wdr.wdr_nonl_open = ia_results->awdr.wdr_nonl_open;
			ia_results->wdr.wdr_nonl_mode1 = ia_results->awdr.wdr_nonl_mode1;
			ia_results->wdr.wdr_coe0 = ia_results->awdr.wdr_coe0;
			ia_results->wdr.wdr_coe1 = ia_results->awdr.wdr_coe1;
			ia_results->wdr.wdr_coe2 = ia_results->awdr.wdr_coe2;
			ia_results->wdr.wdr_coe_off = ia_results->awdr.wdr_coe_off;
			
            ia_results->active |= CAMIA10_WDR_MASK;
        }
    }

    if (mCamIAEngine->getAFResults(&ia_results->af) == RET_SUCCESS)
    {
        ia_results->active |= CAMIA10_AFC_MASK;
        ia_results->afc_meas_enabled = BOOL_TRUE;
    }

    return 0;
}

void IspEngine::setExternalAEHandlerDesc(XCamAEDescription* desc)
{
    mCamIAEngine->setExternalAEHandlerDesc(desc);
}

void IspEngine::setExternalAWBHandlerDesc(XCamAWBDescription* desc)
{
    mCamIAEngine->setExternalAWBHandlerDesc(desc);
}

void IspEngine::setExternalAFHandlerDesc(XCamAFDescription* desc)
{
    mCamIAEngine->setExternalAFHandlerDesc(desc);
}

bool IspEngine::configure(const Configuration& config)
{
    bool ret = true;

    osMutexLock(&mApiLock);
    //HAL_AE_FLK_MODE_to_AecEcmFlickerPeriod_t(
    //  config.aec_cfg.flk,
    //  &mCamIA_DyCfg.aec.flicker);

    mCamIA_DyCfg.aec_cfg = config.aec_cfg;
    if ((mCamIA_DyCfg.aec_cfg.win.right_width == 0) ||
            (mCamIA_DyCfg.aec_cfg.win.bottom_height == 0))
    {
        mCamIA_DyCfg.aec_cfg.win.right_width =
            config.sensor_mode.isp_input_width;
        mCamIA_DyCfg.aec_cfg.win.bottom_height =
            config.sensor_mode.isp_input_height;
        mCamIA_DyCfg.aec_cfg.win.left_hoff = 0;
        mCamIA_DyCfg.aec_cfg.win.top_voff = 0;
    }

    mCamIA_DyCfg.afc_cfg = config.afc_cfg;
    if (mCamIA_DyCfg.afc_cfg.mode != HAL_AF_MODE_NOT_SET)
    {
        if ((mCamIA_DyCfg.afc_cfg.win_a.right_width == 0) ||
                (mCamIA_DyCfg.afc_cfg.win_a.bottom_height == 0))
        {
            // set default afc win to 0x0@0,0, means
            // default win is decided by af algorithm
            mCamIA_DyCfg.afc_cfg.win_a.left_hoff = 0;
            mCamIA_DyCfg.afc_cfg.win_a.top_voff = 0;
            mCamIA_DyCfg.afc_cfg.win_a.right_width = 0;
            mCamIA_DyCfg.afc_cfg.win_a.bottom_height = 0;

            mCamIA_DyCfg.afc_cfg.win_num = 1;
            mCamIA_DyCfg.afc_cfg.mode = HAL_AF_MODE_CONTINUOUS_VIDEO;
        }
    }

    mCamIA_DyCfg.aaa_locks = config.aaa_locks;
    mCamIA_DyCfg.awb_cfg = config.awb_cfg;
    mCamIA_DyCfg.flash_mode = config.flash_mode;
    mCamIA_DyCfg.uc = config.uc;

    mCamIA_DyCfg.sensor_mode.isp_input_width =
        config.sensor_mode.isp_input_width;
    mCamIA_DyCfg.sensor_mode.isp_input_height =
        config.sensor_mode.isp_input_height;
    mCamIA_DyCfg.sensor_mode.isp_output_width =
        config.sensor_mode.isp_output_width;
    mCamIA_DyCfg.sensor_mode.isp_output_height =
        config.sensor_mode.isp_output_height;
    mCamIA_DyCfg.sensor_mode.horizontal_crop_offset =
        config.sensor_mode.horizontal_crop_offset;
    mCamIA_DyCfg.sensor_mode.vertical_crop_offset =
        config.sensor_mode.vertical_crop_offset;
    mCamIA_DyCfg.sensor_mode.cropped_image_width =
        config.sensor_mode.cropped_image_width;
    mCamIA_DyCfg.sensor_mode.cropped_image_height =
        config.sensor_mode.cropped_image_height;
    mCamIA_DyCfg.sensor_mode.pixel_clock_freq_mhz =
        config.sensor_mode.pixel_clock_freq_mhz;
    mCamIA_DyCfg.sensor_mode.pixel_periods_per_line =
        config.sensor_mode.pixel_periods_per_line;
    mCamIA_DyCfg.sensor_mode.line_periods_per_field =
        config.sensor_mode.line_periods_per_field;
    mCamIA_DyCfg.sensor_mode.sensor_output_height =
        config.sensor_mode.sensor_output_height;
    mCamIA_DyCfg.sensor_mode.fine_integration_time_min =
        config.sensor_mode.fine_integration_time_min;
    mCamIA_DyCfg.sensor_mode.fine_integration_time_max_margin =
        config.sensor_mode.fine_integration_time_max_margin;
    mCamIA_DyCfg.sensor_mode.coarse_integration_time_min =
        config.sensor_mode.coarse_integration_time_min;
    mCamIA_DyCfg.sensor_mode.coarse_integration_time_max_margin =
        config.sensor_mode.coarse_integration_time_max_margin;

    osMutexUnlock(&mApiLock);

    //cproc
    osMutexLock(&mApiLock);
    if ((config.cproc.contrast != mCamIA_DyCfg.cproc.contrast)
            || (config.cproc.hue != mCamIA_DyCfg.cproc.hue)
            || (config.cproc.brightness != mCamIA_DyCfg.cproc.brightness)
            || (config.cproc.saturation != mCamIA_DyCfg.cproc.saturation)
            || (config.cproc.sharpness != mCamIA_DyCfg.cproc.sharpness))
    {
        struct HAL_ISP_cfg_s cfg ;
        struct HAL_ISP_cproc_cfg_s cproc_cfg;
        cproc_cfg.range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
        cfg.updated_mask = 0;
        cproc_cfg.cproc.contrast = config.cproc.contrast;
        cproc_cfg.cproc.hue = config.cproc.hue;
        cproc_cfg.cproc.brightness = config.cproc.brightness;
        cproc_cfg.cproc.saturation = config.cproc.saturation;
        cproc_cfg.cproc.sharpness = config.cproc.sharpness;
        cfg.cproc_cfg = &cproc_cfg;
        cfg.updated_mask |= HAL_ISP_CPROC_MASK;
        cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_SETTING;
        if ((fabs(config.cproc.contrast -  1.0f) < 0.001)
                && (fabs(config.cproc.hue -  0.0f) < 0.001)
                && (fabs(config.cproc.saturation -  1.0f) < 0.001)
                && (fabs(config.cproc.sharpness -  0.0f) < 0.001)
                && (config.cproc.brightness == 0))
            cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_FALSE;
        mCamIA_DyCfg.cproc = config.cproc;
        osMutexUnlock(&mApiLock);
        LOGD("%s:set cproc ct=%f,hue=%f,bt=%d,st=%f", __func__,
              cproc_cfg.cproc.contrast,
              cproc_cfg.cproc.hue,
              cproc_cfg.cproc.brightness,
              cproc_cfg.cproc.saturation);
        configureISP(&cfg);
    }
    else
        osMutexUnlock(&mApiLock);

    osMutexLock(&mApiLock);
    if (config.ie_mode != mCamIA_DyCfg.ie_mode)
    {
        struct HAL_ISP_cfg_s cfg;
        struct HAL_ISP_ie_cfg_s ie_cfg;
        cfg.updated_mask = 0;
        cfg.ie_cfg = &ie_cfg;
        ie_cfg.range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
        ie_cfg.mode = config.ie_mode;
        cfg.updated_mask |= HAL_ISP_IE_MASK;
        cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_SETTING;
        if (ie_cfg.mode == HAL_EFFECT_NONE)
            cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_FALSE;
        mCamIA_DyCfg.ie_mode = config.ie_mode;
        osMutexUnlock(&mApiLock);
        configureISP(&cfg);
    }
    else
        osMutexUnlock(&mApiLock);

    return ret;
}

/* control ISP module directly*/
bool IspEngine::configureISP(const void* config)
{

    osMutexLock(&mApiLock);
    struct HAL_ISP_cfg_s* cfg = (struct HAL_ISP_cfg_s*)config;
    bool_t ret = BOOL_TRUE;
    /* following configs may confilt with 3A algorithm */
    if (cfg->updated_mask & HAL_ISP_HST_MASK)
    {
        if (cfg->enabled[HAL_ISP_HST_ID] && cfg->hst_cfg)
        {
            mHstNeededUpdate = BOOL_TRUE;
            mHstEnabled = HAL_ISP_ACTIVE_SETTING;
            hst_cfg = *cfg->hst_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_HST_ID])
        {
            mHstNeededUpdate = BOOL_TRUE;
            mHstEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_HST_ID])
        {
            mHstNeededUpdate = BOOL_TRUE;
            mHstEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mHstNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP hst !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_AEC_MASK)
    {
        if (cfg->enabled[HAL_ISP_AEC_ID] && cfg->aec_cfg)
        {
            mAecNeededUpdate = BOOL_TRUE;
            mAecEnabled = HAL_ISP_ACTIVE_SETTING;
            aec_cfg = *cfg->aec_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_AEC_ID])
        {
            mAecNeededUpdate = BOOL_TRUE;
            mAecEnabled = HAL_ISP_ACTIVE_FALSE;
            if (cfg->aec_cfg)
            {
                aec_cfg = *cfg->aec_cfg;
            }
            else
            {
                aec_cfg.exp_time = 0.0f;
                aec_cfg.exp_gain = 0.0f;
            }
        }
        else if (cfg->enabled[HAL_ISP_AEC_ID])
        {
            mAecNeededUpdate = BOOL_TRUE;
            mAecEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mAecNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP aec !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_LSC_MASK)
    {
        if (cfg->enabled[HAL_ISP_LSC_ID] && cfg->lsc_cfg)
        {
            mLscNeededUpdate = BOOL_TRUE;
            mLscEnabled = HAL_ISP_ACTIVE_SETTING;
            lsc_cfg = *cfg->lsc_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_LSC_ID])
        {
            mLscNeededUpdate = BOOL_TRUE;
            mLscEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_LSC_ID])
        {
            mLscNeededUpdate = BOOL_TRUE;
            mLscEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mLscNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP lsc !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_AWB_GAIN_MASK)
    {
        if (cfg->enabled[HAL_ISP_AWB_GAIN_ID] && cfg->awb_gain_cfg)
        {
            mAwbGainNeededUpdate = BOOL_TRUE;
            mAwbEnabled = HAL_ISP_ACTIVE_SETTING;
            awb_gain_cfg = *cfg->awb_gain_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_AWB_GAIN_ID])
        {
            mAwbGainNeededUpdate = BOOL_TRUE;
            mAwbEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_AWB_GAIN_ID])
        {
            mAwbGainNeededUpdate = BOOL_TRUE;
            mAwbEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mAwbGainNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP awb gain !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_BPC_MASK)
    {
        if (cfg->enabled[HAL_ISP_BPC_ID] && cfg->dpcc_cfg)
        {
            mDpccNeededUpdate = BOOL_TRUE;
            mDpccEnabled = HAL_ISP_ACTIVE_SETTING;
            dpcc_cfg = *cfg->dpcc_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_BPC_ID])
        {
            mDpccNeededUpdate = BOOL_TRUE;
            mDpccEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_BPC_ID])
        {
            mDpccNeededUpdate = BOOL_TRUE;
            mDpccEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mDpccNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP dpcc !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_SDG_MASK)
    {
        if (cfg->enabled[HAL_ISP_SDG_ID] && cfg->sdg_cfg)
        {
            mSdgNeededUpdate = BOOL_TRUE;
            mSdgEnabled = HAL_ISP_ACTIVE_SETTING;
            sdg_cfg = *cfg->sdg_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_SDG_ID])
        {
            mSdgNeededUpdate = BOOL_TRUE;
            mSdgEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_SDG_ID])
        {
            mSdgNeededUpdate = BOOL_TRUE;
            mSdgEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mSdgNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP sdg !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_CTK_MASK)
    {
        if (cfg->enabled[HAL_ISP_CTK_ID] && cfg->ctk_cfg)
        {
            mCtkNeededUpdate = BOOL_TRUE;
            mCtkEnabled = HAL_ISP_ACTIVE_SETTING;
            ctk_cfg = *cfg->ctk_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_CTK_ID])
        {
            mCtkNeededUpdate = BOOL_TRUE;
            mCtkEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_CTK_ID])
        {
            mCtkNeededUpdate = BOOL_TRUE;
            mCtkEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mCtkNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP ctk !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_AWB_MEAS_MASK)
    {
        if (cfg->enabled[HAL_ISP_AWB_MEAS_ID] && cfg->awb_cfg)
        {
            mAwbMeNeededUpdate = BOOL_TRUE;
            mAwbMeEnabled = HAL_ISP_ACTIVE_SETTING;
            awb_cfg = *cfg->awb_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_AWB_MEAS_ID])
        {
            mAwbMeNeededUpdate = BOOL_TRUE;
            mAwbMeEnabled = HAL_ISP_ACTIVE_FALSE;
            if (cfg->awb_cfg)
            {
                awb_cfg.illuIndex =  cfg->awb_cfg->illuIndex;
            }
            else
            {
                awb_cfg.illuIndex =  -1;
            }
        }
        else if (cfg->enabled[HAL_ISP_AWB_MEAS_ID])
        {
            mAwbMeNeededUpdate = BOOL_TRUE;
            mAwbMeEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mAwbMeNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP awb measure !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_AFC_MASK)
    {
        if (cfg->enabled[HAL_ISP_AFC_ID] && cfg->afc_cfg)
        {
            mAfcNeededUpdate = BOOL_TRUE;
            mAfcEnabled = HAL_ISP_ACTIVE_SETTING;
            afc_cfg = *cfg->afc_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_AFC_ID])
        {
            mAfcNeededUpdate = BOOL_TRUE;
            mAfcEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_AFC_ID])
        {
            mAfcNeededUpdate = BOOL_TRUE;
            mAfcEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mAfcNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP afc !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_DPF_MASK)
    {
        if (cfg->enabled[HAL_ISP_DPF_ID] && cfg->dpf_cfg)
        {
            mDpfNeededUpdate = BOOL_TRUE;
            mDpfEnabled = HAL_ISP_ACTIVE_SETTING;
            dpf_cfg = *cfg->dpf_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_DPF_ID])
        {
            mDpfNeededUpdate = BOOL_TRUE;
            mDpfEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_DPF_ID])
        {
            mDpfNeededUpdate = BOOL_TRUE;
            mDpfEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mDpfNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP dpf !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_DPF_STRENGTH_MASK)
    {
        if (cfg->enabled[HAL_ISP_DPF_STRENGTH_ID] && cfg->dpf_strength_cfg)
        {
            mDpfStrengthNeededUpdate = BOOL_TRUE;
            mDpfStrengthEnabled = HAL_ISP_ACTIVE_SETTING;
            dpf_strength_cfg = *cfg->dpf_strength_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_DPF_STRENGTH_ID])
        {
            mDpfStrengthNeededUpdate = BOOL_TRUE;
            mDpfStrengthEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_DPF_STRENGTH_ID])
        {
            mDpfStrengthNeededUpdate = BOOL_TRUE;
            mDpfStrengthEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mDpfStrengthNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP dpf strength!", __func__);
            goto config_end;
        }
    }

    /* following configs may confilt with user settings */

    if (cfg->updated_mask & HAL_ISP_CPROC_MASK)
    {
        if (cfg->enabled[HAL_ISP_CPROC_ID] && cfg->cproc_cfg)
        {
            mCprocNeededUpdate = BOOL_TRUE;
            mCprocEnabled = HAL_ISP_ACTIVE_SETTING;
            cproc_cfg = *cfg->cproc_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_CPROC_ID])
        {
            mCprocNeededUpdate = BOOL_TRUE;
            mCprocEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_CPROC_ID])
        {
            mCprocNeededUpdate = BOOL_TRUE;
            mCprocEnabled = HAL_ISP_ACTIVE_DEFAULT;
            cproc_cfg = *cfg->cproc_cfg;
        }
        else
        {
            mCprocNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP cproc!", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_IE_MASK)
    {
        if (cfg->enabled[HAL_ISP_IE_ID] && cfg->ie_cfg)
        {
            mIeNeededUpdate = BOOL_TRUE;
            mIeEnabled = HAL_ISP_ACTIVE_SETTING;
            ie_cfg = *cfg->ie_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_IE_ID])
        {
            mIeNeededUpdate = BOOL_TRUE;
            mIeEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_IE_ID])
        {
            mIeNeededUpdate = BOOL_TRUE;
            mIeEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mIeNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP ie!", __func__);
            goto config_end;
        }
    }

    /* can config free*/
    if (cfg->updated_mask & HAL_ISP_GOC_MASK)
    {
        if (cfg->enabled[HAL_ISP_GOC_ID]== HAL_ISP_ACTIVE_SETTING && cfg->goc_cfg)
        {
            mGocNeededUpdate = BOOL_TRUE;
            mGocEnabled = HAL_ISP_ACTIVE_SETTING;
            goc_cfg = *cfg->goc_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_GOC_ID])
        {
            mGocNeededUpdate = BOOL_TRUE;
            mGocEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_GOC_ID])
        {
            mGocNeededUpdate = BOOL_TRUE;
            mGocEnabled = HAL_ISP_ACTIVE_DEFAULT;
            if(cfg->goc_cfg)
            {
                goc_cfg = *cfg->goc_cfg;
            }
        }
        else
        {
            mGocNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP goc!", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_FLT_MASK)
    {
        if (cfg->enabled[HAL_ISP_FLT_ID] && cfg->flt_cfg)
        {
            mFltNeededUpdate = BOOL_TRUE;
            mFltEnabled = HAL_ISP_ACTIVE_SETTING;
            flt_cfg = *cfg->flt_cfg;
            ALOGE("%s:HAL_ISP_FLT_MASK HAL_ISP_ACTIVE_SETTING!", __func__);
        }
        else if (!cfg->enabled[HAL_ISP_FLT_ID])
        {
            mFltNeededUpdate = BOOL_TRUE;
            mFltEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_FLT_ID])
        {
            mFltNeededUpdate = BOOL_TRUE;
            mFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mFltNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP flt!", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_BDM_MASK)
    {
        if (cfg->enabled[HAL_ISP_BDM_ID] && cfg->bdm_cfg)
        {
            mBdmNeededUpdate = BOOL_TRUE;
            mBdmEnabled = HAL_ISP_ACTIVE_SETTING;
            bdm_cfg = *cfg->bdm_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_BDM_ID])
        {
            mBdmNeededUpdate = BOOL_TRUE;
            mBdmEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_BDM_ID])
        {
            mBdmNeededUpdate = BOOL_TRUE;
            mBdmEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mBdmNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP bdm!", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_BLS_MASK)
    {
        if (cfg->enabled[HAL_ISP_BLS_ID] && cfg->bls_cfg)
        {
            mBlsNeededUpdate = BOOL_TRUE;
            mBlsEnabled = HAL_ISP_ACTIVE_SETTING;
            bls_cfg = *cfg->bls_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_BLS_ID])
        {
            mBlsNeededUpdate = BOOL_TRUE;
            mBlsEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_BLS_ID])
        {
            mBlsNeededUpdate = BOOL_TRUE;
            mBlsEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mBlsNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config bls !", __func__);
            goto config_end;
        }
    }

    if (cfg->updated_mask & HAL_ISP_WDR_MASK)
    {
        if (cfg->enabled[HAL_ISP_WDR_ID] && cfg->wdr_cfg)
        {
            mWdrNeededUpdate = BOOL_TRUE;
            mWdrEnabled = HAL_ISP_ACTIVE_SETTING;
            wdr_cfg = *cfg->wdr_cfg;
        }
        else if (!cfg->enabled[HAL_ISP_WDR_ID])
        {
            mWdrNeededUpdate = BOOL_TRUE;
            mWdrEnabled = HAL_ISP_ACTIVE_FALSE;
        }
        else if (cfg->enabled[HAL_ISP_WDR_ID])
        {
            mWdrNeededUpdate = BOOL_TRUE;
            mWdrEnabled = HAL_ISP_ACTIVE_DEFAULT;
        }
        else
        {
            mWdrNeededUpdate = BOOL_FALSE;
            ALOGE("%s:can't config ISP bdm!", __func__);
            goto config_end;
        }
    }

	if (cfg->updated_mask & HAL_ISP_3DNR_MASK) {
	    if (cfg->enabled[HAL_ISP_3DNR_ID] && cfg->dsp_3dnr_cfg) 
		{
	      m3DnrNeededUpdate = BOOL_TRUE;
	      m3DnrEnabled = HAL_ISP_ACTIVE_SETTING;
	      dsp_3dnr_cfg = *cfg->dsp_3dnr_cfg;
	    } 
		else if (!cfg->enabled[HAL_ISP_3DNR_ID]) 
    	{
	      m3DnrNeededUpdate = BOOL_TRUE;
	      m3DnrEnabled = HAL_ISP_ACTIVE_FALSE;
	    } 
		else if (cfg->enabled[HAL_ISP_3DNR_ID]) 
	    {
	      m3DnrNeededUpdate = BOOL_TRUE;
	      m3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;
	    } 
		else 
	    {
	      m3DnrNeededUpdate = BOOL_FALSE;
	      ALOGE("%s:can't config dsp 3dnr!", __func__);
	    }
	}

	if (cfg->updated_mask & HAL_ISP_NEW_3DNR_MASK) {
	    if (cfg->enabled[HAL_ISP_NEW_3DNR_ID] && cfg->newDsp3DNR_cfg) 
		{
	      mNew3DnrNeededUpdate = BOOL_TRUE;
	      mNew3DnrEnabled = HAL_ISP_ACTIVE_SETTING;
	      new_dsp_3dnr_cfg = *cfg->newDsp3DNR_cfg;
	    } 
		else if (!cfg->enabled[HAL_ISP_NEW_3DNR_ID]) 
		{
	      mNew3DnrNeededUpdate = BOOL_TRUE;
	      mNew3DnrEnabled = HAL_ISP_ACTIVE_FALSE;
	    } 
		else if (cfg->enabled[HAL_ISP_NEW_3DNR_ID]) 
		{
	      mNew3DnrNeededUpdate = BOOL_TRUE;
	      mNew3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;
	    } 
		else 
		{
	      mNew3DnrNeededUpdate = BOOL_FALSE;
	      ALOGE("%s:can't config new dsp 3dnr!", __func__);
	    }
  	}

	if (cfg->updated_mask & HAL_ISP_DEMOSAICLP_MASK) {
		if (cfg->enabled[HAL_ISP_DEMOSAICLP_ID] && cfg->demosaicLP_cfg) 
		{
	      mDemosaicLPNeededUpdate = BOOL_TRUE;
	      mDemosaicLPEnable = HAL_ISP_ACTIVE_SETTING;
	      demosaiclp_cfg = *cfg->demosaicLP_cfg;
	    } 
		else if (!cfg->enabled[HAL_ISP_DEMOSAICLP_ID]) 
		{
	      mDemosaicLPNeededUpdate = BOOL_TRUE;
	      mDemosaicLPEnable = HAL_ISP_ACTIVE_FALSE;
	    } 
		else if (cfg->enabled[HAL_ISP_DEMOSAICLP_ID]) 
		{
	      mDemosaicLPNeededUpdate = BOOL_TRUE;
	      mDemosaicLPEnable = HAL_ISP_ACTIVE_DEFAULT;
	    } 
		else 
		{
	      mDemosaicLPNeededUpdate = BOOL_FALSE;
	      ALOGE("%s:can't config new dsp 3dnr!", __func__);
	    }
	}

	if (cfg->updated_mask & HAL_ISP_RK_IESHARP_MASK) {
		if (cfg->enabled[HAL_ISP_RKIESHARP_ID] && cfg->rkIEsharp_cfg) 
		{
	      mrkIEsharpNeededUpdate = BOOL_TRUE;
	      mrkIEsharpEnable = HAL_ISP_ACTIVE_SETTING;
	      rkIEsharp_cfg = *cfg->rkIEsharp_cfg;
	    } 
		else if (!cfg->enabled[HAL_ISP_RKIESHARP_ID]) 
		{
	      mrkIEsharpNeededUpdate = BOOL_TRUE;
	      mrkIEsharpEnable = HAL_ISP_ACTIVE_FALSE;
	    } 
		else if (cfg->enabled[HAL_ISP_RKIESHARP_ID]) 
		{
	      mrkIEsharpNeededUpdate = BOOL_TRUE;
	      mrkIEsharpEnable = HAL_ISP_ACTIVE_DEFAULT;
	    } 
		else 
		{
	      mrkIEsharpNeededUpdate = BOOL_FALSE;
	      ALOGE("%s:can't config new dsp 3dnr!", __func__);
	    }
	}
    /* should reconfig 3A algorithm ?*/
config_end:
    osMutexUnlock(&mApiLock);
    return ret;
}

bool IspEngine::start()
{
    LOGD("%s: ready to start\n", __func__);
    bool ret = true;
    /*
    //should call this after camera stream on, or driver will return error
    if (mCamHwItf->isSupportedIrCut() >= 0)
      mSupportedSubDevs |= SUBDEV_IRCUT_MASK;
    */
    osMutexLock(&mApiLock);
    if (!mInitialized)
        goto end;
    if (++mStartCnt > 1)
        goto end;

    LOGD("%s: startMeasurements\n", __func__);
    if (!startMeasurements())
    {
        ALOGE("%s failed to start measurements", __func__);
        --mStartCnt;
        ret = false;
        goto end;
    }

end:
    osMutexUnlock(&mApiLock);
    return ret;
}
bool IspEngine::stop()
{
    bool ret = true;

    osMutexLock(&mApiLock);
    if (!mInitialized)
        goto end;
    if ((!mStartCnt) || ((mStartCnt > 0) && (--mStartCnt)))
        goto end;
    osMutexUnlock(&mApiLock);

    osMutexLock(&mApiLock);
    stopMeasurements();

end:
    osMutexUnlock(&mApiLock);
    return ret;
}

bool IspEngine::setISPDeviceFd(int ispFd)
{
    if (ispFd <= 0 )
    {
        ALOGE("%s: invalid isp device file description(%d)/n",
              __func__,
              ispFd);
    }
    mIspFd = ispFd;

    return true;
}

bool IspEngine::initISPStream(const char* ispDev)
{
    struct v4l2_requestbuffers req;
    struct v4l2_buffer v4l2_buf;

    mIspFd = open(ispDev, O_RDWR | O_NONBLOCK);
    if (mIspFd < 0)
    {
        ALOGE("%s: Cannot open %s (error : %s)\n",
              __func__,
              ispDev,
              strerror(errno));
        return false;
    }

    req.count = CAM_ISP_NUM_OF_STAT_BUFS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(mIspFd, VIDIOC_REQBUFS, &req) < 0)
    {
        ALOGE("%s: VIDIOC_REQBUFS failed, strerror: %s",
              __func__,
              strerror(errno));
        return false;
    }

    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    for (unsigned int i = 0; i < req.count; i++)
    {
        v4l2_buf.index = i;
        if (ioctl(mIspFd, VIDIOC_QUERYBUF, &v4l2_buf) < 0)
        {
            ALOGE("%s: VIDIOC_QUERYBUF failed\n", __func__);
            return false;
        }

        mIspStatBuf[i] = mmap(0,
                              v4l2_buf.length,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED,
                              mIspFd,
                              v4l2_buf.m.offset);
        if (mIspStatBuf[i] == MAP_FAILED)
        {
            ALOGE("%s mmap() failed\n", __func__);
            return false;
        }

        if (ioctl(mIspFd, VIDIOC_QBUF, &v4l2_buf) < 0)
        {
            ALOGE("QBUF failed index %d", v4l2_buf.index);
            return false;
        }
    }

    mIspStatBufSize = v4l2_buf.length;
    return true;
}

bool IspEngine::getMeasurement(struct v4l2_buffer& v4l2_buf)
{
    int retrycount = 300, ret;
    struct pollfd fds[1];
    int timeout_ms = 3000;

    fds[0].fd = mIspFd;
    fds[0].events = POLLIN | POLLERR;

    while (retrycount > 0)
    {
        ret = poll(fds, 1, timeout_ms);
        if (ret <= 0)
        {
            ALOGE("%s: poll error, %s",
                  __FUNCTION__,
                  strerror(errno));
            return false;
        }

        if (fds[0].revents & POLLERR)
        {
            LOGD("%s: POLLERR in isp node", __FUNCTION__);
            return false;
        }

        if (fds[0].revents & POLLIN)
        {
            v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2_buf.memory = V4L2_MEMORY_MMAP;

            if (ioctl(mIspFd, VIDIOC_DQBUF, &v4l2_buf) < 0)
            {
                LOGD("%s: VIDIOC_DQBUF failed, retry count %d\n",
                      __FUNCTION__,
                      retrycount);
                retrycount--;
                continue;
            }
            LOGD("%s:  VIDIOC_DQBUF v4l2_buf: %d",
                  __func__,
                  v4l2_buf.index);
            if (v4l2_buf.sequence == (uint32_t) - 1)
            {
                LOGD("%s: sequence=-1 qbuf: %d", v4l2_buf.index);
                releaseMeasurement(&v4l2_buf);
            }

            return true;
        }
    }
    return false;
}
bool IspEngine::releaseMeasurement(struct v4l2_buffer* v4l2_buf)
{
    if (ioctl(mIspFd, VIDIOC_QBUF, v4l2_buf) < 0)
    {
        ALOGE("%s: QBUF failed", __func__);
        return false;
    }

    return true;

}
bool IspEngine::stopMeasurements()
{
    bool ret = false;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    LOGD("%s: going to STREAMOFF", __func__);
    if (ioctl(mIspFd, VIDIOC_STREAMOFF, &type) < 0)
    {
        ALOGE("%s: VIDIOC_STREAMON failed\n", __func__);
        return false;
    }

    mStreaming = false;
    return ret;
}
bool IspEngine::startMeasurements()
{
    int ret;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //LOGD("%s: mIspFd: %d",
    //  __func__,
    //  mIspFd);
    if ((ret = ioctl(mIspFd, VIDIOC_STREAMON, &type)) < 0)
    {
        ALOGE("%s: VIDIOC_STREAMON failed, %s\n", __func__,
              strerror(ret));
        return false;
    }

    mStreaming = true;
    return true;
}

bool IspEngine::runISPManual(struct CamIA10_Results* ia_results, bool_t lock)
{
    struct HAL_ISP_cfg_s  manCfg;

    memset(&manCfg, 0, sizeof(struct HAL_ISP_cfg_s));
    if (lock)
        osMutexLock(&mApiLock);

    /* TODO: following may conflict with AEC, now update always to override AEC settings*/
    if (mHstNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_HST_ID] = mHstEnabled;
        if (mHstEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by aec default
            manCfg.updated_mask &= ~HAL_ISP_HST_MASK;
            mHstNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.hst_cfg = &hst_cfg;
            manCfg.updated_mask |= HAL_ISP_HST_MASK;
        }
    }

    if (mAecNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_AEC_ID] = mAecEnabled;
        if (mAecEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by aec default
            manCfg.updated_mask &= ~HAL_ISP_AEC_MASK;
            mAecNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.aec_cfg = &aec_cfg;
            manCfg.updated_mask |= HAL_ISP_AEC_MASK;
        }
    }

    /* TODO: following may conflict with AWB, now update always to override AWB settings*/
    if (mLscNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_LSC_ID] = mLscEnabled;
        if (mLscEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by awb default
            manCfg.updated_mask &= ~HAL_ISP_LSC_MASK;
            mLscNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.lsc_cfg = &lsc_cfg;
            manCfg.updated_mask |= HAL_ISP_LSC_MASK;
        }
    }

    if (mAwbGainNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_AWB_GAIN_ID] = mAwbEnabled;
        if (mAwbEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by awb default
            manCfg.updated_mask &= ~HAL_ISP_AWB_GAIN_MASK;
            mAwbGainNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.awb_gain_cfg = &awb_gain_cfg;
            manCfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
        }
    }

    if (mAwbMeNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_AWB_MEAS_ID] = mAwbMeEnabled;
        if (mAwbMeEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by awb default
            manCfg.updated_mask &= ~HAL_ISP_AWB_MEAS_MASK;
            mAwbMeNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.awb_cfg = &awb_cfg;
            manCfg.updated_mask |= HAL_ISP_AWB_MEAS_MASK;
            //if awb gain is set, awb measure should enable
            if ((mAwbMeEnabled == HAL_ISP_ACTIVE_FALSE) &&
                    (mAwbEnabled == HAL_ISP_ACTIVE_SETTING))
            {
                //controlled by awb default
                manCfg.updated_mask &= ~HAL_ISP_AWB_MEAS_ID;
                mAwbMeNeededUpdate = BOOL_FALSE;
            }
        }
    }

    if (mCtkNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_CTK_ID] = mCtkEnabled;
        if (mCtkEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by awb default
            manCfg.updated_mask &= ~HAL_ISP_CTK_MASK;
            mCtkNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.ctk_cfg = &ctk_cfg;
            manCfg.updated_mask |= HAL_ISP_CTK_MASK;
        }
    }

    /* TODO: following may conflict with AWB, now update always to override AWB settings*/
    if (mDpfNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_DPF_ID] = mDpfEnabled;
        if (mDpfEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by adpf default
            manCfg.updated_mask &= ~HAL_ISP_DPF_MASK;
            mDpfNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.dpf_cfg = &dpf_cfg;
            manCfg.updated_mask |= HAL_ISP_DPF_MASK;
        }
    }

    if (mDpfStrengthNeededUpdate)
    {
        manCfg.enabled[HAL_ISP_DPF_STRENGTH_ID] = mDpfStrengthEnabled;
        if (mDpfStrengthEnabled == HAL_ISP_ACTIVE_DEFAULT)
        {
            //controlled by adpf default
            manCfg.updated_mask &= ~HAL_ISP_DPF_STRENGTH_MASK;
            mDpfStrengthNeededUpdate = BOOL_FALSE;
        }
        else
        {
            manCfg.dpf_strength_cfg = &dpf_strength_cfg;
            manCfg.updated_mask |= HAL_ISP_DPF_STRENGTH_MASK;
        }
    }

    /* TODO: following may conflict with AFC, now update always to override AFC settings*/
    if (mAfcNeededUpdate)
    {
        manCfg.afc_cfg = &afc_cfg;
        manCfg.updated_mask |= HAL_ISP_AFC_MASK;
        manCfg.enabled[HAL_ISP_AFC_ID] = mAfcEnabled;
        //mAfcNeededUpdate= BOOL_FALSE;
    }

    if (mBlsNeededUpdate)
    {
        manCfg.bls_cfg = &bls_cfg;
        manCfg.updated_mask |= HAL_ISP_BLS_MASK;
        manCfg.enabled[HAL_ISP_BLS_ID] = mBlsEnabled;
        mBlsNeededUpdate = BOOL_FALSE;
    }

    if (mIeNeededUpdate)
    {
        manCfg.ie_cfg = &ie_cfg;
        manCfg.updated_mask |= HAL_ISP_IE_MASK;
        manCfg.enabled[HAL_ISP_IE_ID] = mIeEnabled;
        mIeNeededUpdate = BOOL_FALSE;
    }

    if (mDpccNeededUpdate)
    {
        manCfg.dpcc_cfg = &dpcc_cfg;
        manCfg.updated_mask |= HAL_ISP_BPC_MASK;
        manCfg.enabled[HAL_ISP_BPC_ID] = mDpccEnabled;
        mDpccNeededUpdate = BOOL_FALSE;
    }

    if (mSdgNeededUpdate)
    {
        manCfg.sdg_cfg = &sdg_cfg;
        manCfg.updated_mask |= HAL_ISP_SDG_MASK;
        manCfg.enabled[HAL_ISP_SDG_ID] = mSdgEnabled;
        mSdgNeededUpdate = BOOL_FALSE;
    }
    if (mFltNeededUpdate)
    {
        manCfg.flt_cfg = &flt_cfg;
        manCfg.updated_mask |= HAL_ISP_FLT_MASK;
        manCfg.enabled[HAL_ISP_FLT_ID] = mFltEnabled;
        mFltNeededUpdate = BOOL_FALSE;
    }

    if (mBdmNeededUpdate)
    {
        manCfg.bdm_cfg = &bdm_cfg;
        manCfg.updated_mask |= HAL_ISP_BDM_MASK;
        manCfg.enabled[HAL_ISP_BDM_ID] = mBdmEnabled;
        mBdmNeededUpdate = BOOL_FALSE;
    }

    if (mGocNeededUpdate)
    {
        manCfg.goc_cfg = &goc_cfg;
        manCfg.updated_mask |= HAL_ISP_GOC_MASK;
        manCfg.enabled[HAL_ISP_GOC_ID] = mGocEnabled;
        mGocNeededUpdate = BOOL_FALSE;
    }

    if (mCprocNeededUpdate)
    {
        manCfg.cproc_cfg = &cproc_cfg;
        manCfg.updated_mask |= HAL_ISP_CPROC_MASK;
        manCfg.enabled[HAL_ISP_CPROC_ID] = mCprocEnabled;
        mCprocNeededUpdate = BOOL_FALSE;
    }

    if (mWdrNeededUpdate)
    {
        manCfg.wdr_cfg = &wdr_cfg;
        manCfg.updated_mask |= HAL_ISP_WDR_MASK;
        manCfg.enabled[HAL_ISP_WDR_ID] = mWdrEnabled;
        mWdrNeededUpdate = BOOL_FALSE;
    }

	if (m3DnrNeededUpdate) 
	{
        manCfg.dsp_3dnr_cfg = &dsp_3dnr_cfg;
        manCfg.updated_mask |= HAL_ISP_3DNR_MASK;
        manCfg.enabled[HAL_ISP_3DNR_ID] = m3DnrEnabled;
        m3DnrNeededUpdate = BOOL_FALSE;
    }
  
    /*new 3dnr */
    if (mNew3DnrNeededUpdate) 
	{
        manCfg.newDsp3DNR_cfg = &new_dsp_3dnr_cfg;
        manCfg.updated_mask |= HAL_ISP_NEW_3DNR_MASK;
        manCfg.enabled[HAL_ISP_NEW_3DNR_ID] = mNew3DnrEnabled;
        mNew3DnrNeededUpdate = BOOL_FALSE;
    }
  
	if (mDemosaicLPNeededUpdate)
	{
		manCfg.demosaicLP_cfg = &demosaiclp_cfg;
		manCfg.updated_mask |= HAL_ISP_DEMOSAICLP_MASK;
		manCfg.enabled[HAL_ISP_DEMOSAICLP_ID] = mDemosaicLPEnable;
		mDemosaicLPNeededUpdate = BOOL_FALSE;
	}

	if (mrkIEsharpNeededUpdate)
	{
		manCfg.rkIEsharp_cfg = &rkIEsharp_cfg;
		manCfg.updated_mask |= HAL_ISP_RK_IESHARP_MASK;
		manCfg.enabled[HAL_ISP_RKIESHARP_ID] = mrkIEsharpEnable;
		mrkIEsharpNeededUpdate = BOOL_FALSE;
	}

    if (lock)
        osMutexUnlock(&mApiLock);
    if (mCamIAEngine.get() &&
            (RET_SUCCESS == mCamIAEngine->runManISP(&manCfg, ia_results)))
        return BOOL_TRUE;
    else
        return BOOL_FALSE;
}

void IspEngine::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
)
{
    return;
}

int IspEngine::switchSubDevIrCutMode(int mode)
{
#if 1
    /*
      if (!mCamHwItf || ((mSupportedSubDevs & SUBDEV_IRCUT_MASK) == 0))
        return 0;
      int ircut_state = mCamHwItf->getIrCutState();
      if ((ircut_state < 0) ||
          (((mode > 0) && (ircut_state > 0)) ||
           ((mode == 0) && (ircut_state == 0)))) {
        return 0;
      }
    */
#else
    static int last_mode = 1;
    if (mode != last_mode)
        last_mode = mode;
    else
        return 0;
#endif
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_ie_cfg_s ie_cfg;
    struct HAL_ISP_goc_cfg_s goc_cfg;
    memset(&cfg, 0, sizeof(struct HAL_ISP_cfg_s));
    cfg.updated_mask = 0;
    ie_cfg.range = HAL_ISP_COLOR_RANGE_OUT_BT601;
    cfg.ie_cfg = &ie_cfg;
    cfg.goc_cfg = &goc_cfg;
    if (mode == 0)
    {
        //stop awb
        //here HAL_WB_DAYLIGHT means awb stopped
        mLastWbMode = mCamIA_DyCfg.awb_cfg.mode;
        mCamIA_DyCfg.awb_cfg.mode = HAL_WB_INVAL;
        cfg.updated_mask |= HAL_ISP_CTK_MASK;
        cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_FALSE;
        cfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
        cfg.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_FALSE;
        //set ie mode to mono
        osMutexLock(&mApiLock);
        ie_cfg.mode = HAL_EFFECT_MONO;
        cfg.updated_mask |= HAL_ISP_IE_MASK;
        cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_SETTING;
        mCamIA_DyCfg.ie_mode = HAL_EFFECT_MONO;
        osMutexUnlock(&mApiLock);
        //disable wdr
        cfg.updated_mask |= HAL_ISP_WDR_MASK;
        cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_FALSE;
        //disable lsc
        cfg.updated_mask |= HAL_ISP_LSC_MASK;
        cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_FALSE;
        //set gamma using wdr off gammay
        cfg.updated_mask |= HAL_ISP_GOC_MASK;
        cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
        mCamIA_DyCfg.LightMode = LIGHT_MODE_NIGHT;
        goc_cfg.light_mode = mCamIA_DyCfg.LightMode;

        //close ircut
        //mCamHwItf->setIrCutState(0);

    }
    else
    {
        //start awb
        mCamIA_DyCfg.awb_cfg.mode = mLastWbMode;
        cfg.updated_mask |= HAL_ISP_CTK_MASK;
        cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_DEFAULT;
        cfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
        cfg.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_DEFAULT;
        //set ie mode to normal
        osMutexLock(&mApiLock);
        ie_cfg.mode = HAL_EFFECT_NONE;
        cfg.updated_mask |= HAL_ISP_IE_MASK;
        cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_FALSE;
        mCamIA_DyCfg.ie_mode = HAL_EFFECT_NONE;
        osMutexUnlock(&mApiLock);
        //enable wdr
        cfg.updated_mask |= HAL_ISP_WDR_MASK;
        cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_DEFAULT;
        //enable lsc,now this will cause fliker
        cfg.updated_mask |= HAL_ISP_LSC_MASK;
        cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
        //set gamma using wdron gammaY
        cfg.updated_mask |= HAL_ISP_GOC_MASK;
        cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
        mCamIA_DyCfg.LightMode = LIGHT_MODE_DAY;
        goc_cfg.light_mode = mCamIA_DyCfg.LightMode;
        //open ircut
        //mCamHwItf->setIrCutState(1);
    }

    //ALOGE("%s: 111 lightmode:%d", __func__, mCamIA_DyCfg.LightMode);
    configureISP(&cfg);
    return 0;
}

bool IspEngine::runIA(struct CamIA10_DyCfg* ia_dcfg,
                      struct CamIA10_Stats* ia_stats,
                      struct CamIA10_Results* ia_results)
{
    //LOGD("runIA, ia_dcfg: %d, ia_stats: %d", ia_dcfg, ia_stats);
    if (ia_dcfg)
        mCamIAEngine->initDynamic(ia_dcfg);

    if (ia_stats)
    {
        mCamIAEngine->setStatistics(ia_stats);

        if (ia_stats->meas_type & CAMIA10_AEC_MASK)
        {
            mCamIAEngine->runAEC();
            mCamIAEngine->runADPF();
            mCamIAEngine->runAWDR();
        }

        if (ia_stats->meas_type & CAMIA10_AWB_MEAS_MASK)
        {
            mCamIAEngine->runAWB();
        }

        if (ia_stats->meas_type & CAMIA10_AFC_MASK)
        {
            mCamIAEngine->runAF();
        }
    }

    if (ia_results)
    {
        ia_results->active = 0;
        if (mCamIAEngine->getAECResults(&ia_results->aec) == RET_SUCCESS)
        {
            ia_results->active |= CAMIA10_AEC_MASK;
            ia_results->hst.enabled = BOOL_TRUE;
            //copy aec hst result to struct hst, may be override by manual settings after
            ia_results->hst.mode = CAMERIC_ISP_HIST_MODE_RGB_COMBINED;
            ia_results->hst.Window.width =
                ia_results->aec.meas_win.h_size;
            ia_results->hst.Window.height =
                ia_results->aec.meas_win.v_size;
            ia_results->hst.Window.hOffset =
                ia_results->aec.meas_win.h_offs;
            ia_results->hst.Window.vOffset =
                ia_results->aec.meas_win.v_offs;
            ia_results->hst.StepSize =
                ia_results->aec.stepSize;

            if (ia_results->aec.aoe_enable)
            {
                ia_results->hst.Weights[0] = 0;
                ia_results->hst.Weights[1] = 0;
                ia_results->hst.Weights[2] = 0;
                ia_results->hst.Weights[3] = 0;
                ia_results->hst.Weights[4] = 0;

                ia_results->hst.Weights[5] = 0;
                ia_results->hst.Weights[6] = 0;
                ia_results->hst.Weights[7] = 10;
                ia_results->hst.Weights[8] = 0;
                ia_results->hst.Weights[9] = 0;

                ia_results->hst.Weights[10] = 0;
                ia_results->hst.Weights[11] = 10;
                ia_results->hst.Weights[12] = 10;
                ia_results->hst.Weights[13] = 10;
                ia_results->hst.Weights[14] = 0;

                ia_results->hst.Weights[15] = 0;
                ia_results->hst.Weights[16] = 0;
                ia_results->hst.Weights[17] = 10;
                ia_results->hst.Weights[18] = 0;
                ia_results->hst.Weights[19] = 0;

                ia_results->hst.Weights[20] = 0;
                ia_results->hst.Weights[21] = 0;
                ia_results->hst.Weights[22] = 0;
                ia_results->hst.Weights[23] = 0;
                ia_results->hst.Weights[24] = 0;
            }
            else
            {
                memcpy(ia_results->hst.Weights,
                       ia_results->aec.GridWeights, sizeof(ia_results->aec.GridWeights));
            }


            ia_results->aec_enabled = BOOL_TRUE;
        }

        memset(&ia_results->awb, 0, sizeof(ia_results->awb));
        if (mCamIAEngine->getAWBResults(&ia_results->awb) == RET_SUCCESS)
        {
            if (ia_results->awb.actives & AWB_RECONFIG_GAINS)
                ia_results->active |= CAMIA10_AWB_GAIN_MASK;
            if ((ia_results->awb.actives & AWB_RECONFIG_CCMATRIX)
                    || (ia_results->awb.actives & AWB_RECONFIG_CCOFFSET))
                ia_results->active |= CAMIA10_CTK_MASK;
            if ((ia_results->awb.actives & AWB_RECONFIG_LSCMATRIX)
                    || (ia_results->awb.actives & AWB_RECONFIG_LSCSECTOR))
                ia_results->active |= CAMIA10_LSC_MASK;
            if ((ia_results->awb.actives & AWB_RECONFIG_MEASMODE)
                    || (ia_results->awb.actives & AWB_RECONFIG_MEASCFG)
                    || (ia_results->awb.actives & AWB_RECONFIG_AWBWIN))
                ia_results->active |= CAMIA10_AWB_MEAS_MASK;
            ia_results->awb_gains_enabled = BOOL_TRUE;
            ia_results->awb_meas_enabled = BOOL_TRUE;
            ia_results->lsc_enabled = BOOL_TRUE;
            ia_results->ctk_enabled = BOOL_TRUE;
        }

        if (mCamIAEngine->getADPFResults(&ia_results->adpf) == RET_SUCCESS)
        {
            if (ia_results->adpf.actives & ADPF_MASK)
            {
                ia_results->active |= CAMIA10_DPF_MASK;
                ia_results->adpf_enabled = BOOL_TRUE;
            }
            if (ia_results->adpf.actives & ADPF_STRENGTH_MASK)
            {
                ia_results->active |= CAMIA10_DPF_STRENGTH_MASK;
                ia_results->adpf_strength_enabled = BOOL_TRUE;
            }

            if (ia_results->adpf.actives & ADPF_DENOISE_SHARP_LEVEL_MASK)
            {
                flt_cfg.denoise_level = ia_results->adpf.denoise_level;
                flt_cfg.sharp_level = ia_results->adpf.sharp_level;
                flt_cfg.light_mode = mCamIA_DyCfg.LightMode;
                mFltEnabled = HAL_ISP_ACTIVE_SETTING;
                mFltNeededUpdate = BOOL_TRUE;
                runISPManual(ia_results, BOOL_FALSE);
                ia_results->flt.enabled = ia_results->adpf.FltEnable;
                ia_results->active |= CAMIA10_FLT_MASK;
            }

            if (ia_results->adpf.actives & ADPF_DEMOSAIC_TH_MASK)
            {
                bdm_cfg.demosaic_th = ia_results->adpf.demosaic_th;
                mBdmEnabled = HAL_ISP_ACTIVE_SETTING;
                mBdmNeededUpdate = BOOL_TRUE;
                runISPManual(ia_results, BOOL_FALSE);
                ia_results->bdm.enabled = BOOL_TRUE;
                ia_results->active |= CAMIA10_BDM_MASK;
            }

            if ((ia_results->adpf.actives & ADPF_DSP_3DNR_MASK) &&
				(m3DnrEnabled == HAL_ISP_ACTIVE_DEFAULT) )
            {
                ia_results->active |= CAMIA10_DSP_3DNR_MASK;
            }

			if ((ia_results->adpf.actives & ADPF_NEW_DSP_3DNR_MASK) &&
		        (mNew3DnrEnabled == HAL_ISP_ACTIVE_DEFAULT)) {
		        ia_results->active |= CAMIA10_NEW_DSP_3DNR_MASK;
		    }
		
			if (ia_results->adpf.actives & ADPF_DEMOSAICLP_MASK)	        
			{
				ia_results->rkDemosaicLP.lp_en = ia_results->adpf.RKDemosaicLpResult.lp_en;
				ia_results->rkDemosaicLP.rb_filter_en = ia_results->adpf.RKDemosaicLpResult.rb_filter_en;
				ia_results->rkDemosaicLP.hp_filter_en = ia_results->adpf.RKDemosaicLpResult.hp_filter_en;
				ia_results->rkDemosaicLP.th_grad = ia_results->adpf.RKDemosaicLpResult.th_grad;
				ia_results->rkDemosaicLP.th_diff = ia_results->adpf.RKDemosaicLpResult.th_diff;
				ia_results->rkDemosaicLP.th_csc = ia_results->adpf.RKDemosaicLpResult.th_csc;
				ia_results->rkDemosaicLP.th_var = ia_results->adpf.RKDemosaicLpResult.th_var;
				ia_results->rkDemosaicLP.th_var_en = ia_results->adpf.RKDemosaicLpResult.th_var_en;
				ia_results->rkDemosaicLP.th_csc_en = ia_results->adpf.RKDemosaicLpResult.th_csc_en;
				ia_results->rkDemosaicLP.th_diff_en = ia_results->adpf.RKDemosaicLpResult.th_diff_en;
				ia_results->rkDemosaicLP.th_grad_en = ia_results->adpf.RKDemosaicLpResult.th_grad_en;
				ia_results->rkDemosaicLP.use_old_lp = ia_results->adpf.RKDemosaicLpResult.use_old_lp;
				ia_results->rkDemosaicLP.similarity_th = ia_results->adpf.RKDemosaicLpResult.similarity_th;
				ia_results->rkDemosaicLP.flat_level_sel = ia_results->adpf.RKDemosaicLpResult.flat_level_sel;
				ia_results->rkDemosaicLP.pattern_level_sel = ia_results->adpf.RKDemosaicLpResult.pattern_level_sel;
				ia_results->rkDemosaicLP.edge_level_sel = ia_results->adpf.RKDemosaicLpResult.edge_level_sel;
				ia_results->rkDemosaicLP.thgrad_r_fct = ia_results->adpf.RKDemosaicLpResult.thgrad_r_fct;
				ia_results->rkDemosaicLP.thdiff_r_fct = ia_results->adpf.RKDemosaicLpResult.thdiff_r_fct;
				ia_results->rkDemosaicLP.thvar_r_fct = ia_results->adpf.RKDemosaicLpResult.thvar_r_fct;
				ia_results->rkDemosaicLP.thgrad_b_fct = ia_results->adpf.RKDemosaicLpResult.thgrad_b_fct;
				ia_results->rkDemosaicLP.thdiff_b_fct = ia_results->adpf.RKDemosaicLpResult.thdiff_b_fct;
				ia_results->rkDemosaicLP.thvar_b_fct = ia_results->adpf.RKDemosaicLpResult.thvar_b_fct;
				memcpy(ia_results->rkDemosaicLP.lu_divided,
						ia_results->adpf.RKDemosaicLpResult.lu_divided,
						sizeof(ia_results->rkDemosaicLP.lu_divided));
				memcpy(ia_results->rkDemosaicLP.thgrad_divided,
						ia_results->adpf.RKDemosaicLpResult.thgrad_divided,
						sizeof(ia_results->rkDemosaicLP.thgrad_divided));
				memcpy(ia_results->rkDemosaicLP.thdiff_divided,
						ia_results->adpf.RKDemosaicLpResult.thdiff_divided,
						sizeof(ia_results->rkDemosaicLP.thdiff_divided));
				memcpy(ia_results->rkDemosaicLP.thcsc_divided,
						ia_results->adpf.RKDemosaicLpResult.thcsc_divided,
						sizeof(ia_results->rkDemosaicLP.thcsc_divided));
				memcpy(ia_results->rkDemosaicLP.thvar_divided,
						ia_results->adpf.RKDemosaicLpResult.thvar_divided,
						sizeof(ia_results->rkDemosaicLP.thvar_divided));
				
				ia_results->active |= CAMIA10_DEMOSAICLP_MASK;		
			}
	        

			if (ia_results->adpf.actives & ADPF_RKIESHARP_MASK)
	        {
	        	ia_results->rkIEsharp.iesharpen_en = ia_results->adpf.RKIESharpResult.iesharpen_en;	
				ia_results->rkIEsharp.coring_thr = ia_results->adpf.RKIESharpResult.coring_thr;	
				ia_results->rkIEsharp.full_range = ia_results->adpf.RKIESharpResult.full_range;	
				ia_results->rkIEsharp.switch_avg = ia_results->adpf.RKIESharpResult.switch_avg;	
				memcpy(ia_results->rkIEsharp.yavg_thr,
						ia_results->adpf.RKIESharpResult.yavg_thr,
						sizeof(ia_results->rkIEsharp.yavg_thr));
				memcpy(ia_results->rkIEsharp.delta1,
						ia_results->adpf.RKIESharpResult.delta1,
						sizeof(ia_results->rkIEsharp.delta1));
				memcpy(ia_results->rkIEsharp.delta2,
						ia_results->adpf.RKIESharpResult.delta2,
						sizeof(ia_results->rkIEsharp.delta2));
				memcpy(ia_results->rkIEsharp.maxnumber,
						ia_results->adpf.RKIESharpResult.maxnumber,
						sizeof(ia_results->rkIEsharp.maxnumber));
				memcpy(ia_results->rkIEsharp.minnumber,
						ia_results->adpf.RKIESharpResult.minnumber,
						sizeof(ia_results->rkIEsharp.minnumber));
				memcpy(ia_results->rkIEsharp.gauss_flat_coe,
						ia_results->adpf.RKIESharpResult.gauss_flat_coe,
						sizeof(ia_results->rkIEsharp.gauss_flat_coe));
				memcpy(ia_results->rkIEsharp.gauss_noise_coe,
						ia_results->adpf.RKIESharpResult.gauss_noise_coe,
						sizeof(ia_results->rkIEsharp.gauss_noise_coe));
				memcpy(ia_results->rkIEsharp.gauss_other_coe,
						ia_results->adpf.RKIESharpResult.gauss_other_coe,
						sizeof(ia_results->rkIEsharp.gauss_other_coe));
				memcpy(ia_results->rkIEsharp.uv_gauss_flat_coe,
						ia_results->adpf.RKIESharpResult.uv_gauss_flat_coe,
						sizeof(ia_results->rkIEsharp.uv_gauss_flat_coe));
				memcpy(ia_results->rkIEsharp.uv_gauss_noise_coe,
						ia_results->adpf.RKIESharpResult.uv_gauss_noise_coe,
						sizeof(ia_results->rkIEsharp.uv_gauss_noise_coe));
				memcpy(ia_results->rkIEsharp.uv_gauss_other_coe,
						ia_results->adpf.RKIESharpResult.uv_gauss_other_coe,
						sizeof(ia_results->rkIEsharp.uv_gauss_other_coe));		
				memcpy(ia_results->rkIEsharp.p_grad,
						ia_results->adpf.RKIESharpResult.p_grad,
						sizeof(ia_results->rkIEsharp.p_grad));
				memcpy(ia_results->rkIEsharp.sharp_factor,
						ia_results->adpf.RKIESharpResult.sharp_factor,
						sizeof(ia_results->rkIEsharp.sharp_factor));
				memcpy(ia_results->rkIEsharp.line1_filter_coe,
						ia_results->adpf.RKIESharpResult.line1_filter_coe,
						sizeof(ia_results->rkIEsharp.line1_filter_coe));
				memcpy(ia_results->rkIEsharp.line2_filter_coe,
						ia_results->adpf.RKIESharpResult.line2_filter_coe,
						sizeof(ia_results->rkIEsharp.line2_filter_coe));
				memcpy(ia_results->rkIEsharp.line3_filter_coe,
						ia_results->adpf.RKIESharpResult.line3_filter_coe,
						sizeof(ia_results->rkIEsharp.line3_filter_coe));
				memcpy(ia_results->rkIEsharp.lap_mat_coe,
						ia_results->adpf.RKIESharpResult.lap_mat_coe,
						sizeof(ia_results->rkIEsharp.lap_mat_coe));
			
			
            ia_results->active |= CAMIA10_RKIESHARP_MASK;		
		}

        }

        if (mCamIAEngine->getAWDRResults(&ia_results->awdr) == RET_SUCCESS)
        {
            if (ia_results->awdr.actives & AWDR_WDR_MAXGAIN_LEVEL_MASK)
            {
            	ia_results->wdr.enabled = ia_results->awdr.wdr_enable;
				ia_results->wdr.mode = (CameraIcWdrMode_t)(ia_results->awdr.mode);
				memcpy(ia_results->wdr.segment,
						ia_results->awdr.wdr_dx,
						sizeof(ia_results->wdr.segment));

				memcpy(ia_results->wdr.wdr_block_y,
						ia_results->awdr.wdr_block_dy,
						sizeof(ia_results->wdr.wdr_block_y));

				memcpy(ia_results->wdr.wdr_global_y,
						ia_results->awdr.wdr_global_dy,
						sizeof(ia_results->wdr.wdr_global_y));

				ia_results->wdr.wdr_noiseratio = ia_results->awdr.wdr_noiseratio;
				ia_results->wdr.wdr_bestlight = ia_results->awdr.wdr_bestlight;
				ia_results->wdr.wdr_gain_off1 = ia_results->awdr.wdr_gain_off1;
				ia_results->wdr.wdr_pym_cc = ia_results->awdr.wdr_pym_cc;
				ia_results->wdr.wdr_epsilon = ia_results->awdr.wdr_epsilon;
				ia_results->wdr.wdr_lvl_en = ia_results->awdr.wdr_lvl_en;
				ia_results->wdr.wdr_flt_sel = ia_results->awdr.wdr_flt_sel;
				ia_results->wdr.wdr_gain_max_clip_enable = ia_results->awdr.wdr_gain_max_clip_enable;
				ia_results->wdr.wdr_gain_max_value = ia_results->awdr.wdr_gain_max_value;
				ia_results->wdr.wdr_bavg_clip = ia_results->awdr.wdr_bavg_clip;
				ia_results->wdr.wdr_nonl_segm = ia_results->awdr.wdr_nonl_segm;
				ia_results->wdr.wdr_nonl_open = ia_results->awdr.wdr_nonl_open;
				ia_results->wdr.wdr_nonl_mode1 = ia_results->awdr.wdr_nonl_mode1;
				ia_results->wdr.wdr_coe0 = ia_results->awdr.wdr_coe0;
				ia_results->wdr.wdr_coe1 = ia_results->awdr.wdr_coe1;
				ia_results->wdr.wdr_coe2 = ia_results->awdr.wdr_coe2;
				ia_results->wdr.wdr_coe_off = ia_results->awdr.wdr_coe_off;
			
                ia_results->active |= CAMIA10_WDR_MASK;
            }
        }

        if (mCamIAEngine->getAFResults(&ia_results->af) == RET_SUCCESS)
        {
            ia_results->active |= CAMIA10_AFC_MASK;
            ia_results->afc_meas_enabled = BOOL_TRUE;
        }
    }

    return true;
}


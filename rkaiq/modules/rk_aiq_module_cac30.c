#include "rk_aiq_isp33_modules.h"

#define RKCAC_V12_EDGE_DETECT_FIX_BITS	4
#define RKCAC_STRENGTH_FIX_BITS    7
// fix bits
#define		RKCAC_V12_PSF_RADIUS_X					3
#define		RKCAC_V12_PSF_RADIUS_Y					2
#define		RKCAC_V12_FIX_BIT_HI_DRCT_RATIO			10
#define		RKCAC_V12_FIX_BIT_RATIO					20
#define		RKCAC_V12_FIX_BIT_RATIO_SLOPE			10
#define		RKCAC_V12_FIX_BIT_LUMA_SLOPE			20
#define		RKCAC_V12_FIX_BIT_WGT					10
#define		RKCAC_V12_SUM_WGT						(1 << RKCAC_V12_FIX_BIT_WGT)
#define		RKCAC_V12_FIX_BIT_SEARCH_RANGE_RATIO	10
#define		RKCAC_V12_FIX_BIT_CHROMA_RATIO			10
#define		RKCAC_V12_FIX_BIT_DIV_WGT 				10

#define MALLOC(ptr, type, size)               do{(ptr) = (type *)malloc((sizeof(type))*(size)); \
                                                if((ptr) == NULL){ printf("Malloc Fail!\n"); exit(1);} \
                                                memset(ptr, 0, (sizeof(type))*(size));\
                                              }while(0);
#define FREE(ptr)                           do{\
                                                if((ptr) != NULL){ free(ptr); (ptr) = NULL;}\
                                            }while(0);


static void cac30_multi_cvt(struct isp33_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta,int maxValue, bool is_multi_isp);

void rk_aiq_cac30_params_cvt(void* attr, struct isp33_cac_cfg* cac_cfg,
    struct isp33_cac_cfg* cac_cfg_right, bool is_multi_isp, common_cvt_info_t* cvtinfo)
{
    struct isp33_cac_cfg* phwcfg = cac_cfg;
    cac_param_t *cac_param = (cac_param_t*)attr;
    cac_params_dyn_t* pdyn = &cac_param->dyn;
    cac_params_static_t* psta = &cac_param->sta;

    int maxValue = (1<<12)-1; //to do hdr ratio predgain
    RKAiqAecExpInfo_t *ae_exp = cvtinfo->ae_exp;
    float hdrmge_gain0_1 = 1;
    float isp_ob_predgain = 1;
    float preDGain_cac = cvtinfo->preDGain_drc;
    if (cvtinfo->frameNum <= 1) {
        isp_ob_predgain = preDGain_cac>1?preDGain_cac:1;
    }

    if(ae_exp!=NULL){
        if(cvtinfo->frameNum > 1){
            float sExpo = ae_exp->HdrExp[0].exp_real_params.analog_gain *
                          ae_exp->HdrExp[0].exp_real_params.digital_gain *
                          ae_exp->HdrExp[0].exp_real_params.integration_time;

            float lExpo = ae_exp->HdrExp[1].exp_real_params.analog_gain *
                          ae_exp->HdrExp[1].exp_real_params.digital_gain *
                          ae_exp->HdrExp[1].exp_real_params.integration_time;
            if(sExpo > 0.001) {
                hdrmge_gain0_1 = lExpo / sExpo;
            }

        }
        //LOGD_ACAC("hdrmge_gain0_1(%f)",hdrmge_gain0_1);
    }else{
       LOGD_ACAC("%s ae_exp is null",__FUNCTION__);
    }
    maxValue*=hdrmge_gain0_1*isp_ob_predgain;
    LOGD_ACAC("isp_ob_predgain(%f),hdrmge_gain0_1(%f),maxValue(%d)",isp_ob_predgain,hdrmge_gain0_1,maxValue);

    cac30_multi_cvt(phwcfg, pdyn, psta, maxValue, is_multi_isp);
    if (is_multi_isp) {
        phwcfg = cac_cfg_right;
        cac30_multi_cvt(phwcfg, pdyn, psta, maxValue, is_multi_isp);
    }
}

static void CreateKernelCoeffs(int radius_x, int radius_y, float rsigma, int *kernel_coeffs, int fix_bits)
{
	double e 		= 2.71828182845905;

	float *coeff_float;
	MALLOC(coeff_float, float, (2 * radius_y + 1) * (2 * radius_x + 1));

	float sumTable = 0;
	for(int y = -radius_y; y <= radius_y; y ++)
	{
		for(int x = -radius_x; x <= radius_x; x ++)
		{
			float dis 	= y * y + x * x;
			float coeff = pow(e, -dis / 2.0 / rsigma / rsigma);

			coeff_float[(y + radius_y) * (2 * radius_x + 1) + x + radius_x] = coeff;
			sumTable += coeff;
		}
	}
	int sum_coeff = 0;
	for(int y = -radius_y; y <= radius_y; y ++)
	{
		for(int x = -radius_x; x <= radius_x; x ++)
		{
			float coeff	= coeff_float[(y + radius_y) * (2 * radius_x + 1) + x + radius_x];
			coeff 		= coeff / sumTable;
			coeff 		= ROUND_F(coeff * (1 << fix_bits));
			kernel_coeffs[(y + radius_y) * (2 * radius_x + 1) + x + radius_x] = coeff;
			sum_coeff 	+= coeff;
		}
	}

	// check coeff
	int offset = (1 << fix_bits) - sum_coeff;
	kernel_coeffs[radius_y * (2 * radius_x + 1) + radius_x] = kernel_coeffs[radius_y * (2 * radius_x + 1) + radius_x] + offset;

	FREE(coeff_float);
}

static void cac30_multi_cvt(struct isp33_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta, int maxValue,bool is_multi_isp) {

    int index = (int)is_multi_isp;
    //phwcfg->bypass_en = pdyn->globalCtrl.hw_cacT_globalCtrl_bypass;
    //////////////////////////////////////////////////////////////////////////
    phwcfg->psf_table_fix_bit   = psta->hw_cacT_psfTableFix_bit;
    // generate psf coeff
    int bluePsf_coeffs[(2 * RKCAC_V12_PSF_RADIUS_Y + 1) * (2 * RKCAC_V12_PSF_RADIUS_X + 1)];
    int redPsf_coeffs[(2 * RKCAC_V12_PSF_RADIUS_Y + 1) * (2 * RKCAC_V12_PSF_RADIUS_X + 1)];
    CreateKernelCoeffs(
        RKCAC_V12_PSF_RADIUS_X,
        RKCAC_V12_PSF_RADIUS_Y,
        pdyn->psfTable.hw_cacT_bluePsf_sigma,
        bluePsf_coeffs,
        phwcfg->psf_table_fix_bit  + 6);

    CreateKernelCoeffs(
        RKCAC_V12_PSF_RADIUS_X,
        RKCAC_V12_PSF_RADIUS_Y,
        pdyn->psfTable.hw_cacT_redPsf_sigma,
        redPsf_coeffs,
        phwcfg->psf_table_fix_bit  + 6);

    phwcfg->psf_b_ker[0] 	= bluePsf_coeffs[0]; //cxf 由于是对称的所以只存了部分
    phwcfg->psf_b_ker[1] 	= bluePsf_coeffs[1];
    phwcfg->psf_b_ker[2] 	= bluePsf_coeffs[2];
    phwcfg->psf_b_ker[3] 	= bluePsf_coeffs[3];

    phwcfg->psf_b_ker[4] 	= bluePsf_coeffs[7];
    phwcfg->psf_b_ker[5] 	= bluePsf_coeffs[8];
    phwcfg->psf_b_ker[6] 	= bluePsf_coeffs[9];
    phwcfg->psf_b_ker[7] 	= bluePsf_coeffs[10];

    phwcfg->psf_b_ker[8] 	= bluePsf_coeffs[14];
    phwcfg->psf_b_ker[9] 	= bluePsf_coeffs[15];
    phwcfg->psf_b_ker[10] 	= bluePsf_coeffs[16];

    //red
    phwcfg->psf_r_ker[0] 	= redPsf_coeffs[0];
    phwcfg->psf_r_ker[1] 	= redPsf_coeffs[1];
    phwcfg->psf_r_ker[2] 	= redPsf_coeffs[2];
    phwcfg->psf_r_ker[3] 	= redPsf_coeffs[3];

    phwcfg->psf_r_ker[4] 	= redPsf_coeffs[7];
    phwcfg->psf_r_ker[5] 	= redPsf_coeffs[8];
    phwcfg->psf_r_ker[6] 	= redPsf_coeffs[9];
    phwcfg->psf_r_ker[7] 	= redPsf_coeffs[10];

    phwcfg->psf_r_ker[8] 	= redPsf_coeffs[14];
    phwcfg->psf_r_ker[9] 	= redPsf_coeffs[15];
    phwcfg->psf_r_ker[10] 	= redPsf_coeffs[16];

    //////////////////////////////////////////////////////////////////////////
    // g_interp
    phwcfg->hi_drct_ratio       = ROUND_F(pdyn->gInterp.hw_cacT_hiDrct_ratio * (1 << RKCAC_V12_FIX_BIT_HI_DRCT_RATIO));

    //////////////////////////////////////////////////////////////////////////
    // cac_edge_detect
    phwcfg->edge_detect_en		= pdyn->edgeDetect.hw_cacT_edgeDetect_en;
    phwcfg->flat_thred			= ROUND_F(pdyn->edgeDetect.hw_cacT_flat_thred * (1 << RKCAC_V12_EDGE_DETECT_FIX_BITS));
    phwcfg->flat_offset			= ROUND_F(pdyn->edgeDetect.hw_cacT_flat_offset);

    //////////////////////////////////////////////////////////////////////////
    // filter chroma, hw[0] : coeff of center;
    phwcfg->chroma_lo_flt_coeff0	=  pdyn->chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1;
    phwcfg->chroma_lo_flt_coeff1	=  pdyn->chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0;
    int tmp=pdyn->chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1+2*pdyn->chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0;
    if(tmp>7){
        LOGE_ACAC("hw_cacT_chromaLoFlt_coeff1+2*hw_cacT_chromaLoFlt_coeff0 = %d,which shoulde be <= 7!!!!",tmp);
    }else if(tmp<1){
        LOGE_ACAC("hw_cacT_chromaLoFlt_coeff1+2*hw_cacT_chromaLoFlt_coeff0 = %d,which shoulde be >= 1!!!!",tmp);
    }
    //min_chroma
    phwcfg->search_range_ratio	= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_searchRange_ratio * (1 << RKCAC_V12_FIX_BIT_SEARCH_RANGE_RATIO));

    phwcfg->wgt_color_b_min_thred	= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtColorB_minThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    int		hw_cacT_wgtColorB_maxThred	= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtColorB_maxThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    float	sw_cacT_wgtColorB_slope		= 1.0f / MAX( pdyn->chromaAberrCorr.hw_cacT_wgtColorB_maxThred -  pdyn->chromaAberrCorr.hw_cacT_wgtColorB_minThred, 0.001);
    sw_cacT_wgtColorB_slope 			= MIN(sw_cacT_wgtColorB_slope, 1024);
    phwcfg->wgt_color_b_slope		= MAX(1, ROUND_F(sw_cacT_wgtColorB_slope * (1 << RKCAC_V12_FIX_BIT_RATIO_SLOPE)));

    phwcfg->wgt_color_r_min_thred	= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtColorR_minThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    int		hw_cacT_wgtColorR_maxThred	= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtColorR_maxThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    float	sw_cacT_wgtColorR_slope		= 1.0f / MAX( pdyn->chromaAberrCorr.hw_cacT_wgtColorR_maxThred -  pdyn->chromaAberrCorr.hw_cacT_wgtColorR_minThred, 0.001);

    sw_cacT_wgtColorR_slope 			= MIN(sw_cacT_wgtColorR_slope, 1024);
    phwcfg->wgt_color_r_slope		= MAX(1, ROUND_F(sw_cacT_wgtColorR_slope * (1 << RKCAC_V12_FIX_BIT_RATIO_SLOPE)));
    phwcfg->wgt_color_min_luma		= ROUND_F(maxValue *pdyn->chromaAberrCorr.hw_cacT_wgtColor_minLuma);
    int sw_cacT_wgtColor_maxLuma			= ROUND_F(maxValue *pdyn->chromaAberrCorr.hw_cacT_wgtColor_maxLuma);
    float sw_cacT_wgtColorLuma_slope		= 1.0f / MAX(sw_cacT_wgtColor_maxLuma - phwcfg->wgt_color_min_luma, 0.01);
    sw_cacT_wgtColorLuma_slope 				= MIN(sw_cacT_wgtColorLuma_slope, 16);
    phwcfg->wgt_color_luma_slope		= MAX(1, ROUND_F(sw_cacT_wgtColorLuma_slope * (1 << RKCAC_V12_FIX_BIT_LUMA_SLOPE)));

    phwcfg->residual_chroma_ratio= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_residualChroma_ratio * (1 << RKCAC_V12_FIX_BIT_CHROMA_RATIO));

    phwcfg->color_lo_flt_coeff0=  pdyn->chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1;
    phwcfg->color_lo_flt_coeff1=  pdyn->chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0;
    tmp=pdyn->chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1+2*pdyn->chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0;
    if(tmp>7){
        LOGE_ACAC("hw_cacT_wgtColorLoFlt_coeff1+2*hw_cacT_wgtColorLoFlt_coeff0 = %d,which shoulde be <= 7!!!!",tmp);
    }else if(tmp<1){
        LOGE_ACAC("hw_cacT_wgtColorLoFlt_coeff1+2*hw_cacT_wgtColorLoFlt_coeff0 = %d,which shoulde be >= 1!!!!",tmp);
    }
    //////////////////////////////////////////////////////////////////////////
    //fusion_chroma

    phwcfg->wgt_over_expo_min_thred	= ROUND_F(maxValue *pdyn->chromaAberrCorr.hw_cacT_wgtOverExpo_minThred);
    int hw_cacT_wgtOverExpo_maxThred		= ROUND_F(maxValue *pdyn->chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred);
    float sw_cacT_wgtOverExpo_slope			= 1.0f / MAX(hw_cacT_wgtOverExpo_maxThred -  phwcfg->wgt_over_expo_min_thred, 0.01);
    sw_cacT_wgtOverExpo_slope 				= MIN(sw_cacT_wgtOverExpo_slope, 16);
    phwcfg->wgt_over_expo_slope		= MAX(1, ROUND_F(sw_cacT_wgtOverExpo_slope * (1 << RKCAC_V12_FIX_BIT_LUMA_SLOPE)));


    phwcfg->wgt_contrast_offset 	=  pdyn->chromaAberrCorr.hw_cacT_contrast_offset;
    phwcfg->wgt_contrast_min_thred= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtContrast_minThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    int		sw_cacT_wgtContrast_maxThred= ROUND_F( pdyn->chromaAberrCorr.hw_cacT_wgtContrast_maxThred * (1 << RKCAC_V12_FIX_BIT_RATIO));
    float sw_cacT_wgtContrast_slope		= 1.0f / MAX( pdyn->chromaAberrCorr.hw_cacT_wgtContrast_maxThred -  pdyn->chromaAberrCorr.hw_cacT_wgtContrast_minThred, 0.01);
    sw_cacT_wgtContrast_slope 			= MIN(sw_cacT_wgtContrast_slope, 16);
    phwcfg->wgt_contrast_slope	= MAX(1, ROUND_F(sw_cacT_wgtContrast_slope * (1 << RKCAC_V12_FIX_BIT_RATIO_SLOPE)));

    // dark area, wgt
     phwcfg->wgt_dark_thed	= ROUND_F(maxValue * pdyn->chromaAberrCorr.hw_cacT_wgtDarkArea_minThred);
    int 	sw_cacT_wgtDarkArea_maxThred	= ROUND_F(maxValue * pdyn->chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred);

    float sw_cacT_wgtDarkArea_slope 		= 1.0f / MAX(sw_cacT_wgtDarkArea_maxThred - phwcfg->wgt_dark_thed, 0.01);
    sw_cacT_wgtDarkArea_slope				= MIN(sw_cacT_wgtDarkArea_slope, 16);
     phwcfg->wgt_dark_slope		= MAX(1, ROUND_F(sw_cacT_wgtDarkArea_slope * (1 << RKCAC_V12_FIX_BIT_LUMA_SLOPE)));

    phwcfg->wgt_color_en 		=  pdyn->chromaAberrCorr.hw_cacT_wgtColor_en;


    //////////////////////////////////////////////////////////////////////////
     phwcfg->edge_detect_en		= ROUND_F(pdyn->edgeDetect.hw_cacT_edgeDetect_en);
     phwcfg->flat_thred			= ROUND_F(pdyn->edgeDetect.hw_cacT_flat_thred * (1 << RKCAC_V12_EDGE_DETECT_FIX_BITS));
     phwcfg->flat_offset			= ROUND_F(pdyn->edgeDetect.hw_cacT_flat_offset);

    //////////////////////////////////////////////////////////////////////////
    phwcfg->neg_clip0_en = pdyn->hfCalc.hw_cacT_negClip0_en;
    bool hw_cacT_overExpoDetect_en			=  pdyn->hfCalc.hw_cacT_overExpoDetect_en;
    if (hw_cacT_overExpoDetect_en)
    {
        phwcfg->over_expo_thred		= ROUND_F(pdyn->hfCalc.hw_cacT_overExpo_thred * maxValue);
        phwcfg->over_expo_adj		= ROUND_F(pdyn->hfCalc.hw_cacT_overExpo_adj * maxValue);
    }
    else
    {
        phwcfg->over_expo_thred		= maxValue;
        phwcfg->over_expo_adj		= maxValue;
    }

    LOGD_ACAC("bypass_en: %d\n", phwcfg->bypass_en);
    LOGD_ACAC("edge_detect_en: %d\n", phwcfg->edge_detect_en);
    LOGD_ACAC("neg_clip0_en: %d\n", phwcfg->neg_clip0_en);
    LOGD_ACAC("wgt_color_en: %d\n", phwcfg->wgt_color_en);
    LOGD_ACAC("psf_table_fix_bit: %d\n", phwcfg->psf_table_fix_bit);
    LOGD_ACAC("hi_drct_ratio: %d\n", phwcfg->hi_drct_ratio);
    LOGD_ACAC("over_expo_thred: %d\n", phwcfg->over_expo_thred);
    LOGD_ACAC("over_expo_adj: %d\n", phwcfg->over_expo_adj);
    LOGD_ACAC("flat_thred: %d\n", phwcfg->flat_thred);
    LOGD_ACAC("flat_offset: %d\n", phwcfg->flat_offset);
    LOGD_ACAC("chroma_lo_flt_coeff0: %d\n", phwcfg->chroma_lo_flt_coeff0);
    LOGD_ACAC("chroma_lo_flt_coeff1: %d\n", phwcfg->chroma_lo_flt_coeff1);
    LOGD_ACAC("color_lo_flt_coeff0: %d\n", phwcfg->color_lo_flt_coeff0);
    LOGD_ACAC("color_lo_flt_coeff1: %d\n", phwcfg->color_lo_flt_coeff1);
    LOGD_ACAC("search_range_ratio: %d\n", phwcfg->search_range_ratio);
    LOGD_ACAC("residual_chroma_ratio: %d\n", phwcfg->residual_chroma_ratio);
    LOGD_ACAC("wgt_color_b_min_thred: %d\n", phwcfg->wgt_color_b_min_thred);
    LOGD_ACAC("wgt_color_r_min_thred: %d\n", phwcfg->wgt_color_r_min_thred);
    LOGD_ACAC("wgt_color_b_slope: %d\n", phwcfg->wgt_color_b_slope);
    LOGD_ACAC("wgt_color_r_slope: %d\n", phwcfg->wgt_color_r_slope);
    LOGD_ACAC("wgt_color_min_luma: %d\n", phwcfg->wgt_color_min_luma);
    LOGD_ACAC("wgt_color_luma_slope: %d\n", phwcfg->wgt_color_luma_slope);
    LOGD_ACAC("wgt_over_expo_min_thred: %d\n", phwcfg->wgt_over_expo_min_thred);
    LOGD_ACAC("wgt_over_expo_slope: %d\n", phwcfg->wgt_over_expo_slope);
    LOGD_ACAC("wgt_contrast_min_thred: %d\n", phwcfg->wgt_contrast_min_thred);
    LOGD_ACAC("wgt_contrast_slope: %d\n", phwcfg->wgt_contrast_slope);
    LOGD_ACAC("wgt_contrast_offset: %d\n", phwcfg->wgt_contrast_offset);
    LOGD_ACAC("wgt_dark_thed: %d\n", phwcfg->wgt_dark_thed);
    LOGD_ACAC("wgt_dark_slope: %d\n", phwcfg->wgt_dark_slope);
    // 打印 psf_b_ker 数组
    LOGD_ACAC("psf_b_ker: ");
    for (int i = 0; i < ISP33_CAC_PSF_NUM; i++) {
        LOGD_ACAC("%d ", phwcfg->psf_b_ker[i]);
    }
    LOGD_ACAC("\n");
    // 打印 psf_r_ker 数组
    LOGD_ACAC("psf_r_ker: ");
    for (int i = 0; i < ISP33_CAC_PSF_NUM; i++) {
        LOGD_ACAC("%d ", phwcfg->psf_r_ker[i]);
    }
    LOGD_ACAC("\n");
}

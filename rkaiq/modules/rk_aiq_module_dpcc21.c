#include "rk_aiq_isp39_modules.h"

void rk_aiq_dpcc21_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg)
{
    struct isp39_dpcc_cfg* phwcfg = &isp_cfg->others.dpcc_cfg;
    dpc_param_t *dpc_attrib = (dpc_param_t*) attr;
    dpc_params_dyn_t* pdyn = &dpc_attrib->dyn;
    dpc_params_static_t* psta = &dpc_attrib->sta;

    //mode 0x0000
    phwcfg->stage1_enable = 1;
    phwcfg->grayscale_mode =
        psta->hw_dpcCfg_src_fmt == dpc_srcFmt_bayer ? 0 : 1;
    //phwcfg->enable = pdyn->arBasic.enable;

    //output_mode 0x0004
    phwcfg->border_bypass_mode = (int)psta->hw_dpcCfg_dpcROI_mode;
    phwcfg->sw_rk_out_sel =
        pdyn->dpcProc.hw_dpcT_dpcByMux_mode == dpc_dpcByMedFilt_mode ? 0 :
        (int)pdyn->dpcProc.hw_dpcT_dpcByMux_mode - 1;
    phwcfg->sw_dpcc_output_sel =
        pdyn->dpcProc.hw_dpcT_dpcByMux_mode == dpc_dpcByMedFilt_mode ? 0 : 1;
    phwcfg->stage1_rb_3x3 =
        pdyn->dpcProc.hw_dpcT_medFiltRB_mode == dpc_medEntireKernel_mode ? 0 : 1;
    phwcfg->stage1_g_3x3 =
        pdyn->dpcProc.hw_dpcT_medFiltG_mode == dpc_medEntireKernel_mode ? 0 : 1;
    phwcfg->stage1_incl_rb_center =
        pdyn->dpcProc.hw_dpcT_medFiltG_mode == dpc_medNrstNhoodDpInc_mode ? 1 : 0;
    phwcfg->stage1_incl_green_center =
        pdyn->dpcProc.hw_dpcT_medFiltG_mode == dpc_medNrstNhoodDpInc_mode ? 1 : 0;

    //set_use 0x0008
    phwcfg->stage1_use_fix_set = pdyn->dpDct_fixEngine.hw_dpcT_engine_en;
    phwcfg->stage1_use_set_3 = pdyn->dpDct_cfgEngine[2].hw_dpcT_engine_en;
    phwcfg->stage1_use_set_2 = pdyn->dpDct_cfgEngine[1].hw_dpcT_engine_en;
    phwcfg->stage1_use_set_1 = pdyn->dpDct_cfgEngine[0].hw_dpcT_engine_en;

    //methods_set_1 0x000c
    phwcfg->sw_rk_red_blue1_en = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dctRB_en;
    phwcfg->rg_red_blue1_enable = pdyn->dpDct_cfgEngine[0].dpDct_grad.hw_dpcT_dctRB_en;
    phwcfg->rnd_red_blue1_enable = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_dctRB_en;
    phwcfg->ro_red_blue1_enable = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en;
    phwcfg->lc_red_blue1_enable = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dctRB_en;
    phwcfg->pg_red_blue1_enable = pdyn->dpDct_cfgEngine[0].dpDct_peak.hw_dpcT_dctRB_en;
    phwcfg->sw_rk_green1_en = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dctG_en;
    phwcfg->rg_green1_enable = pdyn->dpDct_cfgEngine[0].dpDct_grad.hw_dpcT_dctG_en;
    phwcfg->rnd_green1_enable = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_dctG_en;
    phwcfg->ro_green1_enable = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en;
    phwcfg->lc_green1_enable = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dctG_en;
    phwcfg->pg_green1_enable = pdyn->dpDct_cfgEngine[0].dpDct_peak.hw_dpcT_dctG_en;

    //methods_set_2 0x0010
    phwcfg->sw_rk_red_blue2_en = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dctRB_en;
    phwcfg->rg_red_blue2_enable = pdyn->dpDct_cfgEngine[1].dpDct_grad.hw_dpcT_dctRB_en;
    phwcfg->rnd_red_blue2_enable = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_dctRB_en;
    phwcfg->ro_red_blue2_enable = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en;
    phwcfg->lc_red_blue2_enable = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dctRB_en;
    phwcfg->pg_red_blue2_enable = pdyn->dpDct_cfgEngine[1].dpDct_peak.hw_dpcT_dctRB_en;
    phwcfg->sw_rk_green2_en = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dctG_en;
    phwcfg->rg_green2_enable = pdyn->dpDct_cfgEngine[1].dpDct_grad.hw_dpcT_dctG_en;
    phwcfg->rnd_green2_enable = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_dctG_en;
    phwcfg->ro_green2_enable = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en;
    phwcfg->lc_green2_enable = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dctG_en;
    phwcfg->pg_green2_enable = pdyn->dpDct_cfgEngine[1].dpDct_peak.hw_dpcT_dctG_en;

    //methods_set_3 0x0014
    phwcfg->sw_rk_red_blue3_en = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dctRB_en;
    phwcfg->rg_red_blue3_enable = pdyn->dpDct_cfgEngine[2].dpDct_grad.hw_dpcT_dctRB_en;
    phwcfg->rnd_red_blue3_enable = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_dctRB_en;
    phwcfg->ro_red_blue3_enable = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThRB_en;
    phwcfg->lc_red_blue3_enable = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dctRB_en;
    phwcfg->pg_red_blue3_enable = pdyn->dpDct_cfgEngine[2].dpDct_peak.hw_dpcT_dctRB_en;
    phwcfg->sw_rk_green3_en = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dctG_en;
    phwcfg->rg_green3_enable = pdyn->dpDct_cfgEngine[2].dpDct_grad.hw_dpcT_dctG_en;
    phwcfg->rnd_green3_enable = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_dctG_en;
    phwcfg->ro_green3_enable = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dctByDpIdxThG_en;
    phwcfg->lc_green3_enable = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dctG_en;
    phwcfg->pg_green3_enable = pdyn->dpDct_cfgEngine[2].dpDct_peak.hw_dpcT_dctG_en;

    //line_thresh_1 0x0018
    phwcfg->sw_mindis1_rb = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit;
    phwcfg->sw_mindis1_g = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit;
    phwcfg->line_thr_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dpEdgThRB_offset;
    phwcfg->line_thr_1_g = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dpEdgThG_offset;

    //line_mad_fac_1 0x001c
    phwcfg->sw_dis_scale_min1 = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale;
    phwcfg->sw_dis_scale_max1 = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale;
    phwcfg->line_mad_fac_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dpEdgThRB_scale;
    phwcfg->line_mad_fac_1_g = pdyn->dpDct_cfgEngine[0].dpDct_edg.hw_dpcT_dpEdgThG_scale;

    //pg_fac_1 0x0020
    phwcfg->pg_fac_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_peak.hw_dpcT_dpPeakThRb_scale;
    phwcfg->pg_fac_1_g = pdyn->dpDct_cfgEngine[0].dpDct_peak.hw_dpcT_dpPeakThG_scale;

    //rnd_thresh_1 0x0024
    phwcfg->rnd_thr_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread;
    phwcfg->rnd_thr_1_g = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_dpLumaG_thread;

    //rg_fac_1 0x0028
    phwcfg->rg_fac_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_grad.hw_dpcT_dpGradThRB_scale;
    phwcfg->rg_fac_1_g = pdyn->dpDct_cfgEngine[0].dpDct_grad.hw_dpcT_dpGradThG_scale;


    //line_thresh_2 0x002c
    phwcfg->sw_mindis2_rb = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit;
    phwcfg->sw_mindis2_g = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit;
    phwcfg->line_thr_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dpEdgThRB_offset;
    phwcfg->line_thr_2_g = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dpEdgThG_offset;

    //line_mad_fac_2 0x0030
    phwcfg->sw_dis_scale_min2 = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale;
    phwcfg->sw_dis_scale_max2 = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale;
    phwcfg->line_mad_fac_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dpEdgThRB_scale;
    phwcfg->line_mad_fac_2_g = pdyn->dpDct_cfgEngine[1].dpDct_edg.hw_dpcT_dpEdgThG_scale;

    //pg_fac_2 0x0034
    phwcfg->pg_fac_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_peak.hw_dpcT_dpPeakThRb_scale;
    phwcfg->pg_fac_2_g = pdyn->dpDct_cfgEngine[1].dpDct_peak.hw_dpcT_dpPeakThG_scale;

    //rnd_thresh_2 0x0038
    phwcfg->rnd_thr_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread;
    phwcfg->rnd_thr_2_g = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_dpLumaG_thread;

    //rg_fac_2 0x003c
    phwcfg->rg_fac_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_grad.hw_dpcT_dpGradThRB_scale;
    phwcfg->rg_fac_2_g = pdyn->dpDct_cfgEngine[1].dpDct_grad.hw_dpcT_dpGradThG_scale;


    //line_thresh_3 0x0040
    phwcfg->sw_mindis3_rb = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_norDistRB_minLimit;
    phwcfg->sw_mindis3_g = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_norDistG_minLimit;
    phwcfg->line_thr_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dpEdgThRB_offset;
    phwcfg->line_thr_3_g = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dpEdgThG_offset;

    //line_mad_fac_3 0x0044
    phwcfg->sw_dis_scale_min3 = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dist2DarkDpTh_scale;
    phwcfg->sw_dis_scale_max3 = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_dist2LightDpTh_scale;
    phwcfg->line_mad_fac_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dpEdgThRB_scale;
    phwcfg->line_mad_fac_3_g = pdyn->dpDct_cfgEngine[2].dpDct_edg.hw_dpcT_dpEdgThG_scale;

    //pg_fac_3 0x0048
    phwcfg->pg_fac_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_peak.hw_dpcT_dpPeakThRb_scale;
    phwcfg->pg_fac_3_g = pdyn->dpDct_cfgEngine[2].dpDct_peak.hw_dpcT_dpPeakThG_scale;

    //rnd_thresh_3 0x004c
    phwcfg->rnd_thr_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_dpLumaRB_thread;
    phwcfg->rnd_thr_3_g = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_dpLumaG_thread;

    //rg_fac_3 0x0050
    phwcfg->rg_fac_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_grad.hw_dpcT_dpGradThRB_scale;
    phwcfg->rg_fac_3_g = pdyn->dpDct_cfgEngine[2].dpDct_grad.hw_dpcT_dpGradThG_scale;

    //ro_limits 0x0054
    phwcfg->ro_lim_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx;
    phwcfg->ro_lim_3_g = pdyn->dpDct_cfgEngine[2].dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx;
    phwcfg->ro_lim_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx;
    phwcfg->ro_lim_2_g = pdyn->dpDct_cfgEngine[1].dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx;
    phwcfg->ro_lim_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_ordDpThRB_idx;
    phwcfg->ro_lim_1_g = pdyn->dpDct_cfgEngine[0].dpDct_norDist2DpTh.hw_dpcT_ordDpThG_idx;

    //rnd_offs 0x0058
    phwcfg->rnd_offs_3_rb = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx;
    phwcfg->rnd_offs_3_g = pdyn->dpDct_cfgEngine[2].dpDct_dpThCfg.hw_dpcT_ordDpThG_idx;
    phwcfg->rnd_offs_2_rb = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx;
    phwcfg->rnd_offs_2_g = pdyn->dpDct_cfgEngine[1].dpDct_dpThCfg.hw_dpcT_ordDpThG_idx;
    phwcfg->rnd_offs_1_rb = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_ordDpThRB_idx;
    phwcfg->rnd_offs_1_g = pdyn->dpDct_cfgEngine[0].dpDct_dpThCfg.hw_dpcT_ordDpThG_idx;

    //pdaf_en 0x0070
    phwcfg->sw_pdaf_en = psta->spc.hw_dpcCfg_spc_en;

    //pdaf_point_en 0x0074
    for(int i = 0; i < ISP2X_DPCC_PDAF_POINT_NUM; i++) {
        phwcfg->pdaf_point_en[i] = psta->spc.hw_dpcCfg_spcEnInZone_bit[i];
    }

    //pdaf_offset 0x0078
    phwcfg->pdaf_offsety = psta->spc.hw_dpcCfg_win_y;
    phwcfg->pdaf_offsetx = psta->spc.hw_dpcCfg_win_x;

    //pdaf_wrap 0x007c
    phwcfg->pdaf_wrapy = psta->spc.hw_dpcCfg_zone_height;
    phwcfg->pdaf_wrapx = psta->spc.hw_dpcCfg_zone_width;

    //pdaf_scope 0x0080
    phwcfg->pdaf_wrapy_num = psta->spc.hw_dpcCfg_zonesCol_num;
    phwcfg->pdaf_wrapx_num = psta->spc.hw_dpcCfg_zonesRow_num;

    //pdaf_point_0 0x0084
    for(int i = 0; i < ISP2X_DPCC_PDAF_POINT_NUM; i++) {
        phwcfg->point[i].x = psta->spc.hw_dpcCfg_spInZoneCoord_x[i];
        phwcfg->point[i].y = psta->spc.hw_dpcCfg_spInZoneCoord_y[i];
    }

    //pdaf_forward_med 0x00a4
    phwcfg->pdaf_forward_med =
        psta->spc.hw_dpc_Cfg_spc_mode == dpc_spcByMedBackward_mode ? 0 : 1;
}

#include "rk_aiq_isp32_modules.h"

void rk_aiq_dpcc20_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    struct isp2x_dpcc_cfg* phwcfg = &isp_cfg->others.dpcc_cfg;
    dpcc_param_t *drc_attrib = (dpcc_param_t*) attr;
    dpcc_params_dyn_t* pdyn = &drc_attrib->dyn;
    dpcc_params_static_t* psta = &drc_attrib->sta;
    
    //mode 0x0000
    phwcfg->stage1_enable = pdyn->arBasic.stage1_enable;
    phwcfg->grayscale_mode = pdyn->arBasic.grayscale_mode;
    //phwcfg->enable = pdyn->arBasic.enable;

    //output_mode 0x0004
    phwcfg->sw_rk_out_sel = pdyn->arBasic.sw_rk_out_sel;
    phwcfg->sw_dpcc_output_sel = pdyn->arBasic.sw_dpcc_output_sel;
    phwcfg->stage1_rb_3x3 = pdyn->arBasic.stage1_rb_3x3;
    phwcfg->stage1_g_3x3 = pdyn->arBasic.stage1_g_3x3;
    phwcfg->stage1_incl_rb_center = pdyn->arBasic.stage1_incl_rb_center;
    phwcfg->stage1_incl_green_center = pdyn->arBasic.stage1_incl_green_center;

    //set_use 0x0008
    phwcfg->stage1_use_fix_set = pdyn->arBasic.stage1_use_fix_set;
    phwcfg->stage1_use_set_3 = pdyn->arBasic.stage1_use_set_3;
    phwcfg->stage1_use_set_2 = pdyn->arBasic.stage1_use_set_2;
    phwcfg->stage1_use_set_1 = pdyn->arBasic.stage1_use_set_1;

    //methods_set_1 0x000c
    phwcfg->sw_rk_red_blue1_en = pdyn->arBasic.sw_rk_red_blue1_en;
    phwcfg->rg_red_blue1_enable = pdyn->arBasic.rg_red_blue1_enable;
    phwcfg->rnd_red_blue1_enable = pdyn->arBasic.rnd_red_blue1_enable;
    phwcfg->ro_red_blue1_enable = pdyn->arBasic.ro_red_blue1_enable;
    phwcfg->lc_red_blue1_enable = pdyn->arBasic.lc_red_blue1_enable;
    phwcfg->pg_red_blue1_enable = pdyn->arBasic.pg_red_blue1_enable;
    phwcfg->sw_rk_green1_en = pdyn->arBasic.sw_rk_green1_en;
    phwcfg->rg_green1_enable = pdyn->arBasic.rg_green1_enable;
    phwcfg->rnd_green1_enable = pdyn->arBasic.rnd_green1_enable;
    phwcfg->ro_green1_enable = pdyn->arBasic.ro_green1_enable;
    phwcfg->lc_green1_enable = pdyn->arBasic.lc_green1_enable;
    phwcfg->pg_green1_enable = pdyn->arBasic.pg_green1_enable;

    //methods_set_2 0x0010
    phwcfg->sw_rk_red_blue2_en = pdyn->arBasic.sw_rk_red_blue2_en;
    phwcfg->rg_red_blue2_enable = pdyn->arBasic.rg_red_blue2_enable;
    phwcfg->rnd_red_blue2_enable = pdyn->arBasic.rnd_red_blue2_enable;
    phwcfg->ro_red_blue2_enable = pdyn->arBasic.ro_red_blue2_enable;
    phwcfg->lc_red_blue2_enable = pdyn->arBasic.lc_red_blue2_enable;
    phwcfg->pg_red_blue2_enable = pdyn->arBasic.pg_red_blue2_enable;
    phwcfg->sw_rk_green2_en = pdyn->arBasic.sw_rk_green2_en;
    phwcfg->rg_green2_enable = pdyn->arBasic.rg_green2_enable;
    phwcfg->rnd_green2_enable = pdyn->arBasic.rnd_green2_enable;
    phwcfg->ro_green2_enable = pdyn->arBasic.ro_green2_enable;
    phwcfg->lc_green2_enable = pdyn->arBasic.lc_green2_enable;
    phwcfg->pg_green2_enable = pdyn->arBasic.pg_green2_enable;

    //methods_set_3 0x0014
    phwcfg->sw_rk_red_blue3_en = pdyn->arBasic.sw_rk_red_blue3_en;
    phwcfg->rg_red_blue3_enable = pdyn->arBasic.rg_red_blue3_enable;
    phwcfg->rnd_red_blue3_enable = pdyn->arBasic.rnd_red_blue3_enable;
    phwcfg->ro_red_blue3_enable = pdyn->arBasic.ro_red_blue3_enable;
    phwcfg->lc_red_blue3_enable = pdyn->arBasic.lc_red_blue3_enable;
    phwcfg->pg_red_blue3_enable = pdyn->arBasic.pg_red_blue3_enable;
    phwcfg->sw_rk_green3_en = pdyn->arBasic.sw_rk_green3_en;
    phwcfg->rg_green3_enable = pdyn->arBasic.rg_green3_enable;
    phwcfg->rnd_green3_enable = pdyn->arBasic.rnd_green3_enable;
    phwcfg->ro_green3_enable = pdyn->arBasic.ro_green3_enable;
    phwcfg->lc_green3_enable = pdyn->arBasic.lc_green3_enable;
    phwcfg->pg_green3_enable = pdyn->arBasic.pg_green3_enable;

    //line_thresh_1 0x0018
    phwcfg->sw_mindis1_rb = pdyn->arBasic.sw_mindis1_rb;
    phwcfg->sw_mindis1_g = pdyn->arBasic.sw_mindis1_g;
    phwcfg->line_thr_1_rb = pdyn->arBasic.line_thr_1_rb;
    phwcfg->line_thr_1_g = pdyn->arBasic.line_thr_1_g;

    //line_mad_fac_1 0x001c
    phwcfg->sw_dis_scale_min1 = pdyn->arBasic.sw_dis_scale_min1;
    phwcfg->sw_dis_scale_max1 = pdyn->arBasic.sw_dis_scale_max1;
    phwcfg->line_mad_fac_1_rb = pdyn->arBasic.line_mad_fac_1_rb;
    phwcfg->line_mad_fac_1_g = pdyn->arBasic.line_mad_fac_1_g;

    //pg_fac_1 0x0020
    phwcfg->pg_fac_1_rb = pdyn->arBasic.pg_fac_1_rb;
    phwcfg->pg_fac_1_g = pdyn->arBasic.pg_fac_1_g;

    //rnd_thresh_1 0x0024
    phwcfg->rnd_thr_1_rb = pdyn->arBasic.rnd_thr_1_rb;
    phwcfg->rnd_thr_1_g = pdyn->arBasic.rnd_thr_1_g;

    //rg_fac_1 0x0028
    phwcfg->rg_fac_1_rb = pdyn->arBasic.rg_fac_1_rb;
    phwcfg->rg_fac_1_g = pdyn->arBasic.rg_fac_1_g;


    //line_thresh_2 0x002c
    phwcfg->sw_mindis2_rb = pdyn->arBasic.sw_mindis2_rb;
    phwcfg->sw_mindis2_g = pdyn->arBasic.sw_mindis2_g;
    phwcfg->line_thr_2_rb = pdyn->arBasic.line_thr_2_rb;
    phwcfg->line_thr_2_g = pdyn->arBasic.line_thr_2_g;

    //line_mad_fac_2 0x0030
    phwcfg->sw_dis_scale_min2 = pdyn->arBasic.sw_dis_scale_min2;
    phwcfg->sw_dis_scale_max2 = pdyn->arBasic.sw_dis_scale_max2;
    phwcfg->line_mad_fac_2_rb = pdyn->arBasic.line_mad_fac_2_rb;
    phwcfg->line_mad_fac_2_g = pdyn->arBasic.line_mad_fac_2_g;

    //pg_fac_2 0x0034
    phwcfg->pg_fac_2_rb = pdyn->arBasic.pg_fac_2_rb;
    phwcfg->pg_fac_2_g = pdyn->arBasic.pg_fac_2_g;

    //rnd_thresh_2 0x0038
    phwcfg->rnd_thr_2_rb = pdyn->arBasic.rnd_thr_2_rb;
    phwcfg->rnd_thr_2_g = pdyn->arBasic.rnd_thr_2_g;

    //rg_fac_2 0x003c
    phwcfg->rg_fac_2_rb = pdyn->arBasic.rg_fac_2_rb;
    phwcfg->rg_fac_2_g = pdyn->arBasic.rg_fac_2_g;


    //line_thresh_3 0x0040
    phwcfg->sw_mindis3_rb = pdyn->arBasic.sw_mindis3_rb;
    phwcfg->sw_mindis3_g = pdyn->arBasic.sw_mindis3_g;
    phwcfg->line_thr_3_rb = pdyn->arBasic.line_thr_3_rb;
    phwcfg->line_thr_3_g = pdyn->arBasic.line_thr_3_g;

    //line_mad_fac_3 0x0044
    phwcfg->sw_dis_scale_min3 = pdyn->arBasic.sw_dis_scale_min3;
    phwcfg->sw_dis_scale_max3 = pdyn->arBasic.sw_dis_scale_max3;
    phwcfg->line_mad_fac_3_rb = pdyn->arBasic.line_mad_fac_3_rb;
    phwcfg->line_mad_fac_3_g = pdyn->arBasic.line_mad_fac_3_g;

    //pg_fac_3 0x0048
    phwcfg->pg_fac_3_rb = pdyn->arBasic.pg_fac_3_rb;
    phwcfg->pg_fac_3_g = pdyn->arBasic.pg_fac_3_g;

    //rnd_thresh_3 0x004c
    phwcfg->rnd_thr_3_rb = pdyn->arBasic.rnd_thr_3_rb;
    phwcfg->rnd_thr_3_g = pdyn->arBasic.rnd_thr_3_g;

    //rg_fac_3 0x0050
    phwcfg->rg_fac_3_rb = pdyn->arBasic.rg_fac_3_rb;
    phwcfg->rg_fac_3_g = pdyn->arBasic.rg_fac_3_g;

    //ro_limits 0x0054
    phwcfg->ro_lim_3_rb = pdyn->arBasic.ro_lim_3_rb;
    phwcfg->ro_lim_3_g = pdyn->arBasic.ro_lim_3_g;
    phwcfg->ro_lim_2_rb = pdyn->arBasic.ro_lim_2_rb;
    phwcfg->ro_lim_2_g = pdyn->arBasic.ro_lim_2_g;
    phwcfg->ro_lim_1_rb = pdyn->arBasic.ro_lim_1_rb;
    phwcfg->ro_lim_1_g = pdyn->arBasic.ro_lim_1_g;

    //rnd_offs 0x0058
    phwcfg->rnd_offs_3_rb = pdyn->arBasic.rnd_offs_3_rb;
    phwcfg->rnd_offs_3_g = pdyn->arBasic.rnd_offs_3_g;
    phwcfg->rnd_offs_2_rb = pdyn->arBasic.rnd_offs_2_rb;
    phwcfg->rnd_offs_2_g = pdyn->arBasic.rnd_offs_2_g;
    phwcfg->rnd_offs_1_rb = pdyn->arBasic.rnd_offs_1_rb;
    phwcfg->rnd_offs_1_g = pdyn->arBasic.rnd_offs_1_g;

    //bpt_ctrl 0x005c
    phwcfg->bpt_rb_3x3 = psta->stBptParams.bpt_rb_3x3;
    phwcfg->bpt_g_3x3 = psta->stBptParams.bpt_g_3x3;
    phwcfg->bpt_incl_rb_center = psta->stBptParams.bpt_incl_rb_center;
    phwcfg->bpt_incl_green_center = psta->stBptParams.bpt_incl_green_center;
    phwcfg->bpt_use_fix_set = psta->stBptParams.bpt_use_fix_set;
    phwcfg->bpt_use_set_3 = psta->stBptParams.bpt_use_set_3;
    phwcfg->bpt_use_set_2 = psta->stBptParams.bpt_use_set_2;
    phwcfg->bpt_use_set_1 = psta->stBptParams.bpt_use_set_1;
    phwcfg->bpt_cor_en = psta->stBptParams.bpt_cor_en;
    phwcfg->bpt_det_en = psta->stBptParams.bpt_det_en;

    //bpt_number 0x0060
    phwcfg->bp_number = psta->stBptParams.bp_number;

    //bpt_addr 0x0064
    phwcfg->bp_table_addr = psta->stBptParams.bp_table_addr;

    //bpt_data 0x0068
    phwcfg->bpt_v_addr = psta->stBptParams.bpt_v_addr;
    phwcfg->bpt_h_addr = psta->stBptParams.bpt_h_addr;

    //bp_cnt 0x006c
    phwcfg->bp_cnt = psta->stBptParams.bp_cnt;

    //pdaf_en 0x0070
    phwcfg->sw_pdaf_en = psta->stPdafParams.sw_pdaf_en;

    //pdaf_point_en 0x0074
    for(int i = 0; i < ISP2X_DPCC_PDAF_POINT_NUM; i++) {
        phwcfg->pdaf_point_en[i] = psta->stPdafParams.pdaf_point_en[i];
    }

    //pdaf_offset 0x0078
    phwcfg->pdaf_offsety = psta->stPdafParams.pdaf_offsety;
    phwcfg->pdaf_offsetx = psta->stPdafParams.pdaf_offsetx;

    //pdaf_wrap 0x007c
    phwcfg->pdaf_wrapy = psta->stPdafParams.pdaf_wrapy;
    phwcfg->pdaf_wrapx = psta->stPdafParams.pdaf_wrapx;

    //pdaf_scope 0x0080
    phwcfg->pdaf_wrapy_num = psta->stPdafParams.pdaf_wrapy_num;
    phwcfg->pdaf_wrapx_num = psta->stPdafParams.pdaf_wrapx_num;

    //pdaf_point_0 0x0084
    for(int i = 0; i < ISP2X_DPCC_PDAF_POINT_NUM; i++) {
        phwcfg->point[i].x = psta->stPdafParams.point[i].x;
        phwcfg->point[i].y = psta->stPdafParams.point[i].y;
    }

    //pdaf_forward_med 0x00a4
    phwcfg->pdaf_forward_med = psta->stPdafParams.pdaf_forward_med;
}

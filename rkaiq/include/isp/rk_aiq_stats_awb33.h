#ifndef __TYPE_AWB_STAT_33H__
#define __TYPE_AWB_STAT_33H__

#define AWBSTATS_NONROI_NUM                       4
#define AWBSTATS_WPDCT_LS_NUM                     4
#define AWBSTATS_WPDCT_UVREGION_NUM               4
#define AWBSTATS_WPDCT_UVREGIONVTX_NUM            4
#define AWBSTATS_WP_HIST_BIN_NUM                  8
#define AWBSTATS_WPLUMAWGTCURVE_SEGMENT_MAX       (AWBSTATS_WP_HIST_BIN_NUM+1)
#define AWBSTATS_WPFLTOUTFULL_ENTITY_NUM          4
#define AWBSTATS_WPFLTOUTSMP_ENTITY_NUM           3
#define AWBSTATS_WPFLTOUT_TOTAL_ENTITY_NUM        (AWBSTATS_WPFLTOUTFULL_ENTITY_NUM+AWBSTATS_WPFLTOUTSMP_ENTITY_NUM)
#define AWBSTATS_WPVECTDISCURVE_SEGMENT_MAX       6
#define AWBSTATS_ZONE_15x15_NUM                   225

typedef enum awbStats_wpHistSrc_mode_e {
    awbStats_norWpHist_mode = 0,
    awbStats_bigWpHist_mode = 1
} awbStats_wpHistSrc_mode_t;

typedef enum awbStats_ds_mode_e {
	/*
	reg: (sw_rawawb_ds16x8_mode_en = = 0) && (sw_rawawb_wind_size == 0)
    */
    awbStats_ds_4x4 = 0,
	/*
	reg: (sw_rawawb_ds16x8_mode_en = = 0) && (sw_rawawb_wind_size == 1)
    */
    awbStats_ds_8x8 = 1,
	/*
	reg: (sw_rawawb_ds16x8_mode_en = = 1)
    */
	awbStats_ds_16x8 = 2
} awbStats_ds_mode_t;

typedef enum awbStats_src_mode_e {
    /*
    reg: (sw_drc2awb_sel_en=0) && (sw_bnr2awb_sel_en=0) && (sw_3a_rawawb_sel=0)
    */
    awbStats_chl0DegamOut_mode = 0,
    /*
    reg: (sw_drc2awb_sel_en=0) && (sw_bnr2awb_sel_en=0) && (sw_3a_rawawb_sel=1)
    */
    awbStats_chl1DegamOut_mode  = 1,
    /*
    reg: (sw_drc2awb_sel_en=0) && (sw_bnr2awb_sel_en=0) && (sw_3a_rawawb_sel=2)
    */
    awbStats_chl2DegamOut_mode  = 2,
    /*
    reg: (sw_drc2awb_sel_en=0) && (sw_bnr2awb_sel_en=1)
    */
    awbStats_btnrOut_mode = 3,
    /*
    reg: (sw_drc2awb_sel_en=1)
    */
    awbStats_drcOut_mode = 4
} awbStats_src_mode_t;

typedef struct awbStats_blc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_blc_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Whether to enable awb blc.\nFreq of use: high))  */
		//reg:sw_bls2_en
    bool hw_awbCfg_blc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_obR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The optical black correction of bayer R pixel in awb statics module.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls2_b_fixed
    float hw_awbCfg_obR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_obGr_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The optical black correction of bayer Gr pixel in awb statics module.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls2_a_fixed
    float hw_awbCfg_obGr_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_obGb_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The optical black correction of bayer Gb pixel in awb statics module.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls2_d_fixed
    float hw_awbCfg_obGb_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_obB_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The optical black correction of bayer B pixel in awb statics module.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls2_c_fixed
    float hw_awbCfg_obB_val;
} awbStats_blc_t;

typedef struct awbStats_nonROI_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_nonROI_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of awb statics Non-Region of Interest.\n
        Freq of use: low))  */
	//reg:sw_rawawb_multiwindowX_h_offs
    uint16_t hw_awbCfg_nonROI_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_nonROI_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Vertical offset of awb statics Non-Region of Interest.\n
        Freq of use: low))  */
	//reg:sw_rawawb_multiwindowX_v_offs
    uint16_t hw_awbCfg_nonROI_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_nonROI_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal size of awb statics Non-Region of Interest.\n
        Freq of use: high))  */
	//reg:sw_rawawb_multiwindowX_h_size
    uint16_t hw_awbCfg_nonROI_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_nonROI_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Vertical size of awb statics Non-Region of Interest.\n
        Freq of use: high))  */
    //reg:sw_rawawb_multiwindowX_v_size
    uint16_t hw_awbCfg_nonROI_height;
} awbStats_nonROI_t;

typedef enum awbStats_mainWinSize_mode_s{
    awbStats_winSizeFull_mode = 0,
    awbStats_winSizeFixed_mode = 1,
}awbStats_mainWinSize_mode_e;

typedef struct awbStats_mainWin_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_win_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awbStats_mainWinSize_mode_e),
        M4_DEFAULT(awbStats_winSizeFull_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(.\nFreq of use: high))  */
    awbStats_mainWinSize_mode_e hw_awbCfg_win_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of awb statics window.\n
        Freq of use: high))  */
	//reg:sw_rawawb_h_offs
    uint16_t hw_awbCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Vertical offset of awb statics window.\n
        Freq of use: high))  */
    //reg:sw_rawawb_v_offs
    uint16_t hw_awbCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal size of awb statics window.\n
        Freq of use: high))  */
		//reg:sw_rawawb_h_size
    uint16_t hw_awbCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Vertical size of awb statics window.\n
        Freq of use: high))  */
		//reg:sw_rawawb_v_size
    uint16_t hw_awbCfg_win_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_nonROI_en),
        M4_TYPE(bool),
        M4_DEFAULT(false),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(.\n
        Reference enum types.\n
        Freq of use: high))  */
    bool hw_awbCfg_nonROI_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(nonROI),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The pixels in the non-ROI region do not participate in the awb statistical calculation\n
        Freq of use: low))  */
        //reg:w_rawawb_multiwindow0_h/v_offs/size
    awbStats_nonROI_t nonROI[AWBSTATS_NONROI_NUM];
} awbStats_win_t;

typedef struct awbStats_rgbyWp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMaxR_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max red value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_r_max
    float hw_awbT_wpMaxR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMinR_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min red value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_r_min
    float hw_awbT_wpMinR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMaxR_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max green value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_g_max
    float hw_awbT_wpMaxG_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMinG_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min green value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_g_min
    float hw_awbT_wpMinG_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMaxB_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max blue value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_b_max
    float hw_awbT_wpMaxB_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMinB_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min blue value in white point detection.\nFreq of use: high))  */
		////reg:sw_rawawb_b_min
    float hw_awbT_wpMinB_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMaxY_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max y value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_y_max
    float hw_awbT_wpMaxY_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpMinY_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min y value in white point detection.\nFreq of use: high))  */
		//reg:sw_rawawb_y_min
    float hw_awbT_wpMinY_thred;
} awbStats_rgbyWp_t;
typedef struct awb_rgb2xy_para_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_rgb2xy_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(6f12),
        M4_NOTES(RGB2XY weight matrix.\n
        Freq of use: high))  */
		//reg:sw_rawawb_wt0/1/2
    float hw_awbCfg_rgb2xy_coeff[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_xyTransMatrix_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(6f12),
        M4_NOTES(RGB2XY, rotation matrix.\nFreq of use: high))  */
		//reg:sw_rawawb_mat0/1/2_x/y
    float hw_awbCfg_xyTransMatrix_coeff[6];
} awb_rgb2xy_para_t;

typedef struct awbStats_xyRegionVtx_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxX_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-8,8),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(X coordinate of the vertex in the XY space white point region.\n
        Freq of use: low))  */
    //reg:sw_rawawb_nor_x0~1_0~3, sw_rawawb_big_x0~1_0~3
    float hw_awbT_vtxX_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxY_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-8,8),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Y coordinate of the vertex in the XY space white point region.\n
        Freq of use: low))  */
    //reg:sw_rawawb_nor_y0~1_0~3, sw_rawawb_big_x0~1_0~3
    float hw_awbT_vtxY_val;
} awbStats_xyRegionVtx_t;

typedef struct awbStats_xyRegion_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(lbVtx),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( The left top vertex of the XY space white point region\n
        Freq of use: low))  */
    awbStats_xyRegionVtx_t ltVtx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rtVtx),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( The right bottom vertex of the XY space white point region\n
        Freq of use: low))  */
    awbStats_xyRegionVtx_t rbVtx;
} awbStats_xyRegion_t;

typedef struct awbStats_xyWpDct_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(rgb2xy),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for converting rgb space to xy space\n
        Freq of use: high))  */
    awb_rgb2xy_para_t rgb2xy;
    /* M4_GENERIC_DESC(
        M4_ALIAS(norWpRegion),
        M4_SIZE_EX(1,4),
        M4_TYPE(struct_list),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting normal white points in XY space \n
        Freq of use: high))  */
    awbStats_xyRegion_t norWpRegion[AWBSTATS_WPDCT_LS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(bigWpRegion),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for big detecting white points in XY space \n
        Freq of use: high))  */
    awbStats_xyRegion_t bigWpRegion[AWBSTATS_WPDCT_LS_NUM];
} awbStats_xyWpDct_t;

typedef struct awbStats_uvRegionVtx_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxU_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(U coordinate of the vertex in the UV space white point region.\n
        Freq of use: high))  */
    //reg: sw_rawawb_vertex0_u_0
    float hw_awbT_vtxU_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxV_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DIGIT_EX(2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(V coordinate of the vertex in the UV space white point region.\n
        Freq of use: high))  */
    //reg: sw_rawawb_vertex0_v_0
    float hw_awbT_vtxV_val;
} awbStats_uvRegionVtx_t;

typedef struct awbStats_uvRegion_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(regionVtx),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Four vertexs of the UV space white point region\n
        Freq of use: low))  */
    awbStats_uvRegionVtx_t regionVtx[AWBSTATS_WPDCT_UVREGIONVTX_NUM];
} awbStats_uvRegion_t;

typedef struct awbStats_uvWpDct_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpRegion),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points in UV space \n
        Freq of use: low))  */
    awbStats_uvRegion_t wpRegion[AWBSTATS_WPDCT_UVREGION_NUM];
} awbStats_uvWpDct_t;

typedef enum awbStats_wpFiltOut_mode_e {
    /*
	reg: (sw_rawawb_exc_wp_regionX_domain == 0)
    */
    awbStats_uvWp_mode = 0,
    /*
	reg: sw_rawawb_exc_wp_regionX_domain == 1
    */
    awbStats_xyWp_mode = 1

} awb_wpSpace_mode_t;

typedef struct awbStats_filtOutRegionVtx_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxU_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-8,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(X or U coordinate of the vertex of the filted out white point region.\n
        Freq of use: high))  */
    //reg: sw_rawawb_exc_wp_region0~6_xu0
    float hw_awbT_vtxXU_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vtxV_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-8,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Y or V coordinate of the vertex of the filted out white point region.\n
        Freq of use: high))  */
    //reg: sw_rawawb_exc_wp_region0~6_xu1
    float hw_awbT_vtxYV_val;
} awbStats_filtOutRegionVtx_t;

typedef struct awbStats_filtOutRegion_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(lbVtx),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( The left top vertex of the filted out white point region\n
        Freq of use: high))  */
    awbStats_filtOutRegionVtx_t ltVtx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rtVtx),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The right bottom vertex of the filted out white point region\n
        Freq of use: high))  */
    awbStats_filtOutRegionVtx_t rbVtx;
} awbStats_filtOutRegion_t;

typedef struct awb_wpFiltOutFull_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpSpace_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awb_wpSpace_mode_t),
        M4_DEFAULT(awbStats_xyWp_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This mode bit allows you to choose whether to perform the filtering of specific white points from the xy space or the uv space.\n
        Reference enum types.\n
        Freq of use: high))  */
		//reg:sw_rawawb_exc_wp_region0/1/2/3_domain
    awb_wpSpace_mode_t hw_awbT_wpSpace_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpRegion),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for filted out white points from XY or UV space \n
        Freq of use: high))  */
    awbStats_filtOutRegion_t wpRegion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpStats_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The weight of white point when filtering from original space.\n
        Freq of use: high))  */
    //reg:sw_rawawb_exc_wp_region0~3_weight
    float hw_awbT_stats_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_measure_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enabled bit for independent statistics of the filtered white points\n
        Freq of use: high))  */
   //reg:sw_rawawb_exc_wp_region0~3_measen
   bool hw_awbCfg_wpStats_en;
} awb_wpFiltOutFull_t;

typedef struct awb_wpFiltOutSmp_s {
     /* M4_GENERIC_DESC(
         M4_ALIAS(hw_awbT_wpSpace_mode),
         M4_TYPE(enum),
         M4_ENUM_DEF(awb_wpSpace_mode_t),
         M4_DEFAULT(awbStats_xyWp_mode),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(0),
         M4_NOTES(This mode bit allows you to choose whether to perform the filtering of specific white points from the xy space or the uv space.\n
        Reference enum types.\n
        Freq of use: high))  */
     //reg:sw_rawawb_exc_wp_region4/5/6_domain
     awb_wpSpace_mode_t hw_awbT_wpSpace_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpRegion),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for filted out white points from XY or UV space \n
        Freq of use: high))  */
     awbStats_filtOutRegion_t wpRegion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_wpStats_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The weight of white point when filtering from original space.\n
        Freq of use: high))  */
    //reg:sw_rawawb_exc_wp_region0~3_weight
    float hw_awbT_stats_wgt;
} awb_wpFiltOutSmp_t;

typedef struct awbStats_wpLumaWgtCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u8),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The X coordinate of luma weight curve.\nFreq of use: high))  */
	//reg:sw_rawawb_wp_luma_weicurve_y0/1/2/3/4/5/6/7/8
    uint8_t idx[AWBSTATS_WPLUMAWGTCURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Y coordinate of luma weight curve.\nFreq of use: high))  */
	//reg:sw_rawawb_wp_luma_weicurve_y0/1/2/3/4/5/6/7/8
    float val[AWBSTATS_WPLUMAWGTCURVE_SEGMENT_MAX];
} awbStats_wpLumaWgtCurve_t;

typedef struct awbStats_wpVectDistThCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The X coordinate of curve.\nFreq of use: high))  */
	//reg:sw_rawawb_rotu0~5_lsX
    float idx[AWBSTATS_WPVECTDISCURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(4f4),
        M4_NOTES(The Y coordinate of curve.\nFreq of use: high))  */
	//reg:sw_rawawb_th0~5_lsX
    float val[AWBSTATS_WPVECTDISCURVE_SEGMENT_MAX];
} awbStats_wpVectDistThCurve_t;

typedef struct awbStats_rotYuvVectEdp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_edpY_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(4f4),
        M4_NOTES(The y coordinate values of the endpoints of the light source white point vector in the rotYuv space.\n
        Freq of use: high))  */
        //reg:sw_rawawb_coor_x1_lsX_y, sw_rawawb_vec_x21_lsX_y
	float hw_awbT_edpY_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_edpU_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(4f4),
        M4_NOTES(The u coordinate values of the endpoints of the light source white point vector in the rotYuv space.\n
        Freq of use: high))  */
        //reg:sw_rawawb_coor_x1_lsX_u, sw_rawawb_vec_x21_lsX_u
	float hw_awbT_edpU_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_edpV_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_DIGIT_EX(4f4),
        M4_NOTES(The v coordinate values of the endpoints of the light source white point vector in the rotYuv space.\n
        Freq of use: high))  */
        //reg:sw_rawawb_coor_x1_lsX_v, sw_rawawb_vec_x21_lsX_v
	float hw_awbT_edpV_val;
}awbStats_rotYuvVectEdp_t;

typedef struct awbStats_rotYuvVect_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(edp),
        M4_TYPE(struct_list),
        M4_UI_MODULE(normal_ui_style),
        M4_SIZE_EX(1,2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The end point of the ideal white point vector of the light source in the rotYUV space \n
        Freq of use: high))  */
    awbStats_rotYuvVectEdp_t edp[2];
    #if 0
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_vectMag_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The magnitude of the ideal white point vector of the light source. The magnitude is 2^hw_awbCfg_vectMag_val;\n
        Freq of use: high))  */
        //reg: sw_rawawb_dis_x1x2_ls0/1/2/3   
	uint8_t hw_awbT_vectMag_val;
    #endif
} awbStats_rotYuvVect_t;

typedef struct awbStats_rotYuvRegion_s {
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_u2WpDisTh_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points \n
        Freq of use: low))  */
    awbStats_wpVectDistThCurve_t hw_awbT_u2WpDistTh_curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lsVect),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for the ideal white point vector of the light source in the rotYUV space \n
        Freq of use: low))  */
    awbStats_rotYuvVect_t lsVect;
} awbStats_rotYuvRegion_t;

typedef struct awbStats_rotYuvWpDct_s {
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_rgb2RYuv_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(3,4),
        M4_RANGE_EX(-255,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(RGB2ROTYUV coefficient mat.\n
        Freq of use: low))  */
    float hw_awbCfg_rgb2RotYuv_coeff[12];
     /* M4_GENERIC_DESC(
        M4_ALIAS(wpRegion),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points in rotate YUV space \n
        Freq of use: low))  */
    awbStats_rotYuvRegion_t wpRegion[AWBSTATS_WPDCT_LS_NUM];
} awbStats_rotYuvWpDct_t;

typedef struct awbStats_norWpStatsCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_xyDct_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable white point detection in the XY space\n
        Freq of use: high))  */
    //reg:sw_rawawb_xy_en0
    bool hw_awbCfg_xyDct_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_uvDct_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable white point detection in the UV space \n
        Freq of use: high))  */
    //reg:sw_rawawb_uv_en0
    bool hw_awbCfg_uvDct_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpFiltOut_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable filter out specific white points from detected white points in xy space or uv space.\n
        Freq of use: high))  */
    //reg: sw_rawawb_exc_wp_region0/1/2/3/4/5/6_excen bit0
	bool hw_awbCfg_wpFiltOut_en[AWBSTATS_WPFLTOUTFULL_ENTITY_NUM+AWBSTATS_WPFLTOUTSMP_ENTITY_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_lumaWgt_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(It is perform weighted statistics on the luma weights of the normal white point when the bit is enabled.\n
        Freq of use: high))  */
	//reg:sw_rawawb_wp_luma_wei_en0
    bool hw_awbCfg_lumaWgt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_zoneWgt_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(It is perform weighted statistics on the zone weights of the normal white point when the bit is enabled.\n
        Freq of use: high))  */
	//reg:sw_rawawb_wp_blk_wei_en0
    bool hw_awbCfg_zoneWgt_en;
} awbStats_norWpStatsCfg_t;

typedef struct awbStats_bigWpStatsCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_xyBigDct_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable white point detection in the XY space\n
        Freq of use: high))  */
    //reg:sw_rawawb_xy_en1
    bool hw_awbCfg_xyBigDct_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_uvDct_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable white point detection in the UV space from the normal white points in the XY space\n
        Freq of use: high))  */
    //reg:sw_rawawb_uv_en1
    bool hw_awbCfg_uvDct_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpFiltOut_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable filter out specific white points from detected white points in xy space or uv space.\n
        Freq of use: high))  */
    //reg: sw_rawawb_exc_wp_region0/1/2/3/4/5/6_excen  bit1
	bool hw_awbCfg_wpFiltOut_en[AWBSTATS_WPFLTOUTFULL_ENTITY_NUM+AWBSTATS_WPFLTOUTSMP_ENTITY_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_rotYuvDct_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable white point detection in the rotate YUV space\n
        Freq of use: high))  */
    //reg:sw_rawawb_3dyuv_en1
    bool hw_awbCfg_rotYuvDct_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_lumaWgt_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(It is perform weighted statistics on the luma weights of the big white point when the bit is enabled.\n
        Freq of use: high))  */
	//reg:sw_rawawb_wp_luma_wei_en1
    bool hw_awbCfg_lumaWgt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_zoneWgt_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,2),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(It is perform weighted statistics on the zone weights of the big white point when the bit is enabled.\n
        Freq of use: high))  */
	//reg:sw_rawawb_wp_blk_wei_en1
    bool hw_awbCfg_zoneWgt_en;
} awbStats_bigWpStatsCfg_t;

typedef struct awbStats_wpEngine_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpDct_rgbySpace),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points in rgb-y space\n
        Freq of use: high))  */
    awbStats_rgbyWp_t wpDct_rgbySpace;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_lightNum),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Number of light sources.\nFreq of use: high))  */
		//sw_rawawb_light_num
    uint8_t hw_awbCfg_lightSrcNum_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpDct_xySpace),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points in XY space \n
        Freq of use: low))  */
    awbStats_xyWpDct_t wpDct_xySpace;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpDct_uvSpace),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters for detecting white points in UV space \n
        Freq of use: low))  */
    awbStats_uvWpDct_t wpDct_uvSpace;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpDct_uvSpace),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The feature supports defining special white points in the XY space or UV space and filtering them from the original space.\n
        These white points are filtered can be independently statisticed in full-featured engine only.\n
        Freq of use: high))  */
    awb_wpFiltOutFull_t wpFiltOut_fullEntity[AWBSTATS_WPFLTOUTFULL_ENTITY_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpDct_uvSpace),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The feature supports defining special white points in the XY space or UV space and filtering them from the original space.\n
        These white points are filtered can be independently statisticed in full-featured engine only. \n
        Freq of use: high))  */
    awb_wpFiltOutSmp_t wpFiltOut_smpEntity[AWBSTATS_WPFLTOUTSMP_ENTITY_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_luma2WpWgt_ccm),
        M4_TYPE(f32),
        M4_SIZE_EX(3,3),
        M4_RANGE_EX(0,8),
        M4_DEFAULT([1,0,0,0,1,0,0,0,1]),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: low))  */
    float hw_awbT_luma2WpWgt_ccm[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbT_luma2WpWgt_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The white point statistical weight lookup table with white point luma as the index \n
        Freq of use: high))  */
    awbStats_wpLumaWgtCurve_t hw_awbT_luma2WpWgt_curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpHistSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awbStats_wpHistSrc_mode_t),
        M4_DEFAULT(awbStats_norWpHist_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This feature supports selecting normal white points or big white points for histogram statistics. Reference enum types.\n
        Freq of use: high))  */
    //reg:sw_rawawb_wp_hist_xytype
    awbStats_wpHistSrc_mode_t hw_awbCfg_wpHistSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_zone_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(15,15),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The weight of each zone in awb statistics..\n
        Freq of use: high))  */
    //reg:sw_rawawb_wp_blk_wei_w0~224
    uint8_t hw_awbCfg_zone_wgt[AWBSTATS_ZONE_15x15_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(norWpStatsCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The statistic of normal white points will be output individually.\n
        This structure contains the combined enabling configuration of various white point detection engines used to detect normal white points.\n
        Freq of use: high))  */
    awbStats_norWpStatsCfg_t norWpStatsCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bigWpStatsCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The statistic of big white points will be output individually.\n
        This structure contains the combined enabling configuration of various white point detection engines used to detect big white points.
        Freq of use: high))  */
    awbStats_norWpStatsCfg_t bigWpStatsCfg;
} awbStats_wpEngine_t;

typedef enum awbStats_pixEngineSrc_mode_e {
    /*
	reg: (sw_rawawb_blk_measure_mode = = 0)
    */
    awbStats_pixAll_mode = 0,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 0)
    */
    awbStats_norWpLs0_mode = 0x10,
    /*
	reg:(sw_rawawb_blk_measure_mode = = 1) && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 1)
    */
    awbStats_norWpLs1_mode = 0x11,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1) && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 2)
    */
    awbStats_norWpLs2_mode = 0x12,
    /*
	reg:(sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 3)
    */
    awbStats_norWpLs3_mode = 0x13,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 4)
    */
    awbStats_norWpLs4_mode = 0x14,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 5)
    */
    awbStats_norWpLs5_mode = 0x15,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 6)
    */
    awbStats_norWpLs6_mode = 0x16,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 0) && (sw_rawawb_blk_measure_illu_idx == 7)
    */
    awbStats_norWpAll_mode = 0x17,
    /*
	reg:(sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 0)
    */
    awbStats_bigWpLs0_mode = 0x20,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 1)
    */
    awbStats_bigWpLs1_mode = 0x21,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 2)
    */
    awbStats_bigWpLs2_mode = 0x22,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 3)
    */
    awbStats_bigWpLs3_mode = 0x23,
    /*
	reg:(sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 4)
    */
    awbStats_bigWpLs4_mode = 0x24,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 5)
    */
    awbStats_bigWpLs5_mode = 0x25,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 6)
    */
    awbStats_bigWpLs6_mode = 0x26,
    /*
	reg: (sw_rawawb_blk_measure_mode = = 1)  && (sw_rawawb_blk_measure_xytype == 1) && (sw_rawawb_blk_measure_illu_idx == 7)
    */
    awbStats_bigWpAll_mode = 0x27
} awbStats_pixEngineSrc_mode_t;

typedef struct awbStats_pixEngine_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_zoneStats_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Whether to enable block stat.\nFreq of use: high))  */
    //reg:sw_rawawb_blk_measure_en
    bool hw_awbCfg_stats_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_zoneStatsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awbStats_zoneStatsSrc_mode_t),
        M4_DEFAULT(awbStats_pixAll_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Measure mode for block stat. Reference enum types.\nFreq of use: high))  */
    //reg:sw_rawawb_blk_measure_mode
    awbStats_pixEngineSrc_mode_t hw_awbCfg_zoneStatsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_blkStatWithLumaWeight_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Whether to enable luma weight.\nFreq of use: high))  */
     //reg:sw_rawawb_blk_with_luma_wei_en
    bool hw_awbCfg_lumaWgt_en;
} awbStats_pixEngine_t;

typedef struct awbStats_cfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_stats_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable awb statics.\n
        Freq of use: high))  */
	//reg:sw_rawawb_en
    bool hw_awbCfg_stats_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_statsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awbStats_src_mode_t),
        M4_DEFAULT(awbStats_drcOut_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode bit can be used to switch the source of the AWB statistics module.\n
        Reference enum types.\n
        Freq of use: high))  */
    awbStats_src_mode_t hw_awbCfg_statsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of blc in the awb statistics module\n
        Freq of use: high))  */
    awbStats_blc_t blc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_lsc_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of lsc in the awb statistics module.\nFreq of use: high))  */
	//reg:sw_rawlsc_bypass_en
    bool hw_awbCfg_lsc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_ds_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(awbStats_ds_mode_t),
        M4_DEFAULT(awbStats_ds_8x8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode bit is used to switch the way bayer raw is converted to rgb.\n
        Reference enum types.\n
        Freq of use: high))  */
     //reg:sw_rawawb_ds16x8_mode_en+sw_rawawb_wind_size
    awbStats_ds_mode_t hw_awbCfg_ds_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(stat window\n
        Freq of use: high))  */
    awbStats_win_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpEngine),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters of the white balance white point statistics engine \n
        Freq of use: high))  */
    awbStats_wpEngine_t wpEngine;
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoneStatsCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Parameters of the white balance pixel statistics engine\n
        Freq of use: high))  */
    awbStats_pixEngine_t pixEngine;
} awbStats_cfg_t;

typedef struct awbStats_wpStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpNo),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(The accumulation of white points with luma weight, block weight and include weight\n
        Freq of use: high))  */
    uint32_t hw_awbCfg_statsWp_count;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_rGainSum_val),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point R-gain sum with luma weight, block weight and include weight\n
        Freq of use: high))  */
    uint32_t hw_awbCfg_rGainSum_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_bGainSum_val),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point B-gain sum with luma weight, block weight and include weight\n
        Freq of use: high))  */
    uint32_t hw_awbCfg_bGainSum_val;
} awbStats_wpStats_t;

typedef struct awbStats_wpEngineStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(norWpStats),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Normal white point statistics for all light sources\n
    Freq of use: high))  */
    awbStats_wpStats_t norWp[AWBSTATS_WPDCT_LS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(bigWpStats),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Big white point statistics for all light sources\n
        Freq of use: high))  */
    awbStats_wpStats_t bigWp[AWBSTATS_WPDCT_LS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpNoWithoutWgt),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(The accumulation of normal white points and big white points from xy and uv spaces without luma weight, zone weight and include weight\n
        Freq of use: high))  */
    uint32_t hw_awbCfg_wpXyUvSpcRaw_cnt[AWBSTATS_WPDCT_LS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awb_wpHistBin_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point luma histogram\nFreq of use: high))  */
    uint32_t hw_awb_wpHistBin_val[AWBSTATS_WP_HIST_BIN_NUM];
} awbStats_wpEngineStats_t;

typedef struct awbStats_wpfltEngineStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(fltPix),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( statistics for fi\n
        Freq of use: high))  */
    awbStats_wpStats_t fltPix[AWBSTATS_WPFLTOUTFULL_ENTITY_NUM];
} awbStats_wpfltEngineStats_t;


typedef struct awbStats_pixStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_wpNo),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point sum number with luma weight and include weight\nFreq of use: high))  */
    uint32_t hw_awbCfg_statsPix_count;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_rSum_val),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point red sum with luma weight and include weight\nFreq of use: high))  */
    uint32_t hw_awbCfg_rSum_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_gSum_val),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point green sum with luma weight and include weight\nFreq of use: high))  */
    uint32_t hw_awbCfg_gSum_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_awbCfg_bSum_val),
        M4_TYPE(u64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4294967296),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(White point blue sum with luma weight and include weight\nFreq of use: high))  */
    uint32_t hw_awbCfg_bSum_val;
} awbStats_pixStats_t;

typedef struct awbStats_pixEngineStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(blkStat),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( statistics for block\n
        Freq of use: high))  */
    awbStats_pixStats_t zonePix[AWBSTATS_ZONE_15x15_NUM];
} awbStats_pixEngineStats_t;

typedef struct awbStats_stats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpEngine),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Statistics of the white balance white point statistics engine\n
        Freq of use: high))  */
    awbStats_wpEngineStats_t wpEngine;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wpFltOutFullEngine),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Statistics of the filter out specific white points from detected white points in xy space or uv space\n
        Freq of use: high))  */
    awbStats_wpfltEngineStats_t wpFltOutFullEngine;
    /* M4_GENERIC_DESC(
        M4_ALIAS(pixEngine),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Statistics of the white balance pixel statistics engine\n
        Freq of use: high))  */
    awbStats_pixEngineStats_t pixEngine;
} awbStats_stats_t;

typedef enum awbStats_cfg_mode_s{
    AWB_CFG_MODE_API = 0,
    AWB_CFG_MODE_RK = 1,
    AWB_CFG_MODE_ThP = 2
}awbStats_cfg_mode_e;

typedef struct awbStats_cfg_priv_s{
    awbStats_cfg_t com;// Must be placed first
    awbStats_cfg_mode_e mode;
    int count;
    int  groupIllIndxCurrent;//for time share
    int  IllIndxSetCurrent[AWBSTATS_WPDCT_LS_NUM];//for time share
    char timeSign[64];
    float preWbgainSw[4];//rggb
}awbStats_cfg_priv_t;

typedef struct rk_aiq_awb_stat_cfg_effect_s {
    awbStats_cfg_mode_e mode;
    awbStats_pixEngineSrc_mode_t blkMeasureMode;
    unsigned char lightNum;
    int  groupIllIndxCurrent;//for time share
    int  IllIndxSetCurrent[AWBSTATS_WPDCT_LS_NUM];//for time share
    char timeSign[64];
    float preWbgainSw[4];//rggb

} rk_aiq_awb_stat_cfg_effect_t;


typedef struct awbStats_stats_priv_s{
    awbStats_stats_t com;// Must be placed first
    int dbginfo_fd;
    rk_aiq_awb_stat_cfg_effect_t awb_cfg_effect;
    awbStats_pixStats_t sumBlkRGB;
}awbStats_stats_priv_t;




#endif

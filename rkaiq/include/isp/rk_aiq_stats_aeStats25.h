#ifndef __TYPES_AE_HW_H__
#define __TYPES_AE_HW_H__

#include "../algos/ae/rk_aiq_types_ae_hw.h"

#define AESTATS_SUBWIN_NUM                4
#define AESTATS_ZONE_15x15_ROW            15
#define AESTATS_ZONE_15x15_COL            15
#define AESTATS_ZONE_15x15_NUM            225
#define AESTATS_HIST_BIN_NUM              256

/*****************************************************************************/
/**
 * @brief   ISP3.9 AEC HW-Meas Config Params
 */
/*****************************************************************************/

typedef enum aeStats_winZone_mode_e {
    aeStats_winZone_5x5_mode    = 1,

    aeStats_winZone_15x15_mode    = 2,
} aeStats_winZone_mode_t;

typedef enum aeStats_histPix_mode_e {
    /*
    reg:(sw_aehist_mode == 2)
    R histogram mode;
    */
    aeStats_histPixR_mode  = 2,
    /*
    reg:(sw_aehist_mode == 3)
    G histogram mode;
    */
    aeStats_histPixG_mode  = 3,
    /*
    reg:(sw_aehist_mode == 4)
    B histogram mode;
    */
    aeStats_histPixB_mode  = 4,
    /*
    reg:(sw_aehist_mode == 5)
    Y histogram mode;
    */
    aeStats_histPixY_mode  = 5,
} aeStats_histPix_mode_t;

typedef struct aeStats_hist_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_winZone_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_winZone_mode_t),
        M4_DEFAULT(ae_winZone_15x15_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    aeStats_winZone_mode_t hw_aeCfg_winZone_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aehist_h_offset
    uint16_t hw_aeCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Vertical offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aehist_v_offset
    uint16_t hw_aeCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Horizontal size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aehist_h_size
    uint16_t hw_aeCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Vertical size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aehist_v_size
    uint16_t hw_aeCfg_win_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_pixIn_shift),
        M4_TYPE(u8),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    //reg: sw_aehist_data_sel
    uint8_t hw_aeCfg_pixIn_shift;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_pixVal_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x0FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Minimal statics threshold value for histogram.\nFreq of use: high))  */
    //reg: sw_aehist_waterline
    uint16_t hw_aeCfg_pixVal_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_histT_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_histPix_mode_t),
        M4_DEFAULT(ae_histPixY_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Histgram meas mode. Reference enum types.\nFreq of use: high))  */
    //reg: sw_aehist_mode
    aeStats_histPix_mode_t hw_aeCfg_pix_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_pixSkip_step),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Histogram previous divider, process every (step size) pixel.\nFreq of use: high))  */
    //reg: sw_aehist_stepsize
    uint8_t hw_aeCfg_pixSkip_step;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_bayer2Y_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,0xff),
        M4_DEFAULT(29),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(A conversion matrix for combining 4 bayer raw pixels into 1 y pixel.\n
        Y = coeff[0]*BayerR + coeff[1]*(BayerGr+BayerGb)/2 + coeff[2]*BayerB + coeff[3]\n
        Freq of use: high))  */
    //reg: sw_rawae_bcc, sw_rawae_gcc, sw_rawae_rcc,sw_rawae_offset
    uint8_t hw_aeCfg_bayer2Y_coeff[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_zone_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(15,15),
        M4_RANGE_EX(0,0x20),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The weight of each zone in histogram statistics.\n
        Freq of use: high))  */
    //reg: WEIGHT_BASE
    uint8_t hw_aeCfg_zone_wgt[AESTATS_ZONE_15x15_NUM];
} aeStats_hist_t;

typedef struct aeStats_mainWin_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_h_offset
    uint16_t hw_aeCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Vertical offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_v_offset
    uint16_t hw_aeCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Horizontal size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_h_size
    uint16_t hw_aeCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Vertical size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_v_size
    uint16_t hw_aeCfg_win_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_winZone_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_winZone_mode_t),
        M4_DEFAULT(ae_winZone_15x15_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    aeStats_winZone_mode_t hw_aeCfg_winZone_mode;
} aeStats_mainWin_t;

typedef struct aeStats_subWin_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_subWin_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(.....\nFreq of use: high))  */
    //reg: sw_rawae_big_wnd1_en ~ sw_rawae_big_wnd4_en
    bool hw_aeCfg_subWin_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_h_offset
    uint16_t hw_aeCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Vertical offset of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_v_offset
    uint16_t hw_aeCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Horizontal size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_h_size
    uint16_t hw_aeCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0x1FFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Vertical size of ae meas roi.\nFreq of use: high))  */
    //reg: sw_aemeas_v_size
    uint16_t hw_aeCfg_win_height;
} aeStats_subWin_t;

typedef enum aeStats_srcEntity0_mode_e {    // Domain,        linear
    /*
    reg: (sw_rawae_from_vicap == 1)
    */
    aeStats_vicapOut_mode = 0,                   //Raw12,         Y
    /*
    reg: (sw_rawae0_sel == 0) && (sw_bnr2ae0_sel == 0) && (sw_rawae_from_vicap == 0)
    */
    aeStats_entity0_chl0DpcOut_mode = 1,           //Raw12,         Y
    /*
    reg: (sw_rawae0_sel == 1) && (sw_bnr2ae0_sel == 0) && (sw_rawae_from_vicap == 0)
    */
    aeStats_entity0_chl1DpcOut_mode = 2,           //Raw12,         Y
    /*
    reg: (sw_bnr2ae0_sel == 1) && (sw_rawae_from_vicap == 0)
    */
    aeStats_btnrOutLow_mode = 3,                  //Raw20[9:0],    Y
    /*
    reg: (sw_bnr2ae0_sel == 1) && (sw_aemeas_brn_be_sel == 1)
    */
    aeStats_btnrBeOutLow_mode = 4                 //Raw20[9:0],    Y
} aeStats_srcEntity0_mode_t;

typedef struct aeStats_entity0_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_statsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aeStats_srcEntity0_mode_t),
        M4_DEFAULT(aeStats_entity0_chl0Wb0Out_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    //reg: sw_rawae0_sel, sw_bnr2ae0_sel, sw_rawae_from_vicap
    aeStats_srcEntity0_mode_t hw_aeCfg_statsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_mainWin_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    //reg: WND1_BLK_SIZE,WND1_OFFSET ~ WND4_BLK_SIZE,WND4_OFFSET
    aeStats_subWin_t subWin[AESTATS_SUBWIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_hist_t hist;
} aeStats_entity0_t;

typedef enum aeStats_srcEntity3_mode_e {    // Domain,        linear
    /*
    reg: (sw_rawae_sel == 0) && (sw_bnr2aebig_sel == 0)
    */
    aeStats_entity3_chl0DpcOut_mode = 0,           //Raw12bit,         Y
    /*
    reg: (sw_rawae_sel == 1) && (sw_bnr2aebig_sel == 0)
    */
    aeStats_entity3_chl1DpcOut_mode = 1,           //Raw12bit,         Y
    /*
    reg: (sw_rawae_sel == 3)
    */
    aeStats_dmIn_mode = 2,                        //Raw12bit,         Y
    /*
    reg: (sw_bnr2aebig_sel == 1)
    */
    aeStats_btnrOutHigh_mode = 3,                 //Raw20bit[10:19],    Y
    /*
    reg: (sw_bnr2ae0_sel == 1) && (sw_aemeas_brn_be_sel == 1)
    */
    aeStats_btnrBeOutHigh_mode = 4               //Raw20bit[10:19],    Y
} aeStats_srcEntity3_mode_t;

typedef struct aeStats_entity3_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_statsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aeStats_srcEntity3_mode_t),
        M4_DEFAULT(aeStats_entity3_chl1Wb0Out_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    //reg: sw_bnr2aebig_sel, sw_rawae_sel
    aeStats_srcEntity3_mode_t hw_aeCfg_statsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_mainWin_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    //reg: WND1_BLK_SIZE,WND1_OFFSET ~ WND4_BLK_SIZE,WND4_OFFSET
    aeStats_subWin_t subWin[AESTATS_SUBWIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_hist_t hist;
} aeStats_entity3_t;

typedef enum aeStats_srcCoWkEnt03_mode_e {
    aeStats_btnrOut_mode = 0,
    aeStats_btnrBeOut_mode = 1
} aeStats_srcCoWkEnt03_mode_t;

typedef struct aeStats_coWkEntity03_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_statsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aeStats_srcCoWkEnt03_mode_t),
        M4_DEFAULT(aeStats_btnrOut_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    aeStats_srcCoWkEnt03_mode_t sw_aeCfg_statsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_mainWin_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    //reg: WND1_BLK_SIZE,WND1_OFFSET ~ WND4_BLK_SIZE,WND4_OFFSET
    aeStats_subWin_t subWin[AESTATS_SUBWIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    //reg: WND0_BLK_SIZE,WND0_OFFSET
    aeStats_hist_t hist;
} aeStats_coWkEntity03_t;

typedef enum aeStats_entityGroupWk_mode_e {
    aeStats_entity03_coWk_mode = 0,
    aeStats_entity03_indWk_mode = 1,
    aeStats_entity0_wkOnly_mode = 2,
    aeStats_entity3_wkOnly_mode = 3,
} aeStats_entityGroupWk_mode_t;

typedef struct aeStats_entities_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(entity0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only valid in ae_entity03_independentWork_mode or aeStats_entity0_wkOnly_mode mode))  */
    aeStats_entity0_t entity0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(entity3),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only valid in ae_entity03_independentWork_mode or aeStats_entity3_wkOnly_mode mode))  */
    aeStats_entity3_t entity3;
} aeStats_entities_t;

typedef union aeStats_entityGroup_u {
    /* M4_GENERIC_DESC(
        M4_ALIAS(coWkEntity03),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only valid in ae_entity03_coWork_mode mode))  */
    aeStats_coWkEntity03_t coWkEntity03;
    /* M4_GENERIC_DESC(
        M4_ALIAS(entities),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only not valid in ae_entity03_coWork_mode mode))  */
    aeStats_entities_t entities;
} aeStats_entityGroup_t;

typedef struct aeStats_cfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeCfg_stats_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable ae statics\nFreq of use: low))  */
    bool sw_aeCfg_stats_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeCfg_update),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    bool sw_aeCfg_update;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_entityGroup_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aeStats_entityGroupWk_mode_t),
        M4_DEFAULT(aeStats_entity03_independentWk_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\nFreq of use: high))  */
    aeStats_entityGroupWk_mode_t hw_aeCfg_entityGroup_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(entityGroup),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(...))  */
    aeStats_entityGroup_t entityGroup;
} aeStats_cfg_t;

/*****************************************************************************/
/**
 * @brief   ISP3.9 AEC AEC HW-Meas Res Params
 */
/*****************************************************************************/

typedef struct aeStats_mainWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerR_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of R channel in ae main roi))  */
    uint16_t hw_ae_meanBayerR_val[AESTATS_ZONE_15x15_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerGrGb_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint16_t hw_ae_meanBayerGrGb_val[AESTATS_ZONE_15x15_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerB_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint16_t hw_ae_meanBayerB_val[AESTATS_ZONE_15x15_NUM];
} aeStats_mainWinStats_t;

typedef struct aeStats_subWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerR_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,536870911),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of R channel in ae main roi))  */
    uint32_t hw_ae_sumBayerR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_sumBayerGrGb_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4294967295),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint32_t hw_ae_sumBayerGrGb_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_sumBayerB_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,536870911),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of B channel in ae main roi))  */
    uint32_t hw_ae_sumBayerB_val;
} aeStats_subWinStats_t;

typedef struct aeStats_histStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_histBin_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(0,268435455),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(histogram stats))  */
    unsigned int hw_ae_histBin_val[AESTATS_HIST_BIN_NUM];
} aeStats_histStats_t;

typedef struct aeStats_entityStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    aeStats_mainWinStats_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    aeStats_subWinStats_t subWin[AESTATS_SUBWIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    aeStats_histStats_t hist;
} aeStats_entityStats_t;

typedef struct aeStats_entitiesStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(entity0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only valid in aeStats_entity03_indWk_mode | aeStats_entity0_wkOnly_mode))  */
    aeStats_entityStats_t entity0;
    /* M4_GENERIC_DESC(
         M4_ALIAS(entity3),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity03_indWk_mode mode | aeStats_entity3_wkOnly_mode))  */
    aeStats_entityStats_t entity3;
} aeStats_entitiesStats_t;

typedef struct aeStats_entityGrpStats_s {
    /* M4_GENERIC_DESC(
         M4_ALIAS(coWkEnt03),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity03_coWk_mode mode))  */
    aeStats_entityStats_t coWkEnt03;
    /* M4_GENERIC_DESC(
         M4_ALIAS(entities),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity0_wkOnly_mode | aeStats_entity3_wkOnly_mode | aeStats_entity03_indWk_mode mode))  */
    aeStats_entitiesStats_t entities;
} aeStats_entityGrpStats_t;

typedef struct aeStats_stats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_entityGroup_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aeStats_entityGroupWk_mode_t),
        M4_DEFAULT(aeStats_entity03_indWk_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\n
        Freq of use: high))  */
    aeStats_entityGroupWk_mode_t hw_ae_entityGroup_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(entityGroup),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(...))  */
    aeStats_entityGrpStats_t entityGroup;
} aeStats_stats_t;

typedef struct RKAiqAecStatsV25_s {
    aeStats_stats_t ae_data;  //AeHwMeas_Res
    RKAiqAecExpInfo_t ae_exp; //AeExp_Info
} RKAiqAecStatsV25_t;


#endif

#include "rk_aiq_isp39_modules.h"

static void convertAiqAeToIsp39Params(isp_params_t* isp_params, aeStats_cfg_t* ae_meas)
{
#if ISP_HW_V35
    if (ae_meas->sw_aeCfg_stats_en) {

        // 0) module_ens
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE1_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE2_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_YUVAE_ID;

        // 1) rawae0
        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawae0.rawae_sel = 0x20;

            if(ae_meas->entityGroup.coWkEntity03.sw_aeCfg_statsSrc_mode == aeStats_btnrBeOut_mode)
                isp_params->isp_cfg->meas.rawae0.bnr_be_sel = 1;
            else
                isp_params->isp_cfg->meas.rawae0.bnr_be_sel = 0;

            isp_params->isp_cfg->meas.rawae0.wnd_num = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae0.win0_h_offset = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win0_v_offset = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win0_h_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win0_v_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.wnd1_en = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.win1_h_offset = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win1_v_offset = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win1_h_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win1_v_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_height;

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

            if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl0DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 0;
            } else if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl1DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 1;
            } else if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutLow_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 0x20;
                isp_params->isp_cfg->meas.rawae0.bnr_be_sel = 0;
            } else if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_btnrBeOutLow_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 0x20;
                isp_params->isp_cfg->meas.rawae0.bnr_be_sel = 1;
            }

            isp_params->isp_cfg->meas.rawae0.wnd_num = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae0.win0_h_offset = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win0_v_offset = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win0_h_size = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win0_v_size = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.wnd1_en = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.win1_h_offset = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win1_v_offset = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win1_h_size = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win1_v_size = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height;
        }

        // rawae3

        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawae3.rawae_sel = 0x10;

            if(ae_meas->entityGroup.coWkEntity03.sw_aeCfg_statsSrc_mode == aeStats_btnrBeOut_mode)
                isp_params->isp_cfg->meas.rawae3.bnr_be_sel = 1;
            else
                isp_params->isp_cfg->meas.rawae3.bnr_be_sel = 0;

            isp_params->isp_cfg->meas.rawae3.wnd_num = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae3.win0_h_offset = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win0_v_offset = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win0_h_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win0_v_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.wnd1_en = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.win1_h_offset = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win1_v_offset = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win1_h_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win1_v_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_height;

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

            if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl0DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 0;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl1DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 1;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_dmIn_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 3;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutHigh_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 0x10;
            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_btnrBeOutHigh_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 0x20;
                isp_params->isp_cfg->meas.rawae3.bnr_be_sel = 1;
            }

            isp_params->isp_cfg->meas.rawae3.wnd_num = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae3.win0_h_offset = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win0_v_offset = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win0_h_size = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win0_v_size = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.wnd1_en = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.win1_h_offset = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win1_v_offset = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win1_h_size = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win1_v_size = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_height;

        }

        if(/*aec_meas.ae_meas_update*/1) {
            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE0_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE1_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE1_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE2_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE2_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE3_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_YUVAE_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_YUVAE_ID;

        } else {
            return;
        }
    } else {
        // only update en bit, not update cfg params
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE0_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE1_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE2_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE3_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_YUVAE_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE0_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE1_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE1_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE2_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE2_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE3_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_YUVAE_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_YUVAE_ID);

        return;
    }

    LOGD_CAMHW_SUBM(0xff, "xuhf-debug: isp_cfg: win size: [%dx%d]-[%dx%d] [%dx%d]-[%dx%d]\n",
                    isp_params->isp_cfg->meas.rawae0.win0_h_offset,
                    isp_params->isp_cfg->meas.rawae0.win0_v_offset,
                    isp_params->isp_cfg->meas.rawae0.win0_h_size,
                    isp_params->isp_cfg->meas.rawae0.win0_v_size,
                    isp_params->isp_cfg->meas.rawae3.win0_h_offset,
                    isp_params->isp_cfg->meas.rawae3.win0_v_offset,
                    isp_params->isp_cfg->meas.rawae3.win0_h_size,
                    isp_params->isp_cfg->meas.rawae3.win0_v_size);
#else
    if(ae_meas->sw_aeCfg_stats_en) {

        // 0) module_ens
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE1_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE2_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_YUVAE_ID;

        // 1) rawae0
        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawae0.rawae_sel = 0x20;

            isp_params->isp_cfg->meas.rawae0.wnd_num = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae0.win.h_offs = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win.v_offs = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win.h_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win.v_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[1] = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[1].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[1].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[1].h_size = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[1].v_size = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[2] = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[2].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[2].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[2].h_size = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[2].v_size = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[3] = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[3].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[3].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[3].h_size = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[3].v_size = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_height;

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

            if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl0DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 0;

            } else if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl1DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 1;

            } else if(ae_meas->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutLow_mode) {
                isp_params->isp_cfg->meas.rawae0.rawae_sel = 0x20;
            }

            isp_params->isp_cfg->meas.rawae0.wnd_num = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae0.win.h_offs = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.win.v_offs = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.win.h_size = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.win.v_size = ae_meas->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[1] = ae_meas->entityGroup.entities.entity0.subWin[1].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[1].h_offs = ae_meas->entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[1].v_offs = ae_meas->entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[1].h_size = ae_meas->entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[1].v_size = ae_meas->entityGroup.entities.entity0.subWin[1].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[2] = ae_meas->entityGroup.entities.entity0.subWin[2].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[2].h_offs = ae_meas->entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[2].v_offs = ae_meas->entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[2].h_size = ae_meas->entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[2].v_size = ae_meas->entityGroup.entities.entity0.subWin[2].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae0.subwin_en[3] = ae_meas->entityGroup.entities.entity0.subWin[3].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae0.subwin[3].h_offs = ae_meas->entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae0.subwin[3].v_offs = ae_meas->entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae0.subwin[3].h_size = ae_meas->entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae0.subwin[3].v_size = ae_meas->entityGroup.entities.entity0.subWin[3].hw_aeCfg_win_height;

        }

        // rawae3

        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawae3.rawae_sel = 0x10;

            isp_params->isp_cfg->meas.rawae3.wnd_num = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae3.win.h_offs = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win.v_offs = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win.h_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win.v_size = ae_meas->entityGroup.coWkEntity03.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->entityGroup.coWkEntity03.subWin[0].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[1] = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[1].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[1].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[1].h_size = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[1].v_size = ae_meas->entityGroup.coWkEntity03.subWin[1].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->entityGroup.coWkEntity03.subWin[2].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[3] = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[3].h_offs = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[3].v_offs = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[3].h_size = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[3].v_size = ae_meas->entityGroup.coWkEntity03.subWin[3].hw_aeCfg_win_height;

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

            if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl0DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 0;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl1DpcOut_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 1;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_dmIn_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 3;

            } else if(ae_meas->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutHigh_mode) {
                isp_params->isp_cfg->meas.rawae3.rawae_sel = 0x10;
            }

            isp_params->isp_cfg->meas.rawae3.wnd_num = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawae3.win.h_offs = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.win.v_offs = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.win.h_size = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.win.v_size = ae_meas->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[1] = ae_meas->entityGroup.entities.entity3.subWin[1].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[1].h_offs = ae_meas->entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[1].v_offs = ae_meas->entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[1].h_size = ae_meas->entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[1].v_size = ae_meas->entityGroup.entities.entity3.subWin[1].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[2] = ae_meas->entityGroup.entities.entity3.subWin[2].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[2].h_offs = ae_meas->entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[2].v_offs = ae_meas->entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[2].h_size = ae_meas->entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[2].v_size = ae_meas->entityGroup.entities.entity3.subWin[2].hw_aeCfg_win_height;

            isp_params->isp_cfg->meas.rawae3.subwin_en[3] = ae_meas->entityGroup.entities.entity3.subWin[3].hw_aeCfg_subWin_en;
            isp_params->isp_cfg->meas.rawae3.subwin[3].h_offs = ae_meas->entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawae3.subwin[3].v_offs = ae_meas->entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawae3.subwin[3].h_size = ae_meas->entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawae3.subwin[3].v_size = ae_meas->entityGroup.entities.entity3.subWin[3].hw_aeCfg_win_height;

        }

        if(/*aec_meas.ae_meas_update*/1) {
            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE0_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE1_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE1_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE2_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE2_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE3_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_YUVAE_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_YUVAE_ID;

        } else {
            return;
        }
    } else {
        // only update en bit, not update cfg params
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE0_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE1_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE2_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWAE3_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_YUVAE_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE0_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE1_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE1_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE2_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE2_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWAE3_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_YUVAE_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_YUVAE_ID);

        return;
    }

    LOGD_CAMHW_SUBM(0xff, "xuhf-debug: isp_cfg: win size: [%dx%d]-[%dx%d] [%dx%d]-[%dx%d]\n",
                    isp_params->isp_cfg->meas.rawae0.win.h_offs,
                    isp_params->isp_cfg->meas.rawae0.win.v_offs,
                    isp_params->isp_cfg->meas.rawae0.win.h_size,
                    isp_params->isp_cfg->meas.rawae0.win.v_size,
                    isp_params->isp_cfg->meas.rawae3.win.h_offs,
                    isp_params->isp_cfg->meas.rawae3.win.v_offs,
                    isp_params->isp_cfg->meas.rawae3.win.h_size,
                    isp_params->isp_cfg->meas.rawae3.win.v_size);
#endif
}

static void convertAiqHistToIsp39Params(isp_params_t* isp_params, const aeStats_cfg_t *ae_meas)
{
#if ISP_HW_V35
    if (ae_meas->sw_aeCfg_stats_en) {

        // 0) module_ens
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST0_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST1_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST2_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST3_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_SIHST_ID;

        // 1) rawhist0
        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawhist0.wnd_num = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist0.data_sel = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist0.waterline = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist0.mode = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist0.stepsize = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist0.rcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist0.gcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist0.bcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist0.off = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist0.h_offset = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist0.v_offset = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist0.h_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist0.v_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist0.weight[i] = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

            isp_params->isp_cfg->meas.rawhist0.wnd_num = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist0.data_sel = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist0.waterline = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist0.mode = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist0.stepsize = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist0.rcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist0.gcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist0.bcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist0.off = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist0.h_offset = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist0.v_offset = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist0.h_size = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist0.v_size = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist0.weight[i] = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt[i];

        }

        // rawhist3

        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawhist3.wnd_num = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist3.data_sel = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist3.waterline = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist3.mode = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist3.stepsize = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist3.rcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist3.gcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist3.bcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist3.off = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist3.h_offset = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist3.v_offset = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist3.h_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist3.v_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist3.weight[i] = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

            isp_params->isp_cfg->meas.rawhist3.wnd_num = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist3.data_sel = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist3.waterline = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist3.mode = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist3.stepsize = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist3.rcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist3.gcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist3.bcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist3.off = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist3.h_offset = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist3.v_offset = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist3.h_size = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist3.v_size = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist3.weight[i] = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt[i];

        }

        if(/*aec_meas.ae_meas_update*/1) {
            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST0_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST0_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST1_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST1_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST2_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST2_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST3_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST3_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_SIHST_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_SIHST_ID;

        } else {
            return;
        }
    } else {
        // only update en bit, not update cfg params
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST0_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST1_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST2_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST3_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_SIHST_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST0_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST0_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST1_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST1_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST2_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST2_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST3_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST3_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_SIHST_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_SIHST_ID);

        return;
    }

    LOGD_CAMHW_SUBM(0xff, "xuhf-debug: isp_cfg: hist win size: [%dx%d]-[%dx%d] [%dx%d]-[%dx%d]\n",
                    isp_params->isp_cfg->meas.rawhist0.h_offset,
                    isp_params->isp_cfg->meas.rawhist0.v_offset,
                    isp_params->isp_cfg->meas.rawhist0.h_size,
                    isp_params->isp_cfg->meas.rawhist0.v_size,
                    isp_params->isp_cfg->meas.rawhist3.h_offset,
                    isp_params->isp_cfg->meas.rawhist3.v_offset,
                    isp_params->isp_cfg->meas.rawhist3.h_size,
                    isp_params->isp_cfg->meas.rawhist3.v_size);
#else
    if (ae_meas->sw_aeCfg_stats_en) {

        // 0) module_ens
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST0_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST1_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST2_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST3_ID;
        isp_params->isp_cfg->module_ens |= 1LL << RK_ISP2X_SIHST_ID;

        // 1) rawhist0
        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawhist0.wnd_num = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist0.data_sel = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist0.waterline = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist0.mode = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist0.stepsize = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist0.rcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist0.gcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist0.bcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist0.off = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist0.win.h_offs = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist0.win.v_offs = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist0.win.h_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist0.win.v_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist0.weight[i] = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

            isp_params->isp_cfg->meas.rawhist0.wnd_num = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist0.data_sel = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist0.waterline = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist0.mode = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist0.stepsize = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist0.rcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist0.gcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist0.bcc = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist0.off = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist0.win.h_offs = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist0.win.v_offs = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist0.win.h_size = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist0.win.v_size = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist0.weight[i] = ae_meas->entityGroup.entities.entity0.hist.hw_aeCfg_zone_wgt[i];

        }

        // rawhist3

        if(ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            isp_params->isp_cfg->meas.rawhist3.wnd_num = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist3.data_sel = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist3.waterline = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist3.mode = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist3.stepsize = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist3.rcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist3.gcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist3.bcc = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist3.off = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist3.win.h_offs = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist3.win.v_offs = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist3.win.h_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist3.win.v_size = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist3.weight[i] = ae_meas->entityGroup.coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

        } else if (ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
                   ae_meas->hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

            isp_params->isp_cfg->meas.rawhist3.wnd_num = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_winZone_mode;
            isp_params->isp_cfg->meas.rawhist3.data_sel = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixIn_shift;
            isp_params->isp_cfg->meas.rawhist3.waterline = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixVal_thred;
            isp_params->isp_cfg->meas.rawhist3.mode = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pix_mode;
            isp_params->isp_cfg->meas.rawhist3.stepsize = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_pixSkip_step;
            isp_params->isp_cfg->meas.rawhist3.rcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[0];
            isp_params->isp_cfg->meas.rawhist3.gcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[1];
            isp_params->isp_cfg->meas.rawhist3.bcc = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[2];
            isp_params->isp_cfg->meas.rawhist3.off = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_bayer2Y_coeff[3];

            isp_params->isp_cfg->meas.rawhist3.win.h_offs = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_x;
            isp_params->isp_cfg->meas.rawhist3.win.v_offs = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_y;
            isp_params->isp_cfg->meas.rawhist3.win.h_size = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_width;
            isp_params->isp_cfg->meas.rawhist3.win.v_size = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_win_height;

            for(int i = 0; i < ISP2X_RAWHISTBIG_SUBWIN_NUM; i++)
                isp_params->isp_cfg->meas.rawhist3.weight[i] = ae_meas->entityGroup.entities.entity3.hist.hw_aeCfg_zone_wgt[i];

        }

        if(/*aec_meas.ae_meas_update*/1) {
            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST0_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST0_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST1_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST1_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST2_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST2_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST3_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST3_ID;

            isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_SIHST_ID;
            isp_params->isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_SIHST_ID;

        } else {
            return;
        }
    } else {
        // only update en bit, not update cfg params
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST0_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST1_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST2_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_RAWHIST3_ID);
        isp_params->isp_cfg->module_ens &= ~(1LL << RK_ISP2X_SIHST_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST0_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST0_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST1_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST1_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST2_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST2_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST3_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_RAWHIST3_ID);

        isp_params->isp_cfg->module_en_update |= 1LL << RK_ISP2X_SIHST_ID;
        isp_params->isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_SIHST_ID);

        return;
    }

    LOGD_CAMHW_SUBM(0xff, "xuhf-debug: isp_cfg: hist win size: [%dx%d]-[%dx%d] [%dx%d]-[%dx%d]\n",
                    isp_params->isp_cfg->meas.rawhist0.win.h_offs,
                    isp_params->isp_cfg->meas.rawhist0.win.v_offs,
                    isp_params->isp_cfg->meas.rawhist0.win.h_size,
                    isp_params->isp_cfg->meas.rawhist0.win.v_size,
                    isp_params->isp_cfg->meas.rawhist3.win.h_offs,
                    isp_params->isp_cfg->meas.rawhist3.win.v_offs,
                    isp_params->isp_cfg->meas.rawhist3.win.h_size,
                    isp_params->isp_cfg->meas.rawhist3.win.v_size);
#endif
}

void rk_aiq_ae25_stats_cfg_cvt(void* meas, isp_params_t* isp_params)
{
    aeStats_cfg_t *aeMeas = (aeStats_cfg_t *)meas;
    convertAiqAeToIsp39Params(isp_params, aeMeas);
    convertAiqHistToIsp39Params(isp_params, aeMeas);
}

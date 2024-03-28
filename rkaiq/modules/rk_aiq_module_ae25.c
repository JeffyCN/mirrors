void convertAiqAeToIsp39Params(struct isp39_isp_params_cfg *isp_cfg, aeStats_cfg_t *ae_meas)
{
	if(ae_meas->sw_aeCfg_stats_en) {

		// 0) module_ens
		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){
			//for bnr20bit input
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;

			isp_cfg->meas.rawae3.rawae_sel = 0x10;

		}else if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity0_wkOnly_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE0_ID;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity3_wkOnly_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWAE3_ID;
		}

		// 1) rawae0
		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){

			isp_cfg->meas.rawae0.rawae_sel = 0x20;

			isp_cfg->meas.rawae0.wnd_num = ae_meas->coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawae0.win.h_offs = ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.win.v_offs= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.win.h_size= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.win.v_size= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[1] = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[1].h_offs = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[1].v_offs = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[1].h_size = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[1].v_size = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[3] = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[3].h_offs = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[3].v_offs = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[3].h_size = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[3].v_size = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_height;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode || \
			ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity0_wkOnly_mode){

			if(ae_meas->entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl0Wb0Out_mode){
				isp_cfg->meas.rawae0.rawae_sel = 0;

			}else if(ae_meas->entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl1Wb0Out_mode){
				isp_cfg->meas.rawae0.rawae_sel = 1;

			}else if(ae_meas->entity0.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutLow_mode){
				isp_cfg->meas.rawae0.rawae_sel = 0x20;
			}

			isp_cfg->meas.rawae0.wnd_num = ae_meas->entity0.mainWin.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawae0.win.h_offs = ae_meas->entity0.mainWin.hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.win.v_offs= ae_meas->entity0.mainWin.hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.win.h_size= ae_meas->entity0.mainWin.hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.win.v_size= ae_meas->entity0.mainWin.hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->entity0.subWin[0].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->entity0.subWin[0].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->entity0.subWin[0].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->entity0.subWin[0].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->entity0.subWin[0].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[1] = ae_meas->entity0.subWin[1].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[1].h_offs = ae_meas->entity0.subWin[1].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[1].v_offs = ae_meas->entity0.subWin[1].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[1].h_size = ae_meas->entity0.subWin[1].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[1].v_size = ae_meas->entity0.subWin[1].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[0] = ae_meas->entity0.subWin[2].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[0].h_offs = ae_meas->entity0.subWin[2].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[0].v_offs = ae_meas->entity0.subWin[2].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[0].h_size = ae_meas->entity0.subWin[2].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[0].v_size = ae_meas->entity0.subWin[2].hw_aeCfg_win_height;

			isp_cfg->meas.rawae0.subwin_en[3] = ae_meas->entity0.subWin[3].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae0.subwin[3].h_offs = ae_meas->entity0.subWin[3].hw_aeCfg_win_x;
			isp_cfg->meas.rawae0.subwin[3].v_offs = ae_meas->entity0.subWin[3].hw_aeCfg_win_y;
			isp_cfg->meas.rawae0.subwin[3].h_size = ae_meas->entity0.subWin[3].hw_aeCfg_win_width;
			isp_cfg->meas.rawae0.subwin[3].v_size = ae_meas->entity0.subWin[3].hw_aeCfg_win_height;

		}

		// rawae3

		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){

			isp_cfg->meas.rawae3.rawae_sel = 0x10;

			isp_cfg->meas.rawae3.wnd_num = ae_meas->coWkEntity03.mainWin.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawae3.win.h_offs = ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.win.v_offs= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.win.h_size= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.win.v_size= ae_meas->coWkEntity03.mainWin.hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->coWkEntity03.subWin[0].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[1] = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[1].h_offs = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[1].v_offs = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[1].h_size = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[1].v_size = ae_meas->coWkEntity03.subWin[1].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->coWkEntity03.subWin[2].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[3] = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[3].h_offs = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[3].v_offs = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[3].h_size = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[3].v_size = ae_meas->coWkEntity03.subWin[3].hw_aeCfg_win_height;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode || \
			ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity3_wkOnly_mode){

			if(ae_meas->entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl0Wb0Out_mode){
				isp_cfg->meas.rawae3.rawae_sel = 0;

			}else if(ae_meas->entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl1Wb0Out_mode){
				isp_cfg->meas.rawae3.rawae_sel = 1;

			}else if(ae_meas->entity3.hw_aeCfg_statsSrc_mode ==  aeStats_dmIn_mode){
				isp_cfg->meas.rawae3.rawae_sel = 3;

			}else if(ae_meas->entity3.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutHigh_mode){
				isp_cfg->meas.rawae3.rawae_sel = 0x10;
			}

			isp_cfg->meas.rawae3.wnd_num = ae_meas->entity3.mainWin.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawae3.win.h_offs = ae_meas->entity3.mainWin.hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.win.v_offs= ae_meas->entity3.mainWin.hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.win.h_size= ae_meas->entity3.mainWin.hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.win.v_size= ae_meas->entity3.mainWin.hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->entity3.subWin[0].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->entity3.subWin[0].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->entity3.subWin[0].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->entity3.subWin[0].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->entity3.subWin[0].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[1] = ae_meas->entity3.subWin[1].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[1].h_offs = ae_meas->entity3.subWin[1].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[1].v_offs = ae_meas->entity3.subWin[1].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[1].h_size = ae_meas->entity3.subWin[1].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[1].v_size = ae_meas->entity3.subWin[1].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[0] = ae_meas->entity3.subWin[2].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[0].h_offs = ae_meas->entity3.subWin[2].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[0].v_offs = ae_meas->entity3.subWin[2].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[0].h_size = ae_meas->entity3.subWin[2].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[0].v_size = ae_meas->entity3.subWin[2].hw_aeCfg_win_height;

			isp_cfg->meas.rawae3.subwin_en[3] = ae_meas->entity3.subWin[3].hw_aeCfg_subWin_en;
			isp_cfg->meas.rawae3.subwin[3].h_offs = ae_meas->entity3.subWin[3].hw_aeCfg_win_x;
			isp_cfg->meas.rawae3.subwin[3].v_offs = ae_meas->entity3.subWin[3].hw_aeCfg_win_y;
			isp_cfg->meas.rawae3.subwin[3].h_size = ae_meas->entity3.subWin[3].hw_aeCfg_win_width;
			isp_cfg->meas.rawae3.subwin[3].v_size = ae_meas->entity3.subWin[3].hw_aeCfg_win_height;

		}

		if(/*aec_meas.ae_meas_update*/1) {
			isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE0_ID;
			isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE0_ID;

			isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWAE3_ID;
			isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWAE3_ID;

		} else {
			return;
		}
	} else {
		return;
	}
}

void convertAiqHistToIsp39Params(struct isp39_isp_params_cfg *isp_cfg, const aeStats_cfg_t *ae_meas)
{

	if(ae_meas->sw_aeCfg_stats_en) {

		// 0) module_ens
		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){
			//for bnr20bit input
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST0_ID;
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST3_ID;

			isp_cfg->meas.rawae3.rawae_sel = 0x10;

		}else if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST0_ID;
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST3_ID;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity0_wkOnly_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST0_ID;

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity3_wkOnly_mode){
			isp_cfg->module_ens |= 1LL << RK_ISP2X_RAWHIST3_ID;
		}

		// 1) rawhist0
		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){

			isp_cfg->meas.rawhist0.wnd_num = ae_meas->coWkEntity03.hist.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawhist0.data_sel = ae_meas->coWkEntity03.hist.hw_aeCfg_pixIn_shift;
			isp_cfg->meas.rawhist0.waterline = ae_meas->coWkEntity03.hist.hw_aeCfg_pixCnt_thred;
			isp_cfg->meas.rawhist0.mode = ae_meas->coWkEntity03.hist.hw_aeCfg_pix_mode;
			isp_cfg->meas.rawhist0.stepsize = ae_meas->coWkEntity03.hist.hw_aeCfg_pixSkip_step;
			isp_cfg->meas.rawhist0.rcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
			isp_cfg->meas.rawhist0.gcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
			isp_cfg->meas.rawhist0.bcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
			isp_cfg->meas.rawhist0.off = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

			isp_cfg->meas.rawhist0.win.h_offs = ae_meas->coWkEntity03.hist.hw_aeCfg_win_x;
			isp_cfg->meas.rawhist0.win.v_offs= ae_meas->coWkEntity03.hist.hw_aeCfg_win_y;
			isp_cfg->meas.rawhist0.win.h_size= ae_meas->coWkEntity03.hist.hw_aeCfg_win_width;
			isp_cfg->meas.rawhist0.win.v_size= ae_meas->coWkEntity03.hist.hw_aeCfg_win_height;

			for(int i=0;i<ISP2X_RAWHISTBIG_SUBWIN_NUM;i++)
				isp_cfg->meas.rawhist0.weight[i] = ae_meas->coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode || \
			ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity0_wkOnly_mode){

			isp_cfg->meas.rawhist0.wnd_num = ae_meas->entity0.hist.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawhist0.data_sel = ae_meas->entity0.hist.hw_aeCfg_pixIn_shift;
			isp_cfg->meas.rawhist0.waterline = ae_meas->entity0.hist.hw_aeCfg_pixCnt_thred;
			isp_cfg->meas.rawhist0.mode = ae_meas->entity0.hist.hw_aeCfg_pix_mode;
			isp_cfg->meas.rawhist0.stepsize = ae_meas->entity0.hist.hw_aeCfg_pixSkip_step;
			isp_cfg->meas.rawhist0.rcc = ae_meas->entity0.hist.hw_aeCfg_bayer2Y_coeff[0];
			isp_cfg->meas.rawhist0.gcc = ae_meas->entity0.hist.hw_aeCfg_bayer2Y_coeff[1];
			isp_cfg->meas.rawhist0.bcc = ae_meas->entity0.hist.hw_aeCfg_bayer2Y_coeff[2];
			isp_cfg->meas.rawhist0.off = ae_meas->entity0.hist.hw_aeCfg_bayer2Y_coeff[3];

			isp_cfg->meas.rawhist0.win.h_offs = ae_meas->entity0.hist.hw_aeCfg_win_x;
			isp_cfg->meas.rawhist0.win.v_offs= ae_meas->entity0.hist.hw_aeCfg_win_y;
			isp_cfg->meas.rawhist0.win.h_size= ae_meas->entity0.hist.hw_aeCfg_win_width;
			isp_cfg->meas.rawhist0.win.v_size= ae_meas->entity0.hist.hw_aeCfg_win_height;

			for(int i=0;i<ISP2X_RAWHISTBIG_SUBWIN_NUM;i++)
				isp_cfg->meas.rawhist0.weight[i] = ae_meas->entity0.hist.hw_aeCfg_zone_wgt[i];

		}

		// rawhist3

		if(ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_coWk_mode){

			isp_cfg->meas.rawhist3.wnd_num = ae_meas->coWkEntity03.hist.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawhist3.data_sel = ae_meas->coWkEntity03.hist.hw_aeCfg_pixIn_shift;
			isp_cfg->meas.rawhist3.waterline = ae_meas->coWkEntity03.hist.hw_aeCfg_pixCnt_thred;
			isp_cfg->meas.rawhist3.mode = ae_meas->coWkEntity03.hist.hw_aeCfg_pix_mode;
			isp_cfg->meas.rawhist3.stepsize = ae_meas->coWkEntity03.hist.hw_aeCfg_pixSkip_step;
			isp_cfg->meas.rawhist3.rcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[0];
			isp_cfg->meas.rawhist3.gcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[1];
			isp_cfg->meas.rawhist3.bcc = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[2];
			isp_cfg->meas.rawhist3.off = ae_meas->coWkEntity03.hist.hw_aeCfg_bayer2Y_coeff[3];

			isp_cfg->meas.rawhist3.win.h_offs = ae_meas->coWkEntity03.hist.hw_aeCfg_win_x;
			isp_cfg->meas.rawhist3.win.v_offs= ae_meas->coWkEntity03.hist.hw_aeCfg_win_y;
			isp_cfg->meas.rawhist3.win.h_size= ae_meas->coWkEntity03.hist.hw_aeCfg_win_width;
			isp_cfg->meas.rawhist3.win.v_size= ae_meas->coWkEntity03.hist.hw_aeCfg_win_height;

			for(int i=0;i<ISP2X_RAWHISTBIG_SUBWIN_NUM;i++)
				isp_cfg->meas.rawhist3.weight[i] = ae_meas->coWkEntity03.hist.hw_aeCfg_zone_wgt[i];

		}else if (ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity03_independentWk_mode || \
			ae_meas->hw_aeCfg_statsWork_mode == aeStats_entity3_wkOnly_mode){

			isp_cfg->meas.rawhist3.wnd_num = ae_meas->entity3.hist.hw_aeCfg_winZone_mode;
			isp_cfg->meas.rawhist3.data_sel = ae_meas->entity3.hist.hw_aeCfg_pixIn_shift;
			isp_cfg->meas.rawhist3.waterline = ae_meas->entity3.hist.hw_aeCfg_pixCnt_thred;
			isp_cfg->meas.rawhist3.mode = ae_meas->entity3.hist.hw_aeCfg_pix_mode;
			isp_cfg->meas.rawhist3.stepsize = ae_meas->entity3.hist.hw_aeCfg_pixSkip_step;
			isp_cfg->meas.rawhist3.rcc = ae_meas->entity3.hist.hw_aeCfg_bayer2Y_coeff[0];
			isp_cfg->meas.rawhist3.gcc = ae_meas->entity3.hist.hw_aeCfg_bayer2Y_coeff[1];
			isp_cfg->meas.rawhist3.bcc = ae_meas->entity3.hist.hw_aeCfg_bayer2Y_coeff[2];
			isp_cfg->meas.rawhist3.off = ae_meas->entity3.hist.hw_aeCfg_bayer2Y_coeff[3];

			isp_cfg->meas.rawhist3.win.h_offs = ae_meas->entity3.hist.hw_aeCfg_win_x;
			isp_cfg->meas.rawhist3.win.v_offs= ae_meas->entity3.hist.hw_aeCfg_win_y;
			isp_cfg->meas.rawhist3.win.h_size= ae_meas->entity3.hist.hw_aeCfg_win_width;
			isp_cfg->meas.rawhist3.win.v_size= ae_meas->entity3.hist.hw_aeCfg_win_height;

			for(int i=0;i<ISP2X_RAWHISTBIG_SUBWIN_NUM;i++)
				isp_cfg->meas.rawhist3.weight[i] = ae_meas->entity3.hist.hw_aeCfg_zone_wgt[i];

		}

		if(/*aec_meas.ae_meas_update*/1) {
			isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST0_ID;
			isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST0_ID;

			isp_cfg->module_en_update |= 1LL << RK_ISP2X_RAWHIST3_ID;
			isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_RAWHIST3_ID;

		} else {
			return;
		}
	} else {
		return;
	}
}

void rk_aiq_ae25_stats_cfg_cvt(void* meas, struct isp39_isp_params_cfg* isp_cfg)
{
    aeStats_cfg_t *aeMeas = (aeStats_cfg_t *)meas;
    convertAiqAeToIsp39Params(isp_cfg, aeMeas);
    convertAiqHistToIsp39Params(isp_cfg, aeMeas);
}

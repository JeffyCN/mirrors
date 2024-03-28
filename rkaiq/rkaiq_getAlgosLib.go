package rkaiqdefaults

import (
    // "fmt"
)

func rkaiq_getAlgosLib(macros_map map[string]bool) []string {

    var flag0 bool = false;
    var flag1 bool = false;
    var flag2 bool = false;
    var flag3 bool = false;
    var flag4 bool = false;
    var flag5 bool = false;
    var flag6 bool = false;
    var flag7 bool = false;
    var flag8 bool = false;
    var flag9 bool = false;

    var static_lib []string;

    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_camgroup_ae")
        static_lib = append(static_lib, "librkaiq_camgroup_awb")
        static_lib = append(static_lib, "librkaiq_camgroup_misc")
    }
    flag0 = macros_map["RKAIQ_HAVE_AWB_V20"]
    flag1 = macros_map["RKAIQ_HAVE_AWB_V21"]
    flag2 = macros_map["RKAIQ_HAVE_AWB_V32"]
    flag3 = macros_map["RKAIQ_HAVE_AWB_V32LT"]
    if (flag0 || flag1 || flag2 || flag3) {
        static_lib = append(static_lib, "librkaiq_awb")
    }

    flag0 = macros_map["RKAIQ_HAVE_GIC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_GIC_V2"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_agic")
    }

    flag0 = macros_map["RKAIQ_HAVE_ORB_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aorb")
    }
    flag0 = macros_map["RKAIQ_HAVE_EIS_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aeis")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V03"]
    flag1 = macros_map["RKAIQ_HAVE_CAC_V10"]
    flag2 = macros_map["RKAIQ_HAVE_CAC_V11"]
    flag3 = macros_map["RKAIQ_HAVE_CAC_V12"]
    if (flag0 || flag1 || flag2) {
        static_lib = append(static_lib, "librkaiq_acac")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aynr_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aynr_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V3"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aynr_v3")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V22"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aynr_v22")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V24"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aynr_v24")
    }
    flag0 = macros_map["RKAIQ_HAVE_UVNR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "rkaiq_auvnr_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acnr_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acnr_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V30"]
    flag1 = macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
	if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_acnr_v30")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V31"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acnr_v31")
    }
    flag0 = macros_map["RKAIQ_HAVE_YUVME_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_ayuvme_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_asharp")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_asharp_v3")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_asharp_v4")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_asharp_v33")
    }
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_asharp_v34")
    }
    flag0 = macros_map["RKAIQ_HAVE_MFNR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_amfnr_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_arawnr_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_arawnr_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_abayer2dnr_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
    flag1 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23_LITE"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_abayer2dnr_v23")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_abayertnr_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_abayertnr_v23")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_abayertnr_v30")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAIN_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_again_v1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_again_v2")
    }
    flag0 = macros_map["RKAIQ_HAVE_BLC_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_ablc")
    }
    flag0 = macros_map["RKAIQ_HAVE_BLC_V32"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_ablc_v32")
    }
    flag0 = macros_map["RKAIQ_HAVE_ANR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_anr")
    }
    flag0 = macros_map["RKAIQ_HAVE_AMD_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_amd")
    }
    flag0 = macros_map["RKAIQ_HAVE_PDAF"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_pdaf")
    }
    flag0 = macros_map["RKAIQ_HAVE_3DLUT_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_a3dlut")
    }

    flag0 = macros_map["RKAIQ_HAVE_CCM_V1"]
    flag1 = macros_map["RKAIQ_HAVE_CCM_V2"]
    flag2 = macros_map["RKAIQ_HAVE_CCM_V3"]
    if (flag0 || flag1 || flag2) {
        static_lib = append(static_lib, "librkaiq_accm")
    }

    flag0 = macros_map["RKAIQ_HAVE_GAMMA_V11"]
    flag1 = macros_map["RKAIQ_HAVE_GAMMA_V10"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_agamma")
    }

    flag0 = macros_map["RKAIQ_HAVE_MERGE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_MERGE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_MERGE_V12"]
    if (flag0 || flag1 ||flag2) {
        static_lib = append(static_lib, "librkaiq_amerge")
    }

    flag0 = macros_map["RKAIQ_HAVE_DRC_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DRC_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DRC_V12"]
    flag3 = macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
    flag4 = macros_map["RKAIQ_HAVE_DRC_V20"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        static_lib = append(static_lib, "librkaiq_adrc")
    }

    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_argbir")
    }

    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DEHAZE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
    flag3 = macros_map["RKAIQ_HAVE_DEHAZE_V12"]
    flag4 = macros_map["RKAIQ_HAVE_DEHAZE_V14"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        static_lib = append(static_lib, "librkaiq_adehaze")
    }

    flag0 = macros_map["RKAIQ_HAVE_TMO_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_atmo")
    }
    flag0 = macros_map["RKAIQ_HAVE_WDR_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_awdr")
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V21"]
    flag1 = macros_map["RKAIQ_HAVE_LDCH_V10"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_aldch")
    }
    flag0 = macros_map["RKAIQ_HAVE_FEC_V10"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_afec")
    }
    flag0 = macros_map["RKAIQ_HAVE_CSM_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acsm")
    }
    flag0 = macros_map["RKAIQ_HAVE_CGC_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acgc")
    }
    flag0 = macros_map["RKAIQ_HAVE_AE_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_ae")
    }

    flag0 = macros_map["RKAIQ_HAVE_AFD_V1"]
    flag1 = macros_map["RKAIQ_HAVE_AFD_V2"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_afd")
    }

    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DEBAYER_V2"]
    flag2 = macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
    flag3 = macros_map["RKAIQ_HAVE_DEBAYER_V3"]
    if (flag0 || flag1 || flag2 || flag3) {
        static_lib = append(static_lib, "librkaiq_adebayer")
    }

    flag0 = macros_map["RKAIQ_HAVE_LSC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_LSC_V2"]
    flag2 = macros_map["RKAIQ_HAVE_LSC_V3"]
    if (flag0 || flag1 || flag2) {
        static_lib = append(static_lib, "librkaiq_alsc")
    }
    flag0 = macros_map["RKAIQ_HAVE_DPCC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DPCC_V2"]
    if (flag0 || flag1) {
        static_lib = append(static_lib, "librkaiq_adpcc")
    }
    flag0 = macros_map["RKAIQ_HAVE_ACP_V10"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_acp")
    }
    flag0 = macros_map["RKAIQ_HAVE_AIE_V10"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_aie")
    }
    flag0 = macros_map["RKAIQ_HAVE_ASD_V10"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_asd")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEGAMMA_V1"]
    if (flag0) {
        static_lib = append(static_lib, "librkaiq_adegamma")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V20"]
    flag1 = macros_map["RKAIQ_HAVE_AF_V30"]
    flag2 = macros_map["RKAIQ_HAVE_AF_V31"]
    flag3 = macros_map["RKAIQ_HAVE_AF_V32_LITE"]
    flag4 = macros_map["RKAIQ_ONLY_AF_STATS_V20"]
    flag5 = macros_map["RKAIQ_ONLY_AF_STATS_V30"]
    flag6 = macros_map["RKAIQ_ONLY_AF_STATS_V31"]
    flag7 = macros_map["RKAIQ_ONLY_AF_STATS_V32_LITE"]
    flag8 = macros_map["RKAIQ_HAVE_AF_V33"]
    flag9 = macros_map["RKAIQ_ONLY_AF_STATS_V33"]
    if (flag0 || flag1 || flag2 || flag3 || flag8) {
        static_lib = append(static_lib, "librkaiq_af")
    } else if (flag4 || flag5 || flag6 || flag7 || flag9) {
        static_lib = append(static_lib, "librkaiq_thirdaf")
    }

    // for i, v := range static_lib {
    //     fmt.Printf("%d %s\n", i, v)
    // }

    // fmt.Printf("static_lib size: %d\n", len(static_lib))
    return static_lib;
}

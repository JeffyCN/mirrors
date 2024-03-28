package rkaiqdefaults

import (
    "fmt"
)

func rkaiq_getAlgosMacros(macros_map map[string]bool) []string {
    var flag0 bool = false;

    var cflags []string;

    flag0 = macros_map["RKAIQ_USE_RAWSTREAM_LIB"]
    if (flag0) {
        cflags = append(cflags, "-DUSE_RAWSTREAM_LIB")
    }

    flag0 = macros_map["RKAIQ_ENABLE_PARSER_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_ENABLE_PARSER_V1")
    }
    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_ENABLE_CAMGROUP")
    }
    flag0 = macros_map["RKAIQ_HAVE_THUMBNAILS"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_THUMBNAILS")
    }
    flag0 = macros_map["RKAIQ_HAVE_FAKECAM"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_ENABLE_FAKECAM")
    }
    flag0 = macros_map["RKAIQ_HAVE_SPSTREAM"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SPSTREAM")
    }
    flag0 = macros_map["RKAIQ_HAVE_MULTIISP"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_MULTIISP")
    }
    flag0 = macros_map["RKAIQ_HAVE_AWB_V20"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB_V20=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AWB_V21"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB_V21=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AWB_V32"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB_V32=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GIC_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GIC_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GIC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GIC_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GIC_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GIC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_ORB_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_ORB_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_ORB=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_EIS_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_EIS_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_EIS=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CAC_V03"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC_V03=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CAC_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CAC_V11"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC_V11=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CAC_V12"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC_V12=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CAC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V3"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR_V3=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V22"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR_V22=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YNR_V24"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR_V24=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_UVNR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_UVNR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_UVNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V30"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR_V30=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V31"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR_V31=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V3=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V4=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V33=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V34=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_MFNR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_MFNR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_MFNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERNR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERNR_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYER2DNR_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYER2DNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYER2DNR_V23=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYER2DNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR_V23=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR_V30=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAIN_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GAIN_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GAIN=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GAIN_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GAIN=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BLC_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BLC_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BLC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BLC_V32"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BLC_V32=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BLC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_ANR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_ANR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_ANR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AMD_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AMD_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AMD=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_YUVME_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_YUVME_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_YUVME=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_PDAF"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_PDAF=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_3DLUT_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_3DLUT_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_3DLUT=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CCM_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CCM_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CCM_V3"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM_V3=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CCM=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAMMA_V11"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GAMMA_V11=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GAMMA=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_GAMMA_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_GAMMA_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_GAMMA=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_MERGE_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_MERGE_V11"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE_V11=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_MERGE_V12"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE_V12=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_MERGE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DRC_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DRC_V11"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC_V11=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DRC_V12"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC_V12=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DRC_V20"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC_V20=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V11"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE_V11=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE_V11_DUO=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V12"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE_V12=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V14"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE_V14=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEHAZE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_RGBIR_REMOSAIC_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_RGBIR_REMOSAIC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_TMO_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_TMO_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_TMO=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_WDR_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_WDR_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_WDR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_LDCH_V21"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_LDCH_V21=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_LDCH=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_LDCH_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_LDCH_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_LDCH=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_FEC_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_FEC_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_FEC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CSM_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CSM_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CSM=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CGC_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CGC_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CGC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AE_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AE_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AFD_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AFD_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AFD=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AFD_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AFD_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AFD=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER_V2_LITE=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V3"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER_V3=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEBAYER=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_LSC_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_LSC_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_LSC_V3"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC_V3=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_LSC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DPCC_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DPCC_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DPCC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DPCC_V2"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DPCC_V2=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DPCC=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_ACP_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_ACP_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_ACP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AIE_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AIE_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AIE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_ASD_V10"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_ASD_V10=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_ASD=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEGAMMA_V1"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DEGAMMA_V1=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DEGAMMA=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V20"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AF_V20=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V30"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AF_V30=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V31"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AF_V31=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V33"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AF_V33=1")
    }
    flag0 = macros_map["RKAIQ_ONLY_AF_STATS_V20"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_ONLY_AF_STATS_V20=1")
    }
    flag0 = macros_map["RKAIQ_ONLY_AF_STATS_V30"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_ONLY_AF_STATS_V30=1")
    }
    flag0 = macros_map["RKAIQ_ONLY_AF_STATS_V31"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_ONLY_AF_STATS_V31=1")
    }
    flag0 = macros_map["RKAIQ_ONLY_AF_STATS_V33"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_ONLY_AF_STATS_V33=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AWB_V32LT"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB_V32LT=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AWB_V32LT=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR_V30_LITE=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_CNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP_V33_LITE=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_SHARP=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR_V23_LITE=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_BAYERTNR=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_AF_V32_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_AF_V32_LITE=1")
    }
    flag0 = macros_map["RKAIQ_ONLY_AF_STATS_V32_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_AF=1")
        cflags = append(cflags, "-DRKAIQ_ONLY_AF_STATS_V32_LITE=1")
    }
    flag0 = macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
    if (flag0) {
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC_V12_LITE=1")
        cflags = append(cflags, "-DRKAIQ_HAVE_DRC=1")
    }
    for i, v := range cflags {
        fmt.Printf("%d %s\n", i, v)
    }
    fmt.Printf("cflags size: %d\n", len(cflags))
    return cflags;
}

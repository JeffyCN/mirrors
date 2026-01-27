package rkaiqdefaults

import (
    // "fmt"
    "strings"
)

func rkaiq_getStaticLibSrcs(name string, macros_map map[string]bool) []string {
    var srcs []string

    if (strings.Compare(name, "librkaiq_awb") == 0) {
        srcs = srcs[len(srcs):]
        flag0 := macros_map["RKAIQ_HAVE_AWB_V20"]
        flag1 := macros_map["RKAIQ_HAVE_AWB_V21"]
        flag2 := macros_map["RKAIQ_HAVE_AWB_V32"]
        flag3 := macros_map["RKAIQ_HAVE_AWB_V32LT"]
        flag4 := macros_map["RKAIQ_HAVE_AWB_V39"]
        if flag0 || flag1 || flag2 || flag3 || flag4 {
            srcs = append(srcs, "color_space_convert.c")
            srcs = append(srcs, "fixfloat.c")
            srcs = append(srcs, "rk_aiq_algo_awb_itf.c")
            srcs = append(srcs, "rk_aiq_awb_algo_com2.c")
            srcs = append(srcs, "rk_aiq_uapi_awb_int.c")

            if flag0 {
                srcs = append(srcs, "rk_aiq_awb_algo_v200.c")
                srcs = append(srcs, "rk_aiq_awb_algo_sgc.c")
                srcs = append(srcs, "rk_aiq_awb_algo_com1.c")
                srcs = append(srcs, "rk_aiq_uapi_awb_int.c")
            }
            if flag1 {
                srcs = append(srcs, "rk_aiq_awb_algo_v201.c")
                srcs = append(srcs, "rk_aiq_awb_algo_sgc.c")
                srcs = append(srcs, "rk_aiq_awb_algo_com1.c")
                srcs = append(srcs, "rk_aiq_uapiv2_awb_int.c")
            }
            if flag2 || flag3 {
                srcs = append(srcs, "rk_aiq_awb_algo_v32.c")
                srcs = append(srcs, "rk_aiq_awb_algo_sgc.c")
                srcs = append(srcs, "rk_aiq_awb_algo_com1.c")
                srcs = append(srcs, "rk_aiq_uapiv2_awb_int.c")
            }
            if flag4 {
                srcs = append(srcs, "rk_aiq_awb_algo_v39.c")
                srcs = append(srcs, "rk_aiq_awb_algo_sgc2.c")
                srcs = append(srcs, "rk_aiq_awb_algo_com3.c")
                srcs = append(srcs, "rk_aiq_uapiv3_awb_int.c")
                srcs = append(srcs, "rk_aiq_awb_algo_scenedet.c")
            }
        }
        // fmt.Printf("%s srcs:", name, srcs)
        // fmt.Printf("\n")
        return srcs
    }

    useNewstruct := macros_map["RKAIQ_NEWSTRUCT_TEST"]
    if useNewstruct {
        srcs = rkaiq_getNewStructStaticLibSrcs(name, macros_map)
        return srcs
    }

    switch name {
    case "librkaiq_agic":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_GIC_V1"]
            flag1 := macros_map["RKAIQ_HAVE_GIC_V2"]
            if flag0 || flag1 {
                srcs = append(srcs, "agic/rk_aiq_algo_agic_itf.cpp")
                srcs = append(srcs, "agic/rk_aiq_uapi_agic_int.cpp")
                if flag0 {
                    srcs = append(srcs, "agic/rk_aiq_algo_agic_v1.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "agic/rk_aiq_algo_agic_v2.cpp")
                }
            }
            break
        }

    case "librkaiq_aorb":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_ORB_V1"]
            if flag0 {
                srcs = append(srcs, "aorb/rk_aiq_algo_aorb_itf.cpp")
                srcs = append(srcs, "aorb/orb_algos.cpp")
                srcs = append(srcs, "aorb/orb_algos_opencv.cpp")
            }
            break
        }

    case "librkaiq_aeis":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_EIS_V1"]
            if flag0 {
                srcs = append(srcs, "aeis/eis_loader.cpp")
                srcs = append(srcs, "aeis/scaler_service.cpp")
                srcs = append(srcs, "aeis/remap_backend.cpp")
                srcs = append(srcs, "aeis/imu_service.cpp")
                srcs = append(srcs, "aeis/eis_algo_service.cpp")
                srcs = append(srcs, "aeis/rk_aiq_uapi_aeis_int.cpp")
                srcs = append(srcs, "aeis/rk_aiq_algo_aeis_itf.cpp")
            }
            break
        }

    case "librkaiq_acac":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CAC_V03"]
            flag1 := macros_map["RKAIQ_HAVE_CAC_V10"]
            flag2 := macros_map["RKAIQ_HAVE_CAC_V11"]
            flag3 := macros_map["RKAIQ_HAVE_CAC_V12"]
            if flag0 || flag1 || flag2 || flag3 {
                srcs = append(srcs, "acac/cac_adaptor.cpp")
                srcs = append(srcs, "acac/rk_aiq_algo_acac_itf.cpp")
                srcs = append(srcs, "acac/rk_aiq_uapi_acac_int.cpp")
            }
            break
        }

    case "librkaiq_aynr_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YNR_V1"]
            if flag0 {
                srcs = append(srcs, "aynr/rk_aiq_aynr_algo_itf_v1.cpp")
                srcs = append(srcs, "aynr/rk_aiq_aynr_algo_ynr_v1.cpp")
                srcs = append(srcs, "aynr/rk_aiq_aynr_algo_v1.cpp")
                srcs = append(srcs, "aynr/rk_aiq_uapi_aynr_int_v1.cpp")
            }
            break
        }

    case "librkaiq_aynr_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YNR_V2"]
            if flag0 {
                srcs = append(srcs, "aynr2/rk_aiq_aynr_algo_itf_v2.cpp")
                srcs = append(srcs, "aynr2/rk_aiq_aynr_algo_ynr_v2.cpp")
                srcs = append(srcs, "aynr2/rk_aiq_aynr_algo_v2.cpp")
                srcs = append(srcs, "aynr2/rk_aiq_uapi_aynr_int_v2.cpp")
            }
            break
        }

    case "librkaiq_aynr_v3":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YNR_V3"]
            if flag0 {
                srcs = append(srcs, "aynr3/rk_aiq_aynr_algo_itf_v3.cpp")
                srcs = append(srcs, "aynr3/rk_aiq_aynr_algo_ynr_v3.cpp")
                srcs = append(srcs, "aynr3/rk_aiq_aynr_algo_v3.cpp")
                srcs = append(srcs, "aynr3/rk_aiq_uapi_aynr_int_v3.cpp")
            }
            break
        }

    case "librkaiq_aynr_v22":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YNR_V22"]
            if flag0 {
                srcs = append(srcs, "aynrV22/rk_aiq_aynr_algo_itf_v22.cpp")
                srcs = append(srcs, "aynrV22/rk_aiq_aynr_algo_ynr_v22.cpp")
                srcs = append(srcs, "aynrV22/rk_aiq_aynr_algo_v22.cpp")
                srcs = append(srcs, "aynrV22/rk_aiq_uapi_aynr_int_v22.cpp")
            }
            break
        }

    case "librkaiq_aynr_v24":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YNR_V24"]
            if flag0 {
                srcs = append(srcs, "aynrV24/rk_aiq_aynr_algo_itf_v24.cpp")
                srcs = append(srcs, "aynrV24/rk_aiq_aynr_algo_ynr_v24.cpp")
                srcs = append(srcs, "aynrV24/rk_aiq_aynr_algo_v24.cpp")
                srcs = append(srcs, "aynrV24/rk_aiq_uapi_aynr_int_v24.cpp")
            }
            break
        }

    case "librkaiq_auvnr_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_UVNR_V1"]
            if flag0 {
                srcs = append(srcs, "auvnr/rk_aiq_auvnr_algo_itf_v1.cpp")
                srcs = append(srcs, "auvnr/rk_aiq_auvnr_algo_uvnr_v1.cpp")
                srcs = append(srcs, "auvnr/rk_aiq_auvnr_algo_v1.cpp")
                srcs = append(srcs, "auvnr/rk_aiq_uapi_auvnr_int_v1.cpp")
            }
            break
        }

    case "librkaiq_acnr_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CNR_V1"]
            if flag0 {
                srcs = append(srcs, "acnr/rk_aiq_acnr_algo_itf_v1.cpp")
                srcs = append(srcs, "acnr/rk_aiq_acnr_algo_cnr_v1.cpp")
                srcs = append(srcs, "acnr/rk_aiq_acnr_algo_v1.cpp")
                srcs = append(srcs, "acnr/rk_aiq_uapi_acnr_int_v1.cpp")
            }
            break
        }

    case "librkaiq_acnr_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CNR_V2"]
            if flag0 {
                srcs = append(srcs, "acnr2/rk_aiq_acnr_algo_itf_v2.cpp")
                srcs = append(srcs, "acnr2/rk_aiq_acnr_algo_cnr_v2.cpp")
                srcs = append(srcs, "acnr2/rk_aiq_acnr_algo_v2.cpp")
                srcs = append(srcs, "acnr2/rk_aiq_uapi_acnr_int_v2.cpp")
            }
            break
        }

    case "librkaiq_acnr_v30":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CNR_V30"]
            flag1 := macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
            if flag0 || flag1 {
                srcs = append(srcs, "acnrV30/rk_aiq_acnr_algo_itf_v30.cpp")
                srcs = append(srcs, "acnrV30/rk_aiq_acnr_algo_cnr_v30.cpp")
                srcs = append(srcs, "acnrV30/rk_aiq_acnr_algo_v30.cpp")
                srcs = append(srcs, "acnrV30/rk_aiq_uapi_acnr_int_v30.cpp")
            }
            break
        }

    case "librkaiq_acnr_v31":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CNR_V31"]
            if flag0 {
                srcs = append(srcs, "acnrV31/rk_aiq_acnr_algo_itf_v31.cpp")
                srcs = append(srcs, "acnrV31/rk_aiq_acnr_algo_cnr_v31.cpp")
                srcs = append(srcs, "acnrV31/rk_aiq_acnr_algo_v31.cpp")
                srcs = append(srcs, "acnrV31/rk_aiq_uapi_acnr_int_v31.cpp")
            }
            break
        }

    case "librkaiq_asharp":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_SHARP_V1"]
            if flag0 {
                srcs = append(srcs, "asharp/rk_aiq_algo_asharp_itf.cpp")
                srcs = append(srcs, "asharp/rk_aiq_asharp_algo.cpp")
                srcs = append(srcs, "asharp/rk_aiq_asharp_algo_edgefilter.cpp")
                srcs = append(srcs, "asharp/rk_aiq_asharp_algo_sharp.cpp")
                srcs = append(srcs, "asharp/rk_aiq_uapi_asharp_int.cpp")
            }
            break
        }

    case "librkaiq_asharp_v3":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_SHARP_V3"]
            if flag0 {
                srcs = append(srcs, "asharp3/rk_aiq_asharp_algo_itf_v3.cpp")
                srcs = append(srcs, "asharp3/rk_aiq_asharp_algo_sharp_v3.cpp")
                srcs = append(srcs, "asharp3/rk_aiq_asharp_algo_v3.cpp")
                srcs = append(srcs, "asharp3/rk_aiq_uapi_asharp_int_v3.cpp")
            }
            break
        }

    case "librkaiq_asharp_v4":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_SHARP_V4"]
            if flag0 {
                srcs = append(srcs, "asharp4/rk_aiq_asharp_algo_itf_v4.cpp")
                srcs = append(srcs, "asharp4/rk_aiq_asharp_algo_sharp_v4.cpp")
                srcs = append(srcs, "asharp4/rk_aiq_asharp_algo_v4.cpp")
                srcs = append(srcs, "asharp4/rk_aiq_uapi_asharp_int_v4.cpp")
            }
            break
        }

    case "librkaiq_asharp_v33":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_SHARP_V33"]
            flag1 := macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
            if flag0 || flag1 {
                srcs = append(srcs, "asharpV33/rk_aiq_asharp_algo_itf_v33.cpp")
                srcs = append(srcs, "asharpV33/rk_aiq_asharp_algo_sharp_v33.cpp")
                srcs = append(srcs, "asharpV33/rk_aiq_asharp_algo_v33.cpp")
                srcs = append(srcs, "asharpV33/rk_aiq_uapi_asharp_int_v33.cpp")
            }
            break
        }

    case "librkaiq_asharp_v34":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_SHARP_V34"]
            if flag0 {
                srcs = append(srcs, "asharpV34/rk_aiq_asharp_algo_itf_v34.cpp")
                srcs = append(srcs, "asharpV34/rk_aiq_asharp_algo_sharp_v34.cpp")
                srcs = append(srcs, "asharpV34/rk_aiq_asharp_algo_v34.cpp")
                srcs = append(srcs, "asharpV34/rk_aiq_uapi_asharp_int_v34.cpp")
            }
            break
        }

    case "librkaiq_amfnr_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_MFNR_V1"]
            if flag0 {
                srcs = append(srcs, "amfnr/rk_aiq_amfnr_algo_itf_v1.cpp")
                srcs = append(srcs, "amfnr/rk_aiq_amfnr_algo_mfnr_v1.cpp")
                srcs = append(srcs, "amfnr/rk_aiq_amfnr_algo_v1.cpp")
                srcs = append(srcs, "amfnr/rk_aiq_uapi_amfnr_int_v1.cpp")
            }
            break
        }

    case "librkaiq_arawnr_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYERNR_V1"]
            if flag0 {
                srcs = append(srcs, "arawnr/rk_aiq_abayernr_algo_itf_v1.cpp")
                srcs = append(srcs, "arawnr/rk_aiq_abayernr_algo_bayernr_v1.cpp")
                srcs = append(srcs, "arawnr/rk_aiq_abayernr_algo_v1.cpp")
                srcs = append(srcs, "arawnr/rk_aiq_uapi_abayernr_int_v1.cpp")
            }
            break
        }

    case "librkaiq_arawnr_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYERNR_V2"]
            if flag0 {
                srcs = append(srcs, "arawnr2/rk_aiq_abayernr_algo_itf_v2.cpp")
                srcs = append(srcs, "arawnr2/rk_aiq_abayernr_algo_bayernr_v2.cpp")
                srcs = append(srcs, "arawnr2/rk_aiq_abayernr_algo_v2.cpp")
                srcs = append(srcs, "arawnr2/rk_aiq_uapi_abayernr_int_v2.cpp")
            }
            break
        }

    case "librkaiq_abayer2dnr_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
            if flag0 {
                srcs = append(srcs, "abayer2dnr2/rk_aiq_abayer2dnr_algo_itf_v2.cpp")
                srcs = append(srcs, "abayer2dnr2/rk_aiq_abayer2dnr_algo_bayernr_v2.cpp")
                srcs = append(srcs, "abayer2dnr2/rk_aiq_abayer2dnr_algo_v2.cpp")
                srcs = append(srcs, "abayer2dnr2/rk_aiq_uapi_abayer2dnr_int_v2.cpp")
            }
            break
        }

    case "librkaiq_abayer2dnr_v23":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
            if flag0 {
                srcs = append(srcs, "abayer2dnrV23/rk_aiq_abayer2dnr_algo_itf_v23.cpp")
                srcs = append(srcs, "abayer2dnrV23/rk_aiq_abayer2dnr_algo_bayernr_v23.cpp")
                srcs = append(srcs, "abayer2dnrV23/rk_aiq_abayer2dnr_algo_v23.cpp")
                srcs = append(srcs, "abayer2dnrV23/rk_aiq_uapi_abayer2dnr_int_v23.cpp")
            }
            break
        }

    case "librkaiq_abayertnr_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
            if flag0 {
                srcs = append(srcs, "abayertnr2/rk_aiq_abayertnr_algo_itf_v2.cpp")
                srcs = append(srcs, "abayertnr2/rk_aiq_abayertnr_algo_bayertnr_v2.cpp")
                srcs = append(srcs, "abayertnr2/rk_aiq_abayertnr_algo_v2.cpp")
                srcs = append(srcs, "abayertnr2/rk_aiq_uapi_abayertnr_int_v2.cpp")
            }
            break
        }

    case "librkaiq_abayertnr_v23":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
            flag1 := macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
            if flag0 || flag1 {
                srcs = append(srcs, "abayertnrV23/rk_aiq_abayertnr_algo_itf_v23.cpp")
                srcs = append(srcs, "abayertnrV23/rk_aiq_abayertnr_algo_bayertnr_v23.cpp")
                srcs = append(srcs, "abayertnrV23/rk_aiq_abayertnr_algo_v23.cpp")
                srcs = append(srcs, "abayertnrV23/rk_aiq_uapi_abayertnr_int_v23.cpp")
            }
            break
        }

    case "librkaiq_abayertnr_v30":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
            if flag0 {
                srcs = append(srcs, "abayertnrV30/rk_aiq_abayertnr_algo_itf_v30.cpp")
                srcs = append(srcs, "abayertnrV30/rk_aiq_abayertnr_algo_bayertnr_v30.cpp")
                srcs = append(srcs, "abayertnrV30/rk_aiq_abayertnr_algo_v30.cpp")
                srcs = append(srcs, "abayertnrV30/rk_aiq_uapi_abayertnr_int_v30.cpp")
            }
            break
        }

    case "librkaiq_again_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_GAIN_V1"]
            if flag0 {
                srcs = append(srcs, "again/rk_aiq_again_algo_itf.cpp")
                srcs = append(srcs, "again/rk_aiq_again_algo_gain.cpp")
                srcs = append(srcs, "again/rk_aiq_again_algo.cpp")
                srcs = append(srcs, "again/rk_aiq_uapi_again_int.cpp")
            }
            break
        }

    case "librkaiq_again_v2":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_GAIN_V2"]
            if flag0 {
                srcs = append(srcs, "again2/rk_aiq_again_algo_itf_v2.cpp")
                srcs = append(srcs, "again2/rk_aiq_again_algo_gain_v2.cpp")
                srcs = append(srcs, "again2/rk_aiq_again_algo_v2.cpp")
                srcs = append(srcs, "again2/rk_aiq_uapi_again_int_v2.cpp")
            }
            break
        }

    case "librkaiq_ablc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BLC_V1"]
            if flag0 {
                srcs = append(srcs, "ablc/rk_aiq_algo_ablc_itf.cpp")
                srcs = append(srcs, "ablc/rk_aiq_ablc_algo.cpp")
                srcs = append(srcs, "ablc/rk_aiq_uapi_ablc_int.cpp")
            }
            break
        }

    case "librkaiq_ablc_v32":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_BLC_V32"]
            if flag0 {
                srcs = append(srcs, "ablcV32/rk_aiq_ablc_algo_itf_v32.cpp")
                srcs = append(srcs, "ablcV32/rk_aiq_ablc_algo_v32.cpp")
                srcs = append(srcs, "ablcV32/rk_aiq_uapi_ablc_int_v32.cpp")
            }
            break
        }

    case "librkaiq_anr":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_ANR_V1"]
            if flag0 {
                srcs = append(srcs, "anr/rk_aiq_algo_anr_itf.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo_bayernr.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo_mfnr.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo_uvnr.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo_ynr.cpp")
                srcs = append(srcs, "anr/rk_aiq_uapi_anr_int.cpp")
                srcs = append(srcs, "anr/rk_aiq_anr_algo_gain.cpp")
            }
            break
        }

    case "librkaiq_amd":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AMD_V1"]
            if flag0 {
                srcs = append(srcs, "amd/rk_aiq_algo_amd_itf.cpp")
            }
            break
        }

    case "librkaiq_a3dlut":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_3DLUT_V1"]
            if flag0 {
                srcs = append(srcs, "a3dlut/rk_aiq_algo_a3dlut_itf.cpp")
                srcs = append(srcs, "a3dlut/rk_aiq_uapi_a3dlut_int.cpp")
                srcs = append(srcs, "a3dlut/rk_aiq_a3dlut_algo.cpp")
            }
            break
        }

    case "librkaiq_accm":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CCM_V1"]
            flag1 := macros_map["RKAIQ_HAVE_CCM_V2"]
            flag2 := macros_map["RKAIQ_HAVE_CCM_V3"]
            if flag0 || flag1 || flag2 {
                srcs = append(srcs, "accm/rk_aiq_algo_accm_itf.cpp")
                srcs = append(srcs, "accm/rk_aiq_accm_algo_com.cpp")
                srcs = append(srcs, "accm/rk_aiq_uapi_accm_int.cpp")
                if flag0 {
                    srcs = append(srcs, "accm/rk_aiq_accm_algo_v1.cpp")
                }
                if flag1 || flag2 {
                    srcs = append(srcs, "accm/rk_aiq_accm_algo_v2.cpp")
                }
            }
            break
        }

    case "librkaiq_agamma":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_GAMMA_V10"]
            flag1 := macros_map["RKAIQ_HAVE_GAMMA_V11"]
            if flag0 || flag1 {
                srcs = append(srcs, "agamma/rk_aiq_algo_agamma_itf.cpp")
                srcs = append(srcs, "agamma/rk_aiq_uapi_agamma_int.cpp")
                if flag0 {
                    srcs = append(srcs, "agamma/rk_aiq_agamma_algo_v10.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "agamma/rk_aiq_agamma_algo_v11.cpp")
                }
            }
            break
        }

    case "librkaiq_amerge":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_MERGE_V10"]
            flag1 := macros_map["RKAIQ_HAVE_MERGE_V11"]
            flag2 := macros_map["RKAIQ_HAVE_MERGE_V12"]
            if flag0 || flag1 || flag2 {
                srcs = append(srcs, "amerge/rk_aiq_algo_amerge_itf.cpp")
                srcs = append(srcs, "amerge/rk_aiq_uapi_amerge_int.cpp")
                if flag0 {
                    srcs = append(srcs, "amerge/rk_aiq_amerge_algo_v10.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "amerge/rk_aiq_amerge_algo_v11.cpp")
                }
                if flag2 {
                    srcs = append(srcs, "amerge/rk_aiq_amerge_algo_v12.cpp")
                }
            }
            break
        }

    case "librkaiq_adrc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DRC_V10"]
            flag1 := macros_map["RKAIQ_HAVE_DRC_V11"]
            flag2 := macros_map["RKAIQ_HAVE_DRC_V12"]
            flag3 := macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
            flag4 := macros_map["RKAIQ_HAVE_DRC_V20"]
            if flag0 || flag1 || flag2 || flag3 || flag4 {
                srcs = append(srcs, "adrc/rk_aiq_algo_adrc_itf.cpp")
                srcs = append(srcs, "adrc/rk_aiq_uapi_adrc_int.cpp")
                if flag0 {
                    srcs = append(srcs, "adrc/rk_aiq_adrc_algo_v10.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "adrc/rk_aiq_adrc_algo_v11.cpp")
                }
                if flag2 || flag3 {
                    srcs = append(srcs, "adrc/rk_aiq_adrc_algo_v12.cpp")
                }
                if flag4 {
                    srcs = append(srcs, "adrc/rk_aiq_adrc_algo_v20.cpp")
                }
            }
            break
        }

    case "librkaiq_adehaze":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DEHAZE_V10"]
            flag1 := macros_map["RKAIQ_HAVE_DEHAZE_V11"]
            flag2 := macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
            flag3 := macros_map["RKAIQ_HAVE_DEHAZE_V12"]
            flag4 := macros_map["RKAIQ_HAVE_DEHAZE_V14"]
            if flag0 || flag1 || flag2 || flag3 || flag4 {
                srcs = append(srcs, "adehaze/rk_aiq_algo_adehaze_itf.cpp")
                srcs = append(srcs, "adehaze/rk_aiq_uapi_adehaze_int.cpp")
                if flag0 {
                    srcs = append(srcs, "adehaze/rk_aiq_adehaze_algo_v10.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "adehaze/rk_aiq_adehaze_algo_v11.cpp")
                }
                if flag2 {
                    srcs = append(srcs, "adehaze/rk_aiq_adehaze_algo_v11_duo.cpp")
                }
                if flag3 {
                    srcs = append(srcs, "adehaze/rk_aiq_adehaze_algo_v12.cpp")
                }
                if flag4 {
                    srcs = append(srcs, "adehaze/rk_aiq_adehaze_algo_v14.cpp")
                }
            }
            break
        }

    case "librkaiq_argbir":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
            if flag0 {
                srcs = append(srcs, "argbir/rk_aiq_algo_argbir_itf.cpp")
                srcs = append(srcs, "argbir/rk_aiq_uapi_argbir_int.cpp")
                if flag0 {
                    srcs = append(srcs, "argbir/rk_aiq_argbir_algo_v10.cpp")
                }
            }
            break
        }

    case "librkaiq_adegamma":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DEGAMMA_V1"]
            if flag0 {
                srcs = append(srcs, "adegamma/rk_aiq_algo_adegamma_itf.cpp")
                srcs = append(srcs, "adegamma/rk_aiq_adegamma_algo.cpp")
                srcs = append(srcs, "adegamma/rk_aiq_uapi_adegamma_int.cpp")
            }
            break
        }

    case "librkaiq_atmo":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_TMO_V1"]
            if flag0 {
                srcs = append(srcs, "atmo/rk_aiq_algo_atmo_itf.cpp")
                srcs = append(srcs, "atmo/rk_aiq_atmo_algo.cpp")
                srcs = append(srcs, "atmo/rk_aiq_uapi_atmo_int.cpp")
            }
            break
        }

    case "librkaiq_awdr":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_WDR_V1"]
            if flag0 {
                srcs = append(srcs, "awdr/rk_aiq_algo_awdr_itf.cpp")
            }
            break
        }

    case "librkaiq_aldch":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_LDCH_V10"]
            flag1 := macros_map["RKAIQ_HAVE_LDCH_V21"]
            if flag0 || flag1 {
                if flag0 {
                    srcs = append(srcs, "aldch/rk_aiq_uapi_aldch_int.cpp")
                    srcs = append(srcs, "aldch/rk_aiq_algo_aldch_itf.cpp")
                    srcs = append(srcs, "aldch/rk_aiq_ldch_generate_mesh.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "aldch/rk_aiq_uapi_aldch_v21_int.cpp")
                    srcs = append(srcs, "aldch/rk_aiq_algo_aldch_v21_itf.cpp")
                    srcs = append(srcs, "aldch/rk_aiq_ldch_generate_mesh.cpp")
                }
            }
            break
        }

    case "librkaiq_afec":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_FEC_V10"]
            if flag0 {
                srcs = append(srcs, "afec/rk_aiq_algo_afec_itf.cpp")
                srcs = append(srcs, "afec/rk_aiq_uapi_afec_int.cpp")
            }
            break
        }

    case "librkaiq_acsm":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CSM_V1"]
            if flag0 {
                srcs = append(srcs, "acsm/rk_aiq_algo_acsm_itf.cpp")
                srcs = append(srcs, "acsm/rk_aiq_uapi_acsm_int.cpp")
            }
            break
        }

    case "librkaiq_acgc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CGC_V1"]
            if flag0 {
                srcs = append(srcs, "acgc/rk_aiq_algo_acgc_itf.cpp")
                srcs = append(srcs, "acgc/rk_aiq_uapi_acgc_int.cpp")
            }
            break
        }

    case "librkaiq_adebayer":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DEBAYER_V1"]
            flag1 := macros_map["RKAIQ_HAVE_DEBAYER_V2"]
            flag2 := macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
            flag3 := macros_map["RKAIQ_HAVE_DEBAYER_V3"]
            flag4 := macros_map["RKAIQ_HAVE_DEBAYER_V4"]
            if flag0 || flag1 || flag2 || flag3 || flag4 {
                srcs = append(srcs, "adebayer/rk_aiq_algo_adebayer_itf.cpp")
                srcs = append(srcs, "adebayer/rk_aiq_uapi_adebayer_int.cpp")
                if flag0 {
                    srcs = append(srcs, "adebayer/rk_aiq_adebayer_algo_v1.cpp")
                }
                if flag1 || flag2 {
                    srcs = append(srcs, "adebayer/rk_aiq_adebayer_algo_v2.cpp")
                }
                if flag3 {
                    srcs = append(srcs, "adebayer/rk_aiq_adebayer_algo_v3.cpp")
                }
            }
            break
        }

    case "librkaiq_alsc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_LSC_V1"]
            flag1 := macros_map["RKAIQ_HAVE_LSC_V2"]
            flag2 := macros_map["RKAIQ_HAVE_LSC_V3"]
            if flag0 || flag1 || flag2 {
                srcs = append(srcs, "alsc/rk_aiq_algo_alsc_itf.cpp")
                srcs = append(srcs, "alsc/rk_aiq_alsc_algo.cpp")
                srcs = append(srcs, "alsc/rk_aiq_uapi_alsc_int.cpp")
                srcs = append(srcs, "alsc/rk_aiq_alsc_convert_otp.cpp")
            }
            break
        }

    case "librkaiq_adpcc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DPCC_V1"]
            flag1 := macros_map["RKAIQ_HAVE_DPCC_V2"]
            if flag0 || flag1 {
                srcs = append(srcs, "adpcc/rk_aiq_algo_adpcc_itf.cpp")
                srcs = append(srcs, "adpcc/rk_aiq_adpcc_algo.cpp")
                srcs = append(srcs, "adpcc/rk_aiq_uapi_adpcc_int.cpp")
            }
            break
        }

    case "librkaiq_aie":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AIE_V10"]
            if flag0 {
                srcs = append(srcs, "aie/rk_aiq_algo_aie_itf.cpp")
                srcs = append(srcs, "aie/rk_aiq_algo_aie_int.cpp")
            }
            break
        }

    case "librkaiq_acp":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_ACP_V10"]
            if flag0 {
                srcs = append(srcs, "acp/rk_aiq_algo_acp_itf.cpp")
                srcs = append(srcs, "acp/rk_aiq_uapi_acp_int.cpp")
            }
            break
        }

    case "librkaiq_afd":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AFD_V1"]
            flag1 := macros_map["RKAIQ_HAVE_AFD_V2"]
            if flag0 || flag1 {
                srcs = append(srcs, "afd/rk_aiq_algo_afd_itf.cpp")
                srcs = append(srcs, "afd/rk_aiq_afd_algo.cpp")
                srcs = append(srcs, "afd/rk_aiq_uapi_afd_int.cpp")
            }
            break
        }

    case "librkaiq_ayuvme_v1":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_YUVME_V1"]
            if flag0 {
                srcs = append(srcs, "ayuvmeV1/rk_aiq_ayuvme_algo_itf_v1.cpp")
                srcs = append(srcs, "ayuvmeV1/rk_aiq_ayuvme_algo_yuvme_v1.cpp")
                srcs = append(srcs, "ayuvmeV1/rk_aiq_ayuvme_algo_v1.cpp")
                srcs = append(srcs, "ayuvmeV1/rk_aiq_uapi_ayuvme_int_v1.cpp")
            }
            break
        }

    case "librkaiq_camgroup_misc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_ENABLE_CAMGROUP"]
            if flag0 {
                srcs = rkaiq_getCamgroupStaticLibSrcs(macros_map);
            }
        }
        break
    default:
        break
    }

    // fmt.Printf("%s srcs:", name, srcs)
    // fmt.Printf("\n")
    return srcs
}

func rkaiq_getCamgroupStaticLibSrcs(macros_map map[string]bool) []string {
    var flag0 bool = false;
    var flag1 bool = false;
    var flag2 bool = false;
    var flag3 bool = false;
    var flag4 bool = false;
    var srcs []string

    useNewstruct := macros_map["RKAIQ_NEWSTRUCT_TEST"]
    if useNewstruct {
        srcs = append(srcs, "newStruct/blc/rk_aiq_algo_camgroup_blc.c")
        srcs = append(srcs, "newStruct/3dlut/rk_aiq_algo_camgroup_3dlut.c")
        srcs = append(srcs, "newStruct/gamma/rk_aiq_algo_camgroup_gamma.c")
        srcs = append(srcs, "newStruct/drc/rk_aiq_algo_camgroup_drc.c")
        srcs = append(srcs, "newStruct/dpc/rk_aiq_algo_camgroup_dpc.c")
        srcs = append(srcs, "newStruct/merge/rk_aiq_algo_camgroup_merge.c")
        srcs = append(srcs, "newStruct/ccm/rk_aiq_algo_camgroup_ccm.c")
        srcs = append(srcs, "newStruct/lsc/rk_aiq_algo_camgroup_lsc.c")
        srcs = append(srcs, "newStruct/histeq/rk_aiq_algo_camgroup_histeq.c")
        flag1 = macros_map["RKAIQ_HAVE_DEHAZE"]
        if (flag1) {
            srcs = append(srcs, "newStruct/dehaze/rk_aiq_algo_camgroup_dehaze.c")
        }
        flag1 = macros_map["RKAIQ_HAVE_ENHANCE"]
        if (flag1) {
            srcs = append(srcs, "newStruct/enh/rk_aiq_algo_camgroup_enh.c")
        }
    } else {
        // misc
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_alsc_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_adpcc_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_accm_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_a3dlut_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_amerge_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_ablc_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_ablc_itf_v32.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_agamma_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_adrc_itf.cpp")
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_adehaze_itf.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if flag0 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/gain/rk_aiq_algo_camgroup_gain.c")
        } else {
            srcs = append(srcs, "again2/rk_aiq_algo_camgroup_again_itf_v2.cpp")
            srcs = append(srcs, "again2/rk_aiq_uapi_camgroup_again_int_v2.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
    flag1 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
    if flag0 {
        srcs = append(srcs, "abayer2dnr2/rk_aiq_uapi_camgroup_abayer2dnr_int_v2.cpp")
        srcs = append(srcs, "abayer2dnr2/rk_aiq_algo_camgroup_abayer2dnr_itf_v2.cpp")
    } else if flag1 {
        srcs = append(srcs, "abayer2dnrV23/rk_aiq_uapi_camgroup_abayer2dnr_int_v23.cpp")
        srcs = append(srcs, "abayer2dnrV23/rk_aiq_algo_camgroup_abayer2dnr_itf_v23.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V2"]
    flag1 = macros_map["RKAIQ_HAVE_YNR_V3"]
    flag2 = macros_map["RKAIQ_HAVE_YNR_V22"]
    flag3 = macros_map["RKAIQ_HAVE_YNR_V24"]
    if flag0 {
        srcs = append(srcs, "aynr2/rk_aiq_algo_camgroup_aynr_itf_v2.cpp")
    } else if flag1 {
        srcs = append(srcs, "aynr3/rk_aiq_algo_camgroup_aynr_itf_v3.cpp")
        srcs = append(srcs, "aynr3/rk_aiq_uapi_camgroup_aynr_int_v3.cpp")
    } else if flag2 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/ynr/rk_aiq_algo_camgroup_ynr.c")
        } else {
            srcs = append(srcs, "aynrV22/rk_aiq_algo_camgroup_aynr_itf_v22.cpp")
            srcs = append(srcs, "aynrV22/rk_aiq_uapi_camgroup_aynr_int_v22.cpp")
        }
    } else if flag3 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/ynr/rk_aiq_algo_camgroup_ynr.c")
        } else {
            srcs = append(srcs, "aynrV24/rk_aiq_algo_camgroup_aynr_itf_v24.cpp")
            srcs = append(srcs, "aynrV24/rk_aiq_uapi_camgroup_aynr_int_v24.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    flag2 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    flag3 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if flag0 {
        srcs = append(srcs, "abayertnr2/rk_aiq_algo_camgroup_abayertnr_itf_v2.cpp")
        srcs = append(srcs, "abayertnr2/rk_aiq_uapi_camgroup_abayertnr_int_v2.cpp")
    } else if flag1 || flag2 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.c")
        } else {
            srcs = append(srcs, "abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.cpp")
            srcs = append(srcs, "abayertnrV23/rk_aiq_algo_camgroup_abayertnr_itf_v23.cpp")
        }
    } else if flag3 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.c")
        } else {
            srcs = append(srcs, "abayertnrV30/rk_aiq_uapi_camgroup_abayertnr_int_v30.cpp")
            srcs = append(srcs, "abayertnrV30/rk_aiq_algo_camgroup_abayertnr_itf_v30.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if flag0 || flag1 {
        srcs = append(srcs, "abayernr2/rk_aiq_algo_camgroup_abayernr_itf_v2.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if flag0 {
        if useNewstruct {
            srcs = append(srcs, "newStruct/rgbir/rk_aiq_algo_camgroup_rgbir.c")
        } else {
            srcs = append(srcs, "misc/rk_aiq_algo_camgroup_argbir_itf.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    flag2 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    flag3 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    flag4 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if ((flag0 || flag1 || flag4) && useNewstruct) {
        srcs = append(srcs, "newStruct/sharp/rk_aiq_algo_camgroup_sharp.c")
    } else if flag0 {
        srcs = append(srcs, "asharpV34/rk_aiq_uapi_camgroup_asharp_int_v34.cpp")
        srcs = append(srcs, "asharpV34/rk_aiq_algo_camgroup_asharp_itf_v34.cpp")
    } else if flag1 || flag4 {
        srcs = append(srcs, "asharpV33/rk_aiq_uapi_camgroup_asharp_int_v33.cpp")
        srcs = append(srcs, "asharpV33/rk_aiq_algo_camgroup_asharp_itf_v33.cpp")
    } else if flag2 {
        srcs = append(srcs, "asharp3/rk_aiq_algo_camgroup_asharp_itf_v3.cpp")
    } else if flag3 {
        srcs = append(srcs, "asharp4/rk_aiq_algo_camgroup_asharp_itf_v4.cpp")
        srcs = append(srcs, "asharp4/rk_aiq_uapi_camgroup_asharp_int_v4.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V31"]
    flag1 = macros_map["RKAIQ_HAVE_CNR_V30"]
    flag2 = macros_map["RKAIQ_HAVE_CNR_V1"]
    flag3 = macros_map["RKAIQ_HAVE_CNR_V2"]
    flag4 = macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
    if ((flag0 || flag1 || flag4) && useNewstruct) {
        srcs = append(srcs, "newStruct/cnr/rk_aiq_algo_camgroup_cnr.c")
    } else if flag0 {
        srcs = append(srcs, "acnrV31/rk_aiq_algo_camgroup_acnr_itf_v31.cpp")
        srcs = append(srcs, "acnrV31/rk_aiq_uapi_camgroup_acnr_int_v31.cpp")
    } else if flag1 || flag4 {
        srcs = append(srcs, "acnrV30/rk_aiq_algo_camgroup_acnr_itf_v30.cpp")
        srcs = append(srcs, "acnrV30/rk_aiq_uapi_camgroup_acnr_int_v30.cpp")
    } else if flag2 {
        srcs = append(srcs, "acnr/rk_aiq_algo_camgroup_acnr_itf.cpp")
    } else if flag3 {
        srcs = append(srcs, "acnr2/rk_aiq_algo_camgroup_acnr_itf_v2.cpp")
        srcs = append(srcs, "acnr2/rk_aiq_uapi_camgroup_acnr_int_v2.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YUVME_V1"]
    if flag0 {
        if useNewstruct {

        } else {
            srcs = append(srcs, "ayuvmeV1/rk_aiq_algo_camgroup_ayuvme_itf_v1.cpp")
            srcs = append(srcs, "ayuvmeV1/rk_aiq_uapi_camgroup_ayuvme_int_v1.cpp")
        }
    }

    // fmt.Printf("libcamgroup_misc srcs:", srcs)
    // fmt.Printf("\n")
    return srcs;
}

func rkaiq_getNewStructStaticLibSrcs(name string, macros_map map[string]bool) []string {

    var srcs []string

    dumpsys := macros_map["RKAIQ_HAVE_DUMPSYS"]

    switch name {
    case "librkaiq_3dlut":
        flag0 := macros_map["RKAIQ_HAVE_3DLUT_V1"]
        if flag0 {
            srcs = append(srcs, "3dlut/algo_3dlut.c")
        }
        if (len(srcs) != 0 && dumpsys) {
            srcs = append(srcs, "3dlut/algo_3dlut_info.c")
        }
        break

    case "librkaiq_bayertnr":
        flag0 := macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
        flag1 := macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
        flag2 := macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
        if flag0 || flag1 || flag2 {
            srcs = append(srcs, "bayertnr/algo_bayertnr.c")
        }
        break

    case "librkaiq_blc":
        flag0 := macros_map["RKAIQ_HAVE_BLC_V32"]
        if flag0 {
            srcs = append(srcs, "blc/algo_blc.c")
        }
        break

    case "librkaiq_cac":
        flag0 := macros_map["RKAIQ_HAVE_CAC_V03"]
        flag1 := macros_map["RKAIQ_HAVE_CAC_V10"]
        flag2 := macros_map["RKAIQ_HAVE_CAC_V11"]
        flag3 := macros_map["RKAIQ_HAVE_CAC_V30"]
        if flag0 || flag1 || flag2 || flag3{
            srcs = append(srcs, "cac/algo_cac.c")
        }
        break

    case "librkaiq_ccm":
        flag0 := macros_map["RKAIQ_HAVE_CCM_V1"]
        flag1 := macros_map["RKAIQ_HAVE_CCM_V2"]
        flag2 := macros_map["RKAIQ_HAVE_CCM_V3"]
        if flag0 || flag1 || flag2 {
            srcs = append(srcs, "ccm/algo_ccm.c")
        }
        if (len(srcs) != 0 && dumpsys) {
            srcs = append(srcs, "ccm/algo_ccm_info.c")
        }
        break

    case "librkaiq_cgc":
        flag0 := macros_map["RKAIQ_HAVE_CGC_V1"]
        if flag0 {
            srcs = append(srcs, "cgc/algo_cgc.c")
        }
        break

    case "librkaiq_cnr":
        flag0 := macros_map["RKAIQ_HAVE_CNR_V30"]
        flag1 := macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
        flag2 := macros_map["RKAIQ_HAVE_CNR_V31"]
        if flag0 || flag1 || flag2 {
            srcs = append(srcs, "cnr/algo_cnr.c")
        }
        break

    case "librkaiq_cp":
        flag0 := macros_map["RKAIQ_HAVE_ACP_V10"]
        if flag0 {
            srcs = append(srcs, "cp/algo_cp.c")
        }
        break

    case "librkaiq_csm":
        flag0 := macros_map["RKAIQ_HAVE_CSM_V1"]
        if flag0 {
            srcs = append(srcs, "csm/algo_csm.c")
        }
        break

    case "librkaiq_dehaze":
        flag0 := macros_map["RKAIQ_HAVE_DEHAZE_V10"]
        flag1 := macros_map["RKAIQ_HAVE_DEHAZE_V11"]
        flag2 := macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
        flag3 := macros_map["RKAIQ_HAVE_DEHAZE_V12"]
        flag4 := macros_map["RKAIQ_HAVE_DEHAZE_V14"]
        if flag0 || flag1 || flag2 || flag3 || flag4 {
            srcs = append(srcs, "dehaze/algo_dehaze.c")
        }
        break

    case "librkaiq_demosaic":
        flag0 := macros_map["RKAIQ_HAVE_DEBAYER_V1"]
        flag1 := macros_map["RKAIQ_HAVE_DEBAYER_V2"]
        flag2 := macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
        flag3 := macros_map["RKAIQ_HAVE_DEBAYER_V3"]
        flag4 := macros_map["RKAIQ_HAVE_DEBAYER_V4"]
        if flag0 || flag1 || flag2 || flag3 || flag4 {
            srcs = append(srcs, "demosaic/algo_demosaic.c")
        }
        break

    case "librkaiq_dpc":
        flag0 := macros_map["RKAIQ_HAVE_DPCC_V1"]
        flag1 := macros_map["RKAIQ_HAVE_DPCC_V2"]
        if flag0 || flag1 {
            srcs = append(srcs, "dpc/algo_dpc.c")
        }
        break

    case "librkaiq_drc":
        flag0 := macros_map["RKAIQ_HAVE_DRC_V10"]
        flag1 := macros_map["RKAIQ_HAVE_DRC_V11"]
        flag2 := macros_map["RKAIQ_HAVE_DRC_V12"]
        flag3 := macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
        flag4 := macros_map["RKAIQ_HAVE_DRC_V20"]
        flag5 := macros_map["RKAIQ_HAVE_DRC_V21"]
        if flag0 || flag1 || flag2 || flag3 || flag4 || flag5 {
            srcs = append(srcs, "drc/algo_drc.c")
        }
        break

    case "librkaiq_gain":
        flag0 := macros_map["RKAIQ_HAVE_GAIN_V2"]
        if flag0 {
            srcs = append(srcs, "gain/algo_gain.c")
        }
        break

    case "librkaiq_gamma":
        flag0 := macros_map["RKAIQ_HAVE_GAMMA_V10"]
        flag1 := macros_map["RKAIQ_HAVE_GAMMA_V11"]
        if flag0 || flag1 {
            srcs = append(srcs, "gamma/algo_gamma.c")
        }
        break

    case "librkaiq_gic":
        flag0 := macros_map["RKAIQ_HAVE_GIC_V2"]
        if flag0 {
            srcs = append(srcs, "gic/algo_gic.c")
        }
        break

    case "librkaiq_ie":
        flag0 := macros_map["RKAIQ_HAVE_AIE_V10"]
        if flag0 {
            srcs = append(srcs, "ie/algo_ie.c")
        }
        break

    case "librkaiq_ldch":
        flag0 := macros_map["RKAIQ_HAVE_LDCH_V10"]
        flag1 := macros_map["RKAIQ_HAVE_LDCH_V21"]
        if flag0 || flag1 {
            srcs = append(srcs, "ldch/algo_ldch.cpp")
            srcs = append(srcs, "ldch/ldch_generate_mesh.cpp")
        }
        break

    case "librkaiq_lsc":
        flag0 := macros_map["RKAIQ_HAVE_LSC_V1"]
        flag1 := macros_map["RKAIQ_HAVE_LSC_V2"]
        flag2 := macros_map["RKAIQ_HAVE_LSC_V3"]
        if flag0 || flag1 || flag2 {
            srcs = append(srcs, "lsc/algo_lsc.c")
        }
        if (len(srcs) != 0 && dumpsys) {
            srcs = append(srcs, "lsc/algo_lsc_info.c")
        }
        break

    case "librkaiq_merge":
        flag0 := macros_map["RKAIQ_HAVE_MERGE_V10"]
        flag1 := macros_map["RKAIQ_HAVE_MERGE_V11"]
        flag2 := macros_map["RKAIQ_HAVE_MERGE_V12"]
        flag3 := macros_map["RKAIQ_HAVE_MERGE_V13"]
        if flag0 || flag1 || flag2 || flag3 {
            srcs = append(srcs, "merge/algo_merge.c")
        }
        break

    case "librkaiq_rgbir":
        flag0 := macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
        if flag0 {
            srcs = append(srcs, "rgbir/algo_rgbir.c")
        }
        break

    case "librkaiq_sharp":
        flag0 := macros_map["RKAIQ_HAVE_SHARP_V33"]
        flag1 := macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
        flag2 := macros_map["RKAIQ_HAVE_SHARP_V34"]
        if flag0 || flag1 || flag2 {
            srcs = append(srcs, "sharp/algo_sharp.c")
        }
        break

    case "librkaiq_yme":
        flag0 := macros_map["RKAIQ_HAVE_YUVME_V1"]
        if flag0 {
            srcs = append(srcs, "yme/algo_yme.c")
        }
        break

    case "librkaiq_ynr":
        flag0 := macros_map["RKAIQ_HAVE_YNR_V22"]
        flag1 := macros_map["RKAIQ_HAVE_YNR_V24"]
        if flag0 || flag1 {
            srcs = append(srcs, "ynr/algo_ynr.c")
        }
        break

    case "librkaiq_histeq":
        flag0 := macros_map["RKAIQ_HAVE_DEHAZE_V14"]
        flag1 := macros_map["RKAIQ_HAVE_HISTEQ_V10"]
        if flag0 || flag1 {
            srcs = append(srcs, "histeq/algo_histeq.c")
        }
        break

    case "librkaiq_aldc":
        {
            flag0 := macros_map["RKAIQ_HAVE_LDCH_V22"]
            flag1 := macros_map["RKAIQ_HAVE_LDCV_V22"]
            if flag0 {
                srcs = append(srcs, "ldc/algo_ldc.c")
                srcs = append(srcs, "ldc/ldc_algo_adaptor.c")
                srcs = append(srcs, "ldc/ldc_gen_mesh.c")
                srcs = append(srcs, "ldc/ldc_ldch_adaptee.c")
                srcs = append(srcs, "ldc/ldc_lut_buffer.c")
                if flag1 {
                    srcs = append(srcs, "ldc/ldc_ldcv_adaptee.c")
                }
            }
            if (len(srcs) != 0 && dumpsys) {
                srcs = append(srcs, "ldc/algo_ldc_info.c")
            }
            break
        }

    case "librkaiq_camgroup_misc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_ENABLE_CAMGROUP"]
            if flag0 {
                srcs = rkaiq_getCamgroupStaticLibSrcs(macros_map);
            }
        }
        break

    default:
        // fmt.Printf("%s can't find srcs files\n", name)
        break
    }

    if (dumpsys) {
        switch name {
        case "librkaiq_dumpcam_server":
            {
                srcs = append(srcs, "rk_aiq_registry.c")
            }
            break
        case "librkaiq_ipcs":
            {
                srcs = append(srcs, "ipcs/c_rk_ipcs_request.c")
                srcs = append(srcs, "ipcs/c_rk_ipcs_response.c")
                srcs = append(srcs, "ipcs/c_rk_ipcs_service.c")
                srcs = append(srcs, "ipcs/sock/c_rk_ipcs_sock_server.c")
            }
            break

        case "librkaiq_info":
            {
                srcs = append(srcs, "info/rk_info_utils.c")
                srcs = append(srcs, "info/c_rk_info_service_control.c")
                srcs = append(srcs, "info/rk_cmd_service_control.c")
                srcs = append(srcs, "info/st_string.c")
            }
            break

        case "librkaiq_cjson":
            {
                srcs = append(srcs, "third-party/cjson/c_RTJsonParser.c")
            }
            break

        case "librkaiq_argparse":
            {
                srcs = append(srcs, "third-party/argparse/argparse.c")
            }
            break
        }
    }

    // fmt.Printf("%s srcs:", name, srcs)
    // fmt.Printf("\n")
    return srcs
}
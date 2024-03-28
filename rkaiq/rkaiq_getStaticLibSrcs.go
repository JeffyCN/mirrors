package rkaiqdefaults

import (
    // "fmt"
)

func rkaiq_getStaticLibSrcs(name string, macros_map map[string]bool) []string {
    var srcs []string
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
        }

    case "librkaiq_amd":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AMD_V1"]
            if flag0 {
                srcs = append(srcs, "amd/rk_aiq_algo_amd_itf.cpp")
            }
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
        }

    case "librkaiq_awdr":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_WDR_V1"]
            if flag0 {
                srcs = append(srcs, "awdr/rk_aiq_algo_awdr_itf.cpp")
            }
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
        }

    case "librkaiq_afec":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_FEC_V10"]
            if flag0 {
                srcs = append(srcs, "afec/rk_aiq_algo_afec_itf.cpp")
                srcs = append(srcs, "afec/rk_aiq_uapi_afec_int.cpp")
            }
        }

    case "librkaiq_acsm":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CSM_V1"]
            if flag0 {
                srcs = append(srcs, "acsm/rk_aiq_algo_acsm_itf.cpp")
                srcs = append(srcs, "acsm/rk_aiq_uapi_acsm_int.cpp")
            }
        }

    case "librkaiq_acgc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_CGC_V1"]
            if flag0 {
                srcs = append(srcs, "acgc/rk_aiq_algo_acgc_itf.cpp")
                srcs = append(srcs, "acgc/rk_aiq_uapi_acgc_int.cpp")
            }
        }

    case "librkaiq_adebayer":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_DEBAYER_V1"]
            flag1 := macros_map["RKAIQ_HAVE_DEBAYER_V2"]
            flag2 := macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
            flag3 := macros_map["RKAIQ_HAVE_DEBAYER_V3"]
            if flag0 || flag1 || flag2 || flag3 {
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
        }

    case "librkaiq_aie":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AIE_V10"]
            if flag0 {
                srcs = append(srcs, "aie/rk_aiq_algo_aie_itf.cpp")
                srcs = append(srcs, "aie/rk_aiq_algo_aie_int.cpp")
            }
        }

    case "librkaiq_acp":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_ACP_V10"]
            if flag0 {
                srcs = append(srcs, "acp/rk_aiq_algo_acp_itf.cpp")
                srcs = append(srcs, "acp/rk_aiq_uapi_acp_int.cpp")
            }
        }

    case "librkaiq_awb":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_HAVE_AWB_V20"]
            flag1 := macros_map["RKAIQ_HAVE_AWB_V21"]
            flag2 := macros_map["RKAIQ_HAVE_AWB_V32"]
            flag3 := macros_map["RKAIQ_HAVE_AWB_V32LT"]
            if flag0 || flag1 || flag2 || flag3 {
                srcs = append(srcs, "color_space_convert.cpp")
                srcs = append(srcs, "fixfloat.cpp")
                srcs = append(srcs, "rk_aiq_algo_awb_itf.cpp")
                srcs = append(srcs, "rk_aiq_awb_algo_com1.cpp")
                srcs = append(srcs, "rk_aiq_awb_algo_com2.cpp")
                srcs = append(srcs, "rk_aiq_awb_algo_sgc.cpp")
                srcs = append(srcs, "rk_aiq_uapi_awb_int.cpp")
                srcs = append(srcs, "rk_aiq_uapiv2_awb_int.cpp")
                if flag0 {
                    srcs = append(srcs, "rk_aiq_awb_algo_v200.cpp")
                }
                if flag1 {
                    srcs = append(srcs, "rk_aiq_awb_algo_v201.cpp")
                }
                if flag2 || flag3 {
                    srcs = append(srcs, "rk_aiq_awb_algo_v32.cpp")
                }
            }
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
        }

    case "librkaiq_camgroup_misc":
        {
            srcs = srcs[len(srcs):]
            flag0 := macros_map["RKAIQ_ENABLE_CAMGROUP"]
            if flag0 {
                srcs = rkaiq_getCamgroupStaticLibSrcs(macros_map);
            }
        }

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

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if flag0 {
        srcs = append(srcs, "again2/rk_aiq_algo_camgroup_again_itf_v2.cpp")
        srcs = append(srcs, "again2/rk_aiq_uapi_camgroup_again_int_v2.cpp")
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
        srcs = append(srcs, "aynrV22/rk_aiq_algo_camgroup_aynr_itf_v22.cpp")
        srcs = append(srcs, "aynrV22/rk_aiq_uapi_camgroup_aynr_int_v22.cpp")
    } else if flag3 {
        srcs = append(srcs, "aynrV24/rk_aiq_algo_camgroup_aynr_itf_v24.cpp")
        srcs = append(srcs, "aynrV24/rk_aiq_uapi_camgroup_aynr_int_v24.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    flag2 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    flag3 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if flag0 {
        srcs = append(srcs, "abayertnr2/rk_aiq_algo_camgroup_abayertnr_itf_v2.cpp")
        srcs = append(srcs, "abayertnr2/rk_aiq_uapi_camgroup_abayertnr_int_v2.cpp")
    } else if flag1 || flag2 {
        srcs = append(srcs, "abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.cpp")
        srcs = append(srcs, "abayertnrV23/rk_aiq_algo_camgroup_abayertnr_itf_v23.cpp")
    } else if flag3 {
        srcs = append(srcs, "abayertnrV30/rk_aiq_uapi_camgroup_abayertnr_int_v30.cpp")
        srcs = append(srcs, "abayertnrV30/rk_aiq_algo_camgroup_abayertnr_itf_v30.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if flag0 || flag1 {
        srcs = append(srcs, "abayernr2/rk_aiq_algo_camgroup_abayernr_itf_v2.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if flag0 {
        srcs = append(srcs, "misc/rk_aiq_algo_camgroup_argbir_itf.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    flag2 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    flag3 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    flag4 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if flag0 {
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
    if flag0 {
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
        srcs = append(srcs, "ayuvmeV1/rk_aiq_algo_camgroup_ayuvme_itf_v1.cpp")
        srcs = append(srcs, "ayuvmeV1/rk_aiq_uapi_camgroup_ayuvme_int_v1.cpp")
    }

    // fmt.Printf("libcamgroup_misc srcs:", srcs)
    // fmt.Printf("\n")
    return srcs;
}

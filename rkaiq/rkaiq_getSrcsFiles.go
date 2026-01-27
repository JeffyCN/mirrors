package rkaiqdefaults

import (
    "strings"
    "fmt"
)

func rkaiq_getSrcsFiles(soc string, macros_map map[string]bool) []string {

    var flag0 bool = false;
    var flag1 bool = false;
    var flag2 bool = false;
    var flag3 bool = false;

    var srcs []string;
    var hdl_srcs []string;
    var srcs_c []string;

    use_newstruct := macros_map["RKAIQ_NEWSTRUCT_TEST"]

    flag0 = macros_map["ISP_HW_V39"]
    if flag0 {
        srcs = append(srcs, "hwi/isp39/CamHwIsp39.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_FAKECAM"]
    if (flag0) {
        srcs = append(srcs, "hwi/fakecamera/FakeCamHwIsp20.cpp")
        srcs = append(srcs, "hwi/fakecamera/FakeSensorHw.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SPSTREAM"]
    if (flag0) {
        srcs = append(srcs, "hwi/isp20/SPStreamProcUnit.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_MULTIISP"]
    if (flag0) {
        srcs = append(srcs, "hwi/IspParamsSplitter.cpp")
    }

    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_camgroup_handlers/*.cpp")
        srcs = append(srcs, "RkAiqCamGroupManager.cpp")
        srcs = append(srcs, "aiq_core/RkAiqCamgroupHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_ENABLE_PARSER_V1"]
    if flag0 {
       //IQ_PARSER_SRC
        srcs = append(srcs, "iq_parser/RkAiqCalibParser.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibDb.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibTag.cpp")
        srcs = append(srcs, "iq_parser/tinyxml2.cpp")
        srcs = append(srcs, "iq_parser/xmltags.cpp")
        srcs = append(srcs, "iq_parser/RkAiqCalibApi.cpp")
       //IQ_PARSER_V2_SRC
        srcs = append(srcs, "iq_parser_v2/awb_xml2json.cpp")
        srcs = append(srcs, "iq_parser_v2/bayernr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/mfnr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/ynr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/uvnr_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/sharp_xml2json_v1.cpp")
        srcs = append(srcs, "iq_parser_v2/edgefilter_xml2json_v1.cpp")
    }
    if soc == "rv1109" {
        srcs = append(srcs, "hwi/isp20/NrStatsStream.cpp")
        srcs = append(srcs, "hwi/isp20/TnrStatsStream.cpp")
        srcs = append(srcs, "hwi/isp20/NrStreamProcUnit.cpp")
        srcs = append(srcs, "hwi/isp20/FecParamStream.cpp")
        srcs = append(srcs, "hwi/isp20/TnrStreamProcUnit.cpp")
    }

    // AIQ_CORE_SRC
    srcs = append(srcs, "aiq_core/RkAiqHandle.cpp")
    srcs = append(srcs, "aiq_core/RkAiqCore.cpp")
    srcs = append(srcs, "aiq_core/RkLumaCore.cpp")

    srcs = append(srcs, "aiq_core/MessageBus.cpp")
    srcs = append(srcs, "aiq_core/RkAiqAnalyzeGroupManager.cpp")
    srcs = append(srcs, "aiq_core/RkAiqResourceTranslator.cpp")
    srcs = append(srcs, "aiq_core/RkAiqResourceTranslatorV21.cpp")
    srcs = append(srcs, "aiq_core/RkAiqResourceTranslatorV3x.cpp")
    srcs = append(srcs, "aiq_core/RkAiqResourceTranslatorV32.cpp")
    srcs = append(srcs, "aiq_core/RkAiqResourceTranslatorV39.cpp")
    srcs = append(srcs, "aiq_core/thumbnails.cpp")

    // ALGO_COMMON_SRC
    srcs = append(srcs, "algos/interpolation.c")

    // COMMON_SRC
    srcs = append(srcs, "common/mediactl/mediactl.c")
    srcs = append(srcs, "common/code_to_pixel_format.c")
    flag0 = macros_map["RKAIQ_ENABLE_SIMULATOR"]
    flag1 = macros_map["RKAIQ_HAVE_LDCH_V10"]
    flag2 = macros_map["RKAIQ_HAVE_LDCH_V21"]
    if (!flag0 && (flag1 || flag2)) {
        srcs = append(srcs, "common/gen_mesh/genMesh.cpp")
    }

    // HWI_SRC
    srcs = append(srcs, "hwi/CamHwBase.cpp")
    srcs = append(srcs, "hwi/isp20/CamHwIsp20.cpp")
    srcs = append(srcs, "hwi/isp21/CamHwIsp21.cpp")
    srcs = append(srcs, "hwi/isp3x/CamHwIsp3x.cpp")
    srcs = append(srcs, "hwi/isp32/CamHwIsp32.cpp")
    srcs = append(srcs, "hwi/isp20/Isp20Params.cpp")
    srcs = append(srcs, "hwi/isp20/Isp20StatsBuffer.cpp")
    srcs = append(srcs, "hwi/isp20/Isp20_module_dbg.cpp")
    srcs = append(srcs, "hwi/SensorHw.cpp")
    srcs = append(srcs, "hwi/LensHw.cpp")
    srcs = append(srcs, "hwi/FlashLight.cpp")
    srcs = append(srcs, "hwi/isp20/Stream.cpp")
    srcs = append(srcs, "hwi/isp20/RawStreamCapUnit.cpp")
    srcs = append(srcs, "hwi/isp20/RawStreamProcUnit.cpp")
    srcs = append(srcs, "hwi/isp20/PdafStreamProcUnit.cpp")
    srcs = append(srcs, "hwi/isp20/CaptureRawData.cpp")
    srcs = append(srcs, "hwi/image_processor.cpp")
    srcs = append(srcs, "hwi/isp20/Isp20Evts.cpp")
    srcs = append(srcs, "hwi/isp20/CifScaleStream.cpp")
    srcs = append(srcs, "hwi/isp20/DumpRkRaw.cpp")

    srcs = append(srcs, "iq_parser/tinyxml2.cpp")

    // UAPI_SRC
    srcs = append(srcs, "uAPI2/rk_aiq_user_api2_sysctl.cpp")
    srcs = append(srcs, "uAPI2/rk_aiq_user_api2_debug.cpp")
    srcs = append(srcs, "uAPI2/rk_aiq_user_api2_helper.cpp")
    srcs = append(srcs, "uAPI2/rk_aiq_user_api2_wrapper.cpp")

    // XCORE_SRC
    srcs = append(srcs, "xcore/xcam_common.c")
    srcs = append(srcs, "xcore/xcam_log.c")
    srcs = append(srcs, "xcore/xcam_thread.cpp")
    srcs = append(srcs, "xcore/xcam_buffer.cpp")
    srcs = append(srcs, "xcore/video_buffer.cpp")
    srcs = append(srcs, "xcore/v4l2_buffer_proxy.cpp")
    srcs = append(srcs, "xcore/v4l2_device.cpp")
    srcs = append(srcs, "xcore/buffer_pool.cpp")
    srcs = append(srcs, "xcore/poll_thread.cpp")
    srcs = append(srcs, "xcore/smart_buffer_priv.cpp")
    srcs = append(srcs, "xcore/fake_v4l2_device.cpp")

    // IPC_SERVER_SRC
    srcs = append(srcs, "ipc_server/socket_server.cpp")
    srcs = append(srcs, "ipc_server/rkaiq_tool_ae.cpp")
    srcs = append(srcs, "ipc_server/rkaiq_tool_anr.cpp")
    srcs = append(srcs, "ipc_server/rkaiq_tool_imgproc.cpp")
    srcs = append(srcs, "ipc_server/rkaiq_tool_sharp.cpp")
    srcs = append(srcs, "ipc_server/rkaiq_tool_sysctl.cpp")
    srcs = append(srcs, "ipc_server/command_process.cpp")
    srcs = append(srcs, "ipc_server/MessageParser.cpp")

    srcs = append(srcs, "RkAiqManager.cpp")
    srcs = append(srcs, "RkAiqGlobalParamsManager.cpp")
    srcs = append(srcs, "RkAiqCamProfiles.cpp")

    // C_BASE_SRC
    srcs = append(srcs, "c_base/aiq_base.c")
    srcs = append(srcs, "c_base/aiq_mutex.c")
    srcs = append(srcs, "c_base/aiq_cond.c")
    srcs = append(srcs, "c_base/aiq_thread.c")
    srcs = append(srcs, "c_base/aiq_queue.c")
    srcs = append(srcs, "c_base/aiq_pool.c")
    srcs = append(srcs, "c_base/aiq_map.c")
    srcs = append(srcs, "c_base/aiq_list.c")
    // debug
    srcs = append(srcs, "c_base/aiq_cbase_test.c")
    // INI_READER_SRC
    srcs = append(srcs, "common/rkaiq_ini.c")

    if use_newstruct {
        srcs = append(srcs, "modules/rk_aiq_isp32_modules.c")
        flag1 = macros_map["ISP_HW_V39"]
        if flag1 {
            srcs = append(srcs, "modules/rk_aiq_isp39_modules.c")
        }
        flag2 = macros_map["ISP_HW_V33"]
        if flag2 {
            srcs = append(srcs, "modules/rk_aiq_isp33_modules.c")
        }
        flag3 = macros_map["ISP_HW_V35"]
        if flag3 {
            srcs = append(srcs, "modules/rk_aiq_isp35_modules.c")
        }
        srcs = append(srcs, "algos/newStruct/algo_common.c")
    }

    hdl_srcs = rkaiq_getAlgoHandleSrcs(soc, macros_map)

    srcs_c = rkaiq_getSrcsFiles_c(soc, macros_map)

    srcs = append(srcs, hdl_srcs...)

    flag0 = macros_map["RKAIQ_IMPLEMENT_C"]
    if flag0 {
        srcs = append(srcs, srcs_c...)
        srcs = rkaiq_rmSrcsCppFile(srcs)
    }

    // for i, v := range srcs {
    //     fmt.Printf("%d %s\n", i, v)
    // }
    fmt.Printf("srcs size: %d\n", len(srcs))
    return srcs;
}

func rkaiq_getAlgoHandleSrcs(soc string, macros_map map[string]bool) []string {

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

    var srcs []string

    useNewstruct := macros_map["RKAIQ_NEWSTRUCT_TEST"]

    srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbHandle.cpp")

    flag0 = macros_map["RKAIQ_HAVE_AWB_V21"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV21Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AWB_V32"]
    flag1 = macros_map["RKAIQ_HAVE_AWB_V32LT"]
    if (flag0 || flag1) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV32Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AWB_V39"]
    if flag0 {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwbV39Handle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_GIC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_GIC_V2"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGicHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgicHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ORB_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAorbHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_EIS_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAeisHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V03"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CAC_V11"]
    flag1 = macros_map["RKAIQ_HAVE_CAC_V12"]
    flag2 = macros_map["RKAIQ_HAVE_CAC_V30"]
    if (flag0 || flag1 || flag2) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCacHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcacV11Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V3"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YNR_V22"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV22Handle.cpp")
        }
    }
    
    flag0 = macros_map["RKAIQ_HAVE_YNR_V24"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAynrV24Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_UVNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV1Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V30"]
    flag1 = macros_map["RKAIQ_HAVE_CNR_V30_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV30Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CNR_V31"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcnrV31Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V3"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV3Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V4"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV4Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V33"]
    flag1 = macros_map["RKAIQ_HAVE_SHARP_V33_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqSharpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV33Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_SHARP_V34"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqSharpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsharpV34Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_MFNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmfnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArawnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArawnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayer2dnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYER2DNR_V23"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayer2dnrV23Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V2"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV2Handle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V23"]
    flag1 = macros_map["RKAIQ_HAVE_BAYERTNR_V23_LITE"]
    if (flag0 || flag1) {
        if flag0 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBtnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV23Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BAYERTNR_V30"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBtnrHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAbayertnrV30Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgainHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_GAIN_V2"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGainHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgainV2Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_BLC_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAblcHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_BLC_V32"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqBlcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAblcV32Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ANR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAnrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AMD_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_3DLUT_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiq3dlutHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqA3dlutHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CCM_V1"]
    flag1 = macros_map["RKAIQ_HAVE_CCM_V2"]
    flag2 = macros_map["RKAIQ_HAVE_CCM_V3"]
    if (flag0 || flag1 || flag2) {
        if flag2 && useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCcmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAccmHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_GAMMA_V11"]
    flag1 = macros_map["RKAIQ_HAVE_GAMMA_V10"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqGammaHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAgammaHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_MERGE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_MERGE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_MERGE_V12"]
    flag3 = macros_map["RKAIQ_HAVE_MERGE_V13"]
    if (flag0 || flag1 || flag2 || flag3) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqMergeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAmergeHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DRC_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DRC_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DRC_V12"]
    flag3 = macros_map["RKAIQ_HAVE_DRC_V12_LITE"]
    flag4 = macros_map["RKAIQ_HAVE_DRC_V20"]
    flag5 = macros_map["RKAIQ_HAVE_DRC_V21"]
    if (flag0 || flag1 || flag2 || flag3 || flag4 || flag5) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDrcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdrcHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DEHAZE_V10"]
    flag1 = macros_map["RKAIQ_HAVE_DEHAZE_V11"]
    flag2 = macros_map["RKAIQ_HAVE_DEHAZE_V11_DUO"]
    flag3 = macros_map["RKAIQ_HAVE_DEHAZE_V12"]
    flag4 = macros_map["RKAIQ_HAVE_DEHAZE_V14"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDehazeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdehazeHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqRgbirHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqArgbirHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_TMO_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAtmoHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_WDR_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAwdrHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V21"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqLdchHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAldchHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V22"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAldchHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_LDCH_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAldchHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_FEC_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfecHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_CSM_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCsmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcsmHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_CGC_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCgcHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcgcHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_AE_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAeHandle.cpp")
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqCustomAeHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_AFD_V1"]
    flag1 = macros_map["RKAIQ_HAVE_AFD_V2"]
    if (flag0 || flag1) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_DEBAYER_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DEBAYER_V2"]
    flag2 = macros_map["RKAIQ_HAVE_DEBAYER_V2_LITE"]
    flag3 = macros_map["RKAIQ_HAVE_DEBAYER_V3"]
    flag4 = macros_map["RKAIQ_HAVE_DEBAYER_V4"]
    if (flag0 || flag1 || flag2 || flag3 || flag4) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDmHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdebayerHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_LSC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_LSC_V2"]
    flag2 = macros_map["RKAIQ_HAVE_LSC_V3"]
    if (flag0 || flag1 || flag2) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqLscHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAlscHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DPCC_V1"]
    flag1 = macros_map["RKAIQ_HAVE_DPCC_V2"]
    if (flag0 || flag1) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqDpccHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdpccHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ACP_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqCpHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAcpHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_AIE_V10"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqIeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAieHandle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_ASD_V10"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAsdHandle.cpp")
    }

    flag0 = macros_map["RKAIQ_HAVE_YUVME_V1"]
    if (flag0) {
        if useNewstruct {
            srcs = append(srcs, "aiq_core/algo_handlers/newStruct/RkAiqYmeHandler.cpp")
        } else {
            srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAyuvmeV1Handle.cpp")
        }
    }

    flag0 = macros_map["RKAIQ_HAVE_DEGAMMA_V1"]
    if (flag0) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAdegammaHandle.cpp")
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
    if (flag0 || flag1 || flag2 || flag3 || flag4 || flag5 || flag6 || flag7 || flag8 || flag9) {
        srcs = append(srcs, "aiq_core/algo_handlers/RkAiqAfHandle.cpp")
    }
    return srcs
}

func rkaiq_getSrcsFiles_c(soc string, macros_map map[string]bool) []string {
    var flag0 bool = false;

    var srcs []string;

    // ipc src
    srcs = append(srcs, "ipc/socket_client.c")
    srcs = append(srcs, "ipc/message_receiver.c")
    srcs = append(srcs, "uAPI2_c/rk_aiq_user_api2_helper.c")
    srcs = append(srcs, "uAPI2_c/rk_aiq_user_api2_wrapper.c")

    // XCORE_C_SRC
    srcs = append(srcs, "xcore_c/aiq_video_buffer.c")
    srcs = append(srcs, "xcore_c/aiq_v4l2_buffer.c")
    srcs = append(srcs, "xcore_c/aiq_v4l2_device.c")
    srcs = append(srcs, "xcore_c/aiq_fake_v4l2_device.c")

    // HWI_C_SRC
    srcs = append(srcs, "hwi_c/aiq_sensorHw.c")
    srcs = append(srcs, "hwi_c/aiq_fake_sensor.c")
    srcs = append(srcs, "hwi_c/aiq_ispParamsCvt.c")
    srcs = append(srcs, "hwi_c/aiq_rawStreamCapUnit.c")
    srcs = append(srcs, "hwi_c/aiq_rawStreamProcUnit.c")
    srcs = append(srcs, "hwi_c/aiq_stream.c")
    srcs = append(srcs, "hwi_c/aiq_CamHwBase.c")
    srcs = append(srcs, "hwi_c/aiq_fake_camhw.c")
    srcs = append(srcs, "hwi_c/aiq_lensHw.c")
    srcs = append(srcs, "hwi_c/aiq_ispParamsSplitter.c")
    srcs = append(srcs, "hwi_c/aiq_ispParamsSplitterCom.c")
    srcs = append(srcs, "hwi_c/aiq_cifScaleStream.c")
    srcs = append(srcs, "hwi_c/aiq_aiIspLoader.c")
    srcs = append(srcs, "hwi_c/aiq_dumpRkRaw.c")
    flag0 = macros_map["RKAIQ_HAVE_PDAF"]
    if flag0 {
        srcs = append(srcs, "hwi_c/aiq_PdafStreamProcUnit.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_DUMPSYS"]
    if flag0 {
        srcs = append(srcs, "hwi_c/info/aiq_CamHwBaseInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_sensorHwInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_streamCapInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_streamProcInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_ispParamsCvtInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_ispActiveParamsInfo.c")
        srcs = append(srcs, "hwi_c/info/aiq_hwiIspModsInfo.c")
    }

    // CORE SRCS
    if flag0 {
        srcs = append(srcs, "aiq_core_c/info/aiq_coreInfo.c")
        srcs = append(srcs, "aiq_core_c/info/aiq_coreBufMgrInfo.c")
        srcs = append(srcs, "aiq_core_c/info/aiq_groupAnalyzerInfo.c")
        srcs = append(srcs, "aiq_core_c/info/aiq_coreIspParamsInfo.c")
        srcs = append(srcs, "aiq_core_c/info/aiq_coreIspModsInfo.c")
        srcs = append(srcs, "aiq_core_c/info/aiq_coreAlgosInfo.c")
    }

    // COMMON_SRC
    if flag0 {
        srcs = append(srcs, "common/aiq_notifier.c")
    } else {
        srcs = append(srcs, "dumpcam_server/rk_aiq_dummy_function.c")
    }


    flag0 = macros_map["ISP_HW_V39"]
    if flag0 {
        srcs = append(srcs, "hwi_c/isp39/aiq_CamHwIsp39.c")
        srcs = append(srcs, "hwi_c/isp39/aiq_isp39ParamsCvt.c")
        srcs = append(srcs, "hwi_c/isp39/aiq_isp39ParamsSplitter.c")
    }

    flag0 = macros_map["ISP_HW_V33"]
    if flag0 {
        srcs = append(srcs, "hwi_c/isp33/aiq_CamHwIsp33.c")
        srcs = append(srcs, "hwi_c/isp39/aiq_isp39ParamsCvt.c")
        srcs = append(srcs, "hwi_c/isp33/aiq_isp33ParamsCvt.c")
    }

    // CORE_C_SRC
    srcs = append(srcs, "aiq_core_c/aiq_core.c")
    srcs = append(srcs, "aiq_core_c/aiq_stats_translator.c")
    srcs = append(srcs, "aiq_core_c/aiq_algogroups_manager.c")
    srcs = append(srcs, "aiq_core_c/aiq_algo_handler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqBlcHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqBtnrHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCnrHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqDmHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqDrcHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqDpccHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqGammaHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqGicHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqSharpHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqYnrHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCcmHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqLscHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCgcHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCpHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCsmHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqGainHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqIeHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqMergeHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqAeHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqAwbHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqAfdHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqCacHandler.c")
    srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqAmtdHandler.c")

    flag0 = macros_map["ISP_HW_V39"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/aiq_stats_translator_v39.c")
    }

    flag0 = macros_map["ISP_HW_V33"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/aiq_stats_translator_v33.c")
    }

    flag0 = macros_map["RKAIQ_HAVE_AF"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqAfHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_3DLUT"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiq3dlutHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_RGBIR_REMOSAIC"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqRgbirHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_DEHAZE"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqDehazeHandler.c")
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqHisteqHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_YUVME"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqYmeHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_HISTEQ"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqHisteqHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_ENHANCE"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqEnhHandler.c")
    }
    flag0 = macros_map["RKAIQ_HAVE_LDCH_V22"]
    if (flag0) {
        srcs = append(srcs, "aiq_core_c/algo_handlers/RkAiqLdcHandler.c")
    }

    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if flag0 {
        srcs = append(srcs, "aiq_core_c/aiq_algo_camgroup_handler.c")
        srcs = append(srcs, "aiq_core_c/algo_camgroup_handlers/RkAiqCamGroupAeHandle.c")
        srcs = append(srcs, "aiq_core_c/algo_camgroup_handlers/RkAiqCamGroupAwbHandle.c")
        srcs = append(srcs, "algos_camgroup/newStruct/rk_aiq_algo_camgroup_common.c")
    }

    // UAPI2_C_SRC
    srcs = append(srcs, "uAPI2_c/rk_aiq_user_api2_sysctl.c")

    // AIQ_C_SRC
    srcs = append(srcs, "RkAiqGlobalParamsManager_c.c")
    srcs = append(srcs, "RkAiqManager_c.c")
    srcs = append(srcs, "RkAibnrManager.c")
    srcs = append(srcs, "RknnManager.c")

    flag0 = macros_map["RKAIQ_ENABLE_CAMGROUP"]
    if flag0 {
        srcs = append(srcs, "RkAiqCamGroupManager_c.c")
    }

    return srcs
}

func rkaiq_rmSrcsCppFile(srcs []string) []string {

    var srcs_c []string

    for i := 0; i < len(srcs); i++ {
        if strings.Contains(srcs[i], ".cpp") {
            continue
        }
        srcs_c = append(srcs_c, srcs[i])
    }

    return srcs_c
}

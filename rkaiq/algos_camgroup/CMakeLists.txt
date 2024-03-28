
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

# surpress CXX_VISIBILITY_PRESET warning
if (POLICY CMP0063) # Visibility
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
	../xcore
	../xcore/base
	../algos
	./
)

if(WIN32)
    set(PREFIX )
	set(SUFFIX lib)
else()
    set(PREFIX ${LIBRARY_OUTPUT_PATH}/lib)
	set(SUFFIX a)
endif()

if (${C_LIBRARY_NAME} EQUAL "")
    set(C_LIBRARY_NAME "glibc" CACHE INTERNAL "" FORCE)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} OS)
string(TOLOWER ${RKAIQ_TARGET_SOC} SOC)
set(AWBPREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/awb/${OS}/${SOC}/${ARCH}/${C_LIBRARY_NAME}")
set(AEPREBUILT_DIR "${CMAKE_CURRENT_LIST_DIR}/ae/${OS}/${SOC}/${ARCH}/${C_LIBRARY_NAME}")

# ----------------------------------------------------------------------------
# add ae implement
# ----------------------------------------------------------------------------
set (GROUP_AE_SRC
    ae/rk_aiq_algo_camgroup_ae_itf.cpp
	ae/rk_aiq_group_ae_algo.cpp
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ae/rk_aiq_algo_camgroup_ae_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_camgroup_ae STATIC ${GROUP_AE_SRC})
	set_target_properties(rkaiq_camgroup_ae PROPERTIES FOLDER "algos_camgroup/ae")
	set_target_properties(rkaiq_camgroup_ae PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_camgroup_ae PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_camgroup_ae PROPERTIES CXX_VISIBILITY_PRESET default)
	install(TARGETS rkaiq_camgroup_ae ARCHIVE DESTINATION ${AEPREBUILT_DIR})
else()
    message(STATUS "Can NOT found ${CMAKE_CURRENT_SOURCE_DIR}/ae/rk_aiq_algo_camgroup_ae_itf.cpp. Using lib")
    set(PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/ae/${OS}/${SOC}/${ARCH}/${C_LIBRARY_NAME}/lib")
	add_library(rkaiq_camgroup_ae STATIC IMPORTED GLOBAL)
	set_target_properties(rkaiq_camgroup_ae PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_camgroup_ae.${SUFFIX}")
	set_target_properties(rkaiq_camgroup_ae PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_camgroup_ae.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add awb implement
# ----------------------------------------------------------------------------
set (AWB_SRC
    awb/rk_aiq_algo_camgroup_awb_itf.cpp
	# uapi
	awb/rk_aiq_uapiv2_camgroup_awb_int.cpp
)

if (RKAIQ_HAVE_AWB_V21)
    add_compile_options(-DRKAIQ_HAVE_AWB_V21=1)
    list(APPEND AWB_SRC awb/rk_aiq_awb_algo_surround_view_v201.cpp)
endif()

if (RKAIQ_HAVE_AWB_V32)
    add_compile_options(-DRKAIQ_HAVE_AWB_V32=1)
    list(APPEND AWB_SRC awb/rk_aiq_awb_algo_surround_view_v32.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/awb/rk_aiq_algo_camgroup_awb_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_camgroup_awb STATIC ${AWB_SRC})
	set_target_properties(rkaiq_camgroup_awb PROPERTIES FOLDER "algos_camgroup/awb")
	set_target_properties(rkaiq_camgroup_awb PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_camgroup_awb PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_camgroup_awb PROPERTIES CXX_VISIBILITY_PRESET default)
	install(TARGETS rkaiq_camgroup_awb ARCHIVE DESTINATION ${AWBPREBUILT_DIR})
else()
    message(STATUS "Can NOT found ${CMAKE_CURRENT_SOURCE_DIR}/awb/rk_aiq_algo_camgroup_awb_itf.cpp. Using lib")
    set(PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/awb/${OS}/${SOC}/${ARCH}/${C_LIBRARY_NAME}/lib")
	add_library(rkaiq_camgroup_awb STATIC IMPORTED GLOBAL)
	set_target_properties(rkaiq_camgroup_awb PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_camgroup_awb.${SUFFIX}")
	set_target_properties(rkaiq_camgroup_awb PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_camgroup_awb.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add camgroup misc implement
# ----------------------------------------------------------------------------
set (MISC_SRC
	misc/rk_aiq_algo_camgroup_alsc_itf.cpp
	# misc/rk_aiq_algo_camgroup_adpcc_itf.cpp
	misc/rk_aiq_algo_camgroup_accm_itf.cpp
	misc/rk_aiq_algo_camgroup_a3dlut_itf.cpp
	misc/rk_aiq_algo_camgroup_amerge_itf.cpp
)

if(RKAIQ_NEWSTRUCT_TEST)
    list(APPEND MISC_SRC newStruct/blc/rk_aiq_algo_camgroup_blc.cpp)
else()
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_ablc_itf.cpp)
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_ablc_itf_v32.cpp)
endif()

if(RKAIQ_NEWSTRUCT_TEST)
    list(APPEND MISC_SRC newStruct/gamma/rk_aiq_algo_camgroup_gamma.cpp)
    list(APPEND MISC_SRC newStruct/drc/rk_aiq_algo_camgroup_drc.cpp)
    list(APPEND MISC_SRC newStruct/dehaze/rk_aiq_algo_camgroup_dehaze.cpp)
    list(APPEND MISC_SRC newStruct/dpc/rk_aiq_algo_camgroup_dpc.cpp)
else()
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_agamma_itf.cpp)
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_adrc_itf.cpp)
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_adehaze_itf.cpp)
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_adpcc_itf.cpp)
endif()

if (RKAIQ_HAVE_GAIN_V2)
    list(APPEND MISC_SRC again2/rk_aiq_algo_camgroup_again_itf_v2.cpp)
    list(APPEND MISC_SRC again2/rk_aiq_uapi_camgroup_again_int_v2.cpp)
endif()

if (RKAIQ_HAVE_BAYER2DNR_V2)
    list(APPEND MISC_SRC abayer2dnr2/rk_aiq_algo_camgroup_abayer2dnr_itf_v2.cpp)
    list(APPEND MISC_SRC abayer2dnr2/rk_aiq_uapi_camgroup_abayer2dnr_int_v2.cpp)
elseif (RKAIQ_HAVE_BAYER2DNR_V23)
    list(APPEND MISC_SRC abayer2dnrV23/rk_aiq_uapi_camgroup_abayer2dnr_int_v23.cpp)
    list(APPEND MISC_SRC abayer2dnrV23/rk_aiq_algo_camgroup_abayer2dnr_itf_v23.cpp)
endif()

if (RKAIQ_HAVE_YNR_V2)
    list(APPEND MISC_SRC aynr2/rk_aiq_algo_camgroup_aynr_itf_v2.cpp)
elseif (RKAIQ_HAVE_YNR_V3)
    list(APPEND MISC_SRC aynr3/rk_aiq_algo_camgroup_aynr_itf_v3.cpp)
    list(APPEND MISC_SRC aynr3/rk_aiq_uapi_camgroup_aynr_int_v3.cpp)
elseif (RKAIQ_HAVE_YNR_V22)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/ynr/rk_aiq_algo_camgroup_ynr.cpp)
	else ()
        list(APPEND MISC_SRC aynrV22/rk_aiq_algo_camgroup_aynr_itf_v22.cpp)
        list(APPEND MISC_SRC aynrV22/rk_aiq_uapi_camgroup_aynr_int_v22.cpp)
    endif()
elseif (RKAIQ_HAVE_YNR_V24)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/ynr/rk_aiq_algo_camgroup_ynr.cpp)
	else ()
        list(APPEND MISC_SRC aynrV24/rk_aiq_algo_camgroup_aynr_itf_v24.cpp)
        list(APPEND MISC_SRC aynrV24/rk_aiq_uapi_camgroup_aynr_int_v24.cpp)
    endif()
endif()

if (RKAIQ_HAVE_BAYERTNR_V2)
    list(APPEND MISC_SRC abayertnr2/rk_aiq_algo_camgroup_abayertnr_itf_v2.cpp)
    list(APPEND MISC_SRC abayertnr2/rk_aiq_uapi_camgroup_abayertnr_int_v2.cpp)
elseif (RKAIQ_HAVE_BAYERTNR_V23 OR RKAIQ_HAVE_BAYERTNR_V23_LITE)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.cpp)
	else ()
        list(APPEND MISC_SRC abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.cpp)
        list(APPEND MISC_SRC abayertnrV23/rk_aiq_algo_camgroup_abayertnr_itf_v23.cpp)
    endif()
elseif (RKAIQ_HAVE_BAYERTNR_V30)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.cpp)
	else ()
        list(APPEND MISC_SRC abayertnrV30/rk_aiq_uapi_camgroup_abayertnr_int_v30.cpp)
        list(APPEND MISC_SRC abayertnrV30/rk_aiq_algo_camgroup_abayertnr_itf_v30.cpp)
    endif()
endif()

if (RKAIQ_HAVE_BAYERNR_V1 OR RKAIQ_HAVE_BAYERNR_V2)
    list(APPEND MISC_SRC abayernr2/rk_aiq_algo_camgroup_abayernr_itf_v2.cpp)
endif()

if (RKAIQ_HAVE_RGBIR_REMOSAIC)
    list(APPEND MISC_SRC misc/rk_aiq_algo_camgroup_argbir_itf.cpp)
endif()

if(RKAIQ_NEWSTRUCT_TEST)
    list(APPEND MISC_SRC newStruct/sharp/rk_aiq_algo_camgroup_sharp.cpp)
else()
    if (RKAIQ_HAVE_SHARP_V34)
        list(APPEND MISC_SRC asharpV34/rk_aiq_uapi_camgroup_asharp_int_v34.cpp)
        list(APPEND MISC_SRC asharpV34/rk_aiq_algo_camgroup_asharp_itf_v34.cpp)
    elseif (RKAIQ_HAVE_SHARP_V33 OR RKAIQ_HAVE_SHARP_V33_LITE)
        list(APPEND MISC_SRC asharpV33/rk_aiq_uapi_camgroup_asharp_int_v33.cpp)
        list(APPEND MISC_SRC asharpV33/rk_aiq_algo_camgroup_asharp_itf_v33.cpp)
    elseif (RKAIQ_HAVE_SHARP_V3)
        list(APPEND MISC_SRC asharp3/rk_aiq_algo_camgroup_asharp_itf_v3.cpp)
    elseif (RKAIQ_HAVE_SHARP_V4)
        list(APPEND MISC_SRC asharp4/rk_aiq_algo_camgroup_asharp_itf_v4.cpp)
        list(APPEND MISC_SRC asharp4/rk_aiq_uapi_camgroup_asharp_int_v4.cpp)
    endif()
endif()

if (RKAIQ_HAVE_CNR_V31)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/cnr/rk_aiq_algo_camgroup_cnr.cpp)
	else ()
        list(APPEND MISC_SRC acnrV31/rk_aiq_algo_camgroup_acnr_itf_v31.cpp)
        list(APPEND MISC_SRC acnrV31/rk_aiq_uapi_camgroup_acnr_int_v31.cpp)
    endif()
elseif (RKAIQ_HAVE_CNR_V30 OR RKAIQ_HAVE_CNR_V30_LITE)
	if (RKAIQ_NEWSTRUCT_TEST)
        list(APPEND MISC_SRC newStruct/cnr/rk_aiq_algo_camgroup_cnr.cpp)
	else ()
        list(APPEND MISC_SRC acnrV30/rk_aiq_algo_camgroup_acnr_itf_v30.cpp)
        list(APPEND MISC_SRC acnrV30/rk_aiq_uapi_camgroup_acnr_int_v30.cpp)
    endif()
elseif (RKAIQ_HAVE_CNR_V1)
    list(APPEND MISC_SRC acnr/rk_aiq_algo_camgroup_acnr_itf.cpp)
elseif (RKAIQ_HAVE_CNR_V2)
    list(APPEND MISC_SRC acnr2/rk_aiq_algo_camgroup_acnr_itf_v2.cpp)
    list(APPEND MISC_SRC acnr2/rk_aiq_uapi_camgroup_acnr_int_v2.cpp)
endif()

if (RKAIQ_HAVE_YUVME_V1)
    list(APPEND MISC_SRC ayuvmeV1/rk_aiq_algo_camgroup_ayuvme_itf_v1.cpp)
    list(APPEND MISC_SRC ayuvmeV1/rk_aiq_uapi_camgroup_ayuvme_int_v1.cpp)
endif()

add_library(rkaiq_camgroup_misc STATIC ${MISC_SRC})
set_target_properties(rkaiq_camgroup_misc PROPERTIES FOLDER "algos_camgroup/misc")



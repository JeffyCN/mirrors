
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
	misc/rk_aiq_algo_camgroup_adpcc_itf.cpp
	misc/rk_aiq_algo_camgroup_accm_itf.cpp
	misc/rk_aiq_algo_camgroup_a3dlut_itf.cpp
	misc/rk_aiq_algo_camgroup_adehaze_itf.cpp
	misc/rk_aiq_algo_camgroup_agamma_itf.cpp
	misc/rk_aiq_algo_camgroup_amerge_itf.cpp
	misc/rk_aiq_algo_camgroup_adrc_itf.cpp
	misc/rk_aiq_algo_camgroup_ablc_itf.cpp
	misc/rk_aiq_algo_camgroup_ablc_itf_v32.cpp
	aynr2/rk_aiq_algo_camgroup_aynr_itf_v2.cpp
	aynr3/rk_aiq_algo_camgroup_aynr_itf_v3.cpp
	aynr3/rk_aiq_uapi_camgroup_aynr_int_v3.cpp
	aynrV22/rk_aiq_algo_camgroup_aynr_itf_v22.cpp
	aynrV22/rk_aiq_uapi_camgroup_aynr_int_v22.cpp
	acnr/rk_aiq_algo_camgroup_acnr_itf.cpp
	acnr2/rk_aiq_algo_camgroup_acnr_itf_v2.cpp
	acnr2/rk_aiq_uapi_camgroup_acnr_int_v2.cpp
	acnrV30/rk_aiq_algo_camgroup_acnr_itf_v30.cpp
	acnrV30/rk_aiq_uapi_camgroup_acnr_int_v30.cpp
	asharp3/rk_aiq_algo_camgroup_asharp_itf_v3.cpp
	asharp4/rk_aiq_algo_camgroup_asharp_itf_v4.cpp
	asharp4/rk_aiq_uapi_camgroup_asharp_int_v4.cpp
	asharpV33/rk_aiq_uapi_camgroup_asharp_int_v33.cpp
	asharpV33/rk_aiq_algo_camgroup_asharp_itf_v33.cpp
	abayernr2/rk_aiq_algo_camgroup_abayernr_itf_v2.cpp
	abayer2dnr2/rk_aiq_algo_camgroup_abayer2dnr_itf_v2.cpp
	abayer2dnr2/rk_aiq_uapi_camgroup_abayer2dnr_int_v2.cpp
	abayer2dnrV23/rk_aiq_uapi_camgroup_abayer2dnr_int_v23.cpp
	abayer2dnrV23/rk_aiq_algo_camgroup_abayer2dnr_itf_v23.cpp
	abayertnr2/rk_aiq_algo_camgroup_abayertnr_itf_v2.cpp
	abayertnr2/rk_aiq_uapi_camgroup_abayertnr_int_v2.cpp
	abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.cpp
	abayertnrV23/rk_aiq_algo_camgroup_abayertnr_itf_v23.cpp
	again2/rk_aiq_algo_camgroup_again_itf_v2.cpp
	again2/rk_aiq_uapi_camgroup_again_int_v2.cpp
)
add_library(rkaiq_camgroup_misc STATIC ${MISC_SRC})
set_target_properties(rkaiq_camgroup_misc PROPERTIES FOLDER "algos_camgroup/misc")



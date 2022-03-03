
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8)

# surpress CXX_VISIBILITY_PRESET warning
cmake_policy(SET CMP0063 NEW)

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

# ----------------------------------------------------------------------------
# add ae implement
# ----------------------------------------------------------------------------
set (GROUP_AE_SRC
    ae/rk_aiq_algo_camgroup_ae_itf.cpp
	ae/rk_aiq_group_ae_algo.cpp
)

if(EXISTS "${CMAKE_SOURCE_DIR}/algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_camgroup_ae STATIC ${GROUP_AE_SRC})
	set_target_properties(rkaiq_camgroup_ae PROPERTIES FOLDER "algos_camgroup/ae")
	set_target_properties(rkaiq_camgroup_ae PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_camgroup_ae PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_camgroup_ae PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.cpp. Using lib")
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS)
    set(PREFIX "${CMAKE_CURRENT_LIST_DIR}/ae/${OS}/${ARCH}/lib")
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
    awb/rk_aiq_awb_algo_surround_view_v201.cpp
	# uapi
	awb/rk_aiq_uapiv2_camgroup_awb_int.cpp
)

if(EXISTS "${CMAKE_SOURCE_DIR}/algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_camgroup_awb STATIC ${AWB_SRC})
	set_target_properties(rkaiq_camgroup_awb PROPERTIES FOLDER "algos_camgroup/awb")
	set_target_properties(rkaiq_camgroup_awb PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_camgroup_awb PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_camgroup_awb PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.cpp. Using lib")
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS)
    set(PREFIX "${CMAKE_CURRENT_LIST_DIR}/awb/${OS}/${ARCH}/lib")
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
	misc/rk_aiq_algo_camgroup_adhaz_itf.cpp
	misc/rk_aiq_algo_camgroup_agamma_itf.cpp
	misc/rk_aiq_algo_camgroup_amerge_itf.cpp
	misc/rk_aiq_algo_camgroup_adrc_itf.cpp
	misc/rk_aiq_algo_camgroup_ablc_itf.cpp
	aynr/rk_aiq_algo_camgroup_aynr_itf.cpp
	acnr/rk_aiq_algo_camgroup_acnr_itf.cpp
	asharp/rk_aiq_algo_camgroup_asharp_itf.cpp
	abayernr/rk_aiq_algo_camgroup_abayernr_itf.cpp
	atnr/rk_aiq_algo_camgroup_atnr_itf.cpp
	again/rk_aiq_algo_camgroup_again_itf.cpp
	aynr/rk_aiq_uapi_camgroup_aynr_int_v3.cpp
	acnr/rk_aiq_uapi_camgroup_acnr_int_v2.cpp
	abayernr/rk_aiq_uapi_camgroup_abayer2dnr_int_v2.cpp
	asharp/rk_aiq_uapi_camgroup_asharp_int_v4.cpp
	atnr/rk_aiq_uapi_camgroup_atnr_int_v2.cpp
	again/rk_aiq_uapi_camgroup_again_int_v2.cpp
)
add_library(rkaiq_camgroup_misc STATIC ${MISC_SRC})
set_target_properties(rkaiq_camgroup_misc PROPERTIES FOLDER "algos_camgroup/misc")



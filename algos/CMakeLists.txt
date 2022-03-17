
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
set (AE_SRC
    ae/rk_aiq_algo_ae_itf.cpp
    ae/rk_aiq_ae_algo.cpp
    ae/rk_aiq_ae_algo_hdrecm.cpp
    ae/rk_aiq_ae_algo_ecm.cpp
    ae/rk_aiq_ae_algo_clm.cpp
    ae/rk_aiq_ae_algo_ldm.cpp
    ae/rk_aiq_ae_algo_pid.cpp
    ae/rk_aiq_uapi_ae_int.cpp
)

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/ae/rk_aiq_algo_ae_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_ae STATIC ${AE_SRC})
	set_target_properties(rkaiq_ae PROPERTIES FOLDER "algos/ae")
	set_target_properties(rkaiq_ae PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_ae PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_ae PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/ae/rk_aiq_algo_ae_itf.cpp. Using lib")
	add_library(rkaiq_ae STATIC IMPORTED)
	set_target_properties(rkaiq_ae PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_ae.${SUFFIX}")
	set_target_properties(rkaiq_ae PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_ae.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add awb implement
# ----------------------------------------------------------------------------
set (AWB_SRC
	awb/color_space_convert.cpp
	awb/fixfloat.cpp
	awb/rk_aiq_algo_awb_itf.cpp
	awb/rk_aiq_awb_algo_caga.cpp
	awb/rk_aiq_awb_algo_com1.cpp
	awb/rk_aiq_awb_algo_com2.cpp
	awb/rk_aiq_awb_algo_sgc.cpp
	awb/rk_aiq_awb_algo_v200.cpp
	awb/rk_aiq_awb_algo_v200.h
	awb/rk_aiq_awb_algo_v201.cpp
	awb/rk_aiq_algo_awb_itf.cpp
	awb/rk_aiq_uapi_awb_int.cpp
	awb/rk_aiq_uapiv2_awb_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/awb/rk_aiq_algo_awb_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_awb STATIC ${AWB_SRC})
	set_target_properties(rkaiq_awb PROPERTIES FOLDER "algos/awb")
	set_target_properties(rkaiq_awb PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_awb PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_awb PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/awb/rk_aiq_algo_awb_itf.cpp. Using lib")
	add_library(rkaiq_awb STATIC IMPORTED)
	set_target_properties(rkaiq_awb PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_awb.${SUFFIX}")
	set_target_properties(rkaiq_awb PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_awb.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add af implement
# ----------------------------------------------------------------------------
set (AF_SRC
	af/rk_aiq_algo_af_itf.cpp
	af/af.cpp
	af/af_search.cpp
	af/af_trigger.cpp
	af/af_utils.cpp
	af/rk_aiq_uapi_af_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/af/rk_aiq_algo_af_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_af STATIC ${AF_SRC})
	set_target_properties(rkaiq_af PROPERTIES FOLDER "algos/af")
	set_target_properties(rkaiq_af PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_af PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_af PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/af/rk_aiq_algo_af_itf.cpp. Using lib")
	add_library(rkaiq_af STATIC IMPORTED)
	set_target_properties(rkaiq_af PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_af.${SUFFIX}")
	set_target_properties(rkaiq_af PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_af.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add amerge implement
# ----------------------------------------------------------------------------
set (AMERGE_SRC
	amerge/rk_aiq_algo_amerge_itf.cpp
	amerge/rk_aiq_amerge_algo.cpp
	amerge/rk_aiq_uapi_amerge_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/amerge/rk_aiq_algo_amerge_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_amerge STATIC ${AMERGE_SRC})
	set_target_properties(rkaiq_amerge PROPERTIES FOLDER "algos/amerge")
	set_target_properties(rkaiq_amerge PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_amerge PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_amerge PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/amerge/rk_aiq_algo_amerge_itf.cpp. Using lib")
	add_library(rkaiq_amerge STATIC IMPORTED)
	set_target_properties(rkaiq_amerge PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_amerge.${SUFFIX}")
	set_target_properties(rkaiq_amerge PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_amerge.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add atmo implement
# ----------------------------------------------------------------------------
set (ATMO_SRC
	atmo/rk_aiq_algo_atmo_itf.cpp
	atmo/rk_aiq_atmo_algo.cpp
	atmo/rk_aiq_uapi_atmo_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/atmo/rk_aiq_algo_atmo_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_atmo STATIC ${ATMO_SRC})
	set_target_properties(rkaiq_atmo PROPERTIES FOLDER "algos/atmo")
	set_target_properties(rkaiq_atmo PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_atmo PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_atmo PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/atmo/rk_aiq_algo_atmo_itf.cpp. Using lib")
	add_library(rkaiq_atmo STATIC IMPORTED)
	set_target_properties(rkaiq_atmo PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_atmo.${SUFFIX}")
	set_target_properties(rkaiq_atmo PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_atmo.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add adrc implement
# ----------------------------------------------------------------------------
set (ADRC_SRC
	adrc/rk_aiq_algo_adrc_itf.cpp
	adrc/rk_aiq_adrc_algo.cpp
	adrc/rk_aiq_uapi_adrc_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/adrc/rk_aiq_algo_adrc_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_adrc STATIC ${ADRC_SRC})
	set_target_properties(rkaiq_adrc PROPERTIES FOLDER "algos/adrc")
	set_target_properties(rkaiq_adrc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_adrc PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_adrc PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found 
	${CMAKE_SOURCE_DIR}/algos/adrc/rk_aiq_algo_adrc_itf.cpp. Using lib")
	add_library(rkaiq_adrc STATIC IMPORTED)
	set_target_properties(rkaiq_adrc PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_adrc.${SUFFIX}")
	set_target_properties(rkaiq_adrc PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_adrc.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add anr implement
# ----------------------------------------------------------------------------
set (ANR_SRC
	anr/rk_aiq_algo_anr_itf.cpp
	anr/rk_aiq_anr_algo.cpp
	anr/rk_aiq_anr_algo_bayernr.cpp
	anr/rk_aiq_anr_algo_mfnr.cpp
	anr/rk_aiq_anr_algo_uvnr.cpp
	anr/rk_aiq_anr_algo_ynr.cpp
	anr/rk_aiq_uapi_anr_int.cpp
	anr/rk_aiq_anr_algo_gain.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/anr/rk_aiq_algo_anr_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_anr STATIC ${ANR_SRC})
	set_target_properties(rkaiq_anr PROPERTIES FOLDER "algos/anr")
	set_target_properties(rkaiq_anr PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_anr PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_anr PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/anr/rk_aiq_algo_anr_itf.cpp. Using lib")
	add_library(rkaiq_anr STATIC IMPORTED)
	set_target_properties(rkaiq_anr PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_anr.${SUFFIX}")
	set_target_properties(rkaiq_anr PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_anr.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add asharp implement
# ----------------------------------------------------------------------------
set (ASHARP_SRC
	asharp/rk_aiq_algo_asharp_itf.cpp
	asharp/rk_aiq_asharp_algo.cpp
	asharp/rk_aiq_asharp_algo_edgefilter.cpp
	asharp/rk_aiq_asharp_algo_sharp.cpp
	asharp/rk_aiq_uapi_asharp_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/asharp/rk_aiq_algo_asharp_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_asharp STATIC ${ASHARP_SRC})
	set_target_properties(rkaiq_asharp PROPERTIES FOLDER "algos/asharp")
	set_target_properties(rkaiq_asharp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_asharp PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_asharp PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/asharp/rk_aiq_algo_asharp_itf.cpp. Using lib")
	add_library(rkaiq_asharp STATIC IMPORTED)
	set_target_properties(rkaiq_asharp PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_asharp.${SUFFIX}")
	set_target_properties(rkaiq_anr PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_asharp.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add asd implement
# ----------------------------------------------------------------------------
set (ASD_SRC
	asd/rk_aiq_algo_asd_itf.cpp
    asd/scene_detector.cpp
	asd/rk_aiq_uapi_asd_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/asd/rk_aiq_algo_asd_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_asd STATIC ${ASD_SRC})
	set_target_properties(rkaiq_asd PROPERTIES FOLDER "algos/asd")
	set_target_properties(rkaiq_asd PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_asd PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_asd PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/asd/rk_aiq_algo_asd_itf.cpp. Using lib")
	add_library(rkaiq_asd STATIC IMPORTED)
	set_target_properties(rkaiq_asd PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_asd.${SUFFIX}")
	set_target_properties(rkaiq_asd PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_asd.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add acp implement
# ----------------------------------------------------------------------------
set (ACP_SRC
	acp/rk_aiq_algo_acp_itf.cpp
	acp/rk_aiq_uapi_acp_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/acp/rk_aiq_algo_acp_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_acp STATIC ${ACP_SRC})
	set_target_properties(rkaiq_acp PROPERTIES FOLDER "algos/acp")
	set_target_properties(rkaiq_acp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_acp PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_acp PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/acp/rk_aiq_algo_acp_itf.cpp. Using lib")
	add_library(rkaiq_acp STATIC IMPORTED)
	set_target_properties(rkaiq_acp PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_acp.${SUFFIX}")
	set_target_properties(rkaiq_acp PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_acp.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add adhaz implement
# ----------------------------------------------------------------------------
set (ADHAZ_SRC
	adehaze/rk_aiq_algo_adhaz_itf.cpp
    adehaze/rk_aiq_adehaze_algo.cpp

    adehaze/rk_aiq_uapi_adehaze_int.cpp

    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/adehaze/rk_aiq_algo_adhaz_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_adhaz STATIC ${ADHAZ_SRC})
	set_target_properties(rkaiq_adhaz PROPERTIES FOLDER "algos/adehaze")
	set_target_properties(rkaiq_adhaz PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_adhaz PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_adhaz PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/adehaze/rk_aiq_algo_adhaz_itf.cpp. Using lib")
	add_library(rkaiq_adhaz STATIC IMPORTED)
	set_target_properties(rkaiq_adhaz PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_adhaz.${SUFFIX}")
	set_target_properties(rkaiq_adhaz PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_adhaz.${SUFFIX}")
endif()


# ----------------------------------------------------------------------------
# add a3dlut implement
# ----------------------------------------------------------------------------
set (A3DLUT_SRC
	a3dlut/rk_aiq_algo_a3dlut_itf.cpp
	a3dlut/rk_aiq_a3dlut_algo.cpp
	a3dlut/rk_aiq_uapi_a3dlut_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/a3dlut/rk_aiq_algo_a3dlut_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_a3dlut STATIC ${A3DLUT_SRC})
	set_target_properties(rkaiq_a3dlut PROPERTIES FOLDER "algos/a3dlut")
	set_target_properties(rkaiq_a3dlut PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_a3dlut PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_a3dlut PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/a3dlut/rk_aiq_algo_a3dlut_itf.cpp. Using lib")
	add_library(rkaiq_a3dlut STATIC IMPORTED)
	set_target_properties(rkaiq_a3dlut PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_a3dlut.${SUFFIX}")
	set_target_properties(rkaiq_a3dlut PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_a3dlut.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add ablc implement
# ----------------------------------------------------------------------------
set (ABLC_SRC
	ablc/rk_aiq_algo_ablc_itf.cpp
	ablc/rk_aiq_ablc_algo.cpp
	ablc/rk_aiq_uapi_ablc_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/ablc/rk_aiq_algo_ablc_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_ablc STATIC ${ABLC_SRC})
	set_target_properties(rkaiq_ablc PROPERTIES FOLDER "algos/ablc")
	set_target_properties(rkaiq_ablc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_ablc PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_ablc PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/ablc/rk_aiq_algo_ablc_itf.cpp. Using lib")
	add_library(rkaiq_ablc STATIC IMPORTED)
	set_target_properties(rkaiq_ablc PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_ablc.${SUFFIX}")
	set_target_properties(rkaiq_ablc PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_ablc.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add accm implement
# ----------------------------------------------------------------------------
set (ACCM_SRC
	accm/rk_aiq_algo_accm_itf.cpp
	accm/rk_aiq_accm_algo.cpp
	accm/rk_aiq_uapi_accm_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/accm/rk_aiq_algo_accm_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_accm STATIC ${ACCM_SRC})
	set_target_properties(rkaiq_accm PROPERTIES FOLDER "algos/accm")
	set_target_properties(rkaiq_accm PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_accm PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_accm PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/accm/rk_aiq_algo_accm_itf.cpp. Using lib")
	add_library(rkaiq_accm STATIC IMPORTED)
	set_target_properties(rkaiq_accm PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_accm.${SUFFIX}")
	set_target_properties(rkaiq_accm PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_accm.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add acgc implement
# ----------------------------------------------------------------------------
set (ACGC_SRC
	acgc/rk_aiq_algo_acgc_itf.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/acgc/rk_aiq_algo_acgc_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_acgc STATIC ${ACGC_SRC})
	set_target_properties(rkaiq_acgc PROPERTIES FOLDER "algos/acgc")
	set_target_properties(rkaiq_acgc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_acgc PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_acgc PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/acgc/rk_aiq_algo_acgc_itf.cpp. Using lib")
	add_library(rkaiq_acgc STATIC IMPORTED)
	set_target_properties(rkaiq_acgc PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_acgc.${SUFFIX}")
	set_target_properties(rkaiq_acgc PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_acgc.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add adebayer implement
# ----------------------------------------------------------------------------
set (ADEBAYER_SRC
	adebayer/rk_aiq_algo_adebayer_itf.cpp
	adebayer/rk_aiq_algo_adebayer.cpp
	adebayer/rk_aiq_uapi_adebayer_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/adebayer/rk_aiq_algo_adebayer_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_adebayer STATIC ${ADEBAYER_SRC})
	set_target_properties(rkaiq_adebayer PROPERTIES FOLDER "algos/adebayer")
	set_target_properties(rkaiq_adebayer PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_adebayer PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_adebayer PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/adebayer/rk_aiq_algo_adebayer_itf.cpp. Using lib")
	add_library(rkaiq_adebayer STATIC IMPORTED)
	set_target_properties(rkaiq_adebayer PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_adebayer.${SUFFIX}")
	set_target_properties(rkaiq_adebayer PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_adebayer.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add adpcc implement
# ----------------------------------------------------------------------------
set (ADPCC_SRC
	adpcc/rk_aiq_algo_adpcc_itf.cpp
	adpcc/rk_aiq_adpcc_algo.cpp
	adpcc/rk_aiq_uapi_adpcc_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/adpcc/rk_aiq_algo_adpcc_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_adpcc STATIC ${ADPCC_SRC})
	set_target_properties(rkaiq_adpcc PROPERTIES FOLDER "algos/adpcc")
	set_target_properties(rkaiq_adpcc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_adpcc PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_adpcc PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/adpcc/rk_aiq_algo_adpcc_itf.cpp. Using lib")
	add_library(rkaiq_adpcc STATIC IMPORTED)
	set_target_properties(rkaiq_adpcc PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_adpcc.${SUFFIX}")
	set_target_properties(rkaiq_adpcc PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_adpcc.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add afec implement
# ----------------------------------------------------------------------------
set (AFEC_SRC
	afec/rk_aiq_algo_afec_itf.cpp
	afec/rk_aiq_uapi_afec_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/afec/rk_aiq_algo_afec_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_afec STATIC ${AFEC_SRC})
	set_target_properties(rkaiq_afec PROPERTIES FOLDER "algos/afec")
	set_target_properties(rkaiq_afec PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_afec PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_afec PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/afec/rk_aiq_algo_afec_itf.cpp. Using lib")
	add_library(rkaiq_afec STATIC IMPORTED)
	set_target_properties(rkaiq_afec PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_afec.${SUFFIX}")
	set_target_properties(rkaiq_afec PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_afec.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add agamma implement
# ----------------------------------------------------------------------------
set (AGAMMA_SRC
	agamma/rk_aiq_algo_agamma_itf.cpp
	agamma/rk_aiq_agamma_algo.cpp
    agamma/rk_aiq_uapi_agamma_int.cpp

    )


if(EXISTS "${CMAKE_SOURCE_DIR}/algos/agamma/rk_aiq_algo_agamma_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_agamma STATIC ${AGAMMA_SRC})
	set_target_properties(rkaiq_agamma PROPERTIES FOLDER "algos/agamma")
	set_target_properties(rkaiq_agamma PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_agamma PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_agamma PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/agamma/rk_aiq_algo_agamma_itf.cpp. Using lib")
	add_library(rkaiq_agamma STATIC IMPORTED)
	set_target_properties(rkaiq_agamma PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_agamma.${SUFFIX}")
	set_target_properties(rkaiq_agamma PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_agamma.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add adegamma implement
# ----------------------------------------------------------------------------
set (ADEGAMMA_SRC
	adegamma/rk_aiq_algo_adegamma_itf.cpp
	adegamma/rk_aiq_adegamma_algo.cpp
    adegamma/rk_aiq_uapi_adegamma_int.cpp
    )


if(EXISTS "${CMAKE_SOURCE_DIR}/algos/adegamma/rk_aiq_algo_adegamma_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_adegamma STATIC ${ADEGAMMA_SRC})
	set_target_properties(rkaiq_adegamma PROPERTIES FOLDER "algos/adegamma")
	set_target_properties(rkaiq_adegamma PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_adegamma PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_adegamma PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found 
	${CMAKE_SOURCE_DIR}/algos/adegamma/rk_aiq_algo_adegamma_itf.cpp. Using lib")
	add_library(rkaiq_adegamma STATIC IMPORTED)
	set_target_properties(rkaiq_adegamma PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_adegamma.${SUFFIX}")
	set_target_properties(rkaiq_adegamma PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_adegamma.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add agic implement
# ----------------------------------------------------------------------------
set (AGIC_SRC
	agic/rk_aiq_algo_agic_itf.cpp
	agic/rk_aiq_algo_agic.cpp
	agic/rk_aiq_uapi_agic_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/agic/rk_aiq_algo_agic_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_agic STATIC ${AGIC_SRC})
	set_target_properties(rkaiq_agic PROPERTIES FOLDER "algos/agic")
	set_target_properties(rkaiq_agic PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_agic PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_agic PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/agic/rk_aiq_algo_agic_itf.cpp. Using lib")
	add_library(rkaiq_agic STATIC IMPORTED)
	set_target_properties(rkaiq_agic PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_agic.${SUFFIX}")
	set_target_properties(rkaiq_agic PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_agic.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add aie implement
# ----------------------------------------------------------------------------
set (AIE_SRC
	aie/rk_aiq_algo_aie_itf.cpp
	aie/rk_aiq_algo_aie_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aie/rk_aiq_algo_aie_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_aie STATIC ${AIE_SRC})
	set_target_properties(rkaiq_aie PROPERTIES FOLDER "algos/aie")
	set_target_properties(rkaiq_aie PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aie PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aie PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aie/rk_aiq_algo_aie_itf.cpp. Using lib")
	add_library(rkaiq_aie STATIC IMPORTED)
	set_target_properties(rkaiq_aie PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aie.${SUFFIX}")
	set_target_properties(rkaiq_aie PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aie.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add aldch implement
# ----------------------------------------------------------------------------
set (ALDCH_SRC
	aldch/rk_aiq_algo_aldch_itf.cpp
	aldch/rk_aiq_uapi_aldch_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aldch/rk_aiq_algo_aldch_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_aldch STATIC ${ALDCH_SRC})
	set_target_properties(rkaiq_aldch PROPERTIES FOLDER "algos/aldch")
	set_target_properties(rkaiq_aldch PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aldch PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aldch PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aldch/rk_aiq_algo_aldch_itf.cpp. Using lib")
	add_library(rkaiq_aldch STATIC IMPORTED)
	set_target_properties(rkaiq_aldch PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aldch.${SUFFIX}")
	set_target_properties(rkaiq_aldch PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aldch.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add alsc implement
# ----------------------------------------------------------------------------
set (ALSC_SRC
	alsc/rk_aiq_algo_alsc_itf.cpp
	alsc/rk_aiq_alsc_algo.cpp
	alsc/rk_aiq_uapi_alsc_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/alsc/rk_aiq_algo_alsc_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_alsc STATIC ${ALSC_SRC})
	set_target_properties(rkaiq_alsc PROPERTIES FOLDER "algos/alsc")
	set_target_properties(rkaiq_alsc PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_alsc PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_alsc PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/alsc/rk_aiq_algo_alsc_itf.cpp. Using lib")
	add_library(rkaiq_alsc STATIC IMPORTED)
	set_target_properties(rkaiq_alsc PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_alsc.${SUFFIX}")
	set_target_properties(rkaiq_alsc PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_alsc.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add aorb implement
# ----------------------------------------------------------------------------
set (AORB_SRC
	aorb/rk_aiq_algo_aorb_itf.cpp
    aorb/orb_algos.cpp
    aorb/orb_algos_opencv.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aorb/rk_aiq_algo_aorb_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_aorb STATIC ${AORB_SRC})
	set_target_properties(rkaiq_aorb PROPERTIES FOLDER "algos/aorb")
	set_target_properties(rkaiq_aorb PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aorb PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aorb PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aorb/rk_aiq_algo_aorb_itf.cpp. Using lib")
	add_library(rkaiq_aorb STATIC IMPORTED)
	set_target_properties(rkaiq_aorb PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aorb.${SUFFIX}")
	set_target_properties(rkaiq_aorb PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aorb.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add ar2y implement
# ----------------------------------------------------------------------------
set (AR2Y_SRC
	ar2y/rk_aiq_algo_ar2y_itf.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/ar2y/rk_aiq_algo_ar2y_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_ar2y STATIC ${AR2Y_SRC})
	set_target_properties(rkaiq_ar2y PROPERTIES FOLDER "algos/ar2y")
	set_target_properties(rkaiq_ar2y PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_ar2y PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_ar2y PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/ar2y/rk_aiq_algo_ar2y_itf.cpp. Using lib")
	add_library(rkaiq_ar2y STATIC IMPORTED)
	set_target_properties(rkaiq_ar2y PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_ar2y.${SUFFIX}")
	set_target_properties(rkaiq_ar2y PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_ar2y.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add awdr implement
# ----------------------------------------------------------------------------
set (AWDR_SRC
	awdr/rk_aiq_algo_awdr_itf.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/awdr/rk_aiq_algo_awdr_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_awdr STATIC ${AWDR_SRC})
	set_target_properties(rkaiq_awdr PROPERTIES FOLDER "algos/awdr")
	set_target_properties(rkaiq_awdr PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_awdr PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_awdr PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/awdr/rk_aiq_algo_awdr_itf.cpp. Using lib")
	add_library(rkaiq_awdr STATIC IMPORTED)
	set_target_properties(rkaiq_awdr PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_awdr.${SUFFIX}")
	set_target_properties(rkaiq_awdr PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_awdr.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add asharp3 implement
# ----------------------------------------------------------------------------
set (ASHARP_V3_SRC
	asharp3/rk_aiq_asharp_algo_itf_v3.cpp
	asharp3/rk_aiq_asharp_algo_sharp_v3.cpp
	asharp3/rk_aiq_asharp_algo_v3.cpp
	asharp3/rk_aiq_uapi_asharp_int_v3.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/asharp3/rk_aiq_asharp_algo_itf_v3.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_asharp_v3 STATIC ${ASHARP_V3_SRC})
	set_target_properties(rkaiq_asharp_v3 PROPERTIES FOLDER "algos/asharp3")
	set_target_properties(rkaiq_asharp_v3 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_asharp_v3 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_asharp_v3 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/asharp3/rk_aiq_asharp_algo_itf_v3.cpp. Using lib")
	add_library(rkaiq_asharp_v3 STATIC IMPORTED)
	set_target_properties(rkaiq_asharp_v3 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_asharp_v3.${SUFFIX}")
	set_target_properties(rkaiq_asharp_v3 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_asharp_v3.${SUFFIX}")
endif()


# ----------------------------------------------------------------------------
# add aynr2 implement
# ----------------------------------------------------------------------------
set (AYNR_V2_SRC
	aynr2/rk_aiq_aynr_algo_itf_v2.cpp
	aynr2/rk_aiq_aynr_algo_ynr_v2.cpp
	aynr2/rk_aiq_aynr_algo_v2.cpp
	aynr2/rk_aiq_uapi_aynr_int_v2.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aynr2/rk_aiq_aynr_algo_itf_v2.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_aynr_v2 STATIC ${AYNR_V2_SRC})
	set_target_properties(rkaiq_aynr_v2 PROPERTIES FOLDER "algos/aynr2")
	set_target_properties(rkaiq_aynr_v2 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aynr_v2 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aynr_v2 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aynr2/rk_aiq_aynr_algo_itf_v2.cpp. Using lib")
	add_library(rkaiq_aynr_v2 STATIC IMPORTED)
	set_target_properties(rkaiq_aynr_v2 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aynr_v2.${SUFFIX}")
	set_target_properties(rkaiq_aynr_v2 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aynr_v2.${SUFFIX}")
endif()



# ----------------------------------------------------------------------------
# add acnr implement
# ----------------------------------------------------------------------------
set (ACNR_V1_SRC
	acnr/rk_aiq_acnr_algo_itf_v1.cpp
	acnr/rk_aiq_acnr_algo_cnr_v1.cpp
	acnr/rk_aiq_acnr_algo_v1.cpp
	acnr/rk_aiq_uapi_acnr_int_v1.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/acnr/rk_aiq_acnr_algo_itf_v1.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_acnr_v1 STATIC ${ACNR_V1_SRC})
	set_target_properties(rkaiq_acnr_v1 PROPERTIES FOLDER "algos/acnr")
	set_target_properties(rkaiq_acnr_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_acnr_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_acnr_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/acnr/rk_aiq_acnr_algo_itf_v1.cpp. Using lib")
	add_library(rkaiq_acnr_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_acnr_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_acnr_v1.${SUFFIX}")
	set_target_properties(rkaiq_acnr_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_acnr_v1.${SUFFIX}")
endif()


# ----------------------------------------------------------------------------
# add arawnr2 implement
# ----------------------------------------------------------------------------
set (ARAWNR_V2_SRC
	arawnr2/rk_aiq_abayernr_algo_itf_v2.cpp
	arawnr2/rk_aiq_abayernr_algo_bayernr_v2.cpp
	arawnr2/rk_aiq_abayernr_algo_v2.cpp
	arawnr2/rk_aiq_uapi_abayernr_int_v2.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/arawnr2/rk_aiq_abayernr_algo_itf_v2.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_arawnr_v2 STATIC ${ARAWNR_V2_SRC})
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES FOLDER "algos/arawnr2")
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/arawnr2/rk_aiq_abayernr_algo_itf_v2.cpp. Using lib")
	add_library(rkaiq_arawnr_v2 STATIC IMPORTED)
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_arawnr_v2.${SUFFIX}")
	set_target_properties(rkaiq_arawnr_v2 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_arawnr_v2.${SUFFIX}")
endif()

set(AEIS_SRC
    aeis/eis_loader.cpp
    aeis/scaler_service.cpp
    aeis/remap_backend.cpp
    aeis/imu_service.cpp
    aeis/eis_algo_service.cpp
    aeis/rk_aiq_uapi_aeis_int.cpp
    aeis/rk_aiq_algo_aeis_itf.cpp)

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aeis/rk_aiq_algo_aeis_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
    add_library(rkaiq_aeis STATIC ${AEIS_SRC})
    set_target_properties(rkaiq_aeis PROPERTIES FOLDER "algos/aeis")
	set_target_properties(rkaiq_aeis PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aeis PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aeis PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aeis/rk_aiq_algo_aeis_itf.cpp. Using lib")
	add_library(rkaiq_aeis STATIC IMPORTED)
	set_target_properties(rkaiq_aeis PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aeis.${SUFFIX}")
	set_target_properties(rkaiq_aeis PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aeis.${SUFFIX}")
endif()



add_library(dvs_gyro SHARED IMPORTED GLOBAL)
set_target_properties(dvs_gyro PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/algos/aeis/lib/libdvs_gyro.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/algos/aeis")

# ----------------------------------------------------------------------------
# add amd implement
# ----------------------------------------------------------------------------
set (AMD_SRC
	amd/rk_aiq_algo_amd_itf.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/amd/rk_aiq_algo_amd_itf.cpp")
    file(COPY ${CMAKE_SOURCE_DIR}/algos/amd/md_lib/librkaiq_tnr_md.a 	DESTINATION ${LIBRARY_OUTPUT_PATH})
    add_library(rkaiq_tnr_md STATIC IMPORTED)
    set_target_properties(rkaiq_tnr_md PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkaiq_tnr_md.${SUFFIX}")
    set_target_properties(rkaiq_tnr_md PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkaiq_tnr_md.${SUFFIX}")

	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_amd STATIC ${AMD_SRC})
	set_target_properties(rkaiq_amd PROPERTIES FOLDER "algos/amd")
	set_target_properties(rkaiq_amd PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_amd PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_amd PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/amd/rk_aiq_amd_algo_itf.cpp. Using lib")
	add_library(rkaiq_amd STATIC IMPORTED)
	set_target_properties(rkaiq_amd PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_amd.${SUFFIX}")
	set_target_properties(rkaiq_amd PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_amd.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add arawnr1 implement
# ----------------------------------------------------------------------------
set (ARAWNR_V1_SRC
	arawnr/rk_aiq_abayernr_algo_itf_v1.cpp
	arawnr/rk_aiq_abayernr_algo_bayernr_v1.cpp
	arawnr/rk_aiq_abayernr_algo_v1.cpp
	arawnr/rk_aiq_uapi_abayernr_int_v1.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/arawnr/rk_aiq_abayernr_algo_itf_v1.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_arawnr_v1 STATIC ${ARAWNR_V1_SRC})
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES FOLDER "algos/arawnr")
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/arawnr/rk_aiq_abayernr_algo_itf_v1.cpp. Using lib")
	add_library(rkaiq_arawnr_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_arawnr_v1.${SUFFIX}")
	set_target_properties(rkaiq_arawnr_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_arawnr_v1.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add aynr1 implement
# ----------------------------------------------------------------------------
set (AYNR_V1_SRC
	aynr/rk_aiq_aynr_algo_itf_v1.cpp
	aynr/rk_aiq_aynr_algo_ynr_v1.cpp
	aynr/rk_aiq_aynr_algo_v1.cpp
	aynr/rk_aiq_uapi_aynr_int_v1.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/aynr/rk_aiq_aynr_algo_itf_v1.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_aynr_v1 STATIC ${AYNR_V1_SRC})
	set_target_properties(rkaiq_aynr_v1 PROPERTIES FOLDER "algos/aynr")
	set_target_properties(rkaiq_aynr_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_aynr_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_aynr_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/aynr/rk_aiq_aynr_algo_itf_v1.cpp. Using lib")
	add_library(rkaiq_aynr_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_aynr_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_aynr_v1.${SUFFIX}")
	set_target_properties(rkaiq_aynr_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_aynr_v1.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add auvnr1 implement
# ----------------------------------------------------------------------------
set (AUVNR_V1_SRC
	auvnr/rk_aiq_auvnr_algo_itf_v1.cpp
	auvnr/rk_aiq_auvnr_algo_uvnr_v1.cpp
	auvnr/rk_aiq_auvnr_algo_v1.cpp
	auvnr/rk_aiq_uapi_auvnr_int_v1.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/auvnr/rk_aiq_auvnr_algo_itf_v1.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_auvnr_v1 STATIC ${AUVNR_V1_SRC})
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES FOLDER "algos/auvnr")
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/auvnr/rk_aiq_auvnr_algo_itf_v1.cpp. Using lib")
	add_library(rkaiq_auvnr_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_auvnr_v1.${SUFFIX}")
	set_target_properties(rkaiq_auvnr_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_auvnr_v1.${SUFFIX}")
endif()

# ----------------------------------------------------------------------------
# add amfnr v1 implement
# ----------------------------------------------------------------------------
set (AMFNR_V1_SRC
	amfnr/rk_aiq_amfnr_algo_itf_v1.cpp
	amfnr/rk_aiq_amfnr_algo_mfnr_v1.cpp
	amfnr/rk_aiq_amfnr_algo_v1.cpp
	amfnr/rk_aiq_uapi_amfnr_int_v1.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/amfnr/rk_aiq_amfnr_algo_itf_v1.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_amfnr_v1 STATIC ${AMFNR_V1_SRC})
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES FOLDER "algos/amfnr")
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/amfnr/rk_aiq_amfnr_algo_itf_v1.cpp. Using lib")
	add_library(rkaiq_amfnr_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_amfnr_v1.${SUFFIX}")
	set_target_properties(rkaiq_amfnr_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_amfnr_v1.${SUFFIX}")
endif()


# ----------------------------------------------------------------------------
# add again v1 implement
# ----------------------------------------------------------------------------
set (AGAIN_V1_SRC
	again/rk_aiq_again_algo_itf.cpp
	again/rk_aiq_again_algo_gain.cpp
	again/rk_aiq_again_algo.cpp
	again/rk_aiq_uapi_again_int.cpp
    )

if(EXISTS "${CMAKE_SOURCE_DIR}/algos/again/rk_aiq_again_algo_itf.cpp")
	message(${LIBRARY_OUTPUT_PATH})
	add_library(rkaiq_again_v1 STATIC ${AGAIN_V1_SRC})
	set_target_properties(rkaiq_again_v1 PROPERTIES FOLDER "algos/again")
	set_target_properties(rkaiq_again_v1 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(rkaiq_again_v1 PROPERTIES C_VISIBILITY_PRESET default)
	set_target_properties(rkaiq_again_v1 PROPERTIES CXX_VISIBILITY_PRESET default)
else()
	message(STATUS "Can NOT found ${CMAKE_SOURCE_DIR}/algos/again/rk_aiq_again_algo_itf.cpp. Using lib")
	add_library(rkaiq_again_v1 STATIC IMPORTED)
	set_target_properties(rkaiq_again_v1 PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_again_v1.${SUFFIX}")
	set_target_properties(rkaiq_again_v1 PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_again_v1.${SUFFIX}")
endif()

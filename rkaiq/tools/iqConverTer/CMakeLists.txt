cmake_minimum_required(VERSION 2.8.0...3.22)

project(iqconvert)

set(ROOT_PROJ_DIR ${PROJECT_SOURCE_DIR}/../..)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
add_compile_options(-fPIC -Wall -Wno-error -O0 -g)
add_compile_options(-Wno-unused-variable
					-Wno-unused-but-set-variable
					-Wno-unused-function
					-Wno-misleading-indentation
					-Wno-unused-label
					-Wno-format-truncation
					-fno-strict-aliasing)


include_directories(
    ${ROOT_PROJ_DIR}
    ${ROOT_PROJ_DIR}/iq_parser
    ${ROOT_PROJ_DIR}/iq_parser_v2
    ${ROOT_PROJ_DIR}/xcore
    ${ROOT_PROJ_DIR}/xcore/base
    ${ROOT_PROJ_DIR}/include/iq_parser
    ${ROOT_PROJ_DIR}/iq_parser
    ${ROOT_PROJ_DIR}/iq_parser_v2
    ${ROOT_PROJ_DIR}/include/algos
    ${ROOT_PROJ_DIR}/include/xcore
    ${ROOT_PROJ_DIR}/include/xcore/base
    ${ROOT_PROJ_DIR}/include/common
    ${ROOT_PROJ_DIR}/include/uAPI
    )

set(CALIBV1_SRC
    ${ROOT_PROJ_DIR}/iq_parser_v2/awb_xml2json.cpp
    ${ROOT_PROJ_DIR}/iq_parser/RkAiqCalibDb.cpp
    ${ROOT_PROJ_DIR}/iq_parser/RkAiqCalibApi.cpp
    ${ROOT_PROJ_DIR}/iq_parser/RkAiqCalibParser.cpp
    ${ROOT_PROJ_DIR}/iq_parser/RkAiqCalibTag.cpp
    ${ROOT_PROJ_DIR}/iq_parser/RkAiqCalibApi.cpp
    ${ROOT_PROJ_DIR}/iq_parser/tinyxml2.cpp
    ${ROOT_PROJ_DIR}/iq_parser/xmltags.cpp
    ${ROOT_PROJ_DIR}/xcore/xcam_log.cpp
    )

set(CALIBV2_SRC
    ${ROOT_PROJ_DIR}/iq_parser_v2/aec_xml2json.cpp
    ${ROOT_PROJ_DIR}/iq_parser_v2/ccm_xml2json.cpp
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON_Utils.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/j2s.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/j2s_utils.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/RkAiqCalibDbV2.cpp
    ${ROOT_PROJ_DIR}/iq_parser_v2/bayernr_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/mfnr_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/uvnr_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/ynr_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/sharp_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/edgefilter_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/bayernr_xml2json_v2.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/cnr_xml2json_v1.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/ynr_xml2json_v2.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/sharp_xml2json_v3.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/adrc_xml2json.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/af_xml2json.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/adehaze_xml2json.cpp
	${ROOT_PROJ_DIR}/iq_parser_v2/lut3d_xml2json.cpp
    )

include_directories(${ROOT_PROJ_DIR}/include)
include_directories(${ROOT_PROJ_DIR}/include/common)
include_directories(${ROOT_PROJ_DIR}/include/iq_parser_v2)
include_directories(${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_compile_options(${ISP_HW_VERSION})

#########################################################
#         JSON <---> STRUCT  PREPROCESS                 #
#########################################################
add_custom_target(
    j2s_parser
    COMMAND ${CMAKE_C_COMPILER}
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h -E -C -o .temp.h ${ISP_HW_VERSION}
    -I${ROOT_PROJ_DIR}/include/common
    -I${ROOT_PROJ_DIR}/include/uAPI
    -I${ROOT_PROJ_DIR}/include/algos
    COMMAND ${PROJECT_SOURCE_DIR}/script/header_strip.sh
    .temp.h output.h
    COMMAND ${PROJECT_SOURCE_DIR}/script/headerprocess.sh
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    COMMAND ${PROJECT_SOURCE_DIR}/bin/parser output.h >>
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    )

#########################################################
#                    BUILD LIBRARY                      #
#########################################################
add_library(iqconvert STATIC ${CALIBV1_SRC} ${CALIBV2_SRC})
add_dependencies(iqconvert j2s_parser)

#########################################################
#                    BUILD TOOL                         #
#########################################################
add_executable(iqConverTer main.cpp src/iqconverter.cpp)
target_link_libraries(iqConverTer iqconvert)

install(TARGETS iqConverTer DESTINATION bin)


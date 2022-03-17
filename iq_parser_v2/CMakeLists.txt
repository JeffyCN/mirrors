cmake_minimum_required(VERSION 2.8)

set(J2S_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON_Utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s_utils.c
    )

#########################################################
#               HEADER MACRO PREPROCESS                 #
#########################################################
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.h
    COMMAND ${CMAKE_C_COMPILER}
    ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h -E -C -o .temp.h ${ISP_HW_VERSION}
    -I${CMAKE_SOURCE_DIR}/
    -I${CMAKE_SOURCE_DIR}/include
    -I${CMAKE_SOURCE_DIR}/include/common
    -I${CMAKE_SOURCE_DIR}/include/uAPI2
    -I${CMAKE_SOURCE_DIR}/include/algos
    -I${CMAKE_SOURCE_DIR}/include/iq_parser_v2
    -I${CMAKE_SOURCE_DIR}/include/iq_parser
    -I${CMAKE_SOURCE_DIR}/include/xcore
    -I${CMAKE_SOURCE_DIR}/include/algos
    COMMAND ${CMAKE_SOURCE_DIR}/tools/iq_check/script/header_strip.sh
    .temp.h output.h
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RkAiqCalibDbTypesV2_M4.h
    COMMAND cat ${CMAKE_SOURCE_DIR}/tools/iq_check/script/m4/desc.m4
    output.h > RkAiqCalibDbTypesV2.m4
    COMMAND cp ${CMAKE_SOURCE_DIR}/tools/iq_check/script/m4/desc.sh ./
    COMMAND m4 -E RkAiqCalibDbTypesV2.m4 > RkAiqCalibDbTypesV2_M4.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output.h
)

#########################################################
#                BUILD HOST PARSER tools                 #
#########################################################
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    COMMAND gcc -DJ2S_ENABLE_DESC ${CMAKE_SOURCE_DIR}/tools/iq_check/src/parser.c -o parser ${ISP_HW_VERSION}
    -I${CMAKE_SOURCE_DIR}/include/common
    -I${CMAKE_SOURCE_DIR}/
    -I${CMAKE_SOURCE_DIR}/include/uAPI2
    -I${CMAKE_SOURCE_DIR}/include/algos
    -I${CMAKE_SOURCE_DIR}/include/iq_parser_v2
    -I${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s
    COMMAND ${CMAKE_SOURCE_DIR}/tools/iq_check/script/headerprocess.sh
    ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h
    ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/iq_check/bin/parser RkAiqCalibDbTypesV2_M4.h CamCalibDbProj_t >>
    ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RkAiqCalibDbTypesV2_M4.h
    )

#########################################################
#                    BUILD HOST DUMP tool               #
#########################################################
add_custom_target(
    j2s_parser
    COMMAND gcc -DJ2S_ENABLE_DESC -o dump ${J2S_SRC} ${ISP_HW_VERSION}
    ${CMAKE_SOURCE_DIR}/tools/iq_check/src/dump.c
    -I${CMAKE_SOURCE_DIR}/include/common
    -I${CMAKE_SOURCE_DIR}/
    -I${CMAKE_SOURCE_DIR}/include/uAPI2
    -I${CMAKE_SOURCE_DIR}/include/algos
    -I${CMAKE_SOURCE_DIR}/include/iq_parser_v2
    -I${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s

    COMMAND ./dump --template > .j2s_template.json
    COMMAND cp ${CMAKE_SOURCE_DIR}/tools/iq_check/script/dump_template.sh ./
    COMMAND ./dump_template.sh
    ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s/j2s_generated_template_json.h
    DEPENDS ${CMAKE_SOURCE_DIR}/include/iq_parser_v2/j2s/j2s_generated.h
    )

SET(IQ_PARSERV2_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON_Utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/RkAiqCalibDbV2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/scene_manager.cpp
    )

SET(RK_AIQ_SRC ${RK_AIQ_SRC} ${IQ_PARSERV2_SRC} PARENT_SCOPE)

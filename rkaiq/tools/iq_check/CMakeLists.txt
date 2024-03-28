cmake_minimum_required(VERSION 2.8...3.22)

if (ARCH STREQUAL "arm")
    set(J2S_HOST_CFLAGS -m32 -std=gnu99 -malign-double)
elseif (ARCH STREQUAL "aarch64")
    set(J2S_HOST_CFLAGS -m64 -std=gnu99 -malign-double)
endif()

project(j2s)

# ├── iq_check.c
# ├── include
# │   ├── cJSON.h
# │   ├── cJSON_Utils.h
# │   ├── common.h
# │   ├── j2s_generated.h
# │   └── j2s.h
# ├── input.h
# ├── j2s
# │   ├── cJSON.c
# │   ├── cJSON_Utils.c
# │   ├── j2s.c
# │   └── j2s_utils.c

set(ROOT_PROJ_DIR ${PROJECT_SOURCE_DIR}/../..)

set(J2S_SRC
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON_Utils.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/j2s.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/j2s_utils.c
)

include_directories(${CMAKE_SYSROOT}/usr/includ)
include_directories(${ROOT_PROJ_DIR}/include)
include_directories(${ROOT_PROJ_DIR}/include/common)
include_directories(${ROOT_PROJ_DIR}/include/iq_parser_v2)
include_directories(${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s)
include_directories(${ROOT_PROJ_DIR}/)

add_compile_options(${ISP_HW_VERSION})

#########################################################
#               HEADER MACRO PREPROCESS                 #
#########################################################
add_custom_target(
    j2s_parser
    COMMAND ${CMAKE_C_COMPILER}
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h -E -C -o .temp.h ${ISP_HW_VERSION}
    -I${ROOT_PROJ_DIR}/include/common
    -I${ROOT_PROJ_DIR}/include/iq_parser_v2
    -I${ROOT_PROJ_DIR}/include/iq_parser
    COMMAND ${PROJECT_SOURCE_DIR}/script/header_strip.sh
    .temp.h output.h

    COMMAND cat ${PROJECT_SOURCE_DIR}/script/m4/desc.m4
    output.h > RkAiqCalibDbTypesV2.m4
    COMMAND cp ${PROJECT_SOURCE_DIR}/script/m4/desc.sh ./
    COMMAND m4 -E RkAiqCalibDbTypesV2.m4 > RkAiqCalibDbTypesV2_M4.h
)

#########################################################
#                BUILD HOST PARSER TOOL                 #
#########################################################
add_custom_target(
    parser_tool
    COMMAND gcc ${J2S_HOST_CFLAGS} -std=c99 -DJ2S_ENABLE_DESC ${PROJECT_SOURCE_DIR}/src/parser.c -o parser ${ISP_HW_VERSION}
    -I${ROOT_PROJ_DIR}/include/common
    -I${ROOT_PROJ_DIR}/include/iq_parser_v2
    -I${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s
    COMMAND ${PROJECT_SOURCE_DIR}/script/headerprocess.sh
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h
    ${CMAKE_BINARY_DIR}/j2s_generated.h
    COMMAND ${PROJECT_SOURCE_DIR}/bin/parser RkAiqCalibDbTypesV2_M4.h >>
    ${CMAKE_BINARY_DIR}/j2s_generated.h
    )
add_dependencies(parser_tool j2s_parser)

#########################################################
#                    BUILD LIBRARY                      #
#########################################################
add_library(j2s STATIC ${J2S_SRC})
add_dependencies(j2s j2s_parser)

#########################################################
#                    BUILD HOST DUMP TOOL               #
#########################################################
add_custom_target(
    dump_tool
    COMMAND gcc ${J2S_HOST_CFLAGS} -DJ2S_ENABLE_DESC ${ISP_HW_VERSION} -o dump ${J2S_SRC} ${ISP_HW_VERSION}
    ${PROJECT_SOURCE_DIR}/src/dump.c
    -I${ROOT_PROJ_DIR}/include/common
    -I${ROOT_PROJ_DIR}/include/iq_parser_v2
    -I${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s

    COMMAND ./dump --template > .j2s_template.json
    COMMAND cp ${PROJECT_SOURCE_DIR}/script/dump_template.sh ./
    COMMAND ./dump_template.sh
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/j2s_generated_template_json.h
    )
add_dependencies(dump_tool parser_tool)

#########################################################
#                    BUILD TOOL                         #
#########################################################
add_executable(iq_check iq_check.c)
target_link_libraries(iq_check j2s)
add_dependencies(iq_check dump_tool)

install(TARGETS j2s DESTINATION lib)
install(TARGETS iq_check DESTINATION bin)


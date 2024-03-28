cmake_minimum_required(VERSION 2.8...3.22)

if (ARCH STREQUAL "arm")
    set(J2S_HOST_CFLAGS -m32 -std=gnu99 -malign-double)
elseif (ARCH STREQUAL "aarch64")
    set(J2S_HOST_CFLAGS -m64 -std=gnu99 -malign-double)
endif()

set(J2S_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON_Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s_utils.cpp
    )

if (RKAIQ_NEWSTRUCT_TEST)
	set(DEFINE_USE_NEWSTRUCT -DUSE_NEWSTRUCT=1 CACHE INTERNAL "Build with new structure")
endif()

#########################################################
#               HEADER MACRO PREPROCESS                 #
#########################################################
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.h
    COMMAND ${CMAKE_C_COMPILER}
    ${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h -E -C -o .temp.h ${ISP_HW_VERSION} ${DEFINE_USE_NEWSTRUCT}
    -I${rk_aiq_SOURCE_DIR}/
    -I${rk_aiq_SOURCE_DIR}/include
    -I${rk_aiq_SOURCE_DIR}/include/common
    -I${rk_aiq_SOURCE_DIR}/include/uAPI2
    -I${rk_aiq_SOURCE_DIR}/include/algos
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser
    -I${rk_aiq_SOURCE_DIR}/include/xcore
    -I${rk_aiq_SOURCE_DIR}/include/algos
    COMMAND ${rk_aiq_SOURCE_DIR}/tools/iq_check/script/header_strip.sh
    .temp.h output.h
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RkAiqCalibDbTypesV2_M4.h
    COMMAND cat ${rk_aiq_SOURCE_DIR}/tools/iq_check/script/m4/desc.m4
    output.h > RkAiqCalibDbTypesV2.m4
    COMMAND cp ${rk_aiq_SOURCE_DIR}/tools/iq_check/script/m4/desc.sh ./
    COMMAND m4 -E RkAiqCalibDbTypesV2.m4 > RkAiqCalibDbTypesV2_M4.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output.h
)

#########################################################
#                BUILD HOST PARSER tools                 #
#########################################################
add_custom_command(
    OUTPUT ${rk_aiq_BINARY_DIR}/.j2s_generated.h
    COMMAND gcc ${J2S_HOST_CFLAGS} -DJ2S_ENABLE_DESC ${rk_aiq_SOURCE_DIR}/tools/iq_check/src/parser.c -o parser ${ISP_HW_VERSION} ${DEFINE_USE_NEWSTRUCT}
    -I${rk_aiq_SOURCE_DIR}/include/common
    -I${rk_aiq_SOURCE_DIR}/
    -I${rk_aiq_SOURCE_DIR}/include/uAPI2
    -I${rk_aiq_SOURCE_DIR}/include/algos
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/j2s
    COMMAND ${rk_aiq_SOURCE_DIR}/tools/iq_check/script/headerprocess.sh
    ${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/RkAiqCalibDbTypesV2.h
    ${rk_aiq_BINARY_DIR}/.j2s_generated.h
	#COMMAND ${rk_aiq_SOURCE_DIR}/tools/iq_check/bin/parser RkAiqCalibDbTypesV2_M4.h CamCalibDbProj_t >>
	COMMAND ${rk_aiq_BINARY_DIR}/iq_parser_v2/parser RkAiqCalibDbTypesV2_M4.h CamCalibDbProj_t >>
    ${rk_aiq_BINARY_DIR}/.j2s_generated.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RkAiqCalibDbTypesV2_M4.h
    )

#########################################################
#                    BUILD HOST CODE2BIN tool               #
#########################################################
add_custom_target(
    j2s_code2bin
    COMMAND gcc ${J2S_HOST_CFLAGS}
	${rk_aiq_SOURCE_DIR}/tools/iq_check/src/tobin.c -o tobin ${ISP_HW_VERSION} -DRKAIQ_HAVE_LSC_V3 ${DEFINE_USE_NEWSTRUCT}

	-I${rk_aiq_BINARY_DIR}
    -I${rk_aiq_SOURCE_DIR}/include/common
    -I${rk_aiq_SOURCE_DIR}/
    -I${rk_aiq_SOURCE_DIR}/include/uAPI2
    -I${rk_aiq_SOURCE_DIR}/include
    -I${rk_aiq_SOURCE_DIR}/include/algos
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/j2s

	COMMAND ./tobin ${rk_aiq_BINARY_DIR}/j2s_code2bin.bin
	COMMAND cd ${rk_aiq_BINARY_DIR}/ && xxd -i j2s_code2bin.bin j2s_code2bin.h
    DEPENDS ${rk_aiq_BINARY_DIR}/.j2s_generated.h
    )

#########################################################
#                    BUILD HOST j2s_generated.h optimize tool               #
#########################################################
add_custom_target(
    j2s_generated_optim
    COMMAND gcc ${J2S_HOST_CFLAGS} ${rk_aiq_SOURCE_DIR}/tools/iq_check/src/j2s_generated_optim.c -o j2s_generated_optim ${ISP_HW_VERSION} ${DEFINE_USE_NEWSTRUCT}

	-I${rk_aiq_BINARY_DIR}
    -I${rk_aiq_SOURCE_DIR}/include/common
    -I${rk_aiq_SOURCE_DIR}/
    -I${rk_aiq_SOURCE_DIR}/include
    -I${rk_aiq_SOURCE_DIR}/include/uAPI2
    -I${rk_aiq_SOURCE_DIR}/include/algos
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/j2s

	COMMAND ./j2s_generated_optim ${rk_aiq_BINARY_DIR}/j2s_generated.h
    DEPENDS j2s_code2bin
    )

#########################################################
#                    BUILD HOST DUMP tool               #
#########################################################
add_custom_target(
    j2s_parser
	COMMAND g++ -std=gnu++11 -DJ2S_ENABLE_DESC -DCOMPILE_TEMPLATE -o dump ${J2S_SRC} ${ISP_HW_VERSION} ${DEFINE_USE_NEWSTRUCT}
    ${rk_aiq_SOURCE_DIR}/tools/iq_check/src/dump.cpp
    -I${rk_aiq_SOURCE_DIR}/include/common
    -I${rk_aiq_SOURCE_DIR}/
    -I${rk_aiq_SOURCE_DIR}/include/uAPI2
    -I${rk_aiq_SOURCE_DIR}/include
    -I${rk_aiq_SOURCE_DIR}/include/algos
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2
    -I${rk_aiq_SOURCE_DIR}/include/iq_parser_v2/j2s
    -I${rk_aiq_BINARY_DIR}/

    COMMAND ./dump --template > .j2s_template.json
    COMMAND cp ${rk_aiq_SOURCE_DIR}/tools/iq_check/script/dump_template.sh ./
    COMMAND ./dump_template.sh
    ${rk_aiq_BINARY_DIR}/j2s_generated_template_json.h
	DEPENDS ${rk_aiq_BINARY_DIR}/.j2s_generated.h
	DEPENDS j2s_generated_optim
    )

#########################################################
#                    BUILD BINARY IQ FILES              #
#########################################################
if (${ISP_HW_VERSION} STREQUAL "-DISP_HW_V21")
    set(IQFILES_DIR_INTERNAL "${rk_aiq_SOURCE_DIR}/iqfiles/isp21")
endif ()
if (${ISP_HW_VERSION} STREQUAL "-DISP_HW_V30")
    set(IQFILES_DIR_INTERNAL "${rk_aiq_SOURCE_DIR}/iqfiles/isp3x")
endif ()
if (${ISP_HW_VERSION} STREQUAL "-DISP_HW_V32")
    set(IQFILES_DIR_INTERNAL "${rk_aiq_SOURCE_DIR}/iqfiles/isp32")
endif ()
if (${ISP_HW_VERSION} STREQUAL "-DISP_HW_V32_LITE")
    set(IQFILES_DIR_INTERNAL "${rk_aiq_SOURCE_DIR}/iqfiles/isp32_lite")
endif ()
if (${ISP_HW_VERSION} STREQUAL "-DISP_HW_V39")
    set(IQFILES_DIR_INTERNAL "${rk_aiq_SOURCE_DIR}/iqfiles/isp39")
endif ()

add_custom_target(
    j2s4b
    COMMAND ${CMAKE_COMMAND}
    -DROOT_PROJ_BIN_DIR:STRING=${rk_aiq_BINARY_DIR}
    -DRKAIQ_IQFILES_DIR:STRING="${IQFILES_DIR_INTERNAL}"
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH="${rk_aiq_BINARY_DIR}/bin"
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DRKAIQ_TARGET_ARCH=${ARCH}
    -B"host_tool"
    -H"${rk_aiq_SOURCE_DIR}/tools/j2s4b"
    COMMAND ${CMAKE_COMMAND} --build host_tool
    COMMAND make -C host_tool install
	DEPENDS j2s_parser
    )

#build dev j2s4b-dev
if (RKAIQ_J2S4B_DEV)
	# if user has not define CMAKE_INSTALL_PREFIX, do not install dev_bin to
	# avoid of compile error for buildroot
	if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
		set(make_is_install make -C dev_bin)
	else ()
		set(make_is_install make -C dev_bin install)
	endif()
add_custom_target(
    j2s4b_dev
    COMMAND ${CMAKE_COMMAND}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_PROGRAM_PATH=${CMAKE_PROGRAM_PATH}
    -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
    -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
    -DRKAIQ_J2S4B_DEV=TRUE
    -DROOT_PROJ_BIN_DIR:STRING=${rk_aiq_BINARY_DIR}
    -DRKAIQ_IQFILES_DIR:STRING="${IQFILES_DIR_INTERNAL}"
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH="${rk_aiq_BINARY_DIR}/bin"
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DRKAIQ_TARGET_ARCH=${ARCH}
    -DANDROID_ABI=${ANDROID_ABI}
    -B"dev_bin"
    -H"${rk_aiq_SOURCE_DIR}/tools/j2s4b"
    COMMAND ${CMAKE_COMMAND} --build dev_bin
    COMMAND ${make_is_install}
	DEPENDS j2s_parser
    )
endif()

SET(IQ_PARSERV2_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/cJSON_Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/j2s/j2s_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RkAiqCalibDbV2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scene/scene_manager.cpp
    )

SET(RK_AIQ_SRC ${RK_AIQ_SRC} ${IQ_PARSERV2_SRC} PARENT_SCOPE)

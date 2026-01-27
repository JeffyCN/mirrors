if (${CMAKE_SYSTEM_NAME} EQUAL "")
    set(OS "linux")
else()
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS)
endif()

include(CheckGlibc)

find_package(libdrm QUIET)

if (libdrm_FOUND)
    add_compile_options(${LIBDRM_CFLAGS})
    add_compile_options(-DISPDEMO_ENABLE_DRM=1)
else()
    # use the local drm library in demo dir
    if (C_LIBRARY_NAME STREQUAL "glibc" OR C_LIBRARY_NAME STREQUAL "clang")
        if (ARCH STREQUAL "aarch64")
            set(LOCAL_DRM_LIB_DIR ${CMAKE_SOURCE_DIR}/sample/deps/lib/aarch64/)
        elseif (ARCH STREQUAL "arm")
            set(LOCAL_DRM_LIB_DIR ${CMAKE_SOURCE_DIR}/sample/deps/lib/arm/)
        endif()

        message("\n****** build sample_fec with the local drm library: ${LOCAL_DRM_LIB_DIR}\n")
    endif()
endif()

set(ISPFEC_INSTALLED_DIR ${CMAKE_SOURCE_DIR}/build/${OS}/output/${ARCH}/installed/)
set(ISPFEC_LIB_DIR ${ISPFEC_INSTALLED_DIR}/lib/)
set(ISPFEC_LIB IspFec)

set(ISPFEC_INCLUDE_DIR
	${ISPFEC_INCLUDE_DIR}
	${LIBDRM_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/sample/deps/include
	${CMAKE_SOURCE_DIR}/sample/inih
)

if (ENABLE_SAMPLE_ISPFEC)
    set (SAMPLE_FEC_SRCS
         ${CMAKE_SOURCE_DIR}/sample/sample_fec.c
         ${CMAKE_SOURCE_DIR}/sample/sample_drm_buffer.c
         ${CMAKE_SOURCE_DIR}/sample/inih/ini.c
        )

    include_directories(sample ${ISPFEC_INCLUDE_DIR})
    link_directories("${ISPFEC_LIB_DIR}" "${LOCAL_DRM_LIB_DIR}")

    add_executable(sample_fec ${SAMPLE_FEC_SRCS})
    target_link_libraries(sample_fec ${ISPFEC_LIB} drm)

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries(sample_fec -lpthread)
    endif()

    install(TARGETS sample_fec RUNTIME DESTINATION bin)
endif()

if (ENABLE_SAMPLE_ISPFEC_MULTI)
    set (SAMPLE_FEC_MULTI_SRCS
         ${CMAKE_SOURCE_DIR}/sample/sample_fec_multi.c
         ${CMAKE_SOURCE_DIR}/sample/sample_drm_buffer.c
        )

    include_directories(sample ${ISPFEC_INCLUDE_DIR})
    link_directories("${ISPFEC_LIB_DIR}" "${LOCAL_DRM_LIB_DIR}")

    add_executable(sample_fec_multi ${SAMPLE_FEC_MULTI_SRCS})
    target_link_libraries(sample_fec_multi ${ISPFEC_LIB} drm -lpthread)

    install(TARGETS sample_fec_multi RUNTIME DESTINATION bin)
endif()

if (ENABLE_SAMPLE_FEC_FOR_AVSP)
    set (SAMPLE_FOR_AVSP_FEC_SRCS
         ${CMAKE_SOURCE_DIR}/sample/sample_fec_for_avsp.c
         ${CMAKE_SOURCE_DIR}/sample/sample_drm_buffer.c
        )

    link_directories("${LOCAL_DRM_LIB_DIR}")

    add_executable(sample_fec_for_avsp ${SAMPLE_FOR_AVSP_FEC_SRCS})
    target_link_libraries(sample_fec_for_avsp drm)

    install(FILES ${CMAKE_SOURCE_DIR}/include_prv/rk-fec-config.h DESTINATION include/IspFec)
    install(TARGETS sample_fec_for_avsp RUNTIME DESTINATION bin)
endif()

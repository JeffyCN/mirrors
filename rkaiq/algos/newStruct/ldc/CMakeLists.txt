set(LDC_SRC
    algo_ldc.c
    ldc_algo_adaptor.c
    ldc_gen_mesh.c
    ldc_ldch_adaptee.c
    ldc_lut_buffer.c
    )

if (RKAIQ_HAVE_LDCV_V22)
    set(LDC_SRC
        ${LDC_SRC}
        ldc_ldcv_adaptee.c
        )
endif()

if(RKAIQ_HAVE_DUMPSYS)
    set(LDC_SRC ${LDC_SRC} algo_ldc_info.c)
endif()

dec_algo(aldc algo_ldc.c ${LDC_SRC})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    if("${ARCH}" STREQUAL "arm")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/android/arm/librkAlgoGenMesh.a	DESTINATION ${LIBRARY_OUTPUT_PATH})
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/arm/librkAlgoMapSeparate.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
    if("${ARCH}" STREQUAL "aarch64")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/android/aarch64/librkAlgoGenMesh.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/android/aarch64/librkAlgoMapSeparate.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
else ()
    if("${ARCH}" STREQUAL "arm")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/arm/librkAlgoGenMesh.a	DESTINATION ${LIBRARY_OUTPUT_PATH})
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/arm/librkAlgoMapSeparate.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
    if("${ARCH}" STREQUAL "aarch64")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/aarch64/librkAlgoGenMesh.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/aarch64/librkAlgoMapSeparate.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
endif()

add_library(rkAlgoGenMesh STATIC IMPORTED GLOBAL)
set_target_properties(rkAlgoGenMesh PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkAlgoGenMesh.${SUFFIX}")
set_target_properties(rkAlgoGenMesh PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkAlgoGenMesh.${SUFFIX}")

if (RKAIQ_HAVE_LDCV_V22)
    add_library(rkAlgoMapSeparate STATIC IMPORTED GLOBAL)
    set_target_properties(rkAlgoMapSeparate PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkAlgoMapSeparate.${SUFFIX}")
    set_target_properties(rkAlgoMapSeparate PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkAlgoMapSeparate.${SUFFIX}")
endif()

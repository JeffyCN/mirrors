set(ALDC_SRC
    rk_aiq_algo_aldc_itf.cpp
    ldc_algo_adaptor.cpp
    rk_aiq_algo_aldc_gen_mesh.cpp
    rk_aiq_algo_aldch_adaptee.cpp
    rk_aiq_algo_aldcv_adaptee.cpp
    rk_aiq_uapi_aldc_int.cpp
    )

dec_algo(aldc rk_aiq_algo_aldc_itf.cpp ${ALDC_SRC})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    if("${ARCH}" STREQUAL "arm")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/android/arm/librkAlgoGenMesh.a	DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
    if("${ARCH}" STREQUAL "aarch64")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/android/arm64/librkAlgoGenMesh.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
else ()
    if("${ARCH}" STREQUAL "arm")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/arm/librkAlgoGenMesh.a	DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
    if("${ARCH}" STREQUAL "aarch64")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/aarch64/librkAlgoGenMesh.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/gen_mesh/linux/aarch64/librkAlgoMapSeparate.a  DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
endif()

add_library(rkAlgoGenMesh STATIC IMPORTED GLOBAL)
set_target_properties(rkAlgoGenMesh PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkAlgoGenMesh.${SUFFIX}")
set_target_properties(rkAlgoGenMesh PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkAlgoGenMesh.${SUFFIX}")

add_library(rkAlgoMapSeparate STATIC IMPORTED GLOBAL)
set_target_properties(rkAlgoMapSeparate PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkAlgoMapSeparate.${SUFFIX}")
set_target_properties(rkAlgoMapSeparate PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkAlgoMapSeparate.${SUFFIX}")

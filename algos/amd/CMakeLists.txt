set(AMD_SRC
    rk_aiq_algo_amd_itf.cpp
    )

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rk_aiq_algo_amd_itf.cpp")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/md_lib/librkaiq_tnr_md.a 	DESTINATION ${LIBRARY_OUTPUT_PATH})
    add_library(rkaiq_tnr_md STATIC IMPORTED)
    set_target_properties(rkaiq_tnr_md PROPERTIES IMPORTED_LOCATION 	"${PREFIX}rkaiq_tnr_md.${SUFFIX}")
    set_target_properties(rkaiq_tnr_md PROPERTIES IMPORTED_IMPLIB 	    "${PREFIX}rkaiq_tnr_md.${SUFFIX}")

    add_library(rkaiq_amd STATIC ${AMD_SRC})
    set_target_properties(rkaiq_amd PROPERTIES FOLDER "algos/amd")
    set_target_properties(rkaiq_amd PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(rkaiq_amd PROPERTIES C_VISIBILITY_PRESET default)
    set_target_properties(rkaiq_amd PROPERTIES CXX_VISIBILITY_PRESET default)
else()
    message(STATUS "Can NOT found ${CMAKE_CURRENT_SOURCE_DIR}/rk_aiq_amd_algo_itf.cpp. Using lib")
    add_library(rkaiq_amd STATIC IMPORTED)
    set_target_properties(rkaiq_amd PROPERTIES IMPORTED_LOCATION
        "${PREFIX}rkaiq_amd.${SUFFIX}")
    set_target_properties(rkaiq_amd PROPERTIES IMPORTED_IMPLIB
        "${PREFIX}rkaiq_amd.${SUFFIX}")
endif()

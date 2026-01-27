set(AIBNR_SRC
    "algo_aibnr.c"
    )

dec_algo(aibnr algo_aibnr.c ${AIBNR_SRC})

if (RKAIQ_HAVE_AIBNR)
    if("${ARCH}" STREQUAL "arm")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/arm/librkpostbnr.so	DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
    if("${ARCH}" STREQUAL "aarch64")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib/aarch64/librkpostbnr.so	DESTINATION ${LIBRARY_OUTPUT_PATH})
    endif()
endif()
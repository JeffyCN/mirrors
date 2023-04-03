set(AMERGE_SRC
    rk_aiq_algo_amerge_itf.cpp
    rk_aiq_uapi_amerge_int.cpp
    )

if (RKAIQ_HAVE_MERGE_V10)
    list(APPEND AMERGE_SRC rk_aiq_amerge_algo_v10.cpp)
endif()

if (RKAIQ_HAVE_MERGE_V11)
    list(APPEND AMERGE_SRC rk_aiq_amerge_algo_v11.cpp)
endif()

if (RKAIQ_HAVE_MERGE_V12)
    list(APPEND AMERGE_SRC rk_aiq_amerge_algo_v12.cpp)
endif()

dec_algo(amerge rk_aiq_algo_amerge_itf.cpp ${AMERGE_SRC})

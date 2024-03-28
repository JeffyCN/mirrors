set(ADRC_SRC
    rk_aiq_algo_adrc_itf.cpp
    rk_aiq_uapi_adrc_int.cpp
    )

if (RKAIQ_HAVE_DRC_V10)
    list(APPEND ADRC_SRC rk_aiq_adrc_algo_v10.cpp)
endif()

if (RKAIQ_HAVE_DRC_V11)
    list(APPEND ADRC_SRC rk_aiq_adrc_algo_v11.cpp)
endif()

if (RKAIQ_HAVE_DRC_V12 OR RKAIQ_HAVE_DRC_V12_LITE)
    list(APPEND ADRC_SRC rk_aiq_adrc_algo_v12.cpp)
endif()

if (RKAIQ_HAVE_DRC_V20)
    list(APPEND ADRC_SRC rk_aiq_adrc_algo_v20.cpp)
endif()

dec_algo(adrc rk_aiq_algo_adrc_itf.cpp ${ADRC_SRC})

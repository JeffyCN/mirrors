set(ADEBAYER_SRC
    rk_aiq_algo_adebayer_itf.cpp
    rk_aiq_uapi_adebayer_int.cpp
    )
if (RKAIQ_HAVE_DEBAYER_V1)
    list(APPEND ADEBAYER_SRC rk_aiq_adebayer_algo_v1.cpp)
endif()

if (RKAIQ_HAVE_DEBAYER_V2 OR RKAIQ_HAVE_DEBAYER_V2_LITE)
    list(APPEND ADEBAYER_SRC rk_aiq_adebayer_algo_v2.cpp)
endif()

if (RKAIQ_HAVE_DEBAYER_V3)
    list(APPEND ADEBAYER_SRC rk_aiq_adebayer_algo_v3.cpp)
endif()

dec_algo(adebayer rk_aiq_algo_adebayer_itf.cpp ${ADEBAYER_SRC})

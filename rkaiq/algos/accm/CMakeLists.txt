set(ACCM_SRC
    rk_aiq_algo_accm_itf.cpp
    rk_aiq_accm_algo_com.cpp
    rk_aiq_uapi_accm_int.cpp
    )

if (RKAIQ_HAVE_CCM_V1)
    list(APPEND ACCM_SRC rk_aiq_accm_algo_v1.cpp)
endif()

if (RKAIQ_HAVE_CCM_V2 OR RKAIQ_HAVE_CCM_V3)
    list(APPEND ACCM_SRC rk_aiq_accm_algo_v2.cpp)
endif()

dec_algo(accm rk_aiq_algo_accm_itf.cpp ${ACCM_SRC})

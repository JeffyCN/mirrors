set(AGAMMA_SRC
    rk_aiq_algo_agamma_itf.cpp
    rk_aiq_uapi_agamma_int.cpp
    )

if (RKAIQ_HAVE_GAMMA_V10)
    list(APPEND AGAMMA_SRC rk_aiq_agamma_algo_v10.cpp)
endif()

if (RKAIQ_HAVE_GAMMA_V11)
    list(APPEND AGAMMA_SRC rk_aiq_agamma_algo_v11.cpp)
endif()

dec_algo(agamma rk_aiq_algo_agamma_itf.cpp ${AGAMMA_SRC})

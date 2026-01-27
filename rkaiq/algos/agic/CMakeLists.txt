set(AGIC_SRC
    rk_aiq_algo_agic_itf.cpp
    rk_aiq_uapi_agic_int.cpp
    )

if (RKAIQ_HAVE_GIC_V1)
    list(APPEND AGIC_SRC rk_aiq_algo_agic_v1.cpp)
endif()

if (RKAIQ_HAVE_GIC_V2)
    list(APPEND AGIC_SRC rk_aiq_algo_agic_v2.cpp)
endif()

dec_algo(agic rk_aiq_algo_agic_itf.cpp ${AGIC_SRC})

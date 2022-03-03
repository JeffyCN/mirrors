set(AGIC_SRC
    rk_aiq_algo_agic_itf.cpp
    rk_aiq_algo_agic.cpp
    rk_aiq_uapi_agic_int.cpp
    )

dec_algo(agic rk_aiq_algo_agic_itf.cpp ${AGIC_SRC})

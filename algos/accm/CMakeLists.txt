set(ACCM_SRC
    rk_aiq_algo_accm_itf.cpp
    rk_aiq_accm_algo.cpp
    rk_aiq_uapi_accm_int.cpp
    )

dec_algo(accm rk_aiq_algo_accm_itf.cpp ${ACCM_SRC})

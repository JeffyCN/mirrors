set(ALSC_SRC
    rk_aiq_algo_alsc_itf.cpp
    rk_aiq_alsc_algo.cpp
    rk_aiq_uapi_alsc_int.cpp
    )

dec_algo(alsc rk_aiq_algo_alsc_itf.cpp ${ALSC_SRC})

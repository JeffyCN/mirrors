set(ADRC_SRC
    rk_aiq_algo_adrc_itf.cpp
    rk_aiq_adrc_algo.cpp
    rk_aiq_uapi_adrc_int.cpp
    )

dec_algo(adrc rk_aiq_algo_adrc_itf.cpp ${ADRC_SRC})

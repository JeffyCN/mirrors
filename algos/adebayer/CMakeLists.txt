set(ADEBAYER_SRC
    rk_aiq_algo_adebayer_itf.cpp
    rk_aiq_algo_adebayer.cpp
    rk_aiq_uapi_adebayer_int.cpp
    )

dec_algo(adebayer rk_aiq_algo_adebayer_itf.cpp ${ADEBAYER_SRC})

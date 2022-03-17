set(AFEC_SRC
    rk_aiq_algo_afec_itf.cpp
    rk_aiq_uapi_afec_int.cpp
    )

dec_algo(afec rk_aiq_algo_afec_itf.cpp ${AFEC_SRC})

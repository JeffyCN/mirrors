set(ACAC_SRC
    cac_adaptor.cpp
    rk_aiq_algo_acac_itf.cpp
    rk_aiq_uapi_acac_int.cpp
    )

dec_algo(acac rk_aiq_algo_acac_itf.cpp ${ACAC_SRC})

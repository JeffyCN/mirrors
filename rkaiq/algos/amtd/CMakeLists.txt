set(AMTD_SRC
    rk_aiq_algo_amtd_itf.c
    rk_aiq_uapi_amtd_int.c
    rk_aiq_amtd_algo.c
    )

dec_algo(amtd rk_aiq_algo_amtd_itf.c ${AMTD_SRC})

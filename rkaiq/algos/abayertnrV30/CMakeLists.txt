set(ABAYERTNR_V30_SRC
    rk_aiq_abayertnr_algo_itf_v30.cpp
    rk_aiq_abayertnr_algo_bayertnr_v30.cpp
    rk_aiq_abayertnr_algo_v30.cpp
    rk_aiq_uapi_abayertnr_int_v30.cpp
    )

dec_algo_v(abayertnr 30 rk_aiq_abayertnr_algo_itf_v30.cpp ${ABAYERTNR_V30_SRC})

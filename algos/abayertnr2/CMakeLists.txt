set(ABAYERTNR_V2_SRC
    rk_aiq_abayertnr_algo_itf_v2.cpp
    rk_aiq_abayertnr_algo_bayertnr_v2.cpp
    rk_aiq_abayertnr_algo_v2.cpp
    rk_aiq_uapi_abayertnr_int_v2.cpp
    )

dec_algo_v(abayertnr 2 rk_aiq_abayertnr_algo_itf_v2.cpp ${ABAYERTNR_V2_SRC})

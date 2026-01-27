set(ABAYERTNR_V23_SRC
    rk_aiq_abayertnr_algo_itf_v23.cpp
    rk_aiq_abayertnr_algo_bayertnr_v23.cpp
    rk_aiq_abayertnr_algo_v23.cpp
    rk_aiq_uapi_abayertnr_int_v23.cpp
    )

dec_algo_v(abayertnr 23 rk_aiq_abayertnr_algo_itf_v23.cpp ${ABAYERTNR_V23_SRC})

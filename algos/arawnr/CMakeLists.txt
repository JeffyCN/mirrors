set(ARAWNR_V1_SRC
    rk_aiq_abayernr_algo_itf_v1.cpp
    rk_aiq_abayernr_algo_bayernr_v1.cpp
    rk_aiq_abayernr_algo_v1.cpp
    rk_aiq_uapi_abayernr_int_v1.cpp
    )

dec_algo_v(arawnr 1 rk_aiq_abayernr_algo_itf_v1.cpp ${ARAWNR_V1_SRC})

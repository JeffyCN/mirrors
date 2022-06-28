set(ARAWNR_V2_SRC
    rk_aiq_abayernr_algo_itf_v2.cpp
    rk_aiq_abayernr_algo_bayernr_v2.cpp
    rk_aiq_abayernr_algo_v2.cpp
    rk_aiq_uapi_abayernr_int_v2.cpp
    )

dec_algo_v(arawnr 2 rk_aiq_abayernr_algo_itf_v2.cpp ${ARAWNR_V2_SRC})

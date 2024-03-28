set(ACNR_V2_SRC
    rk_aiq_acnr_algo_itf_v2.cpp
    rk_aiq_acnr_algo_cnr_v2.cpp
    rk_aiq_acnr_algo_v2.cpp
    rk_aiq_uapi_acnr_int_v2.cpp
    )

dec_algo_v(acnr 2 rk_aiq_acnr_algo_v2.cpp ${ACNR_V2_SRC})

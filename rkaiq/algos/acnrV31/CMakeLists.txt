set(ACNR_V31_SRC
    rk_aiq_acnr_algo_itf_v31.cpp
    rk_aiq_acnr_algo_cnr_v31.cpp
    rk_aiq_acnr_algo_v31.cpp
    rk_aiq_uapi_acnr_int_v31.cpp
    )

dec_algo_v(acnr 31 rk_aiq_acnr_algo_itf_v31.cpp ${ACNR_V31_SRC})

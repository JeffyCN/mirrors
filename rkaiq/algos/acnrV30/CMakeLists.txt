set(ACNR_V30_SRC
    rk_aiq_acnr_algo_itf_v30.cpp
    rk_aiq_acnr_algo_cnr_v30.cpp
    rk_aiq_acnr_algo_v30.cpp
    rk_aiq_uapi_acnr_int_v30.cpp
    )

dec_algo_v(acnr 30 rk_aiq_acnr_algo_v30.cpp ${ACNR_V30_SRC})

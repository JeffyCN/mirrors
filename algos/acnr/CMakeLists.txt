set(ACNR_V1_SRC
    rk_aiq_acnr_algo_itf_v1.cpp
    rk_aiq_acnr_algo_cnr_v1.cpp
    rk_aiq_acnr_algo_v1.cpp
    rk_aiq_uapi_acnr_int_v1.cpp
    )

dec_algo_v(acnr 1 rk_aiq_acnr_algo_v1.cpp ${ACNR_V1_SRC})

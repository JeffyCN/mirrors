set(AMFNR_V1_SRC
    rk_aiq_amfnr_algo_itf_v1.cpp
    rk_aiq_amfnr_algo_mfnr_v1.cpp
    rk_aiq_amfnr_algo_v1.cpp
    rk_aiq_uapi_amfnr_int_v1.cpp
    )

dec_algo_v(amfnr 1 rk_aiq_amfnr_algo_itf_v1.cpp ${AMFNR_V1_SRC})

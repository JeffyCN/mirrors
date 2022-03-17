set(AYNR_V3_SRC
    rk_aiq_aynr_algo_itf_v3.cpp
    rk_aiq_aynr_algo_ynr_v3.cpp
    rk_aiq_aynr_algo_v3.cpp
    rk_aiq_uapi_aynr_int_v3.cpp
    )

dec_algo_v(aynr 3 rk_aiq_aynr_algo_itf_v3.cpp ${AYNR_V3_SRC})

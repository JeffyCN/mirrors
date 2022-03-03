set(AYNR_V1_SRC
    rk_aiq_aynr_algo_itf_v1.cpp
    rk_aiq_aynr_algo_ynr_v1.cpp
    rk_aiq_aynr_algo_v1.cpp
    rk_aiq_uapi_aynr_int_v1.cpp
    )

dec_algo_v(aynr 1 rk_aiq_aynr_algo_itf_v1.cpp ${AYNR_V1_SRC})

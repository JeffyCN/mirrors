set(ASHARP_V3_SRC
    rk_aiq_asharp_algo_itf_v3.cpp
    rk_aiq_asharp_algo_sharp_v3.cpp
    rk_aiq_asharp_algo_v3.cpp
    rk_aiq_uapi_asharp_int_v3.cpp
    )

dec_algo_v(asharp 3 rk_aiq_asharp_algo_itf_v3.cpp ${ASHARP_V3_SRC})

set(ASHARP_V4_SRC
    rk_aiq_asharp_algo_itf_v4.cpp
    rk_aiq_asharp_algo_sharp_v4.cpp
    rk_aiq_asharp_algo_v4.cpp
    rk_aiq_uapi_asharp_int_v4.cpp
    )

dec_algo_v(asharp 4 rk_aiq_asharp_algo_itf_v4.cpp ${ASHARP_V4_SRC})

set(ABLC_V32_SRC
    rk_aiq_ablc_algo_itf_v32.cpp
    rk_aiq_ablc_algo_v32.cpp
    rk_aiq_uapi_ablc_int_v32.cpp
    )

dec_algo_v(ablc 32 rk_aiq_ablc_algo_itf_v32.cpp ${ABLC_V32_SRC})
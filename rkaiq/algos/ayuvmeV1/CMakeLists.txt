set(AYUVME_V1_SRC
    rk_aiq_ayuvme_algo_itf_v1.cpp
    rk_aiq_ayuvme_algo_yuvme_v1.cpp
    rk_aiq_ayuvme_algo_v1.cpp
    rk_aiq_uapi_ayuvme_int_v1.cpp
    )

dec_algo_v(ayuvme 1 rk_aiq_ayuvme_algo_itf_v1.cpp ${AYUVME_V1_SRC})

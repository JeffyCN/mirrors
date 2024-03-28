set(AGAIN_V2_SRC
    rk_aiq_again_algo_itf_v2.cpp
    rk_aiq_again_algo_gain_v2.cpp
    rk_aiq_again_algo_v2.cpp
    rk_aiq_uapi_again_int_v2.cpp
    )

dec_algo_v(again 2 rk_aiq_again_algo_itf_v2.cpp ${AGAIN_V2_SRC})

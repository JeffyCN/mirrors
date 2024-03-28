set(AGAIN_V1_SRC
    rk_aiq_again_algo_itf.cpp
    rk_aiq_again_algo_gain.cpp
    rk_aiq_again_algo.cpp
    rk_aiq_uapi_again_int.cpp
    )

dec_algo_v(again 1 rk_aiq_again_algo_itf.cpp ${AGAIN_V1_SRC})

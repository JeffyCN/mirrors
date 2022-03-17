set(ABLC_SRC
    rk_aiq_algo_ablc_itf.cpp
    rk_aiq_ablc_algo.cpp
    rk_aiq_uapi_ablc_int.cpp
    )

dec_algo(ablc rk_aiq_algo_ablc_itf.cpp ${ABLC_SRC})

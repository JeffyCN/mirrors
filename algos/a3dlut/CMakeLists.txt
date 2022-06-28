set(A3DLUT_SRC
    rk_aiq_algo_a3dlut_itf.cpp
    rk_aiq_a3dlut_algo.cpp
    rk_aiq_uapi_a3dlut_int.cpp
    )

dec_algo(a3dlut rk_aiq_algo_a3dlut_itf.cpp ${A3DLUT_SRC})

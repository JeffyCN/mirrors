set(AMERGE_SRC
    rk_aiq_algo_amerge_itf.cpp
    rk_aiq_amerge_algo.cpp
    rk_aiq_uapi_amerge_int.cpp
    )

dec_algo(amerge rk_aiq_algo_amerge_itf.cpp ${AMERGE_SRC})

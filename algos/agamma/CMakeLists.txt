set(AGAMMA_SRC
    rk_aiq_algo_agamma_itf.cpp
    rk_aiq_agamma_algo.cpp
    rk_aiq_uapi_agamma_int.cpp
    )

dec_algo(agamma rk_aiq_algo_agamma_itf.cpp ${AGAMMA_SRC})

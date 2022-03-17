set(ATMO_SRC
    rk_aiq_algo_atmo_itf.cpp
    rk_aiq_atmo_algo.cpp
    rk_aiq_uapi_atmo_int.cpp
    )

dec_algo(atmo rk_aiq_algo_atmo_itf.cpp ${ATMO_SRC})

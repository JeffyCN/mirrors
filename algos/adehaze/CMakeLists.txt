set(ADHAZ_SRC
    rk_aiq_algo_adhaz_itf.cpp
    rk_aiq_adehaze_algo.cpp
    rk_aiq_uapi_adehaze_int.cpp
    )

dec_algo(adehaze rk_aiq_algo_adhaz_itf.cpp ${ADHAZ_SRC})

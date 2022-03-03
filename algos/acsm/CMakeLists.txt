set(ACSM_SRC
    rk_aiq_algo_acsm_itf.cpp
    rk_aiq_uapi_acsm.cpp
    )

dec_algo(acsm rk_aiq_algo_acsm_itf.cpp ${ACSM_SRC})

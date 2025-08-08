set(ADEGAMMA_SRC
    rk_aiq_algo_adegamma_itf.cpp
    rk_aiq_adegamma_algo.cpp
    rk_aiq_uapi_adegamma_int.cpp
    )

dec_algo(adegamma rk_aiq_algo_adegamma_itf.cpp ${ADEGAMMA_SRC})

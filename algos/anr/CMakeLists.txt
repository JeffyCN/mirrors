set(ANR_SRC
    rk_aiq_algo_anr_itf.cpp
    rk_aiq_anr_algo.cpp
    rk_aiq_anr_algo_bayernr.cpp
    rk_aiq_anr_algo_mfnr.cpp
    rk_aiq_anr_algo_uvnr.cpp
    rk_aiq_anr_algo_ynr.cpp
    rk_aiq_uapi_anr_int.cpp
    rk_aiq_anr_algo_gain.cpp
    )

dec_algo(anr rk_aiq_algo_anr_itf.cpp ${ANR_SRC})

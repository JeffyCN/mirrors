set(ABAYER2DNR_V2_SRC
    rk_aiq_abayer2dnr_algo_itf_v2.cpp
    rk_aiq_abayer2dnr_algo_bayernr_v2.cpp
    rk_aiq_abayer2dnr_algo_v2.cpp
    rk_aiq_uapi_abayer2dnr_int_v2.cpp
    )

dec_algo_v(abayer2dnr 2 rk_aiq_abayer2dnr_algo_bayernr_v2.cpp ${ABAYER2DNR_V2_SRC})

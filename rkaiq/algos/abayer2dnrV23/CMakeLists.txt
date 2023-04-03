set(ABAYER2DNR_V23_SRC
    rk_aiq_abayer2dnr_algo_itf_v23.cpp
    rk_aiq_abayer2dnr_algo_bayernr_v23.cpp
    rk_aiq_abayer2dnr_algo_v23.cpp
    rk_aiq_uapi_abayer2dnr_int_v23.cpp
    )

dec_algo_v(abayer2dnr 23 rk_aiq_abayer2dnr_algo_bayernr_v23.cpp ${ABAYER2DNR_V23_SRC})

set(AUVNR_V1_SRC
    rk_aiq_auvnr_algo_itf_v1.cpp
    rk_aiq_auvnr_algo_uvnr_v1.cpp
    rk_aiq_auvnr_algo_v1.cpp
    rk_aiq_uapi_auvnr_int_v1.cpp
    )

dec_algo_v(auvnr 1 rk_aiq_auvnr_algo_itf_v1.cpp ${AUVNR_V1_SRC})

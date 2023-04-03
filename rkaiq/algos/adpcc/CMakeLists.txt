set(ADPCC_SRC
    rk_aiq_algo_adpcc_itf.cpp
    rk_aiq_adpcc_algo.cpp
    rk_aiq_uapi_adpcc_int.cpp
    )

dec_algo(adpcc rk_aiq_algo_adpcc_itf.cpp ${ADPCC_SRC})

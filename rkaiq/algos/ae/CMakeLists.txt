set (AE_SRC
    rk_aiq_algo_ae_itf.cpp
    rk_aiq_ae_algo.cpp
    rk_aiq_ae_algo_hdrecm.cpp
    rk_aiq_ae_algo_ecm.cpp
    rk_aiq_ae_algo_clm.cpp
    rk_aiq_ae_algo_ldm.cpp
    rk_aiq_ae_algo_pid.cpp
    rk_aiq_uapi_ae_int.cpp
    rk_aiq_ae_algo_iris.cpp
    )

dec_algo(ae rk_aiq_algo_ae_itf.cpp ${AE_SRC})

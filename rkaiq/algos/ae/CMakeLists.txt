set (AE_SRC
    rk_aiq_algo_ae_itf.c
    rk_aiq_ae_algo.c
    rk_aiq_ae_algo_ecm.c
    rk_aiq_ae_algo_clm.c
    rk_aiq_ae_algo_ldm.c
    rk_aiq_ae_algo_pid.c
    rk_aiq_uapi_ae_int.c
    rk_aiq_ae_algo_iris.c
   )

if (RKAIQ_HAVE_DUMPSYS)
    set(AE_SRC ${AE_SRC} rk_aiq_ae_algo_info.c)
endif()

dec_algo(ae rk_aiq_algo_ae_itf.c ${AE_SRC})

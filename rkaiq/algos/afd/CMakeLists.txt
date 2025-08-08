set(AFD_SRC
    rk_aiq_algo_afd_itf.c
    rk_aiq_uapi_afd_int.c
    rk_aiq_afd_algo.c
    )

if (RKAIQ_HAVE_DUMPSYS)
    set(AFD_SRC ${AFD_SRC} rk_aiq_afd_algo_info.c)
endif()

dec_algo(afd rk_aiq_algo_afd_itf.c ${AFD_SRC})

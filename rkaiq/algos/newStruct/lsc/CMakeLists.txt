set(LSC_SRC
    algo_lsc.c
    )

if(RKAIQ_HAVE_DUMPSYS)
    set(LSC_SRC ${LSC_SRC} algo_lsc_info.c)
endif()

dec_algo(lsc algo_lsc.c ${LSC_SRC})

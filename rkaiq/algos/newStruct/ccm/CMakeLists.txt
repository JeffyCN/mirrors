set(ACCM_SRC
    algo_ccm.c
    )

if(RKAIQ_HAVE_DUMPSYS)
    set(ACCM_SRC ${ACCM_SRC} algo_ccm_info.c)
endif()

dec_algo(ccm algo_ccm.c ${ACCM_SRC})

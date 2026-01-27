set(A3DLUT_SRC
    algo_3dlut.c
    )

if(RKAIQ_HAVE_DUMPSYS)
    set(A3DLUT_SRC ${A3DLUT_SRC} algo_3dlut_info.c)
endif()

dec_algo(3dlut algo_3dlut.c ${A3DLUT_SRC})

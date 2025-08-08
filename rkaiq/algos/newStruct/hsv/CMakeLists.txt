set(HSV_SRC
    algo_hsv.c
    )

if(RKAIQ_HAVE_DUMPSYS)
    set(HSV_SRC ${HSV_SRC} algo_hsv_info.c)
endif()

dec_algo(hsv algo_hsv.c ${HSV_SRC})

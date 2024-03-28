set(ARGBIR_SRC
    rk_aiq_algo_argbir_itf.cpp
    rk_aiq_uapi_argbir_int.cpp
    )

if (RKAIQ_HAVE_RGBIR_REMOSAIC_V10)
    list(APPEND ARGBIR_SRC rk_aiq_argbir_algo_v10.cpp)
endif()

dec_algo(argbir rk_aiq_algo_argbir_itf.cpp ${ARGBIR_SRC})

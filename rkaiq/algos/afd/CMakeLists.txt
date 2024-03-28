set(AFD_SRC
    rk_aiq_algo_afd_itf.cpp
    rk_aiq_uapi_afd_int.cpp
    )
if (RKAIQ_HAVE_AFD_V1)
    list(APPEND AFD_SRC rk_aiq_afd_algo.cpp)
endif()

if (RKAIQ_HAVE_AFD_V2)
    list(APPEND AFD_SRC rk_aiq_afd_algo.cpp)
endif()

dec_algo(afd rk_aiq_algo_afd_itf.cpp ${AFD_SRC})

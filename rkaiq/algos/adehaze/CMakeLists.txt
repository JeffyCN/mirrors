set(ADHAZ_SRC
    rk_aiq_algo_adehaze_itf.cpp
    rk_aiq_uapi_adehaze_int.cpp
    )

if (RKAIQ_HAVE_DEHAZE_V10)
    list(APPEND ADHAZ_SRC rk_aiq_adehaze_algo_v10.cpp)
endif()

if (RKAIQ_HAVE_DEHAZE_V11)
    list(APPEND ADHAZ_SRC rk_aiq_adehaze_algo_v11.cpp)
endif()

if (RKAIQ_HAVE_DEHAZE_V11_DUO)
    list(APPEND ADHAZ_SRC rk_aiq_adehaze_algo_v11_duo.cpp)
endif()

if (RKAIQ_HAVE_DEHAZE_V12)
    list(APPEND ADHAZ_SRC rk_aiq_adehaze_algo_v12.cpp)
endif()

if (RKAIQ_HAVE_DEHAZE_V14)
    list(APPEND ADHAZ_SRC rk_aiq_adehaze_algo_v14.cpp)
endif()

dec_algo(adehaze rk_aiq_algo_adehaze_itf.cpp ${ADHAZ_SRC})

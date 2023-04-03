set(AWB_SRC
    color_space_convert.cpp
    fixfloat.cpp
    rk_aiq_algo_awb_itf.cpp
    rk_aiq_awb_algo_com1.cpp
    rk_aiq_awb_algo_com2.cpp
    rk_aiq_awb_algo_sgc.cpp
    rk_aiq_algo_awb_itf.cpp
    rk_aiq_uapi_awb_int.cpp
    rk_aiq_uapiv2_awb_int.cpp
    )

if (RKAIQ_HAVE_AWB_V20)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v200.cpp)
endif()

if (RKAIQ_HAVE_AWB_V21)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v201.cpp)
endif()

if (RKAIQ_HAVE_AWB_V32)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v32.cpp)
endif()
dec_algo(awb rk_aiq_algo_awb_itf.cpp ${AWB_SRC})

set(AWB_SRC
    color_space_convert.c
    fixfloat.c
    rk_aiq_algo_awb_itf.c
    rk_aiq_awb_algo_com2.c
    rk_aiq_uapi_awb_int.c

    )

if (RKAIQ_HAVE_AWB_V20)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v200.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_com1.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_sgc.c)
    list(APPEND AWB_SRC rk_aiq_uapi_awb_int.c)
endif()

if (RKAIQ_HAVE_AWB_V21)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v201.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_com1.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_sgc.c)
    list(APPEND AWB_SRC rk_aiq_uapiv2_awb_int.c)
endif()

if (RKAIQ_HAVE_AWB_V32)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v32.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_com1.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_sgc.c)
    list(APPEND AWB_SRC rk_aiq_uapiv2_awb_int.c)
endif()

if (RKAIQ_HAVE_AWB_V39)
    list(APPEND AWB_SRC rk_aiq_awb_algo_v39.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_com3.c)
    list(APPEND AWB_SRC rk_aiq_awb_algo_sgc2.c)
    list(APPEND AWB_SRC rk_aiq_uapiv3_awb_int.c)
endif()
dec_algo(awb rk_aiq_algo_awb_itf.c ${AWB_SRC})

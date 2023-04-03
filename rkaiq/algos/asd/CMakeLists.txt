set(ASD_SRC
    rk_aiq_algo_asd_itf.cpp
    scene_detector.cpp
    rk_aiq_uapi_asd_int.cpp
    )

dec_algo(asd rk_aiq_algo_asd_itf.cpp ${ASD_SRC})

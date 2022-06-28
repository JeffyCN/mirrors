set(AORB_SRC
    rk_aiq_algo_aorb_itf.cpp
    orb_algos.cpp
    orb_algos_opencv.cpp
    )

dec_algo(aorb rk_aiq_algo_aorb_itf.cpp ${AORB_SRC})

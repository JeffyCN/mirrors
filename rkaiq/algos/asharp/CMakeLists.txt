set(ASHARP_SRC
    rk_aiq_algo_asharp_itf.cpp
    rk_aiq_asharp_algo.cpp
    rk_aiq_asharp_algo_edgefilter.cpp
    rk_aiq_asharp_algo_sharp.cpp
    rk_aiq_uapi_asharp_int.cpp
    )

dec_algo(asharp rk_aiq_algo_asharp_itf.cpp ${ASHARP_SRC})

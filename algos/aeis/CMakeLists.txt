set(AEIS_SRC
    eis_loader.cpp
    scaler_service.cpp
    remap_backend.cpp
    imu_service.cpp
    eis_algo_service.cpp
    rk_aiq_uapi_aeis_int.cpp
    rk_aiq_algo_aeis_itf.cpp
    )

dec_algo(aeis rk_aiq_algo_aeis_itf.cpp ${AEIS_SRC})

add_library(dvs_gyro SHARED IMPORTED GLOBAL)
set_target_properties(dvs_gyro PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libdvs_gyro.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")

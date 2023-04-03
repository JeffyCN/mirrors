if (RKAIQ_HAVE_LDCH_V10)
    set(ALDCH_SRC
        rk_aiq_uapi_aldch_int.cpp
        rk_aiq_algo_aldch_itf.cpp
        rk_aiq_ldch_generate_mesh.cpp
        )

    dec_algo(aldch rk_aiq_algo_aldch_itf.cpp ${ALDCH_SRC})
endif()

if (RKAIQ_HAVE_LDCH_V21)
    set(ALDCH_SRC
        rk_aiq_uapi_aldch_v21_int.cpp
        rk_aiq_algo_aldch_v21_itf.cpp
        rk_aiq_ldch_generate_mesh.cpp
        )

    dec_algo(aldch rk_aiq_algo_aldch_v21_itf.cpp ${ALDCH_SRC})
endif()

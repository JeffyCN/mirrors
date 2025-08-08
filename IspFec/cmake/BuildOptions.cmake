include(CMakeDependentOption)

cmake_dependent_option(RKAIQ_ENABLE_ASAN "Build libraries/executable with address santilizer" OFF "WIN32" OFF)

if (RKFEC_TARGET_SOC STREQUAL "rv1126b")
    set(RKFEC_VERSION        "20"  CACHE INTERNAL "RKFEC hardware version 20"     FORCE)
    option(RKFEC_HAVE_CUNIT "Have CUnit" ON)
elseif (RKFEC_TARGET_SOC STREQUAL "rk3588")
    set(RKFEC_VERSION        "10"  CACHE INTERNAL "RKFEC hardware version 10"     FORCE)
else()
    set(RKFEC_VERSION        "20"  CACHE INTERNAL "RKFEC hardware version 20"     FORCE)
    message(WARNING "RKFEC_TARGET_SOC SHOULD be set")
endif()

message(STATUS "Building RKFEC v${RKFEC_VERSION} rkfec for soc ${RKFEC_TARGET_SOC}")
message(STATUS "RKFEC_HAVE_CUNIT ${RKFEC_HAVE_CUNIT}")

if (RKFEC_VERSION STREQUAL "20")
    set(RKFEC_HW_V20 TRUE CACHE INTERNAL "Build rkfec with RKFEC_HW_V20"      FORCE)
elseif ()
    set(RKFEC_HW_V10 TRUE CACHE INTERNAL "Build rkfec with RKFEC_HW_V10"      FORCE)
endif()
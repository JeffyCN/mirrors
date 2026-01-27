include(CMakeDependentOption)

cmake_dependent_option(RKAIQ_ENABLE_ASAN "Build libraries/executable with address santilizer" OFF "WIN32" OFF)
set(RKAIQ_ENABLE_LIBDRM      TRUE  CACHE INTERNAL "Build rkaiq with libdrm")
set(RKAIQ_NEWSTRUCT_TEST     FALSE CACHE INTERNAL "Build rkisp_demo with new structs")

if (RKAIQ_TARGET_SOC STREQUAL "rk3576"
		OR RKAIQ_TARGET_SOC STREQUAL "rv1103b"
		OR RKAIQ_TARGET_SOC STREQUAL "rv1126b")
    set(RKAIQ_NEWSTRUCT_TEST     TRUE)
endif()

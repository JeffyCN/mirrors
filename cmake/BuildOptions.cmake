include(CMakeDependentOption)

cmake_dependent_option(RKAIQ_BUILD_DOCS "Build all of rkaiq's documents." OFF "WIN32" OFF)
cmake_dependent_option(RKAIQ_BUILD_TESTS "Build all of rkaiq's own tests." OFF "WIN32" OFF)
cmake_dependent_option(RKAIQ_BUILD_EXAMPLES "Build all of rkaiq's own examples." OFF "WIN32" OFF)
cmake_dependent_option(RKAIQ_ENABLE_ASAN "Build libraries/executable with address santilizer" OFF "WIN32" OFF)
cmake_dependent_option(RKAIQ_ENABLE_SIMULATOR "Build simulator for algos run on PC" OFF "WIN32" OFF)
option(RKAIQ_ENABLE_PARSER_V1 "Build rkaiq's old xml parser" OFF)
option(RKAIQ_ENABLE_CAMGROUP "Build rkaiq's camera group algorithms" ON)

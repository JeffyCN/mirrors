cmake_minimum_required(VERSION 2.8...3.22)
project(aiq_test)

find_package(Threads)

if (ENABLE_TEST_TASK_SERVICE)
    set(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)
    set(CMAKE_THREAD_PREFER_PTHREAD true)
    set(THREADS_PREFER_PTHREAD_FLAG true)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xcore)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xcore/base)
    add_executable(test_task_service test_task_service.cpp)
    target_link_libraries(test_task_service
        PRIVATE
        Threads::Threads
    )

    install(TARGETS test_task_service RUNTIME DESTINATION bin)
endif()

if (RKAIQ_ENABLE_SAMPLE_THREAD_POOL_WAIT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../c_base)
    add_executable(sample_thpool_wait
        ${CMAKE_CURRENT_SOURCE_DIR}/../c_base/aiq_thpool.c
        test_thread_pool/sample_thpool_wait.c
        )
    target_link_libraries(sample_thpool_wait -lpthread)

    install(TARGETS sample_thpool_wait RUNTIME DESTINATION bin)
endif()

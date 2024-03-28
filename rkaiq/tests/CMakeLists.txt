cmake_minimum_required(VERSION 2.8...3.22)
project(aiq_test)

set(THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE)
set(CMAKE_THREAD_PREFER_PTHREAD true)
set(THREADS_PREFER_PTHREAD_FLAG true)
find_package(Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../xcore/base)
add_executable(test_task_service test_task_service.cpp)
target_link_libraries(test_task_service
    PRIVATE
    Threads::Threads
    )

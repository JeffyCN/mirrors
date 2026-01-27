add_executable(pipeline_demo)
target_sources(pipeline_demo PRIVATE main.cpp raw_reader.cpp)
target_link_libraries(pipeline_demo PRIVATE rk_irfpa_pipeline)

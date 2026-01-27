#add_executable(irfpa_stream_demo)
#target_sources(irfpa_stream_demo PRIVATE irfpa_stream_demo.cpp)
#target_link_libraries(irfpa_stream_demo PRIVATE rk_irfpa_hw_stream)


#add_executable(irfpa_isp_demo)
#target_sources(irfpa_isp_demo PRIVATE irfpa_isp_demo.cpp raw_reader.cpp)
#target_link_libraries(irfpa_isp_demo PRIVATE rk_irfpa_hw_isp)

rk_irfpa_add_subdirectory()

set(target rk_irfpa_sensorhw)
rk_irfpa_add_simple_libaray (${target})
target_link_libraries(${target} INTERFACE rk_irfpa_streamhw)

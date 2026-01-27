add_library(media-ctl INTERFACE)
target_sources(media-ctl INTERFACE libmediactl.c)
target_include_directories(media-ctl INTERFACE .)

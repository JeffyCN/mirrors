rkximage_sources = [
  'gstkmsallocator.c',
  'gstkmsbufferpool.c',
  'gstkmsutils.c',
  'ximagesink.c',
]

if not x11_dep.found() or not drm_dep.found()
  subdir_done()
endif

rkximage = library('gstrkximage',
  rkximage_sources,
  c_args : [gst_rockchip_args],
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep, x11_dep, drm_dep],
  install : true,
  install_dir : plugins_install_dir,
)

kmssrc_sources = [
  'gstkmssrc.c',
]

if not drm_dep.found()
  subdir_done()
endif

kmssrc = library('gstkmssrc',
  kmssrc_sources,
  c_args : [gst_rockchip_args],
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep, drm_dep],
  install : true,
  install_dir : plugins_install_dir,
)

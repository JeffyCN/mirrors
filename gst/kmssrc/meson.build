kmssrc_sources = [
  'gstkmssrc.c',
]

if not drm_dep.found()
  subdir_done()
endif

kmssrc_c_args = []
if cc.has_function('drmModeGetFB2', dependencies : drm_dep)
kmssrc_c_args += ['-DHAS_DRM_MODE_FB2']
endif
if cc.has_function('drmCloseBufferHandle', dependencies : drm_dep)
kmssrc_c_args += ['-DHAS_DRM_CLOSE_HANDLE']
endif

kmssrc = library('gstkmssrc',
  kmssrc_sources,
  c_args : [gst_rockchip_args, kmssrc_c_args],
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep, drm_dep],
  install : true,
  install_dir : plugins_install_dir,
)

rga_dep = dependency('librga', required : get_option('rga'))

if not rga_dep.found()
  message('librga not found, skipping the rga plugin')
  subdir_done()
endif

rga_sources = [
  'gstrgavideoconvert.c',
  '../rockchipmpp/gstrgautil.c',
  '../dmaheap/gstdmaheapallocator.c',
]

libgstrga = library('gstrga',
  rga_sources,
  c_args : [gst_rockchip_args],
  include_directories : [configinc, include_directories('../rockchipmpp'),
      include_directories('../dmaheap')],
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep, rga_dep],
  install : true,
  install_dir : plugins_install_dir,
)

dmaheap_sources = [
  'gstdmaheapalloc.c',
  'gstdmaheapallocator.c',
  'gstdmaheapformat.c',
  'gstdmabufdownload.c',
]

libgstdmaheap = library('gstdmaheap',
  dmaheap_sources,
  c_args : [gst_rockchip_args],
  include_directories : [configinc],
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep],
  install : true,
  install_dir : plugins_install_dir,
)

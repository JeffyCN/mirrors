dmaheap_sources = [
  'gstdmaheapalloc.c',
  'gstdmaheapallocator.c',
  'gstdmaheapformat.c',
  'gstdmabufdownload.c',
]

# The download prefers the RGA blitter for the dma-buf -> system
# memory copy when librga and the shared format helpers are
# available; without them the element falls back to the plain CPU
# path and everything still builds.
rga_dep = dependency('librga', required : get_option('rga'))

dmaheap_deps = [gstbase_dep, gstvideo_dep, gstallocators_dep]
dmaheap_incs = [configinc]
dmaheap_c_args = [gst_rockchip_args]

if rga_dep.found()
  dmaheap_sources += ['../rockchipmpp/gstrgautil.c']
  dmaheap_incs += include_directories('../rockchipmpp')
  dmaheap_deps += rga_dep
  dmaheap_c_args += ['-DHAVE_RGA_UTIL']
  message('dmaheap: rga found, running the packed-format checks')

  # The packed 10-bit table entries are gated on the gst headers
  # carrying the vendor formats (the same check the rockchipmpp
  # plugin runs).
  cc = meson.get_compiler('c')
  nv12_ok = cc.has_header_symbol('gst/video/video-format.h',
      'GST_VIDEO_FORMAT_NV12_10LE40', dependencies: gstvideo_dep)
  message('dmaheap: NV12_10LE40 header symbol = @0@', nv12_ok.to_string())
  if nv12_ok
    dmaheap_c_args += ['-DHAVE_NV12_10LE40']
  endif
  if cc.has_header_symbol('gst/video/video-format.h',
      'GST_VIDEO_FORMAT_NV16_10LE40', dependencies: gstvideo_dep)
    dmaheap_c_args += ['-DHAVE_NV16_10LE40']
  endif
endif

libgstdmaheap = library('gstdmaheap',
  dmaheap_sources,
  c_args : dmaheap_c_args,
  include_directories : dmaheap_incs,
  dependencies : dmaheap_deps,
  install : true,
  install_dir : plugins_install_dir,
)

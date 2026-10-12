rockchipmpp_sources = [
  'gstmpp.c',
  'gstmppallocator.c',
  'gstmppdec.c',
  'gstmppgpu.c',
  'gstmppjpegdec.c',
  'gstmpprga.c',
  'gstmppvideodec.c',
  'gstmppenc.c',
  'gstmppjpegenc.c',
  'gstmpph264enc.c',
  'gstmpph265enc.c',
  'gstmppvp8enc.c',
]

if vpxalphadec
  rockchipmpp_sources += ['gstmppalphadecodebin.c', 'gstmppvpxalphadecodebin.c']
endif

if not mpp_dep.found()
  subdir_done()
endif

rockchipmpp_deps = [gstbase_dep, gstvideo_dep, gstallocators_dep, gstpbutils_dep, mpp_dep, rga_dep]

if not get_option('gpu').disabled() and egl_dep.found() and glesv2_dep.found() and \
   gbm_dep.found() and gpu_drm_dep.found()
  rockchipmpp_deps += [egl_dep, glesv2_dep, gbm_dep, gpu_drm_dep]
endif

rockchipmpp = library('gstrockchipmpp',
  rockchipmpp_sources,
  c_args : [gst_rockchip_args, '-Wextra'],
  include_directories : configinc,
  dependencies : rockchipmpp_deps,
  install : true,
  install_dir : plugins_install_dir,
)

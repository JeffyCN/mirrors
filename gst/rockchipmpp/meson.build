rockchipmpp_sources = [
  'gstmpp.c',
  'gstmppallocator.c',
  'gstmppdec.c',
  'gstmppjpegdec.c',
  'gstmppvideodec.c',
  'gstmppenc.c',
  'gstmppjpegenc.c',
  'gstmpph264enc.c',
  'gstmpph265enc.c',
  'gstmppvp8enc.c',
  'gstmppalphadecodebin.c',
  'gstmppvpxalphadecodebin.c',
]

if not mpp_dep.found()
  subdir_done()
endif

rockchipmpp = library('gstrockchipmpp',
  rockchipmpp_sources,
  c_args : [gst_rockchip_args, '-Wextra'],
  include_directories : configinc,
  dependencies : [gstbase_dep, gstvideo_dep, gstallocators_dep, gstpbutils_dep, mpp_dep, rga_dep],
  install : true,
  install_dir : plugins_install_dir,
)

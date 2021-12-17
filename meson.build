project('gst-plugins-bad', 'c',
  version : '1.14.4',
  license : 'LGPL',
  meson_version : '>= 0.47',
  default_options : [ 'warning_level=3',
                      'buildtype=debugoptimized' ])

gst_version = meson.project_version()
version_arr = gst_version.split('.')
gst_version_major = version_arr[0].to_int()
gst_version_minor = version_arr[1].to_int()
gst_version_micro = version_arr[2].to_int()

glib_req = '>= 2.32'
gst_req = '>= @0@.@1@.0'.format(gst_version_major, gst_version_minor)

api_version = '1.0'
soversion = 0
# maintaining compatibility with the previous libtool versioning
# current = minor * 100 + micro
libversion = '@0@.@1@.0'.format(soversion, gst_version_minor * 100 + gst_version_micro)

plugins_install_dir = '@0@/gstreamer-1.0'.format(get_option('libdir'))

cc = meson.get_compiler('c')

# Symbol visibility
if cc.has_argument('-fvisibility=hidden')
  add_project_arguments('-fvisibility=hidden', language: 'c')
endif

# Disable strict aliasing
if cc.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'c')
endif

cast_checks = get_option('gobject-cast-checks')
if cast_checks.disabled() or (cast_checks.auto())
  message('Disabling GLib cast checks')
  add_project_arguments('-DG_DISABLE_CAST_CHECKS', language: 'c')
endif

glib_asserts = get_option('glib-asserts')
if glib_asserts.disabled() or (glib_asserts.auto() and not gst_version_is_dev)
  message('Disabling GLib asserts')
  add_project_arguments('-DG_DISABLE_ASSERT', language: 'c')
endif

message('Disabling deprecated GLib API')
add_project_arguments('-DG_DISABLE_DEPRECATED', language: 'c')

add_project_arguments('-DUNUSED=__attribute__((unused))', language: 'c')

cdata = configuration_data()

cdata.set_quoted('VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-rockchip')
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE_NAME', 'GStreamer Rockchip Plug-ins')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('LIBDIR', join_paths(get_option('prefix'), get_option('libdir')))

# GStreamer package name and origin url
gst_package_name = get_option('package-name')
if gst_package_name == ''
  gst_package_name = 'GStreamer Rockchip Plug-ins'
endif
cdata.set_quoted('GST_PACKAGE_NAME', gst_package_name)
cdata.set_quoted('GST_PACKAGE_ORIGIN', get_option('package-origin'))

# Mandatory GST deps
gst_dep = dependency('gstreamer-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : gst_req,
  fallback : ['gstreamer', 'gst_base_dep'])
gstallocators_dep = dependency('gstreamer-allocators-1.0', version : gst_req,
  fallback : ['gst-plugins-base', 'allocators_dep'])
gstvideo_dep = dependency('gstreamer-video-1.0', version : gst_req,
  fallback : ['gst-plugins-base', 'video_dep'])
gstpbutils_dep = dependency('gstreamer-pbutils-1.0', version : gst_req,
  fallback : ['gst-plugins-base', 'pbutils_dep'])

glib_dep = dependency('glib-2.0', version : glib_req,
  fallback : ['glib', 'libglib_dep'])
x11_dep = dependency('x11', required : get_option('rkximage'))
drm_dep = dependency('libdrm', required : get_option('rkximage'))
mpp_dep = dependency('rockchip_mpp', required : get_option('rockchipmpp'))
rga_dep = dependency('librga', required : get_option('rga'))

if rga_dep.found()
  cdata.set10('HAVE_RGA', 1)
endif

if cc.has_header_symbol('gst/video/video-format.h', 'GST_VIDEO_FORMAT_NV12_10LE40', dependencies : gstvideo_dep)
  cdata.set10('HAVE_NV12_10LE40', 1)
endif

gst_rockchip_args = ['-DHAVE_CONFIG_H']
configinc = include_directories('.')

subdir('gst')

configure_file(output : 'config.h', configuration : cdata)

python = import('python').find_installation()
run_command(python, '-c', 'import shutil; shutil.copy("hooks/pre-commit.hook", ".git/hooks/pre-commit")')

project(
    'librga',
    'cpp',
    version : '2.0.0',
    meson_version : '>=0.44.0',
    default_options : ['warning_level=3', 'cpp_std=c++14']
)

pkgconfig = import('pkgconfig')

libdrm_dep = dependency('', required : false)
libdrm_option = get_option('libdrm')
if libdrm_option != 'false'
    libdrm_dep = dependency('libdrm', version : '>= 2.4.0')
    if libdrm_option == 'true' and not libdrm_dep.found()
        error('libdrm requested, but not found.')
    endif
endif

libpthread_dep = dependency('libpthread', required : true)

if libdrm_dep.found()
    message('Building with libdrm.')
    add_project_arguments('-DLIBDRM=1', language : 'cpp')
else
    message('Building without libdrm.')
endif

add_project_arguments('-DLINUX=1', language : 'cpp')

librga_srcs = [
    'core/GrallocOps.cpp',
    'core/NormalRgaApi.cpp',
	'core/NormalRga.cpp',
	'core/RgaUtils.cpp',
	'core/RockchipRga.cpp',
]

incdir = include_directories('include')

librga = shared_library(
    'rga',
    librga_srcs,
    dependencies : libdrm_dep,
	include_directories : incdir,
    version : meson.project_version(),
    install : true,
)

install_headers(
    'include/rga.h',
    'include/drmrga.h',
    'include/GrallocOps.h',
    'include/RockchipRga.h',
    'include/RgaMutex.h',
	'include/RgaSingleton.h',
	'include/RgaUtils.h',
    subdir : 'rga',
)

pkgconfig.generate(
    libraries : librga,
    filebase : 'librga',
    name : 'librga',
    version : meson.project_version(),
    description : 'Userspace interface to Rockchip RGA 2D accelerator',
)

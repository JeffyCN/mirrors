project(
    'librga',
    'cpp',
    version : '2.1.0',
    meson_version : '>=0.47.0',
    default_options : ['warning_level=3', 'cpp_std=c++14']
)

pkgconfig = import('pkgconfig')

libthreads_dep = dependency('threads')

add_project_arguments('-DLINUX=1', language : 'cpp')

librga_srcs = [
    'core/utils/android_utils/src/android_utils.cpp',
    'core/utils/drm_utils/src/drm_utils.cpp',
    'core/utils/utils.cpp',
    'core/GrallocOps.cpp',
    'core/NormalRgaApi.cpp',
	'core/NormalRga.cpp',
	'core/RgaUtils.cpp',
	'core/RockchipRga.cpp',
	'core/RgaApi.cpp',
	'core/rga_sync.cpp',
    'im2d_api/src/im2d_log.cpp',
    'im2d_api/src/im2d_impl.cpp',
	'im2d_api/src/im2d.cpp',
]

incdir = include_directories('include', 'im2d_api', 'core/hardware', 'core/utils', 'core/3rdparty/libdrm/include/drm')

librga = shared_library(
    'rga',
    librga_srcs,
    dependencies : [libthreads_dep],
	include_directories : incdir,
    version : meson.project_version(),
    cpp_args : ['-w'],
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
	'include/RgaApi.h',
	'im2d_api/im2d.h',
	'im2d_api/im2d.hpp',
    'im2d_api/im2d_type.h',
    'im2d_api/im2d_version.h',
    'im2d_api/im2d_buffer.h',
    'im2d_api/im2d_common.h',
    'im2d_api/im2d_single.h',
    'im2d_api/im2d_task.h',
    'im2d_api/im2d_mpi.h',
    'im2d_api/im2d_expand.h',
    subdir : 'rga',
)

pkgconfig.generate(
    libraries : librga,
    filebase : 'librga',
    name : 'librga',
    version : meson.project_version(),
    description : 'Userspace interface to Rockchip RGA 2D accelerator',
)

librga_demo_option = get_option('librga_demo')
if librga_demo_option != 'false'
    demo_src = [
		'samples/im2d_api_demo/rgaImDemo.cpp',
		'samples/im2d_api_demo/args.cpp'
	]
    demo_incdir = include_directories('include', 'im2d_api')
    librga_dep = dependency('librga')
    executable(
            'rgaImDemo',
            demo_src,
            include_directories : demo_incdir,
            dependencies : librga_dep,
            cpp_args : ['-Wno-pedantic'],
            install : true,
    )
endif

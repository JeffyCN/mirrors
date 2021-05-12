project(
    'librga',
    'cpp',
    version : '2.1.0',
    meson_version : '>=0.47.0',
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

if libdrm_dep.found()
    message('Building with libdrm.')
    add_project_arguments('-DLIBDRM=1', language : 'cpp')
else
    message('Building without libdrm.')
endif

libthreads_dep = dependency('threads')

gen_version = vcs_tag(
	command : ['./genversion.sh', 'meson'],
	replace_string : ['$GIT_BUILD_VERSION'],
	input : 'version.h.template', output : 'version.h',
 )
add_project_arguments('-DLINUX=1', language : 'cpp')

librga_srcs = [
	gen_version,
    'core/GrallocOps.cpp',
    'core/NormalRgaApi.cpp',
	'core/NormalRga.cpp',
	'core/RgaUtils.cpp',
	'core/RockchipRga.cpp',
	'core/RgaApi.cpp',
	'im2d_api/im2d.cpp',
]

incdir = include_directories('include')

librga = shared_library(
    'rga',
    librga_srcs,
    dependencies : [libdrm_dep, libthreads_dep],
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
	'include/RgaApi.h',
	'im2d_api/im2d.h',
	'im2d_api/im2d.hpp',
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

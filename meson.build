project(
    'librga',
    'cpp',
    version : '2.0.0',
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

git_version_h = vcs_tag(
    command : ['git', 'rev-parse', '--short', 'HEAD'],
    input : 'version.h.in', output : 'version.h',
)
add_project_arguments('-include', 'version.h', language : 'cpp')

librga_srcs = [
    git_version_h,
    'RgaApi.cpp',
    'RockchipRga.cpp',
    'normal/NormalRga.cpp',
    'normal/NormalRgaApi.cpp',
]

librga = shared_library(
    'rga',
    librga_srcs,
    dependencies : libdrm_dep,
    version : meson.project_version(),
    install : true,
)

install_headers(
    'rga.h',
    'drmrga.h',
    'RgaApi.h',
    'RockchipRga.h',
    'RockchipRgaMacro.h',
    subdir : 'rga',
)

pkgconfig.generate(
    libraries : librga,
    filebase : 'librga',
    name : 'librga',
    version : meson.project_version(),
    description : 'Userspace interface to Rockchip RGA 2D accelerator',
    requires_private : libdrm_dep,
)

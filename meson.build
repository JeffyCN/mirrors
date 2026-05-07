project(
  'libmali', 'c',
  version : '1.10.0',
  meson_version : '>=0.53.0')

mali_version = meson.project_version()

fs = import('fs')
pkgconfig = import('pkgconfig')

cc = meson.get_compiler('c')

library_path = get_option('library')
if library_path != ''
  if not fs.exists(meson.current_source_dir() / library_path)
    error('Could not find the specified library at: ' + library_path)
  endif

  if library_path.contains('aarch64')
    arch = 'aarch64'
  elif library_path.contains('arm')
    arch = 'arm'
  else
    arch = 'aarch64'
  endif
else
  if get_option('arch') != 'auto'
    arch = get_option('arch')
  else
    arch = host_machine.cpu_family()
  endif

  if arch == 'armv7l' or arch == 'armhf' or arch == 'arm32'
    arch = 'arm'
  elif arch == 'armv8' or arch == 'arm64'
    arch = 'aarch64'
  endif

  gpu = get_option('gpu')
  platform = get_option('platform')

  # Use gles platform for OpenGLES only (without OpenCL)
  if platform == 'nocl'
    platform = 'gles'
  endif

  message('Building for ' + '|'.join([arch, gpu, platform]))

  # Grab libraries with specified configs
  name_pattern = 'libmali-' + gpu + '-[^-]*'
  if platform == 'gles' or platform == 'cl'
    name_pattern += '-' + platform
  endif
  name_pattern += '\.so'

  cmd = run_command('bash', '-c',
    'cd ' + meson.current_source_dir() + ' && ' +
    'find lib -regex "lib/' + arch + '-.*/' + name_pattern +
    '" 2>/dev/null | head -1', check : false)

  library_path = cmd.stdout().strip()
  if library_path == ''
      error('No library found!')
  endif
endif

message('Building for ' + library_path)

# Load original mali library for later function checks and linking
run_command('ln', '-rsf', meson.current_source_dir() / library_path,
  meson.current_build_dir() / 'libmali-prebuilt.so', check : false)
mali = cc.find_library('mali-prebuilt', dirs : meson.current_build_dir())

message('Using ' + library_path)

opencl_icd = get_option('opencl-icd')
vendor_package = get_option('vendor-package')
build_hook = get_option('hooks')
wrappers = get_option('wrappers')

# The hook library is for hacking GBM/EGL APIs
has_gbm = cc.has_function('gbm_create_device', dependencies : mali)
has_egl = cc.has_function('eglCreateContext', dependencies : mali)
if not has_gbm and not has_egl
  build_hook = false
endif

if wrappers
  message('Provide wrappers')

  # Wrappers would link to the real library.
  add_project_link_arguments('-Wl,--no-as-needed', language : 'c')
else
  # The vendor package requires soname of wrappers.
  if vendor_package
    error('Cannot provide vendor package without wrappers')
  endif
endif

# Install wrapper libraries into vendor dir
if vendor_package
  message('Build vendor package')
  wrapper_libdir = get_option('libdir') / 'mali'
else
  wrapper_libdir = get_option('libdir')
endif

# Wrap library name : version
gbm_wrappers = {'gbm' : '1'}
egl_wrappers = {'EGL' : '1'}
glesv1_wrappers = {'GLESv1_CM' : '1'}
glesv2_wrappers = {'GLESv2' : '2'}
cl_wrappers = opencl_icd ? {'MaliOpenCL' : '1'} : {'OpenCL' : '1'}
vk_wrappers = {'MaliVulkan' : '1'}

# Provide newer GBM version with hook library
if build_hook
  gbm_version = '23.1.3'
elif cc.has_function('gbm_bo_get_fd_for_plane', dependencies : mali)
  gbm_version = '21.1.0'
elif cc.has_function('gbm_bo_get_modifier', dependencies : mali)
  gbm_version = '17.1.0'
else
  gbm_version = '10.4.0'
endif

# Source dir : dest dir
gbm_headers = {
  'include/GBM' / gbm_version : '',
}
egl_headers = {
  'include/KHR' : 'KHR',
  'include/EGL' : 'EGL',
}
glesv1_headers = {
  'include/KHR' : 'KHR',
  'include/GLES' : 'GLES',
}
glesv2_headers = {
  'include/KHR' : 'KHR',
  'include/GLES2' : 'GLES2',
  'include/GLES3' : 'GLES3',
}
cl_headers = {
  'include/CL' : 'CL',
}
vulkan_headers = {
  'include/vk_video' : 'vk_video',
  'include/vulkan' : 'vulkan',
}

if cc.has_function('clCreateBufferWithProperties', dependencies : mali)
  cl_version = '3.0'
  cl_target_version = '300'
elif cc.has_function('clSetProgramReleaseCallback', dependencies : mali)
  cl_version = '2.2'
  cl_target_version = '220'
elif cc.has_function('clCloneKernel', dependencies : mali)
  cl_version = '2.1'
  cl_target_version = '210'
elif cc.has_function('clCreatePipe', dependencies : mali)
  cl_version = '2.0'
  cl_target_version = '200'
else
  cl_version = '1.2'
  cl_target_version = '120'
endif

# Package name : required symbol, wrappers, headers, package version
map = {
  'gbm' : ['gbm_create_device', gbm_wrappers, gbm_headers, gbm_version],
  'egl' : ['eglCreateContext', egl_wrappers, egl_headers, '7.10'],
  'glesv1_cm' : ['eglCreateContext', glesv1_wrappers, glesv1_headers, '7.10'],
  'glesv2' : ['eglCreateContext', glesv2_wrappers, glesv2_headers, '7.10'],
  'OpenCL' : ['clCreateContext', cl_wrappers, cl_headers, cl_version],
  'vulkan' : ['vk_icdGetInstanceProcAddr', vk_wrappers, vulkan_headers,
    mali_version],
}

# Create dummy source for building dummy libraries
dummy_source = join_paths(meson.current_build_dir(), 'dummy.c')
run_command('touch', dummy_source, check : false)

# Create a dummy library which will be replaced by the prebuilt mali library
libmali = shared_library(
  'mali',
  dummy_source,
  install : true,
  version : mali_version)

libhook = []
if build_hook
  # Build hook library
  subdir('hook')

  # Recommend to link hook library before libmali
  mali_ldflags = libhook_ldflags
else
  mali_ldflags = []
endif

mali_ldflags += ['-L${libdir}', '-lmali']

pkgconfig.generate(
  libraries : mali_ldflags,
  name : 'mali',
  description : 'Mali GPU User-Space Binary Driver')

foreach name, values : map
  symbol = values[0]
  wrapper_libs = values[1]
  headers = values[2]
  pkg_version = values[3]
  pkg_cflags = []
  is_opencl_icd = opencl_icd and name == 'OpenCL'
  is_vulkan_icd = name == 'vulkan'

  if not cc.has_function(symbol, dependencies : mali)
    continue
  endif

  foreach wrapper, version : wrapper_libs
    shared_library(
      wrapper,
      dummy_source,
      link_with : [libhook, libmali],
      install : true,
      install_dir : wrapper_libdir,
      version : version)
  endforeach

  # Install ICD OpenCL vendor config
  if is_opencl_icd
    custom_target(
      'OpenCL vendor icd',
      output : 'mali.icd',
      command : ['echo', 'libMaliOpenCL.so.1'],
      capture : true,
      install_dir : get_option('sysconfdir') / 'OpenCL' / 'vendors',
      install : true)
  endif

  # Install ICD Vulkan vendor config
  if is_vulkan_icd
    custom_target(
      'Vulkan vendor icd',
      input : 'data/vulkan/mali.json.in',
      output : 'mali.json',
      command : ['sed', 's/@LIB@/libMaliVulkan.so.1/', '@INPUT@'],
      capture : true,
      install_dir : get_option('datadir') / 'vulkan' / 'icd.d',
      install : true)
  endif

  if is_vulkan_icd
    # Install Vulkan WSI config
    custom_target(
      'Vulkan manifest layer file',
      input : 'data/vulkan/VkLayer_window_system_integration.json.in',
      output : 'VkLayer_window_system_integration.json',
      command : ['sed', 's/@LIB@/libVkLayer_window_system_integration.so/', '@INPUT@'],
      capture : true,
      install_dir : get_option('datadir') / 'vulkan' / 'implicit_layer.d',
      install : true)

    # Install Vulkan WSI library
    lib_source = join_paths('data/vulkan/lib', arch, 'libVkLayer_window_system_integration.so')
    install_data(
        lib_source,
        install_dir: get_option('libdir')
    )
  endif

  # No {headers, pkgconfig} for OpenCL ICD and vendor packages
  if is_opencl_icd or vendor_package
    continue
  endif

  foreach src, dst : headers
    install_subdir(
      src,
      install_dir : get_option('includedir') / dst,
      install_mode : ['rw-r--r--', 'root'],
      strip_directory : true)
  endforeach

  if name == 'gbm'
    # Hack unsupported gbm_bo flags
    meson.add_install_script('scripts/fixup_gbm.sh', get_option('includedir'))
  endif

  if name == 'OpenCL'
    pkg_cflags += ['-DCL_TARGET_OPENCL_VERSION=' + cl_target_version]

    meson.add_install_script('scripts/fixup_cl.sh',
      get_option('includedir'), cl_version)
  endif

  # Vulkan ICD does not provide pkg-config metadata.
  if is_vulkan_icd
    continue
  endif

  pkgconfig.generate(
    libraries : mali_ldflags,
    extra_cflags : pkg_cflags,
    version : pkg_version,
    name : name,
    description : 'Mali GPU User-Space Binary Driver Wrappers')
endforeach

# Install RT thread priority config
custom_target(
  'RT thread priority config',
  output : 'mali-priority.sh',
  command : ['echo', 'export MALI_SCHED_RT_THREAD_PRIORITY=95'],
  capture : true,
  install_dir : get_option('sysconfdir') / 'profile.d',
  install : true)

if vendor_package
  # Install vendor ld config
  custom_target(
    'vendor ld config',
    output : '00-' + arch + '-mali.conf',
    command : ['echo', get_option('prefix') / wrapper_libdir],
    capture : true,
    install_dir : '/etc/ld.so.conf.d',
    install : true)
elif get_option('khr-header')
  # Install optional KHR header
  install_data(
    'include/KHR/mali_khrplatform.h',
    install_dir : get_option('includedir') / 'KHR',
    install_mode : ['rw-r--r--', 'root'],
    rename : 'khrplatform.h')
endif

# Install target library
install_data(library_path, install_dir : get_option('libdir'))

# Replace dummy libmali library
meson.add_install_script('scripts/fixup_dummy.sh',
  get_option('libdir'), library_path)

if not wrappers
  # Disable wrappers
  meson.add_install_script('scripts/fixup_nowrap.sh', get_option('libdir'))
endif

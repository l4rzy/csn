## common
project('csn', 'c',
  version : '0.0.1',
  default_options : ['warning_level=1'])

## compiler flags
lib_args = ['-DENABLE_DEBUG', '-Wall', '-O0']

## dirs
incdir = include_directories('inc')
srcdir = include_directories('src')

## dependencies
curl_dep = dependency('libcurl')
tidy_dep = dependency('tidy')

lib_deps = [curl_dep, tidy_dep]

## src
lib_sources = files(
  'src/csn.c',
  'src/parser.c',
  'src/util.c'
)

shlib = shared_library('csn', lib_sources,
  install : true,
  c_args : lib_args,
  include_directories : incdir,
  dependencies: lib_deps,
)
## tests

# Make this library usable as a Meson subproject.
csn_dep = declare_dependency(
  include_directories: include_directories('inc'),
  link_with : shlib)

# Make this library usable from the system's
# package manager.
install_headers('inc/csn.h', subdir : 'csn')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'csn',
  filebase : 'csn',
  description : 'C library for chiasenhac.com',
  subdirs : 'csn',
  libraries : shlib,
  version : '0.0.1',
)

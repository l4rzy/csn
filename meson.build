## common
project('csn', 'c',
  version : '0.1.0',
  default_options : ['warning_level=1'])

csn_version = meson.project_version()

## compiler flags
lib_args = ['-DENABLE_DEBUG', '-Wall', '-O0', '-fuse-ld=gold']

## dirs
incdir = include_directories('inc')
srcdir = include_directories('src')

## dependencies
curl_dep = dependency('libcurl')
tidy_dep = dependency('tidy', version: '>=5.7')

lib_deps = [curl_dep, tidy_dep]

## src
lib_sources = files(
  'src/csn.c',
  'src/parser.c',
  'src/buf.c',
  'src/xpath.c',
  'src/util.c',
)

shlib = shared_library('csn', lib_sources,
  install : true,
  c_args : lib_args,
  include_directories : incdir,
  dependencies: lib_deps,
  version: csn_version.split('.')[0],
)
## tests
test_buf = executable('test_buf',
  files('test/buf.c'),
  link_with: shlib,
  include_directories: incdir,
)

test_queue = executable('test_queue',
  files('test/queue.c'),
  link_with: shlib,
  include_directories: incdir,
)

test_xpath = executable('test_xpath',
  files('test/xpath.c'),
  link_with: shlib,
  include_directories: incdir,
)

test('buf_t functions', test_buf)
test('queue functions', test_queue)
test('xpath functions', test_xpath)

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
  version : csn_version,
)

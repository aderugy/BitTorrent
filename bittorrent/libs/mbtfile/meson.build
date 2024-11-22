mbtfile_inc = [ include_directories('include/') ]

mbtfile = shared_library(
  'mbtfile',
  sources : [ 'src/init.c', 'src/free.c' ],
  include_directories : mbtfile_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep ],
  install : true,
)

test_exe = executable(
  'file_test',
  sources : ['tests/main.c'],
  include_directories : mbtfile_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep ],
  link_with: mbtfile
)

test('mbtfile_test', test_exe)

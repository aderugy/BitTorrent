mbtnet_inc = [ include_directories('include/') ]

curl_dep = dependency('libcurl')

mbtnet = shared_library(
  'mbtnet',
  sources : [ 'src/peer_addr.c', 'src/alloc.c', 'src/peers.c', 'src/tracker.c', 'src/leech.c' ],
  include_directories : mbtnet_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, curl_dep ],
  install : true,
)

test_exe = executable(
  'test_main',
  sources : ['tests/main.c'],
  include_directories : mbtnet_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, curl_dep ],
  link_with: mbtnet
)

test('mbtnet_test', test_exe)

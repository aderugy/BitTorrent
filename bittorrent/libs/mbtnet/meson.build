mbtnet_inc = [ include_directories('include/') ]

curl_dep = dependency('libcurl')

mbtnet = shared_library(
  'mbtnet',
  sources : [ 'src/net/server.c', 'src/net/client.c', 'src/msg.c', 'src/peer_addr.c', 'src/alloc.c', 'src/peers.c', 'src/tracker.c', 'src/leech.c' ],
  include_directories : mbtnet_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, mbtfile_dep, curl_dep ],
  install : true,
)

mbtnet_dep = declare_dependency (
  include_directories : mbtnet_inc,
  link_with : mbtnet,
  dependencies : [ mbtutils_dep, mbtbe_dep, mbtfile_dep, curl_dep ],
)

test_exe = executable(
  'test_main',
  sources : ['tests/main.c'],
  include_directories : mbtnet_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, mbtfile_dep, curl_dep ],
  link_with: mbtnet
)

test('mbtnet_test', test_exe)

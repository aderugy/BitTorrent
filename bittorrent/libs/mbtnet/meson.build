mbtnet_inc = [ include_directories('include/') ]

curl_dep = dependency('libcurl')

mbtnet = shared_library(
  'mbtnet',
  sources: ['./src/fifo.c', './src/alloc.c', './src/msg/cancel.c', './src/msg/choke.c', './src/msg/handler.c', './src/msg/have.c', './src/msg/interested.c', './src/msg/keepalive.c', './src/msg/bitfield.c', './src/msg/piece.c', './src/msg/request.c', './src/msg/msg.c', './src/peers.c', './src/tracker.c', './src/print.c', './src/leech.c', './src/net/server.c', './src/net/utils.c', './src/net/client.c', './tests/main.c'],
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

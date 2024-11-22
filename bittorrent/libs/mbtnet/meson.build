mbtnet_inc = [ include_directories('include/') ]

curl_dep = dependency('libcurl')

mbtnet = shared_library(
  'mbtnet',
  sources : [ 'src/init.c' ],
  include_directories : mbtnet_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, curl_dep ],
  install : true,
)

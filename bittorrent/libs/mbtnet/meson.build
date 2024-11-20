mbtnet_inc = [ include_directories('include/') ]

mbtnet = shared_library(
  'mbtnet',
  # source,
  include_directories = mbtnet_inc,
  dependencies : [ mbtutils_dep ],
  install : true,
)

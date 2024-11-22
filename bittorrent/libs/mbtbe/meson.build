mbtbe_inc = [ include_directories('include/') ]

mbtbe = shared_library(
  'mbtbe',
  sources: [ 'src/decode.c', 'src/encode.c', 'src/node.c', 'src/mbt_be_parse_torrent_file.c'],
  include_directories : mbtbe_inc + [ include_directories('src') ],
  dependencies : [ mbtutils_dep ],
  install: true,
)

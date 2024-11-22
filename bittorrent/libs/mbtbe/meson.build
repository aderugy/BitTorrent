mbtbe_inc = [ include_directories('include/') ]

mbtbe = shared_library(
  'mbtbe',
  sources: [ 'src/decode.c', 'src/encode.c', 'src/node.c', 'src/mbt_be_parse_torrent_file.c', 'src/mbt_torrent_free.c', 'src/mbt_torrent_init.c', 'src/mbt_be_make_torrent_file.c' ],
  include_directories : mbtbe_inc + [ include_directories('src') ],
  dependencies : [ mbtutils_dep ],
  install: true,
)

mbtbe_dep = declare_dependency (
  include_directories : mbtbe_inc,
  link_with : mbtbe
)

subdir('tests')

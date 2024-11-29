mbtutils_inc = [ include_directories('include/') ]

openssl_dep = dependency('openssl')

mbtutils = shared_library(
  'mbtutils',
  sources: [ 'src/logger.c', 'src/hash.c', 'src/random.c', 'src/xalloc.c', 'src/mbt_str_init.c', 'src/mbt_cview_cmp.c', 'src/mbt_cview_contains.c', 'src/mbt_cview_fprint.c', 'src/mbt_str_ctor.c', 'src/mbt_str_dtor.c', 'src/mbt_str_free.c', 'src/mbt_str_pushc.c', 'src/mbt_str_pushcstr.c', 'src/mbt_str_pushcv.c', 'src/mbt_str_read_file.c', 'src/parse.c' ],
  include_directories : mbtutils_inc,
  dependencies: [ openssl_dep ],
  install: true,
)

mbtutils_dep = declare_dependency(
  include_directories : mbtutils_inc,
  dependencies: [ openssl_dep ],
  link_with : mbtutils,
)

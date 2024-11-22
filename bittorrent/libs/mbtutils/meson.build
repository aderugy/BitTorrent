mbtutils_inc = [ include_directories('include/') ]

#subdir('./include/mbt/utils/mbtstr/')

mbtutils = shared_library(
  'mbtutils',
  sources: [ 'src/mbt_str_init.c', 'src/mbt_cview_cmp.c', 'src/mbt_cview_contains.c', 'src/mbt_cview_fprint.c', 'src/mbt_str_ctor.c', 'src/mbt_str_dtor.c', 'src/mbt_str_free.c', 'src/mbt_str_pushc.c', 'src/mbt_str_pushcstr.c', 'src/mbt_str_pushcv.c', 'src/mbt_str_read_file.c' ],
  include_directories : mbtutils_inc,
  install: true,
)

mbtutils_dep = declare_dependency(
  include_directories : mbtutils_inc,
  link_with : mbtutils,
)

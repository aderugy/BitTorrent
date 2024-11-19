project(
  'mbtstr', # project name
  'c', # project language
  version : '1.0.0', # project version
  default_options : [
    'c_args=-D_GNU_SOURCE',
    'c_std=c99', # -std=c99
    'warning_level=2', # -Wall -Wextra
    'werror=true', # -Werror
    'debug=false',
    'buildtype=plain',
  ],
)

mbtstr_inc = [ include_directories('include') ]

mbtstr = shared_library(
  'mbtstr',
  sources: [ 'src/mbt_str_free.c', 'src/mbt_cview_cmp.c', 'src/mbt_cview_contains.c', 'src/mbt_cview_fprint.c', 'src/mbt_str_ctor.c', 'src/mbt_str_dtor.c', 'src/mbt_str_free.c', 'src/mbt_str_pushc.c', 'src/mbt_str_pushcstr.c', 'src/mbt_str_pushcv.c' ],
  include_directories : mbtstr_inc + [ include_directories('src') ],
  install: true,
)

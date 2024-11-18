project(
  'mbtstr', # project name
  'c', # project language
  meson_version : '>= 1.4.2', # meson minimum version
  version : '1.0.0', # project version
  default_options : [
    'c_args=-D_GNU_SOURCE',
    'c_std=c99', # -std=c99
    'debug=true', # -g
    'optimization=0', # -O0
    'warning_level=3', # -Wall -Wextra -Wpedantic
    'werror=true', # -Werror
    'b_sanitize=address,undefined', # -fsanitize=address,undefined
  ],
)

mbtstr_inc = [ include_directories('include') ]

mbtstr = shared_library(
  'mbtstr',
  sources: [ 'src/mbt_cview_cmp.c', 'src/mbt_cview_contains.c', 'src/mbt_cview_fprin.c', 'src/mbt_str.c', 'src/mbt_str_ctor.c', 'src/mbt_str_dtor.c', 'src/mbt_str_free.c', 'src/mbt_str_pushc.c', 'src/mbt_str_pushcstr.c', 'src/mbt_str_pushcv.c' ],
  include_directories : mbtstr_inc + [ include_directories('src') ],
  install : true,
)

mbtstr_dep = declare_dependency(
  include_directories: mbtstr_inc,
  link_with : mbtstr
)

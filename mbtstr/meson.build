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
  sources: [ 'src/test.c' ],
  include_directories : mbtstr_inc + [ include_directories('src') ],
  install : true,
)

mbtstr_dep = declare_dependency(
  include_directories: mbtstr_inc,
  link_with : mbtstr
)

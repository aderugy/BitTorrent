project(
  'mbt', # project name
  'c', # project language
  version : '1.0.0', # project version
  default_options : [
    'c_args=-D_GNU_SOURCE',
    'c_std=c99', # -std=c99
    'warning_level=2', # -Wall -Wextra
    'werror=true', # -Werror
    'debug=true',
    'buildtype=debug',
    'b_sanitize=address,undefined',
  ],
)

subdir('libs/mbtutils')
subdir('libs/mbtbe')
subdir('libs/mbtfile')
subdir('libs/mbtnet')

src = ['src/main.c']

mbt_inc = [ include_directories('src') ]
executable(
  'mbt', 
  sources : src,
  include_directories: mbt_inc,
  dependencies : [ mbtutils_dep, mbtbe_dep, mbtfile_dep, mbtnet_dep ],
  install: true,
)

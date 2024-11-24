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

subdir('mbtutils')
subdir('mbtbe')
subdir('mbtfile')
subdir('mbtnet')

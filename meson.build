project('cat', 'c',
  version: '0.0.1',
  default_options: [
    'c_std=gnu89',
    'warning_level=2',
    'werror=true',
  ],
)

deps = [
  dependency('glib-2.0'),
]

cc_flags = [
  '-Wall',
  '-O0',
  '-D_GNU_SOURCE',
]

cat_lib = static_library(
  'cat_lib',
  [ 'cat_impl.c' ],
  dependencies: [ deps ],
)

executable('cat',
  'cat.c',
  dependencies: [ deps ],
  link_whole: [ cat_lib ],
)

test_bin = executable(
  'cat_test',
  [ 'cat_test.c' ],
  dependencies: [ deps ],
  link_whole: [ cat_lib ],
)

test('cat_test', test_bin)

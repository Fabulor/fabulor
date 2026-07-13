fishlim_test_sources = [
  'tests.c',
  'mock-keystore.c',
  '../fish.c',
  '../utils.c',
  '../dh1080.c',
]

fishlim_tests = executable('fishlim_tests', fishlim_test_sources,
  dependencies: [libgio_dep, libssl_dep, zoitechat_plugin_dep],
  include_directories: include_directories('..'),
)

test('Fishlim Tests', fishlim_tests,
  protocol: 'tap',
  timeout: 600,
)

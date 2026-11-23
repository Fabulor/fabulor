project(
  'fabulor-gtk4-probe',
  'c',
  version: '1.0.0',
  default_options: ['c_std=c11', 'warning_level=3', 'werror=true'],
)

cc = meson.get_compiler('c')
python = find_program('python3', 'python')
gtk4_root = get_option('gtk4_root')
gtk4_staged_root = get_option('gtk4_staged_root')

if gtk4_root == ''
  error('Set -Dgtk4_root to the absolute Fabulor GTK4 dependency root.')
endif
if gtk4_staged_root == ''
  error('Run stage_runtime.py and set -Dgtk4_staged_root to its output directory.')
endif

validator = join_paths(meson.current_source_dir(), 'validate_root.py')
validation = run_command(
  python,
  validator,
  '--root', gtk4_root,
  check: false,
)
if validation.returncode() != 0
  error(validation.stderr().strip())
endif
message(validation.stdout().strip())

gtk4_includes = include_directories(
	join_paths(gtk4_root, 'include'),
  join_paths(gtk4_root, 'include', 'gtk-4.0'),
  join_paths(gtk4_root, 'include', 'glib-2.0'),
  join_paths(gtk4_root, 'lib', 'glib-2.0', 'include'),
  join_paths(gtk4_root, 'include', 'pango-1.0'),
  join_paths(gtk4_root, 'include', 'harfbuzz'),
  join_paths(gtk4_root, 'include', 'cairo'),
  join_paths(gtk4_root, 'include', 'gdk-pixbuf-2.0'),
  join_paths(gtk4_root, 'include', 'graphene-1.0'),
  join_paths(gtk4_root, 'lib', 'graphene-1.0', 'include'),
  is_system: true,
)

gtk4_library_dir = join_paths(gtk4_root, 'lib')
gtk4_library = cc.find_library('gtk-4', dirs: gtk4_library_dir, required: true)
pango_library = cc.find_library('pango-1.0', dirs: gtk4_library_dir, required: true)
pangocairo_library = cc.find_library('pangocairo-1.0', dirs: gtk4_library_dir, required: true)
gdk_pixbuf_library = cc.find_library('gdk_pixbuf-2.0', dirs: gtk4_library_dir, required: true)
cairo_library = cc.find_library('cairo', dirs: gtk4_library_dir, required: true)
graphene_library = cc.find_library('graphene-1.0', dirs: gtk4_library_dir, required: true)
gio_library = cc.find_library('gio-2.0', dirs: gtk4_library_dir, required: true)
gobject_library = cc.find_library('gobject-2.0', dirs: gtk4_library_dir, required: true)
glib_library = cc.find_library('glib-2.0', dirs: gtk4_library_dir, required: true)
intl_library = cc.find_library('intl', dirs: gtk4_library_dir, required: true)
probe_dependencies = [gtk4_library, pango_library, pangocairo_library, gdk_pixbuf_library, cairo_library, graphene_library, gio_library, gobject_library, glib_library, intl_library]

if host_machine.system() == 'windows'
  probe_dependencies += [cc.find_library('comctl32', required: true)]
endif

probe = executable(
  'fabulor-gtk4-probe',
  'probe.c',
  '../../src/fe-gtk/file-chooser-path.c',
  '../../src/fe-gtk/emoji-picker.c',
  '../../src/common/gtk4-theme-discovery.c',
  '../../src/common/gtk4-theme-preferences.c',
  '../../src/common/theme-archive-reader.c',
  '../../src/fe-gtk/theme/theme-gtk4.c',
  '../../src/fe-gtk/theme/theme-gtk4-controller.c',
  '../../src/fe-gtk/theme/theme-preferences-gtk4.c',
  '../../src/fe-gtk/theme/theme-appearance-monitor-gtk4.c',
  '../../src/fe-gtk/tray-action-model.c',
  '../../src/fe-gtk/tray-backend-policy.c',
  '../../src/fe-gtk/tray-menu-composition.c',
  '../../src/fe-gtk/tray-menu-presenter-gtk4.c',
  '../../src/fe-gtk/context-menu-presenter-gtk4.c',
  '../../src/fe-gtk/url-context-menu-model.c',
  '../../src/fe-gtk/channel-context-menu-model.c',
  '../../src/fe-gtk/channel-list-context-menu-model.c',
  '../../src/fe-gtk/nick-context-menu-model.c',
  '../../src/fe-gtk/middle-context-menu-model.c',
  '../../src/fe-gtk/tab-context-menu-model.c',
  '../../src/fe-gtk/window-state.c',
  '../../src/fe-gtk/application-main-loop.c',
  '../../src/fe-gtk/window-geometry.c',
  '../../src/fe-gtk/spell-entry-words.c',
	'../../src/fe-gtk/spell-entry-style.c',
	'../../src/fe-gtk/spell-entry-menu.c',
	'../../src/fe-gtk/spell-entry-widget.c',
  '../../src/fe-gtk/addon-list.c',
  '../../src/fe-gtk/ban-list.c',
  '../../src/fe-gtk/channel-list.c',
  '../../src/fe-gtk/dcc-chat-list.c',
  '../../src/fe-gtk/dcc-transfer-list.c',
  '../../src/fe-gtk/editable-list.c',
  '../../src/fe-gtk/print-event-list.c',
  '../../src/fe-gtk/key-binding-list.c',
  '../../src/fe-gtk/sound-event-list.c',
  '../../src/fe-gtk/preferences-category-list.c',
  '../../src/fe-gtk/server-network-list.c',
  '../../src/fe-gtk/server-entry-list.c',
  '../../src/fe-gtk/xtext-background.c',
  '../../src/fe-gtk/xtext-accessible.c',
  '../../src/fe-gtk/xtext-decoration.c',
  '../../src/fe-gtk/xtext-display.c',
  '../../src/fe-gtk/xtext-geometry.c',
  '../../src/fe-gtk/xtext-hit-test.c',
  '../../src/fe-gtk/xtext-input.c',
  '../../src/fe-gtk/xtext-performance.c',
  '../../src/fe-gtk/xtext-render-target.c',
  '../../src/fe-gtk/xtext-scroll-copy.c',
  '../../src/fe-gtk/xtext-selection.c',
  '../../src/fe-gtk/xtext-widget-class.c',
  '../../src/fe-gtk/gtk4-list-models.c',
  '../../src/fe-gtk/ignore-list.c',
  '../../src/fe-gtk/activity-list.c',
  '../../src/fe-gtk/channel-model.c',
  '../../src/fe-gtk/channel-tree-view.c',
  '../../src/fe-gtk/notify-list.c',
  '../../src/fe-gtk/user-list-model.c',
  '../../src/fe-gtk/user-list-view.c',
  '../../src/fe-gtk/url-list.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies,
)

probe_environment = environment()
probe_environment.prepend('PATH', join_paths(gtk4_root, 'bin'))
test('GTK4 dependency probe', probe, env: probe_environment)

background_cache_test = executable(
  'test-xtext-background-cache',
  'test-xtext-background-cache.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies,
)
test('XText background cache', background_cache_test, env: probe_environment)

if host_machine.system() == 'windows'
  xtext_selection_test = executable(
    'test-xtext-selection',
    'test-xtext-selection.c',
    '../../src/fe-gtk/xtext-accessible.c',
    '../../src/fe-gtk/xtext-background.c',
    '../../src/fe-gtk/xtext-decoration.c',
    '../../src/fe-gtk/xtext-display.c',
    '../../src/fe-gtk/xtext-geometry.c',
    '../../src/fe-gtk/xtext-hit-test.c',
    '../../src/fe-gtk/xtext-input.c',
    '../../src/fe-gtk/xtext-performance.c',
    '../../src/fe-gtk/xtext-render-target.c',
    '../../src/fe-gtk/xtext-scroll-copy.c',
    '../../src/fe-gtk/xtext-selection.c',
    '../../src/fe-gtk/xtext-widget-class.c',
    include_directories: [include_directories('xtext-test-support'), gtk4_includes],
    dependencies: probe_dependencies,
    c_args: ['-DWIN32'],
    override_options: ['warning_level=2'],
  )
  test('XText clipped-prefix selection', xtext_selection_test, env: probe_environment)
endif

paned_handle_test = executable(
  'test-paned-handle-size',
  'test-paned-handle-size.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies,
)
test('Paned handle size', paned_handle_test, env: probe_environment)

native_emoji_flags_test = executable(
  'test-native-emoji-flags',
  'test-native-emoji-flags.c',
  '../../src/fe-gtk/emoji-picker.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies,
)
emoji_font_config = configuration_data()
emoji_font_config.set('GTK4_STAGED_ROOT', gtk4_staged_root.replace('\\', '/'))
emoji_font_config.set('EMOJI_FONT_DIR', join_paths(meson.project_source_root(), '..', '..', 'data', 'fonts').replace('\\', '/'))
emoji_font_file = files('../../data/fonts/NotoColorEmoji.ttf')
emoji_fonts = configure_file(
  input: 'test-emoji-fonts.conf.in',
  output: 'test-emoji-fonts.conf',
  configuration: emoji_font_config,
)
emoji_environment = environment()
emoji_environment.prepend('PATH', join_paths(gtk4_staged_root, 'bin'))
emoji_environment.set('PANGOCAIRO_BACKEND', 'fc')
emoji_environment.set('FONTCONFIG_FILE', join_paths(meson.current_build_dir(), 'test-emoji-fonts.conf'))
emoji_environment.set('FONTCONFIG_PATH', join_paths(gtk4_staged_root, 'etc', 'fonts'))
test('Native emoji flags and shared actions', native_emoji_flags_test, env: emoji_environment, args: emoji_font_file)
emoji_win32_environment = environment()
emoji_win32_environment.prepend('PATH', join_paths(gtk4_staged_root, 'bin'))
emoji_win32_environment.set('PANGOCAIRO_BACKEND', 'win32')
emoji_win32_environment.set('FONTCONFIG_FILE', join_paths(meson.current_build_dir(), 'test-emoji-fonts.conf'))
emoji_win32_environment.set('FONTCONFIG_PATH', join_paths(gtk4_staged_root, 'etc', 'fonts'))
test('Native emoji flags with Win32 default font map', native_emoji_flags_test, env: emoji_win32_environment, args: emoji_font_file)

dalnet_permissions_test = executable(
  'test-dalnet-dcc-permissions',
  'test-dalnet-dcc-permissions.c',
  '../../src/fe-gtk/nick-context-menu-model.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies,
)
test('DALnet DCC permissions', dalnet_permissions_test, env: probe_environment)

spell_autocorrect_test = executable(
  'test-spell-autocorrect',
  'test-spell-autocorrect.c',
  '../../src/fe-gtk/spell-entry-autocorrect.c',
  include_directories: gtk4_includes,
  dependencies: probe_dependencies + (host_machine.system() == 'windows' ? [cc.find_library('user32')] : []),
)
test('Spell auto-correct', spell_autocorrect_test, env: probe_environment)

if host_machine.system() == 'windows'
  sysinfo_test = executable(
    'test-sysinfo-win32',
    'test-sysinfo-win32.c',
    include_directories: gtk4_includes,
    dependencies: [glib_library, cc.find_library('ole32'),
      cc.find_library('oleaut32'), cc.find_library('wbemuuid')],
  )
  test('Windows Sysinfo backend', sysinfo_test, env: probe_environment)
endif

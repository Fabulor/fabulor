project(
  'fabulor-gtk4-probe',
  'c',
  version: '1.0.0',
  default_options: ['c_std=c11', 'warning_level=3', 'werror=true'],
)

cc = meson.get_compiler('c')
python = find_program('python3', 'python')
gtk4_root = get_option('gtk4_root')

if gtk4_root == ''
  error('Set -Dgtk4_root to the absolute Fabulor GTK4 dependency root.')
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
gdk_pixbuf_library = cc.find_library('gdk_pixbuf-2.0', dirs: gtk4_library_dir, required: true)
cairo_library = cc.find_library('cairo', dirs: gtk4_library_dir, required: true)
graphene_library = cc.find_library('graphene-1.0', dirs: gtk4_library_dir, required: true)
gio_library = cc.find_library('gio-2.0', dirs: gtk4_library_dir, required: true)
gobject_library = cc.find_library('gobject-2.0', dirs: gtk4_library_dir, required: true)
glib_library = cc.find_library('glib-2.0', dirs: gtk4_library_dir, required: true)
intl_library = cc.find_library('intl', dirs: gtk4_library_dir, required: true)

probe = executable(
  'fabulor-gtk4-probe',
  'probe.c',
  '../../src/fe-gtk/emoji-picker.c',
  '../../src/common/gtk4-theme-discovery.c',
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
  '../../src/fe-gtk/channel-model.c',
  '../../src/fe-gtk/channel-tree-view.c',
  '../../src/fe-gtk/notify-list.c',
  '../../src/fe-gtk/user-list-model.c',
  '../../src/fe-gtk/user-list-view.c',
  '../../src/fe-gtk/url-list.c',
  include_directories: gtk4_includes,
  dependencies: [gtk4_library, pango_library, gdk_pixbuf_library, cairo_library, graphene_library, gio_library, gobject_library, glib_library, intl_library],
)

probe_environment = environment()
probe_environment.prepend('PATH', join_paths(gtk4_root, 'bin'))
test('GTK4 dependency probe', probe, env: probe_environment)

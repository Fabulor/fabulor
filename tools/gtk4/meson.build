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
gio_library = cc.find_library('gio-2.0', dirs: gtk4_library_dir, required: true)
gobject_library = cc.find_library('gobject-2.0', dirs: gtk4_library_dir, required: true)
glib_library = cc.find_library('glib-2.0', dirs: gtk4_library_dir, required: true)

probe = executable(
  'fabulor-gtk4-probe',
  'probe.c',
  '../../src/fe-gtk/gtk4-list-models.c',
  '../../src/fe-gtk/channel-model.c',
  '../../src/fe-gtk/channel-tree-view.c',
  '../../src/fe-gtk/notify-list.c',
  '../../src/fe-gtk/user-list-model.c',
  '../../src/fe-gtk/user-list-view.c',
  include_directories: gtk4_includes,
  dependencies: [gtk4_library, pango_library, gdk_pixbuf_library, gio_library, gobject_library, glib_library],
)

probe_environment = environment()
probe_environment.prepend('PATH', join_paths(gtk4_root, 'bin'))
test('GTK4 dependency probe', probe, env: probe_environment)

"""Compile/run isolated native panel tests from an x64 MSVC developer shell.

Default run is parser-only; --renderer also constructs hidden GTK widgets with
presentation stubbed out. Neither mode opens an IRC connection or user profile.
Requires a generated config.h from a full-profile common build.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gtk4-root', type=Path, default=ROOT / 'Runtime/GTK4')
    parser.add_argument('--config-dir', type=Path, default=ROOT / 'build/gtk4-full/x64/lib')
    parser.add_argument('--support-root', type=Path, default=ROOT / 'build/vcpkg-installed/x64-windows')
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build/addon-panel-tests')
    parser.add_argument('--renderer', action='store_true')
    parser.add_argument('--schema', type=Path, help='Also render a supplied schema with page buttons (test harness only).')
    args = parser.parse_args()
    if args.schema and (not args.renderer or not args.schema.is_file()):
        parser.error('--schema requires --renderer and an existing schema XML file.')
    if not shutil.which('cl'):
        parser.error('Run from an x64 MSVC developer shell.')
    if not (args.config_dir / 'config.h').is_file():
        parser.error('Build the full-profile common target first, or supply --config-dir.')
    args.build_dir.mkdir(parents=True, exist_ok=True)
    executable = args.build_dir.resolve() / 'test-addon-panel.exe'
    includes = [args.config_dir.resolve(), args.support_root.resolve() / 'include'] + [args.gtk4_root.resolve() / path for path in (
        'include', 'include/gtk-4.0', 'include/glib-2.0', 'lib/glib-2.0/include', 'include/pango-1.0',
        'include/harfbuzz', 'include/cairo', 'include/gdk-pixbuf-2.0', 'include/graphene-1.0', 'lib/graphene-1.0/include')]
    command = ['cl', '/nologo', '/MD', '/std:c11', '/utf-8', '/W4', '/WX', '/DWIN32']
    command += ['/I' + str(path) for path in includes]
    command += [str(ROOT / 'tools/gtk4/test-addon-panel.c'), '/Fo' + str(executable.with_suffix('.obj')),
                '/Fe' + str(executable), '/link', '/LIBPATH:' + str(args.gtk4_root.resolve() / 'lib'),
                'gtk-4.lib', 'glib-2.0.lib', 'gobject-2.0.lib', 'gio-2.0.lib', 'intl.lib']
    subprocess.run(command, check=True, cwd=args.build_dir)
    env = dict(os.environ)
    env['PATH'] = str(args.gtk4_root.resolve() / 'bin') + os.pathsep + env.get('PATH', '')
    options = ['--renderer'] if args.renderer else []
    if args.schema:
        options += ['--schema', str(args.schema.resolve())]
    subprocess.run([str(executable)] + options, check=True, env=env)


if __name__ == '__main__':
    main()

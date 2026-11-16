"""Build/run hidden native OPER tests with mocked transport and credential storage.

Requires an x64 MSVC shell and generated full-profile config.h. No IRC, real
credentials, Credential Manager records or user profile are accessed by tests.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config-dir', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build/oper-login-tests')
    parser.add_argument('--gtk4-root', type=Path, default=ROOT / 'Runtime/GTK4')
    parser.add_argument('--support-root', type=Path, default=ROOT / 'build/vcpkg-installed/x64-windows')
    args = parser.parse_args()
    if not shutil.which('cl') or not (args.config_dir / 'config.h').is_file():
        parser.error('Use an x64 MSVC shell and generated full-profile config.h.')
    args.build_dir.mkdir(parents=True, exist_ok=True)
    includes = [args.config_dir.resolve(), args.support_root.resolve() / 'include'] + [args.gtk4_root.resolve() / p for p in (
        'include', 'include/gtk-4.0', 'include/glib-2.0', 'lib/glib-2.0/include',
        'include/pango-1.0', 'include/harfbuzz', 'include/cairo', 'include/gdk-pixbuf-2.0',
        'include/graphene-1.0', 'lib/graphene-1.0/include')]
    env = dict(os.environ)
    for key in ('AEGIS_OPENAI_API_KEY', 'OPENAI_API_KEY'):
        env.pop(key, None)
    env['PATH'] = str(args.support_root.resolve() / 'bin') + os.pathsep + str(args.gtk4_root.resolve() / 'bin') + os.pathsep + env.get('PATH', '')
    for name in ('test-oper-login', 'test-oper-secretstore'):
        exe = args.build_dir.resolve() / (name + '.exe')
        command = ['cl', '/nologo', '/MD', '/std:c11', '/utf-8', '/W4', '/WX', '/DWIN32']
        command += ['/I' + str(p) for p in includes]
        command += [str(ROOT / 'tools/gtk4' / (name + '.c')), '/Fo' + str(exe.with_suffix('.obj')), '/Fe' + str(exe),
                    '/link', '/LIBPATH:' + str(args.gtk4_root.resolve() / 'lib'),
                    '/LIBPATH:' + str(args.support_root.resolve() / 'lib'),
                    'advapi32.lib', 'libssl.lib', 'libcrypto.lib', 'gtk-4.lib', 'glib-2.0.lib',
                    'gobject-2.0.lib', 'gio-2.0.lib', 'intl.lib']
        subprocess.run(command, cwd=args.build_dir, env=env, check=True)
        subprocess.run([str(exe)], env=env, check=True)


if __name__ == '__main__':
    main()

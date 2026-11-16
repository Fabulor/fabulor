"""Compile exact production sender/watch functions in an isolated loopback fixture.
Application signals, accounting and close/UI services are inert test substitutes.
No IRC, profile, credentials, external peers or received-file writes.
Run from an MSVC x64 developer environment.
"""
from pathlib import Path
import argparse, os, subprocess
ROOT = Path(__file__).resolve().parents[2]

def function(source, name):
    import re
    matches = list(re.finditer(r'(?m)^' + name + r'\s*\([^;]*?\)\s*\{', source))
    if len(matches) != 1:
        raise RuntimeError(f'Expected one definition of {name}')
    start = matches[0].start()
    depth = 0
    # These selected functions have no braces inside strings/comments.
    for end in range(source.index('{', start), len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end+1]
    raise RuntimeError('Unbalanced function')

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--config-dir', type=Path, required=True)
    p.add_argument('--build-dir', type=Path, required=True)
    p.add_argument('--file', type=Path, help='Optional input; defaults to a synthetic 256 KiB payload.')
    p.add_argument('--gtk4-root', type=Path, default=ROOT/'Runtime/GTK4')
    p.add_argument('--support-root', type=Path, default=ROOT/'build/vcpkg-installed/x64-windows')
    a = p.parse_args()
    out = a.build_dir.resolve(); out.mkdir(parents=True, exist_ok=True)
    dcc = (ROOT/'src/common/dcc.c').read_text(encoding='utf-8')
    fe = (ROOT/'src/fe-gtk/fe-gtk.c').read_text(encoding='utf-8')
    names = ['dcc_send_ready', 'dcc_send_watch_update', 'dcc_send_data', 'dcc_handle_new_ack', 'dcc_read_ack']
    types = ['static gboolean', 'static void', 'static gboolean', 'static gboolean', 'static gboolean']
    text = 'static gboolean dcc_send_data(GIOChannel*,GIOCondition,struct DCC*);\nstatic gboolean dcc_read_ack(GIOChannel*,GIOCondition,struct DCC*);\nstatic void dcc_send_watch_update(struct DCC*);\n'
    text += 'void\n' + function(fe, 'fe_input_remove') + '\nint\n' + function(fe, 'fe_input_add') + '\n'
    text += '\n'.join(t+'\n'+function(dcc, n) for t,n in zip(types,names))
    (out/'dcc-send-under-test.inc').write_text(text, encoding='utf-8')
    gtk = a.gtk4_root.resolve()
    exe = out/'test-dcc-sender.exe'
    command = ['cl','/nologo','/MD','/utf-8','/O2','/DWIN32','/D_CRT_SECURE_NO_WARNINGS',
        '/I'+str(a.config_dir.resolve()), '/I'+str(out), '/I'+str(a.support_root.resolve()/'include'),
        '/I'+str(gtk/'include'), '/I'+str(gtk/'include/glib-2.0'), '/I'+str(gtk/'lib/glib-2.0/include'),
        '/Fo'+str(out)+'\\', '/Fe'+str(exe),str(ROOT/'tools/gtk4/test-dcc-sender.c'),
        '/link','/LIBPATH:'+str(gtk/'lib'),'glib-2.0.lib','ws2_32.lib']
    subprocess.run(command,check=True,cwd=ROOT)
    env=os.environ.copy(); env['PATH']=str(gtk/'bin')+';'+env['PATH']
    payload = a.file.resolve() if a.file else out/'dcc-sender-fixture.bin'
    if not a.file:
        payload.write_bytes(bytes(range(256)) * 1024)
    for mode in ['fast','slow','partial','resume','resume-offset','throttle','cancel','peer-close']:
        subprocess.run([str(exe),str(payload),mode],env=env,check=True,timeout=90)

if __name__ == '__main__': main()

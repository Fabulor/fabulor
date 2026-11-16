"""Compile production CTCP/system-string functions with offline service stubs.
Run from an MSVC x64 developer environment. No server or profile access.
"""
from pathlib import Path
import argparse, os, subprocess
from test_dcc_sender_native import function
ROOT = Path(__file__).resolve().parents[2]
CASES = ['peer','peer-no-echo','own-echo','own-case','channel-echo','own-no-echo',
         'self','action-echo','peer-action','dcc-echo','peer-dcc','hidden',
         'missing-cpu','empty-cpu','cache','no-cpu']

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--config-dir',type=Path,required=True)
    p.add_argument('--build-dir',type=Path,required=True)
    p.add_argument('--source-dir',type=Path,default=ROOT/'src/common')
    p.add_argument('--gtk4-root', type=Path, default=ROOT/'Runtime/GTK4')
    p.add_argument('--support-root', type=Path, default=ROOT/'build/vcpkg-installed/x64-windows')
    p.add_argument('--case',choices=CASES,action='append')
    a=p.parse_args();out=a.build_dir.resolve();out.mkdir(parents=True,exist_ok=True)
    text='char *\n'+function((a.source_dir/'util.c').read_text(encoding='utf-8'),'get_sys_str')
    ctcp=(a.source_dir/'ctcp.c').read_text(encoding='utf-8')
    if 'ctcp_is_own_echo (' in ctcp:
        text+='\ngboolean\n'+function(ctcp,'ctcp_is_own_echo')
    text+='\nvoid\n'+function(ctcp,'ctcp_handle')
    (out/'ctcp-under-test.inc').write_text(text, encoding='utf-8')
    gtk=a.gtk4_root.resolve();exe=out/'test-ctcp.exe'
    cmd=['cl','/nologo','/MD','/utf-8','/W3','/WX','/DWIN32','/D_CRT_SECURE_NO_WARNINGS',
         '/I'+str(ROOT/'src/common'),'/I'+str(a.config_dir.resolve()),'/I'+str(out),
         '/I'+str(a.support_root.resolve()/'include'),
         '/I'+str(gtk/'include'),'/I'+str(gtk/'include/glib-2.0'),
         '/I'+str(gtk/'lib/glib-2.0/include'),'/Fo'+str(out)+'\\','/Fe'+str(exe),
         str(ROOT/'tools/gtk4/test-ctcp.c'),'/link','/LIBPATH:'+str(gtk/'lib'),'glib-2.0.lib']
    subprocess.run(cmd,check=True,cwd=ROOT)
    env=os.environ.copy();env['PATH']=str(gtk/'bin')+';'+env['PATH']
    for case in a.case or CASES:
        subprocess.run([str(exe),case],env=env,check=True,timeout=15)

if __name__=='__main__':main()

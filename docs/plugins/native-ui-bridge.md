# Optional native add-on panel bridge

## Scope and status

This is an additive, local GTK4 panel facility for trusted Python add-ons. It
provides a sidebar, pages, bounded fields, labelled health lamps and explicit
button events. It is not an Aegis dependency in the Fabulor core and does not
modify the ordinary Preferences window. No Aegis, SDK, credentials, connection,
telemetry, settings migration or panel is loaded automatically.

The implementation has an isolated Windows x64 frontend/Python-host build and
mocked/parser/hidden-widget tests. Separate-profile maintainer acceptance passed
for the generic demo, ordinary Preferences and Aegis's panel, including draft/
effective state, Save across unload/load, cleanup and pinned tab targeting. Real
provider completion, local alerts and privileged exemptions were also observed
with explicit test-channel consent, then review was saved OFF. This does not
replace production installer/update, DPI/keyboard/accessibility or provider
failure/recovery acceptance. Do not overwrite a running installation with
individual development DLLs.

## Owners and boundaries

- `src/fe-gtk/addon-panel.c/.h` owns native widgets and the lazy panel registry.
  It uses the existing theme-aware `gtkutil_window_new` path.
- `ADDONUI` in `src/common/outbound.c` is the internal local transport.
- `get_info('addon_ui') == '1'` discovers native protocol support;
  `get_info('addon_ui:<token>')` acknowledges an existing panel.
- Additive `get_info('addon_ui_features')` advertises `page_buttons`. Public
  `ui_features()` returns only features supported by BOTH wrapper and frontend.
  Missing features must use the earlier schema/menu fallback; protocol 1 and
  existing global-button schemas remain unchanged. Discovery creates no UI.
- `get_info('id')` now exposes the stable connection ID as text. Older builds
  expose that identity via `get_prefs('id')` and channel-list rows instead.
- `plugins/python/_fabulor.py` validates schemas, owns handles/callbacks and
  exposes `ui_available`/`ui_open` through `fabulor` and compatibility aliases.
- `plugins/python/python.py` closes owned panels before releasing owner hooks.
  Discovery also checks this host ownership support, not just frontend support.
- No public native plugin vtable/header layout is changed. Existing native
  plugins continue to use the existing command/info entry points.

All widget operations and callbacks remain on the main thread. Python worker
threads are explicitly refused by the public UI methods. Full-API cooperative
capability checks apply; the isolated manifest interface is not expanded.
Add-ons remain trusted local code, not sandboxed or isolated from raw commands.
API handles reject accidental cross-owner operations; native transport tokens
are routing identifiers, not a new process security boundary.

Schemas are dictionaries converted to bounded base64 UTF-8 GMarkup. Native
validation is independent of Python validation. Only fixed widget types and
attribute sets are allowed, with no file loading, scripts, command templates,
external entities or arbitrary markup. Status lamps use fixed colours and
explicit text, not colour alone. UI callbacks dispatch registered hooks directly,
never the IRC/unknown-command fallback. No credential-entry field is provided;
do not put keys/passwords in schemas, diagnostics or text inputs.

Development hosts may additionally advertise `oper_login` (Windows with TLS).
The wrapper intersects this with its known features. An owned handle then exposes
`oper_login()` to open a separate host-owned GTK password window and
`oper_login_status()` to read non-secret status. Neither method accepts or returns
a password. They retain main-thread/owner/capability checks and are unavailable
through the isolated manifest API. Opening the dialog does not log in: the native
Log in button and peer/uplink confirmation are required. Normal panel Apply/Save,
values, events, schemas and updates never contain the native credential widgets.

The host uses separate `fabulor/irc-oper` Credential Manager records scoped by
configuration directory, network profile name, peer/port and oper name. Scope and
oper-name components are separately SHA-256 digested for target identifiers to
avoid Windows case-insensitive target-name aliases (not password hashing). It never
reuses/migrates connection passwords or silently falls back to profile files.
It requires a connected named profile with verified client TLS (invalid-certificate
acceptance is refused), validates operands and refuses a busy send queue. OPER
bypasses generic command hooks/history and the ordinary send queue/encoder;
oper names are ASCII and passwords are UTF-8 (maximum 400 bytes). The native send
passes only a redacted literal to Raw Log; Raw Log also masks other outgoing OPER
lines. A handshake-captured successful chain AND hostname verification marker is
required, not merely the current certificate-override preference. Incomplete or
failed TLS writes disable native login and schedule a high-priority connection
abort after callbacks unwind (no QUIT/replay). Delivery is uncertain; sent OPER
cannot be recalled. Ordinary host reconnect policy is separate; native OPER is
never replayed. The peer transport/error paths still require live acceptance.
Native transient secret copies are wiped on release, but this is not protection
against memory inspection or other trusted software under the Windows account.
Bouncer uplink security/remote logging cannot be established from client TLS.

Reply tracking accepts only relevant server-origin numerics for the current nick
and live connection generation: 381 confirms IRC operator login; 464/491 report
rejection, timeout is not denial. No services/channel rights are inferred. No
automatic login/retry; a shared connection cooldown prevents repeated native
attempts. Closing/unloading/disconnecting invalidates dialogs/request tracking.
Existing manual OPER commands can still be present in input history or old logs;
this feature does not read, migrate or delete pre-existing plaintext secrets.

When `page_buttons` is available, each page may have a `buttons` list with the
same fixed `{id, label}` objects as the global footer. Up to six per page, six
global, eight pages; IDs remain globally unique. Page actions emit the same typed
complete editable-value snapshot through owner-bound callbacks. They have no
embedded command templates. UPDATE refuses all buttons, including page buttons.

Limits are documented in [Python API Reference](python-api-reference.md#optional-native-control-panels).
An add-on owns persistence and consent. Controls are drafts until the add-on's
explicit Apply/Save callback handles them. Readonly updates cannot overwrite
editable values. Context is captured at open; add-ons must additionally reject
stale policy/reconnect state. A closing panel cannot leave callbacks attached to
an unloaded Python owner. Closing a parent window removes native ownership too.

Page fields use a shared two-column GTK form: labels have one common width across
pages, values are left-aligned beside them, and labelled checkboxes span the form.
Long plain text wraps. Each page has a scrolling field area and a separate,
compact action strip underneath, so scrolling never hides its buttons. The global
footer aligns to the trailing edge rather than filling the window. This is generic native layout,
not a schema/API extension or add-on-specific CSS. Hidden-widget tests verify
column placement, checkbox spans and action strips outside scrolling fields;
actual visual/DPI acceptance is separate.

The native registry is not allocated until a successful OPEN. The Python host
has empty ownership bookkeeping per add-on; no UI polling or background task is
added by the bridge. Optional add-on dashboards may start their own timers only
while open and must unhook those on close/unload.

## Build without installing

Use an x64 MSVC developer shell and the same validated GTK4/support roots as the
normal full profile. Keep candidate outputs isolated; for example:

```powershell
$properties = @('/p:Configuration=Release', '/p:Platform=x64',
  '/p:FabulorBuild=C:\fabulor-master\build\addon-ui-candidate',
  '/p:FabulorSupportDepsRoot=C:\fabulor-master\build\vcpkg-installed\x64-windows')
msbuild src\common\common.vcxproj @properties /m:1
msbuild win32\nls\nls.vcxproj @properties /m:1
msbuild src\fe-gtk\fe-gtk.vcxproj @properties /m:1
msbuild plugins\python\python3.vcxproj @properties /m:1
```

The frontend DLL, rebuilt Python host DLL and updated external `_fabulor.py`
must be staged together through normal runtime/installer packaging. Merely
copying an add-on cannot add this API to an older Fabulor. A feature-detecting
add-on must retain its existing commands/menus as a fallback. Review unrelated
working-tree changes before producing a release candidate; a successful local
build is not an isolated release/installer acceptance result.

## Validation

No real API keys, public IRC testing or channel transcripts are required:

```powershell
python plugins\python\test_native_ui.py
python plugins\python\test_capabilities.py
python plugins\python\test_manifest_isolation.py
python tools\gtk4\test_addon_panel_native.py --config-dir build\addon-ui-candidate\x64\lib
python tools\gtk4\test_addon_panel_native.py --config-dir build\addon-ui-candidate\x64\lib --renderer
```

The native runner needs an x64 developer shell, generated full-profile config
and the existing GTK4/support roots. Default mode is parser-only, without GTK
initialisation. `--renderer` constructs hidden widgets with presentation stubbed
out; it tests readonly updates, draft preservation and destruction without
showing a window, reading a user profile or connecting to IRC. Optional
`--renderer --schema path/to/panel.xml` also constructs a supplied panel with page
buttons through the production parser. File reading is a test-runner feature,
not part of the production bridge or schema language.

The page-button extension and text-width polish have separate development build,
mixed-version/owner/limits tests and hidden-renderer checks. Earlier installed
acceptance does not cover these additions; repeat controlled visual and coherent
package/update acceptance before deployment.

Before deployment, use a separate test profile and the explicit
`samples/plugins/simple-python-ui/simple-python-ui.py` example, without Aegis.
Load the example then run `/UIPANELDEMO`. Check theming, DPI, keyboard navigation,
close/parent-close, repeated load/unload and no panels after unload. Then test
Aegis's pinned panel in that controlled profile, without enabling external review
unless deliberately approved. Finally check normal startup and Preferences with
neither add-on loaded. Never use the ongoing live moderation trial as the first
host-integration smoke test.

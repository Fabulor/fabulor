# HexChat Documentation Reference Map

Status: initial mapping complete; individual Fabulor pages remain subject to
installed-client verification.

This document maps the official
[HexChat manual](https://hexchat.readthedocs.io/en/latest/) to Fabulor's user
documentation. It prevents obsolete behaviour from being carried forward merely
because Fabulor descends from the same codebase.

The HexChat documentation repository does not publish an explicit documentation
licence. Fabulor therefore uses it as a factual and structural reference while
writing original text. XChat documentation is a secondary historical reference.

## Decision Terms

- **Rewrite**: retain the concept but describe and test Fabulor's behaviour.
- **Replace**: use current Fabulor documentation instead of the inherited
  chapter.
- **Reference only**: useful background, but not part of the user manual.
- **Retire**: unsupported or deliberately removed from Fabulor.
- **Verify**: the concept may remain but needs installed-client confirmation
  before documentation is published.

## Client Chapters

| HexChat chapter | Decision | Fabulor destination | Notes |
| --- | --- | --- | --- |
| Client Documentation | Replace | `README.md` | Fabulor owns the contents and supported-platform statement. |
| Getting Started | Rewrite | `getting-started.md` | Use the current installer, Network List, first server tab, TLS defaults, and Windows 11 screenshots. |
| Settings: Config Files | Rewrite | `preferences.md` | Document `%APPDATA%\Fabulor`, installed mode, portable mode, and files users may safely maintain. |
| Settings: Network List | Rewrite | `networks-and-connections.md` | Cover current identity, server, SASL, TLS, ZNC, auto-connect, autojoin, and ordering behaviour. |
| Settings: Channel Options | Verify | `preferences.md` | Confirm the surviving per-channel options and their UI before publication. |
| Settings: Preferences | Rewrite | `preferences.md` | Inventory the current GTK4 pages; omit retired settings and legacy GTK paths. |
| Settings: Keyboard Shortcuts | Verify | `main-window.md` | Document only shortcuts present and tested in Fabulor. |
| Settings: URL Handlers | Verify | `preferences.md` | Prefer Windows default-browser behaviour; confirm whether configurable handlers remain user-facing. |
| Settings: Auto Replace | Verify | `preferences.md` | Confirm the editor and persistence path before retaining this topic. |
| Settings: CTCP Replies | Verify | `preferences.md` | Confirm the current menu surface and substitution behaviour. |
| Settings: `/SET` | Rewrite | `commands.md` | Explain querying, toggling, wildcards, persistence, and the distinction between supported and retired keys. |
| Settings: List of Settings | Replace | `commands.md` | Generate or verify the list from Fabulor rather than copying HexChat defaults. |
| Commands | Rewrite | `commands.md` | Verify command ordering, `//` escaping, `/HELP`, contexts, aliases, and every published built-in command. |
| Appearance: Theme Files | Rewrite | `themes-and-colours.md` | Fabulor imports `.hct` files through Preferences; manual extraction is not the supported workflow. |
| Appearance: Colours | Rewrite | `themes-and-colours.md` | Use Fabulor's Colours page and bundled Fabulor Dark theme. |
| Appearance: Text Events | Verify | `preferences.md` | Confirm the surviving event-format editor and codes before documenting it. |
| Appearance: Icons | Replace | `main-window.md` | Document packaged GTK4 and emoji/flag assets; do not inherit HexChat icon override paths. |
| Appearance: GTK Theme | Replace | `themes-and-colours.md` | Document GTK4 archive import, system appearance, variants, and approved theme-source policy. Retire GTK2/GTK3 instructions. |
| Appearance: User-list Popup | Verify | `main-window.md` | Describe the current GTK4 nick menu after installed testing. |
| Appearance: User-list Buttons | Verify | `main-window.md` | Confirm editing and command substitutions before publication. |
| Add-ons | Replace | `addons.md` | Use Fabulor's add-on roots, simple scripts, manifests, capability policy, safe mode, and Plugins and Scripts window. |
| Exec | Rewrite | `addons.md` | State that Exec is optional and document its bounded command behaviour and risk. |
| FiSHLiM | Rewrite | `addons.md` | Document only Fabulor's shipped commands and key-management surface. |
| Update Checker | Retire | `security-and-privacy.md` | The inherited updater is removed. Describe the signed-update activation gate without implying availability. |
| Sysinfo | Rewrite | `addons.md` | Verify the shipped command and menu presentation. |
| Checksum | Rewrite | `addons.md` | Verify current DCC checksum behaviour. |
| Winamp | Retire | `migration-from-xchat-and-hexchat.md` | The legacy media-player integration is removed. |
| Frequently Asked Questions | Rewrite | `troubleshooting.md` | Rebuild from Fabulor issues and installed acceptance results. |
| Tips: Spell Check | Rewrite | `main-window.md` | Use Enchant 2.8.19, WinSpell, Windows language support, suggestions, and personal dictionaries. |
| Tips: Localisation | Rewrite | `preferences.md` | Document packaged translations and current Windows language behaviour only. |
| Tips: Special Glyphs | Rewrite | `main-window.md` | Cover the packaged emoji fallback, regional indicators, and font behaviour. |
| Tips: Client Certificates | Verify | `networks-and-connections.md` | Confirm current certificate-file support after retirement of Network List certificate controls. |
| Tips: Custom Server Certificates | Replace | `security-and-privacy.md` | Document the supported CA bundle and invalid-certificate policy; do not recommend editing installed files. |
| Tips: Notice Placement | Verify | `main-window.md` | Test current server, channel, and query routing before documenting it. |

## Contributor And Developer Chapters

| HexChat chapter | Decision | Fabulor destination | Notes |
| --- | --- | --- | --- |
| Plugin Interface | Replace | `../plugins/` | Fabulor exposes its own native ABI and C#, Python, and Tcl contracts. |
| Developers | Replace | `../plugin-authoring-guides.md` | Do not inherit HexChat's recommended languages or versions. |
| Python Interface | Replace | `../plugins/python-plugin-guide.md` | Use the `fabulor` module and current embedded Python runtime. |
| Perl Interface | Retire | `migration-from-xchat-and-hexchat.md` | Perl is not a supported Fabulor plugin runtime. |
| Lua Interface | Retire | `migration-from-xchat-and-hexchat.md` | Lua is not a supported Fabulor plugin runtime. |
| JavaScript Interface | Retire | `migration-from-xchat-and-hexchat.md` | The external HexChat JavaScript interface is not a Fabulor contract. |
| D-Bus Interface | Retire | `migration-from-xchat-and-hexchat.md` | The Unix-oriented HexChat D-Bus interface is outside the Windows product. |
| Building Perl Modules | Retire | `migration-from-xchat-and-hexchat.md` | Perl and its build guidance are retired. |
| Building HexChat | Reference only | Developer documentation | Fabulor uses MSVC, GTK4, WiX, and its current repository workflows. |
| How to Help | Replace | Repository contribution guidance | Use Fabulor issues, pull requests, security policy, and documentation review. |
| HexChat ChangeLog | Reference only | `../../ChangeLog.md` | Fabulor maintains its own change log and credits its project lineage separately. |

## Fabulor-Only Coverage

The HexChat manual does not provide authoritative coverage for these Fabulor
features:

- Windows 11 x64 production scope;
- WiX bootstrapper installed and portable modes;
- GTK4 menus, transcript, list models, tray, emoji picker, and theme manager;
- `.hct` import and GTK4 desktop-theme archive import;
- country-name and country-code flag search;
- per-network and per-tab saved input history;
- scoped `/CLEAR HISTORY` and `/CLEAR LOG`;
- manifest add-on capabilities and safe mode;
- C# plugin hosting and the `fabulor` Python/Tcl APIs;
- bundled private Python, Tcl, and .NET runtime roots;
- ZNC startup, server-tab, and `/CYCLE` behaviour;
- bounded DALnet address fallback;
- signed-update design and updater retirement; and
- current installer repair, upgrade, and uninstall behaviour.

These topics must be written from Fabulor source, current technical
documentation, tests, and installed-client acceptance evidence.

## Verification Record

Each completed user page should record its verification basis in its pull
request:

1. Fabulor version tested;
2. clean-install, upgrade, or portable context;
3. relevant Windows version;
4. commands and menu paths exercised;
5. screenshots added or deliberately omitted; and
6. inherited behaviour explicitly rejected or retained.

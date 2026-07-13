# Enchant Windows Crash Analysis

Date: 2026-07-12

## Symptom

Fabulor 1.0.3 crashed reproducibly when a URL was pasted into the channel input box while spell checking was enabled. Windows reported `c0000005` in `ntdll.dll` at offset `0x5346d`. Disabling input-box spell checking stopped the crash.

## Dump Result

WinDbg/CDB analysis of `fabulor.exe.31140.dmp` classified the failure as heap corruption. The relevant stack was:

```text
ntdll!RtlFreeHeap
msvcrt!free
msvcrt!fclose
libenchant-2-2!enchant_pwl_check
libenchant-2-2!enchant_dict_check
fabulor.exe
```

The URL was a trigger, not the defect. Pasting changed the spell-entry text, which caused Enchant to check words and enter its personal-word-list path.

## Root Cause

The previous Enchant 2.8.19 and WinSpell payload was built with a MinGW-style runtime and imported `msvcrt.dll`. Fabulor's GTK/GLib payload is built with MSVC and imports `VCRUNTIME140.dll`/UCRT.

Enchant opened personal-word-list files through MSVC-built GLib's `g_fopen`, then closed the returned `FILE *` through `msvcrt!fclose`. A C runtime owns the internal representation and allocation of its `FILE` objects; passing one between these runtimes corrupted the heap.

## Remediation

Enchant 2.8.19 core and `enchant_winspell.dll` are now built with MSVC using the same GTK/GLib import libraries as Fabulor. The reproducible recipe is:

```powershell
.\tools\build-enchant-msvc.ps1
```

The script:

1. Downloads the official `enchant-2.8.19.tar.gz` archive if absent.
2. Requires SHA-256 `C8D70991D544EE39274B96BD01D2858A009FE732FF43F2AAF605FD61ECD06F60`.
3. Builds Enchant core and WinSpell as x64 MSVC DLLs against `C:\gtk-build\gtk`.
4. Uses a checked-in WinSpell-first provider ordering and temporary `bcp47.h` compatibility shim.
5. Stages the DLLs into `C:\zoitechat-build\x64\rel`.
6. Runs an isolated native smoke test covering checks, suggestions, personal-word addition, broker restart, and persistence.

Upstream replaced the `v2.8.19` release archive on 2026-07-13 while the Windows CI rollout was being validated. GitHub records the replacement asset digest above. Comparison with the previously audited `8E7F6CB0C3B79BE3146EB3AB93650484ADBC59DAE5F2C1958FDE557080BA678C` archive confirmed that every core source, public/provider header, and WinSpell source consumed by the MSVC recipe is byte-identical; the replacement adds or regenerates Autotools and gnulib distribution files that this recipe does not compile.

Verified staged hashes from this remediation run:

- `libenchant-2-2.dll`: `A2ABB91FFAD175665E73C0ED1E68C5F0782BF6313C5347E5E775A3B6AAAB8C8C`
- `lib\enchant-2\enchant_winspell.dll`: `B4BDA944ADC2FF0917F40706EB46219EA8382D9EE4C4CC2E17EDCC147AA9D3B0`

Import inspection confirms both rebuilt DLLs use `VCRUNTIME140.dll` and UCRT API sets and do not import `msvcrt.dll`.

## Validation And Cutover Status

Completed:

- Enchant MSVC build and import/export inspection.
- Native smoke test, including personal-word persistence.
- Native smoke test under full PageHeap, with no allocator violation; the temporary `enchant_smoke.exe` PageHeap setting was disabled and verified absent afterward.
- Fabulor Release x64 rebuild.
- WiX MSI and bootstrapper rebuild.
- Decompiled MSI tables confirm the rebuilt Enchant core, WinSpell provider, and ordering file are packaged; the retired DLL names are cleanup-only entries and the legacy provider/data directories are removed during installation.
- Final verification package hashes: `Fabulor.msi` `37015E1E1BA75CF2A101BBCB1BEA35CE8D62246971EC771035D7BBE1C98E87F0`; `FabulorSetup.exe` `1B95AA5875A14381F70CD171D553AE4A38C35A1633630FD2192DA28A85FA4FA1`.
- Installed-client testing confirmed repeated URL paste, typo detection, suggestion menus, add-to-personal, persistence after restart, and normal operation during the soak period.
- The Enchant 1.6.1 core and provider fallback has been removed. Windows now loads only the app-local `libenchant-2-2.dll`, and the installer packages only the MSVC/UCRT core, upstream WinSpell provider, and Enchant 2 ordering file.
- The legacy in-tree `libenchant_win8` provider is retired from the Windows solution; upstream WinSpell is the supported Windows provider.
- An in-place update with the final package retained full Enchant functionality and removed `libenchant.dll`, `libenchant-2.dll`, `lib\enchant`, and `share\enchant` from the installed tree. Installed core and WinSpell hashes matched the audited staged binaries.
- A clean uninstall/reinstall produced only `libenchant-2-2.dll`, `lib\enchant-2\enchant_winspell.dll`, and `share\enchant-2\enchant.ordering`; no legacy Enchant DLL/provider/data path remained, and live spell checking functioned correctly. The Enchant 2.8.19 rollout is complete.

Full PageHeap was run after the Windows SDK Debugging Tools feature supplied `gflags.exe`. The isolated replacement PWL smoke test passed under full PageHeap, and the temporary image setting was disabled immediately afterward. The original dump was analyzed with CDB.

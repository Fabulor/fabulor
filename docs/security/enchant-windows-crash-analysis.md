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
2. Requires SHA-256 `8E7F6CB0C3B79BE3146EB3AB93650484ADBC59DAE5F2C1958FDE557080BA678C`.
3. Builds Enchant core and WinSpell as x64 MSVC DLLs against `C:\gtk-build\gtk`.
4. Uses a checked-in WinSpell-first provider ordering and temporary `bcp47.h` compatibility shim.
5. Stages the DLLs into `C:\zoitechat-build\x64\rel`.
6. Runs an isolated native smoke test covering checks, suggestions, personal-word addition, broker restart, and persistence.

Verified staged hashes from this remediation run:

- `libenchant-2-2.dll`: `47F7F9049171FAC5BEF8849F3F320BC03BD2BD66943545E90B540FD68914BB79`
- `lib\enchant-2\enchant_winspell.dll`: `7ACADC4E90C74B5BCF4EA9A0929155A2DD7A9CCD31CF90768DB169871166A88D`

Import inspection confirms both rebuilt DLLs use `VCRUNTIME140.dll` and UCRT API sets and do not import `msvcrt.dll`.

## Validation Status

Completed:

- Enchant MSVC build and import/export inspection.
- Native smoke test, including personal-word persistence.
- Native smoke test under full PageHeap, with no allocator violation; the temporary `enchant_smoke.exe` PageHeap setting was disabled and verified absent afterward.
- Fabulor Release x64 rebuild.
- WiX MSI and bootstrapper rebuild.
- Installer bind tracking confirms the rebuilt Enchant core, WinSpell provider, and ordering file are packaged.

Outstanding interactive validation after installing the rebuilt package:

- Enable input-box spell checking and paste URLs repeatedly.
- Confirm typo detection and suggestion menus.
- Add a word to the dictionary and confirm it survives restart.
- Soak test before removing the Enchant 1.6.1 fallback.

Full PageHeap was run after the Windows SDK Debugging Tools feature supplied `gflags.exe`. The isolated replacement PWL smoke test passed under full PageHeap, and the temporary image setting was disabled immediately afterward. The original dump was analyzed with CDB.

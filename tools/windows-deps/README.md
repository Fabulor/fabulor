# Windows Support Dependencies

The supported GTK4 build takes GTK, GLib, XML, image, compression, and gettext
inputs from `Runtime/GTK4`. This vcpkg manifest supplies only the OpenSSL build
and runtime files that are not part of that root.

CI installs `openssl:x64-windows` from the registry baseline pinned in
`vcpkg-configuration.json` into `build/vcpkg-installed`. The production workflow
also downloads Mozilla's dated CA extract from
`https://curl.se/ca/cacert-2026-07-16.pem` and verifies:

- size: 186,446 bytes
- SHA-256: `3FF344E30B9B1ED2971044EABB438A08F2E2245DDB5F8AB1A3AD8B63AB4EAF91`

Pass the resulting `x64-windows` directory as `FabulorSupportDepsRoot` when
building locally. Do not add GTK, GLib, Lua, Perl, or libarchive to this
manifest; those are either owned by the pinned GTK4 root or outside the
supported Windows profile.

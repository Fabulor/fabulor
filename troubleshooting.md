# Troubleshooting

## Theme palette migration and legacy keys

New builds store theme colors as semantic token keys in `colors.conf` (for example `theme.mode.light.token.mirc_0`).
When loading old configs that only contain legacy keys (`color_*` and `dark_color_*`), ZoiteChat migrates them automatically if `theme.palette.semantic_migrated` is not present.

By default saves write both semantic token keys and legacy keys for compatibility. To phase out legacy writes, set:

```powershell
$env:ZOITECHAT_THEME_WRITE_LEGACY_KEYS=0
```

With that setting, saves write semantic token keys only while legacy-key loading remains available for older config files that have not yet been marked as migrated.

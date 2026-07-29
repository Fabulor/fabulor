# Troubleshooting

## Theme palette migration and legacy keys

New builds store theme colors as semantic token keys in `colors.conf` (for example `theme.mode.light.token.mirc_0`).
When loading old configs that only contain legacy keys (`color_*` and
`dark_color_*`), Fabulor migrates them automatically if
`theme.palette.semantic_migrated` is not present. Saves use semantic token keys
while legacy-key loading remains available for older configurations that have
not yet been migrated.

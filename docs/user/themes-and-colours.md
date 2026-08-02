# Themes And Colours

Fabulor has two independent kinds of theme:

- **Colour themes** control the IRC transcript palette, selection colours,
  marker line, activity states, and spell-check underline. They use `.hct`
  archives and `colors.conf`.
- **GTK4 desktop themes** control toolkit widgets such as menus, buttons,
  lists, fields, and windows. They use a theme directory containing
  `gtk-4.0\gtk.css`.

Changing one kind does not change the other. This separation lets you combine,
for example, the Fabulor Dark transcript palette with either the Windows
system appearance or a compatible GTK4 desktop theme.

Both controls are under **Settings > Preferences > Interface**. Use
**Colours** for the transcript palette and **Appearance** for the desktop
theme.

## Theme Storage

Installed Fabulor profiles use:

```text
%APPDATA%\Fabulor\themes
```

Portable mode uses the `themes` directory beneath the portable `Config`
directory.

The same directory can contain `.hct` colour-theme files and imported GTK4
theme directories. Their different formats keep the two systems distinct.

Do not install personal themes beneath `C:\Program Files\Fabulor`. Installer
repairs and upgrades may replace application files, while the profile is
intended to retain user choices.

## Colour Themes

![Fabulor's Colours preferences page showing the Fabulor Dark palette,
standard IRC colour slots, transcript colours, selection colours, and
interface-status colours.](images/colours-preferences.png)

### Fabulor Dark

Fabulor includes the original **Fabulor Dark** colour theme. It is installed as
a read-only bundled palette and appears in the **Palette theme** list without
being copied into your profile.

Fabulor Dark contains only colour settings. It does not replace text-event
formats or execute code.

### Installing An HCT Theme

The approved source for optional `.hct` files is the
[HexChat colour-theme collection](https://hexchat.github.io/themes.html).

1. Download the `.hct` file.
2. Place the intact file directly in `%APPDATA%\Fabulor\themes`.
3. Open or reopen **Settings > Preferences > Interface > Colours**.
4. Select the theme from **Palette theme**.
5. Review the preview and colour swatches.
6. Select **OK** to keep it or **Cancel** to restore the opening palette.

Do not extract the `.hct` file. Fabulor reads its bounded `colors.conf`
payload directly. Legacy `pevents.conf` content is ignored because inherited
event definitions are not assumed to match Fabulor's current event table.

Only immediate regular `.hct` files are discovered. A file hidden in a
subdirectory, or reached through a symbolic link or Windows reparse point, is
not accepted. Names are sorted consistently and matching is case-insensitive.
A profile theme takes precedence over a bundled palette with the same name.

### Current Colours

**Current colours** represents the palette that was active when Preferences
opened. Selecting it restores that opening palette within the current preview
transaction. It is not a separate archive on disk.

When you select **OK**, the chosen or edited palette is saved to:

```text
%APPDATA%\Fabulor\colors.conf
```

Fabulor writes this file atomically. **Cancel** restores the complete opening
palette rather than replaying individual colour changes.

### Editing Individual Colours

The Colours page provides direct swatches for:

- the standard mIRC colours;
- Fabulor's local colour slots;
- transcript foreground and background;
- selected-text foreground and background;
- new data, new messages, highlights, and away users;
- the marker line; and
- the spell-check indicator.

Select a swatch to use the colour picker. Changes are previewed while
Preferences remains open.

Select **Manage all client colours...** for the full colour manager. It
provides a searchable list, colour values, individual pickers, a combined
preview, and **Reset to defaults**. Closing the colour manager keeps its
changes staged in the main Preferences window; **OK** or **Cancel** in the
main window makes the final decision.

### Colour Stripping

The three colour-stripping choices remove incoming IRC formatting from
different surfaces:

- **Messages** strips formatting from newly received message content.
- **Scrollback** strips formatting while restoring previous transcript text.
- **Topic** strips formatting from the displayed channel topic.

These settings affect IRC colour and style codes, not your chosen Fabulor
palette or GTK4 desktop theme.

## GTK4 Desktop Themes

![Fabulor's Appearance preferences showing the System default desktop theme,
system-following variant, theme-archive import, and advanced appearance
controls.](images/desktop-theme-preferences.png)

### Approved Source And Compatibility

[OpenDesktop](https://www.opendesktop.org/) is Fabulor's sole approved source
for optional GTK4 desktop themes.

Use a theme that explicitly supplies GTK4 support. A GTK3-only theme is not
compatible merely because it has a similar name or directory layout. Fabulor
does not support the retired GTK2 or GTK3 theme paths, `.zct` palettes, or the
old mock Windows theme mechanism.

An approved download source does not make every archive compatible or safe.
Fabulor still validates every selected local archive before importing it.

### Importing A Desktop Theme

Fabulor accepts these archive formats:

- `.tar`
- `.tar.gz`
- `.tgz`
- `.tar.xz`
- `.txz`
- `.zip`

To import one:

1. Download the archive from OpenDesktop.
2. Open **Settings > Preferences > Interface > Appearance**.
3. Under **Fabulor Theme**, select **Import theme archive...**.
4. Select the downloaded archive and choose **Import**.
5. Wait for the inspection and import status to complete.
6. Select the imported theme from **Desktop theme**.
7. Choose a **Variant** and inspect the live result.
8. Select **OK** to save the choice or **Cancel** to restore the opening
   appearance.

Do not extract the archive manually. Fabulor handles archives directly so it
can enforce path, size, file-type, and containment rules. An archive may
contain several compatible theme roots; Fabulor reports how many were imported
and adds each one to the desktop-theme list.

The original downloaded archive is retained. Imported theme directories are
placed beneath the profile `themes` directory.

### What Fabulor Imports

A desktop-theme candidate must have an immediate theme root containing an
ordinary:

```text
gtk-4.0\gtk.css
```

An optional `gtk-4.0\gtk-dark.css`, `index.theme`, and preview images may also
be retained. Fabulor ignores unrelated GTK2, GTK3, GNOME Shell, Metacity, XFWM,
Plank, and other desktop components.

The importer rejects unsafe paths, duplicate entries, links, special files,
Windows reparse points, excessive nesting, excessive file counts, and
excessive compressed or expanded sizes. It does not overwrite an existing
theme directory, and a failed import does not leave a partial installation.

### Precompiled CSS Requirement

Fabulor loads complete GTK4 CSS; it is not a theme compiler. The required CSS
must be valid UTF-8 and ready for GTK4 to parse.

Archives containing unresolved Sass variables or unsupported CSS colour
expressions are rejected. A theme advertised as GTK4 can therefore still be
incompatible with Fabulor's packaged GTK runtime. If Fabulor reports a parser
error, the previous theme remains active. Use a different compatible theme
rather than modifying installed Fabulor files.

### Desktop Theme Variants

The **Variant** control has three choices:

- **Follow system** uses the current Windows application appearance.
- **Prefer light** requests the theme's light stylesheet.
- **Prefer dark** requests its dark stylesheet when one is available.

A complete `gtk-dark.css` replaces the theme's normal stylesheet for the dark
variant; the two are not layered together.

### System Default

**System default** removes Fabulor's custom GTK4 theme provider and follows the
Windows application appearance. Changes to the Windows light/dark setting are
detected while Fabulor is running.

In Windows high-contrast mode, Fabulor suppresses custom GTK4 theme providers
and dark-theme requests so the runtime default can preserve platform
accessibility. The selected `.hct` palette remains independent.

If a saved desktop theme has been removed or is no longer discoverable,
Fabulor falls back safely to **System default** instead of loading a partial or
unknown theme.

## Removing A Theme

To remove a profile desktop theme safely:

1. Select **System default** and choose **OK**.
2. Close Fabulor normally.
3. Back up the profile `themes` directory.
4. Remove only the unwanted imported theme directory.
5. Restart Fabulor and confirm that the remaining choices still load.

To remove a profile colour theme, close Fabulor and remove only its `.hct`
file from the profile `themes` directory. The bundled Fabulor Dark palette is
part of the installed application and should not be altered.

## Backup And Recovery

Before reorganising themes or manually editing colour values, close Fabulor
and back up:

```text
%APPDATA%\Fabulor\themes
%APPDATA%\Fabulor\colors.conf
%APPDATA%\Fabulor\fabulor.conf
```

The desktop-theme identifier and variant are stored in `fabulor.conf`; the
active transcript palette is stored in `colors.conf`.

If a custom transcript palette is unusable, close Fabulor, preserve a backup,
and remove `colors.conf`. Fabulor recreates its default colour state when it
next saves preferences. Removing `colors.conf` does not remove `.hct` archives
or GTK4 desktop themes.

## Troubleshooting

### An HCT Theme Does Not Appear

- Confirm that the filename ends in `.hct`.
- Put the file directly in the profile `themes` directory, not a child folder.
- Do not extract it.
- Reopen Preferences so discovery runs again.
- Confirm that it is a normal local file rather than a link or reparse point.

### An HCT Theme Is Rejected

The archive must contain one valid `colors.conf`. Malformed, duplicate, or
out-of-range supported colour values reject the complete candidate. The
current palette remains active.

### A Desktop Archive Cannot Be Selected

Confirm that it uses one of the supported archive extensions. Fabulor does not
accept theme installer programs or loose GTK CSS through the archive chooser.

### A Desktop Archive Is Rejected

The status text in Appearance gives the reason. Common causes include:

- no immediate GTK4 theme root;
- GTK3-only contents;
- symbolic links or special files;
- an existing destination with the same theme directory name;
- unresolved theme-source variables; or
- CSS that the packaged GTK4 parser cannot load.

The rejection is transactional: the current desktop theme stays installed and
the failed archive is not partially applied.

### The Widgets Changed But The Transcript Did Not

This is expected. Select a `.hct` palette under **Colours** to change the chat
palette. Select a desktop theme under **Appearance** to change GTK4 widgets.

### Cancel Restored The Previous Theme

This is expected preview behaviour. Reopen Preferences, select the desired
palette or desktop theme, and choose **OK** to persist it.

For a broader inventory of every Preferences page, see
[Preferences](preferences.md).

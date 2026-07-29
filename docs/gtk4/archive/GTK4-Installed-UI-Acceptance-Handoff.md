# GTK4 Installed UI Acceptance Handoff

Status: Archived on 2026-07-26.

The five installed-client defects tracked by this handoff passed IRL
acceptance and were published through PR #242. This file is retained as a
historical record; newly discovered transcript rendering and URL activation
issues are separate follow-up work.

Workspace: `C:\fabulor-master`

Branch: `codex/gtk4-final-acceptance-layout-crash`

Important: The worktree contains substantial uncommitted GTK4 acceptance work. Do not reset, discard, or overwrite it. Do not commit until Barry asks.

User-owned add-on changes must remain untouched:

- `addons/aliases/aliases.tcl` deleted
- `addons/ignore-queries/ignore-queries.py` modified
- `addons/dalnet-ignore-whitelist/` untracked
- `addons/set-away-back/` untracked

Latest screenshots:

- `C:\Users\Barry\OneDrive\Screenshots\Screenshot 2026-07-25 034514.png`
- `C:\Users\Barry\OneDrive\Screenshots\Screenshot 2026-07-25 034705.png`
- `C:\Users\Barry\OneDrive\Screenshots\Screenshot 2026-07-25 034803.png`

Current installed-client defects:

1. User-list width is configured as `gui_pane_right_size = 145`, but startup/layout does not preserve that width reliably.
2. Spell checking is enabled, but its right-click spelling menu is unavailable.
3. The scroll-to-bottom down-arrow overlay does not appear when the transcript is scrolled up.
4. The emoji picker remains too large and visually unchanged.
5. Menu composition still produces two `Window` branches. One contains only FiSHLiM and System Info; the other contains the canonical Window commands.
6. Windows dark-theme auto-detection is working correctly and should not be changed.

Root causes identified:

- User-list width: `notify::position` records transient GTK allocation values before initial right-pane restoration finishes.
- Duplicate Window menu: plugin menu roots are wrapped in anonymous `G_MENU_LINK_SECTION` entries. The merger only examined direct root items, so the plugin `Window` submenu remained separate.
- Spell menu: the model is refreshed by a parent click handler after GTK’s internal entry delegate has already prepared/opened its native context menu.
- Scroll overlay: it was observing the scrollbar widget’s adjustment rather than using the transcript’s canonical `GtkScrollable` adjustment/state. The themed icon may also not be visibly resolved.
- Emoji picker: existing maximum viewport was still `640 x 420`, which is oversized on the tested display.

Uncommitted corrective edits already applied:

- `src/fe-gtk/fe-gtk.h`
  - Added `pane_right_restoring` to `session_gui`.
- `src/fe-gtk/maingui.c`
  - Suppresses right-pane preference updates during initial restoration.
  - Captures the intended width in `gui->pane_right_size`.
  - Uses the transcript’s `GtkScrollable` adjustment for the down-arrow.
  - Uses a guaranteed visible Unicode down-arrow button.
  - Disables overlay clipping for the button.
- `src/fe-gtk/xtext.c` and `xtext.h`
  - Added `gtk_xtext_is_at_bottom()`.
- `src/fe-gtk/sexy-spell-entry.c`
  - Refreshes the spell menu on `notify::cursor-position`.
- `src/fe-gtk/middle-context-menu-model.c`
  - Flattens anonymous menu sections before matching canonical branches.
- `src/fe-gtk/emoji-picker.c`
  - Reduced maximum viewport from `640 x 420` to `520 x 320`.
- `tools/gtk4/probe.c`
  - Updated the menu test to wrap plugin branches in a real anonymous section.
  - Updated expected emoji viewport sizes.

Validation completed:

- Full GTK4 frontend MSBuild succeeded.
- Result: zero warnings and zero errors.
- Output:
  `C:\fabulor-master\build\gtk4-full\x64\rel\fabulor-gtk4-frontend.dll`

Validation not completed:

- The strict Meson/Ninja probe did not run because the ordinary terminal lacked `cl.exe`.
- Attempts to launch Ninja through `vcvars64.bat` remained held by the command wrapper and were interrupted.
- No installer was rebuilt after these latest edits.
- No installed-client retest has occurred.
- Documentation has not yet been updated for this latest follow-up.
- Nothing has been committed.

Recommended continuation:

1. Review the existing diff and verify the five latest fixes for correctness.
2. Run the strict probe using a reliable Visual Studio developer environment.
3. Run the complete GTK4 Python tooling suite.
4. Update `To-Do.md` and the GTK4 validation documentation.
5. Rebuild `Fabulor.msi` and `FabulorSetup.exe`.
6. Validate the MSI and bundle.
7. Have Barry upgrade and retest:
   - user-list width remains 145 after restart;
   - spelling suggestions and dictionary commands appear;
   - down-arrow appears when scrolled up and works;
   - emoji picker is compact;
   - exactly one complete Window menu exists.
8. Do not commit until Barry explicitly requests it.

/* Optional native add-on panels. No public plugin ABI layout change. */
#ifndef FABULOR_ADDON_PANEL_H
#define FABULOR_ADDON_PANEL_H
int fe_addon_panel_command (struct session *sess, const char *action,
                           const char *token, const char *payload);
int fe_addon_panel_exists (const char *token);
#endif

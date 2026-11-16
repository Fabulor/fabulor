/* Local Windows integration probe linked against the actual production spell
 * entry objects. Theme services are inert stubs; no IRC/Fabulor profile is loaded.
 * Types only the synthetic phrase "teh sleke btter turkey " into its verified test window.
 * The local runner must preflight the provider before exercising Ignore.
 */
#include <windows.h>
#include <gtk/gtk.h>
#include <gdk/win32/gdkwin32.h>
#include <string.h>
#include "../../src/common/fabulor.h"
#include "../../src/fe-gtk/sexy-spell-entry.h"
#include "../../src/fe-gtk/theme/theme-access.h"
#include "../../src/fe-gtk/theme/theme-manager.h"

struct fabulorprefs prefs;
gboolean theme_get_color (ThemeSemanticToken t, GdkRGBA *c) { (void)t; (void)c; return FALSE; }
gboolean theme_get_mirc_color (unsigned int t, GdkRGBA *c) { (void)t; (void)c; return FALSE; }
void theme_get_widget_style_values_for_widget (GtkWidget *w, ThemeWidgetStyleValues *v)
{ (void)w; memset (v, 0, sizeof *v); }
void theme_palette_color_get_rgb16 (const GdkRGBA *c, guint16 *r, guint16 *g, guint16 *b)
{ (void)c; *r = *g = *b = 0; }
gboolean theme_changed_event_has_reason (const ThemeChangedEvent *e, ThemeChangedReason r)
{ (void)e; (void)r; return FALSE; }
guint theme_listener_register (const char *id, ThemeChangedCallback cb, gpointer p)
{ (void)id; (void)cb; (void)p; return 1; }
void theme_listener_unregister (guint id) { (void)id; }

static gboolean host_check (SexySpellEntry *entry, const char *word, gpointer data)
{ (void)entry; (void)word; (void)data; return TRUE; }
static void drain (void)
{
	guint i;
	for (i = 0; i < 100 && g_main_context_pending (NULL); i++)
		g_main_context_iteration (NULL, FALSE);
}
static int owned_key (HWND hwnd, WORD key)
{
	INPUT input[2] = { 0 };
	guint j;
	if (GetForegroundWindow () != hwnd ||
		(GetAsyncKeyState (VK_CONTROL) & 0x8000) ||
		(GetAsyncKeyState (VK_MENU) & 0x8000) ||
		(GetAsyncKeyState (VK_SHIFT) & 0x8000) ||
		(GetAsyncKeyState (VK_LWIN) & 0x8000) ||
		(GetAsyncKeyState (VK_RWIN) & 0x8000) || (GetKeyState (VK_CAPITAL) & 1))
		return 77;
	input[0].type = input[1].type = INPUT_KEYBOARD;
	input[0].ki.wVk = input[1].ki.wVk = key;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	if (SendInput (2, input, sizeof (INPUT)) != 2) return 1;
	for (j = 0; j < 20; j++) { drain (); g_usleep (10000); }
	return 0;
}
int main (void)
{
	GtkWidget *window, *entry;
	const WORD keys[] = { 'T', 'E', 'H', VK_SPACE, 'S', 'L', 'E', 'K', 'E', VK_SPACE,
		'B', 'T', 'T', 'E', 'R', VK_SPACE, 'T', 'U', 'R', 'K', 'E', 'Y', VK_SPACE };
	guint i, j;
	gboolean misspelled = FALSE, corrected, undone, ignored = TRUE, fresh_misspelled = FALSE;
	GtkWidget *fresh;
	GSList *langs, *p;
	HWND hwnd;
	HRESULT hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
	if (FAILED (hr)) return 1;
	CoInitializeSecurity (NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	gtk_init ();
	strcpy (prefs.hex_text_spell_langs, "en_AU");
	strcpy (prefs.hex_input_command_char, "/");
	prefs.hex_gui_input_spell = prefs.hex_gui_input_autocorrect = 1;
	window = gtk_window_new ();
	gtk_window_set_title (GTK_WINDOW (window), "Fabulor spell-entry test (isolated; no IRC)");
	entry = sexy_spell_entry_new ();
	gtk_window_set_child (GTK_WINDOW (window), entry);
	g_signal_connect (entry, "word-check", G_CALLBACK (host_check), NULL);
	sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	sexy_spell_entry_set_autocorrect (SEXY_SPELL_ENTRY (entry), TRUE);
	langs = sexy_spell_entry_get_active_languages (SEXY_SPELL_ENTRY (entry));
	for (p = langs; p; p = p->next) g_print ("Active dictionary: %s\n", (char *)p->data);
	g_slist_free_full (langs, g_free);
	g_signal_emit_by_name (entry, "word-check", "teh", &misspelled);
	g_print ("Actual word-check rejects test typo: %s\n", misspelled ? "yes" : "no");
	gtk_window_present (GTK_WINDOW (window));
	gtk_widget_grab_focus (entry);
	for (j = 0; j < 30; j++) { drain (); g_usleep (10000); }
	hwnd = gdk_win32_surface_get_handle (gtk_native_get_surface (GTK_NATIVE (window)));
	SetForegroundWindow (hwnd);
	for (j = 0; j < 30; j++) { drain (); g_usleep (10000); }
	for (i = 0; i < G_N_ELEMENTS (keys); i++)
	{
		int status = owned_key (hwnd, keys[i]);
		if (status) { g_print ("Key probe aborted: %d\n", status); return status; }
	}
	corrected = !strcmp (gtk_editable_get_text (GTK_EDITABLE (entry)), "the sleek better T\303\274rkiye ");
	g_print ("Actual spell entry / real Windows keys: preferred suggestions including Unicode: %s\n", corrected ? "yes" : "no");
	if (owned_key (hwnd, VK_BACK)) return 77;
	undone = !strcmp (gtk_editable_get_text (GTK_EDITABLE (entry)), "the sleek better turkey");
	if (owned_key (hwnd, VK_SPACE)) return 77;
	undone = undone && !strcmp (gtk_editable_get_text (GTK_EDITABLE (entry)), "the sleek better turkey ");
	g_print ("Real Backspace restores Unicode correction; Space does not reapply: %s\n", undone ? "yes" : "no");
	/* The runner first rejects any provider with persistent session hooks.
	 * Exercise the actual Ignore action, then check a fresh entry's dictionary. */
	gtk_editable_set_text (GTK_EDITABLE (entry), "teh");
	if (!gtk_widget_activate_action (entry, "spell.ignore", NULL)) return 1;
	g_signal_emit_by_name (entry, "word-check", "teh", &ignored);
	fresh = sexy_spell_entry_new ();
	g_object_ref_sink (fresh);
	g_signal_emit_by_name (fresh, "word-check", "teh", &fresh_misspelled);
	g_print ("Ignore stays in this entry; fresh entry rejects typo: %s\n",
		!ignored && fresh_misspelled ? "yes" : "no");
	g_object_unref (fresh);
	gtk_window_destroy (GTK_WINDOW (window));
	drain ();
	CoUninitialize ();
	return corrected && undone && !ignored && fresh_misspelled ? 0 : 1;
}

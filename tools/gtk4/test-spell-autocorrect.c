#include <gtk/gtk.h>
#include <string.h>
#ifdef G_OS_WIN32
#include <windows.h>
#include <gdk/win32/gdkwin32.h>
#endif
#include "../../src/fe-gtk/spell-entry-autocorrect.h"

typedef struct { GtkWidget *window; GtkEntry *entry; GtkEventController *keys; } Fixture;
static gboolean allow_words = TRUE;
static gboolean mutate_during_approval = FALSE;
static gboolean mutate_during_suggestion = FALSE;
static const char *suggestion_override;
/* Fake provider data only: production has no typo table. */
static char *suggest (GtkEntry *entry, const char *word, gpointer data)
{
	const char *from[] = { "teh", "Teh", "adn", "recieve", "occured", "sleke", "btter", "turkey", "Turkey" };
	const char *to[] = { "the", "The", "and", "receive", "occurred", "sleek", "better", "T\303\274rkiye", "T\303\274rkiye" };
	guint i;
	(void) data;
	if (mutate_during_suggestion)
		gtk_editable_set_text (GTK_EDITABLE (entry), "changed externally");
	if (suggestion_override) return g_strdup (suggestion_override);
	for (i = 0; i < G_N_ELEMENTS (from); i++)
		if (!strcmp (word, from[i])) return g_strdup (to[i]);
	return NULL;
}
static gboolean approve (GtkEntry *entry, const char *old, const char *correct, gpointer data)
{
	(void) old; (void) correct; (void) data;
	if (mutate_during_approval)
		gtk_editable_set_text (GTK_EDITABLE (entry), "changed externally");
	return allow_words;
}
static void drain (void)
{
	guint i;
	for (i = 0; i < 100 && g_main_context_pending (NULL); i++)
		g_main_context_iteration (NULL, FALSE);
}
static Fixture fixture (gboolean enabled)
{
	Fixture f;
	GListModel *controllers;
	guint i;
	f.window = gtk_window_new ();
	f.entry = GTK_ENTRY (gtk_entry_new ());
	f.keys = NULL;
	gtk_window_set_child (GTK_WINDOW (f.window), GTK_WIDGET (f.entry));
	fabulor_spell_autocorrect_bind (f.entry, approve, suggest, NULL);
	fabulor_spell_autocorrect_set_enabled (f.entry, enabled);
	controllers = gtk_widget_observe_controllers (GTK_WIDGET (f.entry));
	for (i = 0; i < g_list_model_get_n_items (controllers); i++)
	{
		GtkEventController *c = g_list_model_get_item (controllers, i);
		if (GTK_IS_EVENT_CONTROLLER_KEY (c) &&
			gtk_event_controller_get_propagation_phase (c) == GTK_PHASE_CAPTURE)
			f.keys = c;
		else g_object_unref (c);
	}
	g_object_unref (controllers);
	g_assert_nonnull (f.keys);
	allow_words = TRUE;
	mutate_during_approval = FALSE;
	mutate_during_suggestion = FALSE;
	suggestion_override = NULL;
	return f;
}
static void finish (Fixture *f)
{
	g_object_unref (f->keys);
	gtk_window_destroy (GTK_WINDOW (f->window));
	drain ();
}
/* Emit the actual controller callback and then simulate the native editable
 * insertion. IME/platform key delivery still needs installed acceptance. */
static gboolean key (Fixture *f, guint keyval, GdkModifierType modifiers)
{
	gboolean handled = FALSE;
	g_signal_emit_by_name (f->keys, "key-pressed", keyval, 0, modifiers, &handled);
	return handled;
}
static void type (Fixture *f, const char *text, gboolean idle)
{
	const char *p;
	for (p = text; *p; p++)
	{
		gint cursor;
		g_assert_false (key (f, gdk_unicode_to_keyval ((guchar) *p), 0));
		cursor = gtk_editable_get_position (GTK_EDITABLE (f->entry));
		gtk_editable_insert_text (GTK_EDITABLE (f->entry), p, 1, &cursor);
		gtk_editable_set_position (GTK_EDITABLE (f->entry), cursor);
		if (idle) drain ();
	}
}
static void assert_text (Fixture *f, const char *expected)
{
	g_assert_cmpstr (gtk_editable_get_text (GTK_EDITABLE (f->entry)), ==, expected);
}
static void test_correction_undo (void)
{
	Fixture f = fixture (TRUE);
	type (&f, "teh ", TRUE);
	assert_text (&f, "the ");
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "teh");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 3);
	type (&f, " ", TRUE);
	assert_text (&f, "teh "); /* do not immediately correct the restored word again */
	finish (&f);
}
static void test_default_and_veto (void)
{
	Fixture f = fixture (FALSE);
	type (&f, "teh ", TRUE);
	assert_text (&f, "teh ");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	fabulor_spell_autocorrect_set_enabled (f.entry, TRUE);
	allow_words = FALSE; /* dictionary/known-nickname veto */
	type (&f, "adn ", TRUE);
	assert_text (&f, "adn ");
	finish (&f);
}
static void test_paste_and_unicode (void)
{
	Fixture f = fixture (TRUE);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "teh");
	gtk_editable_set_position (GTK_EDITABLE (f.entry), -1);
	type (&f, " ", TRUE);
	assert_text (&f, "teh ");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "te");
	gtk_editable_set_position (GTK_EDITABLE (f.entry), -1);
	type (&f, "h ", TRUE);
	assert_text (&f, "teh "); /* keyboard suffix must not rewrite a pasted prefix */
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "\303\251 \360\237\230\212 ");
	gtk_editable_set_position (GTK_EDITABLE (f.entry), -1);
	type (&f, "recieve ", TRUE);
	assert_text (&f, "\303\251 \360\237\230\212 receive ");
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "\303\251 \360\237\230\212 recieve");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 11);
	finish (&f);
}
static void test_ime (void)
{
	Fixture f = fixture (TRUE);
	GtkEditable *delegate = gtk_editable_get_delegate (GTK_EDITABLE (f.entry));
	type (&f, "teh", TRUE);
	g_signal_emit_by_name (delegate, "preedit-changed", "composition");
	type (&f, " ", TRUE);
	assert_text (&f, "teh ");
	g_signal_emit_by_name (delegate, "preedit-changed", "");
	type (&f, "adn ", TRUE);
	assert_text (&f, "teh and ");
	finish (&f);
}
static void test_pending_lifecycle (void)
{
	Fixture f = fixture (TRUE);
	type (&f, "teh ", FALSE);
	g_assert_false (key (&f, GDK_KEY_Return, 0));
	drain ();
	assert_text (&f, "teh ");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "adn ", FALSE);
	fabulor_spell_autocorrect_reset (f.entry); /* tab/context switch */
	drain ();
	assert_text (&f, "adn ");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "teh ", FALSE);
	finish (&f); /* destruction cancels pending idle */
}
static void test_cursor_selection_and_disable (void)
{
	Fixture f = fixture (TRUE);
	type (&f, "teh ", TRUE);
	gtk_editable_set_position (GTK_EDITABLE (f.entry), 0);
	gtk_editable_set_position (GTK_EDITABLE (f.entry), -1);
	g_assert_false (key (&f, GDK_KEY_BackSpace, 0));
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "teh ", FALSE);
	fabulor_spell_autocorrect_set_enabled (f.entry, FALSE);
	drain ();
	assert_text (&f, "teh ");
	finish (&f);
}
static void test_fast_typing (void)
{
	Fixture f = fixture (TRUE);
	type (&f, "Teh adn ", FALSE);
	drain ();
	assert_text (&f, "The and ");
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "The adn");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "teh dayz ", FALSE);
	drain ();
	assert_text (&f, "the dayz ");
	finish (&f);
}
static void test_detach_and_retained_controller (void)
{
	Fixture f = fixture (TRUE);
	g_object_ref (f.entry);
	type (&f, "teh ", FALSE);
	gtk_window_set_child (GTK_WINDOW (f.window), NULL);
	drain ();
	assert_text (&f, "teh ");
	g_object_unref (f.entry);
	/* Externally retained controllers must not call a freed owner. */
	g_assert_false (key (&f, GDK_KEY_space, 0));
	finish (&f);
}
static void test_middle_and_reentrancy (void)
{
	Fixture f = fixture (TRUE);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "prefix  tail");
	gtk_editable_set_position (GTK_EDITABLE (f.entry), 7);
	type (&f, "teh ", TRUE);
	assert_text (&f, "prefix the  tail");
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "prefix teh tail");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 10);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	mutate_during_approval = TRUE;
	type (&f, "teh ", TRUE);
	assert_text (&f, "changed externally");
	finish (&f);
}
static void test_edit_constraints (void)
{
	Fixture f = fixture (TRUE);
	gtk_entry_set_max_length (f.entry, 8);
	type (&f, "occured ", TRUE);
	assert_text (&f, "occured "); /* expanding replacement must never truncate input */
	gtk_entry_set_max_length (f.entry, 0);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "teh ", FALSE);
	gtk_editable_set_editable (GTK_EDITABLE (f.entry), FALSE);
	drain ();
	assert_text (&f, "teh ");
	gtk_editable_set_editable (GTK_EDITABLE (f.entry), TRUE);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "teh", TRUE);
	gtk_editable_select_region (GTK_EDITABLE (f.entry), 0, -1);
	g_assert_false (key (&f, GDK_KEY_space, 0));
	drain ();
	assert_text (&f, "teh");
	gtk_editable_select_region (GTK_EDITABLE (f.entry), 0, 0);
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	gtk_entry_set_visibility (f.entry, FALSE);
	type (&f, "teh ", TRUE);
	assert_text (&f, "teh ");
	finish (&f);
}
static void test_policy (void)
{
	const char *protected[] = { "/say teh", "#teh", "@teh", "teh@example.com",
		"https://teh", "www.teh", "teh.txt", "C:\\teh", "TEH", "tEh", "teh,",
		"`teh`", "teh2", "\002teh" };
	guint i;
	gint start;
	char *candidate;
	for (i = 0; i < G_N_ELEMENTS (protected); i++)
		g_assert_null (fabulor_spell_autocorrect_candidate (protected[i],
			(gint) g_utf8_strlen (protected[i], -1), 0, &start));
	candidate = fabulor_spell_autocorrect_candidate ("Teh", 3, 0, &start);
	g_assert_cmpstr (candidate, ==, "Teh"); g_free (candidate);
	g_assert_null (fabulor_spell_autocorrect_candidate ("teh", 3, 1, &start));
	g_assert_null (fabulor_spell_autocorrect_candidate ("teh", 2, 0, &start));
	candidate = fabulor_spell_autocorrect_candidate ("i", 1, 0, &start);
	g_assert_cmpstr (candidate, ==, "i"); g_free (candidate);
	candidate = fabulor_spell_autocorrect_candidate ("fo", 2, 0, &start);
	g_assert_cmpstr (candidate, ==, "fo"); g_free (candidate);
}
static void test_preferred_and_unicode (void)
{
	Fixture f = fixture (TRUE);
	type (&f, "sleke btter turkey ", TRUE);
	assert_text (&f, "sleek better T\303\274rkiye ");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 21);
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "sleek better turkey");
	type (&f, " ", TRUE);
	assert_text (&f, "sleek better turkey ");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "Turkey ", FALSE);
	drain ();
	assert_text (&f, "T\303\274rkiye ");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 8);
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "Turkey");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "\303\251 \360\237\230\212  tail");
	gtk_editable_set_position (GTK_EDITABLE (f.entry), 4);
	type (&f, "turkey ", TRUE);
	assert_text (&f, "\303\251 \360\237\230\212 T\303\274rkiye  tail");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 12);
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "\303\251 \360\237\230\212 turkey tail");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (f.entry)), ==, 10);
	finish (&f);
}
static void test_suggestion_guards (void)
{
	const char *bad[] = { "", "\377", "bad\ntext", "/say", "@nick", "a  lot", " space", "space ", "https://x" };
	Fixture f = fixture (TRUE);
	guint i;
	for (i = 0; i < G_N_ELEMENTS (bad); i++)
	{
		g_assert_false (fabulor_spell_autocorrect_valid_replacement (bad[i]));
		suggestion_override = bad[i];
		gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
		type (&f, "teh ", TRUE);
		assert_text (&f, "teh ");
	}
	suggestion_override = "a lot";
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	type (&f, "alot ", TRUE);
	assert_text (&f, "a lot ");
	g_assert_true (key (&f, GDK_KEY_BackSpace, 0));
	assert_text (&f, "alot");
	gtk_editable_set_text (GTK_EDITABLE (f.entry), "");
	suggestion_override = "T\303\274rkiye";
	gtk_entry_set_max_length (f.entry, 7);
	type (&f, "turkey ", TRUE);
	assert_text (&f, "turkey "); /* Unicode expansion cannot truncate. */
	finish (&f);
}
static void test_suggestion_reentrancy (void)
{
	Fixture f = fixture (TRUE);
	mutate_during_suggestion = TRUE;
	type (&f, "teh ", TRUE);
	assert_text (&f, "changed externally");
	finish (&f);
}
#ifdef G_OS_WIN32
static void probe_preedit (GtkWidget *widget, const char *text, gpointer data)
{
	(void) widget; (void) data;
	g_print ("Probe preedit: %s\n", text && *text ? "non-empty" : "empty");
}
static int windows_keys (void)
{
	Fixture f = fixture (TRUE);
	const WORD keys[] = { 'T', 'E', 'H', VK_SPACE };
	guint i, j;
	HWND hwnd;
	g_signal_connect (gtk_editable_get_delegate (GTK_EDITABLE (f.entry)),
		"preedit-changed", G_CALLBACK (probe_preedit), NULL);
	gtk_window_present (GTK_WINDOW (f.window));
	gtk_widget_grab_focus (GTK_WIDGET (f.entry));
	for (j = 0; j < 30; j++) { drain (); g_usleep (10000); }
	hwnd = gdk_win32_surface_get_handle (gtk_native_get_surface (GTK_NATIVE (f.window)));
	for (i = 0; i < G_N_ELEMENTS (keys); i++)
	{
		INPUT input[2] = { 0 };
		/* Explicit, local-only mode: never inject into another foreground window. */
		if (GetForegroundWindow () != hwnd ||
			(GetAsyncKeyState (VK_CONTROL) & 0x8000) ||
			(GetAsyncKeyState (VK_MENU) & 0x8000) ||
			(GetAsyncKeyState (VK_SHIFT) & 0x8000) ||
			(GetAsyncKeyState (VK_LWIN) & 0x8000) ||
			(GetAsyncKeyState (VK_RWIN) & 0x8000) || (GetKeyState (VK_CAPITAL) & 1))
		{
			g_print ("Probe aborted: own window is not exclusively focused\n");
			finish (&f); return 77;
		}
		input[0].type = input[1].type = INPUT_KEYBOARD;
		input[0].ki.wVk = input[1].ki.wVk = keys[i];
		input[1].ki.dwFlags = KEYEVENTF_KEYUP;
		if (SendInput (2, input, sizeof (INPUT)) != 2) { finish (&f); return 1; }
		for (j = 0; j < 20; j++) { drain (); g_usleep (10000); }
		g_print ("Probe step %u: text length=%lld\n", i + 1,
			(long long) g_utf8_strlen (gtk_editable_get_text (GTK_EDITABLE (f.entry)), -1));
	}
	i = strcmp (gtk_editable_get_text (GTK_EDITABLE (f.entry)), "the ") == 0;
	g_print ("Real Windows keys corrected: %s\n", i ? "yes" : "no");
	finish (&f);
	return i ? 0 : 1;
}
#endif
int main (int argc, char **argv)
{
	gtk_init ();
#ifdef G_OS_WIN32
	if (argc == 2 && !strcmp (argv[1], "--windows-keys")) return windows_keys ();
#endif
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/autocorrect/policy/protected-tokens", test_policy);
	g_test_add_func ("/autocorrect/widget/preferred-and-unicode", test_preferred_and_unicode);
	g_test_add_func ("/autocorrect/widget/suggestion-guards", test_suggestion_guards);
	g_test_add_func ("/autocorrect/widget/suggestion-reentrancy", test_suggestion_reentrancy);
	g_test_add_func ("/autocorrect/widget/correction-and-undo", test_correction_undo);
	g_test_add_func ("/autocorrect/widget/default-and-veto", test_default_and_veto);
	g_test_add_func ("/autocorrect/widget/paste-and-unicode", test_paste_and_unicode);
	g_test_add_func ("/autocorrect/widget/ime", test_ime);
	g_test_add_func ("/autocorrect/widget/pending-lifecycle", test_pending_lifecycle);
	g_test_add_func ("/autocorrect/widget/cursor-and-disable", test_cursor_selection_and_disable);
	g_test_add_func ("/autocorrect/widget/edit-constraints", test_edit_constraints);
	g_test_add_func ("/autocorrect/widget/fast-typing", test_fast_typing);
	g_test_add_func ("/autocorrect/widget/middle-and-reentrancy", test_middle_and_reentrancy);
	g_test_add_func ("/autocorrect/widget/detach-and-controller", test_detach_and_retained_controller);
	return g_test_run ();
}

#include "spell-entry-autocorrect.h"
#include <string.h>

#define OWNER_KEY "fabulor-spell-autocorrect"

typedef struct
{
	GtkEntry *entry; /* borrowed: owner is attached to entry */
	FabulorAutoCorrectApprove approve;
	FabulorAutoCorrectSuggest suggest;
	gpointer data;
	gboolean enabled, busy, composing, space, ime_guard;
	gint start, end, arm_start, arm_cursor, pending_cursor, undo_cursor, undo_original_cursor, undo_start;
	char *typed, *expected, *before, *replacement;
	char *pending_text, *pending_before, *pending_after;
	char *undo_before, *undo_after;
	guint idle;
} AutoCorrect;

char *
fabulor_spell_autocorrect_candidate (const char *text, gint cursor,
	gint typed_start, gint *word_start)
{
	const char *end, *start, *p;
	char word[65];
	gsize len;
	guint i;
	if (!text || !g_utf8_validate (text, -1, NULL) || strlen (text) > 32768 ||
		cursor < 0 || cursor > g_utf8_strlen (text, -1))
		return NULL;
	for (p = text; g_ascii_isspace (*p); p++) {}
	if (*p == '/') return NULL;
	for (p = text; *p; p++)
		if ((guchar) *p < 32 || *p == 127) return NULL;
	end = g_utf8_offset_to_pointer (text, cursor);
	/* A Space inside an existing word must not split/correct that word. */
	if (*end && !g_unichar_isspace (g_utf8_get_char (end))) return NULL;
	start = end;
	while (start > text)
	{
		p = g_utf8_prev_char (start);
		if (g_unichar_isspace (g_utf8_get_char (p))) break;
		start = p;
	}
	len = (gsize) (end - start);
	if (!len || len >= sizeof word ||
		g_utf8_pointer_to_offset (text, start) != typed_start)
		return NULL;
	memcpy (word, start, len);
	word[len] = 0;
	for (i = 0; i < len; i++)
	{
		if (!g_ascii_isalpha (word[i]) || (i && !g_ascii_islower (word[i])))
			return NULL; /* no acronyms, mixed case, links, mentions or code tokens */
	}
	*word_start = typed_start;
	return g_strdup (word);
}

/* Preserve Windows' case/diacritics and ordinary multiword suggestions, but
 * never insert controls, IRC formatting, commands, URLs or arbitrary text. */
gboolean fabulor_spell_autocorrect_valid_replacement (const char *word)
{
	const char *p;
	gboolean letter = FALSE;
	if (!word || !*word || strlen (word) > 256 ||
		!g_utf8_validate (word, -1, NULL) || g_utf8_strlen (word, -1) > 64)
		return FALSE;
	for (p = word; *p; p = g_utf8_next_char (p))
	{
		gunichar ch = g_utf8_get_char (p);
		if (g_unichar_isalpha (ch)) letter = TRUE;
		else if (letter && g_unichar_ismark (ch)) continue;
		else if (letter && (ch == ' ' || ch == '-' || ch == '\'' || ch == 0x2019))
			letter = FALSE;
		else return FALSE;
	}
	return letter;
}

static void clear_undo (AutoCorrect *ac)
{
	g_clear_pointer (&ac->undo_before, g_free);
	g_clear_pointer (&ac->undo_after, g_free);
}

static void reset (AutoCorrect *ac)
{
	if (ac->idle) { g_source_remove (ac->idle); ac->idle = 0; }
	g_clear_pointer (&ac->typed, g_free);
	g_clear_pointer (&ac->expected, g_free);
	g_clear_pointer (&ac->before, g_free);
	g_clear_pointer (&ac->replacement, g_free);
	g_clear_pointer (&ac->pending_text, g_free);
	g_clear_pointer (&ac->pending_before, g_free);
	g_clear_pointer (&ac->pending_after, g_free);
	clear_undo (ac);
}

void fabulor_spell_autocorrect_reset (GtkEntry *entry)
{
	AutoCorrect *ac = g_object_get_data (G_OBJECT (entry), OWNER_KEY);
	if (ac) reset (ac);
}

void fabulor_spell_autocorrect_set_enabled (GtkEntry *entry, gboolean enabled)
{
	AutoCorrect *ac = g_object_get_data (G_OBJECT (entry), OWNER_KEY);
	if (ac) { reset (ac); ac->enabled = enabled && ac->ime_guard; }
}

static gboolean selection (AutoCorrect *ac)
{
	gint a, b;
	return gtk_editable_get_selection_bounds (GTK_EDITABLE (ac->entry), &a, &b);
}

static gboolean apply_pending (gpointer data)
{
	AutoCorrect *ac = data;
	GtkEditable *edit = GTK_EDITABLE (g_object_ref (ac->entry));
	ac->idle = 0;
	if (ac->enabled && !ac->composing && gtk_widget_get_root (GTK_WIDGET (ac->entry)) &&
		gtk_editable_get_editable (edit) && gtk_entry_get_visibility (ac->entry) &&
		!selection (ac) && ac->pending_text &&
		!strcmp (gtk_editable_get_text (edit), ac->pending_text) &&
		gtk_editable_get_position (edit) == ac->arm_cursor + 1)
	{
		gint original_start = ac->arm_start;
		char *original = gtk_editable_get_chars (edit, original_start, ac->arm_cursor);
		gint original_end = ac->arm_cursor;
		char *snapshot = g_strdup (ac->pending_text);
		char *candidate = ac->suggest (ac->entry, original, ac->data);
		gboolean approved = FALSE;
		if (fabulor_spell_autocorrect_valid_replacement (candidate) &&
			strcmp (original, candidate) && ac->enabled && !ac->composing &&
			ac->pending_text && ac->pending_before &&
			ac->arm_start == original_start && ac->arm_cursor == original_end &&
			!strcmp (ac->pending_text, snapshot) &&
			!strcmp (gtk_editable_get_text (edit), snapshot))
		{
			const char *begin = g_utf8_offset_to_pointer (ac->pending_before, original_start);
			const char *end = g_utf8_offset_to_pointer (ac->pending_before, original_end);
			char *left = g_strndup (ac->pending_before, begin - ac->pending_before);
			ac->pending_after = g_strconcat (left, candidate, " ", end, NULL);
			g_free (left);
			ac->pending_cursor = original_start + (gint) g_utf8_strlen (candidate, -1) + 1;
			approved = ac->approve (ac->entry, original, candidate, ac->data);
		}
		g_free (snapshot);
		if (approved && ac->enabled && !ac->composing && ac->pending_text &&
			ac->pending_after && !strcmp (gtk_editable_get_text (edit), ac->pending_text) &&
			gtk_editable_get_position (edit) == ac->arm_cursor + 1 && !selection (ac) &&
			gtk_editable_get_editable (edit) && gtk_entry_get_visibility (ac->entry) &&
			gtk_widget_get_root (GTK_WIDGET (ac->entry)) &&
			(!gtk_entry_get_max_length (ac->entry) ||
			 g_utf8_strlen (ac->pending_after, -1) <= gtk_entry_get_max_length (ac->entry)))
		{
			gint position = ac->arm_start;
			ac->busy = TRUE;
			gtk_editable_delete_text (edit, ac->arm_start, ac->arm_cursor);
			gtk_editable_insert_text (edit, candidate, -1, &position);
			gtk_editable_set_position (edit, ac->pending_cursor);
			ac->busy = FALSE;
			ac->undo_before = g_steal_pointer (&ac->pending_before);
			ac->undo_after = g_strdup (ac->pending_after);
			ac->undo_cursor = ac->pending_cursor;
			ac->undo_original_cursor = ac->arm_cursor;
			ac->undo_start = ac->arm_start;
		}
		g_free (candidate);
		g_free (original);
	}
	g_clear_pointer (&ac->pending_text, g_free);
	g_clear_pointer (&ac->pending_before, g_free);
	g_clear_pointer (&ac->pending_after, g_free);
	g_clear_pointer (&ac->replacement, g_free);
	g_object_unref (edit);
	return G_SOURCE_REMOVE;
}

static char *insert_at (const char *text, gint cursor, const char *insert)
{
	const char *p = g_utf8_offset_to_pointer (text, cursor);
	char *left = g_strndup (text, p - text);
	char *result = g_strconcat (left, insert, p, NULL);
	g_free (left);
	return result;
}

static void changed (GtkEditable *edit, gpointer data)
{
	AutoCorrect *ac = data;
	const char *text = gtk_editable_get_text (edit);
	if (ac->busy) return;
	if (ac->expected && !strcmp (text, ac->expected))
	{
		if (!ac->space)
		{
			g_free (ac->typed);
			ac->typed = g_strdup (text);
			ac->start = ac->arm_start;
			ac->end = ac->arm_cursor + 1;
		}
		else
		{
			ac->pending_before = g_steal_pointer (&ac->before);
			ac->pending_text = g_strdup (text);
			g_clear_pointer (&ac->typed, g_free);
			/* Wait until the native insertion has updated the cursor. */
			ac->idle = g_idle_add (apply_pending, ac);
		}
		g_clear_pointer (&ac->expected, g_free);
		g_clear_pointer (&ac->before, g_free);
	}
	else reset (ac); /* paste, history, emoji, programmatic edit or unexpected IME input */
}

static gboolean key_pressed (GtkEventControllerKey *controller, guint keyval,
	guint keycode, GdkModifierType state, gpointer data)
{
	AutoCorrect *ac = g_object_get_data (G_OBJECT (data), OWNER_KEY);
	GtkEditable *edit = GTK_EDITABLE (data);
	const char *text;
	gint cursor;
	gboolean plain = !(state & (GDK_CONTROL_MASK | GDK_ALT_MASK | GDK_SUPER_MASK |
		GDK_META_MASK | GDK_HYPER_MASK));
	gunichar ch = gdk_keyval_to_unicode (keyval);
	(void) controller; (void) keycode;
	if (!ac) return FALSE;
	if (ac->idle)
	{
		g_source_remove (ac->idle); ac->idle = 0;
		/* Never do a deferred rewrite on the send key. */
		if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) reset (ac);
		else apply_pending (ac);
	}
	text = gtk_editable_get_text (edit);
	cursor = gtk_editable_get_position (edit);
	if (ac->enabled && !ac->composing && plain && keyval == GDK_KEY_BackSpace &&
		gtk_editable_get_editable (edit) && gtk_entry_get_visibility (ac->entry) &&
		!(state & GDK_SHIFT_MASK) && !selection (ac) && ac->undo_after &&
		!strcmp (text, ac->undo_after) && cursor == ac->undo_cursor &&
		(!gtk_entry_get_max_length (ac->entry) ||
		 g_utf8_strlen (ac->undo_before, -1) <= gtk_entry_get_max_length (ac->entry)))
	{
		gint position = ac->undo_start;
		char *original = g_utf8_substring (ac->undo_before, ac->undo_start,
			ac->undo_original_cursor);
		ac->busy = TRUE;
		gtk_editable_delete_text (edit, ac->undo_start, ac->undo_cursor);
		gtk_editable_insert_text (edit, original, -1, &position);
		gtk_editable_set_position (edit, ac->undo_original_cursor);
		ac->busy = FALSE;
		g_free (original);
		reset (ac);
		return TRUE;
	}
	clear_undo (ac);
	if (!ac->enabled || ac->composing || !plain || !gtk_editable_get_editable (edit) ||
		!gtk_entry_get_visibility (ac->entry) ||
		selection (ac) || strlen (text) > 32768)
	{
		reset (ac); return FALSE;
	}
	g_clear_pointer (&ac->expected, g_free);
	g_clear_pointer (&ac->before, g_free);
	if (ch == ' ' && !(state & GDK_SHIFT_MASK) && ac->typed &&
		!strcmp (text, ac->typed) && cursor == ac->end)
	{
		ac->replacement = fabulor_spell_autocorrect_candidate (text, cursor, ac->start,
			&ac->arm_start);
		if (ac->replacement)
		{
			ac->space = TRUE;
			ac->arm_cursor = cursor;
			ac->before = g_strdup (text);
			ac->expected = insert_at (text, cursor, " ");
			return FALSE;
		}
	}
	else if (ch < 128 && g_ascii_isalpha ((char) ch))
	{
		char letter[2] = { (char) ch, 0 };
		ac->arm_start = ac->typed && !strcmp (text, ac->typed) && cursor == ac->end ?
			ac->start : cursor;
		ac->arm_cursor = cursor;
		ac->space = FALSE;
		ac->expected = insert_at (text, cursor, letter);
		return FALSE;
	}
	reset (ac);
	return FALSE;
}

static void preedit (GtkWidget *delegate, const char *text, gpointer data)
{
	AutoCorrect *ac = g_object_get_data (G_OBJECT (data), OWNER_KEY);
	(void) delegate;
	if (!ac) return;
	reset (ac);
	ac->composing = text && *text;
}

static void focus_leave (GtkEventControllerFocus *controller, gpointer data)
{
	(void) controller; fabulor_spell_autocorrect_reset (GTK_ENTRY (data));
}

static void cursor_changed (GObject *object, GParamSpec *spec, gpointer data)
{
	AutoCorrect *ac = data;
	(void) object; (void) spec;
	if (!ac->busy && ac->undo_after) clear_undo (ac);
}

static void pointer_pressed (GtkGestureClick *gesture, gint count,
	gdouble x, gdouble y, gpointer data)
{
	(void) gesture; (void) count; (void) x; (void) y;
	fabulor_spell_autocorrect_reset (GTK_ENTRY (data));
}

static void free_owner (gpointer data)
{
	AutoCorrect *ac = data;
	reset (ac);
	g_free (ac);
}

void fabulor_spell_autocorrect_bind (GtkEntry *entry,
	FabulorAutoCorrectApprove approve, FabulorAutoCorrectSuggest suggest, gpointer data)
{
	AutoCorrect *ac;
	GtkEventController *keys, *focus;
	GtkGesture *click;
	GtkEditable *delegate;
	g_return_if_fail (GTK_IS_ENTRY (entry) && approve && suggest);
	if (g_object_get_data (G_OBJECT (entry), OWNER_KEY)) return;
	ac = g_new0 (AutoCorrect, 1);
	ac->entry = entry; ac->approve = approve; ac->suggest = suggest; ac->data = data;
	g_object_set_data_full (G_OBJECT (entry), OWNER_KEY, ac, free_owner);
	keys = gtk_event_controller_key_new ();
	gtk_event_controller_set_propagation_phase (keys, GTK_PHASE_CAPTURE);
	g_signal_connect_object (keys, "key-pressed", G_CALLBACK (key_pressed), entry, 0);
	gtk_widget_add_controller (GTK_WIDGET (entry), keys);
	focus = gtk_event_controller_focus_new ();
	g_signal_connect_object (focus, "leave", G_CALLBACK (focus_leave), entry, 0);
	gtk_widget_add_controller (GTK_WIDGET (entry), focus);
	click = gtk_gesture_click_new ();
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), 0);
	gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (click), GTK_PHASE_CAPTURE);
	g_signal_connect_object (click, "pressed", G_CALLBACK (pointer_pressed), entry, 0);
	gtk_widget_add_controller (GTK_WIDGET (entry), GTK_EVENT_CONTROLLER (click));
	g_signal_connect (entry, "changed", G_CALLBACK (changed), ac);
	g_signal_connect (entry, "notify::cursor-position", G_CALLBACK (cursor_changed), ac);
	delegate = gtk_editable_get_delegate (GTK_EDITABLE (entry));
	ac->ime_guard = delegate && g_signal_lookup ("preedit-changed", G_OBJECT_TYPE (delegate));
	if (ac->ime_guard)
		g_signal_connect_object (delegate, "preedit-changed", G_CALLBACK (preedit), entry, 0);
}

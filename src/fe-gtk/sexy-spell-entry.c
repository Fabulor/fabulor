/*
 * @file libsexy/sexy-icon-entry.c Entry widget
 *
 * @Copyright (C) 2004-2006 Christian Hammond.
 * Some of this code is from gtkspell, Copyright (C) 2002 Evan Martin.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include "sexy-spell-entry.h"
#include "spell-entry-style.h"
#include "spell-entry-widget.h"
#include "spell-entry-words.h"
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sexy-iso-codes.h"

#ifdef WIN32
#include "marshal.h"
#else
#include "../common/marshal.h"
#endif

#ifdef WIN32
#include "../common/typedef.h"
#include <glib/gwin32.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "../common/cfgfiles.h"
#include "../common/zoitechatc.h"
#include "theme/theme-access.h"
#include "theme/theme-manager.h"
#include "theme/theme-palette.h"
#include "gtk-compat.h"
#include "gtkutil.h"


#define ICON_ADD "zc-menu-add"
#define ICON_REMOVE "zc-menu-remove"
#define ICON_SPELL_CHECK "zc-menu-spell-check"

static void
color_to_spell_entry_color (const GdkRGBA *source,
	FabulorSpellEntryColor *destination)
{
	destination->red = (guint16) CLAMP (source->red * 65535.0 + 0.5,
		0.0, 65535.0);
	destination->green = (guint16) CLAMP (source->green * 65535.0 + 0.5,
		0.0, 65535.0);
	destination->blue = (guint16) CLAMP (source->blue * 65535.0 + 0.5,
		0.0, 65535.0);
}

static gboolean
resolve_spell_entry_mirc_color (gint color_index,
	FabulorSpellEntryColor *color, gpointer user_data)
{
	GdkRGBA rgba = { 0 };

	(void) user_data;
	if (color_index < 0 ||
		!theme_get_mirc_color ((unsigned int) color_index, &rgba))
		return FALSE;
	color_to_spell_entry_color (&rgba, color);
	return TRUE;
}

static void
get_spell_entry_palette (FabulorSpellEntryPalette *palette)
{
	GdkRGBA rgba = { 0 };

	memset (palette, 0, sizeof (*palette));
	if (theme_get_color (THEME_TOKEN_TEXT_FOREGROUND, &rgba))
		color_to_spell_entry_color (&rgba, &palette->text_foreground);
	if (theme_get_color (THEME_TOKEN_TEXT_BACKGROUND, &rgba))
		color_to_spell_entry_color (&rgba, &palette->text_background);
	if (theme_get_color (THEME_TOKEN_SPELL, &rgba))
		color_to_spell_entry_color (&rgba, &palette->spell_error);
	palette->resolve_mirc_color = resolve_spell_entry_mirc_color;
}

/*
 * Bunch of poop to make enchant into a runtime dependency rather than a
 * compile-time dependency.  This makes it so I don't have to hear the
 * complaints from people with binary distributions who don't get spell
 * checking because they didn't check their configure output.
 */
struct EnchantDict;
struct EnchantBroker;

typedef void (*EnchantDictDescribeFn) (const char * const lang_tag,
                                       const char * const provider_name,
                                       const char * const provider_desc,
                                       const char * const provider_file,
                                       void * user_data);

static struct EnchantBroker * (*enchant_broker_init) (void);
static void (*enchant_broker_free) (struct EnchantBroker * broker);
static void (*enchant_broker_free_dict) (struct EnchantBroker * broker, struct EnchantDict * dict);
static void (*enchant_broker_list_dicts) (struct EnchantBroker * broker, EnchantDictDescribeFn fn, void * user_data);
static struct EnchantDict * (*enchant_broker_request_dict) (struct EnchantBroker * broker, const char *const tag);

static void (*enchant_dict_add_to_personal) (struct EnchantDict * dict, const char *const word, ssize_t len);
static void (*enchant_dict_add_to_session) (struct EnchantDict * dict, const char *const word, ssize_t len);
static int (*enchant_dict_check) (struct EnchantDict * dict, const char *const word, ssize_t len);
static void (*enchant_dict_describe) (struct EnchantDict * dict, EnchantDictDescribeFn fn, void * user_data);
static void (*enchant_dict_free_suggestions) (struct EnchantDict * dict, char **suggestions);
static void (*enchant_dict_store_replacement) (struct EnchantDict * dict, const char *const mis, ssize_t mis_len, const char *const cor, ssize_t cor_len);
static char ** (*enchant_dict_suggest) (struct EnchantDict * dict, const char *const word, ssize_t len, size_t * out_n_suggs);
static gboolean have_enchant = FALSE;

struct _SexySpellEntryPriv
{
	struct EnchantBroker *broker;
	PangoAttrList        *attr_list;
	gint                  mark_character;
	GHashTable           *dict_hash;
	GSList               *dict_list;
	FabulorSpellWords    *words;
	guint                 theme_listener_id;
	gboolean              checked;
	gboolean              parseattr;
};

static void sexy_spell_entry_class_init(SexySpellEntryClass *klass);
static void sexy_spell_entry_init(SexySpellEntry *entry);
static void sexy_spell_entry_finalize(GObject *obj);
static void sexy_spell_entry_destroy(GObject *obj);
static gboolean sexy_spell_entry_button_press (GtkWidget *widget, guint button,
														guint n_press, gdouble x, gdouble y,
														GdkModifierType state, gpointer user_data);

/* GtkEditable handlers */
static void sexy_spell_entry_changed(GtkEditable *editable, gpointer data);

/* Other handlers */
static gboolean sexy_spell_entry_popup_menu(GtkWidget *widget, SexySpellEntry *entry);

/* Internal utility functions */
static gboolean   word_misspelled                             (SexySpellEntry       *entry,
                                                               int                   start,
                                                               int                   end);
static gboolean   default_word_check                          (SexySpellEntry       *entry,
                                                               const gchar          *word);
static gboolean   sexy_spell_entry_activate_language_internal (SexySpellEntry       *entry,
                                                               const gchar          *lang,
                                                               GError              **error);
static gchar     *get_lang_from_dict                          (struct EnchantDict   *dict);
static void       sexy_spell_entry_recheck_all                (SexySpellEntry       *entry);
static void       sexy_spell_entry_refresh_words              (SexySpellEntry       *entry);

static GtkEntryClass *parent_class = NULL;

#ifdef HAVE_ISO_CODES
static int codetable_ref = 0;
#endif

G_DEFINE_TYPE(SexySpellEntry, sexy_spell_entry, GTK_TYPE_ENTRY)

enum
{
	WORD_CHECK,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = {0};

#ifdef G_OS_WIN32
static GModule *
open_enchant_module_from_app_dir (const char *libname, char **loaded_path)
{
	GModule *module = NULL;
	char *base_path;
	char *candidate;

	if (loaded_path)
		*loaded_path = NULL;

	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (!base_path)
		return NULL;

	candidate = g_build_filename (base_path, libname, NULL);
	if (g_file_test (candidate, G_FILE_TEST_IS_REGULAR))
		module = g_module_open (candidate, 0);

	if (module && loaded_path)
		*loaded_path = g_strdup (candidate);

	g_free (candidate);
	g_free (base_path);
	return module;
}
#endif

static gboolean
spell_accumulator(GSignalInvocationHint *hint, GValue *return_accu, const GValue *handler_return, gpointer data)
{
	gboolean ret = g_value_get_boolean(handler_return);
	/* Handlers return TRUE if the word is misspelled.  In this
	 * case, it means that we want to stop if the word is checked
	 * as correct */
	g_value_set_boolean (return_accu, ret);
	return ret;
}

static void
initialize_enchant (void)
{
	GModule *enchant;
	gpointer funcptr;
    gsize i;
    const char * const libnames[] = {
#ifdef G_OS_WIN32
        "libenchant-2-2.dll",
#else
        "libenchant-2.so.2",
#endif
    };

    for (i = 0; i < G_N_ELEMENTS(libnames); ++i)
    {
#ifdef G_OS_WIN32
        char *loaded_path = NULL;
        enchant = open_enchant_module_from_app_dir(libnames[i], &loaded_path);
#else
        enchant = g_module_open(libnames[i], 0);
#endif
        if (enchant)
        {
#ifdef G_OS_WIN32
            g_info ("Loaded %s", loaded_path ? loaded_path : libnames[i]);
            g_free (loaded_path);
#else
            g_info ("Loaded %s", libnames[i]);
#endif
            have_enchant = TRUE;
            break;
        }
#ifdef G_OS_WIN32
        g_free (loaded_path);
#endif
    }

  if (!have_enchant)
    return;

#define MODULE_SYMBOL(name, func, alt_name) G_STMT_START { \
    const char *funcname = name; \
    gboolean ret = g_module_symbol(enchant, funcname, &funcptr); \
    if (ret == FALSE && alt_name) { \
        funcname = alt_name; \
        ret = g_module_symbol(enchant, funcname, &funcptr); \
    } \
    if (ret == FALSE) { \
        g_warning ("Failed to find enchant symbol %s", funcname); \
        have_enchant = FALSE; \
        return; \
    } \
    (func) = funcptr; \
} G_STMT_END;

	MODULE_SYMBOL("enchant_broker_init", enchant_broker_init, NULL)
	MODULE_SYMBOL("enchant_broker_free", enchant_broker_free, NULL)
	MODULE_SYMBOL("enchant_broker_free_dict", enchant_broker_free_dict, NULL)
	MODULE_SYMBOL("enchant_broker_list_dicts", enchant_broker_list_dicts, NULL)
	MODULE_SYMBOL("enchant_broker_request_dict", enchant_broker_request_dict, NULL)

	MODULE_SYMBOL("enchant_dict_add_to_personal", enchant_dict_add_to_personal,
                  "enchant_dict_add")
	MODULE_SYMBOL("enchant_dict_add_to_session", enchant_dict_add_to_session, NULL)
	MODULE_SYMBOL("enchant_dict_check", enchant_dict_check, NULL)
	MODULE_SYMBOL("enchant_dict_describe", enchant_dict_describe, NULL)
	MODULE_SYMBOL("enchant_dict_free_suggestions",
				  enchant_dict_free_suggestions, "enchant_dict_free_string_list")
	MODULE_SYMBOL("enchant_dict_store_replacement",
				  enchant_dict_store_replacement, NULL)
	MODULE_SYMBOL("enchant_dict_suggest", enchant_dict_suggest, NULL)
}

static void
sexy_spell_entry_class_init(SexySpellEntryClass *klass)
{
	GObjectClass *gobject_class;
	GObjectClass *object_class;

	initialize_enchant();

	parent_class = g_type_class_peek_parent(klass);

	gobject_class = G_OBJECT_CLASS(klass);
	object_class  = G_OBJECT_CLASS(klass);

	if (have_enchant)
		klass->word_check = default_word_check;

	gobject_class->finalize = sexy_spell_entry_finalize;

	object_class->dispose = sexy_spell_entry_destroy;

	/**
	 * SexySpellEntry::word-check:
	 * @entry: The entry on which the signal is emitted.
	 * @word: The word to check.
	 *
	 * The ::word-check signal is emitted whenever the entry has to check
	 * a word.  This allows the application to mark words as correct even
	 * if none of the active dictionaries contain it, such as nicknames in
	 * a chat client.
	 *
	 * Returns: %FALSE to indicate that the word should be marked as
	 * correct.
	 */
	signals[WORD_CHECK] = g_signal_new("word_check",
					   G_TYPE_FROM_CLASS(object_class),
					   G_SIGNAL_RUN_LAST,
					   G_STRUCT_OFFSET(SexySpellEntryClass, word_check),
					   (GSignalAccumulator) spell_accumulator, NULL,
					   _zoitechat_marshal_BOOLEAN__STRING,
					   G_TYPE_BOOLEAN,
					   1, G_TYPE_STRING);

}

static guint8
sexy_spell_entry_contrasting_caret_component (guint16 red, guint16 green, guint16 blue)
{
	const guint16 luma = (guint16) (((red >> 8) * 299 + (green >> 8) * 587 + (blue >> 8) * 114) / 1000);
	return luma >= 128 ? 0x00 : 0xff;
}

static void
sexy_spell_entry_apply_caret_style (SexySpellEntry *entry)
{
	ThemeWidgetStyleValues style_values;
	guint16 bg_red = 0, bg_green = 0, bg_blue = 0;
	guint8 caret;
	GtkCssProvider *provider;
	GtkStyleContext *context;
	char css[120];

	theme_get_widget_style_values_for_widget (GTK_WIDGET (entry), &style_values);
	theme_palette_color_get_rgb16 (&style_values.background, &bg_red, &bg_green, &bg_blue);
	caret = sexy_spell_entry_contrasting_caret_component (bg_red, bg_green, bg_blue);
	provider = g_object_get_data (G_OBJECT (entry), "sexy-spell-entry-caret-provider");
	if (!provider)
	{
		provider = gtk_css_provider_new ();
		g_object_set_data_full (G_OBJECT (entry), "sexy-spell-entry-caret-provider", provider, g_object_unref);
	}
	g_snprintf (css, sizeof (css), "#zoitechat-inputbox, #zoitechat-inputbox text { caret-color: #%02x%02x%02x; }",
		caret, caret, caret);
	gtk_css_provider_load_from_data (provider, css, -1, NULL);
	context = gtk_widget_get_style_context (GTK_WIDGET (entry));
	gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static void
sexy_spell_entry_style_updated (GtkWidget *widget, gpointer userdata)
{
	(void) userdata;
	if (SEXY_IS_SPELL_ENTRY (widget))
		sexy_spell_entry_apply_caret_style (SEXY_SPELL_ENTRY (widget));
}

static void
sexy_spell_entry_theme_changed (const ThemeChangedEvent *event, gpointer userdata)
{
	SexySpellEntry *entry = SEXY_SPELL_ENTRY (userdata);

	if (!theme_changed_event_has_reason (event, THEME_CHANGED_REASON_MODE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_PALETTE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_THEME_PACK) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_WIDGET_STYLE))
		return;

	sexy_spell_entry_apply_caret_style (entry);
	sexy_spell_entry_recheck_all (entry);
}

static gboolean
get_word_range_from_position (SexySpellEntry *entry, guint position,
	FabulorSpellWordRange *range)
{
	return fabulor_spell_words_find_character (entry->priv->words, position,
		range);
}

static void
add_to_dictionary(GtkWidget *menuitem, SexySpellEntry *entry)
{
	char *word;
	FabulorSpellWordRange range;
	struct EnchantDict *dict;

	if (!have_enchant)
		return;

	if (!get_word_range_from_position (entry, entry->priv->mark_character,
		&range))
		return;
	word = gtk_editable_get_chars (GTK_EDITABLE (entry),
		(gint) range.character_start, (gint) range.character_end);

	dict = (struct EnchantDict *) g_object_get_data(G_OBJECT(menuitem), "enchant-dict");
	if (dict)
		enchant_dict_add_to_personal(dict, word, -1);

	g_free(word);

	sexy_spell_entry_refresh_words (entry);
	sexy_spell_entry_recheck_all (entry);
}

static void
ignore_all(GtkWidget *menuitem, SexySpellEntry *entry)
{
	char *word;
	FabulorSpellWordRange range;
	GSList *li;

	if (!have_enchant)
		return;

	if (!get_word_range_from_position (entry, entry->priv->mark_character,
		&range))
		return;
	word = gtk_editable_get_chars (GTK_EDITABLE (entry),
		(gint) range.character_start, (gint) range.character_end);

	for (li = entry->priv->dict_list; li; li = g_slist_next (li)) {
		struct EnchantDict *dict = (struct EnchantDict *) li->data;
		enchant_dict_add_to_session(dict, word, -1);
	}

	g_free(word);

	sexy_spell_entry_refresh_words (entry);
	sexy_spell_entry_recheck_all(entry);
}

static void
replace_word(GtkWidget *menuitem, SexySpellEntry *entry)
{
	char *oldword;
	const char *newword;
	FabulorSpellWordRange range;
	gint start;
	gint end;
	gint cursor;
	struct EnchantDict *dict;

	if (!have_enchant)
		return;

	if (!get_word_range_from_position (entry, entry->priv->mark_character,
		&range))
		return;
	start = (gint) range.character_start;
	end = (gint) range.character_end;
	oldword = gtk_editable_get_chars(GTK_EDITABLE(entry), start, end);
	newword = gtk_menu_item_get_label (GTK_MENU_ITEM (menuitem));
	if (!newword)
	{
		/* GTK3 menu items may have a box child (icon + label). */
		GtkWidget *child = gtk_bin_get_child (GTK_BIN (menuitem));
		if (GTK_IS_LABEL (child))
		{
			newword = gtk_label_get_text (GTK_LABEL (child));
		}
		else if (GTK_IS_CONTAINER (child))
		{
			GList *kids, *l;
			kids = gtk_container_get_children (GTK_CONTAINER (child));
			for (l = kids; l; l = l->next)
			{
				if (GTK_IS_LABEL (l->data))
				{
					newword = gtk_label_get_text (GTK_LABEL (l->data));
					break;
				}
			}
			g_list_free (kids);
		}
	}
	if (!newword)
	{
		g_free (oldword);
		return;
	}


	cursor = gtk_editable_get_position(GTK_EDITABLE(entry));
	/* is the cursor at the end? If so, restore it there */
	if (g_utf8_strlen(gtk_entry_get_text(GTK_ENTRY(entry)), -1) == cursor)
		cursor = -1;
	else if(cursor > start && cursor <= end)
		cursor = start;

	gtk_editable_delete_text(GTK_EDITABLE(entry), start, end);
	gtk_editable_set_position(GTK_EDITABLE(entry), start);
	gtk_editable_insert_text(GTK_EDITABLE(entry), newword, strlen(newword),
							 &start);
	gtk_editable_set_position(GTK_EDITABLE(entry), cursor);

	dict = (struct EnchantDict *) g_object_get_data(G_OBJECT(menuitem), "enchant-dict");

        if (dict)
		enchant_dict_store_replacement(dict,
					       oldword, -1,
					       newword, -1);

	g_free(oldword);
}

static void
build_suggestion_menu(SexySpellEntry *entry, GtkWidget *menu, struct EnchantDict *dict, const gchar *word)
{
	GtkWidget *mi;
	gchar **suggestions;
	size_t n_suggestions, i;

	if (!have_enchant)
		return;

	suggestions = enchant_dict_suggest(dict, word, -1, &n_suggestions);

	if (suggestions == NULL || n_suggestions == 0) {
		/* no suggestions.  put something in the menu anyway... */
		GtkWidget *label = gtk_label_new("");
		gtk_label_set_markup(GTK_LABEL(label), _("<i>(no suggestions)</i>"));

		mi = gtk_separator_menu_item_new();
		gtk_container_add(GTK_CONTAINER(mi), label);
		gtk_widget_show_all(mi);
		gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), mi);
	} else {
		/* build a set of menus with suggestions */
		for (i = 0; i < n_suggestions; i++) {
			if ((i != 0) && (i % 10 == 0)) {
				mi = gtk_separator_menu_item_new();
				gtk_widget_show(mi);
				gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

				mi = gtk_menu_item_new_with_label(_("More..."));
				gtk_widget_show(mi);
				gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

				menu = gtk_menu_new();
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), menu);
			}

			mi = gtk_menu_item_new_with_label(suggestions[i]);
			g_object_set_data(G_OBJECT(mi), "enchant-dict", dict);
			g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(replace_word), entry);
			gtk_widget_show(mi);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		}
	}

	enchant_dict_free_suggestions(dict, suggestions);
}

static GtkWidget *
sexy_spell_entry_icon_menu_item (const char *label, const char *stock_name)
{
	GtkWidget *item;
	const char *icon_name;
	GtkWidget *box;
	GtkWidget *image = NULL;
	GtkWidget *label_widget;

	icon_name = gtkutil_icon_name_from_stock (stock_name);
	if (!icon_name)
		icon_name = stock_name;
	item = gtk_menu_item_new ();
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	if (icon_name)
		image = gtkutil_image_new_from_stock (icon_name, GTK_ICON_SIZE_MENU);
	label_widget = gtk_label_new_with_mnemonic (label);
	if (image)
		gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), label_widget, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (item), box);

	return item;
}

static GtkWidget *
build_spelling_menu(SexySpellEntry *entry, const gchar *word)
{
	struct EnchantDict *dict;
	GtkWidget *topmenu, *mi;
	gchar *label;

	if (!have_enchant)
		return NULL;

	topmenu = gtk_menu_new();

	if (entry->priv->dict_list == NULL)
		return topmenu;

	/* Suggestions */
	if (g_slist_length(entry->priv->dict_list) == 1) {
		dict = (struct EnchantDict *) entry->priv->dict_list->data;
		build_suggestion_menu(entry, topmenu, dict, word);
	} else {
		GSList *li;
		GtkWidget *menu;
		gchar *lang, *lang_name;

		for (li = entry->priv->dict_list; li; li = g_slist_next (li)) {
			dict = (struct EnchantDict *) li->data;
			lang = get_lang_from_dict(dict);
			lang_name = sexy_spell_entry_get_language_name (entry, lang);
			if (lang_name)
			{
				mi = gtk_menu_item_new_with_label(lang_name);
				g_free (lang_name);
			}
			else
			{
				mi = gtk_menu_item_new_with_label(lang);
			}
			g_free(lang);

			gtk_widget_show(mi);
			gtk_menu_shell_append(GTK_MENU_SHELL(topmenu), mi);
			menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), menu);
			build_suggestion_menu(entry, menu, dict, word);
		}
	}

	/* Separator */
	mi = gtk_separator_menu_item_new ();
	gtk_widget_show(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(topmenu), mi);

	/* + Add to Dictionary */
	label = g_strdup_printf(_("Add \"%s\" to Dictionary"), word);
	mi = sexy_spell_entry_icon_menu_item (label, ICON_ADD);
	g_free(label);

	if (g_slist_length(entry->priv->dict_list) == 1) {
		dict = (struct EnchantDict *) entry->priv->dict_list->data;
		g_object_set_data(G_OBJECT(mi), "enchant-dict", dict);
		g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(add_to_dictionary), entry);
	} else {
		GSList *li;
		GtkWidget *menu, *submi;
		gchar *lang, *lang_name;

		menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), menu);

		for (li = entry->priv->dict_list; li; li = g_slist_next(li)) {
			dict = (struct EnchantDict *)li->data;
			lang = get_lang_from_dict(dict);
			lang_name = sexy_spell_entry_get_language_name (entry, lang);
			if (lang_name)
			{
				submi = gtk_menu_item_new_with_label(lang_name);
				g_free (lang_name);
			}
			else 
			{
				submi = gtk_menu_item_new_with_label(lang);
			}
			g_free(lang);
			g_object_set_data(G_OBJECT(submi), "enchant-dict", dict);

			g_signal_connect(G_OBJECT(submi), "activate", G_CALLBACK(add_to_dictionary), entry);

			gtk_widget_show(submi);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), submi);
		}
	}

	gtk_widget_show_all(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(topmenu), mi);

	/* - Ignore All */
	mi = sexy_spell_entry_icon_menu_item (_("Ignore All"), ICON_REMOVE);
	g_signal_connect(G_OBJECT(mi), "activate", G_CALLBACK(ignore_all), entry);
	gtk_widget_show_all(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(topmenu), mi);

	return topmenu;
}

static void
sexy_spell_entry_populate_popup(SexySpellEntry *entry, GtkMenu *menu, gpointer data)
{
	GtkWidget *mi;
	FabulorSpellWordRange range;
	gchar *word;

	if ((have_enchant == FALSE) || (entry->priv->checked == FALSE))
		return;

	if (g_slist_length(entry->priv->dict_list) == 0)
		return;

	if (!get_word_range_from_position (entry, entry->priv->mark_character,
		&range))
		return;
	if (!word_misspelled (entry, (gint) range.byte_start,
		(gint) range.byte_end))
		return;

	/* separator */
	mi = gtk_separator_menu_item_new();
	gtk_widget_show(mi);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), mi);

	/* Above the separator, show the suggestions menu */
	mi = sexy_spell_entry_icon_menu_item (_("Spelling Suggestions"), ICON_SPELL_CHECK);

	word = gtk_editable_get_chars (GTK_EDITABLE (entry),
		(gint) range.character_start, (gint) range.character_end);
	g_assert(word != NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi), build_spelling_menu(entry, word));
	g_free(word);

	gtk_widget_show_all(mi);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), mi);
}

static void
sexy_spell_entry_init(SexySpellEntry *entry)
{
	entry->priv = g_new0(SexySpellEntryPriv, 1);

	entry->priv->dict_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	if (have_enchant)
	{
#ifdef HAVE_ISO_CODES
		if (codetable_ref == 0)
			codetable_init ();
		codetable_ref++;
#endif
		sexy_spell_entry_activate_default_languages(entry);
	}

	entry->priv->attr_list = pango_attr_list_new();

	entry->priv->checked = TRUE;
	entry->priv->parseattr = TRUE;

#if GTK_MAJOR_VERSION < 4
	g_signal_connect(G_OBJECT(entry), "popup-menu", G_CALLBACK(sexy_spell_entry_popup_menu), entry);
	g_signal_connect(G_OBJECT(entry), "populate-popup", G_CALLBACK(sexy_spell_entry_populate_popup), NULL);
	g_signal_connect(G_OBJECT(entry), "style-updated", G_CALLBACK(sexy_spell_entry_style_updated), NULL);
#endif
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(sexy_spell_entry_changed), NULL);
	fabulor_gtk_widget_on_multi_click (GTK_WIDGET (entry), sexy_spell_entry_button_press, NULL);
	entry->priv->theme_listener_id = theme_listener_register (
		"spell-entry", sexy_spell_entry_theme_changed, entry);
	sexy_spell_entry_apply_caret_style (entry);
}

static void
sexy_spell_entry_finalize(GObject *obj)
{
	SexySpellEntry *entry;

	g_return_if_fail(obj != NULL);
	g_return_if_fail(SEXY_IS_SPELL_ENTRY(obj));

	entry = SEXY_SPELL_ENTRY(obj);

	if (entry->priv->attr_list)
		pango_attr_list_unref(entry->priv->attr_list);
	if (entry->priv->dict_hash)
		g_hash_table_destroy(entry->priv->dict_hash);
	fabulor_spell_words_free (entry->priv->words);

	if (have_enchant) {
		if (entry->priv->broker) {
			GSList *li;
			for (li = entry->priv->dict_list; li; li = g_slist_next(li)) {
				struct EnchantDict *dict = (struct EnchantDict*) li->data;
				enchant_broker_free_dict (entry->priv->broker, dict);
			}
			g_slist_free (entry->priv->dict_list);

			enchant_broker_free(entry->priv->broker);
		}
	}

	g_free(entry->priv);
#ifdef HAVE_ISO_CODES
	codetable_ref--;
	if (codetable_ref == 0)
		codetable_free ();
#endif

	if (G_OBJECT_CLASS(parent_class)->finalize)
		G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
sexy_spell_entry_destroy(GObject *obj)
{
	SexySpellEntry *entry = SEXY_SPELL_ENTRY (obj);

	if (entry->priv->theme_listener_id)
	{
		theme_listener_unregister (entry->priv->theme_listener_id);
		entry->priv->theme_listener_id = 0;
	}

	if (G_OBJECT_CLASS(parent_class)->dispose)
		G_OBJECT_CLASS(parent_class)->dispose(obj);
}

/**
 * sexy_spell_entry_new
 *
 * Creates a new SexySpellEntry widget.
 *
 * Returns: a new #SexySpellEntry.
 */
GtkWidget *
sexy_spell_entry_new(void)
{
	return GTK_WIDGET(g_object_new(SEXY_TYPE_SPELL_ENTRY, NULL));
}

GQuark
sexy_spell_error_quark(void)
{
	static GQuark q = 0;
	if (q == 0)
		q = g_quark_from_static_string("sexy-spell-error-quark");
	return q;
}

static gboolean
default_word_check(SexySpellEntry *entry, const gchar *word)
{
	gboolean result = TRUE;
	GSList *li;

	if (!have_enchant)
		return result;

	if (g_unichar_isalpha(*word) == FALSE) {
		/* We only want to check words */
		return FALSE;
	}

	if (g_utf8_strlen (word, -1) > 20)
		return FALSE;
	for (li = entry->priv->dict_list; li; li = g_slist_next (li)) {
		struct EnchantDict *dict = (struct EnchantDict *) li->data;
		if (enchant_dict_check(dict, word, strlen(word)) == 0) {
			result = FALSE;
			break;
		}
	}
	return result;
}

static gboolean
word_misspelled(SexySpellEntry *entry, int start, int end)
{
	const gchar *text;
	gchar *word;
	gboolean ret;

	if (start == end)
		return FALSE;
	text = gtk_entry_get_text(GTK_ENTRY(entry));
	word = g_new0(gchar, end - start + 2);

	g_strlcpy(word, text + start, end - start + 1);

	g_signal_emit(entry, signals[WORD_CHECK], 0, word, &ret);

	g_free(word);
	return ret;
}

static void
check_word (SexySpellEntry *entry, int start, int end,
	const FabulorSpellEntryPalette *palette)
{
	if (word_misspelled(entry, start, end))
		fabulor_spell_entry_style_add_misspelling (entry->priv->attr_list,
			(guint) start, (guint) end, palette);
}

static void
sexy_spell_entry_recheck_all(SexySpellEntry *entry)
{
	FabulorSpellEntryPalette palette;
	PangoAttrList *attributes;
	guint i;
	const char *text;

	get_spell_entry_palette (&palette);
	text = gtk_entry_get_text (GTK_ENTRY (entry));
	attributes = fabulor_spell_entry_style_build (text,
		entry->priv->parseattr, &palette);
	pango_attr_list_unref(entry->priv->attr_list);
	entry->priv->attr_list = attributes;

	if (have_enchant && entry->priv->checked
		&& g_slist_length (entry->priv->dict_list) != 0)
	{
		/* Loop through words */
		for (i = 0; i < fabulor_spell_words_count (entry->priv->words); i++)
		{
			FabulorSpellWordRange range;
			if (fabulor_spell_words_get (entry->priv->words, i, &range))
				check_word (entry, (gint) range.byte_start,
					(gint) range.byte_end, &palette);
		}
	}

	gtk_entry_set_attributes (GTK_ENTRY (entry),
		fabulor_spell_entry_style_has_attributes (entry->priv->attr_list) ?
		entry->priv->attr_list : NULL);

	fabulor_spell_entry_queue_redraw (GTK_WIDGET (entry));
}

static gboolean
sexy_spell_entry_button_press (GtkWidget *widget, guint button, guint n_press,
	gdouble x, gdouble y, GdkModifierType state, gpointer user_data)
{
	SexySpellEntry *entry = SEXY_SPELL_ENTRY(widget);

	(void) button;
	(void) n_press;
	(void) y;
	(void) state;
	(void) user_data;
	entry->priv->mark_character = fabulor_spell_entry_pointer_position (
		GTK_ENTRY (widget), x);
	return FALSE;
}

static gboolean
sexy_spell_entry_popup_menu(GtkWidget *widget, SexySpellEntry *entry)
{
	/* Menu popped up from a keybinding (menu key or <shift>+F10). Use
	 * the cursor position as the mark position */
	entry->priv->mark_character = gtk_editable_get_position (GTK_EDITABLE (entry));
	return FALSE;
}

static void
sexy_spell_entry_refresh_words (SexySpellEntry *entry)
{
	PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (entry));
	PangoLanguage *language = pango_context_get_language (context);

	fabulor_spell_words_free (entry->priv->words);
	entry->priv->words = fabulor_spell_words_new (
		gtk_entry_get_text (GTK_ENTRY (entry)), language);
}

static void
sexy_spell_entry_changed(GtkEditable *editable, gpointer data)
{
	SexySpellEntry *entry = SEXY_SPELL_ENTRY(editable);

	sexy_spell_entry_refresh_words (entry);
	sexy_spell_entry_recheck_all(entry);
}

static gboolean
enchant_has_lang(const gchar *lang, GSList *langs) {
	GSList *i;
	for (i = langs; i; i = g_slist_next(i))
	{
		if (strcmp(lang, i->data) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * sexy_spell_entry_activate_default_languages:
 * @entry: A #SexySpellEntry.
 *
 * Activate spell checking for languages specified in the 
 * text_spell_langs setting. These languages are
 * activated by default, so this function need only be called
 * if they were previously deactivated.
 */
void
sexy_spell_entry_activate_default_languages(SexySpellEntry *entry)
{
	GSList *enchant_langs;
	char *lang, **i, **langs;

	if (!have_enchant)
		return;

	if (!entry->priv->broker)
		entry->priv->broker = enchant_broker_init();

	enchant_langs = sexy_spell_entry_get_languages(entry);

	langs = g_strsplit_set (prefs.hex_text_spell_langs, ", \t", 0);

	for (i = langs; *i; i++)
	{
		lang = *i;

		if (enchant_has_lang (lang, enchant_langs))
		{
			sexy_spell_entry_activate_language_internal (entry, lang, NULL);
		}
	}

	g_slist_foreach(enchant_langs, (GFunc) g_free, NULL);
	g_slist_free(enchant_langs);
	g_strfreev (langs);

	/* If we don't have any languages activated, use "en" */
	if (entry->priv->dict_list == NULL)
		sexy_spell_entry_activate_language_internal(entry, "en", NULL);

	sexy_spell_entry_recheck_all (entry);
}

static void
get_lang_from_dict_cb(const char * const lang_tag,
		      const char * const provider_name,
		      const char * const provider_desc,
		      const char * const provider_file,
		      void * user_data) {
	gchar **lang = (gchar **)user_data;
	*lang = g_strdup(lang_tag);
}

static gchar *
get_lang_from_dict(struct EnchantDict *dict)
{
	gchar *lang;

	if (!have_enchant)
		return NULL;

	enchant_dict_describe(dict, get_lang_from_dict_cb, &lang);
	return lang;
}

static gboolean
sexy_spell_entry_activate_language_internal(SexySpellEntry *entry, const gchar *lang, GError **error)
{
	struct EnchantDict *dict;

	if (!have_enchant)
		return FALSE;

	if (!entry->priv->broker)
		entry->priv->broker = enchant_broker_init();

	if (g_hash_table_lookup(entry->priv->dict_hash, lang))
		return TRUE;

	dict = enchant_broker_request_dict(entry->priv->broker, lang);

	if (!dict) {
		g_set_error(error, SEXY_SPELL_ERROR, SEXY_SPELL_ERROR_BACKEND, _("enchant error for language: %s"), lang);
		return FALSE;
	}

	enchant_dict_add_to_session (dict, "ZoiteChat", strlen("ZoiteChat"));
	entry->priv->dict_list = g_slist_append(entry->priv->dict_list, (gpointer) dict);
	g_hash_table_insert(entry->priv->dict_hash, get_lang_from_dict(dict), (gpointer) dict);

	return TRUE;
}

static void
dict_describe_cb(const char * const lang_tag,
		 const char * const provider_name,
		 const char * const provider_desc,
		 const char * const provider_file,
		 void * user_data)
{
	GSList **langs = (GSList **)user_data;

	*langs = g_slist_append(*langs, (gpointer)g_strdup(lang_tag));
}

/**
 * sexy_spell_entry_get_languages:
 * @entry: A #SexySpellEntry.
 *
 * Retrieve a list of language codes for which dictionaries are available.
 *
 * Returns: a new #GList object, or %NULL on error.
 */
GSList *
sexy_spell_entry_get_languages(const SexySpellEntry *entry)
{
	GSList *langs = NULL;

	g_return_val_if_fail(entry != NULL, NULL);
	g_return_val_if_fail(SEXY_IS_SPELL_ENTRY(entry), NULL);

	if (enchant_broker_list_dicts == NULL)
		return NULL;

	if (!entry->priv->broker)
		return NULL;

	enchant_broker_list_dicts(entry->priv->broker, dict_describe_cb, &langs);

	return langs;
}

/**
 * sexy_spell_entry_get_language_name:
 * @entry: A #SexySpellEntry.
 * @lang: The language code to lookup a friendly name for.
 *
 * Get a friendly name for a given locale.
 *
 * Returns: The name of the locale. Should be freed with g_free()
 */
gchar *
sexy_spell_entry_get_language_name(const SexySpellEntry *entry,
								   const gchar *lang)
{
#ifdef HAVE_ISO_CODES
	const gchar *lang_name = "";
	const gchar *country_name = "";

	g_return_val_if_fail (have_enchant, NULL);

	if (codetable_ref == 0)
		codetable_init ();
		
	codetable_lookup (lang, &lang_name, &country_name);

	if (codetable_ref == 0)
		codetable_free ();

	if (strlen (country_name) != 0)
		return g_strdup_printf ("%s (%s)", lang_name, country_name);
	else
		return g_strdup_printf ("%s", lang_name);
#else
	return g_strdup (lang);
#endif
}

/**
 * sexy_spell_entry_language_is_active:
 * @entry: A #SexySpellEntry.
 * @lang: The language to use, in a form enchant understands.
 *
 * Determine if a given language is currently active.
 *
 * Returns: TRUE if the language is active.
 */
gboolean
sexy_spell_entry_language_is_active(const SexySpellEntry *entry,
									const gchar *lang)
{
	return (g_hash_table_lookup(entry->priv->dict_hash, lang) != NULL);
}

/**
 * sexy_spell_entry_activate_language:
 * @entry: A #SexySpellEntry
 * @lang: The language to use in a form Enchant understands. Typically either
 *        a two letter language code or a locale code in the form xx_XX.
 * @error: Return location for error.
 *
 * Activate spell checking for the language specifed.
 *
 * Returns: FALSE if there was an error.
 */
gboolean
sexy_spell_entry_activate_language(SexySpellEntry *entry, const gchar *lang, GError **error)
{
	gboolean ret;

	g_return_val_if_fail(entry != NULL, FALSE);
	g_return_val_if_fail(SEXY_IS_SPELL_ENTRY(entry), FALSE);
	g_return_val_if_fail(lang != NULL && *lang != '\0', FALSE);

	if (!have_enchant)
		return FALSE;

	if (error)
		g_return_val_if_fail(*error == NULL, FALSE);

	ret = sexy_spell_entry_activate_language_internal(entry, lang, error);

	if (ret) {
		sexy_spell_entry_refresh_words (entry);
		sexy_spell_entry_recheck_all(entry);
	}

	return ret;
}

/**
 * sexy_spell_entry_deactivate_language:
 * @entry: A #SexySpellEntry.
 * @lang: The language in a form Enchant understands. Typically either
 *        a two letter language code or a locale code in the form xx_XX.
 *
 * Deactivate spell checking for the language specifed.
 */
void
sexy_spell_entry_deactivate_language(SexySpellEntry *entry, const gchar *lang)
{
	g_return_if_fail(entry != NULL);
	g_return_if_fail(SEXY_IS_SPELL_ENTRY(entry));

	if (!have_enchant)
		return;

	if (!entry->priv->dict_list)
		return;

	if (lang) {
		struct EnchantDict *dict;

		dict = g_hash_table_lookup(entry->priv->dict_hash, lang);
		if (!dict)
			return;
		enchant_broker_free_dict(entry->priv->broker, dict);
		entry->priv->dict_list = g_slist_remove(entry->priv->dict_list, dict);
		g_hash_table_remove (entry->priv->dict_hash, lang);
	} else {
		/* deactivate all */
		GSList *li;
		struct EnchantDict *dict;

		for (li = entry->priv->dict_list; li; li = g_slist_next(li)) {
			dict = (struct EnchantDict *)li->data;
			enchant_broker_free_dict(entry->priv->broker, dict);
		}

		g_slist_free (entry->priv->dict_list);
		g_hash_table_destroy (entry->priv->dict_hash);
		entry->priv->dict_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
		entry->priv->dict_list = NULL;
	}

	sexy_spell_entry_refresh_words (entry);
	sexy_spell_entry_recheck_all(entry);
}

/**
 * sexy_spell_entry_set_active_languages:
 * @entry: A #SexySpellEntry
 * @langs: A list of language codes to activate, in a form Enchant understands.
 *         Typically either a two letter language code or a locale code in the
 *         form xx_XX.
 * @error: Return location for error.
 *
 * Activate spell checking for only the languages specified.
 *
 * Returns: FALSE if there was an error.
 */
gboolean
sexy_spell_entry_set_active_languages(SexySpellEntry *entry, GSList *langs, GError **error)
{
	GSList *li;

	g_return_val_if_fail(entry != NULL, FALSE);
	g_return_val_if_fail(SEXY_IS_SPELL_ENTRY(entry), FALSE);
	g_return_val_if_fail(langs != NULL, FALSE);

	if (!have_enchant)
		return FALSE;

	/* deactivate all languages first */
	sexy_spell_entry_deactivate_language(entry, NULL);

	for (li = langs; li; li = g_slist_next(li)) {
		if (sexy_spell_entry_activate_language_internal(entry,
		    (const gchar *) li->data, error) == FALSE)
			return FALSE;
	}
	sexy_spell_entry_refresh_words (entry);
	sexy_spell_entry_recheck_all(entry);
	return TRUE;
}

/**
 * sexy_spell_entry_get_active_languages:
 * @entry: A #SexySpellEntry
 *
 * Retrieve a list of the currently active languages.
 *
 * Returns: A GSList of char* values with language codes (en, fr, etc).  Both
 *          the data and the list must be freed by the user.
 */
GSList *
sexy_spell_entry_get_active_languages(SexySpellEntry *entry)
{
	GSList *ret = NULL, *li;
	struct EnchantDict *dict;
	gchar *lang;

	g_return_val_if_fail(entry != NULL, NULL);
	g_return_val_if_fail(SEXY_IS_SPELL_ENTRY(entry), NULL);

	if (!have_enchant)
		return NULL;

	for (li = entry->priv->dict_list; li; li = g_slist_next(li)) {
		dict = (struct EnchantDict *) li->data;
		lang = get_lang_from_dict(dict);
		ret = g_slist_append(ret, lang);
	}
	return ret;
}

/**
 * sexy_spell_entry_is_checked:
 * @entry: A #SexySpellEntry.
 *
 * Queries a #SexySpellEntry and returns whether the entry has spell-checking enabled.
 *
 * Returns: TRUE if the entry has spell-checking enabled.
 */
gboolean
sexy_spell_entry_is_checked(SexySpellEntry *entry)
{
	return entry->priv->checked;
}

/**
 * sexy_spell_entry_set_checked:
 * @entry: A #SexySpellEntry.
 * @checked: Whether to enable spell-checking
 *
 * Sets whether the entry has spell-checking enabled.
 */
void
sexy_spell_entry_set_checked(SexySpellEntry *entry, gboolean checked)
{
	GtkWidget *widget;

	if (entry->priv->checked == checked)
		return;

	entry->priv->checked = checked;
	widget = GTK_WIDGET(entry);

	if (checked == FALSE && gtk_widget_get_realized (widget))
	{
		/* This will unmark any existing */
		sexy_spell_entry_recheck_all (entry);
	}
	else
	{
		sexy_spell_entry_refresh_words (entry);
		sexy_spell_entry_recheck_all(entry);
	}
}

/**
* sexy_spell_entry_set_parse_attributes:
* @entry: A #SexySpellEntry.
* @parse: Whether to enable showing attributes
*
* Sets whether to enable showing attributes is enabled.
*/
void
sexy_spell_entry_set_parse_attributes (SexySpellEntry *entry, gboolean parse)
{
	GtkWidget *widget;

	if (entry->priv->parseattr == parse)
		return;

	entry->priv->parseattr = parse;
	widget = GTK_WIDGET (entry);

	if (parse == FALSE && gtk_widget_get_realized (widget))
	{
		/* This will remove current attrs */
		sexy_spell_entry_recheck_all (entry);
	}
	else
	{
		sexy_spell_entry_refresh_words (entry);
		sexy_spell_entry_recheck_all (entry);
	}
}

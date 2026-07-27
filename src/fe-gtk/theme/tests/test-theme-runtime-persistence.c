/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "../../../common/zoitechat.h"
#include "../../../common/zoitechatc.h"

#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>
#ifdef WIN32
#include <io.h>
#define write _write
#else
#include <unistd.h>
#endif

#include "../theme-runtime.h"
#include "../theme-palette-transaction.h"

struct session *current_sess;
struct session *current_tab;
struct session *lastact_sess;
struct zoitechatprefs prefs;

static char *test_home_dir;
static gboolean stub_system_prefers_dark;

static char *
test_home_path (const char *file)
{
	return g_build_filename (test_home_dir, file, NULL);
}

static gboolean
read_line_value (const char *cfg, const char *key, char *out, gsize out_len)
{
	char *pattern;
	char *pos;
	char *line_end;
	gsize value_len;

	pattern = g_strdup_printf ("%s = ", key);
	pos = g_strstr_len (cfg, -1, pattern);
	g_free (pattern);
	if (!pos)
		return FALSE;

	pos = strchr (pos, '=');
	if (!pos)
		return FALSE;
	pos++;
	while (*pos == ' ')
		pos++;

	line_end = strchr (pos, '\n');
	if (!line_end)
		line_end = pos + strlen (pos);
	value_len = (gsize) (line_end - pos);
	if (value_len + 1 > out_len)
		return FALSE;
	memcpy (out, pos, value_len);
	out[value_len] = '\0';
	return TRUE;
}

int
cfg_get_color (char *cfg, char *var, guint16 *r, guint16 *g, guint16 *b)
{
	char value[128];

	if (!read_line_value (cfg, var, value, sizeof (value)))
		return 0;
	if (sscanf (value, "%04hx %04hx %04hx", r, g, b) != 3)
		return 0;
	return 1;
}

int
cfg_get_int (char *cfg, char *var)
{
	char value[128];

	if (!read_line_value (cfg, var, value, sizeof (value)))
		return 0;
	return atoi (value);
}

int
cfg_put_color (int fh, guint16 r, guint16 g, guint16 b, char *var)
{
	char line[256];
	int len;

	len = g_snprintf (line, sizeof line, "%s = %04hx %04hx %04hx\n", var, r, g, b);
	if (len < 0)
		return 0;
	return write (fh, line, (size_t) len) == len;
}

int
cfg_put_int (int fh, int value, char *var)
{
	char line[128];
	int len;

	len = g_snprintf (line, sizeof line, "%s = %d\n", var, value);
	if (len < 0)
		return 0;
	return write (fh, line, (size_t) len) == len;
}

char *
get_xdir (void)
{
	return test_home_dir;
}

int
zoitechat_open_file (const char *file, int flags, int mode, int xof_flags)
{
	char *path;
	int fd;

	(void) xof_flags;
	path = g_build_filename (test_home_dir, file, NULL);
	fd = g_open (path, flags, mode);
	g_free (path);
	return fd;
}

gboolean
fe_dark_mode_is_enabled_for (unsigned int mode)
{
	return mode == ZOITECHAT_DARK_MODE_DARK;
}

gboolean
theme_policy_system_prefers_dark (void)
{
	return stub_system_prefers_dark;
}

gboolean
theme_policy_is_dark_mode_active (unsigned int mode)
{
	if (mode == ZOITECHAT_DARK_MODE_AUTO)
		return stub_system_prefers_dark;

	return mode == ZOITECHAT_DARK_MODE_DARK;
}

static void
setup_temp_home (void)
{
	stub_system_prefers_dark = FALSE;

	if (test_home_dir)
		return;
	test_home_dir = g_dir_make_tmp ("zoitechat-theme-tests-XXXXXX", NULL);
	g_assert_nonnull (test_home_dir);
}

static char *
read_colors_conf (void)
{
	char *path;
	char *content = NULL;
	gsize length = 0;
	gboolean ok;

	path = test_home_path ("colors.conf");
	ok = g_file_get_contents (path, &content, &length, NULL);
	g_free (path);
	g_assert_true (ok);
	g_assert_cmpuint (length, >, 0);
	return content;
}

static gboolean
colors_equal (const GdkRGBA *a, const GdkRGBA *b)
{
	return a->red == b->red && a->green == b->green && a->blue == b->blue;
}

static void
apply_ui_color_edit (unsigned int mode, ThemeSemanticToken token, const char *hex)
{
	GdkRGBA color;

	g_assert_true (gdk_rgba_parse (&color, hex));
	if (theme_policy_is_dark_mode_active (mode))
		theme_runtime_dark_set_color (token, &color);
	else
		theme_runtime_user_set_color (token, &color);
	theme_runtime_apply_mode (mode, NULL);
}

static void
test_persistence_roundtrip_light_and_dark (void)
{
	GdkRGBA light_color;
	GdkRGBA dark_color;
	GdkRGBA loaded;
	char *cfg;

	setup_temp_home ();
	theme_runtime_load ();

	gdk_rgba_parse (&light_color, "#123456");
	theme_runtime_user_set_color (THEME_TOKEN_MIRC_0, &light_color);
	theme_runtime_apply_dark_mode (FALSE);

	theme_runtime_apply_dark_mode (TRUE);
	gdk_rgba_parse (&dark_color, "#abcdef");
	theme_runtime_dark_set_color (THEME_TOKEN_MIRC_0, &dark_color);

	theme_runtime_save ();
	cfg = read_colors_conf ();
	g_assert_nonnull (g_strstr_len (cfg, -1, "theme.mode.light.token.mirc_0"));
	g_assert_nonnull (g_strstr_len (cfg, -1, "theme.mode.dark.token.mirc_0"));
	g_assert_null (g_strstr_len (cfg, -1, "color_0 = "));
	g_assert_null (g_strstr_len (cfg, -1, "dark_color_0 = "));
	g_free (cfg);

	theme_runtime_load ();
	theme_runtime_apply_dark_mode (FALSE);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &loaded));
	g_assert_true (colors_equal (&light_color, &loaded));

	theme_runtime_apply_dark_mode (TRUE);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &loaded));
	g_assert_true (colors_equal (&dark_color, &loaded));
}

static void
test_loads_legacy_color_keys_via_migration_loader (void)
{
	char *path;
	const char *legacy_cfg =
		"color_0 = 1111 2222 3333\n"
		"dark_color_0 = aaaa bbbb cccc\n";
	GdkRGBA loaded;
	GdkRGBA light_expected;
	GdkRGBA dark_expected;
	gboolean ok;

	setup_temp_home ();
	path = test_home_path ("colors.conf");
	ok = g_file_set_contents (path, legacy_cfg, -1, NULL);
	g_free (path);
	g_assert_true (ok);

	theme_runtime_load ();

	gdk_rgba_parse (&light_expected, "#111122223333");
	gdk_rgba_parse (&dark_expected, "#aaaabbbbcccc");

	theme_runtime_apply_dark_mode (FALSE);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &loaded));
	g_assert_true (colors_equal (&loaded, &light_expected));

	theme_runtime_apply_dark_mode (TRUE);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &loaded));
	g_assert_true (colors_equal (&loaded, &dark_expected));
}


static void
test_ui_edits_persist_without_legacy_array_mutation (void)
{
	GdkRGBA light_loaded;
	GdkRGBA dark_loaded;
	GdkRGBA light_expected;
	GdkRGBA dark_expected;

	setup_temp_home ();
	theme_runtime_load ();

	apply_ui_color_edit (ZOITECHAT_DARK_MODE_LIGHT, THEME_TOKEN_SELECTION_FOREGROUND, "#224466");
	apply_ui_color_edit (ZOITECHAT_DARK_MODE_DARK, THEME_TOKEN_SELECTION_FOREGROUND, "#88aacc");
	theme_runtime_save ();

	theme_runtime_load ();
	theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_LIGHT, NULL);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_SELECTION_FOREGROUND, &light_loaded));
	g_assert_true (gdk_rgba_parse (&light_expected, "#224466"));
	g_assert_true (colors_equal (&light_loaded, &light_expected));

	theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_DARK, NULL);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_SELECTION_FOREGROUND, &dark_loaded));
	g_assert_true (gdk_rgba_parse (&dark_expected, "#88aacc"));
	g_assert_true (colors_equal (&dark_loaded, &dark_expected));
}

static void
test_apply_mode_resolves_auto_from_system_preference (void)
{
	gboolean palette_changed = FALSE;
	gboolean dark;

	setup_temp_home ();
	theme_runtime_load ();

	stub_system_prefers_dark = TRUE;
	dark = theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_AUTO, &palette_changed);
	g_assert_true (dark);
	g_assert_true (theme_runtime_is_dark_active ());

	stub_system_prefers_dark = FALSE;
	dark = theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_AUTO, &palette_changed);
	g_assert_false (dark);
	g_assert_false (theme_runtime_is_dark_active ());
}

static void
test_apply_mode_respects_explicit_dark_and_light_modes (void)
{
	gboolean palette_changed = FALSE;

	setup_temp_home ();
	theme_runtime_load ();

	g_assert_true (theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_DARK, &palette_changed));
	g_assert_true (theme_runtime_is_dark_active ());

	g_assert_false (theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_LIGHT, &palette_changed));
	g_assert_false (theme_runtime_is_dark_active ());
}

static void
test_gtk_map_colors_blend_with_palette_without_transparency (void)
{
	ThemeGtkPaletteMap map = { 0 };
	ThemeWidgetStyleValues base_values;
	ThemeWidgetStyleValues values;
	GdkRGBA mapped_bg;
	double alpha;
	double expected_red;
	double expected_green;
	double expected_blue;

	setup_temp_home ();
	theme_runtime_load ();
	theme_runtime_get_widget_style_values (&base_values);

	map.enabled = TRUE;
	g_assert_true (gdk_rgba_parse (&map.text_foreground, "rgba(10, 20, 30, 0.25)"));
	g_assert_true (gdk_rgba_parse (&map.text_background, "rgba(40, 50, 60, 0.30)"));
	g_assert_true (gdk_rgba_parse (&map.selection_foreground, "rgba(70, 80, 90, 0.35)"));
	g_assert_true (gdk_rgba_parse (&map.selection_background, "rgba(100, 110, 120, 0.40)"));
	g_assert_true (gdk_rgba_parse (&map.accent, "rgba(130, 140, 150, 0.45)"));

	theme_runtime_get_widget_style_values_mapped (&map, &values);
	g_assert_cmpfloat (values.foreground.alpha, ==, 1.0);
	g_assert_cmpfloat (values.background.alpha, ==, 1.0);
	g_assert_cmpfloat (values.selection_foreground.alpha, ==, 1.0);
	g_assert_cmpfloat (values.selection_background.alpha, ==, 1.0);

	mapped_bg = map.text_background;
	alpha = mapped_bg.alpha;
	expected_red = (mapped_bg.red * alpha) + (base_values.background.red * (1.0 - alpha));
	expected_green = (mapped_bg.green * alpha) + (base_values.background.green * (1.0 - alpha));
	expected_blue = (mapped_bg.blue * alpha) + (base_values.background.blue * (1.0 - alpha));
	g_assert_true (fabs (values.background.red - expected_red) < 0.0001);
	g_assert_true (fabs (values.background.green - expected_green) < 0.0001);
	g_assert_true (fabs (values.background.blue - expected_blue) < 0.0001);
}


static void
test_gtk_map_uses_theme_defaults_until_custom_token_is_set (void)
{
	ThemeGtkPaletteMap map = { 0 };
	ThemeWidgetStyleValues values;
	GdkRGBA custom;

	setup_temp_home ();
	theme_runtime_load ();

	map.enabled = TRUE;
	g_assert_true (gdk_rgba_parse (&map.text_foreground, "#010203"));
	g_assert_true (gdk_rgba_parse (&map.text_background, "#111213"));
	g_assert_true (gdk_rgba_parse (&map.selection_foreground, "#212223"));
	g_assert_true (gdk_rgba_parse (&map.selection_background, "#313233"));
	g_assert_true (gdk_rgba_parse (&map.accent, "#414243"));

	theme_runtime_get_widget_style_values_mapped (&map, &values);
	g_assert_true (colors_equal (&values.foreground, &map.text_foreground));

	g_assert_true (gdk_rgba_parse (&custom, "#a1b2c3"));
	theme_runtime_user_set_color (THEME_TOKEN_TEXT_FOREGROUND, &custom);
	theme_runtime_apply_mode (ZOITECHAT_DARK_MODE_LIGHT, NULL);
	theme_runtime_get_widget_style_values_mapped (&map, &values);
	g_assert_true (colors_equal (&values.foreground, &custom));
}


static void
test_save_finalize_replaces_colors_conf_atomically (void)
{
	char *path;
	char *temp_path = NULL;
	char *cfg = NULL;
	gboolean ok;

	setup_temp_home ();
	path = test_home_path ("colors.conf");
	ok = g_file_set_contents (path, "theme.mode.light.token.mirc_0 = 0000 0000 0000\n", -1, NULL);
	g_assert_true (ok);

	theme_runtime_load ();
	g_assert_true (theme_runtime_save_prepare (&temp_path));
	g_assert_nonnull (temp_path);
	g_assert_nonnull (g_strrstr (temp_path, "colors.conf.new."));
	g_assert_true (g_file_test (temp_path, G_FILE_TEST_EXISTS));
	g_assert_true (theme_runtime_save_finalize (temp_path));
	g_assert_false (g_file_test (temp_path, G_FILE_TEST_EXISTS));
	ok = g_file_get_contents (path, &cfg, NULL, NULL);
	g_assert_true (ok);
	g_assert_nonnull (g_strstr_len (cfg, -1, "theme.palette.semantic_migrated = 1"));
	g_free (cfg);
	g_free (temp_path);
	g_free (path);
}

static void
test_save_discard_unlinks_temp_file (void)
{
	char *temp_path = NULL;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_true (theme_runtime_save_prepare (&temp_path));
	g_assert_nonnull (temp_path);
	g_assert_true (g_file_test (temp_path, G_FILE_TEST_EXISTS));
	theme_runtime_save_discard (temp_path);
	g_assert_false (g_file_test (temp_path, G_FILE_TEST_EXISTS));
	g_free (temp_path);
}

static void
test_save_writes_only_custom_token_keys (void)
{
	GdkRGBA custom;
	char *cfg;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_true (gdk_rgba_parse (&custom, "#445566"));
	theme_runtime_user_set_color (THEME_TOKEN_TEXT_FOREGROUND, &custom);
	theme_runtime_save ();

	cfg = read_colors_conf ();
	g_assert_nonnull (g_strstr_len (cfg, -1, "theme.mode.light.token.text_foreground"));
	g_assert_null (g_strstr_len (cfg, -1, "theme.mode.light.token.text_background"));
	g_free (cfg);
}

static void
test_palette_candidate_parse_does_not_mutate_runtime (void)
{
	const char *contents = "color_0 = 1234 5678 9abc\n";
	ThemePaletteCandidate candidate = { 0 };
	GdkRGBA before;
	GdkRGBA after;
	GdkRGBA imported;
	GError *error = NULL;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &before));
	g_assert_true (theme_runtime_palette_candidate_from_colors (contents,
		FALSE, &candidate, &error));
	g_assert_no_error (error);
	g_assert_cmpuint (candidate.supplied_count, ==, 1);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &after));
	g_assert_true (colors_equal (&before, &after));
	g_assert_true (theme_palette_get_color (&candidate.palette,
		THEME_TOKEN_MIRC_0, &imported));
	g_assert_cmpfloat_with_epsilon (imported.red, 0x1234 / 65535.0, 0.00001);
	g_assert_cmpfloat_with_epsilon (imported.green, 0x5678 / 65535.0, 0.00001);
	g_assert_cmpfloat_with_epsilon (imported.blue, 0x9abc / 65535.0, 0.00001);
}

static void
test_palette_candidate_rejects_malformed_without_mutation (void)
{
	const char *contents =
		"theme.mode.light.token.mirc_0 = invalid\n"
		"color_0 = 1234 5678 9abc\n";
	ThemePaletteCandidate candidate = { 0 };
	GdkRGBA before;
	GdkRGBA after;
	GError *error = NULL;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &before));
	g_assert_false (theme_runtime_palette_candidate_from_colors (contents,
		FALSE, &candidate, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_clear_error (&error);
	g_assert_false (candidate.initialized);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &after));
	g_assert_true (colors_equal (&before, &after));
}

static void
test_palette_candidate_applies_and_restores_as_complete_palette (void)
{
	const char *contents =
		"color_0 = 1234 5678 9abc\n"
		"color_1 = abcd bcde cdef\n";
	ThemePaletteCandidate snapshot = { 0 };
	ThemePaletteCandidate candidate = { 0 };
	GdkRGBA original_zero;
	GdkRGBA original_one;
	GdkRGBA current;
	GError *error = NULL;
	gboolean changed = FALSE;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0,
		&original_zero));
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_1,
		&original_one));
	theme_runtime_palette_snapshot (FALSE, &snapshot);
	g_assert_true (theme_runtime_palette_candidate_from_colors (contents,
		FALSE, &candidate, &error));
	g_assert_no_error (error);
	g_assert_true (theme_runtime_apply_palette_candidate (&candidate,
		&changed));
	g_assert_true (changed);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &current));
	g_assert_false (colors_equal (&current, &original_zero));
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_1, &current));
	g_assert_false (colors_equal (&current, &original_one));

	changed = FALSE;
	g_assert_true (theme_runtime_apply_palette_candidate (&snapshot,
		&changed));
	g_assert_true (changed);
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_0, &current));
	g_assert_true (colors_equal (&current, &original_zero));
	g_assert_true (theme_runtime_get_color (THEME_TOKEN_MIRC_1, &current));
	g_assert_true (colors_equal (&current, &original_one));
}

static void
test_palette_candidate_rejects_empty_palette (void)
{
	ThemePaletteCandidate candidate = { 0 };
	GError *error = NULL;

	setup_temp_home ();
	theme_runtime_load ();
	g_assert_false (theme_runtime_palette_candidate_from_colors (
		"unrelated = value\n", FALSE, &candidate, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_clear_error (&error);
	g_assert_false (candidate.initialized);
}

static void
test_palette_transaction_tracks_edit_and_revert (void)
{
	ThemePaletteCandidate snapshot = { 0 };
	ThemePaletteTransaction transaction = { 0 };
	GdkRGBA original;
	GdkRGBA edited;
	GdkRGBA staged;

	setup_temp_home ();
	theme_runtime_load ();
	theme_runtime_palette_snapshot (FALSE, &snapshot);
	g_assert_true (theme_palette_get_color (&snapshot.palette,
		THEME_TOKEN_MIRC_0, &original));
	edited = original;
	edited.red = original.red > 0.5 ? 0.25 : 0.75;

	g_assert_true (theme_palette_transaction_begin (&transaction,
		ZOITECHAT_DARK_MODE_LIGHT, &snapshot));
	g_assert_false (transaction.changed);
	g_assert_true (theme_palette_transaction_set_color (&transaction,
		THEME_TOKEN_MIRC_0, &edited));
	g_assert_true (transaction.changed);
	g_assert_true (theme_palette_transaction_get_color (&transaction,
		THEME_TOKEN_MIRC_0, &staged));
	g_assert_true (colors_equal (&edited, &staged));
	g_assert_true (theme_palette_transaction_set_color (&transaction,
		THEME_TOKEN_MIRC_0, &original));
	g_assert_false (transaction.changed);
}

static void
test_palette_transaction_replaces_candidate_and_keeps_snapshot (void)
{
	const char *contents = "color_1 = 1234 5678 9abc\n";
	ThemePaletteCandidate snapshot = { 0 };
	ThemePaletteCandidate imported = { 0 };
	ThemePaletteTransaction transaction = { 0 };
	const ThemePaletteCandidate *saved_snapshot;
	const ThemePaletteCandidate *staged;
	GError *error = NULL;
	GdkRGBA original;
	GdkRGBA current;

	setup_temp_home ();
	theme_runtime_load ();
	theme_runtime_palette_snapshot (FALSE, &snapshot);
	g_assert_true (theme_runtime_palette_candidate_from_colors (contents,
		FALSE, &imported, &error));
	g_assert_no_error (error);
	g_assert_true (theme_palette_transaction_begin (&transaction,
		ZOITECHAT_DARK_MODE_LIGHT, &snapshot));
	g_assert_true (theme_palette_transaction_replace (&transaction,
		&imported));
	g_assert_true (transaction.changed);

	saved_snapshot = theme_palette_transaction_snapshot (&transaction);
	staged = theme_palette_transaction_staged (&transaction);
	g_assert_nonnull (saved_snapshot);
	g_assert_nonnull (staged);
	g_assert_true (theme_palette_get_color (&saved_snapshot->palette,
		THEME_TOKEN_MIRC_1, &original));
	g_assert_true (theme_palette_get_color (&staged->palette,
		THEME_TOKEN_MIRC_1, &current));
	g_assert_false (colors_equal (&original, &current));
}

static void
test_palette_transaction_tracks_custom_ownership (void)
{
	ThemePaletteCandidate snapshot = { 0 };
	ThemePaletteCandidate candidate;
	ThemePaletteTransaction transaction = { 0 };

	setup_temp_home ();
	theme_runtime_load ();
	theme_runtime_palette_snapshot (FALSE, &snapshot);
	candidate = snapshot;
	candidate.custom_tokens[THEME_TOKEN_MIRC_2] =
		!snapshot.custom_tokens[THEME_TOKEN_MIRC_2];

	g_assert_true (theme_palette_transaction_begin (&transaction,
		ZOITECHAT_DARK_MODE_LIGHT, &snapshot));
	g_assert_true (theme_palette_transaction_replace (&transaction,
		&candidate));
	g_assert_true (transaction.changed);
}

static void
test_palette_transaction_rejects_mode_mismatch (void)
{
	ThemePaletteCandidate light = { 0 };
	ThemePaletteCandidate dark = { 0 };
	ThemePaletteTransaction transaction = { 0 };

	setup_temp_home ();
	theme_runtime_load ();
	theme_runtime_palette_snapshot (FALSE, &light);
	theme_runtime_palette_snapshot (TRUE, &dark);
	g_assert_true (theme_palette_transaction_begin (&transaction,
		ZOITECHAT_DARK_MODE_LIGHT, &light));
	g_assert_false (theme_palette_transaction_replace (&transaction,
		&dark));
	g_assert_false (transaction.changed);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/theme/runtime/persistence_roundtrip_light_and_dark",
			 test_persistence_roundtrip_light_and_dark);
	g_test_add_func ("/theme/runtime/loads_legacy_color_keys_via_migration_loader",
			 test_loads_legacy_color_keys_via_migration_loader);
	g_test_add_func ("/theme/runtime/ui_edits_persist_without_legacy_array_mutation",
			 test_ui_edits_persist_without_legacy_array_mutation);
	g_test_add_func ("/theme/runtime/apply_mode_resolves_auto_from_system_preference",
			 test_apply_mode_resolves_auto_from_system_preference);
	g_test_add_func ("/theme/runtime/apply_mode_respects_explicit_dark_and_light_modes",
			 test_apply_mode_respects_explicit_dark_and_light_modes);
	g_test_add_func ("/theme/runtime/gtk_map_colors_blend_with_palette_without_transparency",
			 test_gtk_map_colors_blend_with_palette_without_transparency);
	g_test_add_func ("/theme/runtime/gtk_map_uses_theme_defaults_until_custom_token_is_set",
			 test_gtk_map_uses_theme_defaults_until_custom_token_is_set);
	g_test_add_func ("/theme/runtime/save_finalize_replaces_colors_conf_atomically",
			 test_save_finalize_replaces_colors_conf_atomically);
	g_test_add_func ("/theme/runtime/save_discard_unlinks_temp_file",
			 test_save_discard_unlinks_temp_file);
	g_test_add_func ("/theme/runtime/save_writes_only_custom_token_keys",
			 test_save_writes_only_custom_token_keys);
	g_test_add_func ("/theme/runtime/palette_candidate_parse_is_nonmutating",
			 test_palette_candidate_parse_does_not_mutate_runtime);
	g_test_add_func ("/theme/runtime/palette_candidate_rejects_malformed",
			 test_palette_candidate_rejects_malformed_without_mutation);
	g_test_add_func ("/theme/runtime/palette_candidate_apply_and_restore",
			 test_palette_candidate_applies_and_restores_as_complete_palette);
	g_test_add_func ("/theme/runtime/palette_candidate_rejects_empty",
			 test_palette_candidate_rejects_empty_palette);
	g_test_add_func ("/theme/runtime/palette_transaction_edit_and_revert",
			 test_palette_transaction_tracks_edit_and_revert);
	g_test_add_func ("/theme/runtime/palette_transaction_replace",
			 test_palette_transaction_replaces_candidate_and_keeps_snapshot);
	g_test_add_func ("/theme/runtime/palette_transaction_custom_ownership",
			 test_palette_transaction_tracks_custom_ownership);
	g_test_add_func ("/theme/runtime/palette_transaction_mode_mismatch",
			 test_palette_transaction_rejects_mode_mismatch);
	return g_test_run ();
}

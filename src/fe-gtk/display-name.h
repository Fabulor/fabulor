#ifndef FABULOR_DISPLAY_NAME_H
#define FABULOR_DISPLAY_NAME_H

#include <glib.h>

#define FABULOR_NAME_ELLIPSIS "\342\200\246"

/* Return the UTF-8 boundary where the name needs shortening. */
static inline const char *
fabulor_display_name_cut (const char *name, int limit)
{
	if (limit <= 0 || !g_utf8_validate (name, -1, NULL) ||
		g_utf8_strlen (name, -1) <= limit)
		return NULL;

	return g_utf8_offset_to_pointer (name, limit);
}

static inline char *
fabulor_display_name_shorten (const char *name, int limit)
{
	const char *cut = fabulor_display_name_cut (name, limit);

	return cut ? g_strdup_printf ("%.*s%s", (int) (cut - name),
		name, FABULOR_NAME_ELLIPSIS) : NULL;
}

#endif

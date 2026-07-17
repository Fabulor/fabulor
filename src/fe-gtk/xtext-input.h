/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_INPUT_H
#define FABULOR_XTEXT_INPUT_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct
{
	guint button;
	guint n_press;
	gdouble x;
	gdouble y;
	GdkModifierType state;
} FabulorXTextClick;

typedef enum
{
	FABULOR_XTEXT_SELECTION_PRESS_NONE,
	FABULOR_XTEXT_SELECTION_PRESS_SINGLE,
	FABULOR_XTEXT_SELECTION_PRESS_WORD,
	FABULOR_XTEXT_SELECTION_PRESS_LINE
} FabulorXTextSelectionPress;

FabulorXTextSelectionPress fabulor_xtext_selection_press (guint button,
	guint n_press);
gint fabulor_xtext_scroll_direction (gdouble dy);

G_END_DECLS

#endif

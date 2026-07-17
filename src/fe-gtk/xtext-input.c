/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-input.h"

FabulorXTextSelectionPress
fabulor_xtext_selection_press (guint button, guint n_press)
{
	if (button != 1)
		return FABULOR_XTEXT_SELECTION_PRESS_NONE;
	if (n_press == 2)
		return FABULOR_XTEXT_SELECTION_PRESS_WORD;
	if (n_press >= 3)
		return FABULOR_XTEXT_SELECTION_PRESS_LINE;
	return FABULOR_XTEXT_SELECTION_PRESS_SINGLE;
}

gint
fabulor_xtext_scroll_direction (gdouble dy)
{
	return dy < 0.0 ? -1 : dy > 0.0 ? 1 : 0;
}

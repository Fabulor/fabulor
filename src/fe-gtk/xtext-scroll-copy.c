/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-scroll-copy.h"

gboolean
fabulor_xtext_scroll_copy_plan (gint overlap, gint height,
	gint font_size, gint descent, gboolean native_capture,
	FabulorXTextScrollCopy *plan)
{
	gint remainder;

	g_return_val_if_fail (plan != NULL, FALSE);
	*plan = (FabulorXTextScrollCopy) { 0, 0, 0, 0, 0 };

	if (!native_capture || height < font_size || font_size <= 0 ||
		descent < 0 || descent >= font_size || overlap == 0 ||
		overlap <= -height || overlap >= height)
		return FALSE;

	if (overlap < 0)
	{
		plan->source_y = -overlap;
		plan->copy_height = height + overlap;
		remainder = ((height - descent) % font_size) + descent;
		plan->damage_y = plan->copy_height - remainder;
		plan->damage_height = remainder - overlap;
	}
	else
	{
		plan->destination_y = overlap;
		plan->copy_height = height - overlap;
		plan->damage_height = overlap;
	}

	return plan->copy_height > 0 && plan->damage_height > 0;
}

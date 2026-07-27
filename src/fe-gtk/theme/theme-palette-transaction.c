/* Fabulor
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <string.h>

#include "theme-palette-transaction.h"

static void
theme_palette_transaction_recompute (ThemePaletteTransaction *transaction)
{
	size_t i;

	transaction->changed = FALSE;
	for (i = 0; i < theme_palette_token_def_count (); i++)
	{
		const ThemePaletteTokenDef *def = theme_palette_token_def_at (i);
		GdkRGBA snapshot_color;
		GdkRGBA staged_color;

		g_assert (def != NULL);
		g_assert (theme_palette_get_color (&transaction->snapshot.palette,
			def->token, &snapshot_color));
		g_assert (theme_palette_get_color (&transaction->staged.palette,
			def->token, &staged_color));
		if (!gdk_rgba_equal (&snapshot_color, &staged_color)
			|| transaction->snapshot.custom_tokens[def->token]
				!= transaction->staged.custom_tokens[def->token])
		{
			transaction->changed = TRUE;
			return;
		}
	}
}

void
theme_palette_transaction_reset (ThemePaletteTransaction *transaction)
{
	g_return_if_fail (transaction != NULL);
	memset (transaction, 0, sizeof (*transaction));
}

gboolean
theme_palette_transaction_begin (ThemePaletteTransaction *transaction,
	unsigned int mode, const ThemePaletteCandidate *snapshot)
{
	g_return_val_if_fail (transaction != NULL, FALSE);
	g_return_val_if_fail (snapshot != NULL, FALSE);
	if (!snapshot->initialized)
		return FALSE;
	theme_palette_transaction_reset (transaction);
	transaction->active = TRUE;
	transaction->mode = mode;
	transaction->snapshot = *snapshot;
	transaction->staged = *snapshot;
	return TRUE;
}

gboolean
theme_palette_transaction_set_color (ThemePaletteTransaction *transaction,
	ThemeSemanticToken token, const GdkRGBA *color)
{
	GdkRGBA snapshot_color;

	g_return_val_if_fail (transaction != NULL, FALSE);
	g_return_val_if_fail (color != NULL, FALSE);
	if (!transaction->active
		|| !theme_palette_set_color (&transaction->staged.palette,
			token, color))
		return FALSE;
	g_assert (theme_palette_get_color (&transaction->snapshot.palette,
		token, &snapshot_color));
	transaction->staged.custom_tokens[token] =
		gdk_rgba_equal (&snapshot_color, color)
			? transaction->snapshot.custom_tokens[token]
			: TRUE;
	theme_palette_transaction_recompute (transaction);
	return TRUE;
}

gboolean
theme_palette_transaction_replace (ThemePaletteTransaction *transaction,
	const ThemePaletteCandidate *candidate)
{
	g_return_val_if_fail (transaction != NULL, FALSE);
	g_return_val_if_fail (candidate != NULL, FALSE);
	if (!transaction->active || !candidate->initialized
		|| candidate->dark_mode != transaction->snapshot.dark_mode)
		return FALSE;
	transaction->staged = *candidate;
	theme_palette_transaction_recompute (transaction);
	return TRUE;
}

gboolean
theme_palette_transaction_get_color (
	const ThemePaletteTransaction *transaction, ThemeSemanticToken token,
	GdkRGBA *color)
{
	g_return_val_if_fail (transaction != NULL, FALSE);
	g_return_val_if_fail (color != NULL, FALSE);
	if (!transaction->active)
		return FALSE;
	return theme_palette_get_color (&transaction->staged.palette,
		token, color);
}

const ThemePaletteCandidate *
theme_palette_transaction_snapshot (
	const ThemePaletteTransaction *transaction)
{
	if (!transaction || !transaction->active)
		return NULL;
	return &transaction->snapshot;
}

const ThemePaletteCandidate *
theme_palette_transaction_staged (const ThemePaletteTransaction *transaction)
{
	if (!transaction || !transaction->active)
		return NULL;
	return &transaction->staged;
}

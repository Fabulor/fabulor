/* Fabulor
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_THEME_PALETTE_TRANSACTION_H
#define FABULOR_THEME_PALETTE_TRANSACTION_H

#include "theme-runtime.h"

typedef struct
{
	gboolean active;
	gboolean changed;
	unsigned int mode;
	ThemePaletteCandidate snapshot;
	ThemePaletteCandidate staged;
} ThemePaletteTransaction;

void theme_palette_transaction_reset (ThemePaletteTransaction *transaction);
gboolean theme_palette_transaction_begin (ThemePaletteTransaction *transaction,
	unsigned int mode, const ThemePaletteCandidate *snapshot);
gboolean theme_palette_transaction_set_color (
	ThemePaletteTransaction *transaction, ThemeSemanticToken token,
	const GdkRGBA *color);
gboolean theme_palette_transaction_replace (
	ThemePaletteTransaction *transaction,
	const ThemePaletteCandidate *candidate);
gboolean theme_palette_transaction_get_color (
	const ThemePaletteTransaction *transaction, ThemeSemanticToken token,
	GdkRGBA *color);
const ThemePaletteCandidate *theme_palette_transaction_snapshot (
	const ThemePaletteTransaction *transaction);
const ThemePaletteCandidate *theme_palette_transaction_staged (
	const ThemePaletteTransaction *transaction);

#endif

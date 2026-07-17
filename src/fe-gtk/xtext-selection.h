/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_SELECTION_H
#define FABULOR_XTEXT_SELECTION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorXTextSelection FabulorXTextSelection;

typedef gchar *(*FabulorXTextSelectionTextFunc) (GtkWidget *widget,
	gint *length, gpointer user_data);
typedef void (*FabulorXTextSelectionClearFunc) (GtkWidget *widget,
	gpointer user_data);

FabulorXTextSelection *fabulor_xtext_selection_new (GtkWidget *widget,
	FabulorXTextSelectionTextFunc text_func,
	FabulorXTextSelectionClearFunc clear_func, gpointer user_data);
void fabulor_xtext_selection_free (FabulorXTextSelection *selection);
void fabulor_xtext_selection_publish (FabulorXTextSelection *selection,
	const gchar *text, gint length, guint32 event_time);
gchar *fabulor_xtext_selection_copy_text (const gchar *text, gssize length);

G_END_DECLS

#endif

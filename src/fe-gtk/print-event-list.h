/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_PRINT_EVENT_LIST_H
#define FABULOR_PRINT_EVENT_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorPrintEventList FabulorPrintEventList;

typedef gboolean (*FabulorPrintEventEditFunc) (gint signal_index,
	const gchar *new_text, gpointer user_data);
typedef void (*FabulorPrintEventSelectionFunc) (gint signal_index,
	gpointer user_data);

FabulorPrintEventList *fabulor_print_event_list_new (
	FabulorPrintEventEditFunc edit_func,
	FabulorPrintEventSelectionFunc selection_func, gpointer user_data);
void fabulor_print_event_list_free (FabulorPrintEventList *list);
gboolean fabulor_print_event_list_create_views (FabulorPrintEventList *list,
	GtkPaned *parent, const gchar *event_title, const gchar *text_title,
	const gchar *number_title, const gchar *description_title);
void fabulor_print_event_list_append_event (FabulorPrintEventList *list,
	const gchar *event_name, const gchar *text, gint signal_index);
void fabulor_print_event_list_clear_events (FabulorPrintEventList *list);
void fabulor_print_event_list_append_help (FabulorPrintEventList *list,
	gint number, const gchar *description);
void fabulor_print_event_list_clear_help (FabulorPrintEventList *list);
guint fabulor_print_event_list_get_n_events (FabulorPrintEventList *list);
guint fabulor_print_event_list_get_n_help (FabulorPrintEventList *list);
gboolean fabulor_print_event_list_edit_at (FabulorPrintEventList *list,
	guint position, const gchar *text);
gboolean fabulor_print_event_list_select_at (FabulorPrintEventList *list,
	guint position);
gchar *fabulor_print_event_list_dup_text_at (FabulorPrintEventList *list,
	guint position);
gint fabulor_print_event_list_get_signal_at (FabulorPrintEventList *list,
	guint position);

G_END_DECLS

#endif

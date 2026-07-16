/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_SOUND_EVENT_LIST_H
#define FABULOR_SOUND_EVENT_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorSoundEventList FabulorSoundEventList;

typedef void (*FabulorSoundEventSelectionFunc) (gint event_index,
	gpointer user_data);

FabulorSoundEventList *fabulor_sound_event_list_new (
	FabulorSoundEventSelectionFunc selection_func, gpointer user_data);
void fabulor_sound_event_list_free (FabulorSoundEventList *list);
GtkWidget *fabulor_sound_event_list_create_view (FabulorSoundEventList *list,
	GtkBox *parent, const gchar *event_title, const gchar *file_title);
void fabulor_sound_event_list_append (FabulorSoundEventList *list,
	const gchar *event_name, const gchar *sound_file, gint event_index);
void fabulor_sound_event_list_clear (FabulorSoundEventList *list);
gboolean fabulor_sound_event_list_update_file (FabulorSoundEventList *list,
	gint event_index, const gchar *sound_file);
gboolean fabulor_sound_event_list_select_event (FabulorSoundEventList *list,
	gint event_index);
gint fabulor_sound_event_list_get_selected_event (FabulorSoundEventList *list);
guint fabulor_sound_event_list_get_n_rows (FabulorSoundEventList *list);
gchar *fabulor_sound_event_list_dup_file (FabulorSoundEventList *list,
	gint event_index);

G_END_DECLS

#endif

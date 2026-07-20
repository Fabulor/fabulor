/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_WINDOW_STATE_H
#define FABULOR_WINDOW_STATE_H

#include <gtk/gtk.h>

typedef enum
{
	FABULOR_WINDOW_STATE_MINIMIZED = 1 << 0,
	FABULOR_WINDOW_STATE_MAXIMIZED = 1 << 1,
	FABULOR_WINDOW_STATE_FULLSCREEN = 1 << 2,
	FABULOR_WINDOW_STATE_FOCUSED = 1 << 3
} FabulorWindowStateFlag;

typedef struct
{
	guint changed;
	gboolean minimized;
	gboolean maximized;
	gboolean fullscreen;
	gboolean focused;
} FabulorWindowState;

typedef void (*FabulorWindowStateCallback) (GtkWindow *window,
	const FabulorWindowState *state, gpointer user_data);

guint fabulor_window_state_changes (const FabulorWindowState *previous,
	const FabulorWindowState *current);
void fabulor_window_state_get (GtkWindow *window, FabulorWindowState *state);
void fabulor_window_state_watch (GtkWindow *window,
	FabulorWindowStateCallback callback, gpointer user_data);
gpointer fabulor_window_native_handle (GtkWindow *window);
void fabulor_window_state_allow_autohide_taskbar (GtkWindow *window,
	const FabulorWindowState *state);

#endif

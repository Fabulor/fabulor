/* Copyright (C) 2026 Fabulor contributors */
#include <string.h>

#include "window-state.h"

#ifdef G_OS_WIN32
#include <windows.h>
#include <shellapi.h>
#include <gdk/win32/gdkwin32.h>
#endif

#define FABULOR_WINDOW_NATIVE_HIDDEN_DATA "fabulor-window-native-hidden"

typedef struct
{
	GtkWindow *window;
	FabulorWindowStateCallback callback;
	gpointer user_data;
	FabulorWindowState previous;
	GdkToplevel *toplevel;
	gulong state_handler;
} FabulorWindowStateWatch;

guint
fabulor_window_state_changes (const FabulorWindowState *previous,
	const FabulorWindowState *current)
{
	guint changed = 0;
	g_return_val_if_fail (previous != NULL, 0);
	g_return_val_if_fail (current != NULL, 0);
	if (previous->minimized != current->minimized)
		changed |= FABULOR_WINDOW_STATE_MINIMIZED;
	if (previous->maximized != current->maximized)
		changed |= FABULOR_WINDOW_STATE_MAXIMIZED;
	if (previous->fullscreen != current->fullscreen)
		changed |= FABULOR_WINDOW_STATE_FULLSCREEN;
	if (previous->focused != current->focused)
		changed |= FABULOR_WINDOW_STATE_FOCUSED;
	if (previous->visible != current->visible)
		changed |= FABULOR_WINDOW_STATE_VISIBLE;
	return changed;
}

void
fabulor_window_state_get (GtkWindow *window, FabulorWindowState *state)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (state != NULL);

	memset (state, 0, sizeof (*state));
	state->visible = gtk_widget_get_visible (GTK_WIDGET (window)) &&
		g_object_get_data (G_OBJECT (window),
			FABULOR_WINDOW_NATIVE_HIDDEN_DATA) == NULL;
	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
		if (GDK_IS_TOPLEVEL (surface))
		{
			GdkToplevelState value = gdk_toplevel_get_state (
				GDK_TOPLEVEL (surface));
			state->minimized = (value & GDK_TOPLEVEL_STATE_MINIMIZED) != 0;
			state->maximized = (value & GDK_TOPLEVEL_STATE_MAXIMIZED) != 0;
			state->fullscreen = (value & GDK_TOPLEVEL_STATE_FULLSCREEN) != 0;
			state->focused = (value & GDK_TOPLEVEL_STATE_FOCUSED) != 0;
		}
	}
}

static void
window_state_emit (FabulorWindowStateWatch *watch)
{
	FabulorWindowState current;
	fabulor_window_state_get (watch->window, &current);
	current.changed = fabulor_window_state_changes (&watch->previous, &current);
	watch->previous = current;
	watch->previous.changed = 0;
	if (current.changed != 0)
		watch->callback (watch->window, &current, watch->user_data);
}

static void
window_state_visible_notify_cb (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	(void)object;
	(void)pspec;
	window_state_emit (user_data);
}

static void
window_state_notify_cb (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	(void)object;
	(void)pspec;
	window_state_emit (user_data);
}
static void
window_state_detach_surface (FabulorWindowStateWatch *watch)
{
	if (!watch->toplevel)
		return;
	if (watch->state_handler)
		g_signal_handler_disconnect (watch->toplevel, watch->state_handler);
	watch->state_handler = 0;
	g_clear_object (&watch->toplevel);
}

static void
window_state_attach_surface (FabulorWindowStateWatch *watch)
{
	GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (watch->window));
	window_state_detach_surface (watch);
	if (!GDK_IS_TOPLEVEL (surface))
		return;
	watch->toplevel = g_object_ref (GDK_TOPLEVEL (surface));
	watch->state_handler = g_signal_connect (watch->toplevel, "notify::state",
		G_CALLBACK (window_state_notify_cb), watch);
	window_state_emit (watch);
}

static void
window_state_realize_cb (GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	window_state_attach_surface (user_data);
}

static void
window_state_unrealize_cb (GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	window_state_detach_surface (user_data);
}

static void
window_state_watch_free (gpointer data)
{
	FabulorWindowStateWatch *watch = data;
	window_state_detach_surface (watch);
	g_free (watch);
}

void
fabulor_window_state_watch (GtkWindow *window,
	FabulorWindowStateCallback callback, gpointer user_data)
{
	GPtrArray *watches;
	FabulorWindowStateWatch *watch;

	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (callback != NULL);
	watches = g_object_get_data (G_OBJECT (window), "fabulor-window-state-watches");
	if (!watches)
	{
		watches = g_ptr_array_new_with_free_func (window_state_watch_free);
		g_object_set_data_full (G_OBJECT (window), "fabulor-window-state-watches",
			watches, (GDestroyNotify)g_ptr_array_unref);
	}
	watch = g_new0 (FabulorWindowStateWatch, 1);
	watch->window = window;
	watch->callback = callback;
	watch->user_data = user_data;
	fabulor_window_state_get (window, &watch->previous);
	g_ptr_array_add (watches, watch);
	g_signal_connect (window, "notify::visible",
		G_CALLBACK (window_state_visible_notify_cb), watch);
	g_signal_connect (window, "realize", G_CALLBACK (window_state_realize_cb), watch);
	g_signal_connect (window, "unrealize", G_CALLBACK (window_state_unrealize_cb), watch);
	if (gtk_widget_get_realized (GTK_WIDGET (window)))
		window_state_attach_surface (watch);
}

static void
window_state_emit_all (GtkWindow *window)
{
	GPtrArray *watches;
	guint index;

	watches = g_object_get_data (G_OBJECT (window),
		"fabulor-window-state-watches");
	if (!watches)
		return;
	for (index = 0; index < watches->len; index++)
		window_state_emit (g_ptr_array_index (watches, index));
}

void
fabulor_window_hide (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
#ifdef G_OS_WIN32
	{
		HWND hwnd = (HWND) fabulor_window_native_handle (window);
		if (hwnd)
		{
			g_object_set_data (G_OBJECT (window),
				FABULOR_WINDOW_NATIVE_HIDDEN_DATA, GINT_TO_POINTER (1));
			ShowWindow (hwnd, SW_HIDE);
			window_state_emit_all (window);
			return;
		}
	}
#endif
	gtk_widget_set_visible (GTK_WIDGET (window), FALSE);
}

void
fabulor_window_present (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
#ifdef G_OS_WIN32
	if (g_object_get_data (G_OBJECT (window),
		FABULOR_WINDOW_NATIVE_HIDDEN_DATA) != NULL)
	{
		HWND hwnd = (HWND) fabulor_window_native_handle (window);
		g_object_set_data (G_OBJECT (window),
			FABULOR_WINDOW_NATIVE_HIDDEN_DATA, NULL);
		if (hwnd)
			ShowWindow (hwnd, SW_RESTORE);
		gtk_window_present (window);
		if (hwnd)
			SetForegroundWindow (hwnd);
		window_state_emit_all (window);
		return;
	}
#endif
	gtk_widget_set_visible (GTK_WIDGET (window), TRUE);
	gtk_window_present (window);
}

gpointer
fabulor_window_native_handle (GtkWindow *window)
{
#ifdef G_OS_WIN32
	if (!GTK_IS_WINDOW (window))
		return NULL;
	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
		return GDK_IS_WIN32_SURFACE (surface) ?
			(gpointer) gdk_win32_surface_get_handle (surface) : NULL;
	}
#else
	(void)window;
	return NULL;
#endif
}

void
fabulor_window_state_allow_autohide_taskbar (GtkWindow *window,
	const FabulorWindowState *state)
{
#ifdef G_OS_WIN32
	HWND hwnd;
	if (!GTK_IS_WINDOW (window) || !state || state->fullscreen)
		return;
	hwnd = (HWND) fabulor_window_native_handle (window);
	if (!hwnd)
		return;
	if (state->maximized)
	{
		APPBARDATA appbar_data;
		RECT work_area;
		ZeroMemory (&appbar_data, sizeof (appbar_data));
		appbar_data.cbSize = sizeof (appbar_data);
		if ((SHAppBarMessage (ABM_GETSTATE, &appbar_data) & ABS_AUTOHIDE) != 0 &&
			SHAppBarMessage (ABM_GETTASKBARPOS, &appbar_data) != 0)
		{
			HMONITOR monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitor_info;
			ZeroMemory (&monitor_info, sizeof (monitor_info));
			monitor_info.cbSize = sizeof (monitor_info);
			if (monitor && GetMonitorInfo (monitor, &monitor_info))
			{
				work_area = monitor_info.rcMonitor;
				switch (appbar_data.uEdge)
				{
				case ABE_LEFT: work_area.left += 1; break;
				case ABE_TOP: work_area.top += 1; break;
				case ABE_RIGHT: work_area.right -= 1; break;
				case ABE_BOTTOM:
				default: work_area.bottom -= 1; break;
				}
				SetWindowPos (hwnd, NULL, work_area.left, work_area.top,
					work_area.right - work_area.left,
					work_area.bottom - work_area.top,
					SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			}
		}
	}
	SetWindowPos (hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
#else
	(void)window;
	(void)state;
#endif
}

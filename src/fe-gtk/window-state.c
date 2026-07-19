/* Copyright (C) 2026 Fabulor contributors */
#include <string.h>

#include "window-state.h"

#ifdef G_OS_WIN32
#include <windows.h>
#include <shellapi.h>
#if GTK_MAJOR_VERSION >= 4
#include <gdk/win32/gdkwin32.h>
#else
#include <gdk/gdkwin32.h>
#endif
#endif

typedef struct
{
	GtkWindow *window;
	FabulorWindowStateCallback callback;
	gpointer user_data;
	FabulorWindowState previous;
#if GTK_MAJOR_VERSION >= 4
	GdkToplevel *toplevel;
	gulong state_handler;
#endif
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
	return changed;
}

void
fabulor_window_state_get (GtkWindow *window, FabulorWindowState *state)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (state != NULL);

	memset (state, 0, sizeof (*state));
#if GTK_MAJOR_VERSION >= 4
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
#else
	{
		GdkWindow *gdk_window = gtk_widget_get_window (GTK_WIDGET (window));
		if (gdk_window)
		{
			GdkWindowState value = gdk_window_get_state (gdk_window);
			state->minimized = (value & GDK_WINDOW_STATE_ICONIFIED) != 0;
			state->maximized = (value & GDK_WINDOW_STATE_MAXIMIZED) != 0;
			state->fullscreen = (value & GDK_WINDOW_STATE_FULLSCREEN) != 0;
			state->focused = (value & GDK_WINDOW_STATE_FOCUSED) != 0;
		}
	}
#endif
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

#if GTK_MAJOR_VERSION >= 4
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
#else
static gboolean
window_state_event_cb (GtkWidget *widget, GdkEventWindowState *event,
	gpointer user_data)
{
	FabulorWindowStateWatch *watch = user_data;
	FabulorWindowState current = { 0 };
	(void)widget;
	if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
		current.changed |= FABULOR_WINDOW_STATE_MINIMIZED;
	if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
		current.changed |= FABULOR_WINDOW_STATE_MAXIMIZED;
	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
		current.changed |= FABULOR_WINDOW_STATE_FULLSCREEN;
	if (event->changed_mask & GDK_WINDOW_STATE_FOCUSED)
		current.changed |= FABULOR_WINDOW_STATE_FOCUSED;
	current.minimized = (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) != 0;
	current.maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
	current.fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
	current.focused = (event->new_window_state & GDK_WINDOW_STATE_FOCUSED) != 0;
	watch->previous = current;
	watch->previous.changed = 0;
	watch->callback (watch->window, &current, watch->user_data);
	return FALSE;
}
#endif

static void
window_state_watch_free (gpointer data)
{
	FabulorWindowStateWatch *watch = data;
#if GTK_MAJOR_VERSION >= 4
	window_state_detach_surface (watch);
#endif
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
#if GTK_MAJOR_VERSION >= 4
	g_signal_connect (window, "realize", G_CALLBACK (window_state_realize_cb), watch);
	g_signal_connect (window, "unrealize", G_CALLBACK (window_state_unrealize_cb), watch);
	if (gtk_widget_get_realized (GTK_WIDGET (window)))
		window_state_attach_surface (watch);
#else
	g_signal_connect (window, "window-state-event",
		G_CALLBACK (window_state_event_cb), watch);
#endif
}

void
fabulor_window_state_allow_autohide_taskbar (GtkWindow *window,
	const FabulorWindowState *state)
{
#ifdef G_OS_WIN32
	HWND hwnd = NULL;
	if (!GTK_IS_WINDOW (window) || !state || state->fullscreen)
		return;
#if GTK_MAJOR_VERSION >= 4
	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
		if (GDK_IS_WIN32_SURFACE (surface))
			hwnd = gdk_win32_surface_get_handle (surface);
	}
#else
	{
		GdkWindow *gdk_window = gtk_widget_get_window (GTK_WIDGET (window));
		if (gdk_window)
			hwnd = gdk_win32_window_get_handle (gdk_window);
	}
#endif
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

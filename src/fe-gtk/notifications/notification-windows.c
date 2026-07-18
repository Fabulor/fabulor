/* ZoiteChat
 * Copyright (C) 2015 Arnav Singh.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "notification-backend.h"

#include <gmodule.h>
#include <glib/gwin32.h>
#include <windows.h>

typedef void (*NotificationShowFunc) (const char *title, const char *text);
typedef int (*NotificationInitFunc) (const char **error);
typedef void (*NotificationDeinitFunc) (void);
typedef int (*NotificationSupportedFunc) (void);

static GModule *winrt_module;
static NotificationShowFunc winrt_notification_backend_show;
static NotificationInitFunc winrt_notification_backend_init;
static NotificationDeinitFunc winrt_notification_backend_deinit;
static NotificationSupportedFunc winrt_notification_backend_supported;

static GQuark
notification_backend_error_quark (void)
{
	return g_quark_from_static_string ("fabulor-notification-backend-error");
}

static void
notification_backend_clear_exports (void)
{
	winrt_notification_backend_show = NULL;
	winrt_notification_backend_init = NULL;
	winrt_notification_backend_deinit = NULL;
	winrt_notification_backend_supported = NULL;
}

static gboolean
notification_backend_resolve_exports (GError **error)
{
#define RESOLVE_EXPORT(name) \
	if (!g_module_symbol (winrt_module, #name, (gpointer *) &winrt_##name)) \
	{ \
		g_set_error (error, notification_backend_error_quark (), 2, \
		             "hcnotifications-winrt.dll is missing %s: %s", #name, g_module_error ()); \
		return FALSE; \
	}

	RESOLVE_EXPORT (notification_backend_show);
	RESOLVE_EXPORT (notification_backend_init);
	RESOLVE_EXPORT (notification_backend_deinit);
	RESOLVE_EXPORT (notification_backend_supported);

#undef RESOLVE_EXPORT
	return TRUE;
}

void
notification_backend_show (const char *title, const char *text)
{
	if (winrt_notification_backend_show == NULL)
	{
		return;
	}

	winrt_notification_backend_show (title, text);
}

gboolean
notification_backend_init (GError **error)
{
	char *install_root;
	char *module_path;
	const char *helper_error = NULL;
	DWORD original_error_mode = 0;
	BOOL error_mode_changed;

	if (winrt_module)
		return TRUE;

	install_root = g_win32_get_package_installation_directory_of_module (NULL);
	if (!install_root)
	{
		g_set_error_literal (error, notification_backend_error_quark (), 1,
		                     "Could not resolve the Fabulor installation directory.");
		return FALSE;
	}

	module_path = g_build_filename (install_root, "plugins", "hcnotifications-winrt.dll", NULL);
	g_free (install_root);

	/* Keep a missing dependency from opening a system dialog on this thread. */
	error_mode_changed = SetThreadErrorMode (SEM_FAILCRITICALERRORS, &original_error_mode);
	winrt_module = g_module_open (module_path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (error_mode_changed)
		SetThreadErrorMode (original_error_mode, NULL);

	if (!winrt_module)
	{
		g_set_error (error, notification_backend_error_quark (), 1,
		             "Could not load %s: %s", module_path, g_module_error ());
		g_free (module_path);
		return FALSE;
	}
	g_free (module_path);

	if (!notification_backend_resolve_exports (error))
		goto fail;

	if (!winrt_notification_backend_init (&helper_error))
	{
		g_set_error (error, notification_backend_error_quark (), 3,
		             "Could not initialize hcnotifications-winrt.dll: %s",
		             helper_error ? helper_error : "unknown error");
		goto fail;
	}

	return TRUE;

fail:
	notification_backend_clear_exports ();
	g_module_close (winrt_module);
	winrt_module = NULL;
	return FALSE;
}

void
notification_backend_deinit (void)
{
	GModule *module;

	if (!winrt_module)
		return;

	if (winrt_notification_backend_deinit)
		winrt_notification_backend_deinit ();

	module = winrt_module;
	winrt_module = NULL;
	notification_backend_clear_exports ();
	g_module_close (module);
}

gboolean
notification_backend_supported (void)
{
	return winrt_notification_backend_supported &&
	       winrt_notification_backend_supported ();
}

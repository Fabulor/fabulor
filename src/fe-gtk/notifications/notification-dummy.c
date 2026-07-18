/* ZoiteChat
 * Copyright (C) 2015 Patrick Griffis.
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

#include "notification-backend.h"

void
notification_backend_show (const char *title, const char *text)
{
	(void)title;
	(void)text;
}

gboolean
notification_backend_init (GError **error)
{
	g_set_error_literal (error,
	                     g_quark_from_static_string ("fabulor-notification-backend-error"),
	                     1,
	                     "Desktop notifications are unavailable on this platform.");
	return FALSE;
}

void
notification_backend_deinit (void)
{
}

gboolean
notification_backend_supported (void)
{
	return FALSE;
}

/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_TRAY_BACKEND_POLICY_H
#define FABULOR_TRAY_BACKEND_POLICY_H

#include <glib.h>

typedef enum
{
	FABULOR_TRAY_BACKEND_DISABLED,
	FABULOR_TRAY_BACKEND_WINDOWS_SHELL,
	FABULOR_TRAY_BACKEND_STATUS_NOTIFIER,
	FABULOR_TRAY_BACKEND_LEGACY_STATUS_ICON,
	FABULOR_TRAY_BACKEND_UNAVAILABLE
} FabulorTrayBackendKind;

typedef struct
{
	gboolean enabled;
	gboolean windows;
	guint toolkit_major;
	gboolean windows_shell_available;
	gboolean status_notifier_compiled;
	gboolean status_notifier_available;
	gboolean legacy_status_icon_available;
} FabulorTrayBackendEnvironment;

FabulorTrayBackendKind fabulor_tray_backend_select (
	const FabulorTrayBackendEnvironment *environment);
const char *fabulor_tray_backend_name (FabulorTrayBackendKind kind);
gboolean fabulor_tray_backend_is_usable (FabulorTrayBackendKind kind);

#endif

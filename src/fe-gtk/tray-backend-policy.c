/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "tray-backend-policy.h"

FabulorTrayBackendKind
fabulor_tray_backend_select (
	const FabulorTrayBackendEnvironment *environment)
{
	g_return_val_if_fail (environment != NULL,
		FABULOR_TRAY_BACKEND_UNAVAILABLE);

	if (!environment->enabled)
		return FABULOR_TRAY_BACKEND_DISABLED;
	if (environment->windows)
		return environment->windows_shell_available
			? FABULOR_TRAY_BACKEND_WINDOWS_SHELL
			: FABULOR_TRAY_BACKEND_UNAVAILABLE;
	if (environment->status_notifier_compiled &&
		environment->status_notifier_available)
		return FABULOR_TRAY_BACKEND_STATUS_NOTIFIER;
	if (environment->toolkit_major > 0 &&
		environment->toolkit_major < 4 &&
		environment->legacy_status_icon_available)
		return FABULOR_TRAY_BACKEND_LEGACY_STATUS_ICON;
	return FABULOR_TRAY_BACKEND_UNAVAILABLE;
}

const char *
fabulor_tray_backend_name (FabulorTrayBackendKind kind)
{
	switch (kind)
	{
	case FABULOR_TRAY_BACKEND_DISABLED:
		return "disabled";
	case FABULOR_TRAY_BACKEND_WINDOWS_SHELL:
		return "windows-shell";
	case FABULOR_TRAY_BACKEND_STATUS_NOTIFIER:
		return "status-notifier";
	case FABULOR_TRAY_BACKEND_LEGACY_STATUS_ICON:
		return "legacy-status-icon";
	case FABULOR_TRAY_BACKEND_UNAVAILABLE:
	default:
		return "unavailable";
	}
}

gboolean
fabulor_tray_backend_is_usable (FabulorTrayBackendKind kind)
{
	return kind == FABULOR_TRAY_BACKEND_WINDOWS_SHELL ||
		kind == FABULOR_TRAY_BACKEND_STATUS_NOTIFIER ||
		kind == FABULOR_TRAY_BACKEND_LEGACY_STATUS_ICON;
}

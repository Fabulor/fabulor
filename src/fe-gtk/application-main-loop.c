/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "application-main-loop.h"

struct _FabulorApplicationMainLoop
{
	GMainLoop *loop;
	gboolean running;
	gboolean quit_requested;
};

FabulorApplicationMainLoop *
fabulor_application_main_loop_new (void)
{
	FabulorApplicationMainLoop *owner = g_new0 (
		FabulorApplicationMainLoop, 1);

	owner->loop = g_main_loop_new (NULL, FALSE);
	return owner;
}

void
fabulor_application_main_loop_run (FabulorApplicationMainLoop *owner)
{
	g_return_if_fail (owner != NULL);
	g_return_if_fail (owner->loop != NULL);
	g_return_if_fail (!owner->running);

	if (owner->quit_requested)
		return;

	owner->running = TRUE;
	g_main_loop_run (owner->loop);
	owner->running = FALSE;
}

void
fabulor_application_main_loop_request_quit (
	FabulorApplicationMainLoop *owner)
{
	g_return_if_fail (owner != NULL);

	owner->quit_requested = TRUE;
	if (owner->running)
		g_main_loop_quit (owner->loop);
}

gboolean
fabulor_application_main_loop_is_running (
	const FabulorApplicationMainLoop *owner)
{
	return owner != NULL && owner->running;
}

void
fabulor_application_main_loop_free (FabulorApplicationMainLoop *owner)
{
	if (!owner)
		return;

	g_return_if_fail (!owner->running);
	g_clear_pointer (&owner->loop, g_main_loop_unref);
	g_free (owner);
}

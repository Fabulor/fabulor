/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_APPLICATION_MAIN_LOOP_H
#define FABULOR_APPLICATION_MAIN_LOOP_H

#include <glib.h>

typedef struct _FabulorApplicationMainLoop FabulorApplicationMainLoop;

FabulorApplicationMainLoop *fabulor_application_main_loop_new (void);
void fabulor_application_main_loop_run (FabulorApplicationMainLoop *owner);
void fabulor_application_main_loop_request_quit (
	FabulorApplicationMainLoop *owner);
gboolean fabulor_application_main_loop_is_running (
	const FabulorApplicationMainLoop *owner);
void fabulor_application_main_loop_free (FabulorApplicationMainLoop *owner);

#endif

/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_ACCESSIBLE_H
#define FABULOR_XTEXT_ACCESSIBLE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES (1024 * 1024)

typedef struct _FabulorXTextAccessible FabulorXTextAccessible;

typedef enum
{
	FABULOR_XTEXT_ACCESSIBLE_CHARACTER,
	FABULOR_XTEXT_ACCESSIBLE_WORD,
	FABULOR_XTEXT_ACCESSIBLE_SENTENCE,
	FABULOR_XTEXT_ACCESSIBLE_LINE,
	FABULOR_XTEXT_ACCESSIBLE_PARAGRAPH
} FabulorXTextAccessibleGranularity;

typedef struct
{
	guint remove_start;
	guint remove_end;
	guint insert_start;
	guint insert_end;
} FabulorXTextAccessibleChange;

typedef void (*FabulorXTextAccessibleRefreshFunc) (
	FabulorXTextAccessible *accessible, gpointer user_data);

FabulorXTextAccessible *fabulor_xtext_accessible_new (
	FabulorXTextAccessibleRefreshFunc refresh, gpointer user_data);
void fabulor_xtext_accessible_free (FabulorXTextAccessible *accessible);
gboolean fabulor_xtext_accessible_replace (FabulorXTextAccessible *accessible,
	const gchar *text, FabulorXTextAccessibleChange *change);
guint fabulor_xtext_accessible_length (FabulorXTextAccessible *accessible);
gboolean fabulor_xtext_accessible_is_observed (
	FabulorXTextAccessible *accessible);
GBytes *fabulor_xtext_accessible_contents (FabulorXTextAccessible *accessible,
	guint start, guint end);
GBytes *fabulor_xtext_accessible_contents_at (
	FabulorXTextAccessible *accessible, guint offset,
	FabulorXTextAccessibleGranularity granularity, guint *start, guint *end);
void fabulor_xtext_accessible_attach (GtkWidget *widget,
	FabulorXTextAccessible *accessible);
void fabulor_xtext_accessible_notify (GtkWidget *widget,
	const FabulorXTextAccessibleChange *change);

void fabulor_xtext_accessible_text_interface_init (
	GtkAccessibleTextInterface *iface);

G_END_DECLS

#endif

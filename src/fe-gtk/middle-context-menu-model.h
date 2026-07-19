/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_MIDDLE_CONTEXT_MENU_MODEL_H
#define FABULOR_MIDDLE_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef struct
{
	const char *label;
	const char *plugin_path;
	GMenuModel *model;
} FabulorMiddleContextSection;

typedef struct _FabulorMiddleContextMenuModel FabulorMiddleContextMenuModel;

FabulorMiddleContextMenuModel *fabulor_middle_context_menu_model_new (
	const FabulorMiddleContextSection *sections, gsize section_count,
	GMenuModel *plugin_model);
void fabulor_middle_context_menu_model_free (
	FabulorMiddleContextMenuModel *model);
GMenuModel *fabulor_middle_context_menu_model_get_menu (
	FabulorMiddleContextMenuModel *model);

#endif

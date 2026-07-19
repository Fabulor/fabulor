/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_URL_CONTEXT_MENU_MODEL_H
#define FABULOR_URL_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_URL_CONTEXT_OPEN,
	FABULOR_URL_CONTEXT_COPY
} FabulorUrlContextAction;

typedef void (*FabulorUrlContextDispatch) (FabulorUrlContextAction action,
	const char *url, gpointer user_data);

typedef struct _FabulorUrlContextMenuModel FabulorUrlContextMenuModel;

FabulorUrlContextMenuModel *fabulor_url_context_menu_model_new (
	const char *url, const char *open_label, const char *connect_label,
	const char *copy_label, GMenuModel *plugin_model,
	FabulorUrlContextDispatch dispatch, gpointer user_data);
void fabulor_url_context_menu_model_free (FabulorUrlContextMenuModel *model);
GMenuModel *fabulor_url_context_menu_model_get_menu (
	FabulorUrlContextMenuModel *model);
GActionGroup *fabulor_url_context_menu_model_get_actions (
	FabulorUrlContextMenuModel *model);

#endif

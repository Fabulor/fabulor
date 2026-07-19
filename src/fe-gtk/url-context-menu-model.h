/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_URL_CONTEXT_MENU_MODEL_H
#define FABULOR_URL_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_URL_CONTEXT_OPEN,
	FABULOR_URL_CONTEXT_COPY,
	FABULOR_URL_CONTEXT_HANDLER
} FabulorUrlContextAction;

typedef void (*FabulorUrlContextDispatch) (FabulorUrlContextAction action,
	const char *url, const char *command, gpointer user_data);

typedef enum
{
	FABULOR_URL_HANDLER_COMMAND,
	FABULOR_URL_HANDLER_TOGGLE,
	FABULOR_URL_HANDLER_SUBMENU_BEGIN,
	FABULOR_URL_HANDLER_SUBMENU_END,
	FABULOR_URL_HANDLER_SEPARATOR
} FabulorUrlHandlerKind;

typedef struct
{
	FabulorUrlHandlerKind kind;
	const char *label;
	const char *command;
	const char *icon;
	gboolean enabled;
	gboolean active;
} FabulorUrlHandler;

typedef struct _FabulorUrlContextMenuModel FabulorUrlContextMenuModel;

FabulorUrlContextMenuModel *fabulor_url_context_menu_model_new (
	const char *url, const char *open_label, const char *connect_label,
	const char *copy_label, GMenuModel *plugin_model,
	FabulorUrlContextDispatch dispatch, gpointer user_data);
FabulorUrlContextMenuModel *fabulor_url_context_menu_model_new_with_handlers (
	const char *url, const char *open_label, const char *connect_label,
	const char *copy_label, const FabulorUrlHandler *handlers,
	gsize handler_count, GMenuModel *plugin_model,
	FabulorUrlContextDispatch dispatch, gpointer user_data);
void fabulor_url_context_menu_model_free (FabulorUrlContextMenuModel *model);
GMenuModel *fabulor_url_context_menu_model_get_menu (
	FabulorUrlContextMenuModel *model);
GActionGroup *fabulor_url_context_menu_model_get_actions (
	FabulorUrlContextMenuModel *model);

#endif

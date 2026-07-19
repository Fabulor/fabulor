/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_NICK_CONTEXT_MENU_MODEL_H
#define FABULOR_NICK_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_NICK_CONTEXT_REPLY,
	FABULOR_NICK_CONTEXT_COMMAND
} FabulorNickContextAction;

typedef void (*FabulorNickContextDispatch) (FabulorNickContextAction action,
	const char *nick, const char *command, gboolean selection_dispatch,
	gpointer user_data);

typedef enum
{
	FABULOR_NICK_HANDLER_COMMAND,
	FABULOR_NICK_HANDLER_TOGGLE,
	FABULOR_NICK_HANDLER_SEPARATOR,
	FABULOR_NICK_HANDLER_SUBMENU_BEGIN,
	FABULOR_NICK_HANDLER_SUBMENU_END
} FabulorNickHandlerKind;

typedef struct
{
	FabulorNickHandlerKind kind;
	const char *label;
	const char *command;
	const char *icon;
	gboolean enabled;
	gboolean active;
} FabulorNickHandler;

typedef struct _FabulorNickContextMenuModel FabulorNickContextMenuModel;

FabulorNickContextMenuModel *fabulor_nick_context_menu_model_new (
	const char *nick, const char *heading, gboolean show_reply,
	const char *reply_label, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data);
FabulorNickContextMenuModel *fabulor_nick_context_menu_model_new_with_handlers (
	const char *nick, const char *heading, gboolean show_reply,
	const char *reply_label, gboolean selection_dispatch,
	const FabulorNickHandler *handlers, gsize handler_count,
	GMenuModel *plugin_model, FabulorNickContextDispatch dispatch,
	gpointer user_data);
void fabulor_nick_context_menu_model_free (FabulorNickContextMenuModel *model);
GMenuModel *fabulor_nick_context_menu_model_get_menu (
	FabulorNickContextMenuModel *model);
GActionGroup *fabulor_nick_context_menu_model_get_actions (
	FabulorNickContextMenuModel *model);

#endif

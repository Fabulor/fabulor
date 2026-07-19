/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_NICK_CONTEXT_MENU_MODEL_H
#define FABULOR_NICK_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_NICK_CONTEXT_REPLY
} FabulorNickContextAction;

typedef void (*FabulorNickContextDispatch) (FabulorNickContextAction action,
	const char *nick, gpointer user_data);

typedef struct _FabulorNickContextMenuModel FabulorNickContextMenuModel;

FabulorNickContextMenuModel *fabulor_nick_context_menu_model_new (
	const char *nick, const char *heading, gboolean show_reply,
	const char *reply_label, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data);
void fabulor_nick_context_menu_model_free (FabulorNickContextMenuModel *model);
GMenuModel *fabulor_nick_context_menu_model_get_menu (
	FabulorNickContextMenuModel *model);
GActionGroup *fabulor_nick_context_menu_model_get_actions (
	FabulorNickContextMenuModel *model);

#endif

/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_CHANNEL_CONTEXT_MENU_MODEL_H
#define FABULOR_CHANNEL_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_CHANNEL_CONTEXT_JOIN,
	FABULOR_CHANNEL_CONTEXT_FOCUS,
	FABULOR_CHANNEL_CONTEXT_PART,
	FABULOR_CHANNEL_CONTEXT_CYCLE,
	FABULOR_CHANNEL_CONTEXT_AUTOJOIN
} FabulorChannelContextAction;

typedef void (*FabulorChannelContextDispatch) (
	FabulorChannelContextAction action, const char *channel,
	gboolean state, gpointer user_data);

typedef struct _FabulorChannelContextMenuModel FabulorChannelContextMenuModel;

FabulorChannelContextMenuModel *fabulor_channel_context_menu_model_new (
	const char *channel, gboolean joined, gboolean current,
	gboolean has_network, gboolean autojoin, const char *join_label,
	const char *focus_label, const char *part_label, const char *cycle_label,
	const char *autojoin_label, GMenuModel *plugin_model,
	FabulorChannelContextDispatch dispatch, gpointer user_data);
void fabulor_channel_context_menu_model_free (
	FabulorChannelContextMenuModel *model);
GMenuModel *fabulor_channel_context_menu_model_get_menu (
	FabulorChannelContextMenuModel *model);
GActionGroup *fabulor_channel_context_menu_model_get_actions (
	FabulorChannelContextMenuModel *model);

#endif

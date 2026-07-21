/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_CHANNEL_LIST_CONTEXT_MENU_MODEL_H
#define FABULOR_CHANNEL_LIST_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_CHANNEL_LIST_CONTEXT_JOIN,
	FABULOR_CHANNEL_LIST_CONTEXT_COPY_CHANNELS,
	FABULOR_CHANNEL_LIST_CONTEXT_COPY_TOPICS,
	FABULOR_CHANNEL_LIST_CONTEXT_AUTOJOIN
} FabulorChannelListContextAction;

typedef struct
{
	const char *join;
	const char *copy_channels;
	const char *copy_topics;
	const char *autojoin;
	const char *join_icon;
	const char *copy_icon;
} FabulorChannelListContextLabels;

typedef void (*FabulorChannelListContextDispatch) (
	FabulorChannelListContextAction action, gboolean state,
	const GPtrArray *channels, const GPtrArray *topics, gpointer user_data);

typedef struct _FabulorChannelListContextMenuModel
	FabulorChannelListContextMenuModel;

FabulorChannelListContextMenuModel *
fabulor_channel_list_context_menu_model_new (const GPtrArray *channels,
	const GPtrArray *topics, gboolean has_network, gboolean autojoin,
	const FabulorChannelListContextLabels *labels,
	FabulorChannelListContextDispatch dispatch, gpointer user_data);
void fabulor_channel_list_context_menu_model_free (
	FabulorChannelListContextMenuModel *model);
GMenuModel *fabulor_channel_list_context_menu_model_get_menu (
	FabulorChannelListContextMenuModel *model);
GActionGroup *fabulor_channel_list_context_menu_model_get_actions (
	FabulorChannelListContextMenuModel *model);

#endif

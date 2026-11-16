#include <gio/gio.h>
#include "../../src/fe-gtk/nick-context-menu-model.h"

static void recognition (void)
{
	g_assert_true (fabulor_nick_context_is_dalnet ("DALnet", NULL));
	g_assert_true (fabulor_nick_context_is_dalnet ("dalNET", "bouncer.example"));
	g_assert_true (fabulor_nick_context_is_dalnet ("My ZNC", "irc.DAL.NET"));
	g_assert_true (fabulor_nick_context_is_dalnet (NULL, "dal.net"));
	g_assert_false (fabulor_nick_context_is_dalnet ("NotDALnet", "evildal.net"));
	g_assert_false (fabulor_nick_context_is_dalnet ("DALnet-ish", "dal.net.example"));
	g_assert_false (fabulor_nick_context_is_dalnet ("Libera.Chat", "irc.libera.chat"));
	g_assert_false (fabulor_nick_context_is_dalnet (NULL, NULL));
}

static void commands (void)
{
	const char *invalid[] = { "", "+nick", "-nick", "nick other", "nick\r\nOPER x y",
		"nick\nQUIT", "nick,other", ":nick", "nick\tother", "nick%a", "\xff" };
	char *raw = fabulor_nick_context_dccallow_command ("allow", "LongNicknameOver16Chars");
	g_assert_cmpstr (raw, ==, "DCCALLOW +LongNicknameOver16Chars");
	g_free (raw);
	raw = fabulor_nick_context_dccallow_command ("revoke", "Some[Nick]");
	g_assert_cmpstr (raw, ==, "DCCALLOW -Some[Nick]");
	g_free (raw);
	raw = fabulor_nick_context_dccallow_command ("list", NULL);
	g_assert_cmpstr (raw, ==, "DCCALLOW LIST");
	g_free (raw);
	for (guint i = 0; i < G_N_ELEMENTS (invalid); i++)
		g_assert_null (fabulor_nick_context_dccallow_command ("allow", invalid[i]));
	g_assert_null (fabulor_nick_context_dccallow_command ("allow", NULL));
	g_assert_null (fabulor_nick_context_dccallow_command (NULL, "nick"));
	g_assert_null (fabulor_nick_context_dccallow_command ("allow\r\nQUIT", "nick"));
}

typedef struct { guint calls; const char *operation; } Probe;
static void dispatch (FabulorNickContextAction action, const char *nick,
	const char *command, gboolean selection, gpointer data)
{
	Probe *probe = data;
	g_assert_cmpint (action, ==, FABULOR_NICK_CONTEXT_DCC_PERMISSION);
	g_assert_cmpstr (nick, ==, "FullNicknameOver16Chars");
	g_assert_false (selection);
	probe->calls++;
	probe->operation = command;
}

static void model (void)
{
	FabulorNickHandler handlers[] = {
		{ FABULOR_NICK_HANDLER_SUBMENU_BEGIN, "DCC Permissions (DALnet)", NULL, NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_DCC_PERMISSION, "Allow Files from This User", "allow", NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_DCC_PERMISSION, "Revoke File Permission", "revoke", NULL, FALSE, FALSE },
		{ FABULOR_NICK_HANDLER_DCC_PERMISSION, "Show DCC Allow List", "list", NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_SUBMENU_END, NULL, NULL, NULL, FALSE, FALSE }
	};
	Probe probe = { 0 };
	FabulorNickContextMenuModel *owner = fabulor_nick_context_menu_model_new_with_handlers (
		"FullNicknameOver16Chars", NULL, FALSE, NULL, TRUE, handlers,
		G_N_ELEMENTS (handlers), NULL, dispatch, &probe);
	GActionGroup *actions = fabulor_nick_context_menu_model_get_actions (owner);
	g_assert_true (g_action_group_get_action_enabled (actions, "command-0"));
	g_assert_false (g_action_group_get_action_enabled (actions, "command-1"));
	g_action_group_activate_action (actions, "command-0", NULL);
	g_assert_cmpuint (probe.calls, ==, 1);
	g_assert_cmpstr (probe.operation, ==, "allow");
	g_action_group_activate_action (actions, "command-1", NULL);
	g_assert_cmpuint (probe.calls, ==, 1);
	g_action_group_activate_action (actions, "command-2", NULL);
	g_assert_cmpuint (probe.calls, ==, 2);
	g_assert_cmpstr (probe.operation, ==, "list");
	fabulor_nick_context_menu_model_free (owner);
}

int main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/dalnet/recognition", recognition);
	g_test_add_func ("/dalnet/commands", commands);
	g_test_add_func ("/dalnet/model", model);
	return g_test_run ();
}

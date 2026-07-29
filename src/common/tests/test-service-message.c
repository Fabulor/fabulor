#include <glib.h>

#include "../service-message.h"

static void
test_nickserv_redaction (void)
{
	g_assert_cmpstr (service_message_for_display ("NickServ", "IDENTIFY secret"),
					==, "identify ****");
	g_assert_cmpstr (service_message_for_display ("NickServ@services.dal.net",
					"REGISTER secret user@example.test"), ==, "register ****");
	g_assert_cmpstr (service_message_for_display ("nickserv@example.test",
					"GHOST Barry secret"), ==, "ghost ****");
	g_assert_cmpstr (service_message_for_display ("NickServ", "RECOVER Barry secret"),
					==, "recover ****");
	g_assert_cmpstr (service_message_for_display ("NickServ", "RELEASE Barry secret"),
					==, "release ****");
	g_assert_cmpstr (service_message_for_display ("NickServ", "REGAIN Barry secret"),
					==, "regain ****");
}

static void
test_non_sensitive_message_unchanged (void)
{
	char memoserv_message[] = "LIST";
	char nickserv_message[] = "INFO Barry";
	char other_target_message[] = "IDENTIFY secret";
	char command_only[] = "IDENTIFY";

	g_assert_true (service_message_for_display ("MemoServ@services.dal.net",
					memoserv_message) == memoserv_message);
	g_assert_true (service_message_for_display ("NickServ",
					nickserv_message) == nickserv_message);
	g_assert_true (service_message_for_display ("NotNickServ",
					other_target_message) == other_target_message);
	g_assert_true (service_message_for_display ("NickServ",
					command_only) == command_only);
}

void
service_message_register_tests (void)
{
	g_test_add_func ("/service-message/nickserv-redaction", test_nickserv_redaction);
	g_test_add_func ("/service-message/non-sensitive-unchanged",
					 test_non_sensitive_message_unchanged);
}

#include <glib.h>

#include "../ircv3-chathistory.h"

static void
test_chathistory_limit (void)
{
	g_assert_cmpuint (ircv3_chathistory_parse_limit (NULL), ==, 50);
	g_assert_cmpuint (ircv3_chathistory_parse_limit ("0"), ==, 50);
	g_assert_cmpuint (ircv3_chathistory_parse_limit ("100"), ==, 100);
	g_assert_cmpuint (ircv3_chathistory_parse_limit ("1000"), ==, 500);
	g_assert_cmpuint (ircv3_chathistory_parse_limit ("50invalid"), ==, 50);
}

static void
test_chathistory_latest (void)
{
	char *command = ircv3_chathistory_latest_command ("#fabulor", 100);

	g_assert_cmpstr (command, ==, "CHATHISTORY LATEST #fabulor * 100");
	g_free (command);
	g_assert_null (ircv3_chathistory_latest_command ("#invalid target", 50));
	g_assert_null (ircv3_chathistory_latest_command ("", 50));
}

void
ircv3_chathistory_register_tests (void)
{
	g_test_add_func ("/ircv3/chathistory/limit", test_chathistory_limit);
	g_test_add_func ("/ircv3/chathistory/latest", test_chathistory_latest);
}

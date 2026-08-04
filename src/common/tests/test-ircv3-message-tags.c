#include <glib.h>

#include "../ircv3-message-tags.h"

static void
test_message_tag_values (void)
{
	ircv3_message_tags *tags;
	const char *value;
	gboolean present;

	tags = ircv3_message_tags_parse (
		"time=2026-08-04T01:02:03.000Z;account=barry\\ssmith;"
		"batch=history-1;label=fabulor-7;draft/chathistory-end");

	value = ircv3_message_tags_lookup (tags, "account", &present);
	g_assert_true (present);
	g_assert_cmpstr (value, ==, "barry smith");
	g_assert_cmpstr (ircv3_message_tags_lookup (tags, "batch", NULL), ==,
					 "history-1");
	g_assert_cmpstr (ircv3_message_tags_lookup (tags, "label", NULL), ==,
					 "fabulor-7");
	g_assert_null (ircv3_message_tags_lookup (tags, "draft/chathistory-end",
												&present));
	g_assert_true (present);
	g_assert_null (ircv3_message_tags_lookup (tags, "missing", &present));
	g_assert_false (present);

	ircv3_message_tags_free (tags);
}

static void
test_message_tag_escaping_and_duplicates (void)
{
	ircv3_message_tags *tags;

	tags = ircv3_message_tags_parse (
		"example=first;example=second\\:value\\swith\\\\slash\\r\\n;empty=");
	g_assert_cmpstr (ircv3_message_tags_lookup (tags, "example", NULL), ==,
					 "second;value with\\slash\r\n");
	g_assert_null (ircv3_message_tags_lookup (tags, "empty", NULL));
	ircv3_message_tags_free (tags);
}

void
ircv3_message_tags_register_tests (void)
{
	g_test_add_func ("/ircv3/message-tags/values", test_message_tag_values);
	g_test_add_func ("/ircv3/message-tags/escaping-and-duplicates",
					 test_message_tag_escaping_and_duplicates);
}

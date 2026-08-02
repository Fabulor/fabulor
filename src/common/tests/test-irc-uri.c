#include <glib.h>

#include "../irc-uri.h"

static void
assert_valid_uri (const char *text, const char *host, guint16 port,
				  const char *channel, const char *key, gboolean use_tls)
{
	FabulorIrcUri uri = { 0 };
	GError *error = NULL;

	g_assert_true (fabulor_irc_uri_parse (text, &uri, &error));
	g_assert_no_error (error);
	g_assert_cmpstr (uri.host, ==, host);
	g_assert_cmpuint (uri.port, ==, port);
	g_assert_cmpint (uri.has_port, ==, port != 0);
	g_assert_cmpstr (uri.channel, ==, channel);
	g_assert_cmpstr (uri.key, ==, key);
	g_assert_cmpint (uri.use_tls, ==, use_tls);
	fabulor_irc_uri_clear (&uri);
}

static void
assert_invalid_uri (const char *text)
{
	FabulorIrcUri uri = { 0 };
	GError *error = NULL;

	g_assert_false (fabulor_irc_uri_parse (text, &uri, &error));
	g_assert_nonnull (error);
	g_assert_null (uri.host);
	g_clear_error (&error);
	fabulor_irc_uri_clear (&uri);
}

static void
test_basic_uris (void)
{
	assert_valid_uri ("irc://irc.example.test", "irc.example.test", 0,
				  NULL, NULL, FALSE);
	assert_valid_uri ("IRCS://irc.example.test:6697/fabulor", "irc.example.test",
				  6697, "fabulor", NULL, TRUE);
	assert_valid_uri ("ircs://irc.example.test/%23help?secret", "irc.example.test",
				  0, "help", "secret", TRUE);
}

static void
test_ip_literals_and_encoding (void)
{
	assert_valid_uri ("irc://192.0.2.10:6667/test", "192.0.2.10", 6667,
				  "test", NULL, FALSE);
	assert_valid_uri ("ircs://[2001:db8::1]:6697/fabulor%2Ddev", "2001:db8::1",
				  6697, "fabulor-dev", NULL, TRUE);
}

static void
test_rejected_uris (void)
{
	const char *invalid[] = {
		"https://irc.example.test/test",
		"irc:///test",
		"irc://user@irc.example.test/test",
		"irc://irc.example.test:0/test",
		"irc://irc.example.test:65536/test",
		"irc://irc.example.test/test#fragment",
		"irc://irc.example.test/one,two",
		"irc://irc.example.test/test%0d%0aQUIT",
		"irc://irc.example.test/%ZZ",
		"irc://irc.example.test/?secret",
		NULL
	};
	int index;

	for (index = 0; invalid[index]; index++)
		assert_invalid_uri (invalid[index]);
}

void
irc_uri_register_tests (void)
{
	g_test_add_func ("/irc-uri/basic", test_basic_uris);
	g_test_add_func ("/irc-uri/ip-and-encoding", test_ip_literals_and_encoding);
	g_test_add_func ("/irc-uri/rejected", test_rejected_uris);
}

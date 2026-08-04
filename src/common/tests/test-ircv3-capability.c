#include <glib.h>

#include "../ircv3-capability.h"

static void
test_capability_token (void)
{
	ircv3_capability_token token;

	g_assert_true (ircv3_capability_token_parse ("sasl=PLAIN,EXTERNAL", &token));
	g_assert_cmpstr (token.name, ==, "sasl");
	g_assert_cmpstr (token.value, ==, "PLAIN,EXTERNAL");
	g_assert_false (token.disable);
	ircv3_capability_token_clear (&token);

	g_assert_true (ircv3_capability_token_parse ("-echo-message", &token));
	g_assert_cmpstr (token.name, ==, "echo-message");
	g_assert_null (token.value);
	g_assert_true (token.disable);
	ircv3_capability_token_clear (&token);

	g_assert_false (ircv3_capability_token_parse ("-", &token));
	g_assert_false (ircv3_capability_token_parse ("=value", &token));
}

static void
test_capability_request (void)
{
	static const char * const supported[] = {
		"away-notify", "echo-message", "server-time"
	};
	char *request;

	request = ircv3_capability_build_request (
		"unknown server-time=vendor away-notify server-time -echo-message",
		supported, G_N_ELEMENTS (supported));
	g_assert_cmpstr (request, ==, "server-time away-notify");
	g_free (request);

	request = ircv3_capability_build_request ("unknown other=value",
		supported, G_N_ELEMENTS (supported));
	g_assert_null (request);
}

static void
test_capability_list_contains (void)
{
	g_assert_true (ircv3_capability_list_contains (
		"* :batch draft/chathistory=vendor server-time", "draft/chathistory"));
	g_assert_false (ircv3_capability_list_contains (
		"batch -draft/chathistory server-time", "draft/chathistory"));
	g_assert_false (ircv3_capability_list_contains (
		"batch draft/chathistory-extra server-time", "draft/chathistory"));
}

static void
test_sasl_mechanisms (void)
{
	g_assert_true (ircv3_sasl_mechanism_available (
		"EXTERNAL,PLAIN,SCRAM-SHA-256", "PLAIN"));
	g_assert_true (ircv3_sasl_mechanism_available (
		"EXTERNAL,PLAIN,SCRAM-SHA-256", "SCRAM-SHA-256"));
	g_assert_false (ircv3_sasl_mechanism_available (
		"EXTERNAL,PLAIN,SCRAM-SHA-256", "SCRAM-SHA-512"));
	g_assert_false (ircv3_sasl_mechanism_available ("plain", "PLAIN"));
	g_assert_false (ircv3_sasl_mechanism_available (NULL, "PLAIN"));
}

void
ircv3_capability_register_tests (void)
{
	g_test_add_func ("/ircv3/capability-token", test_capability_token);
	g_test_add_func ("/ircv3/capability-request", test_capability_request);
	g_test_add_func ("/ircv3/capability-list-contains", test_capability_list_contains);
	g_test_add_func ("/ircv3/sasl-mechanisms", test_sasl_mechanisms);
}

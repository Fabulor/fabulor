#include <string.h>

#include <glib.h>

#include "../win32-ipc.h"

static void
assert_payload_valid (const char *payload)
{
	GError *error = NULL;

	g_assert_true (fabulor_win32_ipc_validate_irc_uri_payload (
		payload, strlen (payload) + 1, &error));
	g_assert_no_error (error);
}

static void
assert_payload_invalid (gconstpointer payload, gsize size)
{
	GError *error = NULL;

	g_assert_false (fabulor_win32_ipc_validate_irc_uri_payload (
		payload, size, &error));
	g_assert_nonnull (error);
	g_clear_error (&error);
}

static void
test_valid_payloads (void)
{
	assert_payload_valid ("irc://irc.example.test/fabulor");
	assert_payload_valid ("ircs://[2001:db8::1]:6697/help");
}

static void
test_invalid_wire_payloads (void)
{
	const char unterminated[] = { 'i', 'r', 'c' };
	const char embedded_nul[] = { 'i', 'r', 'c', '\0', 'x', '\0' };
	const unsigned char invalid_utf8[] = {
		'i', 'r', 'c', 's', ':', '/', '/', 0xff, '\0'
	};
	char oversized[FABULOR_WIN32_COPYDATA_MAX_BYTES + 1] = { 0 };

	assert_payload_invalid (NULL, 0);
	assert_payload_invalid (unterminated, sizeof (unterminated));
	assert_payload_invalid (embedded_nul, sizeof (embedded_nul));
	assert_payload_invalid (invalid_utf8, sizeof (invalid_utf8));
	assert_payload_invalid (oversized, sizeof (oversized));
}

static void
test_invalid_uri_payload (void)
{
	const char payload[] = "irc://irc.example.test:0/help";

	assert_payload_invalid (payload, sizeof (payload));
}

void
win32_ipc_register_tests (void)
{
	g_test_add_func ("/win32-ipc/valid-payloads", test_valid_payloads);
	g_test_add_func ("/win32-ipc/invalid-wire-payloads", test_invalid_wire_payloads);
	g_test_add_func ("/win32-ipc/invalid-uri-payload", test_invalid_uri_payload);
}

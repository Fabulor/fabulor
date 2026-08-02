#include <string.h>

#include <glib.h>

#include "irc-uri.h"
#include "win32-ipc.h"

typedef enum
{
	WIN32_IPC_ERROR_INVALID
} Win32IpcError;

static GQuark
win32_ipc_error_quark (void)
{
	return g_quark_from_static_string ("fabulor-win32-ipc-error");
}

static gboolean
set_invalid_error (GError **error, const char *message)
{
	g_set_error_literal (error, win32_ipc_error_quark (), WIN32_IPC_ERROR_INVALID,
					 message);
	return FALSE;
}

gboolean
fabulor_win32_ipc_validate_irc_uri_payload (gconstpointer data, gsize size,
											 GError **error)
{
	const char *payload = data;
	FabulorIrcUri uri = { 0 };
	GError *parse_error = NULL;

	if (!payload || size <= 1 || size > FABULOR_WIN32_COPYDATA_MAX_BYTES)
		return set_invalid_error (error, "The Windows URI message has an invalid size.");

	if (payload[size - 1] != '\0' || memchr (payload, '\0', size - 1))
		return set_invalid_error (error, "The Windows URI message is not a single terminated string.");

	if (!g_utf8_validate (payload, size - 1, NULL))
		return set_invalid_error (error, "The Windows URI message is not valid UTF-8.");

	if (!fabulor_irc_uri_parse (payload, &uri, &parse_error))
	{
		g_propagate_prefixed_error (error, parse_error, "The Windows URI message is invalid: ");
		return FALSE;
	}

	fabulor_irc_uri_clear (&uri);
	return TRUE;
}

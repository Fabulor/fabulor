#ifndef FABULOR_WIN32_IPC_H
#define FABULOR_WIN32_IPC_H

#include <glib.h>

#define FABULOR_WIN32_COPYDATA_IRC_URI 0x46555249UL
#define FABULOR_WIN32_COPYDATA_MAX_BYTES 4096
#define FABULOR_WIN32_IPC_WINDOW_PROPERTY "Fabulor.IrcUriIpc.v1"
#define FABULOR_WIN32_IPC_WINDOW_MARKER 0x46555249UL

gboolean fabulor_win32_ipc_validate_irc_uri_payload (gconstpointer data,
												 gsize size, GError **error);

#endif

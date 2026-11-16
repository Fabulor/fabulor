/* Internal credentialed OPER helpers, not a public plugin ABI. */
#ifndef FABULOR_OPER_AUTH_H
#define FABULOR_OPER_AUTH_H
#include <glib.h>
struct server;
void oper_auth_clear (void *data, gsize length);
gboolean oper_auth_name_valid (const char *name);
gboolean oper_auth_password_valid (const char *password);
const char *oper_auth_log_line (const char *line);
gboolean server_send_oper (struct server *serv, const char *name, const char *password);
#endif

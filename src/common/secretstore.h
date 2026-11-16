#ifndef FABULOR_SECRETSTORE_H
#define FABULOR_SECRETSTORE_H

char *secretstore_get_network_password (const char *network_name);
int secretstore_set_network_password (const char *network_name, const char *password);
int secretstore_delete_network_password (const char *network_name);
int secretstore_is_keyring_available (void);
int secretstore_require_unlock (const char *network_name);
/* Separate purpose-scoped OPER records; never the connection-password slot.
 * No profile-file fallback. Returned password must be wiped before freeing. */
char *secretstore_get_oper_password (const char *scope, const char *oper_name);
int secretstore_set_oper_password (const char *scope, const char *oper_name, const char *password);
int secretstore_delete_oper_password (const char *scope, const char *oper_name);

#endif

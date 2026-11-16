#ifndef FABULOR_OPER_LOGIN_H
#define FABULOR_OPER_LOGIN_H
#include <gtk/gtk.h>
struct session;
struct server;
typedef struct OperLogin OperLogin;
OperLogin *oper_login_new (struct session *sess);
void oper_login_open (OperLogin *login, GtkWindow *parent);
const char *oper_login_status (OperLogin *login);
void oper_login_free (OperLogin *login);
void fe_oper_login_reply (struct server *serv, int numeric);
#endif

/* Hidden GTK and synthetic credentials only. Never WCM, a user profile or IRC. */
#include <gtk/gtk.h>
static void no_present (GtkWindow *window) { (void)window; }
#define gtk_window_present no_present
#define server_send_oper validated_server_send_oper
#include "../../src/common/oper-auth.c"
#undef server_send_oper
gboolean server_send_oper (server *serv, const char *name, const char *password);
#include "../../src/fe-gtk/oper-login.c"
static int sends, saves, reads, deletes;
static gboolean save_ok=TRUE;
static char *last_scope, *wire_capture;
static int wire_calls, aborted;
static int transport_result=-2;
int is_server (server *serv) { return serv!=NULL; }
int _SSL_send (SSL *ssl, char *buf, int len)
{ (void)ssl; wire_calls++; g_free(wire_capture); wire_capture=g_strndup(buf,len); return transport_result==-2?len:transport_result; }
void fe_add_rawlog (server *serv, char *buf, int len, int outbound)
{ (void)serv; (void)len; g_assert_true(outbound); g_assert_cmpstr(buf,==,"OPER [credentials redacted]\r\n"); }
static void disconnect_stub (session *sess, int sendquit, int error)
{ (void)error; g_assert_false(sendquit); aborted++; sess->server->connected=FALSE; }
int is_session (session *sess) { return sess!=NULL; }
char *get_xdir (void) { return "C:/synthetic-profile"; }
char *server_get_network (server *serv, gboolean fallback) { (void)serv; (void)fallback; return "Synthetic network"; }
void theme_manager_attach_window (GtkWidget *window) { (void)window; }
char *secretstore_get_oper_password (const char *scope, const char *name)
{ (void)name; reads++; g_free(last_scope); last_scope=g_strdup(scope); return g_strdup("synthetic-password"); }
int secretstore_set_oper_password (const char *scope, const char *name, const char *password)
{ (void)scope; (void)name; g_assert_cmpstr(password,==,"synthetic-password"); saves++; return save_ok; }
int secretstore_delete_oper_password (const char *scope, const char *name)
{ (void)scope; (void)name; deletes++; return TRUE; }
gboolean server_send_oper (server *serv, const char *name, const char *password)
{ (void)serv; g_assert_cmpstr(name,==,"test_oper"); g_assert_cmpstr(password,==,"synthetic-password"); sends++; return TRUE; }
static void guards (void)
{
 char secret[]="synthetic-password";
 g_assert_true(oper_auth_name_valid("test_oper")); g_assert_false(oper_auth_name_valid("a b"));
 g_assert_false(oper_auth_name_valid("a\nOPER")); g_assert_false(oper_auth_name_valid(":a"));
 g_assert_true(oper_auth_password_valid(secret)); g_assert_false(oper_auth_password_valid("a\r\nQUIT"));
 g_assert_cmpstr(oper_auth_log_line("OPER test_oper :synthetic-password"),==,"OPER [credentials redacted]");
 g_assert_cmpstr(oper_auth_log_line(" oper\tname password"),==,"OPER [credentials redacted]");
 g_assert_cmpstr(oper_auth_log_line("OPERATIONS text"),==,"OPERATIONS text");
 oper_auth_clear(secret,sizeof secret); g_assert_cmpint(secret[0],==,0);
}
static void guarded_dispatch (void)
{
 server serv={0}; session sess={0}; SSL_CTX *ctx=SSL_CTX_new(TLS_method()); SSL *ssl=SSL_new(ctx);
 sess.server=&serv; serv.server_session=&sess; serv.disconnect=disconnect_stub;
 serv.ssl=ssl; serv.network=&serv; serv.connected=TRUE; serv.end_of_motd=TRUE;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 g_assert_cmpint(wire_calls,==,0); serv.oper_auth_tls_verified=TRUE;
 SSL_set_verify_result(ssl,X509_V_ERR_CERT_HAS_EXPIRED);
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 SSL_set_verify_result(ssl,X509_V_OK);
 g_assert_true(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 g_assert_cmpstr(wire_capture,==,"OPER test_oper :synthetic-password\r\n");
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 serv.oper_auth_next_attempt=0; serv.accept_invalid_cert=TRUE;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 serv.accept_invalid_cert=FALSE; serv.ssl=NULL;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 serv.ssl=ssl; serv.outbound_queue=(GSList *)&serv;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 serv.outbound_queue=NULL; serv.next_send=time(NULL)+30;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 serv.next_send=0;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","x\r\nQUIT"));
 g_assert_cmpint(wire_calls,==,1);
 transport_result=-1;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 g_assert_false(serv.oper_auth_tls_verified); g_assert_cmpint(aborted,==,0);
 while (g_main_context_pending(NULL)) g_main_context_iteration(NULL,FALSE);
 g_assert_cmpint(aborted,==,1); g_assert_false(serv.connected);
 serv.connected=TRUE; serv.oper_auth_tls_verified=TRUE; serv.oper_auth_next_attempt=0;
 serv.oper_auth_generation++; transport_result=4;
 g_assert_false(validated_server_send_oper(&serv,"test_oper","synthetic-password"));
 while (g_main_context_pending(NULL)) g_main_context_iteration(NULL,FALSE);
 g_assert_cmpint(aborted,==,2); g_assert_cmpint(wire_calls,==,3);
 oper_auth_clear(wire_capture,strlen(wire_capture)); g_clear_pointer(&wire_capture,g_free);
 SSL_free(ssl); SSL_CTX_free(ctx);
}
static void hidden_flow (void)
{
 server serv={0}; session sess={0}; OperLogin *a; GtkWidget *parent; SSL_CTX *ctx;
 ctx=SSL_CTX_new(TLS_method()); g_assert_nonnull(ctx); serv.ssl=SSL_new(ctx); g_assert_nonnull(serv.ssl);
 serv.id=7; serv.port=6697; serv.connected=TRUE; serv.end_of_motd=TRUE; serv.network=&serv; serv.oper_auth_tls_verified=TRUE;
 g_strlcpy(serv.hostname,"synthetic.invalid",sizeof serv.hostname); sess.server=&serv;
 parent=gtk_window_new(); a=oper_login_new(&sess); g_assert_nonnull(a);
 oper_login_open(a,GTK_WINDOW(parent)); g_assert_nonnull(a->dialog); g_assert_true(GTK_IS_PASSWORD_ENTRY(a->password));
 gtk_editable_set_text(GTK_EDITABLE(a->password),"synthetic-password");
 response(GTK_WINDOW(a->dialog),GTK_RESPONSE_CANCEL,a);
 g_assert_null(a->dialog); g_assert_cmpint(sends,==,0); g_assert_cmpint(reads,==,0);
 oper_login_open(a,GTK_WINDOW(parent));
 gtk_editable_set_text(GTK_EDITABLE(a->name),"test_oper");
 gtk_editable_set_text(GTK_EDITABLE(a->password),"synthetic-password");
 response(GTK_WINDOW(a->dialog),GTK_RESPONSE_ACCEPT,a); g_assert_cmpint(sends,==,0);
 gtk_check_button_set_active(GTK_CHECK_BUTTON(a->confirm),TRUE);
 gtk_check_button_set_active(GTK_CHECK_BUTTON(a->remember),TRUE); save_ok=FALSE;
 response(GTK_WINDOW(a->dialog),GTK_RESPONSE_ACCEPT,a); g_assert_cmpint(sends,==,0); g_assert_cmpint(saves,==,1);
 save_ok=TRUE; gtk_editable_set_text(GTK_EDITABLE(a->password),"synthetic-password");
 response(GTK_WINDOW(a->dialog),GTK_RESPONSE_ACCEPT,a); g_assert_cmpint(sends,==,1); g_assert_true(a->waiting);
 g_assert_null(a->dialog); g_assert_cmpint(saves,==,2);
 fe_oper_login_reply(&serv,381); g_assert_false(a->waiting); g_assert_nonnull(strstr(a->status,"confirmed"));
 a->next_attempt=0; oper_login_open(a,GTK_WINDOW(parent));
 gtk_editable_set_text(GTK_EDITABLE(a->name),"test_oper");
 gtk_check_button_set_active(GTK_CHECK_BUTTON(a->saved),TRUE); gtk_check_button_set_active(GTK_CHECK_BUTTON(a->confirm),TRUE);
 serv.accept_invalid_cert=TRUE; response(GTK_WINDOW(a->dialog),GTK_RESPONSE_ACCEPT,a);
 g_assert_cmpint(reads,==,0); g_assert_cmpint(sends,==,1);
 serv.accept_invalid_cert=FALSE; response(GTK_WINDOW(a->dialog),GTK_RESPONSE_ACCEPT,a);
 g_assert_cmpint(reads,==,1); g_assert_cmpint(sends,==,2);
 g_assert_nonnull(strstr(last_scope,"synthetic-profile"));
 fe_oper_login_reply(&serv,464); g_assert_false(a->waiting); g_assert_nonnull(strstr(a->status,"rejected"));
 a->waiting=TRUE; a->deadline=g_get_monotonic_time()-1; tick(a); g_assert_false(a->waiting); g_assert_nonnull(strstr(a->status,"No OPER confirmation"));
 a->next_attempt=0; oper_login_open(a,GTK_WINDOW(parent)); gtk_editable_set_text(GTK_EDITABLE(a->name),"test_oper");
 response(GTK_WINDOW(a->dialog),1,a); g_assert_cmpint(deletes,==,1); g_assert_cmpint(sends,==,2);
 serv.oper_auth_generation++; tick(a); g_assert_null(a->dialog); g_assert_true(a->expired);
 oper_login_open(a,GTK_WINDOW(parent)); g_assert_null(a->dialog);
 oper_login_free(a); g_assert_null(logins);
 a=oper_login_new(&sess); oper_login_open(a,GTK_WINDOW(parent)); g_assert_nonnull(a->dialog);
 parent_unrealized(parent,a); g_assert_null(a->dialog); g_assert_true(a->expired);
 oper_login_free(a); g_assert_null(logins); gtk_window_destroy(GTK_WINDOW(parent));
 SSL_free(serv.ssl); SSL_CTX_free(ctx); g_clear_pointer(&last_scope,g_free);
}
int main (int argc, char **argv)
{
 gtk_init(); g_test_init(&argc,&argv,NULL);
 g_test_add_func("/oper-auth/guards-and-redaction",guards);
 g_test_add_func("/oper-auth/guarded-dispatch",guarded_dispatch);
 g_test_add_func("/oper-auth/hidden-credential-flow",hidden_flow);
 return g_test_run();
}

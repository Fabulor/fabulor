/* Host-owned credentialed OPER: no password in commands, schemas or callbacks. */
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#endif
#include "../common/fabulor.h"
#include "../common/fabulorc.h"
#include "../common/server.h"
#include "../common/cfgfiles.h"
#include "theme/theme-manager.h"
#include "../common/oper-auth.h"
#include "../common/secretstore.h"
#include "oper-login.h"

struct OperLogin {
 session *session;
 int server_id, port;
 guint64 generation;
 char *host, *network, *scope;
 GtkWidget *dialog, *name, *password, *saved, *remember, *confirm, *feedback;
 GtkWindow *parent;
 guint timer;
 gint64 deadline, next_attempt;
 gboolean waiting, expired;
 const char *status;
};
static GList *logins;
static gboolean live (OperLogin *a)
{
 server *s;
 if (a->expired || !is_session(a->session)) return FALSE;
 s=a->session->server;
 return s->id==a->server_id && s->oper_auth_generation==a->generation
  && s->connected && s->end_of_motd && s->port==a->port
  && !g_strcmp0(s->hostname,a->host)
  && !g_strcmp0(server_get_network(s,FALSE),a->network);
}
static gboolean safe_transport (OperLogin *a)
{
#if defined(WIN32) && defined(USE_OPENSSL)
 server *s;
 if (!live(a)) return FALSE;
 s=a->session->server;
 return s->network && s->ssl && s->oper_auth_tls_verified && !s->accept_invalid_cert
  && SSL_get_verify_result(s->ssl)==X509_V_OK;
#else
 (void)a; return FALSE;
#endif
}
static void message (OperLogin *a, const char *text)
{
 a->status=text;
 if (a->feedback) gtk_label_set_text(GTK_LABEL(a->feedback),text);
}
static void close_dialog (OperLogin *a)
{
 if (a->dialog) {
  g_signal_handlers_disconnect_by_data(a->dialog,a);
  gtk_editable_set_text(GTK_EDITABLE(a->password),"");
  gtk_window_destroy(GTK_WINDOW(a->dialog)); g_clear_object(&a->dialog);
 }
 a->name=a->password=a->saved=a->remember=a->confirm=a->feedback=NULL;
}
static void parent_unrealized (GtkWidget *parent, gpointer data)
{
 OperLogin *a=data; (void)parent;
 a->expired=TRUE; a->waiting=FALSE;
 message(a,"Panel closed; unsent login/tracking cancelled. A sent OPER cannot be recalled.");
 close_dialog(a);
}
static gboolean tick (gpointer data)
{
 OperLogin *a=data;
 if (!live(a)) {
  a->expired=TRUE; a->waiting=FALSE;
  message(a,"Connection changed or disconnected; reopen the control panel.");
  close_dialog(a);
 } else if (a->waiting && g_get_monotonic_time()>=a->deadline) {
  a->waiting=FALSE; message(a,"No OPER confirmation received; no retry made.");
 }
 return G_SOURCE_CONTINUE;
}
OperLogin *oper_login_new (session *sess)
{
 OperLogin *a;
 const char *network;
 if (!is_session(sess)) return NULL;
 network=server_get_network(sess->server,FALSE);
 if (!network || !*network || !sess->server->network) return NULL;
 a=g_new0(OperLogin,1); a->session=sess; a->server_id=sess->server->id;
 a->generation=sess->server->oper_auth_generation; a->port=sess->server->port;
 a->host=g_strdup(sess->server->hostname); a->network=g_strdup(network);
 /* Separate purpose/account records; bind to profile name AND configured peer. */
 {
  char *profile=g_uri_escape_string(get_xdir(),NULL,FALSE);
  char *net=g_uri_escape_string(network,NULL,FALSE);
  char *host=g_uri_escape_string(a->host,NULL,FALSE);
  a->scope=g_strdup_printf("%s/%s/%s:%d",profile,net,host,a->port);
  g_free(profile); g_free(net); g_free(host);
 }
 a->status="No OPER login attempted.";
 a->timer=g_timeout_add(200,tick,a); logins=g_list_prepend(logins,a); return a;
}
const char *oper_login_status (OperLogin *a)
{
 if (!a) return "OPER login is unavailable for this connection.";
 tick(a); return a->status;
}
void oper_login_free (OperLogin *a)
{
 if (!a) return;
 logins=g_list_remove(logins,a);
 if (a->timer) g_source_remove(a->timer);
 if (a->parent) {
  g_signal_handlers_disconnect_by_data(a->parent,a);
  g_object_remove_weak_pointer(G_OBJECT(a->parent),(gpointer *)&a->parent);
 }
 close_dialog(a); g_free(a->host); g_free(a->network); g_free(a->scope); g_free(a);
}
void fe_oper_login_reply (server *serv, int numeric)
{
 GList *item;
 for (item=logins;item;item=item->next) {
  OperLogin *a=item->data;
  if (!a->waiting || !live(a) || a->session->server!=serv) continue;
  if (g_get_monotonic_time()>=a->deadline) { tick(a); continue; }
  if (numeric==381) { a->waiting=FALSE; message(a,"Server confirmed IRC operator login (381). Services/channel rights remain separate."); }
  else if (numeric==464 || numeric==491) { a->waiting=FALSE; message(a,"Server rejected OPER authentication; no retry made."); }
 }
}
static void response (GtkWindow *dialog, int response_id, gpointer data)
{
 OperLogin *a=data; char *name, *password=NULL; const char *typed;
 gboolean use_saved, remember;
 (void)dialog;
 if (response_id!=GTK_RESPONSE_ACCEPT && response_id!=1) {
  close_dialog(a); if (!a->waiting) message(a,"OPER dialog cancelled; no login sent."); return;
 }
 if (!live(a)) { tick(a); return; }
 name=g_strdup(gtk_editable_get_text(GTK_EDITABLE(a->name)));
 if (!oper_auth_name_valid(name)) { message(a,"Enter an oper name of 1-64 printable ASCII characters, without spaces or colons."); goto done; }
 if (response_id==1) {
  /* Explicit forget action affects only this purpose/profile/peer/oper-name record. */
  if (secretstore_delete_oper_password(a->scope,name)) {
   gtk_editable_set_text(GTK_EDITABLE(a->password),"");
   message(a,"Saved OPER credential removed (or not present); no login sent.");
  } else message(a,"Credential Manager deletion failed; no login sent.");
  goto done;
 }
 if (!safe_transport(a)) { message(a,"A verified TLS connection and named network profile are required; no login sent."); goto done; }
 if (a->waiting || g_get_monotonic_time()<a->next_attempt
     || g_get_monotonic_time()<a->session->server->oper_auth_next_attempt) { message(a,"Wait before another explicit OPER attempt; no retry made."); goto done; }
 if (!gtk_check_button_get_active(GTK_CHECK_BUTTON(a->confirm))) { message(a,"Confirm the intended peer and any bouncer uplink before login."); goto done; }
 use_saved=gtk_check_button_get_active(GTK_CHECK_BUTTON(a->saved));
 remember=gtk_check_button_get_active(GTK_CHECK_BUTTON(a->remember));
 typed=gtk_editable_get_text(GTK_EDITABLE(a->password));
 if (use_saved && (*typed || remember)) { message(a,"Choose either a typed password or the saved password, not both."); goto done; }
 password=use_saved?secretstore_get_oper_password(a->scope,name):g_strdup(typed);
 gtk_editable_set_text(GTK_EDITABLE(a->password),"");
 if (!oper_auth_password_valid(password)) { message(a,"No usable password supplied/stored; no login sent."); goto done; }
 if (remember && !secretstore_set_oper_password(a->scope,name,password)) { message(a,"Credential Manager save failed; no profile-file fallback and no login sent."); goto done; }
 /* Recheck at the actual send boundary; no command parser or add-on callback. */
 if (!safe_transport(a) || !server_send_oper(a->session->server,name,password)) { message(a,"OPER delivery not confirmed: transport unavailable or busy; no retry made."); goto done; }
 a->waiting=TRUE; a->deadline=a->next_attempt=g_get_monotonic_time()+20*G_USEC_PER_SEC;
 close_dialog(a); message(a,"OPER request dispatched; awaiting server confirmation, not proof of success.");
 done:
 if (password) { oper_auth_clear(password,strlen(password)); g_free(password); }
 g_free(name);
}
static void action_clicked (GtkButton *button, gpointer data)
{
 OperLogin *a=data;
 response(GTK_WINDOW(a->dialog),GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button),"oper-response")),a);
}
static gboolean dialog_closed (GtkWindow *window, gpointer data)
{
 response(window,GTK_RESPONSE_CANCEL,data); return TRUE;
}
static void append_action (GtkWidget *box, OperLogin *a, const char *label, int action)
{
 GtkWidget *button=gtk_button_new_with_label(label);
 g_object_set_data(G_OBJECT(button),"oper-response",GINT_TO_POINTER(action));
 g_signal_connect(button,"clicked",G_CALLBACK(action_clicked),a);
 gtk_box_append(GTK_BOX(box),button);
}
static void append_label (GtkWidget *box, const char *text)
{
 GtkWidget *label=gtk_label_new(text);
 gtk_label_set_xalign(GTK_LABEL(label),0); gtk_label_set_wrap(GTK_LABEL(label),TRUE);
 gtk_box_append(GTK_BOX(box),label);
}
void oper_login_open (OperLogin *a, GtkWindow *parent)
{
 GtkWidget *box, *actions; char *target;
 if (!a) return;
 if (!safe_transport(a)) { message(a,"OPER login requires the original named profile and verified TLS; reopen after reconnecting."); return; }
 if (a->dialog) { gtk_window_present(GTK_WINDOW(a->dialog)); return; }
 if (a->waiting) { message(a,"An OPER request is already awaiting confirmation; no retry made."); return; }
 if (!a->parent) {
  a->parent=parent;
  g_object_add_weak_pointer(G_OBJECT(parent),(gpointer *)&a->parent);
  g_signal_connect(parent,"unrealize",G_CALLBACK(parent_unrealized),a);
 }
 a->dialog=gtk_window_new(); g_object_ref_sink(a->dialog);
 gtk_window_set_title(GTK_WINDOW(a->dialog),"IRC Operator login");
 gtk_window_set_transient_for(GTK_WINDOW(a->dialog),parent);
 gtk_window_set_modal(GTK_WINDOW(a->dialog),TRUE);
 theme_manager_attach_window(a->dialog); gtk_window_set_default_size(GTK_WINDOW(a->dialog),540,-1);
 box=gtk_box_new(GTK_ORIENTATION_VERTICAL,12); gtk_window_set_child(GTK_WINDOW(a->dialog),box);
 gtk_widget_set_margin_start(box,16); gtk_widget_set_margin_end(box,16);
 gtk_widget_set_margin_top(box,16); gtk_widget_set_margin_bottom(box,16);
 target=g_strdup_printf("Pinned: %s / %s:%d. This is IRC OPER, not OperServ authentication.",a->network,a->host,a->port);
 append_label(box,target); g_free(target);
 append_label(box,"Oper name (not necessarily your nickname)"); a->name=gtk_entry_new();
 gtk_entry_set_max_length(GTK_ENTRY(a->name),64); gtk_box_append(GTK_BOX(box),a->name);
 append_label(box,"Password"); a->password=gtk_password_entry_new();
 gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(a->password),FALSE);
 {
  GtkEditable *delegate=gtk_editable_get_delegate(GTK_EDITABLE(a->password));
  if (GTK_IS_TEXT(delegate)) gtk_text_set_max_length(GTK_TEXT(delegate),400);
 }
 gtk_box_append(GTK_BOX(box),a->password);
 a->saved=gtk_check_button_new_with_label("Use the saved password for this oper name (leave password blank)");
 a->remember=gtk_check_button_new_with_label("Remember the typed password in Windows Credential Manager");
 a->confirm=gtk_check_button_new_with_label("I confirm this peer is intended and any bouncer uplink is secure");
 gtk_box_append(GTK_BOX(box),a->saved); gtk_box_append(GTK_BOX(box),a->remember); gtk_box_append(GTK_BOX(box),a->confirm);
 append_label(box,"No automatic login/retry. Passwords use UTF-8 and stay out of panel events/profile settings. Credential Manager protects storage under your Windows account, not against trusted local software. A TLS write failure closes this connection; sent OPER cannot be recalled.");
 a->feedback=gtk_label_new("Nothing sent. Existing plaintext passwords are not imported or removed.");
 gtk_label_set_wrap(GTK_LABEL(a->feedback),TRUE); gtk_box_append(GTK_BOX(box),a->feedback);
 actions=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
 append_action(actions,a,"Cancel",GTK_RESPONSE_CANCEL);
 append_action(actions,a,"Forget saved password",1);
 append_action(actions,a,"Log in",GTK_RESPONSE_ACCEPT);
 gtk_box_append(GTK_BOX(box),actions);
 g_signal_connect(a->dialog,"close-request",G_CALLBACK(dialog_closed),a);
 message(a,"OPER credential dialog open; nothing sent."); gtk_window_present(GTK_WINDOW(a->dialog));
}

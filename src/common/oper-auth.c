/* Pure input/display guards shared by credentialed login and Raw Log. */
#include <string.h>
#include "oper-auth.h"
#include "fabulor.h"
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#endif
#include "server.h"
#include "fe.h"
#ifdef USE_OPENSSL
#include "ssl.h"
#endif
#include <time.h>

void oper_auth_clear (void *data, gsize length)
{
 volatile unsigned char *p=(volatile unsigned char *)data;
 if (p) while (length--) *p++=0;
}
gboolean oper_auth_name_valid (const char *name)
{
 const unsigned char *p=(const unsigned char *)name;
 if (!name || !*name || strlen(name)>64) return FALSE;
 for (;*p;p++) if (*p<=32 || *p>=127 || *p==':') return FALSE;
 return TRUE;
}
gboolean oper_auth_password_valid (const char *password)
{
 return password && *password && strlen(password)<=400
  && !strchr(password,'\r') && !strchr(password,'\n')
  && g_utf8_validate(password,-1,NULL);
}
const char *oper_auth_log_line (const char *line)
{
 const char *p=line;
 if (!p) return "";
 while (*p==' ' || *p=='\t') p++;
 if (!g_ascii_strncasecmp(p,"OPER",4) && (!p[4] || g_ascii_isspace(p[4])))
  return "OPER [credentials redacted]";
 return line;
}

#if defined(WIN32) && defined(USE_OPENSSL)
typedef struct { server *serv; int id; guint64 generation; } OperTransportAbort;
static gboolean abort_failed_transport (gpointer data)
{
 OperTransportAbort *a=data;
 if (is_server(a->serv) && a->serv->id==a->id
     && a->serv->oper_auth_generation==a->generation && a->serv->server_session)
  a->serv->disconnect(a->serv->server_session,FALSE,0);
 return G_SOURCE_REMOVE;
}
#endif

/* Use the established non-blocking TLS stream directly, never the command
 * parser/hooks/history, encoder/static buffers or ordinary send queue. OPER
 * names are ASCII and passwords are UTF-8. Raw Log receives only a literal. */
gboolean server_send_oper (server *serv, const char *name, const char *password)
{
#if defined(WIN32) && defined(USE_OPENSSL)
 char *wire; char redacted[]="OPER [credentials redacted]\r\n"; int result, length;
 if (!serv || !serv->network || !serv->connected || !serv->end_of_motd || !serv->ssl
     || !serv->oper_auth_tls_verified || serv->accept_invalid_cert
     || SSL_get_verify_result(serv->ssl)!=X509_V_OK
     || serv->outbound_queue || serv->next_send>time(NULL)
     || g_get_monotonic_time()<serv->oper_auth_next_attempt
     || !oper_auth_name_valid(name) || !oper_auth_password_valid(password)) return FALSE;
 wire=g_strdup_printf("OPER %s :%s\r\n",name,password); length=(int)strlen(wire);
 serv->oper_auth_next_attempt=g_get_monotonic_time()+20*G_USEC_PER_SEC;
 fe_add_rawlog(serv,redacted,(int)strlen(redacted),TRUE);
 result=_SSL_send(serv->ssl,wire,length);
 oper_auth_clear(wire,length); g_free(wire);
 if (result!=length) {
  OperTransportAbort *a=g_new0(OperTransportAbort,1);
  /* Do not discard/reuse an incomplete SSL_write with a different buffer.
   * Abort on the next high-priority main-loop turn, after native callbacks
   * unwind. No QUIT, replay or credential retry. Delivery remains uncertain. */
  serv->oper_auth_tls_verified=FALSE;
  a->serv=serv; a->id=serv->id; a->generation=serv->oper_auth_generation;
  g_idle_add_full(G_PRIORITY_HIGH,abort_failed_transport,a,g_free);
 }
 return result==length;
#else
 (void)serv; (void)name; (void)password; return FALSE;
#endif
}

/* Mock WinCred functions: never accesses the real Windows credential store. */
#include <glib.h>
#include <windows.h>
#include <wincred.h>
#include <string.h>
static WCHAR *stored_target;
static char *stored_blob;
static gboolean fail_write;
static int writes, reads;
static BOOL WINAPI fake_write (PCREDENTIALW value, DWORD flags)
{
 (void)flags; writes++;
 if (fail_write) return FALSE;
 g_free(stored_target); g_free(stored_blob);
 stored_target=g_memdup2(value->TargetName,(wcslen(value->TargetName)+1)*sizeof(WCHAR));
 stored_blob=g_strndup((char *)value->CredentialBlob,value->CredentialBlobSize);
 return TRUE;
}
static BOOL WINAPI fake_read (LPCWSTR target, DWORD type, DWORD flags, PCREDENTIALW *result)
{
 CREDENTIALW *value; (void)type; (void)flags; reads++;
 if (!stored_target || _wcsicmp(target,stored_target)) return FALSE;
 value=g_new0(CREDENTIALW,1); value->CredentialBlobSize=(DWORD)strlen(stored_blob);
 value->CredentialBlob=g_memdup2(stored_blob,value->CredentialBlobSize); *result=value;
 return TRUE;
}
static void WINAPI fake_free (PVOID data)
{
 CREDENTIALW *value=data; DWORD i;
 for(i=0;i<value->CredentialBlobSize;i++) g_assert_cmpint(value->CredentialBlob[i],==,0);
 g_free(value->CredentialBlob); g_free(value);
}
static BOOL WINAPI fake_delete (LPCWSTR target, DWORD type, DWORD flags)
{
 (void)type; (void)flags;
 if (stored_target && !_wcsicmp(target,stored_target)) {
  g_clear_pointer(&stored_target,g_free); g_clear_pointer(&stored_blob,g_free); return TRUE;
 }
 SetLastError(ERROR_NOT_FOUND); return FALSE;
}
#define CredWriteW fake_write
#define CredReadW fake_read
#define CredDeleteW fake_delete
#define CredFree fake_free
#include "../../src/common/secretstore.c"
#include "../../src/common/oper-auth.c"
int is_server (server *serv) { (void)serv; return FALSE; }
int _SSL_send (SSL *ssl, char *buf, int len)
{ (void)ssl; (void)buf; (void)len; g_error("No network transport is allowed in credential tests"); }
void fe_add_rawlog (server *serv, char *buf, int len, int outbound)
{ (void)serv; (void)buf; (void)len; (void)outbound; g_error("No raw log is allowed in credential tests"); }
static void scoped_store (void)
{
 char *password, *target;
 g_assert_true(secretstore_set_oper_password("synthetic-profile-A/peer", "test_oper", "synthetic-password"));
 target=g_utf16_to_utf8((gunichar2 *)stored_target,-1,NULL,NULL,NULL);
 g_assert_true(g_str_has_prefix(target,"fabulor/irc-oper/"));
 g_assert_null(strstr(target,"synthetic-password")); g_free(target);
 password=secretstore_get_oper_password("synthetic-profile-A/peer","test_oper");
 g_assert_cmpstr(password,==,"synthetic-password"); oper_auth_clear(password,strlen(password)); g_free(password);
 g_assert_null(secretstore_get_oper_password("synthetic-profile-B/peer","test_oper"));
 g_assert_null(secretstore_get_oper_password("synthetic-profile-A/peer","different_oper"));
 g_assert_false(secretstore_set_oper_password("synthetic-profile-A/peer","test_oper","x\r\nQUIT"));
 g_assert_null(secretstore_get_oper_password("synthetic-profile-a/peer","test_oper"));
 g_assert_null(secretstore_get_oper_password("synthetic-profile-A/peer","TEST_OPER"));
 g_assert_cmpint(writes,==,1); g_assert_cmpint(reads,==,5);
 fail_write=TRUE;
 g_assert_false(secretstore_set_oper_password("synthetic-profile-A/peer","test_oper","replacement"));
 g_assert_true(secretstore_delete_oper_password("synthetic-profile-A/peer","test_oper"));
 g_assert_null(stored_target); g_assert_null(stored_blob);
 g_assert_true(secretstore_delete_oper_password("synthetic-profile-A/peer","test_oper"));
}
int main (int argc, char **argv)
{
 g_test_init(&argc,&argv,NULL);
 g_test_add_func("/oper-auth/mock-windows-credential-scope-failure-wipe",scoped_store);
 return g_test_run();
}

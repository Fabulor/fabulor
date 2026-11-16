/* Production sender functions are generated verbatim by test_dcc_sender_native.py.
 * Loopback only; the supplied file is opened read-only, never offered over IRC. */
#include <winsock2.h>
#include <windows.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include "../../src/common/fabulor.h"
#include "../../src/common/fe.h"
#include "../../src/common/dcc.h"
#undef printf
#undef sprintf
#define lseek _lseeki64
#define read _read
#define EMIT_SIGNAL(...) ((void)0)
struct fabulorprefs prefs;
static GMainLoop *loop;
static struct DCC d;
static guint64 received;
static GChecksum *checksum;
static const char *mode;
static gboolean expired;
static int sock_error(void){return WSAGetLastError();}
static int would_block(void){return WSAGetLastError()==WSAEWOULDBLOCK;}
static const char *errorstring(int e){(void)e;return "test socket error";}
static void dcc_calc_average_cps(struct DCC *x){(void)x;}
static void dcc_close(struct DCC *x,enum dcc_state s,int destroy){
 (void)destroy;if(x->iotag)fe_input_remove(x->iotag);x->iotag=0;x->send_watch_flags=0;
 closesocket(x->sok);x->sok=-1;_close(x->fp);x->fp=-1;x->dccstat=s;g_main_loop_quit(loop);
}
static guint fragmented_reads, send_calls;
static gboolean throttled_once;
static int observed_recv(SOCKET s,char *buf,int len,int flags){
 int n=recv(s,buf,!strcmp(mode,"partial")?MIN(len,1):len,flags);if(n>0&&d.ack_pos>0)++fragmented_reads;return n;
}
static int observed_send(SOCKET s,const char *buf,int len,int flags){
 if(!strcmp(mode,"partial")){
  if(++send_calls%5==0){WSASetLastError(WSAEWOULDBLOCK);return -1;}
  len=MIN(len,137);
 }
 {int n=send(s,buf,len,flags);
  if(n>0&&!strcmp(mode,"throttle")&&!throttled_once){d.throttled=1;throttled_once=TRUE;}
  return n;}
}
#define send observed_send
#define recv observed_recv
#include "dcc-send-under-test.inc"
#undef recv
#undef send
static gpointer receiver(gpointer p){
 SOCKET s=*(SOCKET*)p;char buf[8192];DWORD timeout=60000;int n,buffer_bytes=4096;
 setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof timeout);
 setsockopt(s,SOL_SOCKET,SO_SNDBUF,(char*)&buffer_bytes,sizeof buffer_bytes);
 if(!strcmp(mode,"slow")){int yes=1;setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char*)&yes,sizeof yes);}
 while(received<d.size){guint32 ack;
  n=recv(s,buf,(int)MIN((guint64)((!strcmp(mode,"resume-offset")&&received==d.resumable)?1024:sizeof buf),d.size-received),0);if(n<=0)break;
  g_checksum_update(checksum,(guchar*)buf,n);received+=n;
  if(!strcmp(mode,"peer-close"))break;
  ack=htonl((guint32)(received-(!strcmp(mode,"resume-offset")?d.resumable:0)));
  /* Fragment ACK writes deliberately; TCP is permitted to coalesce them. */
  if(send(s,(char*)&ack,1,0)!=1)break;
  if(!strcmp(mode,"slow"))g_usleep(2000);
  if(send(s,((char*)&ack)+1,3,0)!=3)break;
 }
 closesocket(s);return NULL;
}
static gboolean timer(gpointer unused){
 (void)unused;
 if(!strcmp(mode,"cancel")){dcc_close(&d,STAT_ABORTED,0);return G_SOURCE_REMOVE;}
 if(!strcmp(mode,"throttle")){
  g_assert_cmpint(d.send_watch_flags & FIA_WRITE,==,0);
  g_assert_true(d.pos>0&&d.pos<d.size);
  d.throttled=0;dcc_send_data(NULL,0,&d);return G_SOURCE_REMOVE;
 }
 expired=TRUE;dcc_close(&d,STAT_FAILED,0);return G_SOURCE_REMOVE;
}
static gboolean deadline_expired(gpointer unused){
 (void)unused;expired=TRUE;dcc_close(&d,STAT_FAILED,0);return G_SOURCE_REMOVE;
}
int main(int argc,char **argv){
 WSADATA w;SOCKET listener,peer;struct sockaddr_in a={0};int len=sizeof a;u_long nb=1;GThread*t;
 char b[16384],*expected;GChecksum*hash;guint deadline,action=0;guint64 remaining;int n;
 if(argc!=3)return 2;mode=argv[2];if(WSAStartup(MAKEWORD(2,2),&w))return 2;
 d.fp=g_open(argv[1],_O_RDONLY|_O_BINARY,0);g_assert_cmpint(d.fp,>=,0);
 d.size=(guint64)_lseeki64(d.fp,0,SEEK_END);_lseeki64(d.fp,0,SEEK_SET);
 if(strcmp(mode,"fast"))d.size=MIN(d.size,65536);
 if(!strncmp(mode,"resume",6)){d.pos=d.resumable=d.ack=4096;received=4096;_lseeki64(d.fp,4096,SEEK_SET);}
 hash=g_checksum_new(G_CHECKSUM_SHA256);remaining=d.size-d.pos;
 while(remaining){n=_read(d.fp,b,(unsigned)MIN(remaining,sizeof b));g_assert_cmpint(n,>,0);g_checksum_update(hash,(guchar*)b,n);remaining-=n;}
 expected=g_strdup(g_checksum_get_string(hash));g_checksum_free(hash);
 listener=socket(AF_INET,SOCK_STREAM,0);a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
 g_assert_cmpint(bind(listener,(struct sockaddr*)&a,sizeof a),==,0);g_assert_cmpint(listen(listener,1),==,0);
 g_assert_cmpint(getsockname(listener,(struct sockaddr*)&a,&len),==,0);
 peer=socket(AF_INET,SOCK_STREAM,0);g_assert_cmpint(connect(peer,(struct sockaddr*)&a,sizeof a),==,0);
 d.sok=(int)accept(listener,NULL,NULL);closesocket(listener);ioctlsocket(d.sok,FIONBIO,&nb);
 d.type=TYPE_SEND;d.dccstat=STAT_ACTIVE;d.fastsend=strcmp(mode,"slow")!=0;prefs.hex_dcc_blocksize=1024;
 loop=g_main_loop_new(NULL,FALSE);checksum=g_checksum_new(G_CHECKSUM_SHA256);
 t=g_thread_new("DCC-test-peer",receiver,&peer);
 dcc_send_watch_update(&d);
 if(!strcmp(mode,"throttle"))action=g_timeout_add(50,timer,NULL);
 if(!strcmp(mode,"cancel")){d.throttled=1;dcc_send_watch_update(&d);action=g_timeout_add(25,timer,NULL);}
 else dcc_send_data(NULL,0,&d);
 deadline=g_timeout_add(60000,deadline_expired,NULL);g_main_loop_run(loop);
 if(g_main_context_find_source_by_id(NULL,deadline))g_source_remove(deadline);
 if(action&&g_main_context_find_source_by_id(NULL,action))g_source_remove(action);
 g_thread_join(t);
 g_assert_false(expired);g_assert_cmpint(d.iotag,==,0);g_assert_cmpint(d.wiotag,==,0);
 if(!strcmp(mode,"cancel"))g_assert_cmpint(d.dccstat,==,STAT_ABORTED);
 else if(!strcmp(mode,"peer-close"))g_assert_cmpint(d.dccstat,==,STAT_FAILED);
 else{g_assert_cmpint(d.dccstat,==,STAT_DONE);g_assert_true(received==d.size);g_assert_true(d.pos==d.size);g_assert_cmpstr(g_checksum_get_string(checksum),==,expected);}
 if(!strcmp(mode,"slow"))g_assert_cmpuint(fragmented_reads,>,0);
 printf("PASS %s sent=%llu received=%llu ACK=%llu fragmented-reads=%u sources-removed=yes\n",mode,d.pos,received,d.ack,fragmented_reads);
 g_free(expected);g_checksum_free(checksum);g_main_loop_unref(loop);WSACleanup();return 0;
}

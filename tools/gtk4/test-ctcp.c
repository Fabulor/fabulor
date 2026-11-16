/* Offline CTCP production-function regression. No IRC, WMI or profile access.
 * System-info, ignore/custom reply, events and action/DCC dispatch are inert. */
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "fabulor.h"
#include "proto-irc.h"
#include "ignore.h"
#undef printf
static guint replies, actions, dccs, events, cpu_queries, os_queries;
static char reply[1024], recipient[64];
static const char *cpu;
struct fabulorprefs prefs;
#define EMIT_SIGNAL_TIMESTAMP(...) (++events)
static char *sysinfo_get_os(void){++os_queries;return g_strdup("Windows test (x64)");}
static char *sysinfo_get_cpu(void){++cpu_queries;return g_strdup(cpu);}
static int get_cpu_arch(void){return 64;}
int ignore_check(char *s,int type){(void)s;(void)type;return 0;}
static int is_channel(server*s,char *to){(void)s;return to[0]=='#';}
static session *find_channel(server*s,char *to){(void)s;(void)to;return NULL;}
static int ctcp_check(session*s,char*n,char**w,char**e,char*c){(void)s;(void)n;(void)w;(void)e;(void)c;return 0;}
static void handle_dcc(session*s,char*n,char**w,char**e,const message_tags_data*t){(void)s;(void)n;(void)w;(void)e;(void)t;++dccs;}
static void inbound_action(session*s,char*to,char*n,char*ip,char*msg,int fromme,int id,const message_tags_data*t){(void)s;(void)to;(void)n;(void)ip;(void)msg;(void)fromme;(void)id;(void)t;++actions;}
static void sound_play(char*p,int quiet){(void)p;(void)quiet;}
static void capture(server*s,char*nick,char*text){(void)s;g_strlcpy(recipient,nick,sizeof recipient);g_strlcpy(reply,text,sizeof reply);++replies;}
#include "ctcp-under-test.inc"
int main(int argc,char**argv){
 server s={0};session sess={0};message_tags_data tags={0};char *words[33]={0},*eols[33]={0};
 char msg[160]="VERSION",word[] = ":\001VERSION",empty[]="";
 char *nick="Barry",*to="Alchera";guint expected_replies=1,expected_actions=0,expected_dccs=0,expected_events=1;int i;
 const char *mode;char *expected;
 if(argc!=2)return 2;mode=argv[1];cpu="Test CPU";
 if(!strcmp(mode,"missing-cpu"))cpu=NULL;
 if(!strcmp(mode,"empty-cpu"))cpu="";
 if(!strcmp(mode,"cache")){
  char *first=get_sys_str(1);g_assert_true(first==get_sys_str(1));
  g_assert_cmpstr(first,==,"Windows test (x64) [Test CPU]");
  g_assert_cmpuint(cpu_queries,==,1);g_assert_cmpuint(os_queries,==,1);
  printf("PASS cache\n");return 0;
 }
 if(!strcmp(mode,"no-cpu")){
  char *first=get_sys_str(0);g_assert_true(first==get_sys_str(0));
  g_assert_cmpstr(first,==,"Windows test (x64)");
  g_assert_cmpuint(cpu_queries,==,0);g_assert_cmpuint(os_queries,==,1);
  printf("PASS no-cpu\n");return 0;
 }
 g_strlcpy(s.nick,"Alchera",sizeof s.nick);s.have_echo_message=strcmp(mode,"peer-no-echo")&&strcmp(mode,"own-no-echo");
 s.p_cmp=g_ascii_strcasecmp;s.p_nctcp=capture;s.front_session=&sess;sess.server=&s;
 if(!strcmp(mode,"own-echo")||!strcmp(mode,"own-case")||!strcmp(mode,"channel-echo")||!strcmp(mode,"dcc-echo")){
  nick=!strcmp(mode,"own-case")?"ALCHERA":"Alchera";to=!strcmp(mode,"channel-echo")?"#test":"Barry";
  expected_replies=expected_events=0;
 }
 if(!strcmp(mode,"self"))nick="Alchera";
 if(!strcmp(mode,"own-no-echo")){nick="Alchera";to="Barry";}
 if(!strcmp(mode,"action-echo")||!strcmp(mode,"peer-action")){
  if(!strcmp(mode,"action-echo")){nick="Alchera";to="Barry";}
  g_strlcpy(msg,"ACTION waves",sizeof msg);expected_replies=expected_events=0;expected_actions=1;
 }
 if(!strcmp(mode,"dcc-echo")||!strcmp(mode,"peer-dcc")){
  g_strlcpy(msg,"DCC SEND test.zip 2130706433 50000 1",sizeof msg);
  expected_replies=expected_events=0;expected_dccs=!strcmp(mode,"peer-dcc");
 }
 if(!strcmp(mode,"hidden")){prefs.hex_irc_hide_version=1;expected_replies=0;}
 for(i=0;i<33;i++)words[i]=eols[i]=empty;
 words[1]="offline@test.invalid";words[4]=word;
 ctcp_handle(&sess,to,nick,"offline@test.invalid",msg,words,eols,0,&tags);
 g_assert_cmpuint(replies,==,expected_replies);g_assert_cmpuint(actions,==,expected_actions);
 g_assert_cmpuint(dccs,==,expected_dccs);g_assert_cmpuint(events,==,expected_events);
 if(expected_replies){
  expected=g_strconcat("VERSION Fabulor "PACKAGE_VERSION" [x64] / Windows test (x64)",cpu&&cpu[0]?" [Test CPU]":"",NULL);
  g_assert_cmpstr(reply,==,expected);g_assert_cmpstr(recipient,==,nick);g_free(expected);
 }
 printf("PASS %s replies=%u actions=%u DCC=%u events=%u\n",mode,replies,actions,dccs,events);return 0;
}

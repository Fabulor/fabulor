/* Fabulor
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

/* You can distribute this header with your plugins for easy compilation */
#ifndef FABULOR_PLUGIN_H
#define FABULOR_PLUGIN_H

#include <time.h>

#define FABULOR_PRI_HIGHEST	127
#define FABULOR_PRI_HIGH		64
#define FABULOR_PRI_NORM		0
#define FABULOR_PRI_LOW		(-64)
#define FABULOR_PRI_LOWEST	(-128)

#define FABULOR_FD_READ		1
#define FABULOR_FD_WRITE		2
#define FABULOR_FD_EXCEPTION	4
#define FABULOR_FD_NOTSOCKET	8

#define FABULOR_EAT_NONE		0	/* pass it on through! */
#define FABULOR_EAT_FABULOR		1	/* don't let Fabulor see this event */
#define FABULOR_EAT_PLUGIN	2	/* don't let other plugins see this event */
#define FABULOR_EAT_ALL		(FABULOR_EAT_FABULOR|FABULOR_EAT_PLUGIN)	/* don't let anything see this event */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fabulor_plugin fabulor_plugin;
typedef struct _fabulor_list fabulor_list;
typedef struct _fabulor_hook fabulor_hook;
#ifndef PLUGIN_C
typedef struct _fabulor_context fabulor_context;
#endif
typedef struct
{
	time_t server_time_utc; /* 0 if not used */
} fabulor_event_attrs;

#ifndef PLUGIN_C
struct _fabulor_plugin
{
	/* these are only used on win32 */
	fabulor_hook *(*fabulor_hook_command) (fabulor_plugin *ph,
		    const char *name,
		    int pri,
		    int (*callback) (char *word[], char *word_eol[], void *user_data),
		    const char *help_text,
		    void *userdata);
	fabulor_hook *(*fabulor_hook_server) (fabulor_plugin *ph,
		   const char *name,
		   int pri,
		   int (*callback) (char *word[], char *word_eol[], void *user_data),
		   void *userdata);
	fabulor_hook *(*fabulor_hook_print) (fabulor_plugin *ph,
		  const char *name,
		  int pri,
		  int (*callback) (char *word[], void *user_data),
		  void *userdata);
	fabulor_hook *(*fabulor_hook_timer) (fabulor_plugin *ph,
		  int timeout,
		  int (*callback) (void *user_data),
		  void *userdata);
	fabulor_hook *(*fabulor_hook_fd) (fabulor_plugin *ph,
		   int fd,
		   int flags,
		   int (*callback) (int fd, int flags, void *user_data),
		   void *userdata);
	void *(*fabulor_unhook) (fabulor_plugin *ph,
	      fabulor_hook *hook);
	void (*fabulor_print) (fabulor_plugin *ph,
	     const char *text);
	void (*fabulor_printf) (fabulor_plugin *ph,
	      const char *format, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 3)))
#endif
	;
	void (*fabulor_command) (fabulor_plugin *ph,
	       const char *command);
	void (*fabulor_commandf) (fabulor_plugin *ph,
		const char *format, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 3)))
#endif
	;
	int (*fabulor_nickcmp) (fabulor_plugin *ph,
	       const char *s1,
	       const char *s2);
	int (*fabulor_set_context) (fabulor_plugin *ph,
		   fabulor_context *ctx);
	fabulor_context *(*fabulor_find_context) (fabulor_plugin *ph,
		    const char *servname,
		    const char *channel);
	fabulor_context *(*fabulor_get_context) (fabulor_plugin *ph);
	const char *(*fabulor_get_info) (fabulor_plugin *ph,
		const char *id);
	int (*fabulor_get_prefs) (fabulor_plugin *ph,
		 const char *name,
		 const char **string,
		 int *integer);
	fabulor_list * (*fabulor_list_get) (fabulor_plugin *ph,
		const char *name);
	void (*fabulor_list_free) (fabulor_plugin *ph,
		 fabulor_list *xlist);
	const char * const * (*fabulor_list_fields) (fabulor_plugin *ph,
		   const char *name);
	int (*fabulor_list_next) (fabulor_plugin *ph,
		 fabulor_list *xlist);
	const char * (*fabulor_list_str) (fabulor_plugin *ph,
		fabulor_list *xlist,
		const char *name);
	int (*fabulor_list_int) (fabulor_plugin *ph,
		fabulor_list *xlist,
		const char *name);
	void * (*fabulor_plugingui_add) (fabulor_plugin *ph,
		     const char *filename,
		     const char *name,
		     const char *desc,
		     const char *version,
		     char *reserved);
	void (*fabulor_plugingui_remove) (fabulor_plugin *ph,
			void *handle);
	int (*fabulor_emit_print) (fabulor_plugin *ph,
			const char *event_name, ...);
	int (*fabulor_read_fd) (fabulor_plugin *ph,
			void *src,
			char *buf,
			int *len);
	time_t (*fabulor_list_time) (fabulor_plugin *ph,
		fabulor_list *xlist,
		const char *name);
	char *(*fabulor_gettext) (fabulor_plugin *ph,
		const char *msgid);
	void (*fabulor_send_modes) (fabulor_plugin *ph,
		  const char **targets,
		  int ntargets,
		  int modes_per_line,
		  char sign,
		  char mode);
	char *(*fabulor_strip) (fabulor_plugin *ph,
	     const char *str,
	     int len,
	     int flags);
	void (*fabulor_free) (fabulor_plugin *ph,
	    void *ptr);
	int (*fabulor_pluginpref_set_str) (fabulor_plugin *ph,
		const char *var,
		const char *value);
	int (*fabulor_pluginpref_get_str) (fabulor_plugin *ph,
		const char *var,
		char *dest);
	int (*fabulor_pluginpref_set_int) (fabulor_plugin *ph,
		const char *var,
		int value);
	int (*fabulor_pluginpref_get_int) (fabulor_plugin *ph,
		const char *var);
	int (*fabulor_pluginpref_delete) (fabulor_plugin *ph,
		const char *var);
	int (*fabulor_pluginpref_list) (fabulor_plugin *ph,
		char *dest);
	fabulor_hook *(*fabulor_hook_server_attrs) (fabulor_plugin *ph,
		   const char *name,
		   int pri,
		   int (*callback) (char *word[], char *word_eol[],
							fabulor_event_attrs *attrs, void *user_data),
		   void *userdata);
	fabulor_hook *(*fabulor_hook_print_attrs) (fabulor_plugin *ph,
		  const char *name,
		  int pri,
		  int (*callback) (char *word[], fabulor_event_attrs *attrs,
						   void *user_data),
		  void *userdata);
	int (*fabulor_emit_print_attrs) (fabulor_plugin *ph, fabulor_event_attrs *attrs,
									 const char *event_name, ...);
	fabulor_event_attrs *(*fabulor_event_attrs_create) (fabulor_plugin *ph);
	void (*fabulor_event_attrs_free) (fabulor_plugin *ph,
									  fabulor_event_attrs *attrs);
};
#endif


fabulor_hook *
fabulor_hook_command (fabulor_plugin *ph,
		    const char *name,
		    int pri,
		    int (*callback) (char *word[], char *word_eol[], void *user_data),
		    const char *help_text,
		    void *userdata);

fabulor_event_attrs *fabulor_event_attrs_create (fabulor_plugin *ph);

void fabulor_event_attrs_free (fabulor_plugin *ph, fabulor_event_attrs *attrs);

fabulor_hook *
fabulor_hook_server (fabulor_plugin *ph,
		   const char *name,
		   int pri,
		   int (*callback) (char *word[], char *word_eol[], void *user_data),
		   void *userdata);

fabulor_hook *
fabulor_hook_server_attrs (fabulor_plugin *ph,
		   const char *name,
		   int pri,
		   int (*callback) (char *word[], char *word_eol[],
							fabulor_event_attrs *attrs, void *user_data),
		   void *userdata);

fabulor_hook *
fabulor_hook_print (fabulor_plugin *ph,
		  const char *name,
		  int pri,
		  int (*callback) (char *word[], void *user_data),
		  void *userdata);

fabulor_hook *
fabulor_hook_print_attrs (fabulor_plugin *ph,
		  const char *name,
		  int pri,
		  int (*callback) (char *word[], fabulor_event_attrs *attrs,
						   void *user_data),
		  void *userdata);

fabulor_hook *
fabulor_hook_timer (fabulor_plugin *ph,
		  int timeout,
		  int (*callback) (void *user_data),
		  void *userdata);

fabulor_hook *
fabulor_hook_fd (fabulor_plugin *ph,
		int fd,
		int flags,
		int (*callback) (int fd, int flags, void *user_data),
		void *userdata);

void *
fabulor_unhook (fabulor_plugin *ph,
	      fabulor_hook *hook);

void
fabulor_print (fabulor_plugin *ph,
	     const char *text);

void
fabulor_printf (fabulor_plugin *ph,
	      const char *format, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 3)))
#endif
;

void
fabulor_command (fabulor_plugin *ph,
	       const char *command);

void
fabulor_commandf (fabulor_plugin *ph,
		const char *format, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 3)))
#endif
;

int
fabulor_nickcmp (fabulor_plugin *ph,
	       const char *s1,
	       const char *s2);

int
fabulor_set_context (fabulor_plugin *ph,
		   fabulor_context *ctx);

fabulor_context *
fabulor_find_context (fabulor_plugin *ph,
		    const char *servname,
		    const char *channel);

fabulor_context *
fabulor_get_context (fabulor_plugin *ph);

const char *
fabulor_get_info (fabulor_plugin *ph,
		const char *id);

int
fabulor_get_prefs (fabulor_plugin *ph,
		 const char *name,
		 const char **string,
		 int *integer);

fabulor_list *
fabulor_list_get (fabulor_plugin *ph,
		const char *name);

void
fabulor_list_free (fabulor_plugin *ph,
		 fabulor_list *xlist);

const char * const *
fabulor_list_fields (fabulor_plugin *ph,
		   const char *name);

int
fabulor_list_next (fabulor_plugin *ph,
		 fabulor_list *xlist);

const char *
fabulor_list_str (fabulor_plugin *ph,
		fabulor_list *xlist,
		const char *name);

int
fabulor_list_int (fabulor_plugin *ph,
		fabulor_list *xlist,
		const char *name);

time_t
fabulor_list_time (fabulor_plugin *ph,
		 fabulor_list *xlist,
		 const char *name);

void *
fabulor_plugingui_add (fabulor_plugin *ph,
		     const char *filename,
		     const char *name,
		     const char *desc,
		     const char *version,
		     char *reserved);

void
fabulor_plugingui_remove (fabulor_plugin *ph,
			void *handle);

int
fabulor_emit_print (fabulor_plugin *ph,
		  const char *event_name, ...);

int
fabulor_emit_print_attrs (fabulor_plugin *ph, fabulor_event_attrs *attrs,
						  const char *event_name, ...);

char *
fabulor_gettext (fabulor_plugin *ph,
	       const char *msgid);

void
fabulor_send_modes (fabulor_plugin *ph,
		  const char **targets,
		  int ntargets,
		  int modes_per_line,
		  char sign,
		  char mode);

char *
fabulor_strip (fabulor_plugin *ph,
	     const char *str,
	     int len,
	     int flags);

void
fabulor_free (fabulor_plugin *ph,
	    void *ptr);

int
fabulor_pluginpref_set_str (fabulor_plugin *ph,
		const char *var,
		const char *value);

int
fabulor_pluginpref_get_str (fabulor_plugin *ph,
		const char *var,
		char *dest);

int
fabulor_pluginpref_set_int (fabulor_plugin *ph,
		const char *var,
		int value);
int
fabulor_pluginpref_get_int (fabulor_plugin *ph,
		const char *var);

int
fabulor_pluginpref_delete (fabulor_plugin *ph,
		const char *var);

int
fabulor_pluginpref_list (fabulor_plugin *ph,
		char *dest);

#if !defined(PLUGIN_C) && (defined(WIN32) || defined(__CYGWIN__))
#ifndef FABULOR_PLUGIN_HANDLE
#define FABULOR_PLUGIN_HANDLE (ph)
#endif
#define fabulor_hook_command ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_command)
#define fabulor_event_attrs_create ((FABULOR_PLUGIN_HANDLE)->fabulor_event_attrs_create)
#define fabulor_event_attrs_free ((FABULOR_PLUGIN_HANDLE)->fabulor_event_attrs_free)
#define fabulor_hook_server ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_server)
#define fabulor_hook_server_attrs ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_server_attrs)
#define fabulor_hook_print ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_print)
#define fabulor_hook_print_attrs ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_print_attrs)
#define fabulor_hook_timer ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_timer)
#define fabulor_hook_fd ((FABULOR_PLUGIN_HANDLE)->fabulor_hook_fd)
#define fabulor_unhook ((FABULOR_PLUGIN_HANDLE)->fabulor_unhook)
#define fabulor_print ((FABULOR_PLUGIN_HANDLE)->fabulor_print)
#define fabulor_printf ((FABULOR_PLUGIN_HANDLE)->fabulor_printf)
#define fabulor_command ((FABULOR_PLUGIN_HANDLE)->fabulor_command)
#define fabulor_commandf ((FABULOR_PLUGIN_HANDLE)->fabulor_commandf)
#define fabulor_nickcmp ((FABULOR_PLUGIN_HANDLE)->fabulor_nickcmp)
#define fabulor_set_context ((FABULOR_PLUGIN_HANDLE)->fabulor_set_context)
#define fabulor_find_context ((FABULOR_PLUGIN_HANDLE)->fabulor_find_context)
#define fabulor_get_context ((FABULOR_PLUGIN_HANDLE)->fabulor_get_context)
#define fabulor_get_info ((FABULOR_PLUGIN_HANDLE)->fabulor_get_info)
#define fabulor_get_prefs ((FABULOR_PLUGIN_HANDLE)->fabulor_get_prefs)
#define fabulor_list_get ((FABULOR_PLUGIN_HANDLE)->fabulor_list_get)
#define fabulor_list_free ((FABULOR_PLUGIN_HANDLE)->fabulor_list_free)
#define fabulor_list_fields ((FABULOR_PLUGIN_HANDLE)->fabulor_list_fields)
#define fabulor_list_next ((FABULOR_PLUGIN_HANDLE)->fabulor_list_next)
#define fabulor_list_str ((FABULOR_PLUGIN_HANDLE)->fabulor_list_str)
#define fabulor_list_int ((FABULOR_PLUGIN_HANDLE)->fabulor_list_int)
#define fabulor_plugingui_add ((FABULOR_PLUGIN_HANDLE)->fabulor_plugingui_add)
#define fabulor_plugingui_remove ((FABULOR_PLUGIN_HANDLE)->fabulor_plugingui_remove)
#define fabulor_emit_print ((FABULOR_PLUGIN_HANDLE)->fabulor_emit_print)
#define fabulor_emit_print_attrs ((FABULOR_PLUGIN_HANDLE)->fabulor_emit_print_attrs)
#define fabulor_list_time ((FABULOR_PLUGIN_HANDLE)->fabulor_list_time)
#define fabulor_gettext ((FABULOR_PLUGIN_HANDLE)->fabulor_gettext)
#define fabulor_send_modes ((FABULOR_PLUGIN_HANDLE)->fabulor_send_modes)
#define fabulor_strip ((FABULOR_PLUGIN_HANDLE)->fabulor_strip)
#define fabulor_free ((FABULOR_PLUGIN_HANDLE)->fabulor_free)
#define fabulor_pluginpref_set_str ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_set_str)
#define fabulor_pluginpref_get_str ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_get_str)
#define fabulor_pluginpref_set_int ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_set_int)
#define fabulor_pluginpref_get_int ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_get_int)
#define fabulor_pluginpref_delete ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_delete)
#define fabulor_pluginpref_list ((FABULOR_PLUGIN_HANDLE)->fabulor_pluginpref_list)
#endif

#ifdef __cplusplus
}
#endif
#endif

/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef FABULOR_COMMONPLUGIN_H
#define FABULOR_COMMONPLUGIN_H

#ifdef PLUGIN_C
struct _fabulor_plugin
{
	/* Keep these in sync with fabulor-plugin.h */
	/* !!don't change the order, to keep binary compat!! */
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
	      const char *format, ...);
	void (*fabulor_command) (fabulor_plugin *ph,
	       const char *command);
	void (*fabulor_commandf) (fabulor_plugin *ph,
		const char *format, ...);
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
	void *(*fabulor_read_fd) (fabulor_plugin *ph);
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

	/* PRIVATE FIELDS! */
	void *handle;		/* from dlopen */
	char *filename;	/* loaded from */
	char *name;
	char *desc;
	char *version;
	session *context;
	void *deinit_callback;	/* pointer to fabulor_plugin_deinit */
	unsigned int fake:1;		/* fake plugin. Added by fabulor_plugingui_add() */
	unsigned int free_strings:1;		/* free name,desc,version? */
};
#endif

GModule *module_load (char *filename);
char *plugin_load (session *sess, char *filename, char *arg);
int plugin_reload (session *sess, char *name, int by_filename);
void plugin_add (session *sess, char *filename, void *handle, void *init_func, void *deinit_func, char *arg, int fake);
int plugin_kill (char *name, int by_filename);
void plugin_kill_all (void);
void plugin_auto_load (session *sess);
void plugin_print_startup_report (session *sess);
int plugin_emit_command (session *sess, char *name, char *word[], char *word_eol[]);
int plugin_emit_server (session *sess, char *name, char *word[], char *word_eol[],
						time_t server_time);
int plugin_emit_print (session *sess, char *word[], time_t server_time);
int plugin_emit_dummy_print (session *sess, char *name);
int plugin_emit_keypress (session *sess, unsigned int state, unsigned int keyval, gunichar key);
GList* plugin_command_list(GList *tmp_list);
int plugin_show_help (session *sess, char *cmd);
void plugin_command_foreach (session *sess, void *userdata, void (*cb) (session *sess, void *userdata, char *name, char *usage));
session *plugin_find_context (const char *servname, const char *channel, server *current_server);
void plugin_rebind_context (session *retired, session *replacement);

#define PLUGIN_SUFFIX G_MODULE_SUFFIX

#endif

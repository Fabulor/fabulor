/* X-Chat
 * Copyright (C) 2004-2008 Peter Zelezny.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>

#include "../common/zoitechat.h"
#ifdef USE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif
#include "../common/zoitechatc.h"
#include "../common/servlist.h"
#include "../common/cfgfiles.h"
#include "../common/fe.h"
#include "../common/secretstore.h"
#include "../common/util.h"

#include "fe-gtk.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "file-chooser-path.h"
#include "window-geometry.h"
#include "server-network-list.h"
#include "server-entry-list.h"
#include "menu.h"
#include "pixmaps.h"
#include "fkeys.h"
#include "theme/theme-manager.h"

#define SERVLIST_X_PADDING 4			/* horizontal paddig in the network editor */
#define SERVLIST_Y_PADDING 0			/* vertical padding in the network editor */

#define ICON_SERVLIST_CONNECT "zc-menu-connect"
#define ICON_SERVLIST_ADD "list-add"
#define ICON_SERVLIST_REMOVE "list-remove"
#define ICON_SERVLIST_CLOSE "gtk-close"
#define ICON_SERVLIST_ERROR "dialog-error"

#ifdef USE_OPENSSL
# define DEFAULT_SERVER "newserver/6697"
#else
# define DEFAULT_SERVER "newserver/6667"
#endif

/* servlistgui.c globals */
static GtkWidget *serverlist_win = NULL;
static GtkWidget *networks_tree;		/* network TreeView */
static FabulorServerNetworkList *network_list_view = NULL;

static int netlist_win_width = 0;		/* don't hardcode pixels, just use as much as needed by default, save if resized */
static int netlist_win_height = 0;
static int netedit_win_width = 0;
static int netedit_win_height = 0;

static int netedit_active_tab = 0;

/* global user info */
static GtkWidget *entry_nick1;
static GtkWidget *entry_nick2;
static GtkWidget *entry_nick3;
static GtkWidget *entry_guser;
/* static GtkWidget *entry_greal; */

enum {
		SERVER_TREE,
		CHANNEL_TREE,
		CMD_TREE,
		N_TREES,
};

/* edit area */
static GtkWidget *edit_win;
static GtkWidget *edit_entry_nick;
static GtkWidget *edit_entry_nick2;
static GtkWidget *edit_entry_user;
static GtkWidget *edit_entry_real;
static GtkWidget *edit_entry_pass;
static GtkWidget *edit_check_show_pass;
static GtkWidget *edit_check_use_keyring;
static int edit_pass_changed;
static char *edit_loaded_password;
static GtkWidget *edit_label_nick;
static GtkWidget *edit_label_nick2;
static GtkWidget *edit_label_real;
static GtkWidget *edit_label_user;
static GtkWidget *edit_trees[N_TREES];
static FabulorServerEntryList *edit_lists[N_TREES];
static GtkWidget *edit_button_cert_import;
static GtkWidget *edit_button_cert_info;
static GtkWidget *edit_button_cert_delete;

typedef struct
{
	GWeakRef parent;
	gboolean parent_watch_active;
	char *network_name;
	char *cert_dir;
	char *cert_file;
} servlist_cert_import_data;

static ircnet *selected_net = NULL;
static session *servlist_sess;

static void servlist_network_row_cb (gpointer identity, gpointer user_data);
static GtkWidget *servlist_open_edit (GtkWidget *parent, ircnet *net);
static void servlist_password_changed_cb (GtkEditable *editable, gpointer userdata);
static void servlist_network_list_release (void);

static void
servlist_entry_set_text_silent (GtkWidget *entry, const char *text)
{
	g_signal_handlers_block_by_func (G_OBJECT (entry), G_CALLBACK (servlist_password_changed_cb), NULL);
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry), text);
	g_signal_handlers_unblock_by_func (G_OBJECT (entry), G_CALLBACK (servlist_password_changed_cb), NULL);
}

static char *
servlist_display_password (ircnet *net)
{
	if (!net)
		return NULL;
	if (edit_pass_changed)
		return g_strdup (fabulor_gtk_entry_get_text (GTK_ENTRY (edit_entry_pass)));
	if (edit_loaded_password)
		return g_strdup (edit_loaded_password);
	if (net->flags & FLAG_USE_KEYRING)
		return secretstore_get_network_password (net->name);
	return servlist_password_decrypt_for_storage (net->pass);
}

static void
servlist_toggle_show_password_cb (GtkWidget *toggle, gpointer userdata)
{
	if (fabulor_gtk_check_button_get_active (toggle))
	{
		char *password = servlist_display_password (selected_net);
		if (password)
		{
			if (edit_loaded_password)
			{
				memset (edit_loaded_password, 0, strlen (edit_loaded_password));
				g_free (edit_loaded_password);
			}
			edit_loaded_password = g_strdup (password);
			servlist_entry_set_text_silent (userdata, password);
			memset (password, 0, strlen (password));
			g_free (password);
		}
		gtk_entry_set_visibility (GTK_ENTRY (userdata), TRUE);
	}
	else
	{
		gtk_entry_set_visibility (GTK_ENTRY (userdata), FALSE);
		if (edit_loaded_password && !edit_pass_changed)
			servlist_entry_set_text_silent (userdata, "***");
	}
}


static void
servlist_password_changed_cb (GtkEditable *editable, gpointer userdata)
{
	edit_pass_changed = 1;
	if (edit_loaded_password && strcmp (fabulor_gtk_entry_get_text (GTK_ENTRY (editable)), "***"))
	{
		memset (edit_loaded_password, 0, strlen (edit_loaded_password));
		g_free (edit_loaded_password);
		edit_loaded_password = NULL;
	}
}

static char *
servlist_get_cert_file (ircnet *net)
{
	if (!net || !net->name || !net->name[0])
		return NULL;

	return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "certs" G_DIR_SEPARATOR_S "%s.pem",
								 get_xdir (), net->name);
}

static gboolean
servlist_network_cert_exists (ircnet *net)
{
	char *cert_file;
	gboolean exists;

	cert_file = servlist_get_cert_file (net);
	if (!cert_file)
		return FALSE;

	exists = g_file_test (cert_file, G_FILE_TEST_IS_REGULAR);
	g_free (cert_file);
	return exists;
}

static void
servlist_update_cert_buttons (ircnet *net)
{
	gboolean has_cert = servlist_network_cert_exists (net);

	if (edit_button_cert_import)
		gtk_widget_set_visible (edit_button_cert_import, !has_cert);
	if (edit_button_cert_info)
		gtk_widget_set_visible (edit_button_cert_info, has_cert);
	if (edit_button_cert_delete)
		gtk_widget_set_visible (edit_button_cert_delete, has_cert);
}

static void
servlist_cert_import_data_free (gpointer user_data)
{
	servlist_cert_import_data *data = user_data;

	if (!data)
		return;

	g_weak_ref_clear (&data->parent);
	g_free (data->network_name);
	g_free (data->cert_dir);
	g_free (data->cert_file);
	g_free (data);
}

static void
servlist_cert_import_parent_gone (GtkNativeDialog *dialog)
{
	servlist_cert_import_data *data;

	data = g_object_get_data (G_OBJECT (dialog), "fabulor-cert-import-data");
	if (data)
		data->parent_watch_active = FALSE;
	gtk_native_dialog_hide (dialog);
	g_object_unref (dialog);
}

static void
servlist_cert_import_parent_finalized_cb (gpointer user_data, GObject *parent)
{
	(void) parent;
	servlist_cert_import_parent_gone (GTK_NATIVE_DIALOG (user_data));
}

#ifdef USE_OPENSSL
static SSL_CTX *
servlist_open_client_cert_context (const char *cert_file, char **error_text)
{
	SSL_CTX *ctx;
	unsigned long error_code;
	char error_buffer[256];

	if (error_text)
		*error_text = NULL;

	ERR_clear_error ();
	ctx = SSL_CTX_new (TLS_client_method ());
	if (ctx &&
		 SSL_CTX_use_certificate_chain_file (ctx, cert_file) == 1 &&
		 SSL_CTX_use_PrivateKey_file (ctx, cert_file, SSL_FILETYPE_PEM) == 1 &&
		 SSL_CTX_check_private_key (ctx) == 1)
		return ctx;

	error_code = ERR_get_error ();
	if (error_text)
	{
		if (error_code)
		{
			ERR_error_string_n (error_code, error_buffer, sizeof (error_buffer));
			*error_text = g_strdup (error_buffer);
		}
		else
			*error_text = g_strdup (_("The file must contain a matching PEM client certificate and private key."));
	}

	if (ctx)
		SSL_CTX_free (ctx);
	return NULL;
}

static char *
servlist_format_certificate_time (const ASN1_TIME *time)
{
	BIO *bio;
	char *data;
	long length;
	char *formatted;

	bio = BIO_new (BIO_s_mem ());
	if (!bio)
		return g_strdup (_("Unknown"));
	if (ASN1_TIME_print (bio, time) != 1)
	{
		BIO_free (bio);
		return g_strdup (_("Unknown"));
	}

	length = BIO_get_mem_data (bio, &data);
	formatted = g_strndup (data, length > 0 ? (gsize)length : 0);
	BIO_free (bio);
	return formatted;
}
#endif

static void
servlist_cert_import_response_cb (GtkNativeDialog *dialog, gint response_id,
								 gpointer user_data)
{
	servlist_cert_import_data *data = user_data;
	GtkWindow *parent;
	GtkWidget *message;
	char *source_file = NULL;
	char *contents = NULL;
	char *validation_error = NULL;
	gsize length = 0;
	gboolean imported = FALSE;
#ifdef USE_OPENSSL
	SSL_CTX *validation_ctx = NULL;
	gboolean valid_certificate = FALSE;
#endif

	parent = g_weak_ref_get (&data->parent);
	if (parent)
	{
		if (data->parent_watch_active)
		{
			g_object_weak_unref (G_OBJECT (parent),
				servlist_cert_import_parent_finalized_cb, dialog);
			data->parent_watch_active = FALSE;
		}
	}

	if (parent && response_id == GTK_RESPONSE_ACCEPT)
	{
		source_file = fabulor_gtk_file_chooser_dup_filename (
			GTK_FILE_CHOOSER (dialog));
		if (source_file)
		{
#ifdef USE_OPENSSL
			validation_ctx = servlist_open_client_cert_context (source_file,
																		  &validation_error);
			valid_certificate = validation_ctx != NULL;
			if (validation_ctx)
				SSL_CTX_free (validation_ctx);

			if (valid_certificate &&
				g_mkdir_with_parents (data->cert_dir, 0700) == 0 &&
				g_file_get_contents (source_file, &contents, &length, NULL) &&
				g_file_set_contents (data->cert_file, contents, length, NULL))
			{
				g_chmod (data->cert_file, 0600);
				imported = TRUE;
				if (edit_win == GTK_WIDGET (parent) && selected_net &&
					selected_net->name && !strcmp (selected_net->name, data->network_name))
					servlist_update_cert_buttons (selected_net);
			}
#else
			validation_error = g_strdup (_("TLS support is unavailable in this build."));
#endif

			message = gtk_message_dialog_new (parent,
											 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
											 imported ? GTK_MESSAGE_INFO : GTK_MESSAGE_ERROR,
											 GTK_BUTTONS_CLOSE,
											 imported ?
											 _("Client certificate imported for \"%s\".") :
											 _("Failed to import client certificate for \"%s\"."),
											 data->network_name);
			if (!imported && validation_error)
				gtk_message_dialog_format_secondary_text (
					GTK_MESSAGE_DIALOG (message), "%s", validation_error);
			theme_manager_attach_window (message);
			g_signal_connect (message, "response",
				G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
			gtk_widget_show (message);
		}
	}

	g_free (validation_error);
	g_free (contents);
	g_free (source_file);
	g_clear_object (&parent);
	g_object_unref (dialog);
}

static void
servlist_import_client_cert_cb (GtkWidget *button, gpointer userdata)
{
	ircnet *net = (ircnet *)userdata;
	GtkFileChooserNative *dialog;
	GtkWindow *parent;
	GtkFileFilter *filter;
	servlist_cert_import_data *data;

	(void) button;

	if (!net || !net->name || !net->name[0])
		return;
	parent = GTK_WINDOW (edit_win);
	if (!GTK_IS_WINDOW (parent))
		return;

	data = g_new0 (servlist_cert_import_data, 1);
	g_weak_ref_init (&data->parent, parent);
	data->network_name = g_strdup (net->name);
	data->cert_dir = g_build_filename (get_xdir (), "certs", NULL);
	data->cert_file = servlist_get_cert_file (net);
	if (!data->cert_file)
	{
		servlist_cert_import_data_free (data);
		return;
	}

	dialog = gtk_file_chooser_native_new (_("Import Client Certificate"),
											 parent,
											 GTK_FILE_CHOOSER_ACTION_OPEN,
											 _("_Open"), _("_Cancel"));
	gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (dialog), TRUE);
	fabulor_gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("PEM client certificate and private key"));
	gtk_file_filter_add_pattern (filter, "*.pem");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	g_object_set_data_full (G_OBJECT (dialog), "fabulor-cert-import-data",
								data, servlist_cert_import_data_free);
	data->parent_watch_active = TRUE;
	g_object_weak_ref (G_OBJECT (parent),
		servlist_cert_import_parent_finalized_cb, dialog);
	g_signal_connect (dialog, "response",
		G_CALLBACK (servlist_cert_import_response_cb), data);
	gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
}

static void
servlist_cert_info_cb (GtkWidget *button, gpointer userdata)
{
#ifdef USE_OPENSSL
	ircnet *net = (ircnet *)userdata;
	GtkWidget *dialog;
	char *cert_file;
	char *error_text = NULL;
	char *subject = NULL;
	char *issuer = NULL;
	char *not_before = NULL;
	char *not_after = NULL;
	char *details = NULL;
	SSL_CTX *ctx;
	X509 *cert;
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int digest_length = 0;
	GString *fingerprint;
	unsigned int i;

	cert_file = servlist_get_cert_file (net);
	if (!cert_file)
		return;

	ctx = servlist_open_client_cert_context (cert_file, &error_text);
	cert = ctx ? SSL_CTX_get0_certificate (ctx) : NULL;
	if (cert)
	{
		subject = X509_NAME_oneline (X509_get_subject_name (cert), NULL, 0);
		issuer = X509_NAME_oneline (X509_get_issuer_name (cert), NULL, 0);
		not_before = servlist_format_certificate_time (X509_get0_notBefore (cert));
		not_after = servlist_format_certificate_time (X509_get0_notAfter (cert));
		fingerprint = g_string_new (NULL);
		if (X509_digest (cert, EVP_sha256 (), digest, &digest_length) == 1)
		{
			for (i = 0; i < digest_length; i++)
				g_string_append_printf (fingerprint, "%s%02X",
											 i ? ":" : "", digest[i]);
		}
		details = g_strdup_printf (
			_("Subject: %s\nIssuer: %s\nValid from: %s\nValid until: %s\nSHA-256 fingerprint: %s"),
			subject ? subject : _("Unknown"),
			issuer ? issuer : _("Unknown"),
			not_before, not_after,
			fingerprint->len ? fingerprint->str : _("Unknown"));
		g_string_free (fingerprint, TRUE);

		dialog = gtk_message_dialog_new (GTK_WINDOW (edit_win),
												 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
												 GTK_MESSAGE_INFO,
												 GTK_BUTTONS_CLOSE,
												 _("Client certificate information for \"%s\"."),
												 net->name);
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", details);
	}
	else
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW (edit_win),
												 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
												 GTK_MESSAGE_ERROR,
												 GTK_BUTTONS_CLOSE,
												 _("Failed to read client certificate information for \"%s\"."),
												 net->name);
		if (error_text)
			gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog), "%s", error_text);
	}

	theme_manager_attach_window (dialog);
	g_signal_connect (dialog, "response",
		G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
	gtk_widget_show (dialog);
	if (ctx)
		SSL_CTX_free (ctx);
	if (subject)
		OPENSSL_free (subject);
	if (issuer)
		OPENSSL_free (issuer);
	g_free (not_before);
	g_free (not_after);
	g_free (details);
	g_free (error_text);
	g_free (cert_file);
#else
	return;
#endif
}

static void
servlist_delete_client_cert_cb (GtkWidget *button, gpointer userdata)
{
	ircnet *net = (ircnet *)userdata;
	GtkWidget *dialog;
	char *cert_file;

	cert_file = servlist_get_cert_file (net);
	if (!cert_file)
		return;

	if (g_remove (cert_file) == 0)
	{
		servlist_update_cert_buttons (net);
		dialog = gtk_message_dialog_new (GTK_WINDOW (edit_win),
												 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
												 GTK_MESSAGE_INFO,
												 GTK_BUTTONS_CLOSE,
												 _("Client certificate removed for \"%s\"."),
												 net->name);
	}
	else
	{
		dialog = gtk_message_dialog_new (GTK_WINDOW (edit_win),
												 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
												 GTK_MESSAGE_ERROR,
												 GTK_BUTTONS_CLOSE,
												 _("Failed to remove client certificate for \"%s\"."),
												 net->name);
	}

	theme_manager_attach_window (dialog);
	g_signal_connect (dialog, "response",
		G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
	gtk_widget_show (dialog);
	g_free (cert_file);
}

static GtkWidget *
servlist_icon_button_new (const char *label, const char *icon_name)
{
	GtkWidget *button;

	(void)icon_name;
	button = gtk_button_new_with_mnemonic (label);

	return button;
}


static const char *pages[]=
{
	IRC_DEFAULT_CHARSET,
	"CP1252 (Windows-1252)",
	"ISO-8859-15 (Western Europe)",
	"ISO-8859-2 (Central Europe)",
	"ISO-8859-7 (Greek)",
	"ISO-8859-8 (Hebrew)",
	"ISO-8859-9 (Turkish)",
	"ISO-2022-JP (Japanese)",
	"SJIS (Japanese)",
	"CP949 (Korean)",
	"KOI8-R (Cyrillic)",
	"CP1251 (Cyrillic)",
	"CP1256 (Arabic)",
	"CP1257 (Baltic)",
	"GB18030 (Chinese)",
	"TIS-620 (Thai)",
	NULL
};

/* This is our dictionary for authentication types. Keep these in sync with
 * login_types[]! This allows us to re-order the login type dropdown in the
 * network list without breaking config compatibility.
 *
 * Also make sure inbound_nickserv_login() won't break, i.e. if you add a new
 * type that is NickServ-based, add it there as well so that Fabulor knows to
 * treat it as such.
 */
static int login_types_conf[] =
{
	LOGIN_DEFAULT,			/* default entry - we don't use this but it makes indexing consistent with login_types[] so it's nice */
	LOGIN_SASL,
#ifdef USE_OPENSSL
	LOGIN_SASLEXTERNAL,
	LOGIN_SASL_SCRAM_SHA_1,
	LOGIN_SASL_SCRAM_SHA_256,
	LOGIN_SASL_SCRAM_SHA_512,
#endif
	LOGIN_PASS,
	LOGIN_MSG_NICKSERV,
	LOGIN_NICKSERV,
#ifdef USE_OPENSSL
	LOGIN_CHALLENGEAUTH,
#endif
	LOGIN_CUSTOM
#if 0
	LOGIN_NS,
	LOGIN_MSG_NS,
	LOGIN_AUTH,
#endif
};

static const char *login_types[]=
{
	"Default",
	"SASL PLAIN (username + password)",
#ifdef USE_OPENSSL
	"SASL EXTERNAL (cert)",
	"SASL SCRAM-SHA-1",
	"SASL SCRAM-SHA-256",
	"SASL SCRAM-SHA-512",
#endif
	"Server password (/PASS password)",
	"NickServ (/MSG NickServ + password)",
	"NickServ (/NICKSERV + password)",
#ifdef USE_OPENSSL
	"Challenge Auth (username + password)",
#endif
	"Custom... (connect commands)",
#if 0
	"NickServ (/NS + password)",
	"NickServ (/MSG NS + password)",
	"AUTH (/AUTH nickname password)",
#endif
	NULL
};

/* poor man's IndexOf() - find the dropdown string index that belongs to the given config value */
static int
servlist_get_login_desc_index (int conf_value)
{
	int i;
	int length = sizeof (login_types_conf) / sizeof (login_types_conf[0]);		/* the number of elements in the conf array */

	for (i = 0; i < length; i++)
	{
		if (login_types_conf[i] == conf_value)
		{
			return i;
		}
	}

	return 0;	/* make the compiler happy */
}

static void
servlist_channels_populate (ircnet *net)
{
	GSList *node;
	int i = 0;
	fabulor_server_entry_list_clear (edit_lists[CHANNEL_TREE]);
	for (node = net->favchanlist; node; node = node->next, i++)
	{
		favchannel *channel = node->data;
		fabulor_server_entry_list_append (edit_lists[CHANNEL_TREE], channel,
			channel->name, channel->key);
		if (net->selected == i)
			fabulor_server_entry_list_select (edit_lists[CHANNEL_TREE], channel);
	}
}

static void
servlist_servers_populate (ircnet *net)
{
	GSList *node;
	int i = 0;
	fabulor_server_entry_list_clear (edit_lists[SERVER_TREE]);
	for (node = net->servlist; node; node = node->next, i++)
	{
		ircserver *server = node->data;
		fabulor_server_entry_list_append (edit_lists[SERVER_TREE], server,
			server->hostname, NULL);
		if (net->selected == i)
			fabulor_server_entry_list_select (edit_lists[SERVER_TREE], server);
	}
}

static void
servlist_commands_populate (ircnet *net)
{
	GSList *node;
	int i = 0;
	fabulor_server_entry_list_clear (edit_lists[CMD_TREE]);
	for (node = net->commandlist; node; node = node->next, i++)
	{
		commandentry *entry = node->data;
		fabulor_server_entry_list_append (edit_lists[CMD_TREE], entry,
			entry->command, NULL);
		if (net->selected == i)
			fabulor_server_entry_list_select (edit_lists[CMD_TREE], entry);
	}
}

static void
servlist_networks_populate (GSList *netlist)
{
	ircnet *selected = NULL;
	ircnet *net;
	int i = 0;

	if (!netlist)
	{
		net = servlist_net_add (_("New Network"), "", FALSE);
		servlist_server_add (net, DEFAULT_SERVER);
		netlist = network_list;
	}
	fabulor_server_network_list_clear (network_list_view);
	while (netlist)
	{
		net = netlist->data;
		if (!prefs.hex_gui_slist_fav || (net->flags & FLAG_FAVORITE))
		{
			fabulor_server_network_list_append (network_list_view, net,
				net->name, !!(net->flags & FLAG_FAVORITE), FALSE);
			if (i == prefs.hex_gui_slist_select)
				selected = net;
		}
		i++;
		netlist = netlist->next;
	}
	if (!selected || !fabulor_server_network_list_select (network_list_view,
		selected))
		fabulor_server_network_list_select_first (network_list_view);
}

static void
servlist_entry_items_get (int tree, GSList **items)
{
	*items = tree == SERVER_TREE ? selected_net->servlist :
		tree == CHANNEL_TREE ? selected_net->favchanlist :
		selected_net->commandlist;
}

static void
servlist_entry_row_cb (gpointer identity, gpointer user_data)
{
	int tree = GPOINTER_TO_INT (user_data);
	GSList *items = NULL;
	if (!selected_net)
		return;
	servlist_entry_items_get (tree, &items);
	if (identity)
		selected_net->selected = g_slist_index (items, identity);
}

static void
servlist_entry_selection_clamp (int tree)
{
	GSList *items;
	gpointer identity = fabulor_server_entry_list_get_selected (
		edit_lists[tree]);
	int count;
	int position;

	servlist_entry_items_get (tree, &items);
	position = identity ? g_slist_index (items, identity) : -1;
	if (position >= 0)
		selected_net->selected = position;
	else
	{
		count = (int) g_slist_length (items);
		selected_net->selected = count == 0 ? 0 :
			MIN (MAX (selected_net->selected, 0), count - 1);
	}
}

static void
servlist_addserver (void)
{
	ircserver *server;

	if (!selected_net)
		return;
	server = servlist_server_add (selected_net, DEFAULT_SERVER);
	fabulor_server_entry_list_append (edit_lists[SERVER_TREE], server,
		server->hostname, NULL);
	fabulor_server_entry_list_select (edit_lists[SERVER_TREE], server);
	fabulor_server_entry_list_start_editing_selected (edit_lists[SERVER_TREE]);
}

static void
servlist_addcommand (void)
{
	commandentry *entry;

	if (!selected_net)
		return;

	entry = servlist_command_add (selected_net, "ECHO hello");
	fabulor_server_entry_list_append (edit_lists[CMD_TREE], entry,
		entry->command, NULL);
	fabulor_server_entry_list_select (edit_lists[CMD_TREE], entry);
	fabulor_server_entry_list_start_editing_selected (edit_lists[CMD_TREE]);
}

static void
servlist_addchannel (void)
{
	favchannel *channel;

	if (!selected_net)
		return;

	servlist_favchan_add (selected_net, "#channel");
	channel = g_slist_last (selected_net->favchanlist)->data;
	fabulor_server_entry_list_append (edit_lists[CHANNEL_TREE], channel,
		channel->name, channel->key);
	fabulor_server_entry_list_select (edit_lists[CHANNEL_TREE], channel);
	fabulor_server_entry_list_start_editing_selected (edit_lists[CHANNEL_TREE]);
}

static void
servlist_addnet_cb (GtkWidget *item, gpointer user_data)
{
	ircnet *net;
	(void) item;
	(void) user_data;

	net = servlist_net_add (_("New Network"), "", TRUE);
	net->encoding = g_strdup (IRC_DEFAULT_CHARSET);
	servlist_server_add (net, DEFAULT_SERVER);

	fabulor_server_network_list_append (network_list_view, net, net->name,
		FALSE, TRUE);
	fabulor_server_network_list_select (network_list_view, net);
	fabulor_server_network_list_start_editing_selected (network_list_view);
}

static void
servlist_deletenetwork (ircnet *net)
{
	fabulor_server_network_list_remove (network_list_view, net);
	servlist_net_remove (net);
	fabulor_server_network_list_select_first (network_list_view);
}

static void
servlist_deletenetdialog_cb (GtkDialog *dialog, gint arg1, ircnet *net)
{
	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
	if (arg1 == GTK_RESPONSE_OK)
		servlist_deletenetwork (net);
}

static GSList *
servlist_move_item (GSList *list, gpointer item, int delta)
{
	int length = (int) g_slist_length (list);
	int pos;
	pos = g_slist_index (list, item);
	if (pos >= 0 && pos + delta >= 0 && pos + delta < length)
	{
		pos += delta;
		list = g_slist_remove (list, item);
		list = g_slist_insert (list, item, pos);
	}
	return list;
}

static gboolean
servlist_net_keypress_cb (GtkWidget *wid, guint keyval,
	GdkModifierType state, gpointer user_data)
{
	gboolean handled = FALSE;
	ircnet *net = selected_net;
	int delta = 0;
	(void) wid;
	(void) user_data;

	if (!net || prefs.hex_gui_slist_fav)
		return FALSE;

	if (state & STATE_SHIFT)
	{
		if (keyval == GDK_KEY_Up)
		{
			handled = TRUE;
			delta = -1;
		}
		else if (keyval == GDK_KEY_Down)
		{
			handled = TRUE;
			delta = 1;
		}
	}
	if (handled && fabulor_server_network_list_move (network_list_view,
		net, delta))
	{
		int pos = g_slist_index (network_list, net) + delta;
		network_list = g_slist_remove (network_list, net);
		network_list = g_slist_insert (network_list, net, pos);
	}

	return handled;
}

static gint
servlist_compare (ircnet *net1, ircnet *net2)
{
	gchar *net1_casefolded, *net2_casefolded;
	int result=0;

	net1_casefolded=g_utf8_casefold(net1->name,-1),
	net2_casefolded=g_utf8_casefold(net2->name,-1),

	result=g_utf8_collate(net1_casefolded,net2_casefolded);

	g_free(net1_casefolded);
	g_free(net2_casefolded);

	return result;

}

static void
servlist_sort (GtkWidget *button, gpointer none)
{
	network_list=g_slist_sort(network_list,(GCompareFunc)servlist_compare);
	servlist_networks_populate (network_list);
}

static void
servlist_favor (GtkWidget *button, gpointer none)
{
	if (!selected_net)
		return;
	if (selected_net->flags & FLAG_FAVORITE)
	{
		selected_net->flags &= ~FLAG_FAVORITE;
		fabulor_server_network_list_set_favorite (network_list_view,
			selected_net, FALSE);
	}
	else
	{
		selected_net->flags |= FLAG_FAVORITE;
		fabulor_server_network_list_set_favorite (network_list_view,
			selected_net, TRUE);
	}
}

static void
servlist_update_from_entry (char **str, GtkWidget *entry)
{
	g_free (*str);

	if (fabulor_gtk_entry_get_text (GTK_ENTRY (entry))[0] == 0)
		*str = NULL;
	else
		*str = g_strdup (fabulor_gtk_entry_get_text (GTK_ENTRY (entry)));
}

static char *
servlist_edit_current_password (ircnet *net)
{
	if (!net)
		return NULL;
	if (net->flags & FLAG_USE_KEYRING)
		return secretstore_get_network_password (net->name);
	return servlist_password_decrypt_for_storage (net->pass);
}

static void
servlist_edit_update (ircnet *net)
{
	gboolean use_keyring;
	gboolean keyring_changed;
	char *password = NULL;
	servlist_update_from_entry (&net->nick, edit_entry_nick);
	servlist_update_from_entry (&net->nick2, edit_entry_nick2);
	servlist_update_from_entry (&net->user, edit_entry_user);
	servlist_update_from_entry (&net->real, edit_entry_real);
	if (net && net->name)
	{
		use_keyring = fabulor_gtk_check_button_get_active (
			edit_check_use_keyring);
		keyring_changed = !!(net->flags & FLAG_USE_KEYRING) != !!use_keyring;
		if (!edit_pass_changed && !keyring_changed)
			return;
		if (edit_pass_changed)
			password = g_strdup (fabulor_gtk_entry_get_text (GTK_ENTRY (edit_entry_pass)));
		else
			password = servlist_edit_current_password (net);
		if (use_keyring)
		{
			if (password && *password)
			{
				if (!secretstore_set_network_password (net->name, password))
				{
					fe_message (_("Windows Credential Manager is unavailable. Fabulor can save this password using encrypted profile storage instead."), FE_MSG_WARN);
					memset (password, 0, strlen (password));
					g_free (password);
					return;
				}
			}
			else
				secretstore_delete_network_password (net->name);
			net->flags |= FLAG_USE_KEYRING;
			g_free (net->pass);
			net->pass = NULL;
		}
		else
		{
			char *enc = NULL;
			if (password && *password)
			{
				enc = servlist_password_encrypt_for_storage (password);
				if (!enc)
				{
					fe_message (_("Could not encrypt this password."), FE_MSG_WARN);
					memset (password, 0, strlen (password));
					g_free (password);
					return;
				}
			}
			secretstore_delete_network_password (net->name);
			net->flags &= ~FLAG_USE_KEYRING;
			g_free (net->pass);
			net->pass = enc;
		}
		if (password)
		{
			memset (password, 0, strlen (password));
			g_free (password);
		}
	}
}

static void
servlist_edit_release (GtkWidget *window)
{
	int i;

	if (edit_win != window)
		return;

	if (edit_loaded_password)
	{
		memset (edit_loaded_password, 0, strlen (edit_loaded_password));
		g_free (edit_loaded_password);
		edit_loaded_password = NULL;
	}
	edit_win = NULL;
	edit_entry_nick = NULL;
	edit_entry_nick2 = NULL;
	edit_entry_user = NULL;
	edit_entry_real = NULL;
	edit_entry_pass = NULL;
	edit_check_show_pass = NULL;
	edit_check_use_keyring = NULL;
	edit_label_nick = NULL;
	edit_label_nick2 = NULL;
	edit_label_real = NULL;
	edit_label_user = NULL;
	edit_button_cert_import = NULL;
	edit_button_cert_info = NULL;
	edit_button_cert_delete = NULL;
	for (i = 0; i < N_TREES; i++)
	{
		edit_lists[i] = NULL;
		edit_trees[i] = NULL;
	}
}

static void
servlist_edit_finalized_cb (gpointer user_data, GObject *window)
{
	(void) user_data;
	servlist_edit_release ((GtkWidget *) window);
}

static void
servlist_edit_close_cb (GtkWidget *button, gpointer userdata)
{
	GtkWidget *window = edit_win;

	(void) button;
	(void) userdata;
	if (selected_net)
		servlist_edit_update (selected_net);
	if (window)
		fabulor_gtk_window_destroy (GTK_WINDOW (window));
}

static gboolean
servlist_editwin_close_request_cb (GtkWindow *win, gpointer none)
{
	(void) win;
	(void) none;
	servlist_edit_close_cb (NULL, NULL);
	return TRUE;
}

static void
servlist_geometry_cb (GtkWindow *win, const FabulorWindowGeometry *geometry,
	gpointer none)
{
	(void) win;
	(void) none;
	if (geometry->width > 0)
		netlist_win_width = geometry->width;
	if (geometry->height > 0)
		netlist_win_height = geometry->height;
}

static void
servlist_edit_geometry_cb (GtkWindow *win,
	const FabulorWindowGeometry *geometry, gpointer none)
{
	(void) win;
	(void) none;
	if (geometry->width > 0)
		netedit_win_width = geometry->width;
	if (geometry->height > 0)
		netedit_win_height = geometry->height;
}

static void
servlist_edit_cb (GtkWidget *but, gpointer none)
{
	if (!fabulor_server_network_list_get_selected (network_list_view))
		return;
	if (!selected_net || !selected_net->name)
		return;
	if ((selected_net->flags & FLAG_USE_KEYRING) && !secretstore_require_unlock (selected_net->name))
		return;

	edit_win = servlist_open_edit (serverlist_win, selected_net);
	gtkutil_set_icon (edit_win);
	servlist_servers_populate (selected_net);
	servlist_channels_populate (selected_net);
	servlist_commands_populate (selected_net);
	g_signal_connect (G_OBJECT (edit_win), "close-request",
							G_CALLBACK (servlist_editwin_close_request_cb), NULL);
	fabulor_window_geometry_watch (GTK_WINDOW (edit_win),
		servlist_edit_geometry_cb, NULL);
	gtk_widget_show (edit_win);
}

static void
servlist_deletenet_cb (GtkWidget *item, ircnet *net)
{
	GtkWidget *dialog;

	if (!fabulor_server_network_list_get_selected (network_list_view))
		return;

	net = selected_net;
	if (!net)
		return;
	dialog = gtk_message_dialog_new (GTK_WINDOW (serverlist_win),
												GTK_DIALOG_DESTROY_WITH_PARENT |
												GTK_DIALOG_MODAL,
												GTK_MESSAGE_QUESTION,
												GTK_BUTTONS_OK_CANCEL,
							_("Really remove network \"%s\" and all its servers?"),
												net->name);
	theme_manager_attach_window (dialog);
	g_signal_connect (dialog, "response",
							G_CALLBACK (servlist_deletenetdialog_cb), net);
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
	gtk_widget_show (dialog);
}

static void
servlist_deleteserver (ircserver *serv)
{
	/* don't remove the last server */
	if (selected_net && g_slist_length (selected_net->servlist) < 2)
		return;
	if (selected_net)
	{
		fabulor_server_entry_list_remove (edit_lists[SERVER_TREE], serv);
		servlist_server_remove (selected_net, serv);
		servlist_entry_selection_clamp (SERVER_TREE);
	}
}

static void
servlist_editbutton_cb (GtkWidget *item, GtkNotebook *notebook)
{
	fabulor_server_entry_list_start_editing_selected (
		edit_lists[gtk_notebook_get_current_page (notebook)]);
}

static void
servlist_deleteserver_cb (void)
{
	ircserver *serv = fabulor_server_entry_list_get_selected (
		edit_lists[SERVER_TREE]);
	if (serv)
		servlist_deleteserver (serv);
}

static void
servlist_deletecommand (commandentry *entry)
{
	if (selected_net)
	{
		fabulor_server_entry_list_remove (edit_lists[CMD_TREE], entry);
		servlist_command_remove (selected_net, entry);
		servlist_entry_selection_clamp (CMD_TREE);
	}
}

static void
servlist_deletecommand_cb (void)
{
	commandentry *entry = fabulor_server_entry_list_get_selected (
		edit_lists[CMD_TREE]);
	if (entry)
		servlist_deletecommand (entry);
}

static void
servlist_deletechannel (favchannel *favchan)
{
	if (selected_net)
	{
		fabulor_server_entry_list_remove (edit_lists[CHANNEL_TREE], favchan);
		servlist_favchan_remove (selected_net, favchan);
		servlist_entry_selection_clamp (CHANNEL_TREE);
	}
}

static void
servlist_deletechannel_cb (void)
{
	favchannel *channel = fabulor_server_entry_list_get_selected (
		edit_lists[CHANNEL_TREE]);
	if (channel)
		servlist_deletechannel (channel);
}

static void
servlist_network_row_cb (gpointer identity, gpointer user_data)
{
	(void) user_data;
	selected_net = identity;
	if (selected_net)
		prefs.hex_gui_slist_select = g_slist_index (network_list, selected_net);
}

static int
servlist_savegui (void)
{
	char *sp;
	const char *nick1, *nick2;

	/* check for blank username, ircd will not allow this */
	if (fabulor_gtk_entry_get_text (GTK_ENTRY (entry_guser))[0] == 0)
		return 1;

	/* if (gtk_entry_get_text (GTK_ENTRY (entry_greal))[0] == 0)
		return 1; */

	nick1 = fabulor_gtk_entry_get_text (GTK_ENTRY (entry_nick1));
	nick2 = fabulor_gtk_entry_get_text (GTK_ENTRY (entry_nick2));

	/* ensure unique nicknames */
	if (!rfc_casecmp (nick1, nick2))
		return 2;

	safe_strcpy (prefs.hex_irc_nick1, nick1, sizeof(prefs.hex_irc_nick1));
	safe_strcpy (prefs.hex_irc_nick2, nick2, sizeof(prefs.hex_irc_nick2));
	safe_strcpy (prefs.hex_irc_nick3, fabulor_gtk_entry_get_text (GTK_ENTRY (entry_nick3)), sizeof(prefs.hex_irc_nick3));
	safe_strcpy (prefs.hex_irc_user_name, fabulor_gtk_entry_get_text (GTK_ENTRY (entry_guser)), sizeof(prefs.hex_irc_user_name));
	sp = strchr (prefs.hex_irc_user_name, ' ');
	if (sp)
		sp[0] = 0;	/* spaces will break the login */
	/* strcpy (prefs.hex_irc_real_name, gtk_entry_get_text (GTK_ENTRY (entry_greal))); */
	servlist_save ();
	if (!save_config ())
		fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);

	return 0;
}

static void
servlist_addbutton_cb (GtkWidget *item, GtkNotebook *notebook)
{
		switch (gtk_notebook_get_current_page (notebook))
		{
				case SERVER_TREE:
						servlist_addserver ();
						break;
				case CHANNEL_TREE:
						servlist_addchannel ();
						break;
				case CMD_TREE:
						servlist_addcommand ();
						break;
				default:
						break;
		}
}

static void
servlist_deletebutton_cb (GtkWidget *item, GtkNotebook *notebook)
{
		switch (gtk_notebook_get_current_page (notebook))
		{
				case SERVER_TREE:
						servlist_deleteserver_cb ();
						break;
				case CHANNEL_TREE:
						servlist_deletechannel_cb ();
						break;
				case CMD_TREE:
						servlist_deletecommand_cb ();
						break;
				default:
						break;
		}
}

static gboolean
servlist_keypress_cb (GtkWidget *wid, guint keyval, GdkModifierType state,
	gpointer user_data)
{
	GtkNotebook *notebook = user_data;
	gboolean handled = FALSE;
	int delta = 0;
	int tree;
	gpointer identity;
	(void) wid;

	if (!selected_net)
		return FALSE;

	if (state & STATE_SHIFT)
	{
		if (keyval == GDK_KEY_Up)
		{
			handled = TRUE;
			delta = -1;
		}
		else if (keyval == GDK_KEY_Down)
		{
			handled = TRUE;
			delta = +1;
		}
	}
	
	if (!handled)
		return FALSE;
	tree = gtk_notebook_get_current_page (notebook);
	identity = fabulor_server_entry_list_get_selected (edit_lists[tree]);
	if (!identity || !fabulor_server_entry_list_move (edit_lists[tree],
		identity, delta))
		return TRUE;
	switch (tree)
	{
	case SERVER_TREE:
		selected_net->servlist = servlist_move_item (selected_net->servlist,
			identity, delta);
		break;
	case CHANNEL_TREE:
		selected_net->favchanlist = servlist_move_item (
			selected_net->favchanlist, identity, delta);
		break;
	case CMD_TREE:
		selected_net->commandlist = servlist_move_item (
			selected_net->commandlist, identity, delta);
		break;
	}
	{
		GSList *items;
		servlist_entry_items_get (tree, &items);
		selected_net->selected = g_slist_index (items, identity);
	}
	return TRUE;
}

static void
servlist_entry_list_free_notify (gpointer data)
{
	fabulor_server_entry_list_free (data);
}

void
servlist_autojoinedit (ircnet *net, char *channel, gboolean add)
{
	favchannel *fav;

	if (add)
	{
		servlist_favchan_add (net, channel);
		servlist_save ();
	}
	else
	{
		fav = servlist_favchan_find (net, channel, NULL);
		if (fav)
		{
			servlist_favchan_remove (net, fav);
			servlist_save ();
		}
	}
}

static void
servlist_toggle_global_user (gboolean sensitive)
{
	gtk_widget_set_sensitive (edit_entry_nick, sensitive);
	gtk_widget_set_sensitive (edit_label_nick, sensitive);

	gtk_widget_set_sensitive (edit_entry_nick2, sensitive);
	gtk_widget_set_sensitive (edit_label_nick2, sensitive);

	gtk_widget_set_sensitive (edit_entry_user, sensitive);
	gtk_widget_set_sensitive (edit_label_user, sensitive);

	gtk_widget_set_sensitive (edit_entry_real, sensitive);
	gtk_widget_set_sensitive (edit_label_real, sensitive);
}

static void
servlist_connect_cb (GtkWidget *button, gpointer userdata)
{
	int servlist_err;

	if (!selected_net)
		return;

	servlist_err = servlist_savegui ();
	if (servlist_err == 1)
	{
		fe_message (_("User name cannot be left blank."), FE_MSG_ERROR);
		return;
	}

 	if (!is_session (servlist_sess))
		servlist_sess = NULL;	/* open a new one */

	{
		GSList *list;
		session *sess;
		session *chosen = servlist_sess;

		servlist_sess = NULL;	/* open a new one */

		for (list = sess_list; list; list = list->next)
		{
			sess = list->data;
			if (sess->server->network == selected_net)
			{
				servlist_sess = sess;
				if (sess->server->connected)
					servlist_sess = NULL;	/* open a new one */
				break;
			}
		}

		/* use the chosen one, if it's empty */
		if (!servlist_sess &&
			  chosen &&
			 !chosen->server->connected &&
			  chosen->server->server_session->channel[0] == 0)
		{
			servlist_sess = chosen;
		}
	}

	servlist_connect (servlist_sess, selected_net, TRUE);

	fabulor_gtk_window_destroy (GTK_WINDOW (serverlist_win));
}

static gboolean
servlist_network_edit_cb (gpointer identity, const gchar *new_name,
	gpointer user_data)
{
	ircnet *net = identity;
	char *old_name;
	(void) user_data;

	if (!net || !new_name)
		return FALSE;
	if (!new_name[0])
	{
		servlist_deletenetwork (net);
		return FALSE;
	}
	old_name = net->name;
	net->name = g_strdup (new_name);
	g_free (old_name);
	return TRUE;
}

static void
servlist_check_cb (GtkWidget *but, gpointer num_p)
{
	int num = GPOINTER_TO_INT (num_p);

	if (!selected_net)
		return;

	if ((1 << num) == FLAG_CYCLE || (1 << num) == FLAG_USE_PROXY)
	{
		/* these ones are reversed, so it's compat with 2.0.x */
		if (fabulor_gtk_check_button_get_active (but))
			selected_net->flags &= ~(1 << num);
		else
			selected_net->flags |= (1 << num);
	} else
	{
		if (fabulor_gtk_check_button_get_active (but))
			selected_net->flags |= (1 << num);
		else
			selected_net->flags &= ~(1 << num);
	}

	if ((1 << num) == FLAG_USE_GLOBAL)
	{
		servlist_toggle_global_user (
			!fabulor_gtk_check_button_get_active (but));
	}
}

typedef enum
{
	SERVLIST_ALIGN_START,
	SERVLIST_ALIGN_CENTER,
	SERVLIST_ALIGN_FILL
} servlist_align;

static GtkAlign
servlist_align_to_gtk (servlist_align align)
{
	switch (align)
	{
	case SERVLIST_ALIGN_FILL:
		return GTK_ALIGN_FILL;
	case SERVLIST_ALIGN_CENTER:
		return GTK_ALIGN_CENTER;
	case SERVLIST_ALIGN_START:
	default:
		return GTK_ALIGN_START;
	}
}

static void
servlist_table_attach (GtkWidget *table, GtkWidget *child,
					   guint left_attach, guint right_attach,
					   guint top_attach, guint bottom_attach,
					   gboolean hexpand, gboolean vexpand,
					   servlist_align halign, servlist_align valign,
					   guint xpad, guint ypad)
{
	gtk_widget_set_hexpand (child, hexpand);
	gtk_widget_set_vexpand (child, vexpand);
	gtk_widget_set_halign (child, servlist_align_to_gtk (halign));
	gtk_widget_set_valign (child, servlist_align_to_gtk (valign));
	gtk_widget_set_margin_start (child, xpad);
	gtk_widget_set_margin_end (child, xpad);
	gtk_widget_set_margin_top (child, ypad);
	gtk_widget_set_margin_bottom (child, ypad);
	gtk_grid_attach (GTK_GRID (table), child, left_attach, top_attach,
					 right_attach - left_attach, bottom_attach - top_attach);
}

static GtkWidget *
servlist_create_check (int num, int state, GtkWidget *table, int row, int col, char *labeltext)
{
	GtkWidget *but;

	but = gtk_check_button_new_with_label (labeltext);
	fabulor_gtk_check_button_set_active (but, state);
	g_signal_connect (G_OBJECT (but), "toggled",
							G_CALLBACK (servlist_check_cb), GINT_TO_POINTER (num));
	servlist_table_attach (table, but, col, col + 2, row, row + 1,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);
	gtk_widget_show (but);

	return but;
}

static GtkWidget *
servlist_create_entry (GtkWidget *table, char *labeltext, int row,
							  char *def, GtkWidget **label_ret, char *tip)
{
	GtkWidget *label, *entry;

	label = gtk_label_new_with_mnemonic (labeltext);
	if (label_ret)
		*label_ret = label;
	gtk_widget_show (label);
	servlist_table_attach (table, label, 0, 1, row, row + 1,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	entry = gtk_entry_new ();
	gtk_widget_set_tooltip_text (entry, tip);
	gtk_widget_show (entry);
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry), def ? def : "");
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

	servlist_table_attach (table, entry, 1, 2, row, row + 1,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);

	return entry;
}

static void
servlist_network_list_release (void)
{
	fabulor_server_network_list_free (network_list_view);
	network_list_view = NULL;
	networks_tree = NULL;
}

static void
servlist_window_release (GtkWidget *window)
{
	GtkWidget *editor;

	if (serverlist_win != window)
		return;

	editor = edit_win;
	if (editor)
		fabulor_gtk_window_destroy (GTK_WINDOW (editor));
	serverlist_win = NULL;
	selected_net = NULL;
	servlist_sess = NULL;
	servlist_network_list_release ();
}

static void
servlist_window_finalized_cb (gpointer user_data, GObject *window)
{
	(void) user_data;
	servlist_window_release ((GtkWidget *) window);
}

static gboolean
servlist_delete_cb (GtkWindow *win, gpointer userdata)
{
	(void) win;
	(void) userdata;
	servlist_savegui ();

	if (sess_list == NULL)
		zoitechat_exit ();

	return FALSE;
}

static void
servlist_close_cb (GtkWidget *button, gpointer userdata)
{
	GtkWidget *window = serverlist_win;

	(void) button;
	(void) userdata;
	servlist_savegui ();
	if (window)
		fabulor_gtk_window_destroy (GTK_WINDOW (window));

	if (sess_list == NULL)
		zoitechat_exit ();
}

/* convert "host:port" format to "host/port" */

static char *
servlist_sanitize_hostname (const char *host)
{
	char *ret, *c, *e;

	ret = g_strdup (host);

	c = strchr  (ret, ':');
	e = strrchr (ret, ':');

	/* if only one colon exists it's probably not IPv6 */
	if (c && c == e)
		*c = '/';

	return g_strstrip(ret);
}

/* remove leading slash */
static char *
servlist_sanitize_command (const char *cmd)
{
	if (cmd[0] == '/')
	{
		return (g_strdup (cmd + 1));
	}
	else
	{
		return (g_strdup (cmd));
	}
}

static gboolean
servlist_entry_edit_cb (gpointer identity, FabulorServerEntryField field,
	const gchar *new_text, gpointer user_data)
{
	int tree = GPOINTER_TO_INT (user_data);
	char *old_text;

	if (!selected_net || !identity || !new_text)
		return FALSE;

	if (tree == SERVER_TREE && field == FABULOR_SERVER_ENTRY_PRIMARY)
	{
		ircserver *server = identity;
		if (!new_text[0])
		{
			servlist_deleteserver (server);
			return FALSE;
		}
		old_text = server->hostname;
		server->hostname = servlist_sanitize_hostname (new_text);
		g_free (old_text);
		fabulor_server_entry_list_update (edit_lists[tree], identity, field,
			server->hostname);
	}
	else if (tree == CMD_TREE && field == FABULOR_SERVER_ENTRY_PRIMARY)
	{
		commandentry *entry = identity;
		if (!new_text[0])
		{
			servlist_deletecommand (entry);
			return FALSE;
		}
		old_text = entry->command;
		entry->command = servlist_sanitize_command (new_text);
		g_free (old_text);
		fabulor_server_entry_list_update (edit_lists[tree], identity, field,
			entry->command);
	}
	else if (tree == CHANNEL_TREE)
	{
		favchannel *channel = identity;
		if (field == FABULOR_SERVER_ENTRY_PRIMARY)
		{
			if (!new_text[0])
			{
				servlist_deletechannel (channel);
				return FALSE;
			}
			old_text = channel->name;
			channel->name = g_strdup (new_text);
			g_free (old_text);
			fabulor_server_entry_list_update (edit_lists[tree], identity, field,
				channel->name);
		}
		else
		{
			old_text = channel->key;
			channel->key = new_text[0] ? g_strdup (new_text) : NULL;
			g_free (old_text);
			fabulor_server_entry_list_update (edit_lists[tree], identity, field,
				channel->key ? channel->key : "");
		}
	}

	/* The model has already been updated with its canonical value. */
	return FALSE;
}

static gboolean
servlist_edit_tabswitch_cb (GtkNotebook *nb, gpointer *newtab, guint newindex, gpointer user_data)
{
	/* remember the active tab */
	netedit_active_tab = newindex;

	return FALSE;
}

static void
servlist_combo_cb (GtkEntry *entry, gpointer userdata)
{
	if (!selected_net)
		return;

	g_free (selected_net->encoding);
	selected_net->encoding = g_strdup (fabulor_gtk_entry_get_text (entry));
}

/* Fills up the network's authentication type so that it's guaranteed to be either NULL or a valid value. */
static void
servlist_logintypecombo_cb (GtkComboBox *cb, gpointer *userdata)
{
	int index;

	if (!selected_net)
	{
		return;
	}

	index = gtk_combo_box_get_active (cb);	/* starts at 0, returns -1 for invalid selections */

	if (index == -1)
		return; /* Invalid */

	/* The selection is valid. It can be 0, which is the default type, but we need to allow
	 * that so that you can revert from other types. servlist_save() will dump 0 anyway.
	 */
	selected_net->logintype = login_types_conf[index];

	if (login_types_conf[index] == LOGIN_CUSTOM)
	{
		gtk_notebook_set_current_page (GTK_NOTEBOOK (userdata), 2);		/* FIXME avoid hardcoding? */
	}
	
	/* EXTERNAL uses a cert, not a pass */
	if (login_types_conf[index] == LOGIN_SASLEXTERNAL)
		gtk_widget_set_sensitive (edit_entry_pass, FALSE);
	else
		gtk_widget_set_sensitive (edit_entry_pass, TRUE);
}

static void
servlist_username_changed_cb (GtkEntry *entry, gpointer userdata)
{
	GtkWidget *connect_btn = GTK_WIDGET (userdata);

	if (fabulor_gtk_entry_get_text (entry)[0] == 0)
	{
		gtk_entry_set_icon_from_icon_name (entry, GTK_ENTRY_ICON_SECONDARY, ICON_SERVLIST_ERROR);
		gtk_entry_set_icon_tooltip_text (entry, GTK_ENTRY_ICON_SECONDARY,
										_("User name cannot be left blank."));
		gtk_widget_set_sensitive (connect_btn, FALSE);
	}
	else
	{
		gtk_entry_set_icon_from_icon_name (entry, GTK_ENTRY_ICON_SECONDARY, NULL);
		gtk_widget_set_sensitive (connect_btn, TRUE);
	}
}

static void
servlist_nick_changed_cb (GtkEntry *entry, gpointer userdata)
{
	GtkWidget *connect_btn = GTK_WIDGET (userdata);
	const gchar *nick1 = fabulor_gtk_entry_get_text (GTK_ENTRY (entry_nick1));
	const gchar *nick2 = fabulor_gtk_entry_get_text (GTK_ENTRY (entry_nick2));

	if (!nick1[0] || !nick2[0])
	{
		entry = GTK_ENTRY(!nick1[0] ? entry_nick1 : entry_nick2);
		gtk_entry_set_icon_from_icon_name (entry, GTK_ENTRY_ICON_SECONDARY, ICON_SERVLIST_ERROR);
		gtk_entry_set_icon_tooltip_text (entry, GTK_ENTRY_ICON_SECONDARY,
		                                 _("You cannot have an empty nick name."));
		gtk_widget_set_sensitive (connect_btn, FALSE);
	}
	else if (!rfc_casecmp (nick1, nick2))
	{
		gtk_entry_set_icon_from_icon_name (entry, GTK_ENTRY_ICON_SECONDARY, ICON_SERVLIST_ERROR);
		gtk_entry_set_icon_tooltip_text (entry, GTK_ENTRY_ICON_SECONDARY,
										_("You must have two unique nick names."));
		gtk_widget_set_sensitive (connect_btn, FALSE);
	}
	else
	{
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(entry_nick1), GTK_ENTRY_ICON_SECONDARY, NULL);
		gtk_entry_set_icon_from_icon_name (GTK_ENTRY(entry_nick2), GTK_ENTRY_ICON_SECONDARY, NULL);
		gtk_widget_set_sensitive (connect_btn, TRUE);
	}
}

static GtkWidget *
servlist_create_charsetcombo (void)
{
	GtkWidget *cb;
	GtkEntry *entry;
	int i;

	cb = gtk_combo_box_text_new_with_entry ();
	i = 0;
	while (pages[i])
	{
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (cb), (char *)pages[i]);
		i++;
	}

	entry = fabulor_gtk_combo_box_get_entry (GTK_COMBO_BOX (cb));
	if (entry)
	{
		fabulor_gtk_entry_set_text (entry,
			selected_net->encoding ? selected_net->encoding : pages[0]);
		g_signal_connect (G_OBJECT (entry), "changed",
			G_CALLBACK (servlist_combo_cb), NULL);
	}

	return cb;
}

static GtkWidget *
servlist_create_logintypecombo (GtkWidget *data)
{
	GtkWidget *cb;
	int i;

	cb = gtk_combo_box_text_new ();

	i = 0;

	while (login_types[i])
	{
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (cb), (char *)login_types[i]);
		i++;
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (cb), servlist_get_login_desc_index (selected_net->logintype));

	gtk_widget_set_tooltip_text (cb, _("The way you identify yourself to the server. For custom login methods use connect commands."));
	g_signal_connect (G_OBJECT (cb), "changed",
		G_CALLBACK (servlist_logintypecombo_cb), data);

	return cb;
}

static void
no_servlist (GtkWidget * igad, gpointer serv)
{
	if (fabulor_gtk_check_button_get_active (igad))
		prefs.hex_gui_slist_skip = TRUE;
	else
		prefs.hex_gui_slist_skip = FALSE;
}

static void
fav_servlist (GtkWidget * igad, gpointer serv)
{
	if (fabulor_gtk_check_button_get_active (igad))
		prefs.hex_gui_slist_fav = TRUE;
	else
		prefs.hex_gui_slist_fav = FALSE;

	servlist_networks_populate (network_list);
}

static GtkWidget *
bold_label (char *text)
{
	char buf[128];
	GtkWidget *label;

	g_snprintf (buf, sizeof (buf), "<b>%s</b>", text);
	label = gtk_label_new (buf);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_widget_show (label);

	return label;
}

static GtkWidget *
servlist_open_edit (GtkWidget *parent, ircnet *net)
{
	GtkWidget *editwindow;
	GtkWidget *vbox5;
	GtkWidget *table3;
	GtkWidget *label34;
	GtkWidget *label_logintype;
	GtkWidget *comboboxentry_charset;
	GtkWidget *combobox_logintypes;
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow2;
	GtkWidget *scrolledwindow4;
	GtkWidget *scrolledwindow5;
	GtkWidget *treeview_servers;
	GtkWidget *treeview_channels;
	GtkWidget *treeview_commands;
	GtkWidget *vbuttonbox1;
	GtkWidget *buttonadd;
	GtkWidget *buttonremove;
	GtkWidget *buttonedit;
	GtkWidget *hbox_cert_buttons;
	GtkWidget *hseparator2;
	GtkWidget *hbuttonbox4;
	GtkWidget *button10;
	GtkWidget *check;
	GtkWidget *notebook;
	char buf[128];

	editwindow = fabulor_gtk_window_new ();
	theme_manager_attach_window (editwindow);
	fabulor_gtk_container_set_uniform_inset (editwindow, 4);
	g_snprintf (buf, sizeof (buf), _("Edit %s - %s"), net->name, _(DISPLAY_NAME));
	gtk_window_set_title (GTK_WINDOW (editwindow), buf);
	gtk_window_set_default_size (GTK_WINDOW (editwindow), netedit_win_width, netedit_win_height);
	gtk_window_set_transient_for (GTK_WINDOW (editwindow), GTK_WINDOW (parent));
	gtk_window_set_modal (GTK_WINDOW (editwindow), TRUE);
	fabulor_gtk_window_set_dialog_hint (GTK_WINDOW (editwindow));
	fabulor_gtk_window_set_role (GTK_WINDOW (editwindow), "editserv");

	vbox5 = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	fabulor_gtk_window_set_child (GTK_WINDOW (editwindow), vbox5);


	/* Tabs and buttons */
	hbox1 = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	fabulor_gtk_box_append (GTK_BOX (vbox5), hbox1, TRUE, TRUE, 4);

	scrolledwindow2 = fabulor_gtk_scrolled_window_new ();
	scrolledwindow4 = fabulor_gtk_scrolled_window_new ();
	scrolledwindow5 = fabulor_gtk_scrolled_window_new ();

	notebook = gtk_notebook_new ();
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow2, gtk_label_new (_("Servers")));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow4, gtk_label_new (_("Autojoin channels")));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow5, gtk_label_new (_("Connect commands")));
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
	fabulor_gtk_box_append (GTK_BOX (hbox1), notebook, TRUE, TRUE, SERVLIST_X_PADDING);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (scrolledwindow2), TRUE);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (scrolledwindow4), TRUE);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (scrolledwindow5), TRUE);
	gtk_widget_set_tooltip_text (scrolledwindow5, _("%n=Nick name\n%p=Password\n%r=Real name\n%u=User name"));


	/* Editable server, channel, and command lists. */
	edit_lists[SERVER_TREE] = fabulor_server_entry_list_new (FALSE,
		servlist_entry_row_cb, servlist_entry_edit_cb,
		GINT_TO_POINTER (SERVER_TREE));
	edit_lists[CHANNEL_TREE] = fabulor_server_entry_list_new (TRUE,
		servlist_entry_row_cb, servlist_entry_edit_cb,
		GINT_TO_POINTER (CHANNEL_TREE));
	edit_lists[CMD_TREE] = fabulor_server_entry_list_new (FALSE,
		servlist_entry_row_cb, servlist_entry_edit_cb,
		GINT_TO_POINTER (CMD_TREE));

	edit_trees[SERVER_TREE] = treeview_servers =
		fabulor_server_entry_list_create_view (edit_lists[SERVER_TREE],
			GTK_SCROLLED_WINDOW (scrolledwindow2), NULL, NULL, FALSE);
	edit_trees[CHANNEL_TREE] = treeview_channels =
		fabulor_server_entry_list_create_view (edit_lists[CHANNEL_TREE],
			GTK_SCROLLED_WINDOW (scrolledwindow4), _("Channel"),
			_("Key (Password)"), TRUE);
	edit_trees[CMD_TREE] = treeview_commands =
		fabulor_server_entry_list_create_view (edit_lists[CMD_TREE],
			GTK_SCROLLED_WINDOW (scrolledwindow5), NULL, NULL, FALSE);
	g_object_set_data_full (G_OBJECT (editwindow), "server-entry-list",
		edit_lists[SERVER_TREE],
		servlist_entry_list_free_notify);
	g_object_set_data_full (G_OBJECT (editwindow), "channel-entry-list",
		edit_lists[CHANNEL_TREE],
		servlist_entry_list_free_notify);
	g_object_set_data_full (G_OBJECT (editwindow), "command-entry-list",
		edit_lists[CMD_TREE],
		servlist_entry_list_free_notify);
	g_object_weak_ref (G_OBJECT (editwindow), servlist_edit_finalized_cb, NULL);

	fabulor_gtk_widget_on_key_pressed (treeview_servers,
		servlist_keypress_cb, notebook);
	fabulor_gtk_widget_on_key_pressed (treeview_channels,
		servlist_keypress_cb, notebook);
	fabulor_gtk_widget_on_key_pressed (treeview_commands,
		servlist_keypress_cb, notebook);
	gtk_widget_set_size_request (treeview_servers, -1, 80);
	/* Button Box */
	vbuttonbox1 = fabulor_gtk_button_box_new (GTK_ORIENTATION_VERTICAL,
		FABULOR_GTK_BUTTON_BOX_START, 3);
	fabulor_gtk_box_append (GTK_BOX (hbox1), vbuttonbox1, FALSE, FALSE, 3);

	buttonadd = servlist_icon_button_new (_("_Add"), ICON_SERVLIST_ADD);
	g_signal_connect (G_OBJECT (buttonadd), "clicked",
							G_CALLBACK (servlist_addbutton_cb), notebook);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox1), buttonadd, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (buttonadd, TRUE);

	buttonremove = servlist_icon_button_new (_("_Remove"), ICON_SERVLIST_REMOVE);
	g_signal_connect (G_OBJECT (buttonremove), "clicked",
							G_CALLBACK (servlist_deletebutton_cb), notebook);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox1), buttonremove, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (buttonremove, TRUE);

	buttonedit = gtk_button_new_with_mnemonic (_("_Edit"));
	g_signal_connect (G_OBJECT (buttonedit), "clicked",
							G_CALLBACK (servlist_editbutton_cb), notebook);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox1), buttonedit, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (buttonedit, TRUE);


	/* Checkboxes and entries */
	table3 = gtkutil_grid_new (16, 2, FALSE);
	fabulor_gtk_box_append (GTK_BOX (vbox5), table3, FALSE, FALSE, 0);
	gtk_grid_set_row_spacing (GTK_GRID (table3), 2);
	gtk_grid_set_column_spacing (GTK_GRID (table3), 8);

	check = servlist_create_check (0, !(net->flags & FLAG_CYCLE), table3, 0, 0, _("Connect to selected server only"));
	gtk_widget_set_tooltip_text (check, _("Don't cycle through all the servers when the connection fails."));
	servlist_create_check (3, net->flags & FLAG_AUTO_CONNECT, table3, 1, 0, _("Connect to this network automatically"));
	servlist_create_check (4, !(net->flags & FLAG_USE_PROXY), table3, 2, 0, _("Bypass proxy server"));
	check = servlist_create_check (2, net->flags & FLAG_USE_SSL, table3, 3, 0, _("Use TLS for all the servers on this network"));
#ifndef USE_OPENSSL
	gtk_widget_set_sensitive (check, FALSE);
#endif
	check = servlist_create_check (5, net->flags & FLAG_ALLOW_INVALID, table3, 4, 0, _("Accept invalid TLS certificates"));
#ifndef USE_OPENSSL
	gtk_widget_set_sensitive (check, FALSE);
#endif
	servlist_create_check (1, net->flags & FLAG_USE_GLOBAL, table3, 5, 0, _("Use global user information"));

	edit_check_use_keyring = gtk_check_button_new_with_mnemonic (
		_("Store password in Windows Credential Manager"));
	fabulor_gtk_check_button_set_active (edit_check_use_keyring,
		net->flags & FLAG_USE_KEYRING);
	gtk_widget_set_tooltip_text (edit_check_use_keyring,
		_("Recommended for installed mode. When disabled, Fabulor stores the password in encrypted profile storage."));
	servlist_table_attach (table3, edit_check_use_keyring, 0, 2, 6, 7,
					   FALSE, FALSE,
					   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
					   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);

	edit_entry_nick = servlist_create_entry (table3, _("_Nick name:"), 7, net->nick, &edit_label_nick, 0);
	edit_entry_nick2 = servlist_create_entry (table3, _("Second choice:"), 8, net->nick2, &edit_label_nick2, 0);
	edit_entry_real = servlist_create_entry (table3, _("Rea_l name:"), 9, net->real, &edit_label_real, 0);
	edit_entry_user = servlist_create_entry (table3, _("_User name:"), 10, net->user, &edit_label_user, 0);

	label_logintype = gtk_label_new (_("Login method:"));
	servlist_table_attach (table3, label_logintype, 0, 1, 11, 12,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);
	gtk_widget_set_halign (label_logintype, GTK_ALIGN_START);
	gtk_widget_set_valign (label_logintype, GTK_ALIGN_CENTER);
	combobox_logintypes = servlist_create_logintypecombo (notebook);
	servlist_table_attach (table3, combobox_logintypes, 1, 2, 11, 12,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_FILL,
						   4, 2);

	edit_entry_pass = servlist_create_entry (table3, _("Password:"), 12, NULL, 0, _("Password used for login. If in doubt, leave blank."));
	if (edit_loaded_password)
	{
		memset (edit_loaded_password, 0, strlen (edit_loaded_password));
		g_free (edit_loaded_password);
		edit_loaded_password = NULL;
	}
	edit_pass_changed = 0;
	g_signal_connect (G_OBJECT (edit_entry_pass), "changed",
					  G_CALLBACK (servlist_password_changed_cb), NULL);
	if (net->flags & FLAG_USE_KEYRING)
	{
		char *stored = secretstore_get_network_password (net->name);
		if (stored && *stored)
		{
			edit_loaded_password = g_strdup (stored);
			servlist_entry_set_text_silent (edit_entry_pass, "***");
		}
		if (stored)
		{
			memset (stored, 0, strlen (stored));
			g_free (stored);
		}
	}
	else if (net->pass && *net->pass)
	{
		servlist_entry_set_text_silent (edit_entry_pass, "***");
	}
	edit_pass_changed = 0;
	gtk_entry_set_visibility (GTK_ENTRY (edit_entry_pass), FALSE);
	if (selected_net && selected_net->logintype == LOGIN_SASLEXTERNAL)
		gtk_widget_set_sensitive (edit_entry_pass, FALSE);
	edit_check_show_pass = gtk_check_button_new_with_mnemonic (_("Show password"));
	servlist_table_attach (table3, edit_check_show_pass, 0, 2, 13, 14,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   4, 2);
	g_signal_connect (G_OBJECT (edit_check_show_pass), "toggled",
					  G_CALLBACK (servlist_toggle_show_password_cb), edit_entry_pass);

	label34 = gtk_label_new (_("Character set:"));
	servlist_table_attach (table3, label34, 0, 1, 14, 15,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);
	gtk_widget_set_halign (label34, GTK_ALIGN_START);
	gtk_widget_set_valign (label34, GTK_ALIGN_CENTER);
	comboboxentry_charset = servlist_create_charsetcombo ();
	servlist_table_attach (table3, comboboxentry_charset, 1, 2, 14, 15,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_FILL,
						   4, 2);

	hbox_cert_buttons = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 6);
	edit_button_cert_import = gtk_button_new_with_mnemonic (_("Import client certificate..."));
	g_signal_connect (G_OBJECT (edit_button_cert_import), "clicked",
							G_CALLBACK (servlist_import_client_cert_cb), net);
	fabulor_gtk_box_append (GTK_BOX (hbox_cert_buttons), edit_button_cert_import, FALSE, FALSE, 0);

	edit_button_cert_info = gtk_button_new_with_mnemonic (_("Certificate details"));
	g_signal_connect (G_OBJECT (edit_button_cert_info), "clicked",
							G_CALLBACK (servlist_cert_info_cb), net);
	fabulor_gtk_box_append (GTK_BOX (hbox_cert_buttons), edit_button_cert_info, FALSE, FALSE, 0);

	edit_button_cert_delete = gtk_button_new_with_mnemonic (_("Remove certificate"));
	g_signal_connect (G_OBJECT (edit_button_cert_delete), "clicked",
							G_CALLBACK (servlist_delete_client_cert_cb), net);
	fabulor_gtk_box_append (GTK_BOX (hbox_cert_buttons), edit_button_cert_delete, FALSE, FALSE, 0);

	servlist_table_attach (table3, hbox_cert_buttons, 0, 2, 15, 16,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   SERVLIST_X_PADDING, SERVLIST_Y_PADDING);


	/* Rule and Close button */
	hseparator2 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	fabulor_gtk_box_append (GTK_BOX (vbox5), hseparator2, FALSE, FALSE, 8);

	hbuttonbox4 = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_END, 0);
	fabulor_gtk_box_append (GTK_BOX (vbox5), hbuttonbox4, FALSE, FALSE, 0);

	button10 = servlist_icon_button_new (_("_Close"), ICON_SERVLIST_CLOSE);
	g_signal_connect (G_OBJECT (button10), "clicked",
							G_CALLBACK (servlist_edit_close_cb), 0);
	fabulor_gtk_box_append (GTK_BOX (hbuttonbox4), button10, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button10, TRUE);

	if (net->flags & FLAG_USE_GLOBAL)
	{
		servlist_toggle_global_user (FALSE);
	}
	gtk_widget_grab_focus (button10);
	fabulor_gtk_window_set_default_widget (GTK_WINDOW (editwindow), button10);

	fabulor_gtk_widget_reveal_tree (editwindow);
	servlist_update_cert_buttons (net);

	/* We can't set the active tab without child elements being shown, so this must be *after* gtk_widget_show()s! */
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), netedit_active_tab);

	/* We need to connect this *after* setting the active tab so that the value doesn't get overriden. */
	g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (servlist_edit_tabswitch_cb), notebook);

	return editwindow;
}

static GtkWidget *
servlist_open_networks (void)
{
	GtkWidget *servlist;
	GtkWidget *vbox1;
	GtkWidget *label2;
	GtkWidget *table1;
	GtkWidget *label3;
	GtkWidget *label4;
	GtkWidget *label5;
	GtkWidget *label6;
	/* GtkWidget *label7; */
	GtkWidget *entry1;
	GtkWidget *entry2;
	GtkWidget *entry3;
	GtkWidget *entry4;
	/* GtkWidget *entry5; */
	GtkWidget *vbox2;
	GtkWidget *label1;
	GtkWidget *table4;
	GtkWidget *scrolledwindow3;
	GtkWidget *treeview_networks;
	GtkWidget *checkbutton_skip;
	GtkWidget *checkbutton_fav;
	GtkWidget *hbox;
	GtkWidget *vbuttonbox2;
	GtkWidget *button_add;
	GtkWidget *button_remove;
	GtkWidget *button_edit;
	GtkWidget *button_sort;
	GtkWidget *hseparator1;
	GtkWidget *hbuttonbox1;
	GtkWidget *button_connect;
	GtkWidget *button_close;
	char buf[128];

	servlist = fabulor_gtk_window_new ();
	theme_manager_attach_window (servlist);
	fabulor_gtk_container_set_uniform_inset (servlist, 4);
	g_snprintf(buf, sizeof(buf), _("Network List - %s"), _(DISPLAY_NAME));
	gtk_window_set_title (GTK_WINDOW (servlist), buf);
	gtk_window_set_default_size (GTK_WINDOW (servlist), netlist_win_width, netlist_win_height);
	fabulor_gtk_window_set_role (GTK_WINDOW (servlist), "servlist");
	fabulor_gtk_window_set_dialog_hint (GTK_WINDOW (servlist));
	if (current_sess)
		gtk_window_set_transient_for (GTK_WINDOW (servlist), GTK_WINDOW (current_sess->gui->window));

	vbox1 = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	gtk_widget_show (vbox1);
	fabulor_gtk_window_set_child (GTK_WINDOW (servlist), vbox1);

	label2 = bold_label (_("User Information"));
	fabulor_gtk_box_append (GTK_BOX (vbox1), label2, FALSE, FALSE, 0);

	table1 = gtkutil_grid_new (5, 2, FALSE);
	gtk_widget_show (table1);
	fabulor_gtk_box_append (GTK_BOX (vbox1), table1, FALSE, FALSE, 0);
	fabulor_gtk_container_set_uniform_inset (table1, 8);
	gtk_grid_set_row_spacing (GTK_GRID (table1), 2);
	gtk_grid_set_column_spacing (GTK_GRID (table1), 4);

	label3 = gtk_label_new_with_mnemonic (_("_Nick name:"));
	gtk_widget_show (label3);
	servlist_table_attach (table1, label3, 0, 1, 0, 1,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   0, 0);
	gtk_widget_set_halign (label3, GTK_ALIGN_START);
	gtk_widget_set_valign (label3, GTK_ALIGN_CENTER);

	label4 = gtk_label_new (_("Second choice:"));
	gtk_widget_show (label4);
	servlist_table_attach (table1, label4, 0, 1, 1, 2,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   0, 0);
	gtk_widget_set_halign (label4, GTK_ALIGN_START);
	gtk_widget_set_valign (label4, GTK_ALIGN_CENTER);

	label5 = gtk_label_new (_("Third choice:"));
	gtk_widget_show (label5);
	servlist_table_attach (table1, label5, 0, 1, 2, 3,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   0, 0);
	gtk_widget_set_halign (label5, GTK_ALIGN_START);
	gtk_widget_set_valign (label5, GTK_ALIGN_CENTER);

	label6 = gtk_label_new_with_mnemonic (_("_User name:"));
	gtk_widget_show (label6);
	servlist_table_attach (table1, label6, 0, 1, 3, 4,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_START, SERVLIST_ALIGN_CENTER,
						   0, 0);
	gtk_widget_set_halign (label6, GTK_ALIGN_START);
	gtk_widget_set_valign (label6, GTK_ALIGN_CENTER);

	/* label7 = gtk_label_new_with_mnemonic (_("Rea_l name:"));
	gtk_widget_show (label7);
	gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 4, 5,
							(GtkAttachOptions) (GTK_FILL),
							(GtkAttachOptions) (0), 0, 0);
	*/

	entry_nick1 = entry1 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry1), prefs.hex_irc_nick1);
	gtk_widget_show (entry1);
	servlist_table_attach (table1, entry1, 1, 2, 0, 1,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   0, 0);

	entry_nick2 = entry2 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry2), prefs.hex_irc_nick2);
	gtk_widget_show (entry2);
	servlist_table_attach (table1, entry2, 1, 2, 1, 2,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   0, 0);

	entry_nick3 = entry3 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry3), prefs.hex_irc_nick3);
	gtk_widget_show (entry3);
	servlist_table_attach (table1, entry3, 1, 2, 2, 3,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   0, 0);

	entry_guser = entry4 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry4), prefs.hex_irc_user_name);
	gtk_widget_show (entry4);
	servlist_table_attach (table1, entry4, 1, 2, 3, 4,
						   TRUE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   0, 0);

	/* entry_greal = entry5 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry5), prefs.hex_irc_real_name);
	gtk_widget_show (entry5);
	gtk_table_attach (GTK_TABLE (table1), entry5, 1, 2, 4, 5,
							(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
							(GtkAttachOptions) (0), 0, 0); */

	vbox2 = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	gtk_widget_show (vbox2);
	fabulor_gtk_box_append (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);

	label1 = bold_label (_("Networks"));
	fabulor_gtk_box_append (GTK_BOX (vbox2), label1, FALSE, FALSE, 0);

	table4 = gtkutil_grid_new (2, 2, FALSE);
	gtk_widget_show (table4);
	fabulor_gtk_box_append (GTK_BOX (vbox2), table4, TRUE, TRUE, 0);
	fabulor_gtk_container_set_uniform_inset (table4, 8);
	gtk_grid_set_row_spacing (GTK_GRID (table4), 2);
	gtk_grid_set_column_spacing (GTK_GRID (table4), 3);

	scrolledwindow3 = fabulor_gtk_scrolled_window_new ();
	gtk_widget_show (scrolledwindow3);
	servlist_table_attach (table4, scrolledwindow3, 0, 1, 0, 1,
						   TRUE, TRUE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_FILL,
						   0, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3),
											  GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (scrolledwindow3),
		TRUE);

	network_list_view = fabulor_server_network_list_new (
		servlist_network_row_cb, servlist_network_edit_cb, NULL);
	networks_tree = treeview_networks =
		fabulor_server_network_list_create_view (network_list_view,
			GTK_SCROLLED_WINDOW (scrolledwindow3));

	hbox = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	servlist_table_attach (table4, hbox, 0, 2, 1, 2,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_CENTER,
						   0, 0);
	gtk_widget_show (hbox);

	checkbutton_skip =
		gtk_check_button_new_with_mnemonic (_("Skip network list on startup"));
	fabulor_gtk_check_button_set_active (checkbutton_skip,
		prefs.hex_gui_slist_skip);
	fabulor_gtk_box_append (GTK_BOX (hbox), checkbutton_skip, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (checkbutton_skip), "toggled",
							G_CALLBACK (no_servlist), 0);
	gtk_widget_show (checkbutton_skip);

	checkbutton_fav =
		gtk_check_button_new_with_mnemonic (_("Show favorites only"));
	fabulor_gtk_check_button_set_active (checkbutton_fav,
		prefs.hex_gui_slist_fav);
	fabulor_gtk_box_append (GTK_BOX (hbox), checkbutton_fav, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (checkbutton_fav), "toggled",
							G_CALLBACK (fav_servlist), 0);
	gtk_widget_show (checkbutton_fav);

	vbuttonbox2 = fabulor_gtk_button_box_new (GTK_ORIENTATION_VERTICAL,
		FABULOR_GTK_BUTTON_BOX_START, 3);
	gtk_widget_show (vbuttonbox2);
	servlist_table_attach (table4, vbuttonbox2, 1, 2, 0, 1,
						   FALSE, FALSE,
						   SERVLIST_ALIGN_FILL, SERVLIST_ALIGN_FILL,
						   0, 0);

	button_add = servlist_icon_button_new (_("_Add"), ICON_SERVLIST_ADD);
	g_signal_connect (G_OBJECT (button_add), "clicked",
							G_CALLBACK (servlist_addnet_cb), networks_tree);
	gtk_widget_show (button_add);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox2), button_add, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_add, TRUE);

	button_remove = servlist_icon_button_new (_("_Remove"), ICON_SERVLIST_REMOVE);
	g_signal_connect (G_OBJECT (button_remove), "clicked",
							G_CALLBACK (servlist_deletenet_cb), 0);
	gtk_widget_show (button_remove);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox2), button_remove, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_remove, TRUE);

	button_edit = gtk_button_new_with_mnemonic (_("_Edit..."));
	g_signal_connect (G_OBJECT (button_edit), "clicked",
							G_CALLBACK (servlist_edit_cb), 0);
	gtk_widget_show (button_edit);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox2), button_edit, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_edit, TRUE);

	button_sort = gtk_button_new_with_mnemonic (_("_Sort"));
	gtk_widget_set_tooltip_text (button_sort, _("Sorts the network list in alphabetical order. "
				"Use Shift+Up and Shift+Down keys to move a row."));
	g_signal_connect (G_OBJECT (button_sort), "clicked",
							G_CALLBACK (servlist_sort), 0);
	gtk_widget_show (button_sort);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox2), button_sort, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_sort, TRUE);

	button_sort = gtk_button_new_with_mnemonic (_("_Favor"));
	gtk_widget_set_tooltip_text (button_sort, _("Mark or unmark this network as a favorite."));
	g_signal_connect (G_OBJECT (button_sort), "clicked",
							G_CALLBACK (servlist_favor), 0);
	gtk_widget_show (button_sort);
	fabulor_gtk_box_append (GTK_BOX (vbuttonbox2), button_sort, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_sort, TRUE);

	hseparator1 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_show (hseparator1);
	fabulor_gtk_box_append (GTK_BOX (vbox1), hseparator1, FALSE, TRUE, 4);

	hbuttonbox1 = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_SPREAD, 0);
	gtk_widget_show (hbuttonbox1);
	fabulor_gtk_box_append (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
	fabulor_gtk_container_set_uniform_inset (hbuttonbox1, 8);

	button_close = servlist_icon_button_new (_("_Close"), ICON_SERVLIST_CLOSE);
	gtk_widget_show (button_close);
	g_signal_connect (G_OBJECT (button_close), "clicked",
							G_CALLBACK (servlist_close_cb), 0);
	fabulor_gtk_box_append (GTK_BOX (hbuttonbox1), button_close, FALSE, TRUE, 0);
	fabulor_gtk_widget_set_can_default (button_close, TRUE);

button_connect = gtkutil_button (hbuttonbox1, ICON_SERVLIST_CONNECT, NULL,
												servlist_connect_cb, NULL, _("C_onnect"));
	fabulor_gtk_widget_set_can_default (button_connect, TRUE);

	g_signal_connect (G_OBJECT (entry_guser), "changed", 
					G_CALLBACK(servlist_username_changed_cb), button_connect);
	g_signal_connect (G_OBJECT (entry_nick1), "changed",
					G_CALLBACK(servlist_nick_changed_cb), button_connect);
	g_signal_connect (G_OBJECT (entry_nick2), "changed",
					G_CALLBACK(servlist_nick_changed_cb), button_connect);

	/* Run validity checks now */
	servlist_nick_changed_cb (GTK_ENTRY(entry_nick2), button_connect);
	servlist_username_changed_cb (GTK_ENTRY(entry_guser), button_connect);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label3), entry1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label6), entry4);
	/* gtk_label_set_mnemonic_widget (GTK_LABEL (label7), entry5); */

	gtk_widget_grab_focus (networks_tree);
	fabulor_gtk_window_set_default_widget (GTK_WINDOW (servlist), button_close);
	return servlist;
}

void
fe_serverlist_open (session *sess)
{
	if (serverlist_win)
	{
		gtk_window_present (GTK_WINDOW (serverlist_win));
		return;
	}

	servlist_sess = sess;

	serverlist_win = servlist_open_networks ();
	gtkutil_set_icon (serverlist_win);

	servlist_networks_populate (network_list);

	g_signal_connect (G_OBJECT (serverlist_win), "close-request",
							G_CALLBACK (servlist_delete_cb), NULL);
	fabulor_window_geometry_watch (GTK_WINDOW (serverlist_win),
		servlist_geometry_cb, NULL);
	g_object_weak_ref (G_OBJECT (serverlist_win),
		servlist_window_finalized_cb, NULL);
	fabulor_gtk_widget_on_key_pressed (networks_tree,
		servlist_net_keypress_cb, NULL);

	gtk_widget_show (serverlist_win);
}

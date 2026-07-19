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

#ifndef ZOITECHAT_FKEYS_H
#define ZOITECHAT_FKEYS_H

/* These are cp'ed from history.c --AGL */
#define STATE_SHIFT		GDK_SHIFT_MASK
#if GTK_MAJOR_VERSION >= 4
#define STATE_ALT		GDK_ALT_MASK
#else
#define STATE_ALT		GDK_MOD1_MASK
#endif
#define STATE_CTRL		GDK_CONTROL_MASK

typedef struct
{
	guint keyval;
	GdkModifierType state;
} FabulorKeyInput;

void key_init (void);
void key_dialog_show (void);
gboolean key_handle_key_press (GtkWidget *wid, guint keyval,
								GdkModifierType state, gpointer user_data);
int key_action_insert (GtkWidget *wid, const FabulorKeyInput *key,
					   char *d1, char *d2,
						 session *sess);
void key_check_replace_on_change (GtkEditable *editable, gpointer data);
gboolean key_get_menu_accel (const char *name, guint *keyval, GdkModifierType *mod);

#endif

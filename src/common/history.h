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

#ifndef FABULOR_HISTORY_H
#define FABULOR_HISTORY_H

struct session;

struct history
{
	char **lines;
	int len;
	int max;
	int pos;
	struct session *owner;
	char *storage_path;
	unsigned int loaded:1;
	unsigned int dirty:1;
};

void history_add (struct history *his, char *text);
void history_erase (struct history *his);
void history_free (struct history *his);
void history_restore (struct history *his, struct session *owner);
void history_save (void);
char *history_up (struct history *his, char *current_text);
char *history_down (struct history *his);

#endif

#include <string.h>

#include "ircv3-batch.h"

typedef struct
{
	char *id;
	char *type;
	char *target;
	char *label;
	char *parent;
} ircv3_batch_record;

struct ircv3_batch_state
{
	GHashTable *records;
};

static void
batch_record_free (ircv3_batch_record *record)
{
	if (!record)
		return;

	g_free (record->id);
	g_free (record->type);
	g_free (record->target);
	g_free (record->label);
	g_free (record->parent);
	g_free (record);
}

static gboolean
valid_batch_id (const char *id)
{
	const unsigned char *cursor = (const unsigned char *)id;

	if (!id || !*id || strlen (id) > 128)
		return FALSE;

	for (; *cursor; cursor++)
	{
		if (*cursor <= ' ' || *cursor == ';')
			return FALSE;
	}

	return TRUE;
}

ircv3_batch_state *
ircv3_batch_state_new (void)
{
	ircv3_batch_state *state = g_new0 (ircv3_batch_state, 1);

	state->records = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
												 (GDestroyNotify)batch_record_free);
	return state;
}

void
ircv3_batch_state_free (ircv3_batch_state *state)
{
	if (!state)
		return;

	g_hash_table_destroy (state->records);
	g_free (state);
}

void
ircv3_batch_state_clear (ircv3_batch_state *state)
{
	if (state)
		g_hash_table_remove_all (state->records);
}

gboolean
ircv3_batch_start (ircv3_batch_state *state, const char *id,
					 const char *type, const char *target,
					 const char *label, const char *parent)
{
	ircv3_batch_record *record;

	if (!state || !valid_batch_id (id) || !type || !*type
		|| g_hash_table_size (state->records) >= IRCV3_BATCH_LIMIT
		|| g_hash_table_contains (state->records, id))
		return FALSE;

	if (parent && *parent && !g_hash_table_contains (state->records, parent))
		return FALSE;

	record = g_new0 (ircv3_batch_record, 1);
	record->id = g_strdup (id);
	record->type = g_strdup (type);
	record->target = g_strdup (target);
	record->label = g_strdup (label);
	record->parent = g_strdup (parent);
	g_hash_table_insert (state->records, record->id, record);
	return TRUE;
}

gboolean
ircv3_batch_end (ircv3_batch_state *state, const char *id)
{
	guint removed;

	if (!state || !id)
		return FALSE;

	if (!g_hash_table_remove (state->records, id))
		return FALSE;

	do
	{
		GHashTableIter iter;
		gpointer value;

		removed = 0;
		g_hash_table_iter_init (&iter, state->records);
		while (g_hash_table_iter_next (&iter, NULL, &value))
		{
			ircv3_batch_record *record = value;

			if (record->parent && *record->parent
				&& !g_hash_table_contains (state->records, record->parent))
			{
				g_hash_table_iter_remove (&iter);
				removed++;
			}
		}
	} while (removed > 0);

	return TRUE;
}

gboolean
ircv3_batch_lookup (ircv3_batch_state *state, const char *id,
					  ircv3_batch_info *info)
{
	ircv3_batch_record *record;

	if (!state || !id || !info)
		return FALSE;

	record = g_hash_table_lookup (state->records, id);
	if (!record)
		return FALSE;

	info->id = record->id;
	info->type = record->type;
	info->target = record->target;
	info->label = record->label;
	info->parent = record->parent;
	return TRUE;
}

guint
ircv3_batch_count (ircv3_batch_state *state)
{
	return state ? g_hash_table_size (state->records) : 0;
}

char *
ircv3_label_next (guint64 *counter)
{
	g_return_val_if_fail (counter != NULL, NULL);

	(*counter)++;
	if (*counter == 0)
		(*counter)++;

	return g_strdup_printf ("fabulor-%" G_GUINT64_FORMAT, *counter);
}

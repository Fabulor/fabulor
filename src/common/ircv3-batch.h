#ifndef FABULOR_IRCV3_BATCH_H
#define FABULOR_IRCV3_BATCH_H

#include <glib.h>

#define IRCV3_BATCH_LIMIT 64

typedef struct ircv3_batch_state ircv3_batch_state;

typedef struct
{
	const char *id;
	const char *type;
	const char *target;
	const char *label;
	const char *parent;
} ircv3_batch_info;

ircv3_batch_state *ircv3_batch_state_new (void);
void ircv3_batch_state_free (ircv3_batch_state *state);
void ircv3_batch_state_clear (ircv3_batch_state *state);

gboolean ircv3_batch_start (ircv3_batch_state *state, const char *id,
							 const char *type, const char *target,
							 const char *label, const char *parent);
gboolean ircv3_batch_end (ircv3_batch_state *state, const char *id);
gboolean ircv3_batch_lookup (ircv3_batch_state *state, const char *id,
							  ircv3_batch_info *info);
guint ircv3_batch_count (ircv3_batch_state *state);

char *ircv3_label_next (guint64 *counter);

#endif

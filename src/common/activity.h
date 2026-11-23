#ifndef FABULOR_ACTIVITY_H
#define FABULOR_ACTIVITY_H

#include <glib.h>
#include <time.h>

typedef enum
{
	ACTIVITY_NONE,
	ACTIVITY_OTHER,
	ACTIVITY_MESSAGE,
	ACTIVITY_PRIVATE,
	ACTIVITY_HIGHLIGHT,
	ACTIVITY_CLASS_COUNT
} ActivityClass;

typedef enum
{
	ACTIVITY_POLICY_AUTO,
	ACTIVITY_POLICY_ALL,
	ACTIVITY_POLICY_MESSAGES,
	ACTIVITY_POLICY_PRIVATE,
	ACTIVITY_POLICY_HIGHLIGHTS,
	ACTIVITY_POLICY_MUTED,
	ACTIVITY_POLICY_COUNT
} ActivityPolicy;

typedef struct _ActivityRules ActivityRules;
#define ACTIVITY_RULES_MAX_BYTES 32768

typedef struct
{
	guint unread[ACTIVITY_CLASS_COUNT];
	ActivityClass highest;
	time_t latest_at;
	guint64 latest_sequence;
} ActivityState;

typedef struct
{
	/* Wider than a session counter so saturated contexts can be subtracted
	 * without losing contributions from other contexts. */
	guint64 unread[ACTIVITY_CLASS_COUNT];
} ActivityAggregate;

typedef struct
{
	gboolean selected;
	gboolean focused;
	gboolean visible;
	gboolean minimized;
} ActivityView;

gboolean activity_view_is_read (ActivityView view);
gboolean activity_record (ActivityState *state, ActivityAggregate *aggregate,
						  ActivityClass activity, ActivityView view,
						  time_t timestamp, guint64 *sequence);
gboolean activity_clear (ActivityState *state, ActivityAggregate *aggregate);
ActivityClass activity_aggregate_highest (const ActivityAggregate *aggregate);
const char *activity_policy_name (ActivityPolicy policy);
gboolean activity_policy_parse (const char *name, ActivityPolicy *policy);
gboolean activity_policy_allows (ActivityPolicy policy, ActivityClass activity);
ActivityRules *activity_rules_parse (const char *text, GError **error);
void activity_rules_free (ActivityRules *rules);
ActivityPolicy activity_rules_policy (const ActivityRules *rules,
	const char *network, const char *context, gboolean rfc1459);
ActivityClass activity_rules_filter_nick (const ActivityRules *rules,
	const char *nick, gboolean rfc1459, ActivityClass activity);

#endif

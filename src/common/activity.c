#include "activity.h"

#include <glib/gi18n.h>
#include <string.h>

enum { MAX_CONTEXT_RULES = 64, MAX_NICK_RULES = 128, MAX_NICK_BYTES = 63 };

typedef struct
{
	char *network;
	char *context[2];
	GPatternSpec *network_pattern;
	GPatternSpec *context_pattern[2];
	ActivityPolicy policy;
} ActivityRule;

struct _ActivityRules
{
	GPtrArray *contexts;
	GHashTable *nicks[2];
	guint nick_count;
};

static const char *const policy_names[] = {
	"auto", "all", "messages", "private", "highlights", "muted"
};

const char *
activity_policy_name (ActivityPolicy policy)
{
	return (guint) policy < ACTIVITY_POLICY_COUNT ? policy_names[policy] : NULL;
}

gboolean
activity_policy_parse (const char *name, ActivityPolicy *policy)
{
	for (int i = 0; i < ACTIVITY_POLICY_COUNT; ++i)
	{
		if (g_ascii_strcasecmp (name, policy_names[i]) == 0)
		{
			*policy = (ActivityPolicy) i;
			return TRUE;
		}
	}
	return FALSE;
}

gboolean
activity_policy_allows (ActivityPolicy policy, ActivityClass activity)
{
	switch (policy)
	{
	case ACTIVITY_POLICY_MESSAGES:   return activity >= ACTIVITY_MESSAGE;
	case ACTIVITY_POLICY_PRIVATE:    return activity >= ACTIVITY_PRIVATE;
	case ACTIVITY_POLICY_HIGHLIGHTS: return activity == ACTIVITY_HIGHLIGHT;
	case ACTIVITY_POLICY_MUTED:      return FALSE;
	default:                        return activity != ACTIVITY_NONE;
	}
}

/* Fold exactly the ASCII/RFC1459 pairs supported by the IRC engine. */
static void
activity_fold_name (char *output, const char *input, gboolean rfc)
{
	for (; *input; ++input, ++output)
	{
		char c = g_ascii_tolower (*input);
		if (rfc && c >= '[' && c <= '^')
			c += '{' - '[';
		*output = c;
	}
	*output = '\0';
}

static void
activity_rule_free (gpointer data)
{
	ActivityRule *rule = data;
	g_free (rule->network);
	g_clear_pointer (&rule->network_pattern, g_pattern_spec_free);
	for (int i = 0; i < 2; ++i)
	{
		g_free (rule->context[i]);
		g_clear_pointer (&rule->context_pattern[i], g_pattern_spec_free);
	}
	g_free (rule);
}

void
activity_rules_free (ActivityRules *rules)
{
	g_ptr_array_unref (rules->contexts);
	g_hash_table_unref (rules->nicks[0]);
	g_hash_table_unref (rules->nicks[1]);
	g_free (rules);
}

static gboolean
activity_rule_token (const char *field, gsize limit)
{
	if (!*field || strlen (field) > limit)
		return FALSE;
	for (; *field; ++field)
	{
		if ((guchar) *field < ' ' || *field == '\177')
			return FALSE;
	}
	return TRUE;
}

static gboolean
activity_rules_add (ActivityRules *rules, char **fields)
{
	guint count = g_strv_length (fields);
	ActivityPolicy policy;
	gboolean wildcard = g_str_equal (fields[0], "wildcard");

	if (count == 3 && g_str_equal (fields[0], "nick"))
	{
		guint value;
		if (!activity_rule_token (fields[1], MAX_NICK_BYTES) ||
			strpbrk (fields[1], " *?!@") || rules->nick_count >= MAX_NICK_RULES)
			return FALSE;
		if (g_str_equal (fields[2], "other"))
			value = ACTIVITY_OTHER + 1;
		else if (g_str_equal (fields[2], "muted"))
			value = ACTIVITY_NONE + 1;
		else
			return FALSE;
		for (int i = 0; i < 2; ++i)
		{
			char key[MAX_NICK_BYTES + 1];
			activity_fold_name (key, fields[1], i);
			if (!g_hash_table_contains (rules->nicks[i], key))
				g_hash_table_insert (rules->nicks[i], g_strdup (key), GUINT_TO_POINTER (value));
		}
		++rules->nick_count;
		return TRUE;
	}
	if (count != 4 || (!wildcard && !g_str_equal (fields[0], "exact")) ||
		!activity_rule_token (fields[1], 255) || !activity_rule_token (fields[2], 299) ||
		!activity_policy_parse (fields[3], &policy) || policy == ACTIVITY_POLICY_AUTO ||
		rules->contexts->len >= MAX_CONTEXT_RULES)
		return FALSE;

	ActivityRule *rule = g_new0 (ActivityRule, 1);
	rule->policy = policy;
	rule->network = g_ascii_strdown (fields[1], -1);
	for (int i = 0; i < 2; ++i)
	{
		rule->context[i] = g_malloc (strlen (fields[2]) + 1);
		activity_fold_name (rule->context[i], fields[2], i);
		if (wildcard)
			rule->context_pattern[i] = g_pattern_spec_new (rule->context[i]);
	}
	if (wildcard)
		rule->network_pattern = g_pattern_spec_new (rule->network);
	g_ptr_array_add (rules->contexts, rule);
	return TRUE;
}

ActivityRules *
activity_rules_parse (const char *text, GError **error)
{
	if (strlen (text) > ACTIVITY_RULES_MAX_BYTES || !g_utf8_validate (text, -1, NULL))
	{
		g_set_error_literal (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
			_("Activity rules must be valid UTF-8 and at most 32 KiB."));
		return NULL;
	}

	ActivityRules *rules = g_new0 (ActivityRules, 1);
	rules->contexts = g_ptr_array_new_with_free_func (activity_rule_free);
	for (int i = 0; i < 2; ++i)
		rules->nicks[i] = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	char **lines = g_strsplit (text, "\n", -1);
	for (guint i = 0; lines[i]; ++i)
	{
		char *line = g_strstrip (lines[i]);
		if (!*line || *line == '#')
			continue;

		char **fields = g_strsplit (line, " | ", 5);
		for (guint field = 0; fields[field]; ++field)
			g_strstrip (fields[field]);
		gboolean valid = activity_rules_add (rules, fields);
		g_strfreev (fields);
		if (!valid)
		{
			g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
				_("Invalid activity rule or too many rules on line %u."), i + 1);
			g_clear_pointer (&rules, activity_rules_free);
			break;
		}
	}
	g_strfreev (lines);
	return rules;
}

ActivityPolicy
activity_rules_policy (const ActivityRules *rules, const char *network,
						const char *context, gboolean rfc1459)
{
	char net[256], name[300];
	int mapping = !!rfc1459;

	if (!rules || !network || !context || strlen (network) >= sizeof net ||
		strlen (context) >= sizeof name)
		return ACTIVITY_POLICY_ALL;
	activity_fold_name (net, network, FALSE);
	activity_fold_name (name, context, rfc1459);
	for (guint i = 0; i < rules->contexts->len; ++i)
	{
		const ActivityRule *rule = g_ptr_array_index (rules->contexts, i);
		if (rule->network_pattern ?
			g_pattern_spec_match_string (rule->network_pattern, net) &&
			g_pattern_spec_match_string (rule->context_pattern[mapping], name) :
			g_str_equal (net, rule->network) && g_str_equal (name, rule->context[mapping]))
			return rule->policy;
	}
	return ACTIVITY_POLICY_ALL;
}

ActivityClass
activity_rules_filter_nick (const ActivityRules *rules, const char *nick,
							 gboolean rfc1459, ActivityClass activity)
{
	char key[MAX_NICK_BYTES + 1];
	guint match;

	if (!rules || !nick || activity == ACTIVITY_NONE || activity == ACTIVITY_HIGHLIGHT ||
		!g_hash_table_size (rules->nicks[!!rfc1459]) || strlen (nick) > MAX_NICK_BYTES)
		return activity;
	activity_fold_name (key, nick, rfc1459);
	match = GPOINTER_TO_UINT (g_hash_table_lookup (rules->nicks[!!rfc1459], key));
	return match ? (ActivityClass) (match - 1) : activity;
}

gboolean
activity_view_is_read (ActivityView view)
{
	return view.selected && view.focused && view.visible && !view.minimized;
}

gboolean
activity_record (ActivityState *state, ActivityAggregate *aggregate,
				 ActivityClass activity, ActivityView view,
				 time_t timestamp, guint64 *sequence)
{
	if (activity == ACTIVITY_NONE || activity_view_is_read (view))
		return FALSE;

	if (state->unread[activity] < G_MAXUINT)
	{
		state->unread[activity]++;
		if (aggregate)
			aggregate->unread[activity]++;
	}
	state->highest = MAX (state->highest, activity);
	state->latest_at = timestamp;
	if (*sequence < G_MAXUINT64)
		++*sequence;
	state->latest_sequence = *sequence;
	return TRUE;
}

gboolean
activity_clear (ActivityState *state, ActivityAggregate *aggregate)
{
	if (state->highest == ACTIVITY_NONE)
		return FALSE;

	if (aggregate)
	{
		for (int i = ACTIVITY_OTHER; i < ACTIVITY_CLASS_COUNT; ++i)
			aggregate->unread[i] -= state->unread[i];
	}
	*state = (ActivityState) { 0 };
	return TRUE;
}

ActivityClass
activity_aggregate_highest (const ActivityAggregate *aggregate)
{
	for (int i = ACTIVITY_HIGHLIGHT; i >= ACTIVITY_OTHER; --i)
	{
		if (aggregate->unread[i])
			return (ActivityClass) i;
	}
	return ACTIVITY_NONE;
}

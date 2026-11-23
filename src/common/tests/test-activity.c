#include "../activity.h"

static void
test_activity_priority_and_order (void)
{
	ActivityState state = { 0 };
	ActivityView background = { 0 };
	guint64 sequence = 0;

	g_assert_true (activity_record (&state, NULL, ACTIVITY_HIGHLIGHT, background, 300, &sequence));
	g_assert_true (activity_record (&state, NULL, ACTIVITY_MESSAGE, background, 200, &sequence));
	g_assert_true (activity_record (&state, NULL, ACTIVITY_PRIVATE, background, 100, &sequence));
	g_assert_true (activity_record (&state, NULL, ACTIVITY_OTHER, background, 400, &sequence));
	g_assert_cmpuint (state.unread[ACTIVITY_HIGHLIGHT], ==, 1);
	g_assert_cmpuint (state.unread[ACTIVITY_MESSAGE], ==, 1);
	g_assert_cmpuint (state.unread[ACTIVITY_PRIVATE], ==, 1);
	g_assert_cmpuint (state.unread[ACTIVITY_OTHER], ==, 1);
	g_assert_cmpint (state.highest, ==, ACTIVITY_HIGHLIGHT);
	g_assert_cmpint (state.latest_at, ==, 400);
	g_assert_cmpuint (state.latest_sequence, ==, 4);
}

static void
test_activity_view (void)
{
	ActivityState state = { 0 };
	ActivityAggregate aggregate = { 0 };
	guint64 sequence = 0;
	const ActivityView foreground = { TRUE, TRUE, TRUE, FALSE };
	const ActivityView unread_views[] = {
		{ FALSE, TRUE, TRUE, FALSE }, /* another tab in the active window */
		{ TRUE, FALSE, TRUE, FALSE }, /* selected in a background window */
		{ TRUE, TRUE, TRUE, TRUE },   /* minimised during a focus transition */
		{ TRUE, TRUE, FALSE, FALSE }, /* hidden in the tray */
	};

	g_assert_false (activity_record (&state, &aggregate, ACTIVITY_MESSAGE, foreground, 0, &sequence));
	g_assert_false (activity_record (&state, &aggregate, ACTIVITY_NONE, unread_views[0], 0, &sequence));
	g_assert_cmpuint (sequence, ==, 0);
	for (guint i = 0; i < G_N_ELEMENTS (unread_views); ++i)
		g_assert_true (activity_record (&state, &aggregate, ACTIVITY_MESSAGE, unread_views[i], 0, &sequence));
	g_assert_cmpuint (state.unread[ACTIVITY_MESSAGE], ==, G_N_ELEMENTS (unread_views));
	g_assert_cmpuint (aggregate.unread[ACTIVITY_MESSAGE], ==, G_N_ELEMENTS (unread_views));
}

static void
test_activity_clear_and_close (void)
{
	ActivityState server_tab = { 0 };
	ActivityState first_child = { 0 };
	ActivityState second_child = { 0 };
	ActivityAggregate children = { 0 };
	ActivityView background = { 0 };
	guint64 sequence = 0;

	activity_record (&server_tab, NULL, ACTIVITY_HIGHLIGHT, background, 1, &sequence);
	activity_record (&first_child, &children, ACTIVITY_HIGHLIGHT, background, 2, &sequence);
	activity_record (&second_child, &children, ACTIVITY_MESSAGE, background, 3, &sequence);
	g_assert_cmpuint (children.unread[ACTIVITY_HIGHLIGHT], ==, 1);
	g_assert_cmpuint (children.unread[ACTIVITY_MESSAGE], ==, 1);
	g_assert_cmpuint (server_tab.unread[ACTIVITY_MESSAGE], ==, 0);
	g_assert_true (activity_clear (&server_tab, NULL));
	g_assert_cmpint (activity_aggregate_highest (&children), ==, ACTIVITY_HIGHLIGHT);

	g_assert_true (activity_clear (&first_child, &children));
	g_assert_false (activity_clear (&first_child, &children));
	g_assert_cmpuint (children.unread[ACTIVITY_HIGHLIGHT], ==, 0);
	g_assert_cmpint (first_child.highest, ==, ACTIVITY_NONE);
	g_assert_cmpuint (first_child.latest_sequence, ==, 0);
	g_assert_cmpint (first_child.latest_at, ==, 0);
	g_assert_cmpint (activity_aggregate_highest (&children), ==, ACTIVITY_MESSAGE);

	g_assert_true (activity_clear (&second_child, &children));
	g_assert_cmpint (activity_aggregate_highest (&children), ==, ACTIVITY_NONE);
}

static void
test_activity_saturation (void)
{
	ActivityState state = { 0 };
	ActivityAggregate aggregate = { 0 };
	ActivityView background = { 0 };
	guint64 sequence = G_MAXUINT64 - 1;

	state.unread[ACTIVITY_MESSAGE] = G_MAXUINT - 1;
	state.highest = ACTIVITY_MESSAGE;
	/* Another child already contributed five events. */
	aggregate.unread[ACTIVITY_MESSAGE] = (guint64) G_MAXUINT + 4;
	g_assert_true (activity_record (&state, &aggregate, ACTIVITY_MESSAGE, background, 10, &sequence));
	g_assert_true (activity_record (&state, &aggregate, ACTIVITY_MESSAGE, background, 20, &sequence));
	g_assert_cmpuint (state.unread[ACTIVITY_MESSAGE], ==, G_MAXUINT);
	g_assert_cmpuint (aggregate.unread[ACTIVITY_MESSAGE], ==, (guint64) G_MAXUINT + 5);
	g_assert_cmpint (state.latest_at, ==, 20);
	g_assert_cmpuint (state.latest_sequence, ==, G_MAXUINT64);
	g_assert_true (activity_clear (&state, &aggregate));
	g_assert_cmpuint (aggregate.unread[ACTIVITY_MESSAGE], ==, 5);
}

static void
test_activity_policies (void)
{
	ActivityPolicy policy;
	g_assert_true (activity_policy_parse ("HIGHLIGHTS", &policy));
	g_assert_cmpint (policy, ==, ACTIVITY_POLICY_HIGHLIGHTS);
	g_assert_false (activity_policy_parse ("invalid", &policy));
	g_assert_null (activity_policy_name ((ActivityPolicy) 999));
	g_assert_true (activity_policy_allows (ACTIVITY_POLICY_AUTO, ACTIVITY_OTHER));
	g_assert_false (activity_policy_allows (ACTIVITY_POLICY_MESSAGES, ACTIVITY_OTHER));
	g_assert_true (activity_policy_allows (ACTIVITY_POLICY_MESSAGES, ACTIVITY_MESSAGE));
	g_assert_false (activity_policy_allows (ACTIVITY_POLICY_PRIVATE, ACTIVITY_MESSAGE));
	g_assert_true (activity_policy_allows (ACTIVITY_POLICY_PRIVATE, ACTIVITY_PRIVATE));
	g_assert_false (activity_policy_allows (ACTIVITY_POLICY_HIGHLIGHTS, ACTIVITY_PRIVATE));
	g_assert_true (activity_policy_allows (ACTIVITY_POLICY_HIGHLIGHTS, ACTIVITY_HIGHLIGHT));
	g_assert_false (activity_policy_allows (ACTIVITY_POLICY_MUTED, ACTIVITY_HIGHLIGHT));
}

static void
test_activity_automatic_rules (void)
{
	GError *error = NULL;
	ActivityRules *rules = activity_rules_parse (
		"exact | Test Net | @server | messages\n"
		"exact | Test Net | #Foo[ | highlights\n"
		"wildcard | Test * | #Foo? | private\n"
		"wildcard | Other* | bot* | muted\n", &error);

	g_assert_no_error (error);
	g_assert_nonnull (rules);
	g_assert_cmpint (activity_rules_policy (rules, "TEST NET", "@server", TRUE), ==, ACTIVITY_POLICY_MESSAGES);
	g_assert_cmpint (activity_rules_policy (rules, "test net", "#foo{", TRUE), ==, ACTIVITY_POLICY_HIGHLIGHTS);
	g_assert_cmpint (activity_rules_policy (rules, "test net", "#foo{", FALSE), ==, ACTIVITY_POLICY_PRIVATE);
	g_assert_cmpint (activity_rules_policy (rules, "OtherOne", "BotName", FALSE), ==, ACTIVITY_POLICY_MUTED);
	g_assert_cmpint (activity_rules_policy (rules, "unmatched", "#foo{", TRUE), ==, ACTIVITY_POLICY_ALL);
	activity_rules_free (rules);
}

static void
test_activity_noisy_nicks (void)
{
	ActivityRules *rules = activity_rules_parse (
		"nick | Bot[ | other\nnick | QuietBot | muted\n", NULL);

	g_assert_nonnull (rules);
	g_assert_cmpint (activity_rules_filter_nick (rules, "bot{", TRUE, ACTIVITY_MESSAGE), ==, ACTIVITY_OTHER);
	g_assert_cmpint (activity_rules_filter_nick (rules, "bot{", FALSE, ACTIVITY_MESSAGE), ==, ACTIVITY_MESSAGE);
	g_assert_cmpint (activity_rules_filter_nick (rules, "quietBOT", TRUE, ACTIVITY_PRIVATE), ==, ACTIVITY_NONE);
	g_assert_cmpint (activity_rules_filter_nick (rules, "QuietBot", TRUE, ACTIVITY_HIGHLIGHT), ==, ACTIVITY_HIGHLIGHT);
	g_assert_cmpint (activity_rules_filter_nick (rules, "QuietB0t", TRUE, ACTIVITY_PRIVATE), ==, ACTIVITY_PRIVATE);
	g_assert_false (activity_policy_allows (ACTIVITY_POLICY_MESSAGES,
		activity_rules_filter_nick (rules, "Bot[", TRUE, ACTIVITY_MESSAGE)));
	activity_rules_free (rules);
}

static void
test_activity_invalid_rules (void)
{
	const char *invalid[] = {
		"exact | Net | #channel | invalid",
		"exact | Net | #channel | auto",
		"nick | Bot* | other",
		"nick | Bot | highlights",
		"wildcard | | missing | muted"
	};
	GString *too_many = g_string_new (NULL);
	char *too_large = g_strnfill (ACTIVITY_RULES_MAX_BYTES + 1, 'a');

	for (guint i = 0; i < G_N_ELEMENTS (invalid); ++i)
		g_assert_null (activity_rules_parse (invalid[i], NULL));
	for (guint i = 0; i < 129; ++i)
		g_string_append (too_many, "nick | Bot | other\n");
	g_assert_null (activity_rules_parse (too_many->str, NULL));
	g_assert_null (activity_rules_parse (too_large, NULL));
	g_string_free (too_many, TRUE);
	g_free (too_large);
}

void
activity_register_tests (void)
{
	g_test_add_func ("/activity/priority-and-order", test_activity_priority_and_order);
	g_test_add_func ("/activity/view", test_activity_view);
	g_test_add_func ("/activity/clear-and-close", test_activity_clear_and_close);
	g_test_add_func ("/activity/saturation", test_activity_saturation);
	g_test_add_func ("/activity/policies", test_activity_policies);
	g_test_add_func ("/activity/automatic-rules", test_activity_automatic_rules);
	g_test_add_func ("/activity/noisy-nicks", test_activity_noisy_nicks);
	g_test_add_func ("/activity/invalid-rules", test_activity_invalid_rules);
}

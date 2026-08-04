#include <glib.h>

#include "../ircv3-batch.h"

static void
test_batch_lifecycle (void)
{
	ircv3_batch_state *state = ircv3_batch_state_new ();
	ircv3_batch_info info;

	g_assert_true (ircv3_batch_start (state, "outer", "chathistory", "#test",
										 "request-1", NULL));
	g_assert_true (ircv3_batch_start (state, "inner", "labeled-response", NULL,
										 NULL, "outer"));
	g_assert_false (ircv3_batch_start (state, "inner", "duplicate", NULL,
										  NULL, NULL));
	g_assert_false (ircv3_batch_start (state, "orphan", "test", NULL,
										  NULL, "missing"));
	g_assert_cmpuint (ircv3_batch_count (state), ==, 2);
	g_assert_true (ircv3_batch_lookup (state, "outer", &info));
	g_assert_cmpstr (info.type, ==, "chathistory");
	g_assert_cmpstr (info.target, ==, "#test");
	g_assert_cmpstr (info.label, ==, "request-1");
	g_assert_true (ircv3_batch_end (state, "inner"));
	g_assert_true (ircv3_batch_end (state, "outer"));
	g_assert_false (ircv3_batch_end (state, "missing"));

	g_assert_true (ircv3_batch_start (state, "outer", "test", NULL, NULL, NULL));
	g_assert_true (ircv3_batch_start (state, "inner", "test", NULL, NULL,
										"outer"));
	g_assert_true (ircv3_batch_start (state, "leaf", "test", NULL, NULL,
										"inner"));
	g_assert_true (ircv3_batch_end (state, "outer"));
	g_assert_cmpuint (ircv3_batch_count (state), ==, 0);

	ircv3_batch_state_free (state);
}

static void
test_batch_limit (void)
{
	ircv3_batch_state *state = ircv3_batch_state_new ();
	guint i;

	for (i = 0; i < IRCV3_BATCH_LIMIT; i++)
	{
		char *id = g_strdup_printf ("batch-%u", i);
		g_assert_true (ircv3_batch_start (state, id, "test", NULL, NULL, NULL));
		g_free (id);
	}

	g_assert_false (ircv3_batch_start (state, "overflow", "test", NULL,
										  NULL, NULL));
	ircv3_batch_state_clear (state);
	g_assert_cmpuint (ircv3_batch_count (state), ==, 0);
	ircv3_batch_state_free (state);
}

static void
test_labels (void)
{
	guint64 counter = 0;
	char *first = ircv3_label_next (&counter);
	char *second = ircv3_label_next (&counter);

	g_assert_cmpstr (first, ==, "fabulor-1");
	g_assert_cmpstr (second, ==, "fabulor-2");
	g_free (first);
	g_free (second);
}

void
ircv3_batch_register_tests (void)
{
	g_test_add_func ("/ircv3/batch-lifecycle", test_batch_lifecycle);
	g_test_add_func ("/ircv3/batch-limit", test_batch_limit);
	g_test_add_func ("/ircv3/labels", test_labels);
}

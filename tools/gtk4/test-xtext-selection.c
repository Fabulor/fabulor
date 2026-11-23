/* Exercise the production widget/private hit test without an inspection API.
 * Stub only unrelated application services. Never publish to the clipboard. */
#ifdef _MSC_VER
/* Match production fe-gtk.vcxproj/fabulor.props for the legacy include only. */
#pragma warning(push)
#pragma warning(disable: 4244 4267 4996)
#endif
#include "../../src/fe-gtk/xtext.c"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

struct fabulorprefs prefs;
void safe_strcpy (char *d, const char *s, int n) { g_strlcpy (d, s, n); }
gsize strftime_utf8 (char *d, gsize n, const char *f, time_t t)
{ (void) f; (void) t; g_strlcpy (d, "[12:34:56]", n); return strlen (d); }
int url_last (int *s, int *e) { *s = *e = 0; return 0; }
int xtext_get_stamp_str (time_t t, char **s)
{ (void) t; *s = g_strdup ("[12:34:56]"); return 10; }
gboolean fabulor_ui_profile_enabled (void) { return FALSE; }
void fabulor_ui_profile_log (const char *c, const char *f, ...) { (void) c; (void) f; }
void theme_get_xtext_colors_for_widget (GtkWidget *w, XTextColor *p, size_t n)
{ (void) w; memset (p, 0, n * sizeof *p); }

typedef struct { GtkWidget *window; GtkXText *text; } Fixture;
static const char *long_prefix = "\00312-HostServ(services.example.net)-\017";
static const char *message = "You have requested vhost Aussie.Aussie.Aussie.Oi.Oi.Oi.";

static Fixture fixture (gboolean stamps)
{
 Fixture f;
 XTextColor palette[XTEXT_COLS] = {0};
 f.window = gtk_window_new ();
 f.text = GTK_XTEXT (gtk_xtext_new (palette, FALSE));
 gtk_window_set_child (GTK_WINDOW (f.window), GTK_WIDGET (f.text));
 gtk_widget_realize (f.window);
 g_assert_true (gtk_xtext_set_font (f.text, "Consolas 12"));
 gtk_xtext_set_indent (f.text, TRUE);
 f.text->buffer->time_stamp = stamps;
 f.text->buffer->indent = (stamps ? f.text->stamp_width : 0) + 10 * f.text->space_width;
 gtk_xtext_set_max_indent (f.text, f.text->buffer->indent);
 f.text->buffer->window_width = 2400;
 f.text->buffer->window_height = 400;
 gtk_adjustment_configure (f.text->adj, 0, 0, 1000, 1, 10, 20);
 f.text->pixel_offset = 0;
 return f;
}
static void finish (Fixture *f)
{
 gtk_window_destroy (GTK_WINDOW (f->window));
 while (g_main_context_pending (NULL)) g_main_context_iteration (NULL, FALSE);
}
static textentry *append (Fixture *f, const char *prefix, const char *body)
{
 gtk_xtext_append_indent (f->text->buffer, (unsigned char *) prefix, -1,
  (unsigned char *) body, -1, 1);
 return f->text->buffer->text_last;
}
static int hit (Fixture *f, textentry *ent, int x, int subline)
{
 int outside = FALSE;
 int result = gtk_xtext_find_x (f->text, x, ent, subline, 0, &outside);
 g_assert_false (outside);
 return result;
}
static char *selected (Fixture *f, textentry *ent, int start, int end, gboolean reverse)
{
 int length;
 f->text->selection_anchor_ent = f->text->selection_focus_ent = ent;
 f->text->selection_anchor_offset = reverse ? end : start;
 f->text->selection_focus_offset = reverse ? start : end;
 gtk_xtext_selection_apply_anchors (f->text, FALSE);
 f->text->buffer->last_ent_start = f->text->buffer->last_ent_end = ent;
 f->text->selection_buffer = f->text->buffer;
 return gtk_xtext_selection_get_text (f->text, &length);
}
static void check_body (Fixture *f, textentry *ent, const char *body, int origin, int raw_start)
{
 int end_x = origin + gtk_xtext_text_width (f->text, (unsigned char *) body, (int) strlen (body));
 int start = hit (f, ent, origin, 0);
 int end = hit (f, ent, end_x + 1, 0);
 char *copy;
 unsigned char *plain = gtk_xtext_strip_color ((unsigned char *) body,
  (int) strlen (body), NULL, NULL, NULL, TRUE);
 g_assert_cmpint (start, ==, raw_start);
 g_assert_cmpint (end, ==, ent->str_len);
 copy = selected (f, ent, start, end, TRUE);
 g_assert_cmpstr (copy, ==, (char *) plain); g_free (copy);
 copy = selected (f, ent, start, end, FALSE);
 g_assert_cmpstr (copy, ==, (char *) plain); g_free (copy);
 g_free (plain);
}
static void test_clipped (void)
{
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, long_prefix, message);
 XTextPrefix prefix;
 g_assert_cmpint (ent->indent, ==, MARGIN); /* Actual append-path clamp. */
 g_assert_true (gtk_xtext_prefix_geometry (f.text, ent, &prefix));
 check_body (&f, ent, message, f.text->buffer->indent, ent->left_len + 1);
 g_assert_cmpstr ((char *) ent->str + ent->left_len + 1, ==, message);
 finish (&f);
}
static void test_no_stamp (void)
{
 Fixture f = fixture (FALSE);
 textentry *ent = append (&f, long_prefix, message);
 XTextPrefix prefix;
 int origin = ent->indent + gtk_xtext_text_width (f.text, ent->str, ent->left_len + 1);
 g_assert_false (gtk_xtext_prefix_geometry (f.text, ent, &prefix));
 check_body (&f, ent, message, origin, ent->left_len + 1);
 finish (&f);
}
static void test_short_prefix (void)
{
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, "Nick", message);
 XTextPrefix prefix;
 g_assert_false (gtk_xtext_prefix_geometry (f.text, ent, &prefix));
 check_body (&f, ent, message, f.text->buffer->indent, ent->left_len + 1);
 finish (&f);
}
static void test_plain (void)
{
 Fixture f = fixture (FALSE);
 textentry *ent;
 gtk_xtext_append (f.text->buffer, (unsigned char *) message, -1, 1);
 ent = f.text->buffer->text_last;
 check_body (&f, ent, message, ent->indent, 0);
 finish (&f);
}
static void test_recalculated (void)
{
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, long_prefix, message);
 ent->indent = f.text->buffer->indent - gtk_xtext_text_width (f.text, ent->str, ent->left_len + 1);
 g_assert_cmpint (ent->indent, <, MARGIN); /* As after width/font recalculation. */
 check_body (&f, ent, message, f.text->buffer->indent, ent->left_len + 1);
 finish (&f);
}
static void test_unicode (void)
{
 const char *body = "Unicode caf\303\251 \360\237\230\212 final text";
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, long_prefix, body);
 check_body (&f, ent, body, f.text->buffer->indent, ent->left_len + 1);
 finish (&f);
}
static void test_formatted_body (void)
{
 const char *body = "\00306You have requested vhost\017 \002Aussie.Aussie.Oi.Oi.Oi.\002";
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, long_prefix, body);
 check_body (&f, ent, body, f.text->buffer->indent, ent->left_len + 1);
 finish (&f);
}
static void test_prefix_identity (void)
{
 Fixture f = fixture (TRUE);
 textentry *ent = append (&f, long_prefix, message);
 XTextPrefix prefix;
 int offset;
 g_assert_true (gtk_xtext_prefix_geometry (f.text, ent, &prefix));
 offset = hit (&f, ent, prefix.left + prefix.visible_width + prefix.ellipsis_width / 2, 0);
 g_assert_cmpint (offset, <, ent->left_len);
 g_assert_cmpint (offset, >=, 0);
 g_assert_cmpmem (ent->str, ent->left_len, long_prefix, strlen (long_prefix));
 finish (&f);
}
static void test_wrapped (void)
{
 Fixture f = fixture (TRUE);
 textentry *ent;
 int start;
 f.text->buffer->window_width = 450;
 ent = append (&f, long_prefix, message);
 g_assert_cmpint (ent->subline_count, >, 1);
 start = gtk_xtext_find_subline (f.text, ent, 1);
 g_assert_cmpint (hit (&f, ent, f.text->buffer->indent, 1), ==, start);
 finish (&f);
}

static void test_fixed_nickname (void)
{
 Fixture f = fixture (FALSE);
 unsigned char line[] = "<\00304Caf\303\251Long\017>\tHello there";
 const char *full = "Caf\303\251Long";
 textentry *ent;
 char *copy, *word;
 GList *matches;
 offsets_t result;
 int x, end;

 gtk_xtext_append_nick (f.text->buffer, line, sizeof line - 1,
  (int) (strchr ((char *) line, '\t') - (char *) line + 1),
  1, 1, 3 + (int) strlen (full), 4);
 ent = f.text->buffer->text_last;
 g_assert_nonnull (ent->nickname);
 g_assert_cmpstr ((char *) ent->str, ==, "<\00304Caf\303\251" FABULOR_NAME_ELLIPSIS "\017> Hello there");
 end = ent->nickname->ellipsis + XTEXT_NAME_ELLIPSIS_LEN;
 copy = selected (&f, ent, 0, ent->str_len, TRUE);
 g_assert_cmpstr (copy, ==, "<Caf\303\251Long> Hello there");
 g_free (copy);
 copy = selected (&f, ent, ent->nickname->nick_start, end, FALSE);
 g_assert_cmpstr (copy, ==, full);
 g_free (copy);
 x = ent->indent + gtk_xtext_text_width (f.text, ent->str, ent->nickname->ellipsis) + 1;
 word = gtk_xtext_get_word (f.text, x, 1, NULL, NULL, NULL, NULL);
 g_assert_cmpstr (word, ==, full);
 gtk_xtext_search_init (f.text->buffer, "Long", case_match, NULL);
 matches = gtk_xtext_search_textentry (f.text->buffer, ent);
 g_assert_nonnull (matches);
 result.u = GPOINTER_TO_UINT (matches->data);
 g_assert_cmpint (result.o.start, ==, ent->nickname->ellipsis);
 g_assert_cmpint (result.o.end, ==, end + 1); /* Includes the trailing IRC reset. */
 g_list_free (matches);
 finish (&f);
}

static void test_fixed_action_and_disabled_limit (void)
{
 Fixture f = fixture (FALSE);
 unsigned char line[] = "*\tNickname waved";
 textentry *ent;
 char *copy;

 gtk_xtext_append_nick (f.text->buffer, line, sizeof line - 1, 2, 1, 2, 8, 4);
 ent = f.text->buffer->text_last;
 g_assert_cmpstr ((char *) ent->str, ==, "* Nick" FABULOR_NAME_ELLIPSIS " waved");
 copy = selected (&f, ent, 0, ent->str_len, FALSE);
 g_assert_cmpstr (copy, ==, "* Nickname waved");
 g_free (copy);
 gtk_xtext_append_nick (f.text->buffer, line, sizeof line - 1, 2, 1, 2, 8, 0);
 ent = f.text->buffer->text_last;
 g_assert_null (ent->nickname);
 g_assert_cmpstr ((char *) ent->str, ==, "* Nickname waved");
 finish (&f);
}
int main (int argc, char **argv)
{
 gtk_init ();
 g_test_init (&argc, &argv, NULL);
 g_test_add_func ("/xtext/selection/clipped-prefix", test_clipped);
 g_test_add_func ("/xtext/selection/no-timestamp", test_no_stamp);
 g_test_add_func ("/xtext/selection/channel-prefix", test_short_prefix);
 g_test_add_func ("/xtext/selection/plain-server", test_plain);
 g_test_add_func ("/xtext/selection/recalculated-indent", test_recalculated);
 g_test_add_func ("/xtext/selection/unicode", test_unicode);
 g_test_add_func ("/xtext/selection/formatted-body", test_formatted_body);
 g_test_add_func ("/xtext/selection/prefix-identity", test_prefix_identity);
 g_test_add_func ("/xtext/selection/wrapped-line", test_wrapped);
 g_test_add_func ("/xtext/nickname/fixed-limit-identity", test_fixed_nickname);
 g_test_add_func ("/xtext/nickname/action-and-disabled-limit", test_fixed_action_and_disabled_limit);
 return g_test_run ();
}

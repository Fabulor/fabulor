/* Default: headless production-parser tests. --renderer adds hidden widgets
 * with presentation stubbed out. Neither mode uses IRC or a user profile. */
#include <gtk/gtk.h>
static void no_present (GtkWindow *window) { (void)window; }
#define gtk_window_present no_present
#include "../../src/fe-gtk/addon-panel.c"
static char *last_event, *last_values;

int is_session (session *sess) { return sess != NULL; }
int plugin_emit_command (session *sess, char *name, char *word[], char *eol[])
{ gsize size; (void)sess; (void)eol; g_assert_cmpstr(name,==,"_FABULOR_UI_EVENT");
 g_free(last_event); g_free(last_values); last_event=g_strdup(word[3]); last_values=(char*)g_base64_decode(word[4],&size);
 last_values=g_realloc(last_values,size+1); last_values[size]=0; return 1; }
GtkWidget *gtkutil_window_new (char *title, char *role, int width, int height, int flags)
{ (void)title; (void)role; (void)width; (void)height; (void)flags; return gtk_window_new(); }

static AddonPanel *parse (const char *xml)
{
 char *encoded=g_base64_encode((const guchar*)xml,strlen(xml));
 AddonPanel *p=parse_panel(encoded); g_free(encoded); return p;
}
static void valid_schema (void)
{
 AddonPanel *p=parse("<panel title='Test' context='Net / #test'><page id='overview' title='Overview'>"
 "<field id='status' type='status' label='Review' value='Waiting' state='waiting'/>"
 "<field id='seconds' type='int' label='Seconds' value='1800' min='1' max='3600'/>"
 "<field id='check' type='check' label='Exempt' value='false'/>"
 "<field id='choice' type='choice' label='Preset' value='HATE' choices='HATE|POLITICS'/>"
 "</page><button id='apply' label='Apply'/></panel>");
 g_assert_nonnull(p); g_assert_cmpuint(p->pages->len,==,1); g_assert_null(panels); panel_free(p);
}
static void invalid_schemas (void)
{
 const char *cases[]={
 "<panel title='Test' context=''><page id='x' title='X'><script/></page></panel>",
 "<panel title='Test' context=''><page id='x' title='X'><field id='x' type='label' label='X' value='X'/></page></panel>",
 "<panel title='Test' context=''><page id='x' title='X'><field id='f' type='check' label='X' value='yes'/></page></panel>",
 "<panel title='Test' context=''><page id='x' title='X'><field id='f' type='int' label='X' value='4000' min='1' max='3600'/></page></panel>",
 "<panel title='Test' context=''><page id='x' title='X'><field id='f' type='status' label='X' value='X' state='arbitrary'/></page></panel>",
 "<panel title='Test' context='' command='PRIVMSG'><page id='x' title='X'/></panel>",
 "<panel title='Test' context=''/>", "<panel title='Test' context=''><page id='x' title='X'/></panel><panel title='Again' context=''/>", NULL};
 int i; for(i=0;cases[i];i++) g_assert_null(parse(cases[i]));
}
static void page_buttons (void)
{
 AddonPanel *p=parse("<panel title='Test' context=''><page id='p' title='Page'><button id='local' label='Local'/></page><button id='global' label='Global'/></panel>");
 PanelPage *page;
 g_assert_nonnull(p); page=g_ptr_array_index(p->pages,0); g_assert_cmpuint(page->buttons->len,==,1); panel_free(p);
 g_assert_null(parse("<panel title='Test' context=''><page id='p' title='Page'><button id='same' label='Local'/></page><button id='same' label='Global'/></panel>"));
 g_assert_null(parse("<panel title='Test' context=''><page id='p' title='Page'><button id='b' label='Local' command='PRIVMSG'/></page></panel>"));
 { GString *xml=g_string_new("<panel title='Test' context=''><page id='p' title='Page'>"); int i;
 for(i=0;i<7;i++) g_string_append_printf(xml,"<button id='b%d' label='Local'/>",i);
 g_string_append(xml,"</page></panel>"); g_assert_null(parse(xml->str)); g_string_free(xml,TRUE); }
}
/* Authentication has its own mocked native harness; these UI fixtures never
 * open a credential dialog or access Credential Manager. */
OperLogin *oper_login_new (session *sess) { (void)sess; return NULL; }
void oper_login_open (OperLogin *a, GtkWindow *parent) { (void)a; (void)parent; }
const char *oper_login_status (OperLogin *a) { (void)a; return "fixture unavailable"; }
void oper_login_free (OperLogin *a) { (void)a; }
static GtkWidget *find_action (GtkWidget *root, const char *id)
{
 GtkWidget *child; const char *action=g_object_get_data(G_OBJECT(root),"addon-action");
 if (action && !strcmp(action,id)) return root;
 for(child=gtk_widget_get_first_child(root);child;child=gtk_widget_get_next_sibling(child)) { GtkWidget *found=find_action(child,id); if(found) return found; }
 return NULL;
}
static void resource_limits (void)
{
 GString *xml=g_string_new("<panel title='Test' context=''><page id='page' title='Page'>"); int i;
 for(i=0;i<65;i++) g_string_append_printf(xml,"<field id='f%d' type='label' label='X' value='X'/>",i);
 g_string_append(xml,"</page></panel>"); g_assert_null(parse(xml->str)); g_string_free(xml,TRUE);
 g_assert_null(parse_panel("not base64"));
 g_assert_null(panels);
 g_assert_true(fe_addon_panel_command(NULL,"CLOSE","absent",""));
 g_assert_false(fe_addon_panel_command(NULL,"OPEN","invalid-token",""));
 g_assert_false(fe_addon_panel_exists("absent"));
}
static void assert_form_layout (AddonPanel *p)
{
 guint i,j; GtkWidget *root=gtk_window_get_child(GTK_WINDOW(p->window));
 GtkWidget *body=gtk_widget_get_next_sibling(gtk_widget_get_first_child(root)),*stack=gtk_widget_get_last_child(body);
 g_assert_true(GTK_IS_STACK(stack));
 g_assert_cmpint(gtk_widget_get_halign(gtk_widget_get_last_child(root)),==,GTK_ALIGN_END);
 for(i=0;i<p->pages->len;i++) { PanelPage *page=g_ptr_array_index(p->pages,i);
  GtkWidget *page_root=gtk_stack_get_child_by_name(GTK_STACK(stack),page->id),*scroll=gtk_widget_get_first_child(page_root);
  g_assert_true(GTK_IS_SCROLLED_WINDOW(scroll)); g_assert_true(gtk_widget_get_vexpand(scroll));
  for(j=0;j<page->buttons->len;j++) { PanelButton *button=g_ptr_array_index(page->buttons,j); GtkWidget *widget=find_action(page_root,button->id);
   g_assert_nonnull(widget); g_assert_true(gtk_widget_is_ancestor(widget,page_root)); g_assert_false(gtk_widget_is_ancestor(widget,scroll));
  }
  for(j=0;j<page->fields->len;j++) {
   PanelField *f=g_ptr_array_index(page->fields,j); GtkWidget *cell=f->widget,*grid,*label; int col,row,width,height;
   if(f->lamp) cell=gtk_widget_get_parent(cell);
   grid=gtk_widget_get_parent(cell); g_assert_true(GTK_IS_GRID(grid)); g_assert_true(gtk_widget_is_ancestor(cell,scroll));
   gtk_grid_query_child(GTK_GRID(grid),cell,&col,&row,&width,&height);
   g_assert_cmpint(row,==,(int)j); g_assert_cmpint(height,==,1);
   if(!strcmp(f->type,"check")) {
    g_assert_cmpint(col,==,0); g_assert_cmpint(width,==,2);
    g_assert_cmpstr(gtk_check_button_get_label(GTK_CHECK_BUTTON(f->widget)),==,f->label);
   } else {
    g_assert_cmpint(col,==,1); g_assert_cmpint(width,==,1);
    label=gtk_grid_get_child_at(GTK_GRID(grid),0,(int)j); g_assert_true(GTK_IS_LABEL(label));
    g_assert_cmpfloat(gtk_label_get_xalign(GTK_LABEL(label)),==,0);
    g_assert_cmpint(gtk_widget_get_halign(label),==,GTK_ALIGN_START);
    if(GTK_IS_LABEL(f->widget)) g_assert_cmpfloat(gtk_label_get_xalign(GTK_LABEL(f->widget)),==,0);
   }
  }
 }
}
static void hidden_renderer (void)
{
 server serv={0}; session sess={0}; char *wire; AddonPanel *p; PanelPage *page; PanelField *field;
 const char *xml="<panel title='Hidden fixture' context='Offline / #fixture'><page id='page' title='Overview'>"
 "<field id='status' type='status' label='Health' value='Waiting' state='waiting'/>"
 "<field id='check' type='check' label='Enabled' value='false'/>"
 "<field id='seconds' type='int' label='Seconds' value='1800' min='1' max='3600'/>"
 "<field id='text' type='text' label='Text' value='Unapplied draft'/>"
 "<field id='choice' type='choice' label='Preset' value='HATE' choices='HATE|POLITICS'/>"
 "<field id='plain' type='label' label='Readonly' value='Plain text'/><button id='local' label='Local'/></page><button id='apply' label='Apply'/></panel>";
 serv.id=7; g_strlcpy(serv.servername,"offline.fixture",sizeof serv.servername); sess.server=&serv; g_strlcpy(sess.channel,"#fixture",sizeof sess.channel);
 wire=g_base64_encode((const guchar*)xml,strlen(xml));
 g_assert_true(fe_addon_panel_command(&sess,"OPEN","fixture",wire)); g_free(wire);
 p=g_hash_table_lookup(panels,"fixture"); g_assert_nonnull(p); g_assert_false(gtk_widget_get_visible(p->window)); assert_form_layout(p);
 page=g_ptr_array_index(p->pages,0); field=g_ptr_array_index(page->fields,1); gtk_check_button_set_active(GTK_CHECK_BUTTON(field->widget),TRUE);
 g_assert_nonnull(find_action(p->window,"local"));
 g_signal_emit_by_name(find_action(p->window,"local"),"clicked"); g_assert_cmpstr(last_event,==,"local");
 emit_event(p,"apply"); g_assert_cmpstr(last_event,==,"apply"); g_assert_nonnull(strstr(last_values,"value=\"true\""));
 g_assert_nonnull(strstr(last_values,"value=\"1800\"")); g_assert_nonnull(strstr(last_values,"Unapplied draft"));
 g_assert_nonnull(strstr(last_values,"value=\"HATE\""));
 xml="<panel title='Update' context=''><page id='updates' title='Updates'><field id='status' type='status' label='Health' value='OK' state='running'/></page></panel>";
 wire=g_base64_encode((const guchar*)xml,strlen(xml)); g_assert_true(fe_addon_panel_command(&sess,"UPDATE","fixture",wire)); g_free(wire);
 xml="<panel title='Update' context=''><page id='updates' title='Updates'><button id='injected' label='Invalid update'/></page></panel>";
 wire=g_base64_encode((const guchar*)xml,strlen(xml)); g_assert_false(fe_addon_panel_command(&sess,"UPDATE","fixture",wire)); g_free(wire);
 g_assert_true(gtk_check_button_get_active(GTK_CHECK_BUTTON(field->widget)));
 field=g_ptr_array_index(page->fields,3); g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(field->widget)),==,"Unapplied draft");
 field=g_ptr_array_index(page->fields,0); g_assert_cmpstr(gtk_label_get_text(GTK_LABEL(field->widget)),==,"OK");
 { GtkWidget *keep=g_object_ref(p->window);
 g_assert_true(fe_addon_panel_command(&sess,"CLOSE","fixture","")); g_assert_false(fe_addon_panel_exists("fixture"));
 xml="<panel title='Reopened' context=''><page id='new_page' title='New'/></panel>";
 wire=g_base64_encode((const guchar*)xml,strlen(xml)); g_assert_true(fe_addon_panel_command(&sess,"OPEN","fixture",wire)); g_free(wire);
 g_object_unref(keep); g_assert_true(fe_addon_panel_exists("fixture"));
 g_assert_true(fe_addon_panel_command(&sess,"CLOSE","fixture","")); }
 while(g_main_context_iteration(NULL,FALSE));
 g_assert_false(fe_addon_panel_exists("fixture")); g_assert_null(panels);
 xml="<panel title='Parent close' context='Offline'><page id='page' title='Page'/></panel>";
 wire=g_base64_encode((const guchar*)xml,strlen(xml));
 g_assert_true(fe_addon_panel_command(&sess,"OPEN","parent_fixture",wire)); g_free(wire);
 p=g_hash_table_lookup(panels,"parent_fixture"); gtk_window_destroy(GTK_WINDOW(p->window));
 while(g_main_context_iteration(NULL,FALSE));
 g_assert_null(panels); g_assert_cmpstr(last_event,==,"closed");
 g_clear_pointer(&last_event,g_free); g_clear_pointer(&last_values,g_free);
}
static char *schema_path;
static void supplied_schema (void)
{
 char *xml=NULL, *wire; gsize size=0; server serv={0}; session sess={0}; AddonPanel *p; guint i; gboolean clicked_local=FALSE;
 g_assert_true(g_file_get_contents(schema_path,&xml,&size,NULL)); g_assert_cmpuint(size,<=,WIRE_LIMIT);
 serv.id=9; g_strlcpy(serv.servername,"offline.fixture",sizeof serv.servername); sess.server=&serv; g_strlcpy(sess.channel,"#fixture",sizeof sess.channel);
 wire=g_base64_encode((const guchar*)xml,size); g_free(xml);
 g_assert_true(fe_addon_panel_command(&sess,"OPEN","supplied_schema",wire)); g_free(wire);
 p=g_hash_table_lookup(panels,"supplied_schema"); g_assert_false(gtk_widget_get_visible(p->window)); assert_form_layout(p);
 for(i=0;i<p->pages->len;i++) { PanelPage *page=g_ptr_array_index(p->pages,i);
  if(page->buttons->len) { PanelButton *button=g_ptr_array_index(page->buttons,0); GtkWidget *widget=find_action(p->window,button->id);
   g_assert_nonnull(widget); g_signal_emit_by_name(widget,"clicked"); g_assert_cmpstr(last_event,==,button->id); clicked_local=TRUE; break; }
 }
 g_assert_true(clicked_local); g_assert_nonnull(strstr(last_values,"<values>")); g_assert_true(g_utf8_validate(last_values,-1,NULL));
 g_assert_true(fe_addon_panel_command(&sess,"CLOSE","supplied_schema",""));
 while(g_main_context_iteration(NULL,FALSE));
 g_assert_null(panels); g_clear_pointer(&last_event,g_free); g_clear_pointer(&last_values,g_free);
}
int main (int argc, char **argv)
{
 gboolean renderer=argc>1 && !strcmp(argv[1],"--renderer");
 if (renderer && argc==4 && !strcmp(argv[2],"--schema")) { schema_path=argv[3]; argc=2; argv[2]=NULL; }
 if (renderer) { argc--; argv[1]=NULL; gtk_init(); }
 g_test_init(&argc,&argv,NULL);
 g_test_add_func("/addon-panel/valid",valid_schema);
 g_test_add_func("/addon-panel/invalid",invalid_schemas);
 g_test_add_func("/addon-panel/page-buttons",page_buttons);
 g_test_add_func("/addon-panel/resource-limits-and-inactive",resource_limits);
 if (renderer) g_test_add_func("/addon-panel/hidden-renderer-and-lifecycle",hidden_renderer);
 if (schema_path) g_test_add_func("/addon-panel/supplied-schema",supplied_schema);
 return g_test_run();
}

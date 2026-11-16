/* Native, declarative add-on panels. Schemas/events never become IRC commands.
 * Optional host-owned OPER login requires its own native user confirmation.
 * Wire format: bounded base64 UTF-8 GMarkup, no files, scripts or custom markup.
 */
#include <string.h>
#include <gtk/gtk.h>
#include "../common/fabulor.h"
#include "../common/fabulorc.h"
#include "../common/fabulor-plugin.h"
#include "../common/plugin.h"
#include "../common/util.h"
#include "gtkutil.h"
#include "addon-panel.h"
#include "oper-login.h"

#define PANEL_LIMIT 8
#define FIELD_LIMIT 64
#define WIRE_LIMIT 32768

typedef struct {
 char *id, *type, *label, *value, *state, *choices;
 int minimum, maximum;
 GtkWidget *widget, *lamp;
} PanelField;
typedef struct { char *id, *title; GPtrArray *fields, *buttons; } PanelPage;
typedef struct { char *id, *label; } PanelButton;
typedef struct {
 char *token, *title, *context;
 GPtrArray *pages, *buttons;
 GHashTable *ids;
 GtkWidget *window;
 session *session;
 int server_id;
 char *server, *channel;
 gboolean closing;
 OperLogin *oper_login;
 const char *oper_login_message;
} AddonPanel;
typedef struct { AddonPanel *panel; PanelPage *page; int depth, fields; gboolean ended; } PanelParser;
static GHashTable *panels; /* Allocated only on first successful OPEN. */
static void emit_destroyed (AddonPanel *p);

static gboolean identifier (const char *s)
{
 const char *p;
 if (!s || !*s || strlen(s) > 64) return FALSE;
 for (p=s; *p; p++) if (!g_ascii_isalnum(*p) && *p != '_') return FALSE;
 return TRUE;
}
static const char *attribute (const char **names, const char **values, const char *name)
{
 int i; for (i=0; names && names[i]; i++) if (!strcmp(names[i], name)) return values[i];
 return NULL;
}
static void invalid (GError **error) { g_set_error_literal(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, "Invalid add-on panel schema"); }
static gboolean attributes (const char **names, const char *allowed, GError **error)
{
 int i; char **list=g_strsplit(allowed, "|", -1);
 for (i=0; names && names[i]; i++) {
  int j; gboolean found=FALSE;
  for (j=0; list[j]; j++) if (!strcmp(names[i],list[j])) found=TRUE;
  if (!found) { g_strfreev(list); invalid(error); return FALSE; }
 }
 g_strfreev(list); return TRUE;
}
static char *bounded (const char *value, gsize maximum)
{
 if (!value || strlen(value)>maximum || !g_utf8_validate(value,-1,NULL)) return NULL;
 return g_strdup(value);
}
static gboolean number (const char *s, int *out)
{
 char *end; gint64 n;
 if (!s || !*s) return FALSE;
 n=g_ascii_strtoll(s,&end,10);
 if (*end || n < -1000000 || n > 1000000) return FALSE;
 *out=(int)n; return TRUE;
}
static gboolean state_valid (const char *s)
{ return s && (!strcmp(s,"off") || !strcmp(s,"waiting") || !strcmp(s,"running") || !strcmp(s,"error")); }
static void field_free (gpointer data)
{
 PanelField *f=data;
 g_free(f->id); g_free(f->type); g_free(f->label); g_free(f->value); g_free(f->state); g_free(f->choices); g_free(f);
}
static void button_free (gpointer data);
static void page_free (gpointer data)
{ PanelPage *p=data; g_free(p->id); g_free(p->title); g_ptr_array_unref(p->fields); g_ptr_array_unref(p->buttons); g_free(p); }
static void button_free (gpointer data)
{ PanelButton *b=data; g_free(b->id); g_free(b->label); g_free(b); }
static void unregister_panel (AddonPanel *p)
{
 if (panels && p->token && g_hash_table_lookup(panels,p->token)==p) {
  g_hash_table_remove(panels,p->token);
  if (!g_hash_table_size(panels)) g_clear_pointer(&panels,g_hash_table_unref);
 }
}
static void panel_free (gpointer data)
{
 AddonPanel *p=data;
 unregister_panel(p);
 g_clear_pointer(&p->oper_login,oper_login_free);
 if (p->window && !p->closing) emit_destroyed(p);
 g_free(p->token); g_free(p->title); g_free(p->context); g_free(p->server); g_free(p->channel);
 g_ptr_array_unref(p->pages); g_ptr_array_unref(p->buttons); g_hash_table_unref(p->ids); g_free(p);
}
static gboolean unique_id (AddonPanel *p, const char *id)
{
 if (!identifier(id) || g_hash_table_contains(p->ids,id)) return FALSE;
 g_hash_table_add(p->ids,g_strdup(id)); return TRUE;
}
static void start_element (GMarkupParseContext *ctx, const char *name, const char **names, const char **values, gpointer data, GError **error)
{
 PanelParser *s=data; AddonPanel *p=s->panel;
 (void)ctx;
 if (s->depth==0 && !s->ended && !strcmp(name,"panel")) {
  if (!attributes(names,"title|context",error)) return;
  p->title=bounded(attribute(names,values,"title"),128);
  p->context=bounded(attribute(names,values,"context"),256);
  if (!p->title || !*p->title || !p->context) { invalid(error); return; }
 } else if (s->depth==1 && !strcmp(name,"page")) {
  PanelPage *page;
  if (!attributes(names,"id|title",error) || p->pages->len>=8) { if (!*error) invalid(error); return; }
  page=g_new0(PanelPage,1); page->fields=g_ptr_array_new_with_free_func(field_free); page->buttons=g_ptr_array_new_with_free_func(button_free);
  page->id=bounded(attribute(names,values,"id"),64); page->title=bounded(attribute(names,values,"title"),128);
  if (!unique_id(p,page->id) || !page->title) { page_free(page); invalid(error); return; }
  g_ptr_array_add(p->pages,page); s->page=page;
 } else if (s->depth==2 && s->page && !strcmp(name,"field")) {
  PanelField *f; int value;
  if (!attributes(names,"id|type|label|value|state|choices|min|max",error) || ++s->fields>FIELD_LIMIT) { if (!*error) invalid(error); return; }
  f=g_new0(PanelField,1);
  f->id=bounded(attribute(names,values,"id"),64); f->type=bounded(attribute(names,values,"type"),16);
  f->label=bounded(attribute(names,values,"label"),256); f->value=bounded(attribute(names,values,"value"),512);
  f->state=g_strdup(attribute(names,values,"state")); f->choices=bounded(attribute(names,values,"choices"),1024);
  if (!unique_id(p,f->id) || !f->type || !f->label || !f->value) goto bad_field;
  if (!strcmp(f->type,"int")) {
   if (!number(attribute(names,values,"min"),&f->minimum) || !number(attribute(names,values,"max"),&f->maximum) ||
       f->minimum>f->maximum || !number(f->value,&value) || value<f->minimum || value>f->maximum) goto bad_field;
  } else if (!strcmp(f->type,"check")) {
   if (strcmp(f->value,"true") && strcmp(f->value,"false")) goto bad_field;
  } else if (!strcmp(f->type,"status")) {
   if (!state_valid(f->state)) goto bad_field;
  } else if (!strcmp(f->type,"choice")) {
   char **choices; int i; gboolean found=FALSE, valid=TRUE;
   if (!f->choices) goto bad_field;
   choices=g_strsplit(f->choices,"|",-1);
   for (i=0; choices[i]; i++) { if (!strcmp(choices[i],f->value)) found=TRUE; if (!*choices[i] || strlen(choices[i])>64) valid=FALSE; }
   g_strfreev(choices); if (!found || !valid || i>16) goto bad_field;
  } else if (strcmp(f->type,"label") && strcmp(f->type,"text")) goto bad_field;
  g_ptr_array_add(s->page->fields,f); s->depth++; return;
 bad_field:
  field_free(f); invalid(error); return;
 } else if ((s->depth==1 || (s->depth==2 && s->page)) && !strcmp(name,"button")) {
  PanelButton *b; GPtrArray *buttons=s->depth==2?s->page->buttons:p->buttons;
  if (!attributes(names,"id|label",error) || buttons->len>=6) { if (!*error) invalid(error); return; }
  b=g_new0(PanelButton,1); b->id=bounded(attribute(names,values,"id"),64); b->label=bounded(attribute(names,values,"label"),128);
  if (!unique_id(p,b->id) || !strcmp(b->id,"closed") || !b->label) { button_free(b); invalid(error); return; }
  g_ptr_array_add(buttons,b);
 } else { invalid(error); return; }
 s->depth++;
}
static void end_element (GMarkupParseContext *ctx, const char *name, gpointer data, GError **error)
{
 PanelParser *s=data; (void)ctx; (void)error;
 s->depth--; if (!strcmp(name,"page")) s->page=NULL; if (!s->depth) s->ended=TRUE;
}
static void text_data (GMarkupParseContext *ctx, const char *text, gsize size, gpointer data, GError **error)
{
 gsize i; (void)ctx; (void)data;
 for (i=0;i<size;i++) if (!g_ascii_isspace(text[i])) { invalid(error); return; }
}
static const GMarkupParser parser={start_element,end_element,text_data,NULL,NULL};
static char *decode (const char *payload, gsize *size)
{
 gsize i; guchar *bytes;
 if (!payload || strlen(payload)>WIRE_LIMIT*2 || strlen(payload)%4) return NULL;
 for (i=0;payload[i];i++) if (!g_ascii_isalnum(payload[i]) && payload[i]!='+' && payload[i]!='/' && payload[i]!='=') return NULL;
 bytes=g_base64_decode(payload,size);
 if (*size>WIRE_LIMIT || memchr(bytes,0,*size) || !g_utf8_validate((char*)bytes,*size,NULL)) { g_free(bytes); return NULL; }
 bytes=g_realloc(bytes,*size+1); bytes[*size]=0; return (char*)bytes;
}
static void lamp_set (GtkWidget *label, const char *state)
{
 const char *colour=!strcmp(state,"running")?"#36a657":!strcmp(state,"waiting")?"#d59b19":!strcmp(state,"error")?"#d44747":"#888888";
 char *markup=g_strdup_printf("<span foreground=\"%s\">●</span>",colour);
 gtk_label_set_markup(GTK_LABEL(label),markup); g_free(markup);
}
static char *field_value (PanelField *f)
{
 if (!strcmp(f->type,"check")) return g_strdup(gtk_check_button_get_active(GTK_CHECK_BUTTON(f->widget))?"true":"false");
 if (!strcmp(f->type,"int")) return g_strdup_printf("%d",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(f->widget)));
 if (!strcmp(f->type,"text")) return g_strdup(gtk_editable_get_text(GTK_EDITABLE(f->widget)));
 if (!strcmp(f->type,"choice")) { GtkStringObject *item=gtk_drop_down_get_selected_item(GTK_DROP_DOWN(f->widget)); return g_strdup(item?gtk_string_object_get_string(item):""); }
 return NULL;
}
static gboolean context_valid (AddonPanel *p)
{
 return is_session(p->session) && p->session->server->id==p->server_id &&
        !strcmp(p->session->server->servername,p->server) && !strcmp(p->session->channel,p->channel);
}
/* Parent-window destruction need not emit close-request. Notify ownership
 * cleanup without touching widgets which are already being finalised. */
static void emit_destroyed (AddonPanel *p)
{
 char *word[PDIWORDS+1], *eol[PDIWORDS+1]; guint i;
 if (!context_valid(p)) return;
 for (i=0;i<=PDIWORDS;i++) word[i]=eol[i]="";
 word[1]=eol[1]="_FABULOR_UI_EVENT"; word[2]=eol[2]=p->token;
 word[3]=eol[3]="closed"; word[4]=eol[4]="PHZhbHVlcy8+";
 plugin_emit_command(p->session,word[1],word,eol);
}
static void emit_event (AddonPanel *p, const char *event)
{
 char *word[PDIWORDS+1], *eol[PDIWORDS+1], *payload; guint i,j;
 GtkWidget *keep=g_object_ref(p->window);
 GString *xml=g_string_new("<values>");
 if (!context_valid(p)) { g_string_free(xml,TRUE); g_object_unref(keep); return; }
 for (i=0;strcmp(event,"closed") && i<p->pages->len;i++) { PanelPage *page=g_ptr_array_index(p->pages,i);
  for (j=0;j<page->fields->len;j++) { PanelField *f=g_ptr_array_index(page->fields,j); char *value=field_value(f);
   if (value) { char *escaped=g_markup_escape_text(value,-1); g_string_append_printf(xml,"<field id=\"%s\" value=\"%s\"/>",f->id,escaped); g_free(escaped); g_free(value); }
  }
 }
 g_string_append(xml,"</values>");
 if (xml->len>WIRE_LIMIT) { g_string_free(xml,TRUE); g_object_unref(keep); return; }
 payload=g_base64_encode((guchar*)xml->str,xml->len); g_string_free(xml,TRUE);
 for (i=0;i<=PDIWORDS;i++) word[i]=eol[i]="";
 word[1]=eol[1]="_FABULOR_UI_EVENT"; word[2]=eol[2]=p->token; word[3]=eol[3]=(char*)event; word[4]=eol[4]=payload;
 /* Dispatch registered hooks directly. Never feed values or action IDs to the
  * command parser or IRC fallback, including when the owning add-on vanished. */
 plugin_emit_command(p->session,word[1],word,eol);
 g_free(payload); g_object_unref(keep);
}
static void clicked (GtkButton *button, gpointer data)
{ AddonPanel *p=data; emit_event(p,g_object_get_data(G_OBJECT(button),"addon-action")); }
static gboolean close_request (GtkWindow *window, gpointer data)
{
 AddonPanel *p=data; (void)window;
 if (!p->closing) { p->closing=TRUE; unregister_panel(p); g_clear_pointer(&p->oper_login,oper_login_free); emit_event(p,"closed"); }
 return FALSE;
}
static void close_clicked (GtkButton *button, gpointer data)
{ (void)button; gtk_window_close(GTK_WINDOW(((AddonPanel*)data)->window)); }
static GtkWidget *build_buttons (AddonPanel *p, GPtrArray *buttons)
{
 GtkWidget *box=gtk_flow_box_new(); guint i;
 gtk_widget_set_halign(box,GTK_ALIGN_START);
 gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(box),GTK_SELECTION_NONE);
 gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(box),1); gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(box),3);
 gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(box),6); gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(box),6);
 for (i=0;i<buttons->len;i++) { PanelButton *b=g_ptr_array_index(buttons,i); GtkWidget *button=gtk_button_new_with_label(b->label); g_object_set_data_full(G_OBJECT(button),"addon-action",g_strdup(b->id),g_free); g_signal_connect(button,"clicked",G_CALLBACK(clicked),p); gtk_flow_box_insert(GTK_FLOW_BOX(box),button,-1); }
 return box;
}
static void build_window (AddonPanel *p)
{
 GtkWidget *root,*body,*stack,*sidebar,*footer,*heading,*close; guint i,j;
 GtkSizeGroup *labels=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
 p->window=gtkutil_window_new(p->title,"addon-control-panel",900,640,2);
 g_object_set_data_full(G_OBJECT(p->window),"addon-panel",p,panel_free);
 g_signal_connect(p->window,"close-request",G_CALLBACK(close_request),p);
 root=gtk_box_new(GTK_ORIENTATION_VERTICAL,12); gtk_widget_set_margin_start(root,16); gtk_widget_set_margin_end(root,16); gtk_widget_set_margin_top(root,16); gtk_widget_set_margin_bottom(root,16);
 heading=gtk_label_new(p->context); gtk_label_set_xalign(GTK_LABEL(heading),0); gtk_label_set_wrap(GTK_LABEL(heading),TRUE); gtk_label_set_wrap_mode(GTK_LABEL(heading),PANGO_WRAP_WORD_CHAR); gtk_box_append(GTK_BOX(root),heading);
 body=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,16); gtk_widget_set_vexpand(body,TRUE);
 stack=gtk_stack_new(); gtk_widget_set_hexpand(stack,TRUE); gtk_widget_set_vexpand(stack,TRUE);
 sidebar=gtk_stack_sidebar_new(); gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar),GTK_STACK(stack)); gtk_widget_set_size_request(sidebar,190,-1);
 gtk_box_append(GTK_BOX(body),sidebar); gtk_box_append(GTK_BOX(body),stack); gtk_box_append(GTK_BOX(root),body);
 for (i=0;i<p->pages->len;i++) {
  PanelPage *page=g_ptr_array_index(p->pages,i); GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,16),*scroll=gtk_scrolled_window_new(),*grid=gtk_grid_new(),*page_root=gtk_box_new(GTK_ORIENTATION_VERTICAL,12);
  gtk_widget_set_margin_end(box,8); gtk_grid_set_row_spacing(GTK_GRID(grid),12); gtk_grid_set_column_spacing(GTK_GRID(grid),16);
  gtk_widget_set_hexpand(grid,TRUE); gtk_widget_set_valign(grid,GTK_ALIGN_START); gtk_box_append(GTK_BOX(box),grid);
  for (j=0;j<page->fields->len;j++) {
   PanelField *f=g_ptr_array_index(page->fields,j); GtkWidget *label,*cell;
   if (!strcmp(f->type,"check")) {
    f->widget=gtk_check_button_new_with_label(f->label); gtk_check_button_set_active(GTK_CHECK_BUTTON(f->widget),!strcmp(f->value,"true"));
    label=gtk_check_button_get_child(GTK_CHECK_BUTTON(f->widget));
    if (GTK_IS_LABEL(label)) { gtk_label_set_wrap(GTK_LABEL(label),TRUE); gtk_label_set_wrap_mode(GTK_LABEL(label),PANGO_WRAP_WORD_CHAR); gtk_label_set_max_width_chars(GTK_LABEL(label),72); }
    gtk_widget_set_halign(f->widget,GTK_ALIGN_START); gtk_grid_attach(GTK_GRID(grid),f->widget,0,(int)j,2,1); continue;
   }
   label=gtk_label_new(f->label); gtk_label_set_xalign(GTK_LABEL(label),0); gtk_label_set_wrap(GTK_LABEL(label),TRUE);
   gtk_label_set_wrap_mode(GTK_LABEL(label),PANGO_WRAP_WORD_CHAR); gtk_label_set_max_width_chars(GTK_LABEL(label),24);
   gtk_widget_set_halign(label,GTK_ALIGN_START); gtk_widget_set_valign(label,GTK_ALIGN_START); gtk_size_group_add_widget(labels,label);
   if (!strcmp(f->type,"int")) { f->widget=gtk_spin_button_new_with_range(f->minimum,f->maximum,1); gtk_spin_button_set_value(GTK_SPIN_BUTTON(f->widget),(gdouble)g_ascii_strtoll(f->value,NULL,10)); gtk_widget_set_halign(f->widget,GTK_ALIGN_START); }
   else if (!strcmp(f->type,"text")) { f->widget=gtk_entry_new(); gtk_entry_set_max_length(GTK_ENTRY(f->widget),512); gtk_editable_set_text(GTK_EDITABLE(f->widget),f->value); gtk_widget_set_hexpand(f->widget,TRUE); }
   else if (!strcmp(f->type,"choice")) { char **choices=g_strsplit(f->choices,"|",-1); guint index=0; while (choices[index] && strcmp(choices[index],f->value)) index++; f->widget=gtk_drop_down_new_from_strings((const char *const*)choices); gtk_drop_down_set_selected(GTK_DROP_DOWN(f->widget),index); g_strfreev(choices); gtk_widget_set_halign(f->widget,GTK_ALIGN_START); }
   else {
    f->widget=gtk_label_new(f->value); gtk_label_set_wrap(GTK_LABEL(f->widget),TRUE); gtk_label_set_selectable(GTK_LABEL(f->widget),TRUE); gtk_label_set_xalign(GTK_LABEL(f->widget),0); gtk_label_set_wrap_mode(GTK_LABEL(f->widget),PANGO_WRAP_WORD_CHAR); gtk_label_set_max_width_chars(GTK_LABEL(f->widget),48); gtk_widget_set_hexpand(f->widget,TRUE);
   }
   cell=f->widget; gtk_widget_set_valign(f->widget,GTK_ALIGN_START); gtk_label_set_mnemonic_widget(GTK_LABEL(label),f->widget);
   if (!strcmp(f->type,"status")) {
    cell=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8); f->lamp=gtk_label_new(NULL); lamp_set(f->lamp,f->state); gtk_widget_set_valign(f->lamp,GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(cell),f->lamp); gtk_box_append(GTK_BOX(cell),f->widget); gtk_widget_set_hexpand(cell,TRUE); gtk_widget_set_valign(cell,GTK_ALIGN_START);
   }
   gtk_grid_attach(GTK_GRID(grid),label,0,(int)j,1,1); gtk_grid_attach(GTK_GRID(grid),cell,1,(int)j,1,1);
  }
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC); gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),box);
  gtk_widget_set_vexpand(scroll,TRUE); gtk_box_append(GTK_BOX(page_root),scroll);
  /* Keep actions outside scrolling fields: long forms must not hide buttons. */
  if (page->buttons->len) gtk_box_append(GTK_BOX(page_root),build_buttons(p,page->buttons));
  gtk_stack_add_titled(GTK_STACK(stack),page_root,page->id,page->title);
 }
 g_object_unref(labels);
 footer=build_buttons(p,p->buttons); gtk_widget_set_halign(footer,GTK_ALIGN_END);
 close=gtk_button_new_with_label("Close"); g_signal_connect(close,"clicked",G_CALLBACK(close_clicked),p); gtk_flow_box_insert(GTK_FLOW_BOX(footer),close,-1); gtk_box_append(GTK_BOX(root),footer);
 gtk_window_set_child(GTK_WINDOW(p->window),root); gtk_window_present(GTK_WINDOW(p->window));
}
/* UPDATE uses the same schema validator: a panel containing readonly fields.
 * Validate every target before applying any changes; never overwrite drafts. */
static gboolean update_panel (AddonPanel *p, AddonPanel *update)
{
 guint i,j; GPtrArray *targets=g_ptr_array_new();
 if (update->buttons->len) { g_ptr_array_unref(targets); return FALSE; }
 for (i=0;i<update->pages->len;i++) { PanelPage *page=g_ptr_array_index(update->pages,i);
  if (page->buttons->len) goto invalid_update;
  for (j=0;j<page->fields->len;j++) {
   PanelField *f=g_ptr_array_index(page->fields,j), *target=NULL; guint a,b;
   if (strcmp(f->type,"label") && strcmp(f->type,"status")) goto invalid_update;
   for (a=0;a<p->pages->len;a++) { PanelPage *old=g_ptr_array_index(p->pages,a); for (b=0;b<old->fields->len;b++) { PanelField *candidate=g_ptr_array_index(old->fields,b); if (!strcmp(candidate->id,f->id)) target=candidate; } }
   if (!target || strcmp(target->type,f->type)) goto invalid_update;
   g_ptr_array_add(targets,target); g_ptr_array_add(targets,f);
  }
 }
 for (i=0;i<targets->len;i+=2) { PanelField *target=g_ptr_array_index(targets,i), *f=g_ptr_array_index(targets,i+1); gtk_label_set_text(GTK_LABEL(target->widget),f->value); if (target->lamp) lamp_set(target->lamp,f->state); }
 g_ptr_array_unref(targets); return TRUE;
 invalid_update: g_ptr_array_unref(targets); return FALSE;
}
static AddonPanel *parse_panel (const char *payload)
{
 AddonPanel *p; PanelParser state={0}; GMarkupParseContext *ctx;
 GError *error=NULL; gsize size; char *xml=decode(payload,&size); gboolean ok;
 if (!xml) return NULL;
 p=g_new0(AddonPanel,1); p->pages=g_ptr_array_new_with_free_func(page_free); p->buttons=g_ptr_array_new_with_free_func(button_free); p->ids=g_hash_table_new_full(g_str_hash,g_str_equal,g_free,NULL); state.panel=p;
 ctx=g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,&state,NULL);
 ok=g_markup_parse_context_parse(ctx,xml,size,&error) && g_markup_parse_context_end_parse(ctx,&error) && state.ended && p->pages->len>0;
 g_markup_parse_context_free(ctx); g_free(xml); g_clear_error(&error);
 if (!ok) { panel_free(p); return NULL; }
 return p;
}
const char *fe_addon_oper_status (const char *token)
{
 AddonPanel *p=panels?g_hash_table_lookup(panels,token):NULL;
 return p?(p->oper_login?oper_login_status(p->oper_login):(p->oper_login_message?p->oper_login_message:"No OPER login attempted.")):NULL;
}
int fe_addon_panel_exists (const char *token) { return panels && g_hash_table_contains(panels,token); }
int fe_addon_panel_command (session *sess, const char *action, const char *token, const char *payload)
{
 AddonPanel *old, *p; gboolean ok;
 if (!identifier(token) || !action) return 0;
 old=panels?g_hash_table_lookup(panels,token):NULL;
 if (!strcmp(action,"CLOSE")) { if (old) { old->closing=TRUE; unregister_panel(old); g_clear_pointer(&old->oper_login,oper_login_free); gtk_window_destroy(GTK_WINDOW(old->window)); } return 1; }
 if (!strcmp(action,"PRESENT")) { if (old) gtk_window_present(GTK_WINDOW(old->window)); return old!=NULL; }
 if (!strcmp(action,"OPERLOGIN")) {
#if defined(WIN32) && defined(USE_OPENSSL)
  if (!old || !context_valid(old)) return 0;
  if (!old->oper_login) old->oper_login=oper_login_new(old->session);
  if (!old->oper_login) old->oper_login_message="A named network profile is required; no login sent.";
  else oper_login_open(old->oper_login,GTK_WINDOW(old->window));
  return 1;
#else
  return 0;
#endif
 }
 if (strcmp(action,"OPEN") && strcmp(action,"UPDATE")) return 0;
 if ((!strcmp(action,"OPEN") && (old || (panels && g_hash_table_size(panels)>=PANEL_LIMIT))) || (!strcmp(action,"UPDATE") && !old)) return 0;
 p=parse_panel(payload); if (!p) return 0;
 if (!strcmp(action,"UPDATE")) { ok=update_panel(old,p); panel_free(p); return ok; }
 if (!is_session(sess)) { panel_free(p); return 0; }
 p->token=g_strdup(token); p->session=sess; p->server_id=sess->server->id; p->server=g_strdup(sess->server->servername); p->channel=g_strdup(sess->channel);
 if (!panels) panels=g_hash_table_new(g_str_hash,g_str_equal);
 g_hash_table_insert(panels,p->token,p); build_window(p); return 1;
}

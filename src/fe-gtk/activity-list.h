#ifndef FABULOR_ACTIVITY_LIST_H
#define FABULOR_ACTIVITY_LIST_H

#include <gtk/gtk.h>
#include "../common/activity.h"

typedef struct _FabulorActivityList FabulorActivityList;
typedef void (*FabulorActivityActivate) (gpointer identity, gpointer data);

FabulorActivityList *fabulor_activity_list_new (FabulorActivityActivate activate,
	FabulorActivityActivate mark_read, gpointer data);
void fabulor_activity_list_free (FabulorActivityList *list);
GtkWidget *fabulor_activity_list_get_widget (FabulorActivityList *list);
void fabulor_activity_list_focus (FabulorActivityList *list);
void fabulor_activity_list_update (FabulorActivityList *list, gpointer identity,
								  const char *context, const char *network,
								  const ActivityState *activity, const char *highlight_preview);
void fabulor_activity_list_remove (FabulorActivityList *list, gpointer identity);

#endif

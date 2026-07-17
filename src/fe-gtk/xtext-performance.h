#ifndef FABULOR_XTEXT_PERFORMANCE_H
#define FABULOR_XTEXT_PERFORMANCE_H

#include <glib.h>

typedef enum
{
	FABULOR_XTEXT_APPEND_REFRESH_NONE,
	FABULOR_XTEXT_APPEND_REFRESH_IMMEDIATE,
	FABULOR_XTEXT_APPEND_REFRESH_IDLE
} FabulorXTextAppendRefresh;

FabulorXTextAppendRefresh fabulor_xtext_append_refresh_plan (
	gboolean current_buffer,
	gboolean refresh_pending,
	gboolean scrollbar_down);
gboolean fabulor_xtext_should_trim_oldest (gint max_lines,
	gint current_lines,
	gboolean has_older_entry);

#endif

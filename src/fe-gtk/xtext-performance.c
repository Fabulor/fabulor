#include "xtext-performance.h"

FabulorXTextAppendRefresh
fabulor_xtext_append_refresh_plan (gboolean current_buffer,
	gboolean refresh_pending,
	gboolean scrollbar_down)
{
	if (!current_buffer)
		return FABULOR_XTEXT_APPEND_REFRESH_NONE;
	if (scrollbar_down)
		return FABULOR_XTEXT_APPEND_REFRESH_IMMEDIATE;
	if (refresh_pending)
		return FABULOR_XTEXT_APPEND_REFRESH_NONE;

	return FABULOR_XTEXT_APPEND_REFRESH_IDLE;
}

gboolean
fabulor_xtext_should_trim_oldest (gint max_lines, gint current_lines,
	gboolean has_older_entry)
{
	return max_lines > 2 && current_lines > max_lines && has_older_entry;
}

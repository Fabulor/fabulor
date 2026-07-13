#include "fabulor-plugin-manifest-json.h"

#include <stdarg.h>
#include <string.h>

#define FABULOR_MANIFEST_KEY_MAX 32U
#define FABULOR_MANIFEST_ID_MAX 128U
#define FABULOR_MANIFEST_NAME_MAX 256U
#define FABULOR_MANIFEST_VERSION_MAX 64U
#define FABULOR_MANIFEST_LANGUAGE_MAX 16U
#define FABULOR_MANIFEST_ENTRYPOINT_MAX 1024U
#define FABULOR_MANIFEST_DESCRIPTION_MAX 2048U
#define FABULOR_MANIFEST_AUTHOR_MAX 256U
#define FABULOR_MANIFEST_HOMEPAGE_MAX 2048U
#define FABULOR_MANIFEST_DEPENDENCIES_MAX 64U
#define FABULOR_MANIFEST_CAPABILITIES_MAX 32U
#define FABULOR_MANIFEST_CAPABILITY_MAX 64U

typedef struct
{
	const char *start;
	const char *cursor;
	const char *end;
	GError **error;
} FabulorManifestJsonParser;

typedef enum
{
	FABULOR_MANIFEST_FIELD_ID = 1U << 0,
	FABULOR_MANIFEST_FIELD_NAME = 1U << 1,
	FABULOR_MANIFEST_FIELD_VERSION = 1U << 2,
	FABULOR_MANIFEST_FIELD_LANGUAGE = 1U << 3,
	FABULOR_MANIFEST_FIELD_ENTRYPOINT = 1U << 4,
	FABULOR_MANIFEST_FIELD_API_VERSION = 1U << 5,
	FABULOR_MANIFEST_FIELD_DEPENDENCIES = 1U << 6,
	FABULOR_MANIFEST_FIELD_CAPABILITIES = 1U << 7,
	FABULOR_MANIFEST_FIELD_DESCRIPTION = 1U << 8,
	FABULOR_MANIFEST_FIELD_AUTHOR = 1U << 9,
	FABULOR_MANIFEST_FIELD_HOMEPAGE = 1U << 10
} FabulorManifestField;

#define FABULOR_MANIFEST_REQUIRED_FIELDS ((1U << 11) - 1U)

static gboolean
fabulor_manifest_json_fail (FabulorManifestJsonParser *parser, const char *format, ...)
{
	va_list args;
	char *detail;

	if (!parser->error || *parser->error)
	{
		return FALSE;
	}

	va_start (args, format);
	detail = g_strdup_vprintf (format, args);
	va_end (args);
	g_set_error (parser->error,
				 G_FILE_ERROR,
				 G_FILE_ERROR_INVAL,
				 "Invalid plugin manifest JSON at byte %" G_GSIZE_FORMAT ": %s",
				 (gsize) (parser->cursor - parser->start),
				 detail);
	g_free (detail);
	return FALSE;
}

static void
fabulor_manifest_json_skip_whitespace (FabulorManifestJsonParser *parser)
{
	while (parser->cursor < parser->end)
	{
		char current = *parser->cursor;
		if (current != ' ' && current != '\t' && current != '\r' && current != '\n')
		{
			break;
		}
		parser->cursor++;
	}
}

static gboolean
fabulor_manifest_json_expect (FabulorManifestJsonParser *parser, char expected)
{
	if (parser->cursor >= parser->end || *parser->cursor != expected)
	{
		return fabulor_manifest_json_fail (parser, "expected '%c'.", expected);
	}
	parser->cursor++;
	return TRUE;
}

static gboolean
fabulor_manifest_json_parse_hex_quad (FabulorManifestJsonParser *parser, gunichar *value)
{
	guint i;
	gunichar result = 0;

	if ((gsize) (parser->end - parser->cursor) < 4)
	{
		return fabulor_manifest_json_fail (parser, "incomplete Unicode escape.");
	}

	for (i = 0; i < 4; i++)
	{
		char current = *parser->cursor++;
		result <<= 4;
		if (current >= '0' && current <= '9')
			result |= (gunichar) (current - '0');
		else if (current >= 'a' && current <= 'f')
			result |= (gunichar) (current - 'a' + 10);
		else if (current >= 'A' && current <= 'F')
			result |= (gunichar) (current - 'A' + 10);
		else
			return fabulor_manifest_json_fail (parser, "invalid Unicode escape.");
	}

	*value = result;
	return TRUE;
}

static gboolean
fabulor_manifest_json_append_unicode_escape (FabulorManifestJsonParser *parser, GString *value)
{
	gunichar codepoint;

	if (!fabulor_manifest_json_parse_hex_quad (parser, &codepoint))
	{
		return FALSE;
	}

	if (codepoint >= 0xd800 && codepoint <= 0xdbff)
	{
		gunichar low_surrogate;
		if ((gsize) (parser->end - parser->cursor) < 6
			|| parser->cursor[0] != '\\'
			|| parser->cursor[1] != 'u')
		{
			return fabulor_manifest_json_fail (parser, "high surrogate is not followed by a low surrogate.");
		}
		parser->cursor += 2;
		if (!fabulor_manifest_json_parse_hex_quad (parser, &low_surrogate))
		{
			return FALSE;
		}
		if (low_surrogate < 0xdc00 || low_surrogate > 0xdfff)
		{
			return fabulor_manifest_json_fail (parser, "invalid low surrogate.");
		}
		codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (low_surrogate - 0xdc00);
	}
	else if (codepoint >= 0xdc00 && codepoint <= 0xdfff)
	{
		return fabulor_manifest_json_fail (parser, "unexpected low surrogate.");
	}

	if (codepoint == 0 || !g_unichar_validate (codepoint))
	{
		return fabulor_manifest_json_fail (parser, "invalid Unicode code point.");
	}
	g_string_append_unichar (value, codepoint);
	return TRUE;
}

static gboolean
fabulor_manifest_json_parse_string (FabulorManifestJsonParser *parser,
											 char **result,
											 gsize maximum_bytes)
{
	GString *value;

	*result = NULL;
	if (!fabulor_manifest_json_expect (parser, '"'))
	{
		return FALSE;
	}

	value = g_string_new (NULL);
	while (parser->cursor < parser->end)
	{
		guchar current = (guchar) *parser->cursor++;

		if (current == '"')
		{
			const char *character;
			if (!g_utf8_validate (value->str, value->len, NULL))
			{
				g_string_free (value, TRUE);
				return fabulor_manifest_json_fail (parser, "string is not valid UTF-8.");
			}
			for (character = value->str; *character; character = g_utf8_next_char (character))
			{
				if (g_unichar_iscntrl (g_utf8_get_char (character)))
				{
					g_string_free (value, TRUE);
					return fabulor_manifest_json_fail (parser, "manifest strings cannot contain control characters.");
				}
			}
			*result = g_string_free (value, FALSE);
			return TRUE;
		}
		if (current < 0x20)
		{
			g_string_free (value, TRUE);
			return fabulor_manifest_json_fail (parser, "unescaped control character in string.");
		}
		if (current != '\\')
		{
			g_string_append_c (value, (char) current);
		}
		else
		{
			char escaped;
			if (parser->cursor >= parser->end)
			{
				g_string_free (value, TRUE);
				return fabulor_manifest_json_fail (parser, "incomplete string escape.");
			}
			escaped = *parser->cursor++;
			switch (escaped)
			{
			case '"': g_string_append_c (value, '"'); break;
			case '\\': g_string_append_c (value, '\\'); break;
			case '/': g_string_append_c (value, '/'); break;
			case 'b': g_string_append_c (value, '\b'); break;
			case 'f': g_string_append_c (value, '\f'); break;
			case 'n': g_string_append_c (value, '\n'); break;
			case 'r': g_string_append_c (value, '\r'); break;
			case 't': g_string_append_c (value, '\t'); break;
			case 'u':
				if (!fabulor_manifest_json_append_unicode_escape (parser, value))
				{
					g_string_free (value, TRUE);
					return FALSE;
				}
				break;
			default:
				g_string_free (value, TRUE);
				return fabulor_manifest_json_fail (parser, "invalid string escape '\\%c'.", escaped);
			}
		}

		if (value->len > maximum_bytes)
		{
			g_string_free (value, TRUE);
			return fabulor_manifest_json_fail (parser, "string exceeds the %" G_GSIZE_FORMAT "-byte field limit.", maximum_bytes);
		}
	}

	g_string_free (value, TRUE);
	return fabulor_manifest_json_fail (parser, "unterminated string.");
}

static gboolean
fabulor_manifest_json_parse_uint (FabulorManifestJsonParser *parser, guint *result)
{
	guint64 value = 0;

	if (parser->cursor >= parser->end || *parser->cursor < '0' || *parser->cursor > '9')
	{
		return fabulor_manifest_json_fail (parser, "expected an unsigned integer.");
	}
	if (*parser->cursor == '0' && parser->cursor + 1 < parser->end
		&& parser->cursor[1] >= '0' && parser->cursor[1] <= '9')
	{
		return fabulor_manifest_json_fail (parser, "unsigned integers cannot contain leading zeroes.");
	}

	while (parser->cursor < parser->end && *parser->cursor >= '0' && *parser->cursor <= '9')
	{
		guint digit = (guint) (*parser->cursor - '0');
		if (value > ((guint64) G_MAXUINT - digit) / 10)
		{
			return fabulor_manifest_json_fail (parser, "unsigned integer exceeds the host range.");
		}
		value = value * 10 + digit;
		parser->cursor++;
	}
	if (value == 0)
	{
		return fabulor_manifest_json_fail (parser, "requires_api_version must be at least 1.");
	}
	*result = (guint) value;
	return TRUE;
}

static gboolean
fabulor_manifest_json_array_contains (GPtrArray *values, const char *candidate)
{
	guint i;
	for (i = 0; i < values->len; i++)
	{
		if (g_strcmp0 (g_ptr_array_index (values, i), candidate) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
fabulor_manifest_json_parse_string_array (FabulorManifestJsonParser *parser,
												   GPtrArray *values,
												   guint maximum_items,
												   gsize maximum_item_bytes)
{
	if (!fabulor_manifest_json_expect (parser, '['))
	{
		return FALSE;
	}
	fabulor_manifest_json_skip_whitespace (parser);
	if (parser->cursor < parser->end && *parser->cursor == ']')
	{
		parser->cursor++;
		return TRUE;
	}

	while (parser->cursor < parser->end)
	{
		char *item = NULL;
		if (values->len >= maximum_items)
		{
			return fabulor_manifest_json_fail (parser, "array exceeds the %u-item limit.", maximum_items);
		}
		if (!fabulor_manifest_json_parse_string (parser, &item, maximum_item_bytes))
		{
			return FALSE;
		}
		if (*item == '\0')
		{
			g_free (item);
			return fabulor_manifest_json_fail (parser, "array items cannot be empty.");
		}
		if (fabulor_manifest_json_array_contains (values, item))
		{
			g_free (item);
			return fabulor_manifest_json_fail (parser, "array contains a duplicate string.");
		}
		g_ptr_array_add (values, item);
		fabulor_manifest_json_skip_whitespace (parser);
		if (parser->cursor < parser->end && *parser->cursor == ']')
		{
			parser->cursor++;
			return TRUE;
		}
		if (!fabulor_manifest_json_expect (parser, ','))
		{
			return FALSE;
		}
		fabulor_manifest_json_skip_whitespace (parser);
		if (parser->cursor < parser->end && *parser->cursor == ']')
		{
			return fabulor_manifest_json_fail (parser, "trailing commas are not allowed.");
		}
	}

	return fabulor_manifest_json_fail (parser, "unterminated array.");
}

static FabulorManifestField
fabulor_manifest_json_field_for_key (const char *key)
{
	if (g_strcmp0 (key, "id") == 0) return FABULOR_MANIFEST_FIELD_ID;
	if (g_strcmp0 (key, "name") == 0) return FABULOR_MANIFEST_FIELD_NAME;
	if (g_strcmp0 (key, "version") == 0) return FABULOR_MANIFEST_FIELD_VERSION;
	if (g_strcmp0 (key, "language") == 0) return FABULOR_MANIFEST_FIELD_LANGUAGE;
	if (g_strcmp0 (key, "entrypoint") == 0) return FABULOR_MANIFEST_FIELD_ENTRYPOINT;
	if (g_strcmp0 (key, "requires_api_version") == 0) return FABULOR_MANIFEST_FIELD_API_VERSION;
	if (g_strcmp0 (key, "dependencies") == 0) return FABULOR_MANIFEST_FIELD_DEPENDENCIES;
	if (g_strcmp0 (key, "capabilities") == 0) return FABULOR_MANIFEST_FIELD_CAPABILITIES;
	if (g_strcmp0 (key, "description") == 0) return FABULOR_MANIFEST_FIELD_DESCRIPTION;
	if (g_strcmp0 (key, "author") == 0) return FABULOR_MANIFEST_FIELD_AUTHOR;
	if (g_strcmp0 (key, "homepage") == 0) return FABULOR_MANIFEST_FIELD_HOMEPAGE;
	return 0;
}

static const char *
fabulor_manifest_json_key_for_field (FabulorManifestField field)
{
	switch (field)
	{
	case FABULOR_MANIFEST_FIELD_ID: return "id";
	case FABULOR_MANIFEST_FIELD_NAME: return "name";
	case FABULOR_MANIFEST_FIELD_VERSION: return "version";
	case FABULOR_MANIFEST_FIELD_LANGUAGE: return "language";
	case FABULOR_MANIFEST_FIELD_ENTRYPOINT: return "entrypoint";
	case FABULOR_MANIFEST_FIELD_API_VERSION: return "requires_api_version";
	case FABULOR_MANIFEST_FIELD_DEPENDENCIES: return "dependencies";
	case FABULOR_MANIFEST_FIELD_CAPABILITIES: return "capabilities";
	case FABULOR_MANIFEST_FIELD_DESCRIPTION: return "description";
	case FABULOR_MANIFEST_FIELD_AUTHOR: return "author";
	case FABULOR_MANIFEST_FIELD_HOMEPAGE: return "homepage";
	default: return "unknown";
	}
}

static gboolean
fabulor_manifest_json_parse_field (FabulorManifestJsonParser *parser,
											FabulorManifestField field,
											FabulorPluginManifest *manifest)
{
	switch (field)
	{
	case FABULOR_MANIFEST_FIELD_ID:
		return fabulor_manifest_json_parse_string (parser, &manifest->id, FABULOR_MANIFEST_ID_MAX);
	case FABULOR_MANIFEST_FIELD_NAME:
		return fabulor_manifest_json_parse_string (parser, &manifest->name, FABULOR_MANIFEST_NAME_MAX);
	case FABULOR_MANIFEST_FIELD_VERSION:
		return fabulor_manifest_json_parse_string (parser, &manifest->version, FABULOR_MANIFEST_VERSION_MAX);
	case FABULOR_MANIFEST_FIELD_LANGUAGE:
		return fabulor_manifest_json_parse_string (parser, &manifest->language_name, FABULOR_MANIFEST_LANGUAGE_MAX);
	case FABULOR_MANIFEST_FIELD_ENTRYPOINT:
		return fabulor_manifest_json_parse_string (parser, &manifest->entrypoint, FABULOR_MANIFEST_ENTRYPOINT_MAX);
	case FABULOR_MANIFEST_FIELD_API_VERSION:
		return fabulor_manifest_json_parse_uint (parser, &manifest->requires_api_version);
	case FABULOR_MANIFEST_FIELD_DEPENDENCIES:
		return fabulor_manifest_json_parse_string_array (parser,
													  manifest->dependencies,
													  FABULOR_MANIFEST_DEPENDENCIES_MAX,
													  FABULOR_MANIFEST_ID_MAX);
	case FABULOR_MANIFEST_FIELD_CAPABILITIES:
		return fabulor_manifest_json_parse_string_array (parser,
													  manifest->capabilities,
													  FABULOR_MANIFEST_CAPABILITIES_MAX,
													  FABULOR_MANIFEST_CAPABILITY_MAX);
	case FABULOR_MANIFEST_FIELD_DESCRIPTION:
		return fabulor_manifest_json_parse_string (parser, &manifest->description, FABULOR_MANIFEST_DESCRIPTION_MAX);
	case FABULOR_MANIFEST_FIELD_AUTHOR:
		return fabulor_manifest_json_parse_string (parser, &manifest->author, FABULOR_MANIFEST_AUTHOR_MAX);
	case FABULOR_MANIFEST_FIELD_HOMEPAGE:
		return fabulor_manifest_json_parse_string (parser, &manifest->homepage, FABULOR_MANIFEST_HOMEPAGE_MAX);
	default:
		return fabulor_manifest_json_fail (parser, "unknown schema field.");
	}
}

gboolean
fabulor_plugin_manifest_parse_json (const char *json,
											 gsize json_length,
											 FabulorPluginManifest *manifest,
											 GError **error)
{
	FabulorManifestJsonParser parser;
	guint seen_fields = 0;
	gboolean object_closed = FALSE;

	if (!json || !manifest)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Manifest parsing requires JSON input and a manifest destination.");
		return FALSE;
	}
	if (!manifest->dependencies || !manifest->capabilities)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Manifest destination arrays are not initialized.");
		return FALSE;
	}
	if (json_length == 0 || json_length > FABULOR_PLUGIN_MANIFEST_MAX_BYTES)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_INVAL,
					 "Plugin manifest must be between 1 and %u bytes.",
					 FABULOR_PLUGIN_MANIFEST_MAX_BYTES);
		return FALSE;
	}

	parser.start = json;
	parser.cursor = json;
	parser.end = json + json_length;
	parser.error = error;
	fabulor_manifest_json_skip_whitespace (&parser);
	if (!fabulor_manifest_json_expect (&parser, '{'))
	{
		return FALSE;
	}
	fabulor_manifest_json_skip_whitespace (&parser);
	if (parser.cursor < parser.end && *parser.cursor == '}')
	{
		return fabulor_manifest_json_fail (&parser, "manifest object cannot be empty.");
	}

	while (parser.cursor < parser.end)
	{
		char *key = NULL;
		FabulorManifestField field;

		if (!fabulor_manifest_json_parse_string (&parser, &key, FABULOR_MANIFEST_KEY_MAX))
		{
			return FALSE;
		}
		field = fabulor_manifest_json_field_for_key (key);
		if (field == 0)
		{
			fabulor_manifest_json_fail (&parser, "unknown field '%s'.", key);
			g_free (key);
			return FALSE;
		}
		if ((seen_fields & (guint) field) != 0)
		{
			fabulor_manifest_json_fail (&parser, "duplicate field '%s'.", key);
			g_free (key);
			return FALSE;
		}
		g_free (key);

		fabulor_manifest_json_skip_whitespace (&parser);
		if (!fabulor_manifest_json_expect (&parser, ':'))
		{
			return FALSE;
		}
		fabulor_manifest_json_skip_whitespace (&parser);
		if (!fabulor_manifest_json_parse_field (&parser, field, manifest))
		{
			return FALSE;
		}
		seen_fields |= (guint) field;
		fabulor_manifest_json_skip_whitespace (&parser);

		if (parser.cursor < parser.end && *parser.cursor == '}')
		{
			parser.cursor++;
			object_closed = TRUE;
			break;
		}
		if (parser.cursor >= parser.end)
		{
			return fabulor_manifest_json_fail (&parser, "unterminated manifest object.");
		}
		if (!fabulor_manifest_json_expect (&parser, ','))
		{
			return FALSE;
		}
		fabulor_manifest_json_skip_whitespace (&parser);
		if (parser.cursor < parser.end && *parser.cursor == '}')
		{
			return fabulor_manifest_json_fail (&parser, "trailing commas are not allowed.");
		}
	}
	if (!object_closed)
	{
		return fabulor_manifest_json_fail (&parser, "unterminated manifest object.");
	}

	fabulor_manifest_json_skip_whitespace (&parser);
	if (parser.cursor != parser.end)
	{
		return fabulor_manifest_json_fail (&parser, "trailing content after the manifest object.");
	}
	if (seen_fields != FABULOR_MANIFEST_REQUIRED_FIELDS)
	{
		FabulorManifestField field;
		for (field = FABULOR_MANIFEST_FIELD_ID;
			 field <= FABULOR_MANIFEST_FIELD_HOMEPAGE;
			 field = (FabulorManifestField) ((guint) field << 1))
		{
			if ((seen_fields & (guint) field) == 0)
			{
				return fabulor_manifest_json_fail (&parser, "missing required field '%s'.", fabulor_manifest_json_key_for_field (field));
			}
		}
	}

	return TRUE;
}

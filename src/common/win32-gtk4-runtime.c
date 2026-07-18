#include "win32-gtk4-runtime.h"

#ifdef WIN32

#include <stddef.h>
#include <string.h>
#include <wchar.h>

static DLL_DIRECTORY_COOKIE gtk4_runtime_cookie;
static wchar_t *gtk4_runtime_bin;

static BOOL
fabulor_win32_runtime_fail (DWORD error, DWORD *error_code)
{
	if (error_code != NULL)
		*error_code = error;
	SetLastError (error);
	return FALSE;
}

static wchar_t *
fabulor_win32_executable_path (DWORD *error_code)
{
	DWORD capacity = MAX_PATH;
	wchar_t *path;

	for (;;)
	{
		DWORD length;

		path = HeapAlloc (GetProcessHeap (), 0,
			(size_t) capacity * sizeof (*path));
		if (path == NULL)
		{
			fabulor_win32_runtime_fail (ERROR_NOT_ENOUGH_MEMORY, error_code);
			return NULL;
		}

		length = GetModuleFileNameW (NULL, path, capacity);
		if (length == 0)
		{
			DWORD error = GetLastError ();
			HeapFree (GetProcessHeap (), 0, path);
			fabulor_win32_runtime_fail (error, error_code);
			return NULL;
		}
		if (length < capacity)
			return path;

		HeapFree (GetProcessHeap (), 0, path);
		if (capacity >= 32768)
		{
			fabulor_win32_runtime_fail (ERROR_INSUFFICIENT_BUFFER, error_code);
			return NULL;
		}
		capacity = capacity > 32768 / 2 ? 32768 : capacity * 2;
	}
}

static BOOL
fabulor_win32_require_directory (const wchar_t *path, DWORD *error_code)
{
	DWORD attributes = GetFileAttributesW (path);

	if (attributes == INVALID_FILE_ATTRIBUTES)
		return fabulor_win32_runtime_fail (GetLastError (), error_code);
	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return fabulor_win32_runtime_fail (ERROR_DIRECTORY, error_code);
	if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
		return fabulor_win32_runtime_fail (ERROR_REPARSE_TAG_INVALID, error_code);
	return TRUE;
}

static BOOL
fabulor_win32_append_directory (wchar_t *path, size_t capacity,
	const wchar_t *name, DWORD *error_code)
{
	size_t path_length = wcslen (path);
	size_t name_length = wcslen (name);

	if (path_length + 1 + name_length + 1 > capacity)
		return fabulor_win32_runtime_fail (ERROR_INSUFFICIENT_BUFFER, error_code);
	path[path_length] = L'\\';
	memcpy (path + path_length + 1, name,
		(name_length + 1) * sizeof (*path));
	return fabulor_win32_require_directory (path, error_code);
}

BOOL
fabulor_win32_configure_gtk4_runtime (DWORD *error_code)
{
	static const wchar_t *directories[] = {L"Runtime", L"GTK4", L"bin"};
	wchar_t *executable_path;
	wchar_t *separator;
	wchar_t *runtime_bin;
	size_t capacity;
	size_t index;

	if (gtk4_runtime_cookie != NULL)
	{
		if (error_code != NULL)
			*error_code = ERROR_SUCCESS;
		return TRUE;
	}

	executable_path = fabulor_win32_executable_path (error_code);
	if (executable_path == NULL)
		return FALSE;

	separator = wcsrchr (executable_path, L'\\');
	if (separator == NULL)
	{
		HeapFree (GetProcessHeap (), 0, executable_path);
		return fabulor_win32_runtime_fail (ERROR_BAD_PATHNAME, error_code);
	}
	*separator = L'\0';

	capacity = wcslen (executable_path) + 1;
	for (index = 0; index < sizeof (directories) / sizeof (directories[0]); index++)
		capacity += 1 + wcslen (directories[index]);

	runtime_bin = HeapReAlloc (GetProcessHeap (), 0, executable_path,
		capacity * sizeof (*runtime_bin));
	if (runtime_bin == NULL)
	{
		HeapFree (GetProcessHeap (), 0, executable_path);
		return fabulor_win32_runtime_fail (ERROR_NOT_ENOUGH_MEMORY, error_code);
	}
	gtk4_runtime_bin = runtime_bin;

	for (index = 0; index < sizeof (directories) / sizeof (directories[0]); index++)
	{
		if (!fabulor_win32_append_directory (gtk4_runtime_bin, capacity,
			directories[index], error_code))
			goto fail;
	}

	if (!SetDefaultDllDirectories (LOAD_LIBRARY_SEARCH_APPLICATION_DIR |
		LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS))
	{
		fabulor_win32_runtime_fail (GetLastError (), error_code);
		goto fail;
	}

	gtk4_runtime_cookie = AddDllDirectory (gtk4_runtime_bin);
	if (gtk4_runtime_cookie == NULL)
	{
		fabulor_win32_runtime_fail (GetLastError (), error_code);
		goto fail;
	}

	if (error_code != NULL)
		*error_code = ERROR_SUCCESS;
	return TRUE;

fail:
	HeapFree (GetProcessHeap (), 0, gtk4_runtime_bin);
	gtk4_runtime_bin = NULL;
	return FALSE;
}

const wchar_t *
fabulor_win32_gtk4_runtime_bin (void)
{
	return gtk4_runtime_bin;
}

#endif

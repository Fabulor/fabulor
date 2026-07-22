#include <stdlib.h>
#include <wchar.h>
#include <windows.h>

#include "../../src/common/win32-gtk4-runtime.h"

typedef int (__cdecl *FabulorFrontendMain) (int argc, char **argv);

static int
fail_startup (const wchar_t *operation, DWORD error)
{
	wchar_t message[320];

	_snwprintf_s (message, sizeof (message) / sizeof (message[0]), _TRUNCATE,
		L"Fabulor could not %ls. Win32 error: %lu", operation,
		(unsigned long) error);
	MessageBoxW (NULL, message, L"Fabulor startup error",
		MB_OK | MB_ICONERROR | MB_TASKMODAL);
	return EXIT_FAILURE;
}

static BOOL
frontend_module_path (wchar_t *path, DWORD capacity, DWORD *error)
{
	static const wchar_t module_name[] = L"fabulor-gtk4-frontend.dll";
	DWORD length;
	wchar_t *separator;
	size_t remaining;

	length = GetModuleFileNameW (NULL, path, capacity);
	if (length == 0 || length == capacity)
	{
		*error = length == 0 ? GetLastError () : ERROR_INSUFFICIENT_BUFFER;
		return FALSE;
	}

	separator = wcsrchr (path, L'\\');
	if (separator == NULL)
	{
		*error = ERROR_BAD_PATHNAME;
		return FALSE;
	}

	separator++;
	remaining = capacity - (size_t) (separator - path);
	if (wcscpy_s (separator, remaining, module_name) != 0)
	{
		*error = ERROR_INSUFFICIENT_BUFFER;
		return FALSE;
	}
	return TRUE;
}

int
main (int argc, char **argv)
{
	union
	{
		FARPROC symbol;
		FabulorFrontendMain function;
	} frontend_main;
	wchar_t module_path[32768];
	DWORD attributes;
	DWORD error;
	HMODULE frontend;
	int result;

	if (!fabulor_win32_configure_gtk4_runtime (&error))
		return fail_startup (L"initialize its packaged GTK4 runtime", error);
	if (!frontend_module_path (module_path,
		(DWORD) (sizeof (module_path) / sizeof (module_path[0])), &error))
		return fail_startup (L"resolve its GTK4 frontend module", error);

	attributes = GetFileAttributesW (module_path);
	if (attributes == INVALID_FILE_ATTRIBUTES)
		return fail_startup (L"find its GTK4 frontend module", GetLastError ());
	if ((attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0)
		return fail_startup (L"trust its GTK4 frontend module", ERROR_REPARSE_TAG_INVALID);

	frontend = LoadLibraryExW (module_path, NULL,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 |
		LOAD_LIBRARY_SEARCH_USER_DIRS);
	if (frontend == NULL)
		return fail_startup (L"load its GTK4 frontend module", GetLastError ());

	frontend_main.symbol = GetProcAddress (frontend, "fabulor_frontend_main");
	if (frontend_main.symbol == NULL)
	{
		error = GetLastError ();
		FreeLibrary (frontend);
		return fail_startup (L"locate its GTK4 frontend entry point", error);
	}

	result = frontend_main.function (argc, argv);
	return result;
}

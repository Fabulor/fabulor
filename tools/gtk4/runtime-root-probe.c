#include <stdio.h>
#include <wchar.h>
#include <windows.h>

#include "../../src/common/win32-gtk4-runtime.h"

typedef unsigned int (*GtkGetMajorVersionFunc) (void);

static int
fail_with_error (const wchar_t *message, DWORD error)
{
	fwprintf (stderr, L"%ls (Win32 error %lu)\n", message,
		(unsigned long) error);
	return 1;
}

int
main (void)
{
	union
	{
		FARPROC symbol;
		GtkGetMajorVersionFunc function;
	} get_major_version;
	const wchar_t *runtime_bin;
	wchar_t loaded_path[32768];
	wchar_t expected_path[32768];
	HMODULE gtk;
	DWORD error;
	DWORD length;
	int written;

	if (!fabulor_win32_configure_gtk4_runtime (&error))
		return fail_with_error (L"Unable to configure the GTK4 runtime", error);
	if (!fabulor_win32_configure_gtk4_runtime (&error))
		return fail_with_error (L"GTK4 runtime configuration is not idempotent", error);

	runtime_bin = fabulor_win32_gtk4_runtime_bin ();
	if (runtime_bin == NULL)
		return fail_with_error (L"GTK4 runtime path was not retained", ERROR_INVALID_DATA);

	gtk = LoadLibraryExW (L"gtk-4-1.dll", NULL,
		LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
	if (gtk == NULL)
		return fail_with_error (L"Unable to load GTK4 from the configured runtime", GetLastError ());

	length = GetModuleFileNameW (gtk, loaded_path,
		(DWORD) (sizeof (loaded_path) / sizeof (loaded_path[0])));
	if (length == 0 || length == (DWORD) (sizeof (loaded_path) / sizeof (loaded_path[0])))
		return fail_with_error (L"Unable to resolve the loaded GTK4 path", GetLastError ());

	written = swprintf_s (expected_path,
		sizeof (expected_path) / sizeof (expected_path[0]),
		L"%ls\\gtk-4-1.dll", runtime_bin);
	if (written < 0 || _wcsicmp (loaded_path, expected_path) != 0)
	{
		fwprintf (stderr, L"GTK4 loaded from an unexpected path: %ls\n", loaded_path);
		return 1;
	}

	get_major_version.symbol = GetProcAddress (gtk, "gtk_get_major_version");
	if (get_major_version.symbol == NULL)
		return fail_with_error (L"Unable to resolve gtk_get_major_version", GetLastError ());
	if (get_major_version.function () != 4)
	{
		fprintf (stderr, "Unexpected GTK major version\n");
		return 1;
	}

	wprintf (L"GTK4 executable-relative runtime validated: %ls\n", loaded_path);
	FreeLibrary (gtk);
	return 0;
}

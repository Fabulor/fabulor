#ifndef FABULOR_WIN32_GTK4_RUNTIME_H
#define FABULOR_WIN32_GTK4_RUNTIME_H

#ifdef WIN32
#include <windows.h>

BOOL fabulor_win32_configure_gtk4_runtime (DWORD *error_code);
const wchar_t *fabulor_win32_gtk4_runtime_bin (void);
#endif

#endif

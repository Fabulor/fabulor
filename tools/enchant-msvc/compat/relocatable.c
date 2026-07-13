#include "config.h"
#include "relocatable.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

static char current_prefix[MAX_PATH];

static void ensure_current_prefix(void)
{
    HMODULE module = NULL;
    char *slash;

    if (current_prefix[0] != '\0')
        return;

    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR)&ensure_current_prefix, &module) ||
        !GetModuleFileNameA(module, current_prefix, MAX_PATH))
        return;

    slash = strrchr(current_prefix, '\\');
    if (slash)
        *slash = '\0';
}

void set_relocation_prefix(const char *original_prefix, const char *new_prefix)
{
    (void)original_prefix;
    if (!new_prefix || strlen(new_prefix) >= MAX_PATH)
        return;
    strcpy_s(current_prefix, sizeof(current_prefix), new_prefix);
}

const char *relocate(const char *path)
{
    const size_t original_length = strlen(INSTALLPREFIX);
    char *result;
    size_t result_length;

    if (!path || _strnicmp(path, INSTALLPREFIX, original_length) != 0)
        return path;

    ensure_current_prefix();
    if (current_prefix[0] == '\0')
        return path;

    result_length = strlen(current_prefix) + strlen(path + original_length) + 1;
    result = (char *)malloc(result_length);
    if (!result)
        return path;
    strcpy_s(result, result_length, current_prefix);
    strcat_s(result, result_length, path + original_length);
    return result;
}

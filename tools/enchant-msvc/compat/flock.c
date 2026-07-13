#include "config.h"
#include <sys/file.h>

#include <io.h>
#include <windows.h>

int flock(int fd, int operation)
{
    HANDLE file = (HANDLE)_get_osfhandle(fd);
    OVERLAPPED overlapped = {0};
    DWORD flags = 0;

    if (file == INVALID_HANDLE_VALUE)
        return -1;
    if (operation & LOCK_UN)
        return UnlockFileEx(file, 0, MAXDWORD, MAXDWORD, &overlapped) ? 0 : -1;
    if (operation & LOCK_EX)
        flags |= LOCKFILE_EXCLUSIVE_LOCK;
    if (operation & LOCK_NB)
        flags |= LOCKFILE_FAIL_IMMEDIATELY;
    return LockFileEx(file, flags, 0, MAXDWORD, MAXDWORD, &overlapped) ? 0 : -1;
}

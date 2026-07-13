#pragma once

#include <BaseTsd.h>
#include <stddef.h>

#define _GL_CONFIG_H_INCLUDED 1
#define _GL_UNUSED
#define ENABLE_RELOCATABLE 1
#define HAVE_VISIBILITY 0
#define BUILDING_DLL 1
#define DLL_EXPORT 1
#define PACKAGE_NAME "enchant"
#define PACKAGE_VERSION "2.8.19"
#define INSTALLPREFIX "C:/fabulor"
/* Enchant appends its API major suffix to these base paths at runtime. */
#define SYSCONFDIR "C:/fabulor/etc"
#define PKGLIBDIR "C:/fabulor/lib/enchant"
#define PKGDATADIR "C:/fabulor/share/enchant"

#ifndef ssize_t
typedef SSIZE_T ssize_t;
#endif

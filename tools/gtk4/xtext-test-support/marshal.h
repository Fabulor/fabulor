/* Test-only equivalent signal marshalling, without the generated application object. */
#ifndef FABULOR_XTEXT_TEST_MARSHAL_H
#define FABULOR_XTEXT_TEST_MARSHAL_H
#include <glib-object.h>
#define _fabulor_marshal_VOID__OBJECT_OBJECT g_cclosure_marshal_generic
#define _fabulor_marshal_VOID__POINTER_POINTER g_cclosure_marshal_generic
#endif

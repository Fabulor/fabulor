#pragma once

#include <stddef.h>

#define BCP47_MAX 256

static inline void bcp47_to_xpg(char *output, const char *input, const char *codeset)
{
    size_t i;
    (void)codeset;
    for (i = 0; input && input[i] && i + 1 < BCP47_MAX; i++)
        output[i] = input[i] == '-' ? '_' : input[i];
    output[i] = '\0';
}

static inline void xpg_to_bcp47(char *output, const char *input)
{
    size_t i;
    for (i = 0; input && input[i] && i + 1 < BCP47_MAX; i++)
        output[i] = input[i] == '_' ? '-' : input[i];
    output[i] = '\0';
}

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void set_relocation_prefix(const char *original_prefix, const char *current_prefix);
const char *relocate(const char *path);

#ifdef __cplusplus
}
#endif

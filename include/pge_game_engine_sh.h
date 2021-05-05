#ifndef GAME_ENGINE_SH
#define GAME_ENGINE_SH

/* Game engine shared include part */

#include <cstdint>
#include <cstring>

#include "common_defines.h"

#define PGE_VERSION 1
#define PGE_EXPORT extern "C"

PGE_EXPORT uint32_t pge_version();

PGE_EXPORT uint32_t *pge_compilde_shader_path(const char *path, int kind,
		size_t *result_len, bool optimize);
PGE_EXPORT uint32_t *pge_compilde_shader_src(const char *name, const char *src, int kind,
		size_t *result_len, bool optimize);
PGE_EXPORT int pge_free_shader_mem(uint32_t *ptr);

#endif
#ifndef SHADER_COMPILE_H
#define SHADER_COMPILE_H

#include <dlfcn.h>
#include "utils.h"

#define LIB_VERSION 1

enum {
	SHC_VERTEX_SHADER,
	SHC_FRAGMENT_SHADER,
	SHC_COMPUTE_SHADER,
	SHC_GEOMETRY_SHADER,
	SHC_TESS_CONTROL_SHADER,
	SHC_TESS_EVALUATION_SHADER,
};

EXTERN_FN uint32_t *shc_compile_path(const char *path, int kind,
		size_t *result_len, bool optimize);
EXTERN_FN uint32_t *shc_compile_src(const char *name, const char *src, int kind,
		size_t *result_len, bool optimize);
EXTERN_FN int shc_free_shader(uint32_t *ptr);
EXTERN_FN int shc_get_version();

/* Struct meant for fast loading the .so into a lib */
struct ShaderC {
	void *handle = nullptr;
	SO_DECLARE_FN(shc_compile_path)
	SO_DECLARE_FN(shc_compile_src)
	SO_DECLARE_FN(shc_free_shader)
	SO_DECLARE_FN(shc_get_version)

	int load(const char *path) {
		handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
		if (!handle) {
			DBG("Couldn't load lib from path: %s, err: %s", path, dlerror());
			return -1;
		}
		bool success = false;
		do {
			SO_LOAD_FN(handle, shc_compile_path);
			SO_LOAD_FN(handle, shc_compile_src);
			SO_LOAD_FN(handle, shc_free_shader);
			SO_LOAD_FN(handle, shc_get_version);
			success = true;
		}
		while (false);
		if (!success) {
			unload();
			return -1;
		}
		int lib_ver = shc_get_version_fn();
		if (lib_ver != LIB_VERSION) {
			DBG("Version missmatch: lib's %d vs ours %d", lib_ver, LIB_VERSION);
			unload();
			return -1;
		}
		return 0;
	}

	void unload() {
		if (handle) {
			dlclose(handle);
			handle = nullptr;
		}
	}
};

#endif

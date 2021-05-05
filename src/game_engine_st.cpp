#include "game_engine_st.h"
#include "pge_game_engine_sh.h"

#include "utils.h"

#include <vector>
#include <dlfcn.h>

#define FN_ENTRY(fnname) \
using fnname ## _t = decltype(&::fnname); \
fnname ## _t real_ ## fnname = NULL;\
template <typename ...Args>\
auto fnname (Args&& ...args) {\
	if (!real_ ## fnname)\
		EXCEPTION("game_engine shared lib was not loaded, try to put it near "\
				"the executable with the name ./libgame_engine.so or call "\
				"ge::init() first. If those don't work, check logs");\
	return real_ ## fnname(std::forward<Args>(args)...);\
}

#define LOAD_FN_SYM(lib, fnname) \
real_ ## fnname = (fnname ## _t)dlsym(lib, #fnname);\
if (!real_ ## fnname) {\
	DBG("Couldn't load symbol %s, aborting", #fnname);\
	return -1;\
}

struct EngineLib {
	bool init_done = false;
	FN_ENTRY(pge_version);
	FN_ENTRY(pge_compilde_shader_path);
	FN_ENTRY(pge_compilde_shader_src);
	FN_ENTRY(pge_free_shader_mem);

	EngineLib() {
		if (!init_done) {
			if (load("./libgame_engine.so") == 0) {
				init_done = true ;
				return ;
			}
		}
	}

	int load(const char *path) {
		auto lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
		if (!lib) {
			DBG("[WARNING] Couldn't load shared object from path: %s, err: %s",
					path, dlerror());
			return -1;
		}
		LOAD_FN_SYM(lib, pge_version);
		LOAD_FN_SYM(lib, pge_compilde_shader_path);
		LOAD_FN_SYM(lib, pge_compilde_shader_src);
		LOAD_FN_SYM(lib, pge_free_shader_mem);

		if (pge_version() != PGE_VERSION) {
			DBG("[ERROR] You are using a different static version from "
					"the shared part");
			return -1;
		}
		DBG("[INFO] Loaded game_engine shared from %s", path);
		return 0;
	}
};

static EngineLib lib;

namespace pge
{

int init(const char *path) {
	return lib.load(path);
}

std::vector<uint32_t> compile_shader_src(const std::string& name,
		const std::string& source, int kind, bool optimize)
{
	size_t len = 0;
	auto buff = lib.pge_compilde_shader_src(name.c_str(), source.c_str(),
			kind, &len, optimize);
	std::vector<uint32_t> ret(len);
	memcpy(ret.data(), buff, len * sizeof(uint32_t));
	lib.pge_free_shader_mem(buff);
	return ret;
}

std::vector<uint32_t> compile_shader_path(const std::string& path,
		int kind, bool optimize)
{
	size_t len = 0;
	auto buff = lib.pge_compilde_shader_path(path.c_str(), kind, &len, optimize);
	std::vector<uint32_t> ret(len);
	memcpy(ret.data(), buff, len * sizeof(uint32_t));
	lib.pge_free_shader_mem(buff);
	return ret;
}

}
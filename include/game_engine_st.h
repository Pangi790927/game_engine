#ifndef GAME_ENGINE_ST_H
#define GAME_ENGINE_ST_H

/* Game engine static include part */

#include <string>
#include <vector>

#include "common_defines.h"

namespace pge
{

// not necesary to call it, but maybe you want to load the shared part from a
// custom location
int init(const char *path);

std::vector<uint32_t> compile_shader_src(const std::string& name,
		const std::string& source, int kind, bool optimize = true);

std::vector<uint32_t> compile_shader_path(const std::string& path,
		int kind, bool optimize = true);

}

#endif
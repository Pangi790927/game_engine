#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

/* 	Pangi's Graphics Engine */
/* 	Game engine final include part, this should be used by users.
	To find out what you shuld find in game engine object's config go look in
the respective constructor */

#include <fstream>
#include <filesystem>

#include "json.h"

#include "pge_drawcore.h"
#include "pge_window.h"
#include "pge_renderer.h"
#include "pge_texture.h"
#include "pge_model.h"

namespace pge
{

nlohmann::json load_config(const std::string& path) try {
	nlohmann::json jret;
	std::ifstream in(path);
	in >> jret;
	jret["base_path"] = std::filesystem::canonical(path).parent_path().string()
			+ "/";
	return jret;
} catch (std::exception &e) {
	EXCEPTION("load config failed: %s", e.what());
}

}

#endif

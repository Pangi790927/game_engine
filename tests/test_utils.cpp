#include "utils.h"

int main(int argc, char const *argv[])
{
	auto cfg = pge::load_config("configs/test_config.json");
	auto window_cfg = JSON_GET(cfg, "window");
	DBG("window_name: %s", JSON_STR(window_cfg, "window_name"));
	DBG("window_width: %d", JSON_INT(window_cfg, "width"));
	DBG("window_debug: %d", JSON_BOOL(window_cfg, "debug_mode"));

	try {
		DBG("window_debug: %d", JSON_BOOL(window_cfg, "debug_modes"));
	}
	catch (std::exception &e) {
		DBG("Intentional exception: %s", e.what());
	}
	return 0;
}
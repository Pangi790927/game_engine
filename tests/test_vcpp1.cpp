#include "pge.h"
#include "pge_pipeline.h"

int main(int argc, char const *argv[])
{
/* INIT:
============================================================================= */
	using namespace pge;
	nlohmann::json jconfig = load_config("../../engine_config.json");
	DBG("base_path: %s", jconfig["base_path"]);
	DrawCore dc(jconfig); 
/* LOOP:
============================================================================= */
	// DBG("====================================================================");
	// DBG("Context Create Test");
	// PgeWindow window(600,  800, "Simple Vulkan Setup");
	// DrawPipeline pipeline(&window);
	// pipeline.begin_pipeline()
	// 	-> add_vertex_input()
	// 	-> add_input_assembly({
	// 		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	// 	})
	// 	-> add_viewport()
	// 	-> add_vertex_shader({
	// 		.info = { .path = "../../shaders/test_shader.vert" }
	// 	})
	// 	-> add_rasterizer()
	// 	-> add_multisampler()
	// 	-> add_fragment_shader({
	// 		.info = { .path = "../../shaders/test_shader.frag" }
	// 	})
	// 	-> add_color_blending()
	// 	-> add_layouts()
	// 	-> add_render_subpass({})
	// 	-> end_pipeline();

	// DBG("====================================================================");

    /* code */
    return 0;
}

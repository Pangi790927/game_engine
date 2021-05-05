#ifndef PGE_DRAWCORE_H
#define PGE_DRAWCORE_H

#include "json.h"

namespace pge
{

struct GlfwIniter {
    GlfwIniter() {
        pge::init("../src/libgame_engine_shared.so");
        if (!glfwInit())
            EXCEPTION("Failed to init glfw");
    }
    ~GlfwIniter() {
        glfwTerminate();
    }
};


struct DrawCore {
	struct CoreDataScope {
		GLFWwindow *window = nullptr;
		VkInstance instance = nullptr;
		VkDebugUtilsMessengerEXT dbg_msger = nullptr;
		// VkSurfaceKHR surface = nullptr;
		VkDevice device = nullptr;
		// VkSwapchainKHR swapchain = nullptr;
		// std::vector<VkImageView> swap_img_views;

		~CoreDataScope() {
			if (device)
				vkDestroyDevice(device, nullptr);
			if (dbg_msger)
				DestroyDebugUtilsMessengerEXT(instance, dbg_msger, nullptr);
			if (instance)
				vkDestroyInstance(instance, nullptr);
		}
	};
	std::unique_ptr<CoreDataScope> d;

	DrawCore(const Config& cfg) {
        static GlfwIniter glfw_initer;
        d = std::make_unique<CoreDataScope>();

	}

	void wait_idle() {
		if (device)
			vkDeviceWaitIdle(device);
	}
};

}

#endif
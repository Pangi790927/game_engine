#include "glfw_vulkan_if.h"

#include <iostream>

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported\n";

	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();


		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}
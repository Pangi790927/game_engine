/* INCLUDE:
============================================================================= */

#include <iostream>
#include <vector>
#include <cstring>
#include <set>
#include <map>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "utils.h"
#include "shader_compile.h"

/* CONFIG:
============================================================================= */
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

// const uint32_t min_dbg_severity = 0;
const uint32_t min_dbg_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

/* HELPER FUNCTIONS:
============================================================================= */

bool checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (int i = 0; auto &&layer : availableLayers)
		DBG("Validation layer[%2d]: %s", i++, layer.layerName);

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
			glfwExtensions + glfwExtensionCount);

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cbk(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
{
	if (messageSeverity < min_dbg_severity)
		return VK_FALSE;

	const char *msg_severity = nullptr;
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			msg_severity = "[VERBOSE]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			msg_severity = "[INFO  ]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			msg_severity = "[WARNING]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			msg_severity = "[ERROR ]";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
			msg_severity = "[MAX_ENUM_HUGE_FATAL_MORTAL_COLOSAL_ERROR]";
			break;
	}
	DBG("[VULKAN_DBG]%s: %s", msg_severity, pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    		instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
    		instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

int rateDeviceSuitability(VkPhysicalDevice device) {
	int score = 1000;
	VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	if (deviceProperties.limits.maxImageDimension2D > 1000)
		score += 1000;

	DBG("Device: [%s] rate: %d", deviceProperties.deviceName, score);
	return score;
}

int main() {

/* GLFW INIT:
============================================================================= */

	DBG("====================================================================");
	if (!glfwInit())
		EXCEPTION("Failed to init glfw");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "window", NULL, NULL);
	if (!window)
		EXCEPTION("Can't create glfw window");
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

/* EXTENSION AND VALIDATION LAYER ENUMERATION:
============================================================================= */

	uint32_t ext_cnt = 0;
	vkEnumerateInstanceExtensionProperties((const char *)NULL, &ext_cnt, NULL);
	DBG("0x%d extensions supported", ext_cnt);

	std::vector<VkExtensionProperties> exts(ext_cnt);
	vkEnumerateInstanceExtensionProperties(nullptr, &ext_cnt, exts.data());

	for (int i = 0; auto &&ext : exts)
		DBG("Extension[%2d]: %s", i++, ext.extensionName);

	if (!checkValidationLayerSupport())
		EXCEPTION("No validation layers");

	auto extensions = getRequiredExtensions();

	for (int i = 0; auto &&ext_name : extensions)
		DBG("Enabled extensions[%2d]: %s", i++, ext_name);

/* INSTANCE AND DEBUG INFO SETUP:
============================================================================= */

	VkInstance instance{};
	VkDebugUtilsMessengerEXT debugMessenger{};

	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};

	VkDebugUtilsMessengerCreateInfoEXT msgInfoInstance{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debug_cbk,
		.pUserData = nullptr,
	};

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &msgInfoInstance,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		EXCEPTION("Can't create vulkan instance");

	VkDebugUtilsMessengerCreateInfoEXT msgInfo{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debug_cbk,
		.pUserData = nullptr,
	};

	if (CreateDebugUtilsMessengerEXT(instance, &msgInfo, nullptr,
			&debugMessenger) != VK_SUCCESS)
	{
		EXCEPTION("failed to set up debug messenger!");
	}

/* WINDOW SURFACE:
============================================================================= */

	VkSurfaceKHR surface;

	if (glfwCreateWindowSurface(instance, window, nullptr, &surface)!= VK_SUCCESS)
		EXCEPTION("failed to create window surface!");


/* PHYSICAL DEVICE SELECTION AND LOGICAL DEVICE CREATION:
============================================================================= */

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	    EXCEPTION("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	int best_score = rateDeviceSuitability(devices[0]);
	physicalDevice = devices[0];
	for (size_t i = 1; i < devices.size(); i++) {
		int score = rateDeviceSuitability(devices[i]);
		if (score > best_score) {
			best_score = score;
			physicalDevice = devices[i];
		}
	}

	// Check if the best candidate is suitable at all
	if (best_score <= 0 || physicalDevice == VK_NULL_HANDLE)
		EXCEPTION("failed to find a suitable GPU!");

	// make rate consider checking for extension support and for format, pres
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_cnt,
			nullptr);
	std::vector<VkExtensionProperties> availableExtensions(ext_cnt);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_cnt,
			availableExtensions.data());
	std::set<std::string> requiredExtensions(
			deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
	    requiredExtensions.erase(extension.extensionName);
	}
	if (!requiredExtensions.empty())
		EXCEPTION("Missing required extensions");

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
			&capabilities);

	std::vector<VkSurfaceFormatKHR> formats;
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
			&formatCount, nullptr);
	
	if (formatCount == 0)
		EXCEPTION("empty surface format");

	formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
			&formatCount, formats.data());

	std::vector<VkPresentModeKHR> presentModes;
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
			&presentModeCount, nullptr);

	if (presentModeCount == 0)
		EXCEPTION("empty surface presentation modes");

	presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
			&presentModeCount, presentModes.data());

	VkSurfaceFormatKHR selected_surface_format = formats[0];
	VkPresentModeKHR selected_surface_presentation = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availableFormat : formats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			selected_surface_format = availableFormat;
			break;
		}

	VkExtent2D actualExtent = capabilities.currentExtent;
	if (capabilities.currentExtent.width == UINT32_MAX) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		actualExtent = VkExtent2D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));
	}
	
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 &&
			imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	// make rate consider checking for valid queue
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
			physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
			physicalDevice, &queueFamilyCount, queueFamilies.data());

	int graphic_index = -1;
	int presentation_index = -1;
	for (int i = 0; const auto& queueFamily : queueFamilies) {
		DBG("Supported queue for our device: %x", queueFamily.queueFlags);
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			graphic_index = i;

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
				&presentSupport);
		if (presentSupport == VK_TRUE)
			presentation_index = i;
		i++;
	}

	if (graphic_index < 0 || presentation_index < 0)
		EXCEPTION("No suitable device queue found");

	VkSwapchainCreateInfoKHR swapchainInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = selected_surface_format.format,
		.imageColorSpace = selected_surface_format.colorSpace,
		.imageExtent = actualExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = selected_surface_presentation,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	uint32_t queueFamilyIndices[] = {
		(uint32_t)graphic_index,
		(uint32_t)presentation_index
	};
	if (graphic_index != presentation_index) {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0; // Optional
		swapchainInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	float queuePriorityes = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> que_infos;
	std::set<int> unique_que_families{ graphic_index, presentation_index };
	for (int que_family : unique_que_families) {
		que_infos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = (uint32_t)que_family,
			.queueCount = 1,
			.pQueuePriorities = &queuePriorityes,
		});
	}

	// for now, everything is on none
	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(que_infos.size()),
		.pQueueCreateInfos = que_infos.data(),
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	VkDevice device;
	if (vkCreateDevice(physicalDevice, &deviceInfo, NULL, &device) != VK_SUCCESS)
		EXCEPTION("failed to create logical device!");

	VkSwapchainKHR swapChain;
	if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr,
			&swapChain) != VK_SUCCESS)
	{
		EXCEPTION("failed to create swap chain!");
	}

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	vkGetDeviceQueue(device, graphic_index, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentation_index, 0, &presentQueue);

/* SWAPCHAIN IMAGES:
============================================================================= */
	std::vector<VkImage> swapChainImages;

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);

	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
			swapChainImages.data());

	VkFormat swapChainImageFormat = selected_surface_format.format;
	VkExtent2D swapChainExtent = actualExtent;

	std::vector<VkImageView> swapChainImageViews;
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapChainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapChainImageFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		if (vkCreateImageView(device, &viewInfo, nullptr,
				&swapChainImageViews[i]) != VK_SUCCESS)
		{
			EXCEPTION("failed to create image views!");
		}
	}

/* SHADERS:
============================================================================= */
	ShaderC shaderc_lib;
	if (shaderc_lib.load("./libshc.so") != 0) {
		DBG("Couldn't load shader compiler lib");
		return -1;
	}

	DBG("Will load shaders");

	// Obs: not needed after module creation
	// Obs: slow, should cache and save to disk
	size_t vs_code_len = 0;
	auto vert_code = shaderc_lib.shc_compile_path_fn(
			"shaders/test_shader.vert", SHC_VERTEX_SHADER, &vs_code_len, true);
	size_t fs_code_len = 0;
	auto frag_code = shaderc_lib.shc_compile_path_fn(
			"shaders/test_shader.frag", SHC_FRAGMENT_SHADER, &fs_code_len, true);

	DBG("Will register loaded shaders");
	VkShaderModuleCreateInfo vertInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = vs_code_len * sizeof(uint32_t),
		.pCode = vert_code,
	};

	VkShaderModule vertShaderModule;
	if (vkCreateShaderModule(device, &vertInfo, nullptr,
			&vertShaderModule) != VK_SUCCESS)
	{
		EXCEPTION("failed to create shader module!");
	}

	VkShaderModuleCreateInfo fragInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = fs_code_len * sizeof(uint32_t),
		.pCode = frag_code,
	};

	VkShaderModule fragShaderModule;
	if (vkCreateShaderModule(device, &fragInfo, nullptr,
			&fragShaderModule) != VK_SUCCESS)
	{
		EXCEPTION("failed to create shader module!");
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo, fragShaderStageInfo
	};

/* PIPELINE CREATION:
============================================================================= */

	DBG("Will create pipeline");
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr, // Optional
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr, // Optional
	};

	// VK_PRIMITIVE_TOPOLOGY_POINT_LIST
	// VK_PRIMITIVE_TOPOLOGY_LINE_LIST
	// VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
	// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor{
		.offset = {0, 0},
		.extent = swapChainExtent,
	};

	VkPipelineViewportStateCreateInfo viewportState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	// VK_POLYGON_MODE_FILL
	// VK_POLYGON_MODE_LINE
	// VK_POLYGON_MODE_POINT
	// Using any mode other than fill requires enabling a GPU feature.
	// any line thicker than 1.0f requires you to enable the wideLines GPU feature
	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f, // Optional
		.depthBiasClamp = 0.0f, // Optional
		.depthBiasSlopeFactor = 0.0f, // Optional
		.lineWidth = 1.0f,
	};

	// disabled for now
	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f, // Optiona,
		.pSampleMask = nullptr, // Optiona,
		.alphaToCoverageEnable = VK_FALSE, // Optiona,
		.alphaToOneEnable = VK_FALSE, // Optiona,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
		.colorBlendOp = VK_BLEND_OP_ADD, // Optional
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
		.alphaBlendOp = VK_BLEND_OP_ADD, // Optional
		.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY, // Optional
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0, // Optional
		.pSetLayouts = nullptr, // Optional
		.pushConstantRangeCount = 0, // Optional
		.pPushConstantRanges = nullptr, // Optional
	};

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
			&pipelineLayout) != VK_SUCCESS)
	{
		EXCEPTION("failed to create pipeline layout!");
	}

	VkAttachmentDescription colorAttachment {
    	.format = swapChainImageFormat,
    	.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkSubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr,
			&renderPass) != VK_SUCCESS)
	{
	    EXCEPTION("failed to create render pass!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlending,
		.pDynamicState = nullptr,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE, // Optional
		.basePipelineIndex = -1, // Optional
	};

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
			nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		EXCEPTION("failed to create graphics pipeline!");
	}

	// not needed after pipeline creation
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);

	DBG("Registered shaders");

/* Framebuffers:
============================================================================= */

	std::vector<VkFramebuffer> swapChainFramebuffers;
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			swapChainImageViews[i],
		};

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = swapChainExtent.width,
			.height = swapChainExtent.height,
			.layers = 1,
		};

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
				&swapChainFramebuffers[i]) != VK_SUCCESS)
		{
		    EXCEPTION("failed to create framebuffer!");
		}
	}

/* COMMANDS:
============================================================================= */

	VkCommandPool commandPool;

	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = 0, // needed for something
		.queueFamilyIndex = (uint32_t)graphic_index,
	};

	if (vkCreateCommandPool(device, &poolInfo, nullptr,
			&commandPool) != VK_SUCCESS)
	{
		EXCEPTION("failed to create command pool!");
	}

	DBG("swap chain count: %ld", swapChainFramebuffers.size());
	std::vector<VkCommandBuffer> commandBuffers(swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)commandBuffers.size(),
	};

	if (vkAllocateCommandBuffers(device, &allocInfo,
			commandBuffers.data()) != VK_SUCCESS)
	{
		EXCEPTION("failed to allocate command buffers!");
	}

	DBG("Will register draw commands");
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // Optional
			.pInheritanceInfo = nullptr, // Optional
		};

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			EXCEPTION("failed to begin recording command buffer!");

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		VkRenderPassBeginInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPass,
			.framebuffer = swapChainFramebuffers[i],
			.renderArea = {
				.offset = {0, 0},
				.extent = swapChainExtent,
			},
			.clearValueCount = 1,
			.pClearValues = &clearColor,
			
		};
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
				VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				graphicsPipeline);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			EXCEPTION("failed to record command buffer!");
	}

/* SEMAPHORES:
============================================================================= */

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
			&imageAvailableSemaphore) != VK_SUCCESS)
	{
		EXCEPTION("Failed to create image available semaphore");
	}

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
			&renderFinishedSemaphore) != VK_SUCCESS)
	{
		EXCEPTION("Failed to create render finished semaphore");
	}

/* MAIN LOOP:
============================================================================= */

	DBG("Will start main loop");
	pge::TimePointMs tp;
	while(!glfwWindowShouldClose(window) && tp.elapsed() < 1000) {
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, 
				imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		};
		VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffers[imageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signalSemaphores,
		};
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
				VK_NULL_HANDLE) != VK_SUCCESS)
		{
			EXCEPTION("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional
		vkQueuePresentKHR(presentQueue, &presentInfo);
		vkQueueWaitIdle(presentQueue);
	}

/* FREE RESOURCES:
============================================================================= */

	vkDeviceWaitIdle(device);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	for (auto imageView : swapChainImageViews) {
	    vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
	DBG("====================================================================");
	return 0;
}
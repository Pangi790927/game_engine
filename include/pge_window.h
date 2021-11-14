#ifndef PGE_WINDOW
#define PGE_WINDOW

#include <functional>
#include <set>

#include "glfw_vulkan_if.h"
#include "utils.h"
#include "game_engine_st.h"
#include "pge_common.h"

#define MIN_DBG_SEVERITY VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT

#define SCOPED_VAR(name, type, deleter, init_val)\
    std::function<void(type *)> name ## _deleter = deleter;\
    std::unique_ptr<type, decltype(name ## _deleter)> name = init_val;


namespace pge
{

struct GlfwIniter {
    GlfwIniter(const Config& cfg) {
        pge::init(JSTR(cfg, "libgame_path"));
        if (!glfwInit())
            EXCEPTION("Failed to init glfw");
    }
    ~GlfwIniter() {
        glfwTerminate();
    }
};

/* The window will hold the glfw window, the vulkan instance, vulkan logical
device and swapbuffers. I don't see a need to configure any of those for the
game engine as they are only configuring normal stuff. Maybe later on, some
options should be added for some of the initialization values, but anything
more seems like a huge waste of time.
    Everything will be destroyed at window destruction.
*/
struct Window {
    struct dev_t {
        VkPhysicalDevice phy_dev;
        int score;
        uint32_t graphic_index;
        uint32_t presentation_index;
        VkSurfaceCapabilitiesKHR capab;
        VkSurfaceFormatKHR surf_fmt;
        VkPresentModeKHR surf_pres;
        VkExtent2D extent;
        uint32_t swch_img_cnt;
        VkQueue graphic_queue;
        VkQueue present_queue;
    };

    struct WindowDataScope {
        GLFWwindow *window = nullptr;
        VkInstance instance = nullptr;
        VkDebugUtilsMessengerEXT dbg_msger = nullptr;
        VkSurfaceKHR surface = nullptr;
        VkDevice device = nullptr;
        VkSwapchainKHR swapchain = nullptr;
        std::vector<VkImageView> swap_img_views;

        ~WindowDataScope() {
            if (device)
                vkDeviceWaitIdle(device);

            
            for (auto img_view : swap_img_views)
                vkDestroyImageView(device, img_view, nullptr);
            if (swapchain)
                vkDestroySwapchainKHR(device, swapchain, nullptr);
            if (device)
                vkDestroyDevice(device, nullptr);
            if (surface)
                vkDestroySurfaceKHR(instance, surface, nullptr);
            if (dbg_msger)
                DestroyDebugUtilsMessengerEXT(instance, dbg_msger, nullptr);
            if (instance)
                vkDestroyInstance(instance, nullptr);
            if (window)
                glfwDestroyWindow(window);
        }
    };
    std::unique_ptr<WindowDataScope> d;
    dev_t dev;

    Window(const Config& cfg) {
        /* initialize glfw */
        d = std::make_unique<WindowDataScope>();
        static GlfwIniter glfw_initer(cfg);

        /* create glfw window */
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        d->window = glfwCreateWindow(JINT(cfg, "width"), JINT(cfg, "height"),
                JSTR(cfg, "window_name"), nullptr, nullptr);
        if (!d->window)
            EXCEPTION("Can't create glfw window");
        glfwSetInputMode(d->window, GLFW_STICKY_KEYS, GLFW_TRUE);

        bool debug_mode = JBOOL(cfg, "debug_mode");
        if (debug_mode && check_dbg_support() == false)
            EXCEPTION("Can't add validation layers");

        /* get required instance extensions */
        auto req_exts = get_required_inst_extensions();

        /* create vulkan instance */
        VkApplicationInfo app_info{
		    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = JSTR(cfg, "app_name"),
            .pEngineName = JSTR(cfg, "engine_name"),
            .apiVersion = VK_API_VERSION_1_1
        };
        VkDebugUtilsMessengerCreateInfoEXT dbgmsg_info{
		    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
            .messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_cbk,
            .pUserData = nullptr,
        };
        const char* validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
        VkInstanceCreateInfo inst_info{
		    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &dbgmsg_info,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = uint32_t(debug_mode ? 1 : 0),
            .ppEnabledLayerNames = debug_mode ? validation_layers : nullptr,
            .enabledExtensionCount = uint32_t(req_exts.size()),
            .ppEnabledExtensionNames = req_exts.data(),
        };
        if (vkCreateInstance(&inst_info, nullptr, &d->instance) != VK_SUCCESS)
            EXCEPTION("Can't create vulkan instance");
        
        if (CreateDebugUtilsMessengerEXT(d->instance, &dbgmsg_info, nullptr,
			    &d->dbg_msger) != VK_SUCCESS)
            EXCEPTION("failed to set up debug messenger!");

        /* create window surface */
        if (glfwCreateWindowSurface(d->instance, d->window,
                nullptr, &d->surface)!= VK_SUCCESS)
    		EXCEPTION("failed to create window surface!");
        
        /* select a physical device */
        dev = select_phy_dev();

        VkSwapchainCreateInfoKHR swapchain_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = d->surface,
            .minImageCount = dev.swch_img_cnt,
            .imageFormat = dev.surf_fmt.format,
            .imageColorSpace = dev.surf_fmt.colorSpace,
            .imageExtent = dev.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = dev.capab.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = dev.surf_pres,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        // TODO: Find why when praphic == pres, we use exclusive
        uint32_t queue_indices[] = {
                dev.graphic_index, dev.presentation_index };
        if (dev.graphic_index != dev.presentation_index) {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_info.queueFamilyIndexCount = 2;
            swapchain_info.pQueueFamilyIndices = queue_indices;
        } else {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_info.queueFamilyIndexCount = 0; // Optional
            swapchain_info.pQueueFamilyIndices = nullptr; // Optional
        }

        float que_priority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> que_infos;
        std::set<uint32_t> unique_que_indexes{
                dev.graphic_index, dev.presentation_index };
        for (uint32_t que_index : unique_que_indexes) {
            que_infos.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = que_index,
                .queueCount = 1,
                .pQueuePriorities = &que_priority,
            });
        }

        // for now, we don't use any feature
        VkPhysicalDeviceFeatures dev_features{};

        std::vector<const char*> dev_exts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkDeviceCreateInfo dev_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = uint32_t(que_infos.size()),
            .pQueueCreateInfos = que_infos.data(),
            .enabledLayerCount = uint32_t(debug_mode ? 1 : 0),
            .ppEnabledLayerNames = debug_mode ? validation_layers : nullptr,
            .enabledExtensionCount = uint32_t(dev_exts.size()),
            .ppEnabledExtensionNames = dev_exts.data(),
            .pEnabledFeatures = &dev_features,
        };

        /* create de logical device */
        if (vkCreateDevice(dev.phy_dev, &dev_info, NULL,
                &d->device) != VK_SUCCESS)
            EXCEPTION("failed to create logical device!");

        if (vkCreateSwapchainKHR(d->device, &swapchain_info, nullptr,
                &d->swapchain) != VK_SUCCESS)
        {
            EXCEPTION("failed to create swap chain!");
        }

        /* get queues for logical device */
        vkGetDeviceQueue(d->device, dev.graphic_index, 0,
                &dev.graphic_queue);
        vkGetDeviceQueue(d->device, dev.presentation_index, 0,
                &dev.present_queue);

        /* create swap images views */
        uint32_t cnt = 0;
        vkGetSwapchainImagesKHR(d->device, d->swapchain, &cnt, nullptr);

        std::vector<VkImage> swap_imgs(cnt);
        vkGetSwapchainImagesKHR(d->device, d->swapchain, &cnt, swap_imgs.data());

        d->swap_img_views.resize(swap_imgs.size());

        for (size_t i = 0; i < swap_imgs.size(); i++) {
            VkImageViewCreateInfo view_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swap_imgs[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = dev.surf_fmt.format,
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
            if (vkCreateImageView(d->device, &view_info, nullptr,
                    &d->swap_img_views[i]) != VK_SUCCESS)
            {
                EXCEPTION("failed to create image views!");
            }
        }
    }

    void wait_idle() {
        if (d && d->device)
            vkDeviceWaitIdle(d->device);
    }

private:
    dev_t select_phy_dev() {
        uint32_t dev_cnt = 0;
        vkEnumeratePhysicalDevices(d->instance, &dev_cnt, nullptr);

        if (dev_cnt == 0)
            EXCEPTION("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(dev_cnt);
        vkEnumeratePhysicalDevices(d->instance, &dev_cnt, devices.data());

        dev_t ret_dev = get_phy_dev(devices[0]);
        int best_score = ret_dev.score;
        for (size_t i = 1; i < devices.size(); i++) {
            auto dev = get_phy_dev(devices[i]);
            if (dev.score > best_score) {
                best_score = dev.score;
                ret_dev = dev;
            }
        }

        if (best_score < 0)
            EXCEPTION("Couldn't find a suitable physical device");

        return ret_dev;
    }

    dev_t get_phy_dev(VkPhysicalDevice phy_dev) {
        dev_t ret_dev;
        ret_dev.phy_dev = phy_dev;
        ret_dev.score = 1000;

        // filter out devices without swapchains
        uint32_t cnt = 0;
        std::vector<const char*> req_exts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        vkEnumerateDeviceExtensionProperties(phy_dev, nullptr, &cnt, nullptr);
        std::vector<VkExtensionProperties> exts(cnt);
        vkEnumerateDeviceExtensionProperties(phy_dev, nullptr, &cnt, exts.data());
        std::set<std::string> req_exts_set(req_exts.begin(), req_exts.end());
        for (const auto& ext : exts)
            req_exts_set.erase(ext.extensionName);
        if (!req_exts_set.empty()) {
            DBG("Device does not have swapchain extension");
            return { .score = -1 };
        }

        cnt = 0;
        VkSurfaceCapabilitiesKHR capab;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_dev, d->surface, &capab);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phy_dev, d->surface, &cnt, nullptr);

        if (cnt == 0) {
            DBG("Device does not have surface formats");
            return { .score = -1 };
        }

        std::vector<VkSurfaceFormatKHR> fmts(cnt);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
                phy_dev, d->surface, &cnt, fmts.data());

        cnt = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
                phy_dev, d->surface, &cnt, nullptr);

        if (cnt == 0) {
            DBG("Device does not have presentation modes");
            return { .score = -1 };
        }

        std::vector<VkPresentModeKHR> pres_modes(cnt);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
                phy_dev, d->surface, &cnt, pres_modes.data());

        ret_dev.capab = capab;
        ret_dev.surf_fmt = fmts[0];
        ret_dev.surf_pres = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& fmt : fmts)
            if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                ret_dev.surf_fmt = fmt;
                break;
            }

        VkExtent2D extent = capab.currentExtent;
        if (capab.currentExtent.width == UINT32_MAX) {
            int width, height;
            glfwGetFramebufferSize(d->window, &width, &height);

            extent = VkExtent2D{ uint32_t(width), uint32_t(height) };

            extent.width = std::max(capab.minImageExtent.width,
                    std::min(capab.maxImageExtent.width, extent.width));
            extent.height = std::max(capab.minImageExtent.height,
                    std::min(capab.maxImageExtent.height, extent.height));
        }

        ret_dev.extent = extent;

        uint32_t img_cnt = capab.minImageCount + 1;
        if (capab.maxImageCount > 0 && img_cnt > capab.maxImageCount)
            img_cnt = capab.maxImageCount;

        ret_dev.swch_img_cnt = img_cnt;

        cnt = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phy_dev, &cnt, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(cnt);
        vkGetPhysicalDeviceQueueFamilyProperties(
                phy_dev, &cnt, queue_families.data());
        for (int i = 0; const auto& queue_family : queue_families) {
            DBG("Supported queue for our device: %x", queue_family.queueFlags);
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                ret_dev.graphic_index = (uint32_t)i;

            VkBool32 pres_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(phy_dev, i, d->surface,
                    &pres_support);
            if (pres_support == VK_TRUE)
                ret_dev.presentation_index = (uint32_t)i;
            i++;
        }

        if (ret_dev.graphic_index < 0 || ret_dev.presentation_index < 0) {
            DBG("No suitable device queue found");
            return { .score = -1 };
        }

        VkPhysicalDeviceProperties dev_props;
        VkPhysicalDeviceFeatures dev_features;
        vkGetPhysicalDeviceProperties(phy_dev, &dev_props);
        vkGetPhysicalDeviceFeatures(phy_dev, &dev_features);

        // Discrete GPUs have a significant performance advantage
        if (dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ret_dev.score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        if (dev_props.limits.maxImageDimension2D > 1000)
            ret_dev.score += 1000;

        DBG("Device: [%s] rate: %d", dev_props.deviceName, ret_dev.score);
        return ret_dev;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
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

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cbk(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* data,
            void* ctx)
    {
        if (severity < MIN_DBG_SEVERITY)
            return VK_FALSE;

        const char *msg_severity = nullptr;
        switch (severity) {
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
        DBG("[VULKAN_DBG]%s: %s", msg_severity, data->pMessage);

        return VK_FALSE;
    }

    std::vector<const char*> get_required_inst_extensions() {
        uint32_t cnt = 0;
        const char**glfw_exts = glfwGetRequiredInstanceExtensions(&cnt);

        std::vector<const char*> req_exts(glfw_exts, glfw_exts + cnt);
        req_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        for (auto &&ext : req_exts)
            DBG("Required extensions: %s", ext);
        
        return req_exts;
    }

    bool check_dbg_support() {
        uint32_t cnt = 0;
        vkEnumerateInstanceLayerProperties(&cnt, nullptr);

        std::vector<VkLayerProperties> layers(cnt);
        vkEnumerateInstanceLayerProperties(&cnt, layers.data());

        for (auto &&layer : layers) 
            if (strcmp("VK_LAYER_KHRONOS_validation", layer.layerName) == 0)
                return true;
        
        return false;
    }
};

} // namespace pge

#endif

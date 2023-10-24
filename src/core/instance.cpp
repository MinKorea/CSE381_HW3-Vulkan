// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "instance.hpp"

// OUR OWN TYPES
#include "common/common.hpp"
#include "common/logging.hpp"
#include "common/utils.hpp"

#include "device.hpp"
#include "physical_device.hpp"
#include "window.hpp"

#ifdef NDEBUG
constexpr const bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr const bool ENABLE_VALIDATION_LAYERS = true;
#endif

PFN_vkCreateDebugUtilsMessengerEXT  create_debug_utils_messenger;
PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance                                instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks              *pAllocator,
                                                              VkDebugUtilsMessengerEXT                 *pMessenger)
{
	return create_debug_utils_messenger(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator)
{
	return destroy_debug_utils_messenger(instance, messenger, pAllocator);
}

namespace W3D
{

const std::vector<const char *> Instance::VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};

Instance::Instance(const std::string &app_name, Window &window)
{
	create_instance(app_name);
	surface_ = window.create_surface(*this);
}

Instance::~Instance()
{
	if (ENABLE_VALIDATION_LAYERS)
	{
		handle_.destroyDebugUtilsMessengerEXT(debug_messenger_);
	}
	handle_.destroySurfaceKHR(surface_);
	handle_.destroy();
}

void Instance::create_instance(const std::string &app_name)
{
	if (ENABLE_VALIDATION_LAYERS && !is_validation_layer_supported())
	{
		throw std::runtime_error("validation layers requested, but not avaliable!");
	};

	vk::ApplicationInfo app_info{
	    .pApplicationName   = app_name.c_str(),
	    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
	    .pEngineName        = "No Engine",
	    .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
	    .apiVersion         = VK_API_VERSION_1_2,
	};

	std::vector<const char *> extensions = get_required_extensions();
	std::vector<const char *> layers     = get_required_layers();

	vk::InstanceCreateInfo instance_cinfo{
	    .flags                   = IS_ON_OSX ? vk::InstanceCreateFlags{vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR} : vk::InstanceCreateFlags{},
	    .pApplicationInfo        = &app_info,
	    .enabledLayerCount       = to_u32(layers.size()),
	    .ppEnabledLayerNames     = layers.data(),
	    .enabledExtensionCount   = to_u32(extensions.size()),
	    .ppEnabledExtensionNames = extensions.data(),
	};

	vk::DebugUtilsMessengerCreateInfoEXT debug_cinfo;
	if (ENABLE_VALIDATION_LAYERS)
	{
		populate_debug_messenger_create_info(debug_cinfo);
		instance_cinfo.pNext = &debug_cinfo;
	}
	handle_ = vk::createInstance(instance_cinfo);
	load_function_ptrs();

	if (ENABLE_VALIDATION_LAYERS)
	{
		init_debug_messenger();
	}
}

std::vector<const char *> Instance::get_required_extensions()
{
	std::vector<const char *> extensions;

	if (ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (IS_ON_OSX)
	{
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	}

	Window::push_required_extensions(extensions);

	return extensions;
}

std::vector<const char *> Instance::get_required_layers()
{
	std::vector<const char *> layers;

	if (ENABLE_VALIDATION_LAYERS)
	{
		for (const char *layer_name : VALIDATION_LAYERS)
		{
			layers.push_back(layer_name);
		}
	}

	return layers;
}

bool Instance::is_validation_layer_supported()
{
	auto layers = vk::enumerateInstanceLayerProperties();

	for (const char *layer_name : VALIDATION_LAYERS)
	{
		bool layer_found = false;

		for (const auto &layerProperties : layers)
		{
			if (!strcmp(layer_name, layerProperties.layerName))
			{
				layer_found = true;
				break;
			}
		}

		if (!layer_found)
		{
			return false;
		}
	}

	return true;
}

void Instance::load_function_ptrs()
{
	create_debug_utils_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(handle_.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
	if (!create_debug_utils_messenger)
	{
		LOGE("Failed to laod vkCreateDebugUtilsMessengerEXT!");
		abort();
	}

	destroy_debug_utils_messenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(handle_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
	if (!destroy_debug_utils_messenger)
	{
		LOGE("Failed to load vkDestroyDebugUtilsMessengerEXT!");
		abort();
	}
}

void Instance::init_debug_messenger()
{
	vk::DebugUtilsMessengerCreateInfoEXT debug_cinfo;
	populate_debug_messenger_create_info(debug_cinfo);
	debug_messenger_ = handle_.createDebugUtilsMessengerEXT(debug_cinfo);
}

void Instance::populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT &createInfo)
{
	using SeverityFlagBits = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using MessageTypeBits  = vk::DebugUtilsMessageTypeFlagBitsEXT;
	createInfo.messageSeverity =
	    SeverityFlagBits::eInfo | SeverityFlagBits::eError | SeverityFlagBits::eVerbose;
	createInfo.messageType =
	    MessageTypeBits::eGeneral | MessageTypeBits::ePerformance | MessageTypeBits::eValidation;
	createInfo.pfnUserCallback = Instance::debug_callback;
}

inline VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data)
{
	if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LOGE("{} - {}: {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
	}
	else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LOGW("{} - {}: {}", p_callback_data->messageIdNumber, p_callback_data->pMessageIdName, p_callback_data->pMessage);
	}
	return VK_FALSE;
}

std::unique_ptr<PhysicalDevice> Instance::pick_physical_device()
{
	auto physical_device_handles = handle_.enumeratePhysicalDevices();
	if (!physical_device_handles.size())
	{
		LOGE("failed to find GPUs with Vulkan Support");
		abort();
	}

	for (auto physical_device_handle : physical_device_handles)
	{
		PhysicalDevice physical_device = PhysicalDevice(physical_device_handle, *this);
		if (is_physical_device_suitable(physical_device))
		{
			return std::make_unique<PhysicalDevice>(std::move(physical_device));
		}
	}

	throw std::runtime_error("failed to find a suitable GPU!");
}

bool Instance::is_physical_device_suitable(const PhysicalDevice &physical_device)
{
	const auto &indices                 = physical_device.get_queue_family_indices();
	bool        is_extensions_supported = physical_device.is_all_extensions_supported(Device::REQUIRED_EXTENSIONS);
	bool        is_swap_chain_supported = false;
	if (is_extensions_supported)
	{
		const auto &details     = physical_device.get_swapchain_support_details();
		is_swap_chain_supported = !details.formats.empty() && !details.present_modes.empty();
	}
	auto supportedFeatures = physical_device.get_handle().getFeatures();

	return indices.is_complete() && is_extensions_supported && is_swap_chain_supported &&
	       supportedFeatures.samplerAnisotropy;
}

const vk::SurfaceKHR &Instance::get_surface() const
{
	return surface_;
}

}        // namespace W3D

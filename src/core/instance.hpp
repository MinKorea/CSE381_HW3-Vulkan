#pragma once

#include "common/vk_common.hpp"
#include "vulkan_object.hpp"
#include <memory>

extern const std::vector<const char *> VALIDATION_LAYERS;
extern const std::vector<const char *> DEVICE_EXTENSIONS;

namespace W3D
{
class Window;
class PhysicalDevice;

/*
* Wrapper class for a Vulkan API Instance, which means an instance
* of the API itself and is used for creating things like devices.
*/
class Instance : public VulkanObject<vk::Instance>
{
  private:
	vk::SurfaceKHR             surface_         = nullptr;
	vk::DebugUtilsMessengerEXT debug_messenger_ = nullptr;

  public:
	static const std::vector<const char *> VALIDATION_LAYERS;

	/*
	* 
	*/
	Instance(const std::string &app_name, Window &window);

	/*
	* Destructor destroys both the surface and the instance.
	*/
	~Instance() override;

	// WE WILL NOT BE USING THESE
	Instance(const Instance &)            = delete;
	Instance &operator=(const Instance &) = delete;
	Instance(Instance &&)                 = delete;
	Instance &operator=(Instance &&)      = delete;

	/*
	* Accessor method for getting the surface our Vulkan app will render to.
	*/
	const vk::SurfaceKHR           &get_surface() const;

	/*
	* Accessor method for getting a vector of the required extensions for the Vulkan instance.
	*/
	std::vector<const char *> get_required_extensions();

	/*
	* Accessor method for getting a vector of the required layers for the Vulkan instance.
	*/
	std::vector<const char *> get_required_layers();

	/*
	* Accessor method to check and see if validation layer is supported. If it is we can
	* setup a validation messenger for use during development which will provide us with
	* robust debug messages about all things going on in Vulkan.
	*/
	bool is_validation_layer_supported();

	/*
	* Helper method to check and see if a found device is suitable while picking one.
	*/
	bool is_physical_device_suitable(const PhysicalDevice &device);

	/*
	* Function for creating the Vulkan API instance, which this object wraps. This
	* would be done at the time of construction of this object.
	*/
	void create_instance(const std::string &app_name);

	/*
	* Selects and returns a physical device (i.e. a GPU) we'll use for rendering.
	*/
	std::unique_ptr<PhysicalDevice> pick_physical_device();

	/*
	* Loads the function pointers needed for the debug messaging service.
	*/
	void load_function_ptrs();

	/*
	* Sets up how we're going to create the debug messenger.
	*/
	void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

	/*
	* Initializes the debug messenger so we can get feedback during development
	* concerning what is going on inside the Vulkan API while an app is running.
	*/
	void init_debug_messenger();

	/*
	* Callback function that will log Errors (E) and Warnings (W) regarding what
	* is going on while using the Vulkan API
	*/
	inline static VKAPI_ATTR VkBool32 VKAPI_CALL
	    debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	                   VkDebugUtilsMessageTypeFlagsEXT             messageType,
	                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

};	// class Instance

}	// namespace W3D
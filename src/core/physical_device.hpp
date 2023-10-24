#pragma once

#include "common/vk_common.hpp"
#include "vulkan_object.hpp"

#include <optional>

namespace W3D
{
class Instance;

/*
* Stores indices for our three queue families.
*/
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics_index;
	std::optional<uint32_t> present_index;
	std::optional<uint32_t> compute_index;

	bool is_complete() const
	{
		return graphics_index.has_value() && present_index.has_value() && compute_index.has_value();
	}
};

/*
* Stores details about the swap chain for a device.
*/
struct SwapchainSupportDetails
{
	vk::SurfaceCapabilitiesKHR        capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR>   present_modes;
};

/*
* A wrapper class for the Vulkan API's physical device (i.e. vk::PhysicalDevice), which
* means this object will have its handle.
*/
class PhysicalDevice : public VulkanObject<typename vk::PhysicalDevice>
{
  private:
	Instance                    &instance_;		// VULKAN API WRAPPER
	vk::PhysicalDeviceFeatures   features_;		// VULKAN PHYSICAL DEVICE FEATURES
	vk::PhysicalDeviceProperties properties_;	// VULKAN PHYSICAL DEVICE PROPERTIES
	QueueFamilyIndices           indices_;		// QUEUE FAMILY INDICES

  public:
	/*
	* This constructor will initialize this device with the Vulkan physical device and
	* API and will find queue family indices.
	*/
	PhysicalDevice(vk::PhysicalDevice handle, Instance &instance);

	/*
	* This constructor is like a clone, loading the properties of the argument.
	*/
	PhysicalDevice(PhysicalDevice &&);

	// WE WON'T BE USING THESE
	PhysicalDevice(const PhysicalDevice &)            = delete;
	PhysicalDevice &operator=(const PhysicalDevice &) = delete;
	PhysicalDevice &operator=(PhysicalDevice &&)      = delete;

	/*
	* The destructor has nothing to destroy.
	*/
	~PhysicalDevice() override = default;

	/*
	* Checks to see if everything in the required_extensions argument
	* is supported by this device.
	*/
	bool is_all_extensions_supported(const std::vector<const char *> &required_extensions) const;

	/*
	* Accessor method for getting the support details for this device's swapchain
	*/
	SwapchainSupportDetails   get_swapchain_support_details() const;

	/*
	* Accessor method for getting this device's queue family indices. Note, there are
	* three types of indices: graphics, computer, and present.
	*/
	const QueueFamilyIndices &get_queue_family_indices() const;

	/*
	* Accessor method for getting this device's graphics queue family index.
	*/
	uint32_t get_graphics_queue_family_index() const;

	/*
	 * Accessor method for getting this device's compute queue family index.
	 */
	uint32_t get_compute_queue_family_index() const;

	/*
	 * Accessor method for getting this device's present queue family index.
	 */
	uint32_t get_present_queue_family_index() const;

	void find_queue_familiy_indices();

};	// class PhysicalDevice

}	// namespace W3D
#pragma once

#include <memory>

#include "common/vk_common.hpp"
#include "device_memory/allocator.hpp"
#include "vulkan_object.hpp"

namespace W3D
{
class Instance;
class PhysicalDevice;
class DeviceMemoryAllocator;
class CommandPool;
class CommandBuffer;

/*
* A wrapper class for a Vulkan logical device. Note, the handle will
* be a vk::Device.
*/
class Device : public VulkanObject<typename vk::Device>
{
  private:
	Instance                              &instance_;
	PhysicalDevice                        &physical_device_;
	std::unique_ptr<DeviceMemoryAllocator> p_device_memory_allocator_;
	vk::Queue                              graphics_queue_ = nullptr;
	vk::Queue                              present_queue_  = nullptr;
	vk::Queue                              compute_queue_  = nullptr;
	std::unique_ptr<CommandPool>           p_one_time_buf_pool_;

  public:
	static const std::vector<const char *> REQUIRED_EXTENSIONS;

	/*
	* Constructor will fully initialize this object, creating the logical device and
	* through that device creating the memory allocator and the command queues.
	*/
	Device(Instance &instance, PhysicalDevice &physical_device);

	/*
	* Destructor cleans up.
	*/
	~Device() override;

	/*
	* Accessor method for getting the Vulkan instance associated with this device.
	*/
	const Instance              &get_instance() const;

	/*
	 * Accessor method for getting the Vulkan physical device associated with this device.
	 */
	const PhysicalDevice &get_physical_device() const;

	/*
	 * Accessor method for getting the graphics queue associated with this device.
	 */
	const vk::Queue &get_graphics_queue() const;

	/*
	 * Accessor method for getting the present queue associated with this device.
	 */
	const vk::Queue &get_present_queue() const;

	/*
	 * Accessor method for getting the compute queue associated with this device.
	 */
	const vk::Queue &get_compute_queue() const;

	/*
	 * Accessor method for getting the VMA wrapper (memory allocator) associated with this device.
	 */
	const DeviceMemoryAllocator &get_device_memory_allocator() const;

	/*
	* Function for activating the sending of commands to the device.
	*/
	CommandBuffer begin_one_time_buf() const;

	/*
	* Function for deactivating the sending of commands to the device.
	*/
	void          end_one_time_buf(CommandBuffer &cmd_buf) const;

};	// class Device

}	// namespace W3D
#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{

class Device;
class CommandBuffer;

enum class CommandPoolResetStrategy
{
	eIndividual,
	ePool
};

/*
 * This is a wrapper class for the Vulkan CommandPool type, providing additional functionality
 * for pools that generate commands. Note that it is a child class of VulkanObject, which has the
 * handle for the vk::CommandPool object. Note, we are specifying the type of the handle that
 * this object wraps (via the parent class, VulkanObject) is vk::CommandPool, which is a Vulkan
 * API class.
 */
class CommandPool : public VulkanObject<vk::CommandPool>
{
  private:
	Device                    &device_;	// LOGICAL DEVICE WRAPPER
	const vk::Queue           &queue_;	// VULKAN QUEUE FOR COMMANDS
	CommandPoolResetStrategy   strategy_;
	std::vector<CommandBuffer> primary_cmd_bufs_;	
	std::vector<CommandBuffer> secondary_cmd_bufs_;

  public:
	/*
	* Constructor for initializing this command pool.
	*/
	CommandPool(Device &device, const vk::Queue &queue, uint32_t queue_family_index, CommandPoolResetStrategy strategy = CommandPoolResetStrategy::eIndividual, vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	/*
	* Destructor
	*/
	~CommandPool() override;

	/*
	* Accessor method for getting the logical device associated with this command pool.
	*/
	const Device            &get_device();

	/*
	* Accessor method for getting the Vulkan queue associated with this command pool.
	*/
	const vk::Queue         &get_queue();

	/*
	* Accessor method for getting the strategy to be used for resetting this command pool.
	*/
	CommandPoolResetStrategy get_reset_strategy();

	/*
	* Wrapper function for vk::CommandPool's resetCommandPool function.
	*/
	void                     reset();

	/*
	* Allocate and return a single command buffer.
	*/
	CommandBuffer              allocate_command_buffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	/*
	* Allocate count command buffers of the type asked for and return them in a vector. Note, this
	* makes use of vk::CommandBuffer's allocateCommandBuffer function to do so.
	*/
	std::vector<CommandBuffer> allocate_command_buffers(uint32_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	/*
	* For deallocating a single command buffer by using the vk:CommandBuffer's freeCommandBuffers function.
	*/
	void                       free_command_buffer(CommandBuffer &cmd_buf);

	/*
	 * For deallocating multiple command buffers by using the vk:CommandBuffer's freeCommandBuffers function.
	 */
	void free_command_buffers(std::vector<CommandBuffer> &cmd_bufs);

	/*
	* Returns the cmd_buf argument to either the primary vector of command buffers or
	* the secondary vector, depending on where it came from.
	*/
	void                       recycle_command_buffer(CommandBuffer &cmd_buf);

};	// class CommandPool

}	// namespace W3D
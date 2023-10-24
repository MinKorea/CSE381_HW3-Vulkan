#pragma once
#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Buffer;
class Image;
class ImageResource;
class CommandPool;

/*
* This is a wrapper class for the Vulkan CommandBuffer type, providing additional functionality
* for managing device commands. Note that it is a child class of VulkanObject, which has the
* handle for the vk::CommandBuffer object. Note, we are specifying the type of the handle that
* this object wraps (via the parent class, VulkanObject) is vk::CommandBuffer, which is a Vulkan
* API class.
*/
class CommandBuffer : public VulkanObject<vk::CommandBuffer>
{
  friend CommandPool;

  private:
	CommandPool            &pool_; // THE POOL THAT WE'LL GET COMMANDS FROM
	vk::CommandBufferLevel level_; // THE LEVEL OF THIS BUFFER

  public:
	/*
	* This constructor sends the handle to the inherited constructor in initializes the
	* pool and level.
	*/
	CommandBuffer(vk::CommandBuffer handle, CommandPool &pool, vk::CommandBufferLevel level);

	/*
	* This constructor uses the handle, pool, and level of the CommandBuffer argument to
	* initialize this object.
	*/
	CommandBuffer(CommandBuffer &&);

	/*
	* The destructor returns this command to the command pool.
	*/
	~CommandBuffer() override;

	// THESE ARE DEACTIVATED
	CommandBuffer(const CommandBuffer &)            = delete;
	CommandBuffer &operator=(const CommandBuffer &) = delete;
	CommandBuffer &operator=(CommandBuffer &&)      = delete;

	/*
	* A wrapper function for the vk::CommandBuffer begin function
	*/
	void begin(vk::CommandBufferUsageFlags flag = {});

	/*
	* A function for submitting all queued commands to the command pool
	*/
	void flush(vk::SubmitInfo submit_info);

	/*
	* A wrapper function for the vk::CommandBuffer reset function
	*/
	void reset();

	/*
	* This function specifies how the pipeline will make use of this image.
	*/
	void set_image_layout(ImageResource &resource, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags src_stage_mask = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dst_stage_mask = vk::PipelineStageFlagBits::eAllCommands);

	/*
	 * This function updates an image with data. It serves as a wrapper function for 
	 * vk::CommandBuffer's copyBufferToImage function.
	 */
	void update_image(ImageResource &resouce, Buffer &staging_buf);

	/*
	 * This function performs a deep copy of image data and returns this data as a
	 * vector of vk::BufferImageCopy.
	 */
	std::vector<vk::BufferImageCopy> full_copy_regions(const vk::ImageSubresourceRange &subresource_range, vk::Extent3D base_extent, uint8_t bits_per_pixel);

	/*
	 * A wrapper function for the vk::CommandBuffer copyBuffer function
	 */
	void copy_buffer(Buffer &src, Buffer &dst, size_t size);

	/*
	 * A wrapper function for the vk::CommandBuffer copyBuffer function
	 */
	void copy_buffer(Buffer &src, Buffer &dst, vk::BufferCopy copy_region = {});

}; // class CommandBuffer

}  // namespace W3D
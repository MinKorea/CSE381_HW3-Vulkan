// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "command_buffer.hpp"

// OUR OWN TYPES
#include "core/command_pool.hpp"
#include "core/device.hpp"
#include "core/image_resource.hpp"
#include "core/image_view.hpp"
#include "device_memory/buffer.hpp"

namespace W3D
{

CommandBuffer::CommandBuffer(vk::CommandBuffer handle, CommandPool &pool, vk::CommandBufferLevel level) :
    VulkanObject(handle),	// INITIALIZE THE handle VIA THE PARENT CONSTRUCTOR (in VulkanObject)
    pool_(pool),			// SET pool_ USING THE pool ARGUMENT
    level_(level)			// SET level_ USING THE level ARGUMENT
{
}

CommandBuffer::CommandBuffer(CommandBuffer &&rhs) :
    VulkanObject(std::move(rhs)),	// INITIALIZE THE handle VIA THE PARENT CONSTRUCTOR (in VulkanObject)
    pool_(rhs.pool_),				// SET pool_ USING THE pool ARGUMENT
    level_(rhs.level_)				// SET level_ USING THE level ARGUMENT
{
}

CommandBuffer::~CommandBuffer()
{
	if (handle_ && pool_.get_handle())
	{
		reset();
		pool_.recycle_command_buffer(*this);
	}
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags flag)
{
	vk::CommandBufferBeginInfo cmd_buf_binfo{
	    .flags = flag,
	};
	handle_.begin(cmd_buf_binfo);
}

void CommandBuffer::flush(vk::SubmitInfo submit_info)
{
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &handle_;
	pool_.get_queue().submit(submit_info);
}

void CommandBuffer::reset()
{
	if (pool_.get_reset_strategy() == CommandPoolResetStrategy::eIndividual)
	{
		handle_.reset();
	}
}

void CommandBuffer::set_image_layout(ImageResource &resource, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags src_stage_mask, vk::PipelineStageFlags dst_stage_mask)
{
	vk::ImageMemoryBarrier barrier{
	    .oldLayout           = old_layout,
	    .newLayout           = new_layout,
	    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	    .image               = resource.get_image().get_handle(),
	    .subresourceRange    = resource.get_view().get_subresource_range(),
	};
	switch (old_layout)
	{
		case vk::ImageLayout::eUndefined:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			barrier.srcAccessMask = {};
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			break;

		case vk::ImageLayout::eTransferSrcOptimal:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eTransferDstOptimal:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			break;

		case vk::ImageLayout::eShaderReadOnlyOptimal:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (new_layout)
	{
		case vk::ImageLayout::eTransferDstOptimal:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			break;

		case vk::ImageLayout::eTransferSrcOptimal:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			break;

		case vk::ImageLayout::eShaderReadOnlyOptimal:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (barrier.srcAccessMask == vk::AccessFlags{})
			{
				barrier.srcAccessMask =
				    vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
			}
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
	}

	handle_.pipelineBarrier(src_stage_mask,
	                        dst_stage_mask,
	                        {},
	                        {},
	                        {},
	                        barrier);
}

void CommandBuffer::update_image(ImageResource &resource, Buffer &staging_buf)
{
	auto                            &subresource_range = resource.get_view().get_subresource_range();

	// GET THE DATA USING OUR HELPER METHOD
	std::vector<vk::BufferImageCopy> copy_regions      = full_copy_regions(resource.get_view().get_subresource_range(), resource.get_image().get_base_extent(), ImageResource::format_to_bytes_per_pixel(resource.get_image().get_format()));

	// USE THE vk::CommandBuffer's copyBufferToImage TO LOAD THE IMAGE
	handle_.copyBufferToImage(staging_buf.get_handle(), resource.get_image().get_handle(), vk::ImageLayout::eTransferDstOptimal, copy_regions);
}

std::vector<vk::BufferImageCopy> CommandBuffer::full_copy_regions(const vk::ImageSubresourceRange &subresource_range, vk::Extent3D base_extent, uint8_t bits_per_pixel)
{
	// WE'LL PUT THE DEEP COPY DATA HERE
	std::vector<vk::BufferImageCopy> buffer_copy_regions;

	uint32_t offset = 0;

	// COPY ALL THE LAYERS
	for (size_t l = 0; l < subresource_range.layerCount; l++)
	{
		for (size_t m = 0; m < subresource_range.levelCount; m++)
		{
			// ADD DATA FOR THE NEXT LAYER
			buffer_copy_regions.emplace_back(vk::BufferImageCopy{
			    .bufferOffset     = offset,
			    .imageSubresource = {
			        .aspectMask     = subresource_range.aspectMask,
			        .mipLevel       = to_u32(m),
			        .baseArrayLayer = to_u32(l),
			        .layerCount     = 1,
			    },
			    .imageExtent = {
			        .width  = base_extent.width >> m,
			        .height = base_extent.height >> m,
			        .depth  = 1,
			    },
			});

			offset += buffer_copy_regions.back().imageExtent.width * buffer_copy_regions.back().imageExtent.height * bits_per_pixel;
		}
	}

	// RETURN THE COPIED DATA
	return buffer_copy_regions;
}

void CommandBuffer::copy_buffer(Buffer &src, Buffer &dst, size_t size)
{
	handle_.copyBuffer(src.get_handle(), dst.get_handle(), vk::BufferCopy{0, 0, size});
}

void CommandBuffer::copy_buffer(Buffer &src, Buffer &dst, vk::BufferCopy copy_region)
{
	handle_.copyBuffer(src.get_handle(), dst.get_handle(), copy_region);
}

}        // namespace W3D
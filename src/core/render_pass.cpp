// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "render_pass.hpp"

// OUR OWN TYPES
#include "common/utils.hpp"
#include "device.hpp"
#include "swapchain.hpp"

namespace W3D
{

vk::AttachmentDescription RenderPass::color_attachment(vk::Format format, vk::ImageLayout initial_layout, vk::ImageLayout final_layout)
{
	vk::AttachmentDescription color_attachment{
	    .format         = format,
	    .samples        = vk::SampleCountFlagBits::e1,
	    .loadOp         = vk::AttachmentLoadOp::eClear,
	    .storeOp        = vk::AttachmentStoreOp::eStore,
	    .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
	    .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
	    .initialLayout  = initial_layout,
	    .finalLayout    = final_layout,
	};
	return color_attachment;
}

vk::AttachmentDescription RenderPass::depth_attachment(vk::Format format, vk::ImageLayout initial_layout, vk::ImageLayout final_layout)
{
	vk::AttachmentDescription depth_attachment{
	    .format         = format,
	    .samples        = vk::SampleCountFlagBits::e1,
	    .loadOp         = vk::AttachmentLoadOp::eClear,
	    .storeOp        = vk::AttachmentStoreOp::eDontCare,
	    .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
	    .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
	    .initialLayout  = initial_layout,
	    .finalLayout    = final_layout,
	};
	return depth_attachment;
};

RenderPass::RenderPass(Device &device, std::nullptr_t nptr) :
    device_(device)
{
}

RenderPass::RenderPass(Device &device, vk::RenderPassCreateInfo render_pass_cinfo) :
    device_(device)
{
	handle_ = device_.get_handle().createRenderPass(render_pass_cinfo);
}

RenderPass::~RenderPass()
{
	if (handle_)
	{
		device_.get_handle().destroyRenderPass(handle_);
	}
}

}	// namespace W3D
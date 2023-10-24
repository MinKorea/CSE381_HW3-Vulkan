// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "framebuffer.hpp"

// OUR OWN TYPES
#include "device.hpp"
#include "device_memory/image.hpp"
#include "image_resource.hpp"
#include "image_view.hpp"
#include "render_pass.hpp"
#include "swapchain.hpp"

namespace W3D
{

Framebuffer::Framebuffer(Device &device, vk::FramebufferCreateInfo framebuffer_cinfo) :
    device_(device)
{
	handle_ = device_.get_handle().createFramebuffer(framebuffer_cinfo);
}

Framebuffer::~Framebuffer()
{
	if (handle_)
	{
		device_.get_handle().destroyFramebuffer(handle_);
	}
}

SwapchainFramebuffer::SwapchainFramebuffer(Device &device, Swapchain &swapchain, RenderPass &render_pass) :
    device_(device),
    swapchain_(swapchain),
    render_pass_(render_pass)
{
	build();
}

SwapchainFramebuffer::~SwapchainFramebuffer()
{
	cleanup();
}

void SwapchainFramebuffer::build()
{
	const std::vector<ImageView> &frame_image_views = swapchain_.get_frame_image_views();
	const ImageView              &depth_image_view  = swapchain_.get_depth_resource().get_view();
	vk::Extent2D                  extent            = swapchain_.get_swapchain_properties().extent;

	std::array<vk::ImageView, 2> attachments;
	vk::FramebufferCreateInfo    framebuffer_cinfo{
	       .renderPass      = render_pass_.get_handle(),
	       .attachmentCount = to_u32(attachments.size()),
	       .pAttachments    = attachments.data(),
	       .width           = extent.width,
	       .height          = extent.height,
	       .layers          = 1,
    };
	for (auto const &frame_image_view : frame_image_views)
	{
		attachments[0] = frame_image_view.get_handle();
		attachments[1] = depth_image_view.get_handle();
		framebuffers_.push_back(device_.get_handle().createFramebuffer(framebuffer_cinfo));
	}
}

void SwapchainFramebuffer::cleanup()
{
	for (vk::Framebuffer framebuffer : framebuffers_)
	{
		device_.get_handle().destroyFramebuffer(framebuffer);
	}
	framebuffers_.clear();
}

void SwapchainFramebuffer::rebuild()
{
	cleanup();
	build();
}

const std::vector<vk::Framebuffer> &SwapchainFramebuffer::get_handles() const
{
	return framebuffers_;
}

const vk::Framebuffer &SwapchainFramebuffer::get_handle(uint32_t idx) const
{
	return framebuffers_[idx];
}

}	// namespace W3D
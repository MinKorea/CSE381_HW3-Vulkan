#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;
class Swapchain;

/*
* A wrapper class for a Vulkan API render pass (i.e. vk::RenderPass), which describes the 
* resources to be used for rendering on a pipeline as well as the general steps.
* 
*/
class RenderPass : public VulkanObject<vk::RenderPass>
{
  private:
	Device &device_;	// LOGICAL DEVICE

  public:
	/*
	* This static function creates a color attachment for a render pass.
	*/
	static vk::AttachmentDescription color_attachment(vk::Format format, vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined, vk::ImageLayout final_layout = vk::ImageLayout::eColorAttachmentOptimal);

	/*
	* This static function creates a depth attachment for a render pass.
	*/
	static vk::AttachmentDescription depth_attachment(vk::Format format, vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined, vk::ImageLayout final_layout = vk::ImageLayout::eDepthAttachmentOptimal);

	/*
	* Constructor sets the device but does not initialize the Vulkan render pass.
	*/
	RenderPass(Device &device, std::nullptr_t nptr);

	/*
	* Constructor sets the device and also creates the Vulkan render pass object, keeping
	* the handle.
	*/
	RenderPass(Device &device, vk::RenderPassCreateInfo render_pass_cinfo);

	/*
	* This destructor will destroy the Vulkan render pass.
	*/
	~RenderPass() override;

};	// class RenderPass

}	// namespace W3D
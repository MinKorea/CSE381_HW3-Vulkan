#pragma once
#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;
class Swapchain;
class RenderPass;

/*
* This class serves as a wrapper class for a Vulkan frame buffer, so it uses
* a vk::Framebuffer object as its handle.
*/
class Framebuffer : public VulkanObject<vk::Framebuffer>
{
  private:
	Device &device_;	// THE LOGICAL DEVICE ASSOCIATED WITH THIS FRAMEBUFFER

  public:
	/*
	* Constructor initializes the object for use, including creating the
	* Vulkan frame buffer object, which is assigned to the handle.
	*/
	Framebuffer(Device &device, vk::FramebufferCreateInfo framebuffer_cinfo);

	/*
	* This destructor will destroy the Vulkan framebuffer.
	*/
	~Framebuffer() override;

};	// class Framebuffer

/*
* This class combines a collection of Vulkan objects needed for a swapchain 
* framebuffer
*/
class SwapchainFramebuffer
{
  private:
	Device                      &device_;
	Swapchain                   &swapchain_;
	RenderPass                  &render_pass_;
	std::vector<vk::Framebuffer> framebuffers_;

  public:
	/*
	* This constructor will build a Vulkan framebuffer.
	*/
	SwapchainFramebuffer(Device &device, Swapchain &swapchain, RenderPass &render_pass);

	/*
	* This constructor will destroy all existing framebuffers.
	*/
	~SwapchainFramebuffer();

	/*
	* This helper creates a new frame buffer and adds it to the vector of frame buffers.
	*/
	void build();

	/*
	* This helper destroys the existing frame buffers.
	*/
	void cleanup();

	/*
	* This helper destroys the existing frame buffers and then makes a new one.
	*/
	void rebuild();

	/*
	* This accessor method gets all the framebuffer handles stored in the vector,
	* in fact it just gets and returns the vector.
	*/
	const std::vector<vk::Framebuffer> &get_handles() const;

	/*
	* This accessor method gets the Vulkan framebuffer associated with idx, which
	* is the id of the framebuffer.
	*/
	const vk::Framebuffer              &get_handle(uint32_t idx) const;

};	// class SwapchainFramebuffer

}	// namespace W3D
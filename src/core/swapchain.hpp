#pragma once

#include "common/vk_common.hpp"
#include "vulkan_object.hpp"
#include <memory>

namespace W3D
{
class Device;
class Instance;
class ImageView;
class ImageResource;

struct SwapchainProperties
{
	vk::SurfaceFormatKHR surface_format;
	vk::PresentModeKHR   present_mode;
	vk::Extent2D         extent;
};

class Swapchain : public VulkanObject<vk::SwapchainKHR>
{
  public:
	Swapchain(Device &device, vk::Extent2D window_extent);
	~Swapchain() override;

	void       cleanup();
	void       rebuild(vk::Extent2D new_window_extent);
	void       build(vk::Extent2D window_extent);
	vk::Format choose_depth_format();

	const SwapchainProperties    &get_swapchain_properties() const;
	const std::vector<ImageView> &get_frame_image_views() const;
	const ImageResource          &get_depth_resource() const;

  private:
	void     choose_features();
	void     choose_format(const std::vector<vk::SurfaceFormatKHR> &formats);
	void     choose_present_mode(const std::vector<vk::PresentModeKHR> &present_modes);
	void     choose_extent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D window_extent);
	uint32_t calc_min_image_count(uint32_t min_image_count, uint32_t max_image_count);

	void create_frame_resources();

	Device                        &device_;
	SwapchainProperties            properties_;
	std::vector<vk::Image>         frame_images_;        // Special images owned by vulkan
	std::vector<ImageView>         frame_image_views_;
	std::unique_ptr<ImageResource> p_depth_resource_;
};
}        // namespace W3D
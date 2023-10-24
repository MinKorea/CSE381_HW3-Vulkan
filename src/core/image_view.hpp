#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;

/*
* This class serves as a wrapper for a Vulkan API ImageView type, which is the handle's type.
*/
class ImageView : public VulkanObject<vk::ImageView>
{
  private:
	const Device             &device_;
	vk::ImageSubresourceRange subresource_range_;

  public:
	/*
	* This static function creates and returns a Vulkan API ImageViewCreateInfo object containing 
	* the settings necessary for representing a two dimensional view Vulkan image.
	*/
	static vk::ImageViewCreateInfo two_dim_view_cinfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspct_flags, uint32_t mip_levels);

	/*
	 * This static function creates and returns a Vulkan API ImageViewCreateInfo object containing
	 * the settings necessary for representing a cube view Vulkan image.
	 */
	static vk::ImageViewCreateInfo cube_view_cinfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspct_flags, uint32_t mip_levels);

	/*
	* This constructor keeps the device but does not initialize the subresource range.
	*/
	ImageView(const Device &device, std::nullptr_t nptr);

	/*
	 * This constructor fully initializes this object using the arguments.
	 */
	ImageView(const Device &device, vk::ImageViewCreateInfo &image_view_cinfo);

	/*
	 * This constructor fully initializes this object using the values from the rhs argument
	 */
	ImageView(ImageView &&rhs);

	/*
	 * This overrides assignments done with ImageView objects.
	 */
	ImageView &operator=(ImageView &&rhs);

	/*
	* This destructor will destroy the Vulkan API ImageView that this object wraps.
	*/
	~ImageView() override;

	/*
	* Accessor method for getting the Vulkan API ImageSubresourceRage associated
	* with this image.
	*/
	const vk::ImageSubresourceRange &get_subresource_range() const;

};	// class ImageView

}	// namespace W3D
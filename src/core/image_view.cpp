// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "image_view.hpp"

// OUR OWN TYPES
#include "device.hpp"

namespace W3D
{

vk::ImageViewCreateInfo ImageView::two_dim_view_cinfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspct_flags, uint32_t mip_levels)
{
	// MAKE AND RETURN A VULKAN API OBJECT
	vk::ImageViewCreateInfo view_cinfo{
	    .image            = image,
	    .viewType         = vk::ImageViewType::e2D,
	    .format           = format,
	    .subresourceRange = {
	        .aspectMask     = aspct_flags,
	        .baseMipLevel   = 0,
	        .levelCount     = mip_levels,
	        .baseArrayLayer = 0,
	        .layerCount     = 1,
	    },
	};
	return view_cinfo;
}

vk::ImageViewCreateInfo ImageView::cube_view_cinfo(vk::Image image, vk::Format format, vk::ImageAspectFlags aspct_flags, uint32_t mip_levels)
{
	// MAKE AND RETURN A VULKAN API OBJECT
	vk::ImageViewCreateInfo view_cinfo{
	    .image            = image,
	    .viewType         = vk::ImageViewType::eCube,
	    .format           = format,
	    .subresourceRange = {
	        .aspectMask     = aspct_flags,
	        .baseMipLevel   = 0,
	        .levelCount     = mip_levels,
	        .baseArrayLayer = 0,
	        .layerCount     = 6,
	    },
	};
	return view_cinfo;
}

ImageView::ImageView(const Device &device, std::nullptr_t nptr) :
    device_(device)
{
}

ImageView::ImageView(const Device &device, vk::ImageViewCreateInfo &image_view_cinfo) :
    device_(device),
    subresource_range_(image_view_cinfo.subresourceRange)
{
	handle_ = device_.get_handle().createImageView(image_view_cinfo);
}

ImageView::ImageView(ImageView &&rhs) :
    VulkanObject(std::move(rhs)),
    device_(rhs.device_),
    subresource_range_(rhs.subresource_range_)
{}

ImageView &ImageView::operator=(ImageView &&rhs)
{
	if (handle_)
	{
		device_.get_handle().destroyImageView(handle_);
	}
	subresource_range_ = rhs.subresource_range_;
	handle_            = rhs.handle_;
	rhs.handle_        = nullptr;
	return *this;
}

ImageView::~ImageView()
{
	if (handle_)
	{
		device_.get_handle().destroyImageView(handle_);
	}
}

const vk::ImageSubresourceRange &ImageView::get_subresource_range() const
{
	return subresource_range_;
}

}        // namespace W3D

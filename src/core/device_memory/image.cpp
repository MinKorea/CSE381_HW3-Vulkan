// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "image.hpp"

// OUR OWN TYPES
#include "common/error.hpp"
#include "core/image_view.hpp"

namespace W3D
{
// NOT DECLARED IN HEADER FILE
Image::Image(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, std::nullptr_t nptr) :
    DeviceMemoryObject(allocator, key)
{
	handle_ = nullptr;
}

Image::Image(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, vk::ImageCreateInfo &image_cinfo, VmaAllocationCreateInfo &allocation_cinfo) :
    DeviceMemoryObject(allocator, key),
    base_extent_(image_cinfo.extent),
    format_(image_cinfo.format)
{
	details_.allocator = allocator;
	VkImage c_image_handle;
	VK_CHECK(vmaCreateImage(details_.allocator, reinterpret_cast<VkImageCreateInfo *>(&image_cinfo), &allocation_cinfo, &c_image_handle, &details_.allocation, &details_.allocation_info));
	handle_ = c_image_handle;
}

Image::Image(Image &&rhs) :
    DeviceMemoryObject(std::move(rhs)),
    base_extent_(rhs.base_extent_),
    format_(rhs.format_)
{
}

Image &Image::operator=(Image &&rhs)
{
	if (handle_)
	{
		vmaDestroyImage(details_.allocator, handle_, details_.allocation);
	}

	handle_      = rhs.handle_;
	base_extent_ = rhs.base_extent_;
	format_      = rhs.format_;
	details_     = rhs.details_;

	rhs.handle_             = nullptr;
	rhs.details_.allocation = nullptr;
	return *this;
}

Image::~Image()
{
	if (handle_)
	{
		vmaDestroyImage(details_.allocator, handle_, details_.allocation);
	}
}

vk::Extent3D Image::get_base_extent()
{
	return base_extent_;
}

vk::Format Image::get_format()
{
	return format_;
}

}	// namespace W3D
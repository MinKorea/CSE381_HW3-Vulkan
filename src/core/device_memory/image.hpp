#pragma once

#include "common/vk_common.hpp"
#include "core/device_memory/allocator.hpp"
#include "core/device_memory/device_memory_object.hpp"

namespace W3D
{

template <typename T>
class Key;

/*
* This class represents an image that can be loaded onto the device. Note, it extends
* DeviceMemoryObject such that it wraps a Vulkan image, i.e. vk::Image, meaning the
* handle associated with this object will be a Vulkan API image.
*/
class Image : public DeviceMemoryObject<vk::Image>
{
  private:
	vk::Extent3D base_extent_;
	vk::Format   format_;

  public:
	/*
	* This constructor initializes the image using another image.
	*/
	Image(Image &&rhs);
	Image &operator=(Image &&);

	/*
	* There is nothing to destroy.
	*/
	~Image();

	/*
	* Implementation missing.
	*/
	Image(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, std::nullptr_t nptr);

	/*
	* Implementation missing.
	*/
	Image(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, vk::ImageCreateInfo &image_cinfo, VmaAllocationCreateInfo &allocation_cinfo);

	/*
	* Accessor method for getting the extents of this image. Note, it returns
	* a Vulkan API Extent3D object.
	*/
	vk::Extent3D get_base_extent();

	/*
	* Accessor method for getting the format of this image. Note, it returns
	* a Vulkan API Format object.
	*/
	vk::Format   get_format();
};
}        // namespace W3D
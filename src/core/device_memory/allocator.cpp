// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "allocator.hpp"

// OUR OWN TYPES
#include "common/utils.hpp"
#include "core/device.hpp"
#include "core/instance.hpp"
#include "core/physical_device.hpp"

#include "buffer.hpp"
#include "image.hpp"

namespace W3D
{

DeviceMemoryAllocator::DeviceMemoryAllocator(Device &device)
{
	const Instance        &instance        = device.get_instance();
	const PhysicalDevice  &physical_device = device.get_physical_device();
	VmaAllocatorCreateInfo allocator_cinfo{
	    .physicalDevice   = physical_device.get_handle(),
	    .device           = device.get_handle(),
	    .instance         = instance.get_handle(),
	    .vulkanApiVersion = VK_API_VERSION_1_2,
	};

	// THIS IS WHERE WE CREATE THE ALLOCATOR FOR USE IN THE OTHER METHODS
	vmaCreateAllocator(&allocator_cinfo, &handle_);
}

DeviceMemoryAllocator::~DeviceMemoryAllocator()
{
	vmaDestroyAllocator(handle_);
}

Buffer DeviceMemoryAllocator::allocate_buffer(vk::BufferCreateInfo &buffer_cinfo, VmaAllocationCreateInfo &allocation_cinfo) const
{
	return Buffer(Key<DeviceMemoryAllocator>{}, handle_, buffer_cinfo, allocation_cinfo);
}

Buffer DeviceMemoryAllocator::allocate_staging_buffer(size_t size) const
{
	vk::BufferCreateInfo buffer_cinfo{};
	buffer_cinfo.size  = size;
	buffer_cinfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	VmaAllocationCreateInfo allocation_cinfo{};
	allocation_cinfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	allocation_cinfo.usage = VMA_MEMORY_USAGE_AUTO;
	return allocate_buffer(buffer_cinfo, allocation_cinfo);
}

Buffer DeviceMemoryAllocator::allocate_vertex_buffer(size_t size) const
{
	vk::BufferCreateInfo buffer_cinfo{};
	buffer_cinfo.size  = size;
	buffer_cinfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	VmaAllocationCreateInfo allocation_cinfo{};
	allocation_cinfo.flags = 0;
	allocation_cinfo.usage = VMA_MEMORY_USAGE_AUTO;
	return allocate_buffer(buffer_cinfo, allocation_cinfo);
}

Buffer DeviceMemoryAllocator::allocate_index_buffer(size_t size) const
{
	vk::BufferCreateInfo buffer_cinfo{};
	buffer_cinfo.size  = size;
	buffer_cinfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	VmaAllocationCreateInfo allocation_cinfo{};
	allocation_cinfo.flags = 0;
	allocation_cinfo.usage = VMA_MEMORY_USAGE_AUTO;
	return allocate_buffer(buffer_cinfo, allocation_cinfo);
}


Buffer DeviceMemoryAllocator::allocate_uniform_buffer(size_t size) const
{
	vk::BufferCreateInfo buffer_cinfo{};
	buffer_cinfo.size  = size;
	buffer_cinfo.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
	VmaAllocationCreateInfo allocation_cinfo{};
	allocation_cinfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	allocation_cinfo.usage = VMA_MEMORY_USAGE_AUTO;
	return allocate_buffer(buffer_cinfo, allocation_cinfo);
}


Buffer DeviceMemoryAllocator::allocate_null_buffer() const
{
	return Buffer(Key<DeviceMemoryAllocator>{}, handle_, nullptr);
}

Image DeviceMemoryAllocator::allocate_device_only_image(vk::ImageCreateInfo &image_cinfo) const
{
	VmaAllocationCreateInfo allocation_cinfo{};
	allocation_cinfo.flags    = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocation_cinfo.usage    = VMA_MEMORY_USAGE_AUTO;
	allocation_cinfo.priority = 1.0f;
	return allocate_image(image_cinfo, allocation_cinfo);
}

Image DeviceMemoryAllocator::allocate_image(vk::ImageCreateInfo &image_cinfo, VmaAllocationCreateInfo &allocation_cinfo) const
{
	return Image(Key<DeviceMemoryAllocator>{}, handle_, image_cinfo, allocation_cinfo);
}

Image DeviceMemoryAllocator::allocate_null_image() const
{
	return Image(Key<DeviceMemoryAllocator>{}, handle_, nullptr);
};

}        // namespace W3D
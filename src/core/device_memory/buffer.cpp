// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "buffer.hpp"

// OUR OWN TYPES
#include "common/error.hpp"
#include "common/utils.hpp"

namespace W3D
{

Buffer::Buffer(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, std::nullptr_t nptr) :
    DeviceMemoryObject(allocator, key)
{
	handle_ = nullptr;
}

Buffer::Buffer(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, vk::BufferCreateInfo &buffer_cinfo, VmaAllocationCreateInfo &allocation_cinfo) :
    DeviceMemoryObject(allocator, key)
{
	details_.allocator = allocator;
	is_persistent_     = allocation_cinfo.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT;
	VkBuffer c_buf_handle;
	VK_CHECK(vmaCreateBuffer(details_.allocator, reinterpret_cast<VkBufferCreateInfo *>(&buffer_cinfo), &allocation_cinfo, &c_buf_handle, &details_.allocation, &details_.allocation_info));
	handle_ = c_buf_handle;
	update_flags();
}

Buffer::Buffer(Buffer &&rhs) :
    DeviceMemoryObject(std::move(rhs)),
    is_persistent_(rhs.is_persistent_),
    p_mapped_data_(rhs.p_mapped_data_)
{
	rhs.p_mapped_data_ = nullptr;
}

Buffer::~Buffer()
{
	if (handle_)
	{
		vmaDestroyBuffer(details_.allocator, handle_, details_.allocation);
	}
};

void Buffer::update(void *p_data, size_t size, size_t offset)
{
	// UPDATE THE BUFFER WITH p_data
	update(to_ubyte_ptr(p_data), size, offset);
}

void Buffer::update(const std::vector<uint8_t> &binary, size_t offset)
{
	// UPDATE THE BUFFER WITH binary
	update(binary.data(), binary.size(), offset);
}

void Buffer::update(const uint8_t *p_data, size_t size, size_t offset)
{
	// IF IT'S PERSISTENT WE DON'T HAVE TO REMAP, WE CAN USE AN EXISTING MAP
	if (is_persistent_)
	{
		// COPY THE CONTENTS OF p_data OVER TO OUR BUFFER AS UNSIGNED BYTES
		std::copy(p_data, p_data + size, to_ubyte_ptr(details_.allocation_info.pMappedData));
	}
	else
	{
		// REMAP THE BUFFER SO WE HAVE A POINTER
		map();

		// THEN COPY DATA OVER TO THE BUFFER
		std::copy(p_data, p_data + size, to_ubyte_ptr(p_mapped_data_));

		// FLUSH THE BUFFER'S CACHE
		flush();

		// AND FOR NOW WE'RE DONE WITH THE POINTER TO THIS BUFFER, NOTE WE HAVE TO DO THIS
		// BECAUSE IT IS NOT A PERSISTENT BUFFER
		unmap();
	}
}

void Buffer::map()
{
	// ONLY IF DATA CAN BE MAPPED
	assert(is_mappable());
	if (p_mapped_data_)
	{
		// USE THE VMA API TO MAP THE BUFFER, i.e. GET A POINTER
		vmaMapMemory(details_.allocator, details_.allocation, &p_mapped_data_);
	}
}

void Buffer::unmap()
{
	// ONLY IF DATA CAN BE MAPPED
	if (p_mapped_data_)
	{
		// USE THE VMA API TO UNMAP THE BUFFER, i.e. DESTROY THE POINTER
		vmaUnmapMemory(details_.allocator, details_.allocation);
		p_mapped_data_ = nullptr;
	}
}

void Buffer::flush()
{
	// ONLY IF DATA CAN BE MAPPED
	if (p_mapped_data_)
	{
		// USE THE VMA API TO FLUSH THE CACHED DATA
		vmaFlushAllocation(details_.allocator, details_.allocation, 0, details_.allocation_info.size);
	}
}

} // namespace W3D
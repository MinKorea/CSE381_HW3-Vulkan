#pragma once
#include "common/vk_common.hpp"
#include "core/device_memory/device_memory_object.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
// WE'LL BE USING Key FOR MAPPING
template <typename T>
class Key;

/*
* This is a device memory object that will store data, like for
* vertex buffers and index buffers.
*/
class Buffer : public DeviceMemoryObject<vk::Buffer>
{
  private:
	bool  is_persistent_;
	void *p_mapped_data_ = nullptr;

  public:
	/*
	 * This constructor initializes a default Buffer without a handle and without allocating
	 * any memory for the buffer.
	 */
	Buffer(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, std::nullptr_t nptr);

	/*
	 * This constructor will use the VMA API to allocate memory for this buffer according
	 * to the needs specified in the arguments. Note, this does not load data into the buffer, that
	 * must be done via use of the update methods.
	 */
	Buffer(Key<DeviceMemoryAllocator> key, VmaAllocator allocator, vk::BufferCreateInfo &buffer_cinfo, VmaAllocationCreateInfo &allocation_cinfo);

	/*
	 * This constructor creates a buffer using another Buffer.
	 */
	Buffer(Buffer &&);

	// NOTE, ASSIGNING delete TO THE FOLLOWING CONSTRUCTORS INSURES THEY CANNOT BE USED IN THIS WAY
	Buffer(Buffer const &)            = delete;
	Buffer &operator=(Buffer const &) = delete;
	Buffer &operator=(Buffer &&)      = delete;

	/*
	 * This destructor cleans up the buffer when we're done using it, which would be good
	 * for when you leave a level or the application closes.
	 */
	~Buffer();

	/*
	 * This update overload loads byte data from the p_data argument into the buffer.
	 */
	void update(void *p_data, size_t size, size_t offset = 0);

	/*
	 * This update overload loads the binary data from the binary argument into the buffer.
	 */
	void update(const std::vector<uint8_t> &binary, size_t offset = 0);

	/*
	 * This update overload loads the data from the p_data argument into the buffer.
	 */
	void update(const uint8_t *p_data, size_t size, size_t offset = 0);

	/*
	 * This function employes the VMA API to get a CPU pointer of the buffer.
	 */
	void map();

	/*
	 * This function employs the VMA API to unmap the buffer.
	 */
	void unmap();

	/*
	 * This function flushes the cache for mapped data, something that should be done
	 * whenever we remap anything.
	 */
	void flush();

}; // class Buffer

}  // namespace W3D
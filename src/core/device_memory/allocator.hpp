#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

#include <vk_mem_alloc.h>

namespace W3D
{
// WE'LL USE THESE TYPES
class Device;
class Image;
class Buffer;

/*
* DeviceMemoryAllocator - this class is responsible for allocating memory
* for all application resources, like vertex buffers and images. Note, all 
* Vulkan resource allocation is done using the Vulkan Memory Allocator API. 
* Documentation for this API can be found at https://gpuopen.com/vulkan-memory-allocator/
*/
class DeviceMemoryAllocator : public VulkanObject<VmaAllocator>
{
  public:
	/*
	 * This constructor initializes this allocator for use. Note, all Vulkan resource
	 * allocation is done using the Vulkan Memory Allocator API. Documentation for
	 * this API can be found at https://gpuopen.com/vulkan-memory-allocator/
	 */
	DeviceMemoryAllocator(Device &device);

	/*
	 * This destructor destroys the VMA allocator.
	 */
	~DeviceMemoryAllocator();

	// VERTEX DATA ALLOCATION FUNCTIONS

	/*
	 * This is a helper function for the other buffer allocation functions
	 * in this class. It simply constructs and returns a Buffer object. Note that the buffer
	 * object is not yet filled with data.
	 */
	Buffer allocate_buffer(vk::BufferCreateInfo &buffer_cinfo, VmaAllocationCreateInfo &alloc_cinfo) const;

	/*
	 * This function is for allocating memory for a vertex buffer for the CPU/RAM, which
	 * we use to prepare vertex-related data before sending it to the device. Note that we
	 * setup its usage as a transfer source (eTransferSrc).
	 */
	Buffer allocate_staging_buffer(size_t size) const;

	/*
	 * This function is for allocating memory for a vertex buffer on the device. Notice we have
	 * specified the usage as a transfer destination (eTransferDst).
	 */
	Buffer allocate_vertex_buffer(size_t size) const;

	/*
	 * This function is for allocating memory for an index buffer, which associates vertices
	 * with the primitives (like triangles, points, lines, or quads) to be drawn. Note, this
	 * allocation will happen on the device as we specify usage to be as a transfer destination.
	 */
	Buffer allocate_index_buffer(size_t size) const;

	/*
	 * This function is for allocating memory for a buffer of uniform data that can be sent to the device.
	 */
	Buffer allocate_uniform_buffer(size_t size) const;

	/*
	 * This function is for allocating an empty buffer, which is sometimes useful as a placeholder.
	 */
	Buffer allocate_null_buffer() const;

	// IMAGE ALLOCATION FUNCTIONS

	/*
	 * This function is for allocating memory for a device image.
	 */
	Image allocate_device_only_image(vk::ImageCreateInfo &image_cinfo) const;

	/*
	 * This function is for allocating memory for an image.
	 */
	Image allocate_image(vk::ImageCreateInfo &image_cinfo, VmaAllocationCreateInfo &alloc_cinfo) const;

	/*
	 * This function is for allocating a placeholder null image.
	 */
	Image allocate_null_image() const;
};

}        // namespace W3D
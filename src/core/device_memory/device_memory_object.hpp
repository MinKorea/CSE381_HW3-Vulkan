#pragma once

#include "common/utils.hpp"
#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"
#include <vk_mem_alloc.h>

namespace W3D
{

class DeviceMemoryAllocator;

/*
* This struct stores information about how this object was allocated
* using the Vulkan Memory Allocator API.
*/
struct DeviceAllocationDetails
{
	VmaAllocator          allocator;
	VmaAllocation         allocation;
	VmaAllocationInfo     allocation_info;
	VkMemoryPropertyFlags flags;
};

/*
* This is a VulkanObject that contains information about how it was allocated. Note,
* this exists to be extended by the resources we wish to allocate.
*/
template <typename THandle>
class DeviceMemoryObject : public VulkanObject<THandle>
{
  protected:
	DeviceAllocationDetails details_;

  public:
	/*
	* Constructor that initializes the allocator to be used for this object.
	*/
	DeviceMemoryObject(VmaAllocator allocator, Key<DeviceMemoryAllocator> const &key)
	{
		details_.allocator = allocator;
	}

	/*
	 * Constructor that initializes the allocator to be used for this object.
	 */
	DeviceMemoryObject(DeviceMemoryObject &&rhs) :
	    VulkanObject<THandle>(std::move(rhs)),
	    details_(rhs.details_)
	{
		rhs.details_.allocator  = nullptr;
		rhs.details_.allocation = nullptr;
	}

	virtual ~DeviceMemoryObject() = default;

	/*
	* Asks the Vulkan Memory Allocator API to get update information
	* about memory that has been allocated.
	*/
	void update_flags()
	{
		if (details_.allocator)
		{
			vmaGetAllocationMemoryProperties(details_.allocator, details_.allocation, &details_.flags);
		}
	}

	/*
	* Returns true if memory allocated with this type can be mapped for host access.
	*/
	inline bool is_mappable()
	{
		return details_.flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}
};

}        // namespace W3D
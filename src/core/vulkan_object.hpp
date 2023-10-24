#pragma once

namespace W3D
{
// NOTE, THIS WILL BE A VULKAN API TYPE, DEPENDING ON WHAT WE ARE WRAPPING,
// SO IT MIGHT BE THINGS LIKE vk::CommandBuffer OR vk::CommandPool, OR OTHER API TYPES
template <typename THandle>
/*
 * This simple class serves as a generic handle wrapper for Vulkan-related
 * objects that will be allocated to the device using the Vulkan Memory Allocator API. Note
 * that this class serves as a base class for DeviceMemoryObject, providing a handle that
 * can be accessed directly in descendant classes.
 */
class VulkanObject
{
  protected:
	// ALL VULKAN-RELATED OBJECTS WILL NEED A HANDLE FOR VARIOUS API PURPOSES
	THandle handle_ = nullptr;

  public:
	/*
	* This constructor simply initializes the handle.
	*/
	VulkanObject(THandle handle = nullptr) :
	    handle_(handle){};

	/*
	* This constructor initializes the handle using the rhs argument.
	*/
	VulkanObject(VulkanObject &&rhs) :
	    handle_(rhs.handle_)
	{
		rhs.handle_ = nullptr;
	}

	/*
	* The destructor has nothing to destroy.
	*/
	virtual ~VulkanObject() = default;

	// THESE ARE DEACTIVATED
	VulkanObject(const VulkanObject &)            = delete;
	VulkanObject &operator=(const VulkanObject &) = delete;
	VulkanObject &operator=(VulkanObject &&)      = delete;

	/*
	* Accessor function for getting this VulkanObject's handle.
	*/
	inline const THandle &get_handle() const
	{
		return handle_;
	}

}; // class VulkanObject

}  // namespace W3D
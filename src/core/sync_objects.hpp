#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;

class Fence : public VulkanObject<vk::Fence>
{
  public:
	Fence(Device &device, std::nullptr_t nptr);
	Fence(Device &device, vk::FenceCreateFlags flags);
	Fence(Fence &&);
	~Fence() override;

  private:
	Device &device_;
};

class Semaphore : public VulkanObject<vk::Semaphore>
{
  public:
	Semaphore(Device &device, std::nullptr_t nptr);
	Semaphore(Device &device);
	Semaphore(Semaphore &&);
	~Semaphore() override;

  private:
	Device &device_;
};

}        // namespace W3D
#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;
class PhysicalDevice;

/*
* A wrapper class for a Vulkan API sampler, i.e. vk::Sampler.
*/
class Sampler : public VulkanObject<vk::Sampler>
{
  private:
	const Device &device_;

  public:
	static vk::SamplerCreateInfo linear_clamp_cinfo(const PhysicalDevice &physical_device, float max_lod);

	Sampler(const Device &device, std::nullptr_t nptr);
	Sampler(const Device &device, vk::SamplerCreateInfo &sampler_cinfo);
	Sampler(Sampler &&rhs);
	~Sampler() override;
};
}        // namespace W3D
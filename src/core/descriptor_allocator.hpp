#pragma once

#include <unordered_map>

#include "common/vk_common.hpp"

namespace W3D
{
class Device;

struct DescriptorAllocation
{
	vk::DescriptorSetLayout set_layout;
	vk::DescriptorSet       set;
};

class DescriptorAllocator
{
	struct PoolSizeFactor
	{
		vk::DescriptorType type;
		float              coeff;
	};

  private:
	Device            &device_;
	vk::DescriptorPool              current_pool{nullptr};
	std::vector<vk::DescriptorPool> free_pools_;
	std::vector<vk::DescriptorPool> used_pools_;

  public:
	const static std::vector<PoolSizeFactor> DESCRIPTOR_SIZE_FACTORS;
	const static uint32_t                    DEFAULT_SIZE;

	DescriptorAllocator(Device &device);
	~DescriptorAllocator();

	vk::DescriptorSet allocate(vk::DescriptorSetLayout &layout);
	void              reset_pools();
	const Device      &get_device();
	vk::DescriptorPool grab_pool();
	vk::DescriptorPool create_pool();

};

class DescriptorLayoutCache
{
  public:
	struct DescriptorSetLayoutDetails
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		bool                                        operator==(const DescriptorSetLayoutDetails &other) const;
		size_t                                      hash() const;
	};

	DescriptorLayoutCache(Device &device);
	~DescriptorLayoutCache();

	vk::DescriptorSetLayout create_descriptor_layout(
	    vk::DescriptorSetLayoutCreateInfo &layout_cinfo);

  private:
	struct DescriptorLayoutHash
	{
		std::size_t operator()(const DescriptorSetLayoutDetails &k) const
		{
			return k.hash();
		}
	};
	Device &device_;
	std::unordered_map<DescriptorSetLayoutDetails, vk::DescriptorSetLayout, DescriptorLayoutHash>
	    cache_;
};

class DescriptorBuilder
{
  public:
	static DescriptorBuilder begin(DescriptorLayoutCache &layout_cache,
	                               DescriptorAllocator   &descriptor_allocator);
	DescriptorBuilder       &bind_buffer(uint32_t binding, vk::DescriptorBufferInfo &buffer_info,
	                                     vk::DescriptorType type, vk::ShaderStageFlags flags);
	DescriptorBuilder       &bind_image(uint32_t binding, vk::DescriptorImageInfo &image_info,
	                                    vk::DescriptorType type, vk::ShaderStageFlags flags);
	DescriptorAllocation     build();

  private:
	DescriptorBuilder(DescriptorLayoutCache &layout_cache,
	                  DescriptorAllocator   &descriptor_allocator);
	std::vector<vk::WriteDescriptorSet>         writes_;
	std::vector<vk::DescriptorSetLayoutBinding> layout_bindings_;

	DescriptorLayoutCache &layout_cache_;
	DescriptorAllocator   &allocator_;
};

struct DescriptorState
{
	DescriptorState(Device &device) :
	    allocator(device),
	    cache(device)
	{
	}
	DescriptorAllocator   allocator;
	DescriptorLayoutCache cache;
};
}        // namespace W3D
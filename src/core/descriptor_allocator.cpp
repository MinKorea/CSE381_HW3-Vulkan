// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "descriptor_allocator.hpp"

// OUR OWN TYPES
#include "common/utils.hpp"
#include "device.hpp"
#include "vulkan/vulkan_hash.hpp"

namespace W3D
{

// Mental Model: DescriptorSets contains a set of ptrs that points to resources (buffers / images).
// To allocate a DescriptorSet, we need to first specify its layout (what images/buffers are binded to which slot)

/* ---------------------------- DescriptorBuilder --------------------------- */

DescriptorBuilder::DescriptorBuilder(DescriptorLayoutCache &layout_cache, DescriptorAllocator &allocator) :
    layout_cache_(layout_cache),
    allocator_(allocator)
{
}

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache &layout_cache,
                                           DescriptorAllocator   &allocator)
{
	DescriptorBuilder builder(layout_cache, allocator);
	return builder;
}

DescriptorBuilder &DescriptorBuilder::bind_buffer(uint32_t                  binding,
                                                  vk::DescriptorBufferInfo &buffer_info,
                                                  vk::DescriptorType        type,
                                                  vk::ShaderStageFlags      flags)
{
	vk::DescriptorSetLayoutBinding new_layout_binding;
	new_layout_binding.descriptorCount = 1;
	new_layout_binding.descriptorType  = type;
	new_layout_binding.stageFlags      = flags;
	new_layout_binding.binding         = binding;

	layout_bindings_.push_back(new_layout_binding);

	vk::WriteDescriptorSet write;
	write.descriptorCount = 1;
	write.descriptorType  = type;
	write.pBufferInfo     = &buffer_info;
	write.dstBinding      = binding;

	writes_.push_back(write);
	return *this;
}

DescriptorBuilder &DescriptorBuilder::bind_image(uint32_t                 binding,
                                                 vk::DescriptorImageInfo &image_info,
                                                 vk::DescriptorType       type,
                                                 vk::ShaderStageFlags     flags)
{
	vk::DescriptorSetLayoutBinding new_layout_binding;
	new_layout_binding.descriptorCount = 1;
	new_layout_binding.descriptorType  = type;
	new_layout_binding.stageFlags      = flags;
	new_layout_binding.binding         = binding;
	layout_bindings_.push_back(new_layout_binding);

	vk::WriteDescriptorSet write;
	write.descriptorCount = 1;
	write.descriptorType  = type;
	write.pImageInfo      = &image_info;
	write.dstBinding      = binding;
	writes_.push_back(write);
	return *this;
}

DescriptorAllocation DescriptorBuilder::build()
{
	vk::DescriptorSetLayoutCreateInfo layout_cinfo{
	    .bindingCount = to_u32(layout_bindings_.size()),
	    .pBindings    = layout_bindings_.data(),
	};
	vk::DescriptorSetLayout set_layout = layout_cache_.create_descriptor_layout(layout_cinfo);
	vk::DescriptorSet       set        = allocator_.allocate(set_layout);
	for (auto &write : writes_)
	{
		write.dstSet = set;
	}
	allocator_.get_device().get_handle().updateDescriptorSets(writes_, {});
	return {
	    .set_layout = set_layout,
	    .set        = set,
	};
}

/* -------------------------- DESCRIPTOR ALLOCATOR -------------------------- */

const std::vector<DescriptorAllocator::PoolSizeFactor>
    DescriptorAllocator::DESCRIPTOR_SIZE_FACTORS = {
        {vk::DescriptorType::eSampler, 0.5f},
        {vk::DescriptorType::eCombinedImageSampler, 4.0f},
        {vk::DescriptorType::eSampledImage, 4.0f},
        {vk::DescriptorType::eStorageImage, 1.0f},
        {vk::DescriptorType::eUniformTexelBuffer, 1.0f},
        {vk::DescriptorType::eStorageTexelBuffer, 1.0f},
        {vk::DescriptorType::eUniformBuffer, 2.0f},
        {vk::DescriptorType::eStorageBuffer, 2.0f},
        {vk::DescriptorType::eUniformBufferDynamic, 1.0f},
        {vk::DescriptorType::eStorageBufferDynamic, 1.0f},
        {vk::DescriptorType::eInputAttachment, 0.5f}};

const uint32_t DescriptorAllocator::DEFAULT_SIZE = 1000;

DescriptorAllocator::DescriptorAllocator(Device &device) :
    device_(device){};

DescriptorAllocator::~DescriptorAllocator()
{
	for (auto p : free_pools_)
	{
		device_.get_handle().destroyDescriptorPool(p);
	}
	for (auto p : used_pools_)
	{
		device_.get_handle().destroyDescriptorPool(p);
	}
}

vk::DescriptorSet DescriptorAllocator::allocate(vk::DescriptorSetLayout &layout)
{
	if (!current_pool)
	{
		current_pool = grab_pool();
		used_pools_.push_back(current_pool);
	}
	vk::DescriptorSetAllocateInfo descriptor_set_ainfo{
	    .descriptorPool     = current_pool,
	    .descriptorSetCount = 1,
	    .pSetLayouts        = &layout,
	};

	try
	{
		auto descriptor_set = device_.get_handle().allocateDescriptorSets(descriptor_set_ainfo);
		return descriptor_set[0];
	}
	catch (vk::FragmentedPoolError &err)
	{
	}
	catch (vk::OutOfPoolMemoryError &err)
	{
	}
	catch (...)
	{
		return vk::DescriptorSet{nullptr};
	}

	current_pool = grab_pool();
	used_pools_.push_back(current_pool);
	descriptor_set_ainfo.descriptorPool = current_pool;
	return device_.get_handle().allocateDescriptorSets(descriptor_set_ainfo)[0];
}

vk::DescriptorPool DescriptorAllocator::grab_pool()
{
	if (free_pools_.size() > 0)
	{
		vk::DescriptorPool pool = free_pools_.back();
		free_pools_.pop_back();
		return pool;
	}
	else
	{
		return create_pool();
	}
}

vk::DescriptorPool DescriptorAllocator::create_pool()
{
	std::vector<vk::DescriptorPoolSize> pool_sizes;
	pool_sizes.reserve(DESCRIPTOR_SIZE_FACTORS.size());
	for (auto &factor : DESCRIPTOR_SIZE_FACTORS)
	{
		pool_sizes.emplace_back(vk::DescriptorPoolSize{factor.type, to_u32(factor.coeff * DEFAULT_SIZE)});
	}
	vk::DescriptorPoolCreateInfo pool_cinfo{};
	pool_cinfo.maxSets       = DEFAULT_SIZE;
	pool_cinfo.poolSizeCount = to_u32(pool_sizes.size());
	pool_cinfo.pPoolSizes    = pool_sizes.data();
	return device_.get_handle().createDescriptorPool(pool_cinfo);
}

void DescriptorAllocator::reset_pools()
{
	for (auto p : used_pools_)
	{
		device_.get_handle().resetDescriptorPool(p);
	}

	free_pools_ = used_pools_;
	used_pools_.clear();
	current_pool = VK_NULL_HANDLE;
}

const Device &DescriptorAllocator::get_device()
{
	return device_;
}

/* -------------------------- DescriptorLayoutCache ------------------------- */

DescriptorLayoutCache::DescriptorLayoutCache(Device &device) :
    device_(device)
{
}

DescriptorLayoutCache::~DescriptorLayoutCache()
{
	auto it = cache_.begin();
	while (it != cache_.end())
	{
		device_.get_handle().destroyDescriptorSetLayout(it->second);
		it++;
	}
}

vk::DescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(
    vk::DescriptorSetLayoutCreateInfo &layout_cinfo)
{
	DescriptorSetLayoutDetails layout_details{};
	layout_details.bindings.reserve(layout_cinfo.bindingCount);
	bool    is_sorted    = true;
	int32_t last_binding = -1;

	for (uint32_t i = 0; i < layout_cinfo.bindingCount; i++)
	{
		layout_details.bindings.push_back(layout_cinfo.pBindings[i]);

		if (to_u32(layout_cinfo.pBindings[i].binding) > last_binding)
		{
			last_binding = layout_cinfo.pBindings[i].binding;
		}
		else
		{
			is_sorted = false;
		}
	}

	vk::DescriptorSetLayoutCreateInfo aaa;
	vk::DescriptorSetLayoutBinding    binding;

	// Vulkan does not require the bindings to be sorted.
	// We want them to be sorted to implement the cache look up logic.
	if (!is_sorted)
	{
		std::sort(layout_details.bindings.begin(), layout_details.bindings.end(), [](vk::DescriptorSetLayoutBinding &a, vk::DescriptorSetLayoutBinding &b) {
			return a.binding < b.binding;
		});
	}

	auto it = cache_.find(layout_details);
	if (it != cache_.end())
	{
		return it->second;
	}
	else
	{
		cache_.insert(std::make_pair(layout_details, device_.get_handle().createDescriptorSetLayout(layout_cinfo)));
		return cache_.at(layout_details);
	}
}

bool DescriptorLayoutCache::DescriptorSetLayoutDetails::operator==(
    const DescriptorSetLayoutDetails &other) const
{
	if (other.bindings.size() != bindings.size())
	{
		return false;
	}

	for (int i = 0; i < bindings.size(); i++)
	{
		const vk::DescriptorSetLayoutBinding &other_binding = other.bindings[i];
		const vk::DescriptorSetLayoutBinding &self_binding  = bindings[i];
		if (other_binding.binding != self_binding.binding ||
		    other_binding.descriptorType != self_binding.descriptorType ||
		    other_binding.descriptorCount != self_binding.descriptorCount ||
		    other_binding.stageFlags != self_binding.stageFlags)
		{
			return false;
		}
	}
	return true;
}

size_t DescriptorLayoutCache::DescriptorSetLayoutDetails::hash() const
{
	using std::hash;
	using std::size_t;

	size_t result = hash<size_t>()(bindings.size());
	for (const auto &b : bindings)
	{
		result ^= hash<vk::DescriptorSetLayoutBinding>()(b);
	}

	return result;
}

}        // namespace W3D
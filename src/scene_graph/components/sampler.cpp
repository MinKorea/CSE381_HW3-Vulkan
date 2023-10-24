// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "sampler.hpp"

// OUR OWN TYPES
#include "core/device.hpp"

namespace W3D::sg
{
Sampler::Sampler(const Device &device, const std::string &name, vk::SamplerCreateInfo &sampler_cinfo) :
    Component(name),
    device_(device),
    sampler_(device_, sampler_cinfo){};

std::type_index Sampler::get_type()
{
	return typeid(Sampler);
}

vk::Sampler Sampler::get_handle()
{
	return sampler_.get_handle();
}
}        // namespace W3D::sg

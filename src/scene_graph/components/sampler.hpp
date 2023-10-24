#pragma once

#include "common/vk_common.hpp"
#include "core/sampler.hpp"
#include "scene_graph/component.hpp"

namespace W3D
{
class Device;

namespace sg
{
/*
* A sampler provides the mechanism for mapping pixels in a texture
* to fragments in a shaded polygon. This serves as a scene component
* that we can use for rendering. Note, we foolishly have two classes
* named Sampler, one here, one in the parent namespace. This class
* has an instance of that Sampler and also serves as a wrapper class
* for a Vulkan API sampler, i.e. vk::Sampler
*/
class Sampler : public Component
{
  private:
	const Device &device_;
	W3D::Sampler  sampler_;

  public:
	Sampler(const Device &device, const std::string &name, vk::SamplerCreateInfo &sampler_cinfo);

	/*
	* This destructor destroys the Vulkan sampler.
	*/
	virtual ~Sampler() override = default;

	/*
	* Accessor method for getting the actual type of this object.
	*/
	virtual std::type_index get_type() override;

	/*
	* Accessor method for getting the Vulkan handle for the Vulkan API sampler.
	*/
	vk::Sampler             get_handle();

};	// class Sampler

}	// namespace sg

}	// namespace W3D
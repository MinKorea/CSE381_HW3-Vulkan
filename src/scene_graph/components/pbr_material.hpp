#pragma once

#include "common/glm_common.hpp"
#include "common/vk_common.hpp"
#include "scene_graph/components/material.hpp"

namespace W3D::sg
{
/*
* A material to be used for a Physically Based Rendering technique,
* see https://pbr-book.org/
*/
class PBRMaterial : public Material
{
  public:
	// THESE INSTANCE VARIABLES CAN BE USED TO GIVE THE SURFACE DIFFERENT PROPERTIES
	glm::vec4         base_color_factor_{0.0f, 0.0f, 0.0f, 0.0f};
	float             metallic_factor{0.0f};
	float             roughness_factor{0.0f};
	vk::DescriptorSet set;

	/*
	* Constructor just initializes the name
	*/
	PBRMaterial(const std::string &name);

	/*
	* Nothing for the destructor to destroy.
	*/
	virtual ~PBRMaterial() = default;

	/*
	* For getting the actual type of this object.
	*/
	virtual std::type_index get_type() override;

};	// class PBRMaterial

}	// namespace W3D::sg
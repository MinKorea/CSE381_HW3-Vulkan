#pragma once

#include <unordered_map>

#include "common/glm_common.hpp"
#include "scene_graph/component.hpp"

namespace W3D::sg
{
enum class AlphaMode
{
	Opaque,
	Mask,
	Blend
};

class Texture;

/*
* This class represents a scene component for storing a mesh's material
* properties, i.e. how it will interact with light. Note that it exists
* to be a base class for various material implementations.
*/
class Material : public Component
{
  public:
	// ALL THE TEXTURES ASSOCIATED WITH THIS MATERIAL
	std::unordered_map<std::string, Texture *> texture_map_;

	// EMISSIVE LIGHTING PROPERTY
	glm::vec3 emissive_{0.0f, 0.0f, 0.0f};

	// ADDITIONAL RENDERING SETTINGS
	bool      is_double_sided{false};
	float     alpha_cutoff_{0.5f};
	AlphaMode alpha_mode_{AlphaMode::Opaque};

	/*
	* Our constructor only sends the name up to the parent constructor.
	*/
	Material(const std::string &name);
	Material(Material &&other) = default;

	/*
	* Nothing to destroy in this class.
	*/
	virtual ~Material()        = default;

	/*
	* For getting the actual type of this object, which will be a subclass.
	*/
	virtual std::type_index get_type() override;

};	// class Material

}	// namespace W3D::sg
#pragma once

#include "scene_graph/component.hpp"

namespace W3D
{

class ImageResource;
class Sampler;

namespace sg
{
/*
* Our Texture class represents a texture to be used in a scene for
* rendering models. Note that this is a simple class for combining
* the image resource that is loaded with the type of sampler to
* be used for extracting pixels. Note, this class exists to be a base
* class for 
*/
class Texture : public Component
{
  public:
	// WE HAVE TWO PUBLIC INSTANCE VARIABLES
	ImageResource *p_resource_ = nullptr;	// IMAGE WE ARE MANAGING
	Sampler       *p_sampler_  = nullptr;	// SAMPLER FOR EXTRACTING PIXELS

	/*
	* Constructor will simply send the name up to the parent constructor,
	* no other data will be initialized.
	*/
	Texture(const std::string &name);
	Texture(Texture &&other) = default;

	/*
	* Nothing to destroy.
	*/
	virtual ~Texture() = default;

	/*
	* Function for getting the actual type of this object, which will be a subclass.
	*/
	virtual std::type_index get_type() override;
};
}        // namespace sg

};        // namespace W3D
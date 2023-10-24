#pragma once

#include <stdint.h>

#include "common/vk_common.hpp"
#include "core/image_resource.hpp"
#include "scene_graph/component.hpp"

namespace W3D
{

class Device;

namespace sg
{

/*
* This class represents an image that is used in a scene, and so it extends Component
* so that it can be managed by the scene graph.
*/
class Image : public Component
{
  private:
	ImageResource resource_;

  public:
	/*
	* Missing implementation
	*/
	Image(ImageResource &&resrc, const std::string &name);

	/*
	* Constructor initializes this image using an image argument.
	*/
	Image(Image &&);

	/*
	* The destructor will need to destroy the Vulkan image.
	*/
	virtual ~Image() = default;

	/*
	* This will get the actual type of the object, which will be a subtype.
	*/
	virtual std::type_index get_type() override;

	/*
	* Accessor method for getting the ImageResource associated with this image.
	*/
	ImageResource           &get_resource();

	/*
	* Accessor method for getting the ImageTransferInfo associated with this image.
	*/
	const ImageTransferInfo &get_image_transfer_info();

	/*
	* Mutator method for setting the ImageResource associated with this image.
	*/
	void set_resource(ImageResource &&resource);

};	// class Image

}	// namespace sg

}	// namespace W3D
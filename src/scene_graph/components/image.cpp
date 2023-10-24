// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "image.hpp"

// OUR OWN TYPES
#include "common/file_utils.hpp"
#include "common/logging.hpp"
#include "common/utils.hpp"
#include "core/device.hpp"

namespace W3D::sg
{

Image::Image(ImageResource &&resource, const std::string &name) :
    Component(name),
    resource_(std::move(resource))
{
}

Image::Image(Image &&rhs) :
    Component(rhs.get_name()),
    resource_(std::move(rhs.resource_))
{
}

std::type_index Image::get_type()
{
	return typeid(Image);
}

ImageResource &Image::get_resource()
{
	return resource_;
}

void Image::set_resource(ImageResource &&resource)
{
	resource_ = std::move(resource);
}

}   // namespace W3D::sg
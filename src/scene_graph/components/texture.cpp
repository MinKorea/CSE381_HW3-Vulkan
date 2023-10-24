// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "texture.hpp"

// OUR OWN TYPES
#include "common/error.hpp"

namespace W3D::sg
{
Texture::Texture(const std::string &name) :
    Component(name)
{
}

std::type_index Texture::get_type()
{
	return typeid(Texture);
}

}	// namespace W3D::sg
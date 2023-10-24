// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "submesh.hpp"

// OUR OWN TYPES
#include "common/vk_common.hpp"
#include "core/device_memory/buffer.hpp"
#include "material.hpp"

namespace W3D::sg
{

std::array<vk::VertexInputAttributeDescription, 5> Vertex::get_input_attr_descriptions()
{
	std::array<vk::VertexInputAttributeDescription, 5> descriptions;
	descriptions[0] = {
	    .location = 0,
	    .binding  = 0,
	    .format   = vk::Format::eR32G32B32Sfloat,
	    .offset   = offsetof(Vertex, pos),
	};
	descriptions[1] = {
	    .location = 1,
	    .binding  = 0,
	    .format   = vk::Format::eR32G32B32Sfloat,
	    .offset   = offsetof(Vertex, norm),
	};
	descriptions[2] = {
	    .location = 2,
	    .binding  = 0,
	    .format   = vk::Format::eR32G32Sfloat,
	    .offset   = offsetof(Vertex, uv),
	};
	descriptions[3] = {
	    .location = 3,
	    .binding  = 0,
	    .format   = vk::Format::eR32G32B32A32Sfloat,
	    .offset   = offsetof(Vertex, joint),
	};
	descriptions[4] = {
	    .location = 4,
	    .binding  = 0,
	    .format   = vk::Format::eR32G32B32A32Sfloat,
	    .offset   = offsetof(Vertex, weight),
	};
	return descriptions;
};

SubMesh::SubMesh(const std::string &name) :
    Component(name)
{
}

SubMesh::~SubMesh()
{}

std::type_index SubMesh::get_type()
{
	return typeid(SubMesh);
}

void SubMesh::set_material(const Material &material)
{
	p_material_ = &material;
}

const Material *SubMesh::get_material() const
{
	return p_material_;
}

}        // namespace W3D::sg
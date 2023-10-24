// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "camera.hpp"

// OUR OWN TYPES
#include "common/error.hpp"
#include "scene_graph/node.hpp"

namespace W3D::sg
{
Camera::Camera(const std::string &name) :
    Component(name)
{
}

std::type_index Camera::get_type()
{
	return typeid(Camera);
}

Node *Camera::get_node()
{
	return pNode_;
}

void Camera::set_node(Node &node)
{
	pNode_ = &node;
}

const glm::mat4 Camera::get_pre_rotation()
{
	return pre_rotation_;
}

void Camera::set_pre_rotation(const glm::mat4 &pre_rotation)
{
	pre_rotation_ = pre_rotation;
}

glm::mat4 Camera::get_view()
{
	if (!pNode_)
	{
		throw std::runtime_error("Camera component is not attached to a node");
	}
	auto &T = pNode_->get_component<Transform>();
	return glm::inverse(T.get_world_M());
}

} // namespace W3D::sg
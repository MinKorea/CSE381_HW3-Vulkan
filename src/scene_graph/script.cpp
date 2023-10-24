// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "script.hpp"

namespace W3D::sg
{
Script::Script(const std::string &name) :
    Component(name){};

std::type_index Script::get_type()
{
	return typeid(Script);
}

void Script::process_event(const Event &event)
{
}

void Script::resize(uint32_t width, uint32_t height)
{
}

NodeScript::NodeScript(Node &node, const std::string &name) :
    Script(name),
    node_(node)
{
}

Node &NodeScript::get_node()
{
	return node_;
};

}        // namespace W3D::sg
// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "component.hpp"

namespace W3D::sg
{

Component::Component(const std::string &name) :
    name_(name){};

const std::string &Component::get_name() const
{
	return name_;
}

}	// namespace W3D::sg
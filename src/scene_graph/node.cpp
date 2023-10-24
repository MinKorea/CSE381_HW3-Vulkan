// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "node.hpp"

namespace W3D::sg
{
Node::Node(const size_t id, const std::string &name) :
    id_(id),
    name_(name),
    T_(*this)
{
	set_component(T_);
}

void Node::add_child(Node &child)
{
	children_.push_back(&child);
}

void Node::set_parent(Node &parent)
{
	parent_ = &parent;
	T_.invalidate_local_M();
}

void Node::set_component(Component &component)
{
	auto it = components_.find(component.get_type());
	if (it != components_.end())
	{
		it->second = &component;
	}
	else
	{
		components_.insert(std::make_pair(component.get_type(), &component));
	}
}

const size_t Node::get_id() const
{
	return id_;
};

const std::string &Node::get_name() const
{
	return name_;
};

Node *Node::get_parent() const
{
	return parent_;
};

const std::vector<Node *> &Node::get_children() const
{
	return children_;
};

Component &Node::get_component(const std::type_index index)
{
	return *components_.at(index);
};

sg::Transform &Node::get_transform() const
{
	return dynamic_cast<sg::Transform &>(*components_.at(typeid(sg::Transform)));
}

bool Node::has_component(const std::type_index index)
{
	return components_.count(index) > 0;
}

}	// namespace W3D::sg
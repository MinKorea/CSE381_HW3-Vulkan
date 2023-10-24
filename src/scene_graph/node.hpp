#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "components/transform.hpp"

namespace W3D::sg
{

/*
* Represents a node in the application's scene graph. Note we are using a hierarchical
* scene graph where each node has components attached to it and nodes are hierarchically
* organized so that a node has a parent and can have child nodes. Note that this class
* serves the scene graph but also exists to serve as a base class for different types of
* objects that will be managed in the scene.
*/
class Node
{
  private:
	size_t                                           id_;				// EACH NODE HAS A UNIQUE ID NUMBER
	std::string                                      name_;				// EACH NODE IS NAMED
	Transform                                        T_;				// STORES TRANSLATION/ROTATION/SCALING
	Node                                            *parent_{nullptr};	// PARENT NODE IN SCENE GRAPH
	std::vector<Node *>                              children_;			// CHILD NODES FOR THIS NODE
	std::unordered_map<std::type_index, Component *> components_;		// COMPONENT ASSOCIATED WITH THIS NODE

  public:
	/*
	* Constructor initializes its id and name.
	*/
	Node(const size_t id, const std::string &name);

	/*
	* Accessor method for getting the node's id.
	*/
	const size_t               get_id() const;

	/*
	* Accessor method for getting the name of the node.
	*/
	const std::string         &get_name() const;

	/*
	* Accessor method for getting the node's parent node.
	*/
	Node                      *get_parent() const;

	/*
	* Accessor method for getting this node's child nodes.
	*/
	const std::vector<Node *> &get_children() const;

	/*
	* Accessor method for getting the world transform for this node.
	*/
	sg::Transform             &get_transform() const;

	/*
	* Accessor method for getting a component managed by this node as its actual type.
	*/
	template <class T>
	inline T &get_component()
	{
		return dynamic_cast<T &>(get_component(typeid(T)));
	}

	/*
	* Accessor method for getting a component managed by this node.
	*/
	Component &get_component(const std::type_index index);

	/*
	* Mutator for setting the parent node.
	*/
	void set_parent(Node &parent);

	/*
	* Adds the component argument to the list of components this node manages.
	*/
	void set_component(Component &component);

	/*
	* Tests to see if this node has a component or not.
	*/
	template <class T>
	bool has_component()
	{
		return has_component(typeid(T));
	}

	/*
	 * Tests to see if this node has a component or not.
	 */
	bool has_component(const std::type_index index);

	/*
	* Adds a node as a child to this node.
	*/
	void add_child(Node &child);

};	// class Node

}	// namespace W3D::sg
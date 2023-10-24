#pragma once

#include <vector>

#include "aabb.hpp"
#include "scene_graph/component.hpp"
#include "scene_graph/node.hpp"

namespace W3D::sg
{
class SubMesh;

/*
* This class represents a mesh that might be made up of submeshes. Such a mesh
* could be animated by animating submeshes separately. Note, this class is a
* component, meaning it is a named object managed by the scene graph.
*/
class Mesh : public Component
{
  private:
	AABB                   bounds_;		// THIS MESH'S BOUNDING BOX
	std::vector<SubMesh *> p_submeshs;	// SUBMESHES CAN BE ANIMATED SEPARATELY
	std::vector<Node *>    p_nodes;		// NODE OBJECTS ASSOCIATED WITH THIS MESH

  public:
	/*
	* This constructor simply initializes the name using the parent constructor.
	*/
	Mesh(const std::string &name);

	/*
	* Nothing to destroy.
	*/
	virtual ~Mesh() = default;

	/*
	* Mutator method that adds submesh to this mesh.
	*/
	void add_submesh(SubMesh &submesh);

	/*
	* Mutator method that adds node to this mesh.
	*/
	void add_node(Node &node);

	/*
	* Accessor method for getting the actual type of this object, which will be a subtype.
	*/
	std::type_index               get_type() override;

	/*
	* Accessor method for getting the bounding box for this object as an immutable object.
	*/
	const AABB                   &get_bounds() const;

	/*
	* Accessor method for getting the bounding box for this object as a mutable object.
	*/
	AABB                         &get_mut_bounds();

	/*
	* Accessor method for getting all the submeshes managed by this mesh.
	*/
	const std::vector<SubMesh *> &get_p_submeshs() const;

	/*
	* Accessor method for getting the nodes associated with this mesh's submeshes.
	*/
	const std::vector<Node *>    &get_p_nodes() const;

};	// class Mesh

}	// namespace W3D::sg
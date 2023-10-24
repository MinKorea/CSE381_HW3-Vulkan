#pragma once

#include "common/glm_common.hpp"
#include "scene_graph/component.hpp"

/*
 * The W3D::sg namespace is the Wolfie3D Scene Graph (sg) types, used to manage demo objects.
 */
namespace W3D::sg
{
class Node;

/*
* A camera can be placed in the game scene and we'll render all scene objects relative to it.
*/
class Camera : public Component
{
  private:
	Node     *pNode_{nullptr};
	glm::mat4 pre_rotation_{1.0f};

  public:
	/*
	* This constructor simply initializes inherited Component data using name.
	*/
	Camera(const std::string &name);

	/*
	* There is nothing to destroy for this object, so default behavior is used.
	*/
	virtual ~Camera() = default;

	/*
	* This is a base class for other types of camera classes so this inherited function
	* is for asking a camera what type it actually is.
	*/
	virtual std::type_index get_type() override;

	/*
	* Accessor method for getting this camera's node, which is used for the camera
	* location and orientation (i.e. rotation).
	*/
	Node *get_node();

	/*
	* This mutator method sets this camera's node, which is used for the camera
	* location and orientation (i.e. rotation).
	*/
	void            set_node(Node &node);

	/*
	* Accessor method for getting a matrix representing the transformation of this camera
	* before rotation.
	*/
	const glm::mat4 get_pre_rotation();

	/*
	 * Mutator method for setting a matrix representing the transformation of this camera
	 * before rotation.
	 */
	void    set_pre_rotation(const glm::mat4 &pre_rotation);

	/*
	* Calculates and returns a matrix that is the inverse of this camera, i.e. the inverse
	* of this camera's node's transform. Scene objects can then be transformed relative to
	* the camera.
	*/
	glm::mat4 get_view();

	/*
	* Since projection can be perspective or othrographic, the implementation of getting
	* the camera's projection matrix will be left up to child classes, therefore this is
	* a virtual void method (i.e. abstract).
	*/
	virtual glm::mat4 get_projection() = 0;

}; // class Camera

}  // namespace W3D::sg
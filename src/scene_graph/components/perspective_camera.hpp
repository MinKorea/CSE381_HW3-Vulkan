#pragma once

#include "camera.hpp"
namespace W3D::sg
{
/*
* Represents a camera in a scene that employs perspective projection. Note, this
* object will provide the perspective matrix used for rendering.
*/
class PerspectiveCamera : public Camera
{
  private:
	float aspect_ratio_ = 1.0f;					// ASPECT RATIO WE'LL PROJECT ONTO
	float fov_          = glm::radians(60.0f);	// FIELD OF VIEW
	float zfar_         = 100.0f;				// FAR CLIPPING PLANE FOR FRUSTUM
	float znear_        = 0.1f;					// NEAR CLIPPING PLANE FOR FRUSTUM

  public:
	/*
	* Constructor for setting up this camera, which is a component and so requires a name.
	*/
	PerspectiveCamera(const std::string &name);

	/*
	* Destuctor has nothing to destroy.
	*/
	virtual ~PerspectiveCamera() = default;

	/*
	* This function employs the glm library to compute and build the projection
	* matrix for perspective projection and then returns that matrix.
	*/
	virtual glm::mat4 get_projection() override;

	/*
	* Accessor method for getting the aspect ratio.
	*/
	float get_aspect_ratio() const;

	/*
	* Accessor method for getting the field of view, i.e. the angle used during projection.
	*/
	float get_field_of_view() const;

	/*
	* Accessor method for getting the far clipping plane distance from the camera.
	*/
	float get_far_plane() const;

	/*
	* Accessor method for getting the near clipping plane distance from the camera.
	*/
	float get_near_plane() const;

	/*
	 * Mutator method for setting the aspect ratio.
	 */
	void set_aspect_ratio(float aspect_ratio);

	/*
	 * Mutator method for setting the field of view angle
	 */
	void set_field_of_view(float fov);

	/*
	 * Mutator method for setting the far clipping plane distance from the camera.
	 */
	void set_far_plane(float zfar);

	/*
	 * Mutator method for setting the near clipping plane distance from the camera.
	 */
	void set_near_plane(float znear);
};
}        // namespace W3D::sg
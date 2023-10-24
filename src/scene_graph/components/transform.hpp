#pragma once

#include <glm/gtx/quaternion.hpp>

#include "common/glm_common.hpp"
#include "scene_graph/component.hpp"

namespace W3D::sg
{
class Node;

// Class that represents a typical TRS transform
class Transform : public Component
{
  public:
	Transform(Node &node);
	virtual ~Transform() = default;
	virtual std::type_index get_type() override;

	// returns a model-to-world matrix
	glm::mat4 get_world_M();

	// returns only the node's transformation matrix (without considering its parent nods)
	glm::mat4 get_local_M();

	glm::quat get_rotation();
	glm::vec3 get_translation();
	glm::vec3 get_scale();

	void set_tranlsation(const glm::vec3 &translation);
	void set_rotation(const glm::quat &rotation);
	void set_scale(const glm::vec3 &scale);
	void set_world_M(const glm::mat4 &world_M);
	void invalidate_local_M();

  private:
	void update_world_M();

	Node     &node_;
	glm::vec3 translation_ = glm::vec3(0.0, 0.0, 0.0);
	glm::quat rotation_    = glm::quat(1.0, 0.0, 0.0, 0.0);
	glm::vec3 scale_       = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 local_M_ = glm::mat4(1.0);

	bool need_update_ = false;
};
}        // namespace W3D::sg
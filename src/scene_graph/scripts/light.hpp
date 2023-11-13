#pragma once

#include "scene_graph/script.hpp"

namespace W3D::sg
{

class Light : public Script
{
  private:
	glm::vec3&                        new_pos;
	float                             speed_multiplier_ = 2.0f;
	std::unordered_map<KeyCode, bool> key_pressed_;

  public:
	static const float TRANSLATION_MOVE_STEP;

	/*
	 * Constructor simply sends the node argument to its parent constructor.
	 */
	Light(glm::vec3& pos, const std::string &name);

	/*
	 * Called each frame, this function updates the player's state. Note, if a key
	 * for controlling the player is pressed it will respond here for movement.
	 */
	void update(float delta_time) override;

	/*
	 * When the player is the active object that can be moved it will be asked to
	 * respond to events like key presses.
	 */
	void process_event(const Event &event) override;

	virtual std::type_index get_type() override;
};

}        // namespace W3D::sg

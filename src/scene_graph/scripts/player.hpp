#pragma once

#include "scene_graph/script.hpp"

namespace W3D::sg
{
/*
* This is the script for a player, which in our demo is just a cube in
* the scene. The update function specifically
*/
class Player : public NodeScript
{
  private:
	float speed_multiplier_ = 2.0f;
	std::unordered_map<KeyCode, bool> key_pressed_;

  public:
	static const float TRANSLATION_MOVE_STEP;

	/*
	* Constructor simply sends the node argument to its parent constructor.
	*/
	Player(Node &node);

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

};	// class Player

}	// namespace W3D::sg
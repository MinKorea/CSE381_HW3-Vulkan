#pragma once

#include "common/glm_common.hpp"
#include "scene_graph/script.hpp"

namespace W3D::sg
{
/*
* This class is a NodeScript that provides a programmed application response for
* events associated with the camera, like camera movement.
*/
class FreeCamera : public NodeScript
{
  private:
	float     speed_multiplier_ = 3.0f;	// CHANGING THIS WILL ALTER HOW FAST THE CAMERA MOVES
	glm::vec2 mouse_move_delta_{0.0f};	// HOW MUCH HAS THE MOUSE MOVED SINCE THE LAST TIME WE CHECKED?
	glm::vec2 mouse_last_pos_{0.0f};	// THE LAST KNOWN MOUSE POSITION
	std::unordered_map<KeyCode, bool>     key_pressed_;				// KEEPS TRACK OF WHICH KEYS WERE PRESSED
	std::unordered_map<MouseButton, bool> mouse_button_pressed_;	// KEEPS TRACK OF WHICH MOUSE BUTTONS WERE PRESSED

  public:
	// THESE LET US CONTROL CAMERA ROTATION AND MOVEMENT INCREMENTS
	static const float    ROTATION_MOVE_WEIGHT;
	static const float    TRANSLATION_MOVE_STEP;
	static const float    TRANSLATION_MOVE_WEIGHT;
	static const uint32_t TRANSLATION_MOVE_SPEED;

	/*
	* Constructor will set the node this object will control.
	*/
	FreeCamera(Node &node);

	/*
	* Destructor doesn't have anything to clean up.
	*/
	virtual ~FreeCamera() = default;

	/*
	* This method override provides a programmed response for when this camera
	* is actively selected in the demo.
	*/
	void update(float delta_time) override;

	/*
	* This method records information about the event that has occured, note it
	* does not provide a programmed response, it leaves that to update.
	*/
	void process_event(const Event &event) override;

	/*
	* This method responds to when the rendering surface has changed. We respond here
	* by updating the aspect ratio.
	*/
	void resize(uint32_t width, uint32_t height) override;

};	// class FreeCamera

}	// namespace W3D::sg
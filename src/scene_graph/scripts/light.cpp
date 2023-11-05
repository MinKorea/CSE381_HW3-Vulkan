#include "light.hpp"

namespace W3D::sg
{
const float Light::TRANSLATION_MOVE_STEP = 5.0f;

Light::Light(glm::vec3 &pos)
{
}

void Light::update(float delta_time)
{

}

void Light::process_event(const Event& event)
{
	if(event.type == EventType::eKeyInput)
	{
		const auto &key_event = static_cast<const KeyInputEvent &>(event);

		// WE DO THIS SO WE CAN RESPOND TO HOLDING DOWN A KEY FOR MULTIPLE FRAMES
		// WHICH WE THEN DO INSIDE THE update FUNCTION
		if (key_event.action == KeyAction::eDown || key_event.action == KeyAction::eRepeat)
		{
			key_pressed_[key_event.code] = true;
		}
		else
		{
			key_pressed_[key_event.code] = false;
		}
	}
}

}
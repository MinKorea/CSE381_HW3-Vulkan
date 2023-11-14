#include "light.hpp"

namespace W3D::sg
{
const float Light::TRANSLATION_MOVE_STEP = 5.0f;

Light::Light(glm::vec3 &pos, const std::string &name) :
    Script(name),
    new_pos(pos)
{}

void Light::update(float delta_time)
{
	glm::vec3 delta_translation(0.0f, 0.0f, 0.0f);

	// Caveat: The models are rotated! Therefore, we translate the objects in model space in z-direction
	if (key_pressed_[KeyCode::eW])
	{
		delta_translation.y += TRANSLATION_MOVE_STEP;
	}

	if (key_pressed_[KeyCode::eS])
	{
		delta_translation.y -= TRANSLATION_MOVE_STEP;
	}

	if (key_pressed_[KeyCode::eA])
	{
		delta_translation.x -= TRANSLATION_MOVE_STEP;
	}

	if (key_pressed_[KeyCode::eD])
	{
		delta_translation.x += TRANSLATION_MOVE_STEP;
	}

	// WE NEED TO SCALE BECAUSE WE HAVE A VARIABLE TIMER
	delta_translation *= speed_multiplier_ * delta_time;

	new_pos = new_pos + delta_translation;

}

void Light::process_event(const Event& event)
{
	if (event.type == EventType::eKeyInput)
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

std::type_index Light::get_type()
{
	return typeid(Light);
}

}


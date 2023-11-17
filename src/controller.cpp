// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "controller.hpp"
#include <iostream>
#include <string>

// OUR OWN TYPES
#include "scene_graph/components/aabb.hpp"
#include "scene_graph/components/mesh.hpp"
#include "scene_graph/event.hpp"
#include "scene_graph/node.hpp"
#include "scene_graph/script.hpp"
#include "scene_graph/components/camera.hpp"
#include "scene_graph/components/perspective_camera.hpp"

namespace W3D
{
// Class that is responsible for dispatching events and answering collision queries

Controller::Controller(sg::Node &camera_node, sg::Node &player_1_node, sg::Node &player_2_node, sg::Node &player_3_node, sg::Node &player_4_node, sg::Node &player_5_node, sg::Node &projectile_node,
                       sg::Script &light_1_script, sg::Script &light_2_script, sg::Script &light_3_script, sg::Script &light_4_script) :
    camera_(camera_node),
    player_1(player_1_node),
    player_2(player_2_node),
    player_3(player_3_node),
    player_4(player_4_node),
    player_5(player_5_node),
    projectile(projectile_node),
    light_1(light_1_script),
    light_2(light_2_script),
    light_3(light_3_script),
    light_4(light_4_script)
{
}

void Controller::process_event(const Event &event)
{
	// IF IT'S A KEY PRESS WE NEED TO CHECK TO SEE IF WE SHOULD SWITCH MODES
	if (event.type == EventType::eKeyInput)
	{
		const auto &key_input_event = static_cast<const KeyInputEvent &>(event);

		if (key_input_event.code == KeyCode::eR)
		{
			auto &p1 = player_1.get_transform();
			auto &p2 = player_2.get_transform();
			auto &p3 = player_3.get_transform();
			auto &p4 = player_4.get_transform();
			auto &p5 = player_5.get_transform();
			p1.set_tranlsation(glm::vec3(0.0f, 0.0f, 0.0f));
			p1.set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
			//p1.set_scale(glm::vec3(0.0f, 0.0f, 0.0f)); // Test edited in
			p2.set_tranlsation(glm::vec3(3.0f, 0.0f, 0.0f));
			p2.set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
			p3.set_scale(glm::vec3(0.0f, 0.0f, 0.0f));
			p3.set_tranlsation(glm::vec3(-3.0f, 0.0f, 0.0f));
			p4.set_scale(glm::vec3(0.0f, 0.0f, 0.0f));
			p4.set_tranlsation(glm::vec3(0.0f, 0.0f, 2.0f));
			p5.set_scale(glm::vec3(0.0f, 0.0f, 0.0f));
			p5.set_tranlsation(glm::vec3(0.0f, 0.0f, -2.0f));
			auto &cam = camera_.get_transform();
			cam.set_tranlsation(glm::vec3(0.0f, 0.0f, 5.0f));
			cam.set_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
		}
		// NUMBER KEYS ARE ALL GREATER
		if (key_input_event.code > KeyCode::eQ)
		{
			switch_mode(key_input_event.code);
			return;
		}
	}

	// DELIVER IT TO THE SCRIPT
	deliver_event(event);
}

void Controller::switch_mode(KeyCode code)
{
	// THE 1 KEY SWITCHES CONTROL TO THE PLAYER 1 CUBE
	if (code == KeyCode::e1)
	{
		mode_ = ControllerMode::ePlayer1;
	}
	// THE 2 KEY SWITCHES CONTROL TO THE PLAYER 2 CUBE
	else if (code == KeyCode::e2)
	{
		mode_ = ControllerMode::ePlayer2;
	}
	// THE 3 KEY SWITCHES CONTROL TO THE CAMERA
	else if (code == KeyCode::e3)
	{
		mode_ = ControllerMode::eCamera;
	}
	else if (code == KeyCode::e4)
	{
		mode_ = ControllerMode::eLight1;
	}
	else if (code == KeyCode::e5)
	{
		mode_ = ControllerMode::eLight2;
	}
	else if (code == KeyCode::e6)
	{
		mode_ = ControllerMode::eLight3;
	}
	else if (code == KeyCode::e7)
	{
		mode_ = ControllerMode::eLight4;
	}
	else if (code == KeyCode::e8)
	{
		mode_ = ControllerMode::ePlayer3;
	}
	else if (code == KeyCode::e9)
	{
		mode_ = ControllerMode::ePlayer4;
	}
	else if (code == KeyCode::e0)
	{
		mode_ = ControllerMode::ePlayer5;
	}
}

void Controller::deliver_event(const Event &event)
{
	// NOTIFY THE ASSOCIATED SCRIPT FOR THE GAME OBJECT SO IT CAN PROVIDE
	// A FURTHER REPONSE. FIRST GET THE ASSOCIATED SCRIPT
	sg::Script *p_script;
	if (mode_ == ControllerMode::ePlayer1)
	{
		p_script = &player_1.get_component<sg::Script>();
	}
	else if (mode_ == ControllerMode::ePlayer2)
	{
		p_script = &player_2.get_component<sg::Script>();
	}
	else if (mode_ == ControllerMode::eCamera)
	{
		p_script = &camera_.get_component<sg::Script>();
	}
	else if (mode_ == ControllerMode::eLight1)
	{
		p_script = &light_1;
	}
	else if (mode_ == ControllerMode::eLight2)
	{
		p_script = &light_2;
	}
	else if (mode_ == ControllerMode::eLight3)
	{
		p_script = &light_3;
	}
	else if(mode_ == ControllerMode::eLight4)
	{
		p_script = &light_4;
	}
	else if (mode_ == ControllerMode::ePlayer3)
	{
		p_script = &player_3.get_component<sg::Script>();
	}
	else if (mode_ == ControllerMode::ePlayer4)
	{
		p_script = &player_4.get_component<sg::Script>();
	}
	else if (mode_ == ControllerMode::ePlayer5)
	{
		p_script = &player_5.get_component<sg::Script>();
	}
	else
	{
		p_script = &camera_.get_component<sg::Script>();
	}
	

	// AND NOW ASK THE SCRIPT TO PROVIDE A PROGRAMMED RESPONSE
	p_script->process_event(event);
}

bool Controller::are_players_colliding()
{
	glm::mat4 p1_M              = player_1.get_transform().get_world_M();
	glm::mat4 p2_M              = player_2.get_transform().get_world_M();
	glm::mat4 p3_M              = player_3.get_transform().get_world_M();
	glm::mat4 p4_M              = player_4.get_transform().get_world_M();
	glm::mat4 p5_M              = player_5.get_transform().get_world_M();
	sg::AABB  p1_transformed_bd = player_1.get_component<sg::Mesh>().get_bounds().transform(p1_M);
	sg::AABB  p2_transformed_bd = player_2.get_component<sg::Mesh>().get_bounds().transform(p2_M);
	sg::AABB  p3_transformed_bd = player_3.get_component<sg::Mesh>().get_bounds().transform(p3_M);
	sg::AABB  p4_transformed_bd = player_4.get_component<sg::Mesh>().get_bounds().transform(p4_M);
	sg::AABB  p5_transformed_bd = player_5.get_component<sg::Mesh>().get_bounds().transform(p5_M);

	if (p1_transformed_bd.collides_with(p2_transformed_bd))
	{
		return p1_transformed_bd.collides_with(p2_transformed_bd);
	}
	else if (p1_transformed_bd.collides_with(p3_transformed_bd))
	{
		return p1_transformed_bd.collides_with(p3_transformed_bd);
	}
	else if (p1_transformed_bd.collides_with(p4_transformed_bd))
	{
		return p1_transformed_bd.collides_with(p4_transformed_bd);
	}
	else if (p1_transformed_bd.collides_with(p5_transformed_bd))
	{
		return p1_transformed_bd.collides_with(p5_transformed_bd);
	}
	else if (p2_transformed_bd.collides_with(p3_transformed_bd))
	{
		return p2_transformed_bd.collides_with(p3_transformed_bd);
	}
	else if (p2_transformed_bd.collides_with(p4_transformed_bd))
	{
		return p2_transformed_bd.collides_with(p4_transformed_bd);
	}
	else if (p2_transformed_bd.collides_with(p5_transformed_bd))
	{
		return p2_transformed_bd.collides_with(p5_transformed_bd);
	}
	else if (p3_transformed_bd.collides_with(p4_transformed_bd))
	{
		return p3_transformed_bd.collides_with(p4_transformed_bd);
	}
	else if (p3_transformed_bd.collides_with(p5_transformed_bd))
	{
		return p3_transformed_bd.collides_with(p5_transformed_bd);
	}
	else if (p4_transformed_bd.collides_with(p5_transformed_bd))
	{
		return p4_transformed_bd.collides_with(p5_transformed_bd);
	}
	else
	{
		return p1_transformed_bd.collides_with(p2_transformed_bd);
	}

	// NOTE THIS AABB FUNCTION DOES THE ACTUAL COLLISION TEST
	
}

std::string Controller::is_projectile_colliding()
{
	glm::mat4 p1_M              = player_1.get_transform().get_world_M();
	glm::mat4 p2_M              = player_2.get_transform().get_world_M();
	glm::mat4 p3_M              = player_3.get_transform().get_world_M();
	glm::mat4 p4_M              = player_4.get_transform().get_world_M();
	glm::mat4 p5_M              = player_5.get_transform().get_world_M();
	glm::mat4 proj_M            = projectile.get_transform().get_world_M();
	sg::AABB  p1_transformed_bd = player_1.get_component<sg::Mesh>().get_bounds().transform(p1_M);
	sg::AABB  p2_transformed_bd = player_2.get_component<sg::Mesh>().get_bounds().transform(p2_M);
	sg::AABB  p3_transformed_bd = player_3.get_component<sg::Mesh>().get_bounds().transform(p3_M);
	sg::AABB  p4_transformed_bd = player_4.get_component<sg::Mesh>().get_bounds().transform(p4_M);
	sg::AABB  p5_transformed_bd = player_5.get_component<sg::Mesh>().get_bounds().transform(p5_M);
	sg::AABB  proj_transformed_bd = player_5.get_component<sg::Mesh>().get_bounds().transform(proj_M);

	if (proj_transformed_bd.collides_with(p1_transformed_bd))
	{
		return "player_1";
	}
	else if (proj_transformed_bd.collides_with(p2_transformed_bd))
	{
		return "player_2";
	}
	else if (proj_transformed_bd.collides_with(p3_transformed_bd))
	{
		return "player_3";
	}
	else if (proj_transformed_bd.collides_with(p4_transformed_bd))
	{
		return "player_4";
	}
	else if (proj_transformed_bd.collides_with(p5_transformed_bd))
	{
		return "player_5";
	}
	else
	{
		return "";
	}
}

}        // namespace W3D
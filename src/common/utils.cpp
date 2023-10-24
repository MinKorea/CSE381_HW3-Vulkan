// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "utils.hpp"

// C/C++ LANGUAGE API TYPES
#include <sstream>

// OUR OWN TYPES
#include "core/device.hpp"
#include "scene_graph/components/camera.hpp"
#include "scene_graph/components/image.hpp"
#include "scene_graph/components/texture.hpp"
#include "scene_graph/node.hpp"
#include "scene_graph/scene.hpp"
#include "scene_graph/scripts/free_camera.hpp"

namespace W3D
{
std::string to_snake_case(const std::string &text)
{
	std::stringstream result;
	for (const auto c : text)
	{
		if (std::isalpha(c))
		{
			if (std::isspace(c))
			{
				result << "_";
			}
			else
			{
				if (std::isupper(c))
				{
					result << "_";
				}

				result << static_cast<char>(std::tolower(c));
			}
		}
		else
		{
			result << c;
		}
	}

	return result.str();
}

sg::Node *add_free_camera_script(sg::Scene &scene, const std::string &node_name, int width,
                                 int height)
{
	auto camera_node = scene.find_node(node_name);

	if (!camera_node)
	{
		camera_node = scene.find_node("default_camera");
	}

	if (!camera_node)
	{
		throw std::runtime_error("Unable to find a camera node!");
	}

	if (!camera_node->has_component<sg::Camera>())
	{
		throw std::runtime_error("No camera component found");
	}

	auto free_camera_script = std::make_unique<sg::FreeCamera>(*camera_node);

	free_camera_script->resize(width, height);
	scene.add_component_to_node(std::move(free_camera_script), *camera_node);

	return camera_node;
}

uint32_t max_mip_levels(uint32_t width, uint32_t height)
{
	uint32_t levels = 1;

	while (width != 1 && height != 1)
	{
		width  = std::max(width / 2, to_u32(1));
		height = std::max(height / 2, to_u32(1));
		levels++;
	}

	return levels;
}

}        // namespace W3D
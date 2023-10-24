#pragma once

#include "common/vk_common.hpp"
#include "vulkan_object.hpp"

namespace W3D
{
class Instance;
class Window;

class Surface : VulkanObject<vk::SurfaceKHR>
{
  public:
	Surface(Instance &instance, Window &window);
	~Surface();

  private:
	Instance &instance_;
};
}        // namespace W3D
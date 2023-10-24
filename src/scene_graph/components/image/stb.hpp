#pragma once

#include "scene_graph/components/image.hpp"

namespace W3D::sg
{
class Stb : public Image
{
  public:
	Stb(const std::string &name, const std::vector<uint8_t> &data);
	virtual ~Stb() = default;
};
}        // namespace W3D::sg
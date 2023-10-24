#pragma once

#include "common/glm_common.hpp"
#include "scene_graph/component.hpp"
#include <memory>

namespace vk
{
struct VertexInputAttributeDescription;
}

namespace W3D
{
class Device;
class Buffer;
namespace sg
{

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 uv;
	glm::vec4 joint;
	glm::vec4 weight;

	static std::array<vk::VertexInputAttributeDescription, 5> get_input_attr_descriptions();
};

class Material;
class SubMesh : public Component
{
  public:
	SubMesh(const std::string &name = "");

	virtual ~SubMesh();
	virtual std::type_index get_type() override;

	void set_material(const Material &material);

	const Material *get_material() const;

	std::uint32_t index_offset_ = 0;
	std::uint32_t vertex_count_ = 0;
	std::uint32_t idx_count_    = 0;

	std::unique_ptr<Buffer> p_vertex_buf_;
	std::unique_ptr<Buffer> p_idx_buf_;

  private:
	const Material *p_material_ = nullptr;
};

}        // namespace sg
}        // namespace W3D
#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;
class PipelineLayout;
class RenderPass;

/*
* This struct is used to describe the format of the vertex buffer's attributes.
*/
struct VertexInputState
{
	vk::ArrayProxy<vk::VertexInputAttributeDescription> attribute_descriptions;
	vk::ArrayProxy<vk::VertexInputBindingDescription>   binding_descriptions;
};

/*
* This struct describes how the vertices will be used, like what types of primitives
* are to be drawn.
*/
struct InputAssemblyState
{
	vk::PrimitiveTopology topology                 = vk::PrimitiveTopology::eTriangleList;
	vk::Bool32            primitive_restart_enable = false;
};

/*
* This struct describes the settings for the rasterization step, which we do not
* provide shader code for.
*/
struct RasterizationState
{
	vk::Bool32        depth_clamp_enable        = false;
	vk::Bool32        depth_bias_enable         = false;
	vk::Bool32        rasterizer_discard_enable = false;
	vk::PolygonMode   polygon_mode              = vk::PolygonMode::eFill;
	vk::CullModeFlags cull_mode                 = vk::CullModeFlagBits::eBack;
	vk::FrontFace     front_face                = vk::FrontFace::eCounterClockwise;
};

struct MultisampleState
{
	vk::SampleCountFlagBits rasterization_samples = vk::SampleCountFlagBits::e1;
};

/*
* This struct describes the settings for depth stenciling.
*/
struct DepthStencilState
{
	vk::Bool32    depth_test_enable        = true;
	vk::Bool32    depth_write_enable       = true;
	vk::CompareOp depth_compare_op         = vk::CompareOp::eLess;
	vk::Bool32    depth_bounds_test_enable = false;
	vk::Bool32    stencil_test_enable      = false;
};

/*
* This struct describes how, or if, we are employing fragment blending.
*/
struct ColorBlendAttachmentState
{
	vk::Bool32              blend_enable     = false;
	vk::ColorComponentFlags color_write_mask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
};

/*
* 
*/
struct ColorBlendState
{
	vk::Bool32  logic_op_enable = false;
	vk::LogicOp logic_op        = vk::LogicOp::eClear;
};

/*
* This struct specifies all the details regarding the pipeline to be created,
* including the names of the vertex and shader files. One of these must be
* filled in before constructing the pipeline as it is to be provided to the
* pipeline constructor.
*/
struct GraphicsPipelineState
{
	const char               *vert_shader_name;
	const char               *frag_shader_name;
	VertexInputState          vertex_input_state;
	InputAssemblyState        input_assembly_state;
	RasterizationState        rasterization_state;
	MultisampleState          multisample_state;
	DepthStencilState         depth_stencil_state;
	ColorBlendAttachmentState color_blend_attachment_state;
	ColorBlendState           color_blend_state;
};

/*
* Wrapper class for a Vulkan pipeline, i.e. vk::Pipeline.
*/
class GraphicsPipeline : public VulkanObject<vk::Pipeline>
{
  private:
	Device            &device_;
	vk::PipelineLayout pl_layout_;

  public:
	/*
	* The constructor does a complete will create the shader module to be associated with this pipeline so
	* keep in mind that it will
	*/
	GraphicsPipeline(Device &device, RenderPass &render_pass, GraphicsPipelineState &state, vk::PipelineLayoutCreateInfo &pl_layout_cinfo);
	GraphicsPipeline(GraphicsPipeline &&) = default;
	~GraphicsPipeline() override;

	/*
	 * Accessor method for getting this pipeline's layout.
	 */
	vk::PipelineLayout get_pipeline_layout();

	/*
	* This function creates a shader module
	*/
	vk::ShaderModule create_shader_module(const std::string &name);
};

}        // namespace W3D
// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "graphics_pipeline.hpp"

// OUR OWN TYPES
#include "common/file_utils.hpp"
#include "common/utils.hpp"
#include "device.hpp"
#include "pipeline_layout.hpp"
#include "render_pass.hpp"

namespace W3D
{

GraphicsPipeline::GraphicsPipeline(Device &device, RenderPass &render_pass, GraphicsPipelineState &state, vk::PipelineLayoutCreateInfo &pl_layout_cinfo) :
    device_(device)
{
	// LOAD AND CREATE THE VERTEX SHADER
	vk::ShaderModule vert_shader_module = create_shader_module(state.vert_shader_name);
	vk::PipelineShaderStageCreateInfo vert_stage_cinfo{
	    .stage  = vk::ShaderStageFlagBits::eVertex,
	    .module = vert_shader_module,
	    .pName  = "main",
	};

	// LOAD AND CREATE THE FRAGMENT SHADER
	vk::ShaderModule frag_shader_module = create_shader_module(state.frag_shader_name);
	vk::PipelineShaderStageCreateInfo frag_stage_cinfo{
	    .stage  = vk::ShaderStageFlagBits::eFragment,
	    .module = frag_shader_module,
	    .pName  = "main",
	};

	// NEXT WE NEED TO SPECIFY ALL THE RENDERING SETTINGS FOR THIS PIPELINE,
	// THIS WILL REQUIRE THE LOADING OF THOSE SETTINGS VIA OUR STRUCTS

	// OUR SHADERS COVER TWO STAGES IN RENDERING, VERTEX AND FRAGMENT SHADING
	std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages{vert_stage_cinfo, frag_stage_cinfo};

	// SPECIFY THE ATTRIBUTE DATA FORMATS WE PLAN TO USE
	vk::PipelineVertexInputStateCreateInfo vertex_input_cinfo{
	    .vertexBindingDescriptionCount   = to_u32(state.vertex_input_state.binding_descriptions.size()),
	    .pVertexBindingDescriptions      = state.vertex_input_state.binding_descriptions.data(),
	    .vertexAttributeDescriptionCount = to_u32(state.vertex_input_state.attribute_descriptions.size()),
	    .pVertexAttributeDescriptions    = state.vertex_input_state.attribute_descriptions.data(),
	};

	// SPECIFY THE TYPES OF PRIMITIVES WE PLAN TO USE (WE'LL USE A TRIANGLE LIST)
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_cinfo{
	    .topology               = state.input_assembly_state.topology,
	    .primitiveRestartEnable = state.input_assembly_state.primitive_restart_enable,
	};

	// VIEWPORT SETTINGS
	vk::PipelineViewportStateCreateInfo viewport_cinfo{
	    .viewportCount = 1,
	    .scissorCount  = 1,
	};

	// RASTERIZATION SETTINGS
	vk::PipelineRasterizationStateCreateInfo rasterization_cinfo{
	    .depthClampEnable        = state.rasterization_state.depth_clamp_enable,
	    .rasterizerDiscardEnable = state.rasterization_state.rasterizer_discard_enable,
	    .polygonMode             = state.rasterization_state.polygon_mode,
	    .cullMode                = state.rasterization_state.cull_mode,
	    .frontFace               = state.rasterization_state.front_face,
	    .depthBiasEnable         = state.rasterization_state.depth_bias_enable,
	    .lineWidth               = 1.0f,
	};

	// MULTISAMPLING SETTINGS
	vk::PipelineMultisampleStateCreateInfo multisample_cinfo{
	    .rasterizationSamples = state.multisample_state.rasterization_samples,
	    .sampleShadingEnable  = false,
	};

	// DEPTH STENCILING SETTINGS
	vk::PipelineDepthStencilStateCreateInfo depth_stencil_cinfo{
	    .depthTestEnable       = state.depth_stencil_state.depth_test_enable,
	    .depthWriteEnable      = state.depth_stencil_state.depth_write_enable,
	    .depthCompareOp        = state.depth_stencil_state.depth_compare_op,
	    .depthBoundsTestEnable = state.depth_stencil_state.depth_bounds_test_enable,
	    .stencilTestEnable     = state.depth_stencil_state.stencil_test_enable,
	};

	// COLOR BLEND SETTINGS
	vk::PipelineColorBlendAttachmentState color_blend_attachment_cinfo{
	    .blendEnable    = state.color_blend_attachment_state.blend_enable,
	    .colorWriteMask = state.color_blend_attachment_state.color_write_mask,
	};
	vk::PipelineColorBlendStateCreateInfo color_blending_cinfo{
	    .logicOpEnable   = state.color_blend_state.logic_op_enable,
	    .logicOp         = state.color_blend_state.logic_op,
	    .attachmentCount = 1,
	    .pAttachments    = &color_blend_attachment_cinfo,
	    .blendConstants  = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
	};


	std::vector<vk::DynamicState>      dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineDynamicStateCreateInfo dynamic_state_cinfo{
	    .dynamicStateCount = to_u32(dynamic_states.size()),
	    .pDynamicStates    = dynamic_states.data(),
	};

	// CREATE THE PIPELINE LAYOUT
	pl_layout_ = device_.get_handle().createPipelineLayout(pl_layout_cinfo);

	// AND NOW USE ALL OF THE ABOVE SETTINGS STRUCTS TO CREATE THE PIPELINE. FIRST
	// WE LOAD ALL THE SETTINGS INTO THIS OBJECT, WHICH WE'LL THEN USE
	vk::GraphicsPipelineCreateInfo graphics_pipeline_cinfo{
	    .stageCount          = to_u32(shader_stages.size()),
	    .pStages             = shader_stages.data(),
	    .pVertexInputState   = &vertex_input_cinfo,
	    .pInputAssemblyState = &input_assembly_cinfo,
	    .pViewportState      = &viewport_cinfo,
	    .pRasterizationState = &rasterization_cinfo,
	    .pMultisampleState   = &multisample_cinfo,
	    .pDepthStencilState  = &depth_stencil_cinfo,
	    .pColorBlendState    = &color_blending_cinfo,
	    .pDynamicState       = &dynamic_state_cinfo,
	    .layout              = pl_layout_,
	    .renderPass          = render_pass.get_handle(),
	    .subpass             = 0,
	};

	// NOW THAT WE HAVE PROVIDED ALL THE APPROPRIATE RENDERING SETTINGS
	// WE WISH TO USE IT'S TIME TO CREATE THE GRAPHIC PIPELINE FOR THIS DEVICE
	handle_ = device_.get_handle().createGraphicsPipeline(nullptr, graphics_pipeline_cinfo).value;

	// WE CAN NOW THROW AWAY THE SHADER MODULES AS THEY HAVE BEEN INCORPORATED INTO THE PIPELINE
	device_.get_handle().destroyShaderModule(vert_shader_module);
	device_.get_handle().destroyShaderModule(frag_shader_module);
}

GraphicsPipeline::~GraphicsPipeline()
{
	if (handle_)
	{
		// DESTROY THE PIPELINE AND ITS LAYOUT
		device_.get_handle().destroyPipelineLayout(pl_layout_);
		device_.get_handle().destroyPipeline(handle_);
	}
}

vk::PipelineLayout GraphicsPipeline::get_pipeline_layout()
{
	return pl_layout_;
}

vk::ShaderModule GraphicsPipeline::create_shader_module(const std::string &name)
{
	// LOAD ALL BYTES FROM THE SHADER FILE, WHICH SHOULD BE COMPILED BYTECODE
	std::vector<uint8_t>       binary = fu::read_shader_binary(name);

	// ASSIGN THE LOADED DATA TO OUR INFO OBJECT WHICH WE'LL USE NEXT TO CREATE THE SHADER
	vk::ShaderModuleCreateInfo shader_module_cinfo{
	    .codeSize = to_u32(binary.size()),
	    .pCode    = reinterpret_cast<const uint32_t *>(binary.data()),
	};

	// HAVE THE LOGIGAL DEVICE CREATE THE SHADER MODULE AND RETURN
	return device_.get_handle().createShaderModule(shader_module_cinfo);
}

}	// namespace W3D
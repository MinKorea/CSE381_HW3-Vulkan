// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "renderer.hpp"

// C/C++ LANGUAGE API TYPES
#include <queue>

// OUR OWN TYPES
#include "common/cvar.hpp"
#include "common/error.hpp"
#include "common/file_utils.hpp"
#include "common/logging.hpp"
#include "common/utils.hpp"
#include "controller.hpp"
#include "core/command_pool.hpp"
#include "core/descriptor_allocator.hpp"
#include "core/device.hpp"
#include "core/framebuffer.hpp"
#include "core/graphics_pipeline.hpp"
#include "core/image_resource.hpp"
#include "core/image_view.hpp"
#include "core/instance.hpp"
#include "core/physical_device.hpp"
#include "core/render_pass.hpp"
#include "core/swapchain.hpp"
#include "core/window.hpp"
#include "gltf_loader.hpp"
#include "scene_graph/components/camera.hpp"
#include "scene_graph/components/mesh.hpp"
#include "scene_graph/components/pbr_material.hpp"
#include "scene_graph/components/sampler.hpp"
#include "scene_graph/components/submesh.hpp"
#include "scene_graph/components/texture.hpp"
#include "scene_graph/scene.hpp"
#include "scene_graph/script.hpp"
#include "scene_graph/scripts/free_camera.hpp"
#include "scene_graph/scripts/player.hpp"

namespace W3D
{
// SCENE LIGHTS
// THESE ARE THE LIGHTS WE WILL PUT INTO OUR SCENE, NOTE EACH
// HAS A UNIQUE LOCATION IN THE SCENE. NOTE WE ARE USING glm
// VECTORS TO REPRESENT POSITION
const uint32_t Renderer::NUM_INFLIGHT_FRAMES  = 2;
const uint32_t Renderer::IRRADIANCE_DIMENSION = 2;
const int      NUM_LIGHTS                     = 4;
glm::vec3      LIGHT_POSITIONS[NUM_LIGHTS]    = {
    glm::vec3(6.0f, 0.0f, 6.0f),
    glm::vec3(-3.0f, 0.0f, 6.0f),
    glm::vec3(0.0f, -6.0f, -6.0f),
    glm::vec3(-6.0f, -6.0f, -6.0f),
};

Renderer::Renderer()
{
	// CREATE OUR WINDOW AND SETUP THE EVENT HANDLERS
	p_window_ = std::make_unique<Window>("Wolfie3D");
	p_window_->register_callbacks(*this);

	// SETUP RENDERING WITH VULKAN, WE'LL NEED A VULKAN INSTANCE AND THROUGH
	// THAT WE CAN INITIALIZE OUR PHYSICAL DEVICE, i.e. THE GPU
	p_instance_         = std::make_unique<Instance>("Wolfie3D", *p_window_);
	p_physical_device_  = p_instance_->pick_physical_device();
	p_device_           = std::make_unique<Device>(*p_instance_, *p_physical_device_);
	p_descriptor_state_ = std::make_unique<DescriptorState>(*p_device_);
	p_cmd_pool_         = std::make_unique<CommandPool>(*p_device_, p_device_->get_graphics_queue(), p_physical_device_->get_graphics_queue_family_index());
	p_swapchain_        = std::make_unique<Swapchain>(*p_device_, p_window_->get_extent());

	// OUR SCENE WILL USE THIS GLTF FILE, WHICH IS JUST A TEXTURED CUBE
	load_scene("2.0/BoxTextured/glTF/HW.gltf");

	// SETUP THE RENDERING RESOURCES
	PBRBaker baker(*p_device_);
	baked_pbr_ = baker.bake();
	create_rendering_resources();
	p_sframe_buffer_ = std::make_unique<SwapchainFramebuffer>(*p_device_, *p_swapchain_, *p_render_pass_);
	create_controller();
}

// DESTRUCTOR, NOTHING TO CLEAN UP
Renderer::~Renderer(){};

void Renderer::start()
{
	main_loop();
	timer_.start();
}

void Renderer::main_loop()
{
	// RUN UNTIL THE USER CLOSES THE APPLICTION WINDOW
	while (!p_window_->should_close())
	{
		// INCREMENT THE TIMER
		timer_.tick();

		// DRAW THE SCENE
		render_frame();

		// UPDATE SCENE OBJECTS
		update();

		// RETRIEVE USER INPUT
		p_window_->poll_events();
	}

	// RELEASE GPU
	p_device_->get_handle().waitIdle();
}

void Renderer::update()
{
	// ADVANCE THE TIMER
	double delta_time = timer_.tick();

	// THESE ARE ALL THE SCENE SCRIPTS, ONE FOR EACH UPDATABLE ITEM
	std::vector<sg::Script *> p_scripts = p_scene_->get_components<sg::Script>();

	// GO THROUGH ALL THE SCRIPTS
	for (sg::Script *p_script : p_scripts)
	{
		// UPDATE THE SCENE OBJECT VIA ITS SCRIPT
		p_script->update(delta_time);
	}
}

void Renderer::process_event(const Event &event)
{
	if (event.type == EventType::eResize)
	{
		is_window_resized_ = true;
	}
	else
	{
		p_controller_->process_event(event);
	}
}

void Renderer::load_scene(const char *scene_name)
{
	GLTFLoader loader(*p_device_);
	p_scene_                   = loader.read_scene_from_file(scene_name);
	vk::Extent2D window_extent = p_window_->get_extent();
	p_camera_node_             = add_free_camera_script(*p_scene_, "main_camera", window_extent.width, window_extent.height);
	p_camera_node_->get_component<sg::Transform>().set_tranlsation(glm::vec3(0.0f, 0.0f, 5.0f));
}

void Renderer::create_controller()
{
	p_controller_ = std::make_unique<Controller>(*p_camera_node_, add_player_script("player_1"), add_player_script("player_2"));
}

void Renderer::render_frame()
{
	uint32_t img_idx = sync_acquire_next_image();
	record_draw_commands(img_idx);
	sync_submit_commands();
	sync_present(img_idx);
	frame_idx_ = (frame_idx_ + 1) % NUM_INFLIGHT_FRAMES;
}

uint32_t Renderer::sync_acquire_next_image()
{
	FrameResource &frame    = get_current_frame_resource();
	vk::Device     device_h = p_device_->get_handle();
	uint32_t       img_idx;

	while (true)
	{
		while (vk::Result::eTimeout ==
		       device_h.waitForFences({frame.in_flight_fence.get_handle()}, true, UINT64_MAX))
		{
			;
		}

		// Opted for plain vkAacquireNextImageKHR here because we want to deal with the error ourselves.
		// Otherwise, vulkan.hpp would've thrown the error and we have to catch it (slow).
		// See, https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
		vk::Result acquired_res = static_cast<vk::Result>(vkAcquireNextImageKHR(device_h, p_swapchain_->get_handle(), UINT64_MAX, frame.image_avaliable_semaphore.get_handle(), VK_NULL_HANDLE, &img_idx));

		if (acquired_res == vk::Result::eErrorOutOfDateKHR)
		{
			resize();
		}
		else if (acquired_res != vk::Result::eSuccess && acquired_res != vk::Result::eSuboptimalKHR)
		{
			LOGE("failed to acquire swapchain image!");
			abort();
		}
		else
		{
			break;
		}
	}

	device_h.resetFences(frame.in_flight_fence.get_handle());

	return img_idx;
};

void Renderer::sync_submit_commands()
{
	FrameResource         &frame       = get_current_frame_resource();
	vk::PipelineStageFlags wait_stages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo         submit_info{
	            .waitSemaphoreCount   = 1,
	            .pWaitSemaphores      = &frame.image_avaliable_semaphore.get_handle(),
	            .pWaitDstStageMask    = &wait_stages,
	            .commandBufferCount   = 1,
	            .pCommandBuffers      = &frame.cmd_buf.get_handle(),
	            .signalSemaphoreCount = 1,
	            .pSignalSemaphores    = &frame.render_finished_semaphore.get_handle(),
    };
	p_device_->get_graphics_queue().submit(submit_info, frame.in_flight_fence.get_handle());
}

void Renderer::sync_present(uint32_t img_idx)
{
	FrameResource     &frame = get_current_frame_resource();
	vk::PresentInfoKHR present_info{
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores    = &frame.render_finished_semaphore.get_handle(),
	    .swapchainCount     = 1,
	    .pSwapchains        = &p_swapchain_->get_handle(),
	    .pImageIndices      = &img_idx,
	};

	// Same reasoing as sync_acquire.
	// See, https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
	vk::Result present_res = static_cast<vk::Result>(
	    vkQueuePresentKHR(p_device_->get_present_queue(), reinterpret_cast<VkPresentInfoKHR *>(&present_info)));

	if (present_res == vk::Result::eErrorOutOfDateKHR || present_res == vk::Result::eSuboptimalKHR || is_window_resized_)
	{
		is_window_resized_ = false;
		resize();
	}
	else if (present_res != vk::Result::eSuccess)
	{
		LOGE("Failed to present swapchain image");
		abort();
	}
}

void Renderer::resize()
{
	vk::Extent2D extent = p_window_->wait_for_non_zero_extent();
	p_device_->get_handle().waitIdle();
	p_camera_node_->get_component<sg::Script>().resize(extent.width, extent.height);
	p_swapchain_->rebuild(extent);
	p_sframe_buffer_->rebuild();
}

void Renderer::record_draw_commands(uint32_t img_idx)
{
	CommandBuffer &cmd_buf = get_current_frame_resource().cmd_buf;
	cmd_buf.reset();
	cmd_buf.begin();
	update_frame_ubo();
	set_dynamic_states(cmd_buf);
	begin_render_pass(cmd_buf, p_sframe_buffer_->get_handle(img_idx));
	draw_skybox(cmd_buf);
	draw_lights(cmd_buf);
	draw_scene(cmd_buf);
	cmd_buf.get_handle().endRenderPass();
	cmd_buf.get_handle().end();
}

void Renderer::update_frame_ubo()
{
	sg::Camera &camera    = p_camera_node_->get_component<sg::Camera>();
	glm::mat4   proj_view = camera.get_projection() * camera.get_view();

	UBO ubo{
	    .proj_view = proj_view,
	    .lights    = {
            glm::vec4(LIGHT_POSITIONS[0], 1.0f),
            glm::vec4(LIGHT_POSITIONS[1], 1.0f),
            glm::vec4(LIGHT_POSITIONS[2], 1.0f),
            glm::vec4(LIGHT_POSITIONS[3], 1.0f),
        },
	};

	get_current_frame_resource().blinn_phong_uni_buf.update(&ubo, sizeof(ubo));
	get_current_frame_resource().light_uni_buf.update(&proj_view, sizeof(proj_view));
}

void Renderer::set_dynamic_states(CommandBuffer &cmd_buf)
{
	vk::Extent2D swapchain_extent = p_swapchain_->get_swapchain_properties().extent;
	vk::Viewport viewport{
	    .x        = 0,
	    .y        = 0,
	    .width    = static_cast<float>(swapchain_extent.width),
	    .height   = static_cast<float>(swapchain_extent.height),
	    .minDepth = 0.0f,
	    .maxDepth = 1.0f,
	};
	vk::Rect2D scissor{
	    .offset = {
	        .x = 0,
	        .y = 0,
	    },
	    .extent = swapchain_extent,
	};

	cmd_buf.get_handle().setViewport(0, viewport);
	cmd_buf.get_handle().setScissor(0, scissor);
}

void Renderer::begin_render_pass(CommandBuffer &cmd_buf, vk::Framebuffer framebuffer)
{
	std::array<vk::ClearValue, 2> clear_values{
	    std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
	};
	clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

	vk::RenderPassBeginInfo render_pass_binfo{
	    .renderPass  = p_render_pass_->get_handle(),
	    .framebuffer = framebuffer,
	    .renderArea  = {
	         .offset = {
	             .x = 0,
	             .y = 0,
            },
	         .extent = p_swapchain_->get_swapchain_properties().extent,
        },
	    .clearValueCount = clear_values.size(),
	    .pClearValues    = clear_values.data(),
	};

	cmd_buf.get_handle().beginRenderPass(render_pass_binfo, vk::SubpassContents::eInline);
}

void Renderer::draw_skybox(CommandBuffer &cmd_buf)
{
	sg::Camera    &camera = p_camera_node_->get_component<sg::Camera>();
	FrameResource &frame  = get_current_frame_resource();
	SkyboxPCO      pco{
	         .proj = camera.get_projection(),
	         .view = camera.get_view(),
    };
	cmd_buf.get_handle().bindPipeline(
	    vk::PipelineBindPoint::eGraphics,
	    skybox_.p_pl->get_handle());
	cmd_buf.get_handle().bindDescriptorSets(
	    vk::PipelineBindPoint::eGraphics,
	    skybox_.p_pl->get_pipeline_layout(),
	    0,
	    frame.skybox_set,
	    {});
	cmd_buf.get_handle().pushConstants<SkyboxPCO>(
	    skybox_.p_pl->get_pipeline_layout(),
	    vk::ShaderStageFlagBits::eVertex,
	    0,
	    pco);
	draw_submesh(cmd_buf, *baked_pbr_.p_box);
}        // namespace W3D

void Renderer::draw_lights(CommandBuffer &cmd_buf)
{
	vk::PipelineLayout pl_layout = light_.p_pl->get_pipeline_layout();
	cmd_buf.get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, light_.p_pl->get_handle());
	cmd_buf.get_handle().bindDescriptorSets(
	    vk::PipelineBindPoint::eGraphics,
	    pl_layout,
	    0,
	    get_current_frame_resource().light_set,
	    {});

	glm::mat4 scaled_m = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		glm::mat4 world_m = glm::translate(scaled_m, LIGHT_POSITIONS[i]);

		cmd_buf.get_handle().pushConstants<glm::mat4>(pl_layout, vk::ShaderStageFlagBits::eVertex, 0, world_m);

		draw_submesh(cmd_buf, *baked_pbr_.p_box);
	}
}

void Renderer::draw_scene(CommandBuffer &cmd_buf)
{
	vk::PipelineLayout pl_layout = blinn_phong_.p_pl->get_pipeline_layout();
	cmd_buf.get_handle().bindPipeline(
	    vk::PipelineBindPoint::eGraphics,
	    blinn_phong_.p_pl->get_handle());
	cmd_buf.get_handle().bindDescriptorSets(
	    vk::PipelineBindPoint::eGraphics,
	    pl_layout,
	    0,
	    get_current_frame_resource().blinn_phong_set,
	    {});

	std::queue<sg::Node *> p_nodes;
	p_nodes.push(&p_scene_->get_root_node());
	while (!p_nodes.empty())
	{
		sg::Node *p_node = p_nodes.front();
		p_nodes.pop();

		if (p_node->has_component<sg::Mesh>())
		{
			push_node_model_matrix(cmd_buf, p_node);
			std::vector<sg::SubMesh *> p_submeshs = p_node->get_component<sg::Mesh>().get_p_submeshs();
			for (sg::SubMesh *p_submesh : p_submeshs)
			{
				const sg::PBRMaterial *p_pbr_material = dynamic_cast<const sg::PBRMaterial *>(p_submesh->get_material());
				bind_material(cmd_buf, *p_pbr_material);
				draw_submesh(cmd_buf, *p_submesh);
			}
		}

		std::vector<sg::Node *> p_children = p_node->get_children();
		for (sg::Node *p_child : p_children)
		{
			p_nodes.push(p_child);
		}
	}
}

void Renderer::push_node_model_matrix(CommandBuffer &cmd_buf, sg::Node *p_node)
{
	BlinnPhongPCO pco{
	    .model        = p_node->get_component<sg::Transform>().get_world_M(),
	    .cam_pos      = p_camera_node_->get_component<sg::Transform>().get_translation(),
	    .is_colliding = p_controller_->are_players_colliding(),
	};
	cmd_buf.get_handle().pushConstants<BlinnPhongPCO>(blinn_phong_.p_pl->get_pipeline_layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, pco);
}        // namespace W3D

void Renderer::bind_material(CommandBuffer &cmd_buf, const sg::PBRMaterial &material)
{
	cmd_buf.get_handle().bindDescriptorSets(
	    vk::PipelineBindPoint::eGraphics,
	    blinn_phong_.p_pl->get_pipeline_layout(),
	    1,
	    material.set,
	    {});
}

void Renderer::draw_submesh(CommandBuffer &cmd_buf, sg::SubMesh &submesh)
{
	cmd_buf.get_handle().bindVertexBuffers(0, submesh.p_vertex_buf_->get_handle(), {0});
	cmd_buf.get_handle().bindIndexBuffer(submesh.p_idx_buf_->get_handle(), 0, vk::IndexType::eUint32);
	cmd_buf.get_handle().drawIndexed(submesh.idx_count_, 1, 0, 0, 0);
}

Renderer::FrameResource &Renderer::get_current_frame_resource()
{
	return frame_resources_[frame_idx_];
};

sg::Node &Renderer::add_player_script(const char *node_name)
{
	sg::Node *p_node = p_scene_->find_node(node_name);
	if (!p_node)
	{
		LOGE("Cannot find node {}", node_name);
		abort();
	};

	std::unique_ptr<sg::Player> p_script = std::make_unique<sg::Player>(*p_node);
	p_scene_->add_component_to_node(std::move(p_script), *p_node);

	return *p_node;
}

void Renderer::create_rendering_resources()
{
	create_frame_resources();
	create_descriptor_resources();
	create_render_pass();
	create_pipeline_resources();
}

void Renderer::create_frame_resources()
{
	const DeviceMemoryAllocator &allocator = p_device_->get_device_memory_allocator();
	for (uint32_t i = 0; i < NUM_INFLIGHT_FRAMES; i++)
	{
		frame_resources_.push_back({
		    .cmd_buf                   = std::move(p_cmd_pool_->allocate_command_buffer()),
		    .blinn_phong_uni_buf       = std::move(allocator.allocate_uniform_buffer(sizeof(UBO))),
		    .light_uni_buf             = std::move(allocator.allocate_uniform_buffer(sizeof(glm::mat4))),
		    .image_avaliable_semaphore = std::move(Semaphore(*p_device_)),
		    .render_finished_semaphore = std::move(Semaphore(*p_device_)),
		    .in_flight_fence           = std::move(Fence(*p_device_, vk::FenceCreateFlagBits::eSignaled)),
		});
	}
}

void Renderer::create_descriptor_resources()
{
	create_skybox_desc_resources();
	create_blinn_phong_desc_resources();
	create_light_desc_resources();
	create_materials_desc_resources();
}

void Renderer::create_blinn_phong_desc_resources()
{
	vk::DescriptorBufferInfo ubo_dinfo{
	    .offset = 0,
	    .range  = sizeof(UBO),
	};
	for (uint32_t i = 0; i < NUM_INFLIGHT_FRAMES; i++)
	{
		ubo_dinfo.buffer = frame_resources_[i].blinn_phong_uni_buf.get_handle();

		DescriptorAllocation desc_allocation = DescriptorBuilder::begin(p_descriptor_state_->cache, p_descriptor_state_->allocator).bind_buffer(0, ubo_dinfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment).build();

		blinn_phong_.desc_layout_ring[DescriptorRingAccessor::eGlobal] = desc_allocation.set_layout;
		frame_resources_[i].blinn_phong_set                            = desc_allocation.set;
	}
};

void Renderer::create_light_desc_resources()
{
	vk::DescriptorBufferInfo ubo_dinfo{
	    .offset = 0,
	    .range  = sizeof(glm::mat4),
	};

	for (uint32_t i = 0; i < NUM_INFLIGHT_FRAMES; i++)
	{
		ubo_dinfo.buffer = frame_resources_[i].light_uni_buf.get_handle();

		DescriptorAllocation desc_allocation =
		    DescriptorBuilder::begin(p_descriptor_state_->cache, p_descriptor_state_->allocator)
		        .bind_buffer(0, ubo_dinfo, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
		        .build();

		light_.desc_layout_ring[DescriptorRingAccessor::eGlobal] = desc_allocation.set_layout;
		frame_resources_[i].light_set                            = desc_allocation.set;
	}
}

void Renderer::create_skybox_desc_resources()
{
	vk::DescriptorImageInfo background{
	    .sampler     = baked_pbr_.p_irradiance->sampler.get_handle(),
	    .imageView   = baked_pbr_.p_irradiance->resource.get_view().get_handle(),
	    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
	};

	for (uint32_t i = 0; i < NUM_INFLIGHT_FRAMES; i++)
	{
		DescriptorAllocation skybox_allocation =
		    DescriptorBuilder::begin(p_descriptor_state_->cache, p_descriptor_state_->allocator)
		        .bind_image(0, background, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
		        .build();

		frame_resources_[i].skybox_set                            = skybox_allocation.set;
		skybox_.desc_layout_ring[DescriptorRingAccessor::eGlobal] = skybox_allocation.set_layout;
	}
}

void Renderer::create_materials_desc_resources()
{
	static const std::vector<std::string> pbr_texture_names = {
	    "base_color_texture",
	    "normal_texture",
	    "occlusion_texture",
	    "emissive_texture",
	    "metallic_roughness_texture",
	};
	sg::Texture *p_default_texture = p_scene_->find_component<sg::Texture>("default_texture");

	std::vector<sg::PBRMaterial *> p_materials = p_scene_->get_components<sg::PBRMaterial>();
	for (sg::PBRMaterial *p_material : p_materials)
	{
		DescriptorBuilder builder =
		    DescriptorBuilder::begin(p_descriptor_state_->cache, p_descriptor_state_->allocator);

		std::vector<vk::DescriptorImageInfo> desc_iinfos;
		desc_iinfos.reserve(pbr_texture_names.size());

		for (int i = 0; i < pbr_texture_names.size(); i++)
		{
			const std::string &name      = pbr_texture_names[i];
			sg::Texture       *p_texture = p_default_texture;
			auto               it        = p_material->texture_map_.find(name);
			if (it != p_material->texture_map_.end())
			{
				p_texture = it->second;
			}
			desc_iinfos.emplace_back(vk::DescriptorImageInfo{
			    .sampler     = p_texture->p_sampler_->get_handle(),
			    .imageView   = p_texture->p_resource_->get_view().get_handle(),
			    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
			});
			builder.bind_image(i, desc_iinfos.back(), vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
		}

		DescriptorAllocation desc_allocation = builder.build();

		p_material->set                                                  = desc_allocation.set;
		blinn_phong_.desc_layout_ring[DescriptorRingAccessor::eMaterial] = desc_allocation.set_layout;
	}
}

void Renderer::create_render_pass()
{
	std::array<vk::AttachmentDescription, 2> attachemnts;

	attachemnts[0] = RenderPass::color_attachment(p_swapchain_->get_swapchain_properties().surface_format.format, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
	vk::AttachmentReference color_attachment_ref{
	    .attachment = 0,
	    .layout     = vk::ImageLayout::eColorAttachmentOptimal,
	};

	attachemnts[1] = RenderPass::depth_attachment(p_swapchain_->choose_depth_format(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depth_attachemnt_ref{
	    .attachment = 1,
	    .layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal,
	};

	vk::SubpassDescription subpass{
	    .pipelineBindPoint       = vk::PipelineBindPoint::eGraphics,
	    .colorAttachmentCount    = 1,
	    .pColorAttachments       = &color_attachment_ref,
	    .pDepthStencilAttachment = &depth_attachemnt_ref,
	};

	vk::SubpassDependency dependency{
	    .srcSubpass   = VK_SUBPASS_EXTERNAL,
	    .dstSubpass   = 0,
	    .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
	                    vk::PipelineStageFlagBits::eEarlyFragmentTests,
	    .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
	                    vk::PipelineStageFlagBits::eEarlyFragmentTests,
	    .srcAccessMask = {},
	    .dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
	                     vk::AccessFlagBits::eColorAttachmentWrite |
	                     vk::AccessFlagBits::eDepthStencilAttachmentWrite,
	};

	vk::RenderPassCreateInfo render_pass_cinfo{
	    .attachmentCount = 2,
	    .pAttachments    = attachemnts.data(),
	    .subpassCount    = 1,
	    .pSubpasses      = &subpass,
	    .dependencyCount = 1,
	    .pDependencies   = &dependency,
	};

	p_render_pass_ = std::make_unique<RenderPass>(*p_device_, render_pass_cinfo);
}

void Renderer::create_pipeline_resources()
{
	std::array<vk::VertexInputBindingDescription, 1> binding_descriptions;
	binding_descriptions[0] = vk::VertexInputBindingDescription{
	    .binding   = 0,
	    .stride    = sizeof(sg::Vertex),
	    .inputRate = vk::VertexInputRate::eVertex,
	};
	GraphicsPipelineState pl_state{
	    .vert_shader_name   = "blinn_phong.vert.spv",
	    .frag_shader_name   = "blinn_phong.frag.spv",
	    .vertex_input_state = {
	        .attribute_descriptions = sg::Vertex::get_input_attr_descriptions(),
	        .binding_descriptions   = binding_descriptions,
	    },
	};

	vk::PushConstantRange blinn_phong_push_const_range{
	    .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
	    .offset     = 0,
	    .size       = sizeof(BlinnPhongPCO),
	};

	vk::PipelineLayoutCreateInfo blinn_phong_pl_layout_cinfo{
	    .setLayoutCount         = 2,
	    .pSetLayouts            = blinn_phong_.desc_layout_ring.data(),
	    .pushConstantRangeCount = 1,
	    .pPushConstantRanges    = &blinn_phong_push_const_range,
	};

	blinn_phong_.p_pl = std::make_unique<GraphicsPipeline>(*p_device_, *p_render_pass_, pl_state, blinn_phong_pl_layout_cinfo);

	pl_state.vert_shader_name = "lights.vert.spv";
	pl_state.frag_shader_name = "lights.frag.spv";

	vk::PushConstantRange light_push_const_range{
	    .stageFlags = vk::ShaderStageFlagBits::eVertex,
	    .offset     = 0,
	    .size       = sizeof(glm::mat4),
	};

	vk::PipelineLayoutCreateInfo light_pl_layout_cinfo{
	    .setLayoutCount         = 1,
	    .pSetLayouts            = light_.desc_layout_ring.data(),
	    .pushConstantRangeCount = 1,
	    .pPushConstantRanges    = &light_push_const_range,
	};

	light_.p_pl = std::make_unique<GraphicsPipeline>(*p_device_, *p_render_pass_, pl_state, light_pl_layout_cinfo);

	vk::PushConstantRange skybox_push_const_range{
	    .stageFlags = vk::ShaderStageFlagBits::eVertex,
	    .offset     = 0,
	    .size       = sizeof(SkyboxPCO),
	};
	vk::PipelineLayoutCreateInfo skybox_pl_layout_cinfo{
	    .setLayoutCount         = 1,
	    .pSetLayouts            = skybox_.desc_layout_ring.data(),
	    .pushConstantRangeCount = 1,
	    .pPushConstantRanges    = &skybox_push_const_range,
	};
	pl_state.vert_shader_name                       = "skybox.vert.spv";
	pl_state.frag_shader_name                       = "skybox.frag.spv";
	pl_state.rasterization_state.cull_mode          = vk::CullModeFlagBits::eFront;
	pl_state.depth_stencil_state.depth_test_enable  = false;
	pl_state.depth_stencil_state.depth_write_enable = false;
	skybox_.p_pl                                    = std::make_unique<GraphicsPipeline>(*p_device_, *p_render_pass_, pl_state, skybox_pl_layout_cinfo);
}

}        // namespace W3D

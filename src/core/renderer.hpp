#pragma once

#include "common/timer.hpp"
#include "common/vk_common.hpp"
#include "command_buffer.hpp"
#include "core/image_resource.hpp"
#include "core/sampler.hpp"
#include "device_memory/buffer.hpp"
#include "pbr_baker.hpp"
#include "sync_objects.hpp"

/*
* renderer.hpp - This is the header file for the Renderer class type and structs it uses
* for this demo. Note that this Wolfie3D demo is just a renderer so this class is rather
* like a game class you might find in a game engine, it has references to the important
* objects (like window, instance, etc.) and everything needed for rendering. Note that it
* also has some important big-picture methods, like:
	start()
	process_event()
    main_loop()
    update()
    render_frame()
	load_scene()
	...
*/

/*
* namespaces are a way to make sure there is no collision between class names. We can define
* our own named classes and scope them to W3D, for example, W3D::Renderer, this way if one of
* the APIs we are using also has a Renderer class there is no confusion for the compiler.
*/
namespace W3D
{

namespace sg
{
class PBRMaterial;
class Texture;
class Camera;
}        // namespace sg

class Window;
class Instance;
class PhysicalDevice;
class Device;
class Swapchain;
class RenderPass;
class SwapchainFramebuffer;
class PipelineResource;
class Controller;

struct DescriptorState;
struct Event;

class Renderer
{
  private:
	// LIGHTING PROPERTIES
	static const uint32_t NUM_INFLIGHT_FRAMES;
	static const uint32_t IRRADIANCE_DIMENSION;

	// EVERYTHING NEEDED TO RENDER A FRAME
	struct FrameResource
	{
		CommandBuffer     cmd_buf;
		Buffer            blinn_phong_uni_buf;
		Buffer            light_uni_buf;
		Semaphore         image_avaliable_semaphore;
		Semaphore         render_finished_semaphore;
		Fence             in_flight_fence;
		vk::DescriptorSet blinn_phong_set;
		vk::DescriptorSet light_set;
		vk::DescriptorSet skybox_set;
	};

	struct PipelineResource
	{
		std::unique_ptr<GraphicsPipeline>      p_pl;
		std::array<vk::DescriptorSetLayout, 4> desc_layout_ring;
	};

	enum DescriptorRingAccessor
	{
		eGlobal   = 0,
		eMaterial = 1,
	};

	struct UBO
	{
		glm::mat4 proj_view;
		glm::vec4 lights[4];
	};

	struct BlinnPhongPCO
	{
		glm::mat4 model;
		alignas(16) glm::vec3 cam_pos;
		alignas(16) int is_colliding;
	};

	struct SkyboxPCO
	{
		glm::mat4 proj;
		glm::mat4 view;
	};

	// THESE ARE ALL THE MAJOR SUBSYSTEMS, INCLUDING THE Vulkan STUFF
	std::unique_ptr<Window>               p_window_;
	std::unique_ptr<Instance>             p_instance_;
	std::unique_ptr<PhysicalDevice>       p_physical_device_;
	std::unique_ptr<Device>               p_device_;
	std::unique_ptr<Swapchain>            p_swapchain_;
	std::unique_ptr<RenderPass>           p_render_pass_;
	std::unique_ptr<SwapchainFramebuffer> p_sframe_buffer_;
	std::unique_ptr<DescriptorState>      p_descriptor_state_;
	std::unique_ptr<CommandPool>          p_cmd_pool_;
	std::unique_ptr<sg::Scene>            p_scene_;
	sg::Node                             *p_camera_node_ = nullptr;
	std::unique_ptr<Controller>           p_controller_;

	Timer                      timer_;
	uint32_t                   frame_idx_ = 0;
	std::vector<FrameResource> frame_resources_;
	PipelineResource           skybox_;
	PipelineResource           blinn_phong_;
	PipelineResource           light_;
	PBR                        baked_pbr_;
	bool                       is_window_resized_ = false;

  public:
	/*
	 * This constructor initializes everything needed for rendering and running the demo
	 * including Vulkan as well as our scene.
	 */
	Renderer();

	/*
	* The destructor has nothing to clean up.
	*/
	~Renderer();

	/*
	 * This is called when the application is started, it simply starts the
	 * renderer's loop and starts the timer.
	 */
	void start();

	/*
	 * This runs the application's main loop, which each frame
	 * must advance the timer, draw the scene, update the scene, and get
	 * user input.
	 */
	void main_loop();

	/*
	* This responds to event input, which it forwards to the appropriate scene scripts.
	*/
	void process_event(const Event &event);

	/*
	* Called once per frame, it updates all scene objects.
	*/
	void update();

	/*
	* Called once per frame, it is the source of all scene rendering, note it 
	* relies on the helper methods to render specific things.
	*/
	void render_frame();

	uint32_t sync_acquire_next_image();
	void     sync_submit_commands();
	void     sync_present(uint32_t img_idx);
	void     record_draw_commands(uint32_t img_idx);

	void update_frame_ubo();
	void set_dynamic_states(CommandBuffer &cmd_buf);
	void begin_render_pass(CommandBuffer &cmd_buf, vk::Framebuffer framebuffer);
	void draw_skybox(CommandBuffer &cmd_buf);
	void draw_lights(CommandBuffer &cmd_buf);
	void draw_scene(CommandBuffer &cmd_buf);
	void draw_submesh(CommandBuffer &cmd_buf, sg::SubMesh &submesh);
	void bind_material(CommandBuffer &cmd_buf, const sg::PBRMaterial &material);
	void push_node_model_matrix(CommandBuffer &cmd_buf, sg::Node *p_node);

	void           resize();
	FrameResource &get_current_frame_resource();

	void load_scene(const char *scene_name);
	void create_controller();
	void create_rendering_resources();
	void create_frame_resources();
	void create_descriptor_resources();
	void create_skybox_desc_resources();
	void create_blinn_phong_desc_resources();
	void create_light_desc_resources();
	void create_materials_desc_resources();
	void create_render_pass();
	void create_pipeline_resources();

	sg::Node &add_player_script(const char *node_name);
};
}        // namespace W3D
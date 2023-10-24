#pragma once

#include <tiny_gltf.h>
#include <memory>
#include "common/glm_common.hpp"

namespace W3D
{
class Device;

namespace DeviceMemory
{
class Allocator;
}

namespace sg
{
class Scene;
class Node;
class Camera;
class Image;
class Mesh;
class SubMesh;
class PBRMaterial;
class Sampler;
class Texture;
struct Vertex;
};	// namespace sg

struct ImageTransferInfo;

/*
* This class is responsible for loading 3D models and scenes for our application. Note
* these things are stored in a GLTF format, which is a JSON format managed by the Khronos
* group that can be used to desribe single 3D objects or collections of 3D objects.
*/
class GLTFLoader
{
  private:
	const Device                  &device_;
	sg::Scene                     *p_scene_;
	tinygltf::Model                gltf_model_;
	std::string                    model_path_;
	std::vector<ImageTransferInfo> img_tinfos_;

  public:
	/*
	* Constructor just sets the device, nothing gets loaded at this time.
	*/
	GLTFLoader(Device const &device);

	/*
	* This class is not responsible for the device or the scene, it is just for loading
	* data so it has nothing to destroy.
	*/
	virtual ~GLTFLoader() = default;

	/*
	* This function loads the mesh data from file_name, and returns it as a SubMesh object.
	*/
	std::unique_ptr<sg::SubMesh> read_model_from_file(const std::string &file_name, int mesh_idx);

	/*
	* This function employs the TinyGLTF library to load the contents of a GLTF file
	* as a single model.
	*/
	void load_gltf_model(const std::string &file_name);

	/*
	 * This function loads the scene data from file_name and returns it as a Scene object.
	 */
	std::unique_ptr<sg::Scene> read_scene_from_file(const std::string &file_name,
	                                                int                scene_index = -1);

	/*
	* This helper function makes use of many other helper functions to load a scene. Note
	* we have separate helper functions for loading textures, materials, models, etc.
	*/
	sg::Scene parse_scene(int scene_idx = -1);

	/*
	* This loads the scene's samplers. Note, a sampler determines how pixels will be
	* extracted from a texturing during the texturing process. For example, one sampler
	* might do an averaging type operation while another might select a single pixel from
	* the texture to represent many.
	*/
	void load_samplers() const;

	/*
	* This helper function extracts inportant information from the loaded sampler part
	* of a GLTF file.
	*/
	std::unique_ptr<sg::Sampler> parse_sampler(const tinygltf::Sampler &gltf_sampler) const;

	/*
	* This function loads all the images from the GLTF scene.
	*/
	void load_images();

	/*
	* This helper method extracts image data from the GLTF file and uses
	* it to create an Image object, which it returns.
	*/
	std::unique_ptr<sg::Image> parse_image(const tinygltf::Image &gltf_image);

	/*
	* This function loads all the textures from the GLTF scene.
	*/
	void load_textures();

	/*
	* This helper method extracts texture data from the GLTF file and uses
	* it to create a Texture object, which it returns.
	*/
	std::unique_ptr<sg::Texture> parse_texture(const tinygltf::Texture &gltf_texture) const;

	/*
	* This function loads all the materials from the GLTF scene.
	*/
	void load_materials();

	/*
	* This helper method extracts material information from the GLTF file and
	* uses it ot create a PBRMaterial (PBR, i.e. Physically Based Rendering), which
	* it returns.
	*/
	std::unique_ptr<sg::PBRMaterial> parse_material(const tinygltf::Material &gltf_material) const;

	/*
	* This function loads all the meshes from the GLTF scene.
	*/
	void load_meshes();

	/*
	* This helper method extracts mesh information from the GLTF file and
	* uses it to create a Mesh object, which it returns.
	*/
	std::unique_ptr<sg::Mesh> parse_mesh(const tinygltf::Mesh &gltf_mesh) const;

	/*
	* This helper method extracts submesh data and uses to to create a SubMesh object, which it returns.
	*/
	std::unique_ptr<sg::SubMesh> parse_submesh(sg::Mesh *p_mesh, const tinygltf::Primitive &gltf_submesh) const;

	/*
	 * This helper method extracts submesh data and uses to to create a SubMesh object, which it returns.
	 */
	std::unique_ptr<sg::SubMesh> parse_submesh_as_model(const tinygltf::Primitive &gltf_primitive) const;

	/*
	* This function loads the cameras from the GLTF scene.
	*/
	void load_cameras();

	/*
	* This function loads the default camera from the GLTF scene.
	*/
	void load_default_camera();

	/*
	* This helper function extracts camera information from the GLTF file and
	* uses it to create a Camera object, which it returns.
	*/
	std::unique_ptr<sg::Camera> parse_camera(const tinygltf::Camera &gltf_camera) const;

	/*
	* This function loads the nodes (i.e. game objects) from the GLTF scene.
	*/
	void load_nodes(int scene_idx);

	/*
	* This helper function extracts node (i.e. game object) informtation from the GLTF
	* file and uses it to create a Node object, which it returns.
	*/
	std::unique_ptr<sg::Node> parse_node(const tinygltf::Node &gltf_node,
	                                     size_t                index) const;

	/*
	* This helper function is used for parsing multiple node loaded from a GLTF file.
	*/
	std::vector<std::unique_ptr<sg::Node>> parse_nodes();

	/*
	* This helper function creates a default material used for loading things
	* like meshes where a material is not specified.
	*/
	std::unique_ptr<sg::PBRMaterial> create_default_material() const;

	/*
	* This helper function creates a default texture used for loading things
	* like meshes where a texture is not specified.
	*/
	std::unique_ptr<sg::Texture>     create_default_texture(sg::Sampler &default_sampler) const;

	/*
	* This helper function creates a default image used for loading things
	* like textures where an image is not specified.
	*/
	std::unique_ptr<sg::Image>       create_default_texture_image() const;

	/*
	* This helper function creates a default sampler, which determines how pixels will be
	* extracted from textures during fragment processing.
	*/
	std::unique_ptr<sg::Sampler>     create_default_sampler() const;

	/*
	* This helper function creates a default camera.
	*/
	std::unique_ptr<sg::Camera>      create_default_camera() const;

	/*
	* This function batch uploads a number of images from a scene.
	*/
	void             batch_upload_images() const;

	/*
	* This function creates an image resource that we'll manage.
	*/
	void             create_image_resource(sg::Image &image, size_t idx) const;

	/*
	* This helper function appends texturers to be used by a material for
	* the rendering of a textured mesh.
	*/
	void             append_textures_to_material(tinygltf::ParameterMap &parameter_map, std::vector<sg::Texture *> &p_textures, sg::PBRMaterial *p_material);

	/*
	* This function counts and returns the number of vertices in a mesh, i.e. the submesh argument.
	*/
	size_t           get_submesh_vertex_count(const tinygltf::Primitive &submesh) const;

	/*
	* This function finds the GLTF scene corresponding to the scene_idx argument and returns it.
	*/
	tinygltf::Scene *pick_scene(int scene_idx);

	/*
	* This function initializes the scene's node hierarchy, as our scene graph stores all
	* game objects in such an arrangement.
	*/
	void             init_node_hierarchy(tinygltf::Scene *p_gltf_scene, std::vector<std::unique_ptr<sg::Node>> &p_nodes, sg::Node &root);

	/*
	* This function gets a pointer to attribute data for a submesh.
	*/
	template <typename T>
	const T *get_attr_data_ptr(const tinygltf::Primitive &submesh, const char *name) const
	{
		auto it = submesh.attributes.find(name);
		if (it == submesh.attributes.end())
		{
			return nullptr;
		}

		uint32_t accessor_id = it->second;
		assert(accessor_id < gltf_model_.accessors.size());
		const tinygltf::Accessor &accessor = gltf_model_.accessors[accessor_id];
		assert(accessor.bufferView < gltf_model_.bufferViews.size());
		const tinygltf::BufferView &buffer_view = gltf_model_.bufferViews[accessor.bufferView];
		assert(buffer_view.buffer < gltf_model_.buffers.size());
		const tinygltf::Buffer &buffer = gltf_model_.buffers[buffer_view.buffer];

		return reinterpret_cast<const T *>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
	}
};

}        // namespace W3D

// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "gltf_loader.hpp"

// C/C++ LANGUAGE API TYPES
#include <queue>

#include <glm/gtc/type_ptr.hpp>

// OUR OWN TYPES
#include "common/error.hpp"
#include "common/file_utils.hpp"
#include "common/utils.hpp"
#include "core/command_buffer.hpp"
#include "core/device.hpp"
#include "core/device_memory/buffer.hpp"
#include "core/image_view.hpp"
#include "core/instance.hpp"
#include "core/physical_device.hpp"
#include "scene_graph/components/camera.hpp"
#include "scene_graph/components/image.hpp"
#include "scene_graph/components/mesh.hpp"
#include "scene_graph/components/pbr_material.hpp"
#include "scene_graph/components/perspective_camera.hpp"
#include "scene_graph/components/sampler.hpp"
#include "scene_graph/components/submesh.hpp"
#include "scene_graph/components/texture.hpp"
#include "scene_graph/components/transform.hpp"
#include "scene_graph/node.hpp"
#include "scene_graph/scene.hpp"

namespace W3D
{

inline vk::Filter             to_vk_min_filter(int min_filter);
inline vk::Filter             to_vk_mag_filter(int mag_filter);
inline vk::SamplerMipmapMode  to_vk_mipmap_mode(int mipmap_mode);
inline vk::SamplerAddressMode to_vk_wrap_mode(int wrap_mode);
inline vk::Format             get_attr_format(const tinygltf::Model &model, uint32_t accessor_id);
inline std::vector<uint8_t>   get_attr_data(const tinygltf::Model &model, uint32_t accessor_id);
inline std::vector<uint8_t>   convert_data_stride(const std::vector<uint8_t> &src, uint32_t src_stride, uint32_t dst_stride);

const glm::vec3 DEFAULT_NORMAL = glm::vec3(0.0f);
const glm::vec2 DEFAULT_UV     = glm::vec2(0.0f);
const glm::vec4 DEFAULT_JOINT  = glm::vec4(0.0f);
const glm::vec4 DEFAULT_WEIGHT = glm::vec4(0.0f);

template <class T, class Y>
struct TypeCast
{
	Y operator()(T value) const noexcept
	{
		return static_cast<Y>(value);
	}
};

GLTFLoader::GLTFLoader(Device const &device) :
    device_(device)
{
}

std::unique_ptr<sg::SubMesh> GLTFLoader::read_model_from_file(const std::string &file_name, int mesh_idx)
{
	load_gltf_model(file_name);
	return parse_submesh(nullptr, gltf_model_.meshes[mesh_idx].primitives[0]);
}

void GLTFLoader::load_gltf_model(const std::string &file_name)
{
	std::string err;
	std::string warn;

	// THIS WILL DO THE REAL WORK LOADING OUR MODEL
	tinygltf::TinyGLTF gltf_loader;

	std::string gltf_file_path = fu::compute_abs_path(fu::FileType::eModelAsset, file_name);
	std::string file_extension = fu::get_file_extension(gltf_file_path);
	bool        load_result;

	// TIME TO LOAD THE CONTENTS OF THE FILE INTO OUR gltf_model_ INSTANCE VARIABLE
	if (file_extension == "bin")
	{
		// A GLTF FILE CAN BE STORED IN BINARY
		load_result =
		    gltf_loader.LoadBinaryFromFile(&gltf_model_, &err, &warn, gltf_file_path.c_str());
	}
	else if (file_extension == "gltf")
	{
		// OR AS A TEXTUAL JSON FILE
		load_result =
		    gltf_loader.LoadASCIIFromFile(&gltf_model_, &err, &warn, gltf_file_path.c_str());
	}
	else
	{
		LOGE("Unsupported file type .{} for gltf models!", file_extension);
		abort();
	}

	// DO SOME FURTHER ERROR CHECKING

	if (!err.empty())
	{
		throw std::runtime_error(err);
	}

	if (!warn.empty())
	{
		throw std::runtime_error(warn);
	}

	if (!load_result)
	{
		throw std::runtime_error("Unable to load gltf file.");
	}

	// SPECIFY WHERE TO FIND THE LOADED MODEL SO THE APP CAN USE IT
	size_t pos  = gltf_file_path.find_last_of('/');
	model_path_ = gltf_file_path.substr(0, pos);
	if (pos == std::string::npos)
	{
		model_path_.clear();
	}
}

std::unique_ptr<sg::Scene> GLTFLoader::read_scene_from_file(const std::string &file_name,
                                                            int                scene_index)
{
	// LOAD ALL THE RAW DATA FROM THE SCENE FILE
	load_gltf_model(file_name);

	// AND NOW EXTRACT ALL THE SCENE DATA FROM WHAT WE'VE LOADED
	return std::make_unique<sg::Scene>(parse_scene(scene_index));
}

sg::Scene GLTFLoader::parse_scene(int scene_idx)
{
	// WE'LL LOAD AND RETURN THIS Scene
	sg::Scene scene = sg::Scene("gltf_scene");

	// WE'LL ALSO KEEP IT
	p_scene_        = &scene;

	// THESE HELPER EACH LOAD DIFFERENT ASPECTS OF OUR SCENE, NOTE THAT
	// EACH ONE OF THESE EMPLOYS ITS OWN HELPER FUNCTIONS FOR PARSING
	// GLTF DATA AND INITIALIZING OUR SCENE OBJECTS
	load_samplers();
	load_images();
	load_textures();
	load_materials();
	batch_upload_images();
	load_meshes();
	load_nodes(scene_idx);
	load_default_camera();

	return scene;
}

void GLTFLoader::load_samplers() const
{
	// LOAD ALL THE SAMPLERS IN THE GLTF FILE
	std::vector<std::unique_ptr<sg::Sampler>> samplers(
	    gltf_model_.samplers.size());
	for (size_t i = 0; i < gltf_model_.samplers.size(); i++)
	{
		// NOTE OUR HELPER FUNCTION EXTRACTS INFO ABOUT EACH SAMPLER
		std::unique_ptr<sg::Sampler> sampler = parse_sampler(gltf_model_.samplers[i]);
		samplers[i]                          = std::move(sampler);
	}
	p_scene_->set_components(std::move(samplers));
}

std::unique_ptr<sg::Sampler> GLTFLoader::parse_sampler(
    const tinygltf::Sampler &gltf_sampler) const
{
	auto name = gltf_sampler.name;

	// SPECIFIES HOW TO DEAL WITH MINIFICATION AND MAGNIFICATION
	vk::Filter min_filter = to_vk_min_filter(gltf_sampler.minFilter);
	vk::Filter mag_filter = to_vk_mag_filter(gltf_sampler.magFilter);

	// SPECIFIES MIPMAPPING MODE, USED TO MANAGE TEXTURE LEVEL OF DETAIL
	vk::SamplerMipmapMode mipmap_mode = to_vk_mipmap_mode(gltf_sampler.minFilter);

	// VERTEX COORDINATE ADDRESSING
	vk::SamplerAddressMode address_mode_u = to_vk_wrap_mode(gltf_sampler.wrapS);
	vk::SamplerAddressMode address_mode_v = to_vk_wrap_mode(gltf_sampler.wrapT);
	vk::SamplerAddressMode address_mode_w = to_vk_wrap_mode(gltf_sampler.wrapS);

	// ALL THE INFO FOR THE SAMPLER
	vk::SamplerCreateInfo sampler_cinfo{
	    .magFilter     = mag_filter,
	    .minFilter     = min_filter,
	    .mipmapMode    = mipmap_mode,
	    .addressModeU  = address_mode_u,
	    .addressModeV  = address_mode_v,
	    .addressModeW  = address_mode_w,
	    .maxAnisotropy = device_.get_physical_device().get_handle().getProperties().limits.maxSamplerAnisotropy,
	    .maxLod        = std::numeric_limits<float>::max(),
	    .borderColor   = vk::BorderColor::eIntOpaqueWhite,
	};

	// NOW THAT WE'VE EXTRACTED ALL THE INFO WE CAN USE IT TO CREATE
	// AND RETURN ONE OF OUR Sampler OBJECTS
	return std::make_unique<sg::Sampler>(device_, name, sampler_cinfo);
}

std::unique_ptr<sg::Sampler> GLTFLoader::create_default_sampler() const
{
	tinygltf::Sampler gltf_sampler;
	gltf_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
	gltf_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;

	gltf_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
	gltf_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;

	return parse_sampler(gltf_sampler);
}

void GLTFLoader::load_images()
{
	std::vector<std::unique_ptr<sg::Image>> p_images;
	p_images.reserve(gltf_model_.images.size());
	img_tinfos_.reserve(gltf_model_.images.size());

	// GO THROUGH ALL THE IMAGES SPECIFIED IN THE GLTF FILE
	for (size_t i = 0; i < gltf_model_.images.size(); i++)
	{
		// LOAD EACH IMAGE USING OUR HELPER FUNCTION, parse_image
		p_images.emplace_back(parse_image(gltf_model_.images[i]));
	}

	p_scene_->set_components(std::move(p_images));
}

std::unique_ptr<sg::Image> GLTFLoader::parse_image(const tinygltf::Image &gltf_image)
{
	// LOAD ALL THE IMAGE DATA
	if (!gltf_image.image.empty())
	{
		img_tinfos_.push_back({
		    .binary = std::move(gltf_image.image),
		    .meta   = {
		          .extent = {
		              .width  = to_u32(gltf_image.width),
		              .height = to_u32(gltf_image.height),
		              .depth  = 1,
                },
		          .format = vk::Format::eR8G8B8A8Unorm,
		          .levels = 1,
            },
		});
	}
	else
	{
		std::string path = model_path_ + "/" + gltf_image.uri;
		img_tinfos_.push_back(ImageResource::load_two_dim_image(path));
	}

	// RETURN THE IMAGE AS OUR Image OBJECT
	return std::make_unique<sg::Image>(
	    ImageResource(device_, nullptr),
	    gltf_image.name);
}

void GLTFLoader::batch_upload_images() const
{
	std::vector<sg::Image *> p_images = p_scene_->get_components<sg::Image>();

	size_t i = 0;
	// WE IGNORE THE LAST IMAGE BECAUSE IT'S THE DEFAULT IMAGE WE'VE CREATED FOR DEFAULT TEXTURES.
	size_t count = p_images.size() - 1;

	while (i < count)
	{
		std::vector<Buffer> staging_bufs;
		CommandBuffer       cmd_buf    = device_.begin_one_time_buf();
		size_t              batch_size = 0;

		while (i < count && batch_size < 64 * 1024 * 1024)
		{
			sg::Image               *p_image   = p_images[i];
			const ImageTransferInfo &img_tinfo = img_tinfos_[i];
			size_t                   img_size  = img_tinfo.binary.size();

			create_image_resource(*p_image, i);

			batch_size += img_size;
			staging_bufs.emplace_back(device_.get_device_memory_allocator().allocate_staging_buffer(img_size));
			staging_bufs.back().update(img_tinfo.binary);
			cmd_buf.set_image_layout(p_image->get_resource(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer);
			cmd_buf.update_image(p_image->get_resource(), staging_bufs.back());
			cmd_buf.set_image_layout(p_image->get_resource(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader);

			i++;
		}
		device_.end_one_time_buf(cmd_buf);
	}
};

void GLTFLoader::create_image_resource(sg::Image &image, size_t idx) const
{
	const ImageTransferInfo &img_tinfo = img_tinfos_[idx];
	vk::ImageCreateInfo      img_cinfo{
	         .imageType   = vk::ImageType::e2D,
	         .format      = img_tinfo.meta.format,
	         .extent      = img_tinfo.meta.extent,
	         .mipLevels   = img_tinfo.meta.levels,
	         .arrayLayers = 1,
	         .samples     = vk::SampleCountFlagBits::e1,
	         .tiling      = vk::ImageTiling::eOptimal,
	         .usage       = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
	         .sharingMode = vk::SharingMode::eExclusive,
    };

	Image vk_image = device_.get_device_memory_allocator().allocate_device_only_image(img_cinfo);

	vk::ImageViewCreateInfo view_cinfo = ImageView::two_dim_view_cinfo(vk_image.get_handle(), img_cinfo.format, vk::ImageAspectFlagBits::eColor, img_cinfo.mipLevels);

	image.set_resource(ImageResource(std::move(vk_image), ImageView(device_, view_cinfo)));
}

void GLTFLoader::load_textures()
{
	std::unique_ptr<sg::Sampler> p_default_sampler = create_default_sampler();
	std::vector<sg::Sampler *>   p_samplers        = p_scene_->get_components<sg::Sampler>();
	std::vector<sg::Image *>     p_images          = p_scene_->get_components<sg::Image>();

	for (auto &gltf_texture : gltf_model_.textures)
	{
		std::unique_ptr<sg::Texture> p_texture = parse_texture(gltf_texture);
		assert(gltf_texture.source < p_images.size());
		p_texture->p_resource_ = &p_images[gltf_texture.source]->get_resource();

		if (gltf_texture.sampler >= 0 && gltf_texture.sampler < static_cast<int>(p_samplers.size()))
		{
			p_texture->p_sampler_ = p_samplers[gltf_texture.sampler];
		}
		else
		{
			if (gltf_texture.name.empty())
			{
				gltf_texture.name = p_images[gltf_texture.source]->get_name();
			}
			p_texture->p_sampler_ = p_default_sampler.get();
		}
		p_scene_->add_component(std::move(p_texture));
	}

	p_scene_->add_component(create_default_texture(*p_default_sampler));
	p_scene_->add_component(std::move(p_default_sampler));
}

std::unique_ptr<sg::Texture> GLTFLoader::create_default_texture(sg::Sampler &default_sampler) const
{
	std::unique_ptr<sg::Texture> p_texture = std::make_unique<sg::Texture>("default_texture");
	std::unique_ptr<sg::Image>   p_image   = create_default_texture_image();
	p_texture->p_resource_                 = &p_image->get_resource();
	p_texture->p_sampler_                  = &default_sampler;
	p_scene_->add_component(std::move(p_image));
	return p_texture;
}

std::unique_ptr<sg::Image> GLTFLoader::create_default_texture_image() const
{
	vk::ImageCreateInfo image_cinfo{
	    .imageType = vk::ImageType::e2D,
	    .format    = vk::Format::eR8G8B8A8Srgb,
	    .extent    = {
	           .width  = 1,
	           .height = 1,
	           .depth  = 1,
        },
	    .mipLevels   = 1,
	    .arrayLayers = 1,
	    .samples     = vk::SampleCountFlagBits::e1,
	    .tiling      = vk::ImageTiling::eOptimal,
	    .usage       = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
	    .sharingMode = vk::SharingMode::eExclusive,
	};

	Image img = device_.get_device_memory_allocator().allocate_device_only_image(image_cinfo);

	vk::ImageViewCreateInfo view_cinfo = ImageView::two_dim_view_cinfo(img.get_handle(), image_cinfo.format, vk::ImageAspectFlagBits::eColor, 1);
	ImageResource           resource   = ImageResource(std::move(img), ImageView(device_, view_cinfo));

	std::vector<uint8_t> binary = {0u, 0u, 0u, 0u};

	Buffer staging_buf = device_.get_device_memory_allocator().allocate_staging_buffer(binary.size());
	staging_buf.update(binary);

	CommandBuffer cmd_buf = device_.begin_one_time_buf();

	cmd_buf.set_image_layout(resource, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer);
	cmd_buf.update_image(resource, staging_buf);
	cmd_buf.set_image_layout(resource, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader);

	device_.end_one_time_buf(cmd_buf);

	return std::make_unique<sg::Image>(std::move(resource), "default_image");
}
std::unique_ptr<sg::Texture> GLTFLoader::parse_texture(
    const tinygltf::Texture &gltf_texture) const
{
	return std::make_unique<sg::Texture>(gltf_texture.name);
}

void GLTFLoader::load_materials()
{
	std::vector<sg::Texture *> p_textures;
	if (p_scene_->has_component<sg::Texture>())
	{
		p_textures = p_scene_->get_components<sg::Texture>();
	}

	for (auto &gltf_material : gltf_model_.materials)
	{
		std::unique_ptr<sg::PBRMaterial> p_material = parse_material(gltf_material);
		append_textures_to_material(gltf_material.values, p_textures, p_material.get());
		append_textures_to_material(gltf_material.additionalValues, p_textures, p_material.get());
		p_scene_->add_component(std::move(p_material));
	}
}

std::unique_ptr<sg::PBRMaterial> GLTFLoader::parse_material(
    const tinygltf::Material &gltf_material) const
{
	auto material = std::make_unique<sg::PBRMaterial>(gltf_material.name);

	for (auto &gltf_value : gltf_material.values)
	{
		if (gltf_value.first == "baseColorFactor")
		{
			const auto &color_factor = gltf_value.second.ColorFactor();
			material->base_color_factor_ =
			    glm::vec4(color_factor[0], color_factor[1], color_factor[2], color_factor[3]);
		}
		else if (gltf_value.first == "metallicFactor")
		{
			material->metallic_factor = static_cast<float>(gltf_value.second.Factor());
		}
		else if (gltf_value.first == "roughnessFactor")
		{
			material->roughness_factor = static_cast<float>(gltf_value.second.Factor());
		}
	}

	for (auto &gltf_value : gltf_material.additionalValues)
	{
		if (gltf_value.first == "emissiveFactor")
		{
			const auto &emissive_factor = gltf_value.second.number_array;
			material->emissive_ =
			    glm::vec3(emissive_factor[0], emissive_factor[1], emissive_factor[2]);
		}
		else if (gltf_value.first == "alphaMode")
		{
			if (gltf_value.second.string_value == "BLEND")
			{
				material->alpha_mode_ = sg::AlphaMode::Blend;
			}
			else if (gltf_value.second.string_value == "OPAQUE")
			{
				material->alpha_mode_ = sg::AlphaMode::Opaque;
			}
			else if (gltf_value.second.string_value == "MASK")
			{
				material->alpha_mode_ = sg::AlphaMode::Mask;
			}
		}
		else if (gltf_value.first == "alphaCutoff")
		{
			material->alpha_cutoff_ = static_cast<float>(gltf_value.second.number_value);
		}
		else if (gltf_value.first == "doubleSided")
		{
			material->is_double_sided = gltf_value.second.bool_value;
		}
	}

	return material;
}

void GLTFLoader::append_textures_to_material(tinygltf::ParameterMap &parameter_map, std::vector<sg::Texture *> &p_textures, sg::PBRMaterial *p_material)
{
	// Edit img formats here
	for (auto &value : parameter_map)
	{
		if (value.first.find("Texture") != std::string::npos)
		{
			int         texture_idx  = value.second.TextureIndex();
			std::string texture_name = to_snake_case(to_string(value.first));
			assert(texture_idx < p_textures.size());
			if (texture_name == "base_color_texture" || texture_name == "emissive_texture")
			{
				img_tinfos_[gltf_model_.textures[texture_idx].source].meta.format = vk::Format::eR8G8B8A8Srgb;
			}
			p_material->texture_map_[texture_name] = p_textures[value.second.TextureIndex()];
		}
	}
}

std::unique_ptr<sg::PBRMaterial> GLTFLoader::create_default_material() const
{
	tinygltf::Material gltf_material;
	return parse_material(gltf_material);
}

void GLTFLoader::load_meshes()
{
	std::unique_ptr<sg::PBRMaterial> p_default_material = create_default_material();
	std::vector<sg::PBRMaterial *>   p_materials        = p_scene_->get_components<sg::PBRMaterial>();

	for (auto &gltf_mesh : gltf_model_.meshes)
	{
		std::unique_ptr<sg::Mesh> p_mesh = parse_mesh(gltf_mesh);

		for (const auto &primitive : gltf_mesh.primitives)
		{
			std::unique_ptr<sg::SubMesh> p_submesh = parse_submesh(p_mesh.get(), primitive);
			if (primitive.material >= 0)
			{
				assert(primitive.material < p_materials.size());
				p_submesh->set_material(*p_materials[primitive.material]);
			}
			else
			{
				p_submesh->set_material(*p_default_material);
			}
			p_mesh->add_submesh(*p_submesh);
			p_scene_->add_component(std::move(p_submesh));
		}

		p_scene_->add_component(std::move(p_mesh));
	}
}

std::unique_ptr<sg::Mesh> GLTFLoader::parse_mesh(const tinygltf::Mesh &gltf_mesh) const
{
	return std::make_unique<sg::Mesh>(gltf_mesh.name);
}

std::unique_ptr<sg::SubMesh> GLTFLoader::parse_submesh(sg::Mesh *p_mesh, const tinygltf::Primitive &gltf_submesh) const
{
	std::vector<Buffer>          transient_bufs;
	std::unique_ptr<sg::SubMesh> p_submesh = std::make_unique<sg::SubMesh>();
	// pos_accessor is guranteed to exist
	const tinygltf::Accessor pos_accessor = gltf_model_.accessors[gltf_submesh.attributes.find("POSITION")->second];
	p_submesh->vertex_count_              = pos_accessor.count;
	if (p_mesh)
	{
		p_mesh->get_mut_bounds().update(
		    glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]),
		    glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]));
	}

	std::vector<sg::Vertex> vertexs;
	vertexs.reserve(p_submesh->vertex_count_);

	// NOTE A MESH CAN BE MADE UP OF SUBMESHES, WHICH CAN BE ANIMATED. HERE
	// WE ARE SPECIFYING THE ATTRIBUTE DATA IN OUR VERTEX BUFFER
	const float    *p_pos    = get_attr_data_ptr<float>(gltf_submesh, "POSITION");
	const float    *p_norm   = get_attr_data_ptr<float>(gltf_submesh, "NORMAL");
	const float    *p_uv     = get_attr_data_ptr<float>(gltf_submesh, "TEXCOORD_0");
	const uint16_t *p_joint  = get_attr_data_ptr<uint16_t>(gltf_submesh, "JOINTS_0");
	const float    *p_weight = get_attr_data_ptr<float>(gltf_submesh, "WEIGHTS_0");

	bool is_skinned = p_joint && p_weight;

	for (size_t i = 0; i < p_submesh->vertex_count_; i++)
	{
		vertexs.emplace_back(sg::Vertex{
		    .pos    = glm::make_vec3(&p_pos[i * 3]),
		    .norm   = p_norm ? glm::normalize(glm::make_vec3(&p_norm[i * 3])) : glm::vec3(0.0f),
		    .uv     = p_uv ? glm::make_vec2(&p_uv[i * 2]) : glm::vec2(0.0f),
		    .joint  = is_skinned ? glm::vec4(glm::make_vec4(&p_joint[i * 4])) : glm::vec4(0.0f),
		    .weight = is_skinned ? glm::make_vec4(&p_weight[i * 4]) : glm::vec4(0.0f),
		});
	}

	size_t vertex_buf_size    = vertexs.size() * sizeof(sg::Vertex);
	Buffer vertex_staging_buf = device_.get_device_memory_allocator().allocate_staging_buffer(vertex_buf_size);
	Buffer vertex_buf         = device_.get_device_memory_allocator().allocate_vertex_buffer(vertex_buf_size);
	vertex_staging_buf.update(vertexs.data(), vertex_buf_size);
	CommandBuffer cmd_buf = device_.begin_one_time_buf();
	cmd_buf.copy_buffer(vertex_staging_buf, vertex_buf, vertex_buf_size);
	p_submesh->p_vertex_buf_ = std::make_unique<Buffer>(std::move(vertex_buf));
	transient_bufs.push_back(std::move(vertex_staging_buf));

	if (gltf_submesh.indices >= 0)
	{
		const tinygltf::Accessor &accessor = gltf_model_.accessors[gltf_submesh.indices];
		p_submesh->idx_count_              = accessor.count;

		vk::Format           format = get_attr_format(gltf_model_, gltf_submesh.indices);
		std::vector<uint8_t> indexs = get_attr_data(gltf_model_, gltf_submesh.indices);

		switch (format)
		{
			case vk::Format::eR32Uint:
				break;
			case vk::Format::eR16Uint:
			{
				indexs = convert_data_stride(indexs, 2, 4);
				break;
			}
			case vk::Format::eR8Uint:
			{
				indexs = convert_data_stride(indexs, 1, 4);
				break;
			}
			default:
				// unreachable;
				break;
		}

		Buffer idx_staging_buf = device_.get_device_memory_allocator().allocate_staging_buffer(indexs.size());
		Buffer idx_buf         = device_.get_device_memory_allocator().allocate_index_buffer(indexs.size());
		idx_staging_buf.update(indexs);
		cmd_buf.copy_buffer(idx_staging_buf, idx_buf, indexs.size());
		transient_bufs.push_back(std::move(idx_staging_buf));
		p_submesh->p_idx_buf_ = std::make_unique<Buffer>(std::move(idx_buf));
	}

	device_.end_one_time_buf(cmd_buf);
	return std::move(p_submesh);
}

size_t GLTFLoader::get_submesh_vertex_count(const tinygltf::Primitive &submesh) const
{
	// GLTF gurantees that a vertex will always have position attribute
	const tinygltf::Accessor accessor = gltf_model_.accessors[submesh.attributes.find("POSITION")->second];
	return accessor.count;
}

void GLTFLoader::load_cameras()
{
	for (const tinygltf::Camera &camera : gltf_model_.cameras)
	{
		p_scene_->add_component(parse_camera(camera));
	}
}

std::unique_ptr<sg::Camera> GLTFLoader::parse_camera(
    const tinygltf::Camera &gltf_camera) const
{
	std::unique_ptr<sg::Camera> camera;

	if (gltf_camera.type == "perspective")
	{
		auto perspective_camera = std::make_unique<sg::PerspectiveCamera>(gltf_camera.name);

		perspective_camera->set_aspect_ratio(
		    static_cast<float>(gltf_camera.perspective.aspectRatio));
		perspective_camera->set_field_of_view(static_cast<float>(gltf_camera.perspective.yfov));
		perspective_camera->set_near_plane(static_cast<float>(gltf_camera.perspective.znear));
		perspective_camera->set_far_plane(static_cast<float>(gltf_camera.perspective.zfar));

		camera = std::move(perspective_camera);
	}
	else
	{
		throw std::runtime_error("Camera type not supported");
	}

	return camera;
}

void GLTFLoader::load_default_camera()
{
	std::unique_ptr<sg::Node>   p_camera_node = std::make_unique<sg::Node>(-1, "default_camera");
	std::unique_ptr<sg::Camera> p_camera      = create_default_camera();

	p_camera->set_node(*p_camera_node);
	p_camera_node->set_component(*p_camera);

	p_scene_->add_component(std::move(p_camera));
	p_scene_->get_root_node().add_child(*p_camera_node);
	p_scene_->add_node(std::move(p_camera_node));
}

std::unique_ptr<sg::Camera> GLTFLoader::create_default_camera() const
{
	tinygltf::Camera gltf_camera;
	gltf_camera.name = "default_camera";
	gltf_camera.type = "perspective";

	gltf_camera.perspective.aspectRatio = 1.77f;
	gltf_camera.perspective.yfov        = 1.0f;
	gltf_camera.perspective.znear       = 0.1f;
	gltf_camera.perspective.zfar        = 1000.0f;

	return parse_camera(gltf_camera);
}

void GLTFLoader::load_nodes(int scene_idx)
{
	std::vector<std::unique_ptr<sg::Node>> p_nodes      = parse_nodes();
	tinygltf::Scene                       *p_gltf_scene = pick_scene(scene_idx);
	std::unique_ptr<sg::Node>              root         = std::make_unique<sg::Node>(0, p_gltf_scene->name);

	init_node_hierarchy(p_gltf_scene, p_nodes, *root);

	p_scene_->set_root_node(*root);
	p_nodes.push_back(std::move(root));
	p_scene_->set_nodes(std::move(p_nodes));
}

void GLTFLoader::init_node_hierarchy(tinygltf::Scene *p_gltf_scene, std::vector<std::unique_ptr<sg::Node>> &p_nodes, sg::Node &root)
{
	struct NodeTraversal
	{
		sg::Node &parent;
		int       curr_idx;
	};

	std::queue<NodeTraversal> q;

	for (int i : p_gltf_scene->nodes)
	{
		q.push({
		    .parent   = root,
		    .curr_idx = i,
		});
	}

	while (!q.empty())
	{
		NodeTraversal traversal = q.front();
		q.pop();
		assert(traversal.curr_idx < p_nodes.size());
		sg::Node &curr = *p_nodes[traversal.curr_idx];
		traversal.parent.add_child(curr);
		curr.set_parent(traversal.parent);

		for (int child_idx : gltf_model_.nodes[traversal.curr_idx].children)
		{
			q.push({
			    .parent   = curr,
			    .curr_idx = child_idx,
			});
		}
	}
}

std::vector<std::unique_ptr<sg::Node>> GLTFLoader::parse_nodes()
{
	std::vector<sg::Camera *>              p_cameras = p_scene_->get_components<sg::Camera>();
	std::vector<sg::Mesh *>                p_meshs   = p_scene_->get_components<sg::Mesh>();
	std::vector<std::unique_ptr<sg::Node>> p_nodes;
	p_nodes.reserve(gltf_model_.nodes.size());

	for (size_t i = 0; i < gltf_model_.nodes.size(); i++)
	{
		const tinygltf::Node     &gltf_node = gltf_model_.nodes[i];
		std::unique_ptr<sg::Node> p_node    = parse_node(gltf_node, i);

		if (gltf_node.mesh >= 0)
		{
			assert(gltf_node.mesh < p_meshs.size());
			sg::Mesh *p_mesh = p_meshs[gltf_node.mesh];
			p_node->set_component(*p_mesh);
			p_mesh->add_node(*p_node);
		}

		if (gltf_node.camera >= 0)
		{
			assert(gltf_node.camera < p_cameras.size());
			sg::Camera *p_camera = p_cameras[gltf_node.camera];
			p_node->set_component(*p_camera);
			p_camera->set_node(*p_node);
		}

		p_nodes.push_back(std::move(p_node));
	};
	return p_nodes;
}

std::unique_ptr<sg::Node> GLTFLoader::parse_node(const tinygltf::Node &gltf_node,
                                                 size_t                index) const
{
	auto node = std::make_unique<sg::Node>(index, gltf_node.name);

	auto &transform = node->get_component<sg::Transform>();

	if (!gltf_node.translation.empty())
	{
		glm::vec3 translation;
		std::transform(gltf_node.translation.begin(), gltf_node.translation.end(), glm::value_ptr(translation), TypeCast<double, float>{});
		transform.set_tranlsation(translation);
	}

	if (!gltf_node.rotation.empty())
	{
		glm::quat rotation;
		std::transform(gltf_node.rotation.begin(), gltf_node.rotation.end(), glm::value_ptr(rotation), TypeCast<double, float>{});
		transform.set_rotation(rotation);
	}

	if (!gltf_node.scale.empty())
	{
		glm::vec3 scale;
		std::transform(gltf_node.scale.begin(), gltf_node.scale.end(), glm::value_ptr(scale), TypeCast<double, float>{});
		transform.set_scale(scale);
	}

	if (!gltf_node.matrix.empty())
	{
		glm::mat4 matrix;
		std::transform(gltf_node.matrix.begin(), gltf_node.matrix.end(), glm::value_ptr(matrix), TypeCast<double, float>{});
		transform.set_world_M(matrix);
	}

	return node;
}

tinygltf::Scene *GLTFLoader::pick_scene(int scene_idx)
{
	tinygltf::Scene *gltf_scene  = nullptr;
	int              scene_count = static_cast<int>(gltf_model_.scenes.size());

	if (scene_idx >= 0 && scene_idx < scene_count)
	{
		gltf_scene = &gltf_model_.scenes[scene_idx];
	}
	else if (gltf_model_.defaultScene >= 0 && gltf_model_.defaultScene < scene_count)
	{
		gltf_scene = &gltf_model_.scenes[gltf_model_.defaultScene];
	}
	else if (gltf_model_.scenes.size() > 0)
	{
		gltf_scene = &gltf_model_.scenes[0];
	}

	if (!gltf_scene)
	{
		LOGE("Couldn't determine which scene to load");
		abort();
	}

	return gltf_scene;
}

inline vk::Filter to_vk_min_filter(int min_filter)
{
	switch (min_filter)
	{
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
			return vk::Filter::eNearest;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return vk::Filter::eLinear;
		default:
			return vk::Filter::eLinear;
	}
}

inline vk::Filter to_vk_mag_filter(int mag_filter)
{
	switch (mag_filter)
	{
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
			return vk::Filter::eLinear;
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
			return vk::Filter::eNearest;
		default:
			return vk::Filter::eLinear;
	}
}

inline vk::SamplerMipmapMode to_vk_mipmap_mode(int mipmap_mode)
{
	switch (mipmap_mode)
	{
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
			return vk::SamplerMipmapMode::eNearest;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return vk::SamplerMipmapMode::eLinear;
		default:
			return vk::SamplerMipmapMode::eLinear;
	}
}

inline vk::SamplerAddressMode to_vk_wrap_mode(int wrap_mode)
{
	switch (wrap_mode)
	{
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			return vk::SamplerAddressMode::eRepeat;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			return vk::SamplerAddressMode::eClampToEdge;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			return vk::SamplerAddressMode::eMirroredRepeat;
		default:
			return vk::SamplerAddressMode::eRepeat;
	}
}

inline vk::Format get_attr_format(const tinygltf::Model &model, uint32_t accessor_id)
{
	assert(accessor_id < model.accessors.size());
	auto &accessor = model.accessors[accessor_id];

	vk::Format format;

	switch (accessor.componentType)
	{
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR8Sint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR8G8Sint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR8G8B8Sint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR8G8B8A8Sint}};

			format = mapped_format.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR8Uint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR8G8Uint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR8G8B8Uint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR8G8B8A8Uint}};

			static const std::map<int, vk::Format> mapped_format_normalize = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR8Unorm},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR8G8Unorm},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR8G8B8Unorm},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR8G8B8A8Unorm}};

			if (accessor.normalized)
			{
				format = mapped_format_normalize.at(accessor.type);
			}
			else
			{
				format = mapped_format.at(accessor.type);
			}

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR16Sint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR16G16Sint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR16G16B16Sint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR16G16B16A16Sint}};

			format = mapped_format.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR16Uint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR16G16Uint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR16G16B16Uint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR16G16B16A16Uint}};

			static const std::map<int, vk::Format> mapped_format_normalize = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR16Unorm},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR16G16Unorm},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR16G16B16Unorm},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR16G16B16A16Unorm}};
			if (accessor.normalized)
			{
				format = mapped_format_normalize.at(accessor.type);
			}
			else
			{
				format = mapped_format.at(accessor.type);
			}

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_INT:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR32Sint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR32G32Sint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR32G32B32Sint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR32G32B32A32Sint}};

			format = mapped_format.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR32Uint},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR32G32Uint},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR32G32B32Uint},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR32G32B32A32Uint}};
			format = mapped_format.at(accessor.type);

			break;
		}
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
		{
			static const std::map<int, vk::Format> mapped_format = {
			    {TINYGLTF_TYPE_SCALAR, vk::Format::eR32Sfloat},
			    {TINYGLTF_TYPE_VEC2, vk::Format::eR32G32Sfloat},
			    {TINYGLTF_TYPE_VEC3, vk::Format::eR32G32B32Sfloat},
			    {TINYGLTF_TYPE_VEC4, vk::Format::eR32G32B32A32Sfloat}};

			format = mapped_format.at(accessor.type);

			break;
		}
		default:
		{
			format = vk::Format::eUndefined;
			break;
		}
	}

	return format;
}

std::vector<uint8_t> get_attr_data(const tinygltf::Model &model, uint32_t accessor_id)
{
	assert(accessor_id < model.accessors.size());
	auto &accessor = model.accessors[accessor_id];
	assert(accessor.bufferView < model.bufferViews.size());
	auto &buffer_view = model.bufferViews[accessor.bufferView];
	assert(buffer_view.buffer < model.buffers.size());
	auto &buffer = model.buffers[buffer_view.buffer];

	size_t stride     = accessor.ByteStride(buffer_view);
	size_t start_byte = accessor.byteOffset + buffer_view.byteOffset;
	size_t end_byte   = start_byte + accessor.count * stride;

	return {buffer.data.begin() + start_byte, buffer.data.begin() + end_byte};
}

std::vector<uint8_t> convert_data_stride(const std::vector<uint8_t> &src,
                                         uint32_t                    src_stride,
                                         uint32_t                    dst_stride)
{
	auto                 elem_count = src.size() / src_stride;
	std::vector<uint8_t> dst(elem_count * dst_stride);

	for (uint32_t src_idx = 0, dst_idx = 0; src_idx < src.size() && dst_idx < dst.size();
	     src_idx += src_stride, dst_idx += dst_stride)
	{
		std::copy(src.begin() + src_idx, src.begin() + src_idx + src_stride, dst.begin() + dst_idx);
	}

	return dst;
}

}        // namespace W3D
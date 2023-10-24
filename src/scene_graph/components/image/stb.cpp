#include "stb.hpp"

#include <stb_image.h>

namespace W3D::sg
{

Stb::Stb(const std::string &name, const std::vector<uint8_t> &data) :
    Image(name)
{
	int width, height;
	int comp;
	int req_comp = 4;

	auto data_buffer = reinterpret_cast<const stbi_uc *>(data.data());
	auto data_size   = static_cast<int>(data.size());

	auto raw_data = stbi_load_from_memory(data_buffer, data_size, &width, &height, &comp, req_comp);

	if (!raw_data)
	{
		throw std::runtime_error("Failed to load" + name + ": " + stbi_failure_reason());
	}

	data_ = {raw_data, raw_data + data_size};

	stbi_image_free(raw_data);

	format_                   = vk::Format::eR8G8B8A8Srgb;
	mipmaps_[0].extent.width  = static_cast<uint32_t>(width);
	mipmaps_[0].extent.height = height;
	mipmaps_[0].extent.depth  = 1u;
}
}        // namespace W3D::sg

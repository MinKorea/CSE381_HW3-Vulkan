// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "file_utils.hpp"

// C/C++ LANGUAGE API TYPES
#include <fstream>
#include <unordered_map>

#include "common/logging.hpp"

namespace W3D::fu
{
// THIS IS WHERE WE'LL STORE OUR SHADERS, MODELS, AND TEXTURES
const std::unordered_map<FileType, std::string> relative_paths = {
    {FileType::eShader, "shaders/"},
    {FileType::eModelAsset, "../assets/models/"},
    {FileType::eImage, "../assets/images/"},
};

std::vector<uint8_t> read_shader_binary(const std::string &file_name)
{
	return read_binary(compute_abs_path(FileType::eShader, file_name));
};

std::vector<uint8_t> read_binary(const std::string &path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		LOGE("failed to open file: {}", path);
		throw std::runtime_error("failed to open file: " + path);
	};

	size_t               file_size = (size_t) file.tellg();
	std::vector<uint8_t> buffer(file_size);

	file.seekg(0);
	file.read(reinterpret_cast<char *>(buffer.data()), file_size);
	file.close();

	return buffer;
}

std::string get_file_extension(const std::string &file_name)
{
	auto extension_pos = file_name.find_last_of(".");
	if (extension_pos != std::string::npos)
	{
		return file_name.substr(extension_pos + 1);
	}
	return "";
}

const std::string compute_abs_path(const FileType type, const std::string &file)
{
	return relative_paths.at(type) + file;
}

}	// namespace W3D::fu
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace W3D::fu
{
/*
* These are the types of files, i.e. resources, that we need to load.
*/
enum class FileType
{
	eShader,
	eModelAsset,
	eImage,
};

/*
 * This function employs the read_binary function to a compiled
 * shader file in byte code located at file_name and returns its binary contents in
 * a vector, returning that vector.
 */
std::vector<uint8_t> read_shader_binary(const std::string &filename);

/*
 * This function loads all the contents of the file at path and
 * loads its contents as binary bytes into a vector and returns that vector.
 */
std::vector<uint8_t> read_binary(const std::string &filename);

/*
 * This function gets and returns the file extension
 * of the file_name argument.
 */
std::string       get_file_extension(const std::string &filename);

/*
 * This helper function determines and returns the absolute
 * path of the file argument.
 */
const std::string    compute_abs_path(const FileType type, const std::string &file);

}        // namespace W3D::fu
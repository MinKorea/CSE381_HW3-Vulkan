#pragma once
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>

namespace vk
{
struct Extent3D;
enum class Format;
};	// namespace vk

namespace W3D
{
namespace sg
{
class Scene;
class Node;
}        // namespace sg

// PassKey idiom. See https://stackoverflow.com/questions/3217390/clean-c-granular-friend-equivalent-answer-attorney-client-idiom/3218920#3218920
template <typename T>
class Key
{
	friend T;
	Key(){};
	Key(Key const &){};
};

template <typename T>
inline uint32_t to_u32(T value)
{
	static_assert(std::is_arithmetic<T>::value, "T must be numeric");

	if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<uint32_t>::max()))
	{
		throw std::runtime_error("to_u32() faile , value is too big to be converted to uint32_t");
	}

	return static_cast<uint32_t>(value);
}

template <typename T>
inline uint8_t *to_ubyte_ptr(T *value)
{
	return reinterpret_cast<uint8_t *>(value);
}

template <typename T>
inline std::string to_string(const T &value)
{
	std::stringstream ss;
	ss << std::fixed << value;
	return ss.str();
}

std::string to_snake_case(const std::string &text);

sg::Node *add_free_camera_script(sg::Scene &scene, const std::string &node_name, int width,
                                 int height);

// FNV-1a 32bit hashing algorithm.
constexpr uint32_t fnv1a_32(const char *s, std::size_t len)
{
	return ((len ? fnv1a_32(s, len - 1) : 2166136261u) ^ s[len]) * 16777619u;
}

constexpr size_t const_strlen(const char *s)
{
	size_t len = 0;
	while (s[len])
	{
		len++;
	}
	return len;
}

constexpr uint32_t string_hash(const char *s)
{
	return fnv1a_32(s, const_strlen(s));
}

uint32_t max_mip_levels(uint32_t width, uint32_t height);

}	// namespace W3D

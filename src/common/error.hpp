#pragma once

#include <cassert>
#include <stdexcept>

#include "logging.hpp"
#include "utils.hpp"

#if defined(__clang__)
// CLANG ENABLE/DISABLE WARNING DEFINITION
#	define W3D_DISABLE_WARNINGS()                                                     \
		_Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wall\"") \
		    _Pragma("clang diagnostic ignored \"-Wextra\"")                            \
		        _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")

#	define W3D_ENABLE_WARNINGS() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
// GCC ENABLE/DISABLE WARNING DEFINITION
#	define W3D_DISABLE_WARNINGS()                                                 \
		_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wall\"") \
		    _Pragma("clang diagnostic ignored \"-Wextra\"")                        \
		        _Pragma("clang diagnostic ignored \"-Wtautological-compare\"")

#	define W3D_ENABLE_WARNINGS() _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
// MSVC ENABLE/DISABLE WARNING DEFINITION
#	define W3D_DISABLE_WARNINGS() __pragma(warning(push, 0))

#	define W3D_ENABLE_WARNINGS() __pragma(warning(pop))
#endif

#define VK_CHECK(x)                                       \
	do                                                    \
	{                                                     \
		VkResult err = x;                                 \
		if (err)                                          \
		{                                                 \
			LOGE("[VULKAN ERROR]: ", x); \
			abort();                                      \
		}                                                 \
	} while (0)
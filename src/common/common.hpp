#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#	define __IS_ON_OSX__
constexpr const bool IS_ON_OSX = true;
#else
constexpr const bool IS_ON_OSX = false;
#endif

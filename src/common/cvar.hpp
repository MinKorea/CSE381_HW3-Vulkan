#pragma once

#include <stdint.h>
#include <string>

namespace W3D
{
class CVarProperty;

/*
* This defines the structure of a console variable system, similar to the one employed
* in the Unreal Engine which is useful for users to be able to type commands into an application
* console to change variable values. Note, this serves as a base class for CVarSystemImpl, which
* is defined in the cpp file. Note, this will be a singleton, and the get method can
* be used for getting it.
* 
* See https://vkguide.dev/docs/extra-chapter/cvar_system/ for more.
*/
class CVarSystem
{
  public:
	static CVarSystem &get();
	virtual CVarProperty      &create_int_cvar(const char *name, int32_t val)     = 0;
	virtual CVarProperty      &create_float_cvar(const char *name, float val)     = 0;
	virtual CVarProperty      &create_str_cvar(const char *name, const char *val) = 0;
	virtual int32_t            get_int_cvar(CVarProperty &prop)                   = 0;
	virtual float              get_float_cvar(CVarProperty &prop)                 = 0;
	virtual const std::string &get_str_cvar(CVarProperty &prop)                   = 0;
	virtual void               set_int_cvar(int32_t val, CVarProperty &prop)      = 0;
	virtual void               set_float_cvar(float val, CVarProperty &prop)      = 0;
	virtual void               set_str_cvar(const char *val, CVarProperty &prop)  = 0;
	virtual CVarProperty      &get_cvar_prop(const char *name)                    = 0;
	// pimpl
};

}        // namespace W3D
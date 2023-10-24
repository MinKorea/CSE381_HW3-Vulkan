// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "cvar.hpp"

// C/C++ LANGUAGE API TYPES
#include <string>

// OUR OWN TYPES
#include "common/logging.hpp"
#include "common/utils.hpp"

namespace W3D
{

struct CVarProperty
{
	uint32_t    idx;
	std::string name;
};

template <typename T>
struct CVar
{
	T             val;
	CVarProperty &property;
};

template <typename T>
class CVarArray
{
  private:
	std::vector<CVar<T>> cvars_;
	uint32_t             max_size_;

  public:
	CVarArray(uint32_t max_size) :
	    max_size_(max_size)
	{
		cvars_.reserve(max_size);
	}

	uint32_t push_back(const T &val, CVarProperty &property)
	{
		if (cvars_.size() >= max_size_)
		{
			LOGE("CVarArray exceeded its limit.");
			abort();
		}

		cvars_.emplace_back(CVar<T>{val, property});
		return cvars_.size() - 1;
	}

	T get(uint32_t idx)
	{
		if (cvars_.size() <= idx)
		{
			LOGE("Accessing not yet allocated CVar.");
			abort();
		}

		return cvars_[idx].val;
	}

	T &get_mut(uint32_t idx)
	{
		if (cvars_.size() <= idx)
		{
			LOGE("Accessing not yet allocated CVar.");
			abort();
		}

		return cvars_[idx].val;
	}
};

class CVarSystemImpl : public CVarSystem
{
  // NOTE WE WILL KEEP THE CONSOLE VARIABLES WE WISH TO MANAGE THROUGH
  // THIS SYSTEM IN THESE ARRAYS
  private:
	CVarArray<int32_t>     int_cvars{1000};
	CVarArray<float>       float_cvars{1000};
	CVarArray<std::string> str_cvars{1000};
	std::unordered_map<uint32_t, CVarProperty> prop_map_;

  public:
	CVarProperty      &create_int_cvar(const char *name, int32_t val) override;
	CVarProperty      &create_float_cvar(const char *name, float val) override;
	CVarProperty      &create_str_cvar(const char *name, const char *val) override;
	int32_t            get_int_cvar(CVarProperty &prop) override;
	float              get_float_cvar(CVarProperty &prop) override;
	const std::string &get_str_cvar(CVarProperty &prop) override;
	void               set_int_cvar(int32_t val, CVarProperty &prop) override;
	void               set_float_cvar(float val, CVarProperty &prop) override;
	void               set_str_cvar(const char *val, CVarProperty &prop) override;
	CVarProperty      &get_cvar_prop(const char *name) override;

	CVarProperty &register_new_prop(const char *name);

	constexpr static uint32_t MAX_INT_COUNT   = 1000;
	constexpr static uint32_t MAX_FLOAT_COUNT = 1000;
	constexpr static uint32_t MAX_STR_COUNT   = 1000;

	template <typename T>
	T get_cvar_val(CVarProperty &prop)
	{
		return get_cvar_arr<T>().get(prop.idx);
	}

	template <typename T>
	T &get_mut_cvar_val(CVarProperty &prop)
	{
		return get_cvar_arr<T>().get_mut(prop.idx);
	}

	template <typename T>
	void set_cvar(const T &val, const CVarProperty &prop)
	{
		get_cvar_arr<T>().get_mut(prop.idx) = val;
	}

	// This function MUST stay undefined. It triggers a linker error if someone try to call this function on a unwanted type.
	template <typename T>
	CVarArray<T> &get_cvar_arr();

	template <>
	CVarArray<int32_t> &get_cvar_arr()
	{
		return int_cvars;
	}

	template <>
	CVarArray<float> &get_cvar_arr()
	{
		return float_cvars;
	}

	template <>
	CVarArray<std::string> &get_cvar_arr()
	{
		return str_cvars;
	}
};

static CVarSystem &get()
{
	static CVarSystemImpl cvar_sys{};
	return cvar_sys;
}

CVarProperty &CVarSystemImpl::create_int_cvar(const char *name, int32_t val)
{
	CVarProperty &prop = register_new_prop(name);
	prop.idx           = get_cvar_arr<int32_t>().push_back(val, prop);
	return prop;
}

CVarProperty &CVarSystemImpl::create_float_cvar(const char *name, float val)
{
	CVarProperty &prop = register_new_prop(name);
	prop.idx           = get_cvar_arr<float>().push_back(val, prop);
	return prop;
}

CVarProperty &CVarSystemImpl::create_str_cvar(const char *name, const char *val)
{
	CVarProperty &prop = register_new_prop(name);
	prop.idx           = get_cvar_arr<std::string>().push_back(val, prop);
	return prop;
}

int32_t CVarSystemImpl::get_int_cvar(CVarProperty &prop)
{
	return get_cvar_val<int32_t>(prop);
}

float CVarSystemImpl::get_float_cvar(CVarProperty &prop)
{
	return get_cvar_val<float>(prop);
}

const std::string &CVarSystemImpl::get_str_cvar(CVarProperty &prop)
{
	return get_mut_cvar_val<std::string>(prop);
}

void CVarSystemImpl::set_int_cvar(int32_t val, CVarProperty &prop)
{
	set_cvar(val, prop);
}

void CVarSystemImpl::set_float_cvar(float val, CVarProperty &prop)
{
	set_cvar(val, prop);
}

void CVarSystemImpl::set_str_cvar(const char *val, CVarProperty &prop)
{
	set_cvar<std::string>(val, prop);
}

CVarProperty &CVarSystemImpl::get_cvar_prop(const char *name)
{
	return prop_map_[string_hash(name)];
}

CVarProperty &CVarSystemImpl::register_new_prop(const char *name)
{
	uint32_t hash   = string_hash(name);
	prop_map_[hash] = {0, name};
	return prop_map_[hash];
};

}        // namespace W3D
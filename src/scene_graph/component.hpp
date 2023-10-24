#pragma once

#include <string>
#include <typeindex>

/*
 * The W3D::sg namespace is the Wolfie3D Scene Graph (sg) types, used to manage demo objects.
 */
namespace W3D::sg
{
/*
* The Component class represents a entity that can be placed into a scene. It will
* serve as a base class 
*/
class Component
{
  private:
	std::string name_;

  public:
	/*
	* The default constructor does no initialization, using just default parent behavior.
	*/
	Component()          = default;

	/*
	* Custom constructor employs the parent constructor to initialize the name.
	*/
	Component(const std::string &name);

	/*
	* We'll use the default destructor behavior as there is nothing in this class to destroy.
	*/
	virtual ~Component() = default;

	/*
	* Accessor method for getting this component's name.
	*/
	const std::string      &get_name() const;

	/*
	* Virtual void (i.e. abstract) function for getting the type of this component. This 
	* function must be implemented by all child classes.
	*/
	virtual std::type_index get_type() = 0;

};	// class Component

}	// namespace W3D::sg
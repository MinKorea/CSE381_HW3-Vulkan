#pragma once
#include "common/glm_common.hpp"
#include "scene_graph/component.hpp"

/*
* The W3D::sg namespace is the Wolfie3D Scene Graph (sg) types, used to manage demo objects.
*/
namespace W3D::sg
{
/*
 * AABB - an axis-aligned bounding box, this is used for collision detection. Since
 * our scene objects are simply boxes these AABBs fit those boxes precisely.
 */
class AABB : public Component
{
  private:
	glm::vec3 min_;	// MINIMUM CORNER OF THIS BOX
	glm::vec3 max_; // MAXIMUM CORNER OF THIS BOX

  // ALL METHODS DEFINED IN SOURCE FILE
  public:
	/*
	 * Default constructor, it just resets this box to maximal extents.
	 */
	AABB();

	/*
	 * Custom constructor that initializes both instance variables using
	 * provided arguments.
	 */
	AABB(const glm::vec3 &min, const glm::vec3 &max);
	virtual ~AABB() = default;

	/*
	 * Gets the actual type of this AABB object.
	 */
	virtual std::type_index get_type() override;

	/*
	 * This update function overload uses a single point to initialize
	 * the extents of this object.
	 */
	void      update(const glm::vec3 &pt);

	/*
	 * This update function overload uses two points to initialize
	 * the extents of this object.
	 */
	void      update(const glm::vec3 &min, const glm::vec3 &max);

	/*
	 * This update function overload uses another AABB to initialize
	 * the extents of this object.
	 */
	void      update(const AABB &other);

	/*
		Used to properly position this AABB according to the transformational matrix artgument, T.
		
		AABB Transform algorithm by Jim Arvo
		See https://www.realtimerendering.com/resources/GraphicsGems/gems/TransBox.c
		
		Returns a new AABB that encloses the transformed AABB
	*/
	AABB      transform(glm::mat4 &T) const;

	/*
	 * This function tests to see if other overlaps with
	 * this AABB. If it does, true is returned, else false.
	 */
	bool      collides_with(const AABB &other) const;

	/*
	 * Calculates and returns the size of the AABB.
	 */
	glm::vec3 get_scale() const;

	/*
	 * Calculates and returns the point at the center of this AABB.
	 */
	glm::vec3 get_center() const;

	/*
	 * Accessor method for getting the minimum corner of this AABB.
	 */
	glm::vec3 get_min() const;

	/*
	 * Accessor method for getting the maximum corner of this AABB.
	 */
	glm::vec3 get_max() const;

	/*
	 * Resets the AABB extents.
	 */
	void      reset();

};	// class AABB

}	// namespace W3D::sg
// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "aabb.hpp"

namespace W3D::sg
{
AABB::AABB()
{
	reset();
}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max) :
    min_(min),
    max_(max)
{
}

std::type_index AABB::get_type()
{
	return typeid(AABB);
}

void AABB::update(const glm::vec3 &point)
{
	min_ = glm::min(min_, point);
	max_ = glm::max(max_, point);
}

void AABB::update(const glm::vec3 &min, const glm::vec3 &max)
{
	min_ = glm::min(min, min_);
	max_ = glm::max(max, max_);
}

void AABB::update(const AABB &other)
{
	min_ = glm::min(other.min_, min_);
	max_ = glm::max(other.max_, max_);
}

AABB AABB::transform(glm::mat4 &T) const
{
	float     a, b;
	glm::vec3 new_min, new_max;
	// Take care of translation
	new_min[0] = new_max[0] = T[3][0];
	new_min[1] = new_max[1] = T[3][1];
	new_min[2] = new_max[2] = T[3][2];

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			a = T[i][j] * min_[j];
			b = T[i][j] * max_[j];
			if (a < b)
			{
				new_min[i] += a;
				new_max[i] += b;
			}
			else
			{
				new_min[i] += b;
				new_max[i] += a;
			}
		}
	}

	return AABB(new_min, new_max);
}


bool AABB::collides_with(const AABB &other) const
{
	return min_.x <= other.max_.x && other.min_.x <= max_.x &&
	       min_.y <= other.max_.y && other.min_.y <= max_.y &&
	       min_.z <= other.max_.z && other.min_.z <= max_.z;
}

glm::vec3 AABB::get_scale() const
{
	return (max_ - min_);
}

glm::vec3 AABB::get_center() const
{
	return (min_ + max_) * 0.5f;
}

glm::vec3 AABB::get_min() const 
{
	return min_;
}

glm::vec3 AABB::get_max() const
{
	return max_;
}

void AABB::reset()
{
	min_ = std::numeric_limits<glm::vec3>::max();
	max_ = std::numeric_limits<glm::vec3>::min();
};
} // namespace W3D::sg
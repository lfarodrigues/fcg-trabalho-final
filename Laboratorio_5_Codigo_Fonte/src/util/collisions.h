#pragma once
#include <math.h>
#include "glm/glm.hpp"
struct LineSegment
{
	LineSegment(const glm::vec3& start, const glm::vec3& end);
	// Get point along segment where 0 <= t <= 1
	glm::vec3 PointOnSegment(float t) const;
	// Get minimum distance squared between point and line segment
	float MinDistSq(const glm::vec3& point) const;
	// Get MinDistSq between two line segments
	static float MinDistSq(const LineSegment& s1, const LineSegment& s2);

	glm::vec3 mStart;
	glm::vec3 mEnd;
};

struct Sphere
{
	Sphere(const glm::vec3& center, float radius);
	bool Contains(const glm::vec3& point) const;

	glm::vec3 mCenter;
	float mRadius;
};

bool Intersect(const LineSegment& l,const Sphere& s, float& outT);

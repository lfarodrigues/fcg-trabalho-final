#include <iostream>
#include "collisions.h"
LineSegment::LineSegment(const glm::vec3& start, const glm::vec3& end)
{
    mStart = start;
}

glm::vec3 LineSegment::PointOnSegment(float t) const
{
	return mStart + (mEnd - mStart) * t;
}

Sphere::Sphere(const glm::vec3& center, float radius)
	:mCenter(center)
	, mRadius(radius)
{
}

bool Intersect(const LineSegment& l,const Sphere& s, float &outT)
{
	// Compute X, Y, a, b, c as per equations
	glm::vec3 X = l.mStart - s.mCenter;
	glm::vec3 Y = l.mEnd - l.mStart;
	//printf("Length of Ray is: \n%d\n", Y.length());
	float a = glm::dot(Y, Y);
	float b = 2.0f * glm::dot(X, Y);
	float c = glm::dot(X, X) - s.mRadius * s.mRadius;
	// Compute discriminant
	float disc = b * b - 4.0f * a * c;
	if (disc < 0.0f)
	{
		return false;
	}
	else
	{
		disc = sqrt(disc);
		// Compute min and max solutions of t
		float tMin = (-b - disc) / (2.0f * a);
		float tMax = (-b + disc) / (2.0f * a);
		// Check whether either t is within bounds of segment
		if (tMin >= 0.0f && tMin <= 1.0f)
		{
			outT = tMin;
			return true;
		}
		else if (tMax >= 0.0f && tMax <= 1.0f)
		{
			outT = tMax;
			return true;
		}
		else
		{
			return false;
		}
	}
}

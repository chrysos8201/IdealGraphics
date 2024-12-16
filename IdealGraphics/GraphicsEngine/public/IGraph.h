#pragma once
#include "../Utils/SimpleMath.h"
namespace Ideal
{
	struct Point
	{
		float x;
		float y;

		Point() : x(0), y(0) {}
		Point(float nx, float ny) : x(nx), y(ny) {}
		Point operator*(float t) const
		{
			return { x * t, y * t };
		}
		Point operator+(const Point& other) const
		{
			return { x + other.x, y + other.y };
		}
	};

	class IBezierCurve
	{
	public:
		virtual void AddControlPoint(Point p) abstract;
		virtual Point GetPoint(float t) abstract;

	};

	class IGradient
	{
	public:
		virtual void AddPoint(DirectX::SimpleMath::Color color, float position) abstract;
		virtual DirectX::SimpleMath::Color GetColorAtPosition(float t) abstract;
	};
}
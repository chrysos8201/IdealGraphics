#pragma once
#include "GraphicsEngine/public/IGraph.h"
//#include "IGraph.h"
#include "Core/Core.h"
using namespace DirectX::SimpleMath;

namespace Ideal
{
	class BezierCurve : public Ideal::IBezierCurve
	{
	public:

	public:
		virtual void AddControlPoint(Point p) override;
		virtual Point GetPoint(float t)override;
	private:
		Point Bezier(float t);

	private:
		std::vector<Point> m_points;
		std::unordered_map<int32, Point> m_cache;
	};

	class Gradient : public Ideal::IGradient
	{
		struct GradientPoint
		{
			Color color;

			float position;
			GradientPoint(Color ncolor, float nposition) : color(ncolor), position(nposition) {};
		};

	public:
		virtual void AddPoint(Color color, float position) override;
		virtual Color GetColorAtPosition(float t) override;

	private:
		std::vector<Ideal::Gradient::GradientPoint> m_points;
	};
}
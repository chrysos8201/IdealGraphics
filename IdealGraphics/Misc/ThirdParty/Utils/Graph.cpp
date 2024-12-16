#include "Graph.h"

void Ideal::BezierCurve::AddControlPoint(Ideal::Point p)
{
	m_points.push_back(p);
}

Ideal::Point Ideal::BezierCurve::GetPoint(float t)
{
	//int32 key = static_cast<int32>(t * 100);
	//if (m_cache.find(key) != m_cache.end())
	//{
	//	return m_cache[key];
	//}
	t = std::clamp(t, 0.0f, 1.0f);
	Ideal::Point point = Bezier(t);
	//m_cache[key] = point;

	return point;
}

Ideal::Point Ideal::BezierCurve::Bezier(float t)
{
	int n = m_points.size();

	if (n == 0) return Point(0, 0);

	if (n < 2) return m_points[0];

	std::vector<Ideal::Point> tempPoints = m_points;

	for (int k = 1; k < n; ++k) {
		for (int i = 0; i < n - k; ++i) {
			tempPoints[i] = tempPoints[i] * (1 - t) + tempPoints[i + 1] * t;
		}
	}

	// 최종적으로 얻은 곡선 위의 점 반환
	return tempPoints[0];
}

void Ideal::Gradient::AddPoint(Color color, float position)
{
	m_points.push_back(GradientPoint(color, position));
}

DirectX::SimpleMath::Color Ideal::Gradient::GetColorAtPosition(float t)
{
	if (m_points.size() == 0)
		return Color(0,0,0,1);
	GradientPoint* p1 = nullptr;
	GradientPoint* p2 = nullptr;

	// t 위치에 맞는 두 포인트를 찾고
	for (uint32 i = 0; i < m_points.size() - 1; ++i)
	{
		if (m_points[i].position <= t && m_points[i + 1].position >= t)
		{
			p1 = &m_points[i];
			p2 = &m_points[i + 1];
			break;
		}
	}
	if (p1 == nullptr || p2 == nullptr) return m_points.back().color;

	// 두 포인트 간의 비율 계산
	float localT = (t - p1->position) / (p2->position - p1->position);

	// 색상 및 알파 값 보간
	float r = p1->color.R() + localT * (p2->color.R() - p1->color.R());
	float g = p1->color.G() + localT * (p2->color.G() - p1->color.G());
	float b = p1->color.B() + localT * (p2->color.B() - p1->color.B());
	float a = p1->color.A() + localT * (p2->color.A() - p1->color.A());

	return Color(r, g, b, a);
}

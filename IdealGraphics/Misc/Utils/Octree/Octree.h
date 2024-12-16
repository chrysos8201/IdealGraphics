#pragma once
#include "Core/Core.h"

//namespace Ideal
//{
struct Bounds
{
	Bounds()
	{
		m_Center = Vector3(0, 0, 0);
		m_Extents = Vector3(0, 0, 0);
	}
	Bounds(Vector3 center, Vector3 size)
	{
		m_Center = center;
		m_Extents = size * 0.5f;
	}

	Vector3 GetSize() { return m_Extents * 2.f; }
	void SetSize(Vector3 Value) { m_Extents = Value * 0.5f; }

	Vector3 GetCenter() { return m_Center; }
	void SetCenter(Vector3 Center) { m_Center = Center; }

	Vector3 GetExtents() { return m_Extents; }

	Vector3 GetMin()
	{
		m_min = m_Center - m_Extents;
		return m_min;
	}
	void SetMin(Vector3 min) { SetMinMax(min, m_max); }

	Vector3 GetMax()
	{
		m_max = m_Center + m_Extents;
		return m_max;
	}
	void SetMax(Vector3 max) { SetMinMax(m_min, max); }

	bool Contains(Vector3 Point)
	{
		Vector3 minBounds = m_Center - m_Extents;
		Vector3 maxBounds = m_Center + m_Extents;
		bool b = (
			Point.x >= minBounds.x && Point.x <= maxBounds.x &&
			Point.y >= minBounds.y && Point.y <= maxBounds.y &&
			Point.z >= minBounds.z && Point.z <= maxBounds.z
			);
		return b;
	}

	bool Intersects(Bounds bounds)
	{
		return m_min.x <= bounds.m_max.x && m_max.x >= bounds.m_min.x && m_min.y <= bounds.m_max.y && m_max.y >= bounds.m_min.y && m_min.z <= bounds.m_max.z && m_max.z >= bounds.m_min.z;
	}

	bool IsCompletelyInside(Bounds objectBounds)
	{
		// 오브젝트가 노드의 경계 내에 완전히 포함되는지 확인
		return GetMin().x <= objectBounds.GetMin().x && GetMax().x >= objectBounds.GetMax().x &&
			GetMin().y <= objectBounds.GetMin().y && GetMax().y >= objectBounds.GetMax().y &&
			GetMin().z <= objectBounds.GetMin().z && GetMax().z >= objectBounds.GetMax().z;
	}

	void Encapsulate(Vector3 point)
	{
		auto min = Vector3::Min(GetMin(), point);
		auto max = Vector3::Max(GetMax(), point);
		SetMinMax(min, max);
	}

	void Encapsulate(Bounds bounds)
	{
		Encapsulate(bounds.GetCenter() - bounds.GetExtents());
		Encapsulate(bounds.GetCenter() + bounds.GetExtents());
	}

	void SetMinMax(Vector3 min, Vector3 max)
	{
		m_Extents = (max - min) * 0.5f;
		m_Center = min + m_Extents;
	}

	std::vector<std::pair<Vector3, Vector3>> GetEdges()
	{
		std::vector<std::pair<Vector3, Vector3>> edges;

		// 정점 계산
		Vector3 min = GetMin();
		Vector3 max = GetMax();

		// 정점 배열
		Vector3 vertices[8] = {
			min,
			Vector3(max.x, min.y, min.z),
			Vector3(min.x, max.y, min.z),
			Vector3(max.x, max.y, min.z),
			Vector3(min.x, min.y, max.z),
			Vector3(max.x, min.y, max.z),
			Vector3(min.x, max.y, max.z),
			max
		};

		// 선 정의 (각 선의 시작점과 끝점)
		edges.push_back({ vertices[0], vertices[1] }); // (0, 1)
		edges.push_back({ vertices[0], vertices[2] }); // (0, 2)
		edges.push_back({ vertices[0], vertices[4] }); // (0, 4)
		edges.push_back({ vertices[1], vertices[3] }); // (1, 3)
		edges.push_back({ vertices[1], vertices[5] }); // (1, 5)
		edges.push_back({ vertices[2], vertices[3] }); // (2, 3)
		edges.push_back({ vertices[2], vertices[6] }); // (2, 6)
		edges.push_back({ vertices[3], vertices[7] }); // (3, 7)
		edges.push_back({ vertices[4], vertices[5] }); // (4, 5)
		edges.push_back({ vertices[4], vertices[6] }); // (4, 6)
		edges.push_back({ vertices[5], vertices[7] }); // (5, 7)
		edges.push_back({ vertices[6], vertices[7] }); // (6, 7)

		return edges;
	}

private:
	Vector3 m_Center;
	Vector3 m_Extents;
	Vector3 m_min;
	Vector3 m_max;
};

template <typename T>
class OctreeNode : public std::enable_shared_from_this<OctreeNode<T>>
{
public:
	OctreeNode()
	{

	}

	OctreeNode(Bounds Bound, float MinNodeSize)
	{
		m_nodeBounds = Bound;
		m_minSize = MinNodeSize;

		float quarter = m_nodeBounds.GetSize().y / 4.0f;
		float childLength = m_nodeBounds.GetSize().y / 2.0f;
		Vector3 childSize = Vector3(childLength, childLength, childLength);
		m_childBounds.resize(8);
		m_childBounds[0] = Bounds(m_nodeBounds.GetCenter() + Vector3(-quarter, quarter, -quarter), childSize);
		m_childBounds[1] = Bounds(m_nodeBounds.GetCenter() + Vector3(quarter, quarter, -quarter), childSize);
		m_childBounds[2] = Bounds(m_nodeBounds.GetCenter() + Vector3(-quarter, quarter, quarter), childSize);
		m_childBounds[3] = Bounds(m_nodeBounds.GetCenter() + Vector3(quarter, quarter, quarter), childSize);
		m_childBounds[4] = Bounds(m_nodeBounds.GetCenter() + Vector3(-quarter, -quarter, -quarter), childSize);
		m_childBounds[5] = Bounds(m_nodeBounds.GetCenter() + Vector3(quarter, -quarter, -quarter), childSize);
		m_childBounds[6] = Bounds(m_nodeBounds.GetCenter() + Vector3(-quarter, -quarter, quarter), childSize);
		m_childBounds[7] = Bounds(m_nodeBounds.GetCenter() + Vector3(quarter, -quarter, quarter), childSize);
	}

	Bounds GetBounds() { return m_nodeBounds; }

	void DivideAndAdd(T Object, const Bounds& objectBounds)
	{
		// 최소 크기 이하로 분할할 수 없으면 이 노드에 오브젝트 추가
		if (m_nodeBounds.GetSize().y <= m_minSize)
		{
			m_objects.push_back(Object);
			m_isFinalNode = true;
			return;
		}
		// 자식 노드가 없다면 초기화
		if (m_childOctreeNodes.size() == 0)
		{
			m_childOctreeNodes.resize(8);
		}
		bool addedToChild = false;
		// 자식 노드를 순회하면서 오브젝트가 속할 자식 노드를 찾는다
		for (int i = 0; i < 8; ++i)
		{
			if (m_childOctreeNodes[i] == nullptr)
			{
				m_childOctreeNodes[i] = std::make_shared<OctreeNode<T>>(m_childBounds[i], m_minSize);
			}

			// 경계가 자식 노드와 겹치면 그 자식 노드에 추가 시도
			//if (m_childBounds[i].Intersects(objectBounds))
			if (m_childBounds[i].IsCompletelyInside(objectBounds))
			{
				addedToChild = true;
				m_childOctreeNodes[i]->DivideAndAdd(Object, objectBounds);
				break;
			}
		}
		// 자식노드에 추가되지 않으면 상위 노드에 추가
		if (!addedToChild)
		{
			//m_childOctreeNodes.clear();
			m_isFinalNode = true;
			m_objects.push_back(Object);
		}
	}

	void DivideAndAdd(T Object, Vector3 Position)
	{
		if (m_nodeBounds.GetSize().y <= m_minSize)
		{
			m_objects.push_back(Object);
			m_isFinalNode = true;
			return;
		}
		if (m_childOctreeNodes.size() == 0)
		{
			m_childOctreeNodes.resize(8);
		}
		bool dividing = false;
		for (int i = 0; i < 8; ++i)
		{
			if (m_childOctreeNodes[i] == nullptr)
			{
				m_childOctreeNodes[i] = std::make_shared<OctreeNode<T>>(m_childBounds[i], m_minSize);
			}
		}
		for (int i = 0; i < 8; ++i)
		{
			if (m_childBounds[i].Contains(Position))
			{
				dividing = true;
				m_childOctreeNodes[i]->DivideAndAdd(Object, Position);
				break;
			}
		}
		if (dividing == false)
		{
			m_childOctreeNodes.clear();
		}
	}
	bool IsFinalNode()
	{
		return m_isFinalNode;
	}
	// 모든 노드를 순회
	template <typename Func>
	void ForeachNodeInternal(Func func)
	{
		// 일단 나부터
		func(shared_from_this());
		// 그리고 자식노드도 다시
		for (auto& c : m_childOctreeNodes)
		{
			if (c)
			{
				c->ForeachNodeInternal(func);
			}
		}
	}

	const std::vector<T>& GetObjects()
	{
		return m_objects;
	}

private:
	Bounds m_nodeBounds;
	std::vector<Bounds> m_childBounds;
	std::vector<std::shared_ptr<OctreeNode<T>>> m_childOctreeNodes;
	float m_minSize;
	std::vector<T> m_objects;
	bool m_isFinalNode = false;
};

template <typename T>
class Octree
{
	template <typename T>
	struct Object
	{
		T object;
		Vector3 pos;
	};

	template <typename T>
	struct ObjectBound
	{
		T object;
		Bounds bounds;
	};

public:

	void AddObject(T object, Vector3 Position)
	{
		m_bounds.Encapsulate(Position);
		Object<T> obj;
		obj.object = object;
		obj.pos = Position;
		m_objects.push_back(obj);
	}

	void AddObject(T object, Bounds bounds)
	{
		m_bounds.Encapsulate(bounds);
		ObjectBound<T> obj;
		obj.object = object;
		obj.bounds = bounds;
		m_objectsBoundVer.push_back(obj);
	}

	void Bake(float minNodeSize)
	{
		float maxSize = max(m_bounds.GetSize().x, max(m_bounds.GetSize().y, m_bounds.GetSize().z));

		Vector3 sizeVector = Vector3(maxSize, maxSize, maxSize) * 0.5f;
		m_bounds.SetMinMax(m_bounds.GetCenter() - sizeVector, m_bounds.GetCenter() + sizeVector);
		m_rootNode = std::make_shared<OctreeNode<T>>(m_bounds, minNodeSize);

		for (auto& o : m_objects)
		{
			m_rootNode->DivideAndAdd(o.object, o.pos);
		}
	}

	void BakeBoundVer(float minNodeSize)
	{
		float maxSize = max(m_bounds.GetSize().x, max(m_bounds.GetSize().y, m_bounds.GetSize().z));

		Vector3 sizeVector = Vector3(maxSize, maxSize, maxSize) * 0.5f;
		m_bounds.SetMinMax(m_bounds.GetCenter() - sizeVector, m_bounds.GetCenter() + sizeVector);
		m_rootNode = std::make_shared<OctreeNode<T>>(m_bounds, minNodeSize);

		for (auto& o : m_objectsBoundVer)
		{
			m_rootNode->DivideAndAdd(o.object, o.bounds);
		}
	}

	template<typename Func>
	void ForeachNodeInternal(Func func)
	{
		m_rootNode->ForeachNodeInternal(func);
	}

	void GetFinalNodes(std::vector<std::shared_ptr<OctreeNode<T>>>& nodes)
	{
		m_rootNode->ForeachNodeInternal([&](std::shared_ptr<OctreeNode<T>> Node) {if (Node->IsFinalNode()) nodes.push_back(Node); });
		//m_rootNode->Foreac ([&](std::shared_ptr<OctreeNode<T>> Node) {if (Node->IsFinalNode()) nodes.push_back(Node); });

		// TODO : 자식
	}

private:
	Bounds m_bounds;
	std::vector<Object<T>> m_objects;
	std::vector<ObjectBound<T>> m_objectsBoundVer;
	std::shared_ptr<OctreeNode<T>> m_rootNode;
};
//}

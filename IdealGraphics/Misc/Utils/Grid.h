#pragma once
#include "Core/Core.h"

namespace Ideal
{
	namespace Utils
	{
		template<typename T>
		struct GridCell {
			std::vector<T> objects; // 이 셀에 포함된 물체
		};

		template<typename T>
		class Grid
		{
		public:
			Grid()
				: m_cellCount(0), m_offset(0)
			{

			}

			Grid(uint32 CellCount)
				:m_cellCount(CellCount)
			{
				m_offset = int32(float(CellCount) * 0.5f);
				m_GridCells.resize(CellCount, std::vector<GridCell<T>>(CellCount));
			}

			void Add(float x, float y, T object)
			{
				int32 nx = static_cast<int32>(x) + m_offset;
				int32 ny = static_cast<int32>(y) + m_offset;


				// offset을 더해도 -일때
				while (nx < 0 || ny < 0 || nx > m_GridCells.size() -1 || ny > m_GridCells.size() - 1)
				{
					ResizeGrid();
					nx = static_cast<int32_t>(std::floor(x)) + m_offset;
					ny = static_cast<int32_t>(std::floor(y)) + m_offset;
				}

				m_GridCells[ny][nx].objects.push_back(object);
			}
			std::vector<std::vector<GridCell<T>>>& GetGrids() { return m_GridCells; }

		private:
			void ResizeGrid() {
				m_cellCount *= 2;
				m_offset = int32(float(m_cellCount) * 0.5f);;
				std::vector<std::vector<GridCell<T>>> newGridCells(m_cellCount, std::vector<GridCell<T>>(m_cellCount));

				// 기존 셀의 내용을 새로운 그리드로 복사
				for (uint32 y = 0; y < m_GridCells.size(); ++y)
				{
					for (uint32 x = 0; x < m_GridCells[y].size(); ++x)
					{
						int32 ny = y + m_offset;
						int32 nx = x + m_offset;
						if (nx >= 0 && ny >= 0)
						{
							newGridCells[ny][nx] = m_GridCells[y][x];
						}
					}
				}

				m_GridCells = std::move(newGridCells); // 새 그리드로 교체
			}
		private:
			uint32 m_offset = 0;
			uint32 m_cellCount = 0;
			std::vector<std::vector<GridCell<T>>> m_GridCells;
		};
	}
}

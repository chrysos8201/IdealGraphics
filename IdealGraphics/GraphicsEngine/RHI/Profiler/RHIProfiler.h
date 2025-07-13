#pragma once
#include "Core/Core.h"
#include "RHI/RHIPoolAllocator.h"
#include "RHI/RHIMemoryPool.h"
#include "imgui.h"

namespace Ideal
{
	namespace Visualization
	{
		class RHIProfiler
		{
		public:
			static void ProfilePoolAllocator(RHIPoolAllocator* PoolAllocator, int32 Index)
			{
				std::string WindowName = "PoolAllocator" + std::to_string(Index);
				ImGui::Begin(WindowName.c_str());

				int PoolIndex = 0;
				for (const auto& Allocator : PoolAllocator->Pools)
				{
					if (Allocator == nullptr) continue;
					ImGui::PushID(PoolIndex++);

					ImDrawList* DrawList = ImGui::GetWindowDrawList();
					ImVec2 CanvasPos = ImGui::GetCursorScreenPos();
					ImVec2 CanvasSize = ImVec2(ImGui::GetContentRegionAvail().x, 30.f);

					DrawList->AddRectFilled(CanvasPos, ImVec2(CanvasPos.x + CanvasSize.x, CanvasPos.y + CanvasSize.y), IM_COL32(200, 80, 80, 255));

					for (const auto& FreeBlock : Allocator->FreeBlocks)
					{
						float xStart = (float)(FreeBlock->GetOffset()) / Allocator->GetPoolSize() * CanvasSize.x;
						float blockWidth = (float)(FreeBlock->GetSize()) / Allocator->GetPoolSize() * CanvasSize.x;

						DrawList->AddRectFilled(
							ImVec2(CanvasPos.x + xStart, CanvasPos.y),
							ImVec2(CanvasPos.x + xStart + blockWidth, CanvasPos.y + CanvasSize.y),
							IM_COL32(80, 200, 80, 255)
						);

					}
					ImGui::Dummy(ImVec2(0.f, CanvasSize.y + ImGui::GetStyle().ItemSpacing.y));
					ImGui::PopID();
				}
				ImGui::End();
			}
		};
	}
}
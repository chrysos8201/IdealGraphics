#pragma once
#include "Core/Core.h"
#include "GraphicsEngine/Resources/D3D12ThirdParty.h"

namespace Ideal
{
	class Bone
	{
		friend class AssimpConverter;

	public:
		Bone();
		virtual ~Bone();

	public:

	private:
		std::string name;
		int32 index = -1;
		int32 parent = -1;
		Matrix transform;
	};
}
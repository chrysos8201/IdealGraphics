#pragma once
#include "GraphicsEngine/Resource/ResourceBase.h"
#include "Core/Core.h"
#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

namespace Ideal
{
	class Model;
}

namespace Ideal
{
	class Bone : public ResourceBase
	{
		friend class Ideal::Model;
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
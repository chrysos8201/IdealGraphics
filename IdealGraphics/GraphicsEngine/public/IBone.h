#pragma once
#include <string>
#include "../Utils/SimpleMath.h"

namespace Ideal
{
	class IBone
	{
	public:
		IBone() {}
		virtual ~IBone() {}

	public:
		virtual	const std::string& GetName() const abstract;
		virtual const DirectX::SimpleMath::Matrix& GetTransform() const abstract;
	};
}

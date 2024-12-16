#pragma once
#include <memory>
#include <string>

namespace Ideal
{
	class IMaterial;
}

namespace Ideal
{
	class IMesh
	{
	public:
		IMesh() {}
		virtual ~IMesh() {}

	public:
		virtual std::string GetName() abstract;
		// 아래의 Set Material은 Particle과 호환이 안됨.
		virtual void SetMaterialObject(std::shared_ptr<Ideal::IMaterial> Material) abstract;

		// 처음에는 기본적인 material을 사용하긴 하는데 원래 무슨 머테리얼을 사용하는지 뽑아준다.
		virtual std::string GetFBXMaterialName() abstract;
		// 나중에 이거 없앨듯
		virtual std::weak_ptr<Ideal::IMaterial> GetMaterialObject() abstract;
	};
}
#pragma once
#include <memory>

namespace Ideal
{
	class IMeshObject;
	class ILight;
}

namespace Ideal
{
	class IRenderScene
	{
	public:
		IRenderScene() {};
		virtual ~IRenderScene() {};

	public:
		virtual void AddObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) abstract;
		virtual void AddDebugObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) abstract;
		virtual void AddLight(std::shared_ptr<Ideal::ILight> Light) abstract;
	};
}

#pragma once
#include "../Utils/SimpleMath.h"
using namespace DirectX::SimpleMath;

namespace Ideal
{
	class ICamera
	{
	public:
		ICamera() {};
		virtual ~ICamera() {};

	public:
		virtual void SetLens(float FovY, float Aspect, float NearZ, float FarZ) abstract;
		virtual void SetLensWithoutAspect(float FovY, float NearZ, float FarZ) abstract;

		virtual void Walk(float Distance) abstract;
		virtual void Strafe(float Distance) abstract;
		virtual void Pitch(float Angle) abstract;
		virtual void RotateY(float Angle) abstract;

		virtual Vector3 GetPosition() abstract;
		virtual void SetPosition(const Vector3& Position) abstract;
		virtual void SetLook(Vector3 Look) abstract;
		virtual Vector3 GetLook() abstract;
	};
}
#pragma once
#include <memory>
#include "../Utils/SimpleMath.h"
#include "IGraph.h"

namespace Ideal
{
	class IMesh;
	class IParticleMaterial;
}
namespace Ideal
{
	namespace ParticleMenu
	{
		// 범위 설정에 따른 Enum
		enum class ERangeMode
		{
			Constant = 0, // 상수 
			Curve,
			RandomBetweenConstants,	// 두 개의 상수 사이의 랜덤 값
			RandomBetweenCurves,	// 두 개의 커브 사이의 랜덤 값
		};

		enum class ERendererMode
		{
			Billboard,	// 카메라만 바라봄
			Mesh,	// 매쉬 형태로 매쉬에 머테리얼이 입혀진다.
		};

		struct ShaderData
		{
			DirectX::SimpleMath::Vector4 CustomData1;
			DirectX::SimpleMath::Vector4 CustomData2;
			float Time;
		};

		enum class ECustomData
		{
			CustomData_1 = 0,
			CustomData_2,
		};
		enum class ECustomDataParameter : unsigned int
		{
			CustomData_x = 0,
			CustomData_y,
			CustomData_z,
			CustomData_w
		};
		
		enum class EShape
		{
			Circle = 0,
		};

		enum class EMode
		{
			Const = 0,
			Random
		};
	}
}
namespace Ideal
{
	class IParticleSystem
	{
	public:
		IParticleSystem() {};
		virtual ~IParticleSystem() {};

	public:
		virtual void SetMaterial(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial) abstract;
		virtual std::weak_ptr<Ideal::IParticleMaterial> GetMaterial() abstract;

		virtual void SetTransformMatrix(const DirectX::SimpleMath::Matrix& Transform) abstract;
		virtual const DirectX::SimpleMath::Matrix& GetTransformMatrix() const abstract;

		virtual void SetActive(bool IsActive) abstract;
		virtual bool GetActive() abstract;

		virtual void SetDeltaTime(float DT) abstract;

		virtual void Play() abstract;
		virtual void Resume() abstract;	// 일시 정지 했을 경우 지금 시간부터 시작
		virtual void Pause() abstract;

		virtual void SetStopWhenFinished(bool StopWhenFinished) abstract;
		virtual void SetPlayOnWake(bool PlayOnWake) abstract;

		virtual float GetCurrentDurationTime() abstract;

		virtual void SetMaxParticles(unsigned int MaxParticles) abstract;

	public:
		virtual void SetStartColor(const DirectX::SimpleMath::Color& StartColor) abstract;
		virtual DirectX::SimpleMath::Color& GetStartColor() abstract;

		virtual void SetStartSize(float Size) abstract;

		// 생명 시간
		virtual void SetStartLifetime(float Time) abstract; // 내부 지속 시간	( 이펙트가 끝나도 duration이 끝나야 다시 Loop를 하든 끝나든함)
		virtual float GetStartLifetime() abstract;

		// 재생 속도 값
		virtual void SetSimulationSpeed(float Speed) abstract;
		virtual float GetSimulationSpeed() abstract;

		// 총 지속 시간
		virtual void SetDuration(float Time) abstract;	// 총 지속 시간
		virtual float GetDuration() abstract;

		virtual void SetLoop(bool Loop) abstract;	
		virtual bool GetLoop() abstract;

		//-------Emission-------//
		virtual void SetRateOverTime(bool Active) abstract;
		// 1초마다 생성할 수 있는 최대 개수
		virtual void SetEmissionRateOverTime(float Count) abstract;

		//------Shape------//
		virtual void SetShapeMode(bool Active) abstract;
		virtual void SetShape(const Ideal::ParticleMenu::EShape& Shape) abstract;
		virtual void SetRadius(float Radius) abstract;
		// 0~1사이의 비율로 현재 반지름의 두께를 결정한다. 최대 반지름에서 안쪽으로 늘어나는 구조. 
		virtual void SetRadiusThickness(float RadiusThickness) abstract;

		//------Velocity Over Lifetime------//
		// 개별 파티클의 방향을 랜덤 or 동일
		virtual void SetVelocityOverLifetime(bool Active) abstract;
		virtual void SetVelocityDirectionMode(const Ideal::ParticleMenu::EMode& Mode) abstract;
		virtual void SetVelocityDirectionRandom(float Min, float Max) abstract;
		virtual void SetVelocityDirectionConst(const DirectX::SimpleMath::Vector3& Direction) abstract;
		// 개별 파티클의 속도를 랜덤 or 동일
		virtual void SetVelocitySpeedModifierMode(const Ideal::ParticleMenu::EMode& Mode) abstract;
		virtual void SetVelocitySpeedModifierRandom(float Min, float Max) abstract;
		virtual void SetVelocitySpeedModifierConst(float Speed) abstract;

		//------Color Over Lifetime------//
		// 생명 시간 동안 색상 값이 바뀔지를 결정. 그라디언트 그래프를 통해 조작함
		virtual void SetColorOverLifetime(bool Active) abstract;
		virtual Ideal::IGradient& GetColorOverLifetimeGradientGraph() abstract;

		//----Rotation Over Lifetime----//
		// 생명 시간 동안 회전 값 조절. 베지어 커브 그래프 사용
		virtual void SetRotationOverLifetime(bool Active) abstract;
		virtual Ideal::IBezierCurve& GetRotationOverLifetimeAxisX() abstract;
		virtual Ideal::IBezierCurve& GetRotationOverLifetimeAxisY() abstract;
		virtual Ideal::IBezierCurve& GetRotationOverLifetimeAxisZ() abstract;

		//----Size Over Lifetime----//
		virtual void SetSizeOverLifetime(bool Active) abstract;
		virtual Ideal::IBezierCurve& GetSizeOverLifetimeAxisX() abstract;
		virtual Ideal::IBezierCurve& GetSizeOverLifetimeAxisY() abstract;
		virtual Ideal::IBezierCurve& GetSizeOverLifetimeAxisZ() abstract;

		//----------Custom Data---------//
		virtual Ideal::IBezierCurve& GetCustomData1X() abstract;
		virtual Ideal::IBezierCurve& GetCustomData1Y() abstract;
		virtual Ideal::IBezierCurve& GetCustomData1Z() abstract;
		virtual Ideal::IBezierCurve& GetCustomData1W() abstract;

		virtual Ideal::IBezierCurve& GetCustomData2X() abstract;
		virtual Ideal::IBezierCurve& GetCustomData2Y() abstract;
		virtual Ideal::IBezierCurve& GetCustomData2Z() abstract;
		virtual Ideal::IBezierCurve& GetCustomData2W() abstract;

		//-------Texture Sheet Animation-------//
		virtual void SetTextureSheetAnimation(bool Active) abstract;
		virtual void SetTextureSheetAnimationTiles(const DirectX::SimpleMath::Vector2& Tiles) abstract;

		//-----------Renderer-----------//
		virtual void SetRenderMode(Ideal::ParticleMenu::ERendererMode ParticleRendererMode) abstract;
		virtual void SetRenderMesh(std::shared_ptr<Ideal::IMesh> ParticleRendererMesh) abstract;
		virtual void SetRenderMaterial(std::shared_ptr<Ideal::IParticleMaterial> ParticleRendererMaterial) abstract;
	};
}
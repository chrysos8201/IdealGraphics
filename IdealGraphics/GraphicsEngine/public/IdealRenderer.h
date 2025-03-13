#pragma once
#include <string>
#include <memory>
#include "../Utils/SimpleMath.h"


namespace Ideal
{
	class ICamera;
	class IMeshObject;
	class IAnimation;
	class ISkinnedMeshObject;
	class IRenderScene;
	class IDirectionalLight;
	class IPointLight;
	class ISpotLight;
	class ITexture;
	class IMaterial;
	class IMesh;
	class ISprite;
	class IText;
	class IShader;
	class IParticleSystem;
	class IParticleMaterial;
	class ILight;
}

namespace Ideal
{
	struct DisplayResolution
	{
		unsigned int Width;
		unsigned int Height;
	};

	namespace Resolution
	{
		enum EDisplayResolutionOption
		{
			R_800_600,
			R_1200_900,
			R_1280_720,
			R_1920_1080,
			R_1920_1200,
			R_2560_1440,
			R_3440_1440,
			R_3840_2160,
			Count
		};
	}

	class IdealRenderer
	{
	public:
		IdealRenderer() {};
		virtual ~IdealRenderer() {};

	public:
		virtual void Init() abstract;
		virtual void Tick() abstract;
		virtual void ShutDown() abstract;
		virtual void Render() abstract;
		virtual void Resize(UINT Width, UINT Height) abstract;
		virtual void ToggleFullScreenWindow() abstract;
		virtual bool IsFullScreen() abstract;
		virtual void SetDisplayResolutionOption(const Resolution::EDisplayResolutionOption&) abstract;

	public:
		virtual std::shared_ptr<ICamera>					CreateCamera() abstract;
		virtual void										SetMainCamera(std::shared_ptr<ICamera> Camera) abstract;

		virtual std::shared_ptr<Ideal::IMeshObject>			CreateStaticMeshObject(const std::wstring& FileName, bool AddRayTracing = true) abstract;
		virtual std::shared_ptr<Ideal::ISkinnedMeshObject>	CreateSkinnedMeshObject(const std::wstring& FileName) abstract;
		virtual std::shared_ptr<Ideal::IAnimation> CreateAnimation(const std::wstring& FileName, const DirectX::SimpleMath::Matrix& offset = DirectX::SimpleMath::Matrix::Identity) abstract;

		// CreateDebugMeshObject : 레이트레이싱에서는 그냥 Static Mesh Object 반환 //
		virtual std::shared_ptr<Ideal::IMeshObject>			CreateDebugMeshObject(const std::wstring& FileName) abstract;

		virtual void										DeleteMeshObject(std::shared_ptr<Ideal::IMeshObject> MeshObject) abstract;
		virtual void										DeleteDebugMeshObject(std::shared_ptr<Ideal::IMeshObject> DebugMeshObject) abstract;

		virtual std::shared_ptr<Ideal::IDirectionalLight>	CreateDirectionalLight() abstract;
		virtual std::shared_ptr<Ideal::ISpotLight>			CreateSpotLight() abstract;
		virtual std::shared_ptr<Ideal::IPointLight>			CreatePointLight() abstract;

		virtual void DeleteLight(std::shared_ptr<Ideal::ILight> Light) abstract;

		virtual void SetSkyBox(std::shared_ptr<Ideal::ITexture> SkyBoxTexture) abstract;
		virtual std::shared_ptr<Ideal::ITexture> CreateSkyBox(const std::wstring& FileName) abstract;

		// Texture
		virtual std::shared_ptr<Ideal::ITexture> CreateTexture(const std::wstring& FileName, bool IsGenerateMips = false, bool IsNormalMap = false, bool IngoreSRGB = false) abstract;
		virtual std::shared_ptr<Ideal::IMaterial>			CreateMaterial() abstract;

		virtual void DeleteTexture(std::shared_ptr<Ideal::ITexture> Texture) abstract;
		virtual void DeleteMaterial(std::shared_ptr<Ideal::IMaterial> Material) abstract;

		// Sprite : UI에 만들어진다.
		virtual std::shared_ptr<Ideal::ISprite>				CreateSprite() abstract;
		virtual void DeleteSprite(std::shared_ptr<Ideal::ISprite>&) abstract;

		// Text
		virtual std::shared_ptr<Ideal::IText> CreateText(unsigned int Width, unsigned int Height, float FontSize, std::wstring Font = L"Tahoma") abstract;
		virtual void DeleteText(std::shared_ptr<Ideal::IText>& Text) abstract;

		// Shader
		virtual void CompileShader(const std::wstring& FilePath, const std::wstring& SavePath, const std::wstring& SaveName, const std::wstring& ShaderVersion, const std::wstring& EntryPoint = L"Main", const std::wstring& IncludeFilePath = L"", bool HasEntry = true) abstract;// 셰이더를 컴파일하여 저장. 한 번만 하면 됨.
		// 이름으로 컴파일 된 셰이더를 불러와 Shader 객체를 반환한다.
		virtual std::shared_ptr<Ideal::IShader> CreateAndLoadParticleShader(const std::wstring& Name) abstract;	// 이름을 파일 경로로 할까

		// Particle
		virtual std::shared_ptr<Ideal::IParticleSystem> CreateParticleSystem(std::shared_ptr<Ideal::IParticleMaterial> ParticleMaterial) abstract;
		virtual void DeleteParticleSystem(std::shared_ptr<Ideal::IParticleSystem>& ParticleSystem) abstract;

		// ParticleMaterial
		virtual std::shared_ptr<Ideal::IParticleMaterial> CreateParticleMaterial() abstract;
		virtual void DeleteParticleMaterial(std::shared_ptr<Ideal::IParticleMaterial>& ParticleMaterial) abstract;

		// ParticleMesh
		virtual std::shared_ptr<Ideal::IMesh> CreateParticleMesh(const std::wstring& FileName) abstract;

		virtual void SetRendererAmbientIntensity(float Value) abstract;

	public:
		virtual void SetAssetPath(const std::wstring& AssetPath) abstract;
		virtual void SetModelPath(const std::wstring& ModelPath) abstract;
		virtual void SetTexturePath(const std::wstring& TexturePath) abstract;

		virtual void ConvertAssetToMyFormat(std::wstring FileName, bool isSkinnedData = false, bool NeedVertexInfo = false, bool NeedConvertCenter = false) abstract;
		virtual void ConvertAnimationAssetToMyFormat(std::wstring FileName) abstract;

		virtual void ConvertParticleMeshAssetToMyFormat(std::wstring FileName, bool SetScale = false, DirectX::SimpleMath::Vector3 Scale = DirectX::SimpleMath::Vector3(1.f)) abstract;

	public:
		virtual bool SetImGuiWin32WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) abstract;
		virtual void ClearImGui() abstract;
		virtual DirectX::SimpleMath::Vector2 GetTopLeftEditorPos() abstract;
		virtual DirectX::SimpleMath::Vector2 GetRightBottomEditorPos() abstract;
	};
}
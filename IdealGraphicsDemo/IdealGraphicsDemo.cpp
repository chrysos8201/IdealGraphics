// IdealGraphicsDemo.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "IdealGraphicsDemo.h"

#ifdef _DEBUG
#pragma comment(lib, "DebugLib/GraphicsEngine/IdealGraphics.lib")
#else
#pragma comment(lib, "ReleaseLib/GraphicsEngine/IdealGraphics.lib")
#endif

#include <iostream>
using namespace std;

#include "GraphicsEngine/public/IdealRendererFactory.h"
#include "GraphicsEngine/public/IdealRenderer.h"
#include "GraphicsEngine/public/IMeshObject.h"
#include "GraphicsEngine/public/ISkinnedMeshObject.h"
#include "GraphicsEngine/public/IAnimation.h"
#include "GraphicsEngine/public/IRenderScene.h"
#include "GraphicsEngine/public/ICamera.h"

#include "GraphicsEngine/public/IDirectionalLight.h"
#include "GraphicsEngine/public/ISpotLight.h"
#include "GraphicsEngine/public/IPointLight.h"

#include "GraphicsEngine/public/ITexture.h"
#include "GraphicsEngine/public/IMesh.h"
#include "GraphicsEngine/public/IMaterial.h"
#include "GraphicsEngine/public/IBone.h"
#include "GraphicsEngine/public/ISprite.h"
#include "GraphicsEngine/public/IText.h"

#include "GraphicsEngine/public/IParticleMaterial.h"
#include "GraphicsEngine/public/IParticleSystem.h"
#include "GraphicsEngine/public/IGraph.h"

//#include "Editor/imgui/imgui.h"
#include "GraphicsEngine/public/imgui.h"

//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"
//#include "GraphicsEngine/public/ICamera.h"
#include "../Utils/SimpleMath.h"
#include "Test.h"

#define DefaultLayer 0
#define PlayerLayer 1

//#define MAKE_PARTICLE

std::string wstring_to_utf8Func(const std::wstring& wstr)
{
	std::string utf8str;
	utf8str.reserve(wstr.size() * 4); // 최대 크기로 예약

	for (wchar_t wc : wstr) {
		if (wc <= 0x7F) {
			// 1바이트 (ASCII)
			utf8str.push_back(static_cast<char>(wc));
		}
		else if (wc <= 0x7FF) {
			// 2바이트
			utf8str.push_back(static_cast<char>(0xC0 | ((wc >> 6) & 0x1F)));
			utf8str.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
		}
		else if (wc <= 0xFFFF) {
			// 3바이트
			utf8str.push_back(static_cast<char>(0xE0 | ((wc >> 12) & 0x0F)));
			utf8str.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
			utf8str.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
		}
		else if (wc <= 0x10FFFF) {
			// 4바이트
			utf8str.push_back(static_cast<char>(0xF0 | ((wc >> 18) & 0x07)));
			utf8str.push_back(static_cast<char>(0x80 | ((wc >> 12) & 0x3F)));
			utf8str.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
			utf8str.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
		}
	}
	return utf8str;
}

// 유틸리티 함수: UTF-8 문자열을 wstring으로 변환
std::wstring utf8_to_wstringFunc(const std::string& utf8Str)
{
	if (utf8Str.empty()) return std::wstring();

	int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
	std::wstring wideStr(wideCharLength, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideCharLength);

	return wideStr;
}

using namespace DirectX::SimpleMath;

#define MAX_LOADSTRING 100

#define WIDTH 1280
#define HEIGHT 960
HWND g_hWnd;
std::shared_ptr<Ideal::IdealRenderer> gRenderer = nullptr;
bool show_demo_window = true;
bool show_another_window = false;
bool show_angle_window = true;
bool show_point_light_window = true;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

DWORD g_FrameCount = 0;
ULONGLONG g_PrvFrameCheckTick = 0;
ULONGLONG g_PrvUpdateTick = 0;


// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Test Function
void InitCamera(std::shared_ptr<Ideal::ICamera> Camera);
void CameraTick(std::shared_ptr<Ideal::ICamera> Camera, std::shared_ptr<Ideal::ISpotLight> SpotLight = nullptr, std::shared_ptr<Ideal::IPointLight> PointLight = nullptr);
void ImGuiTest();
void DirLightAngle(float* x, float* y, float* z);
void PointLightInspecter(std::shared_ptr<Ideal::IPointLight> light);
void SkinnedMeshObjectAnimationTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject);
void AnimationTest(std::shared_ptr<Ideal::IAnimation> Animation);
void LightTest(std::shared_ptr<Ideal::IDirectionalLight> DirLight);
void ImageTest(std::shared_ptr<Ideal::ITexture> Texture);
void SpotLightInspector(std::shared_ptr<Ideal::ISpotLight> PointLight);
void SkinnedMeshObjectBoneInfoTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject);
void SkinnedMeshObjectGetMeshTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject, std::shared_ptr<Ideal::IMaterial> Material, std::shared_ptr<Ideal::IMaterial> Material2 = nullptr, std::shared_ptr<Ideal::ITexture> Texture = nullptr, std::shared_ptr<Ideal::ITexture> Texture2 = nullptr);
void SpriteTest(std::shared_ptr<Ideal::ISprite> Sprite);
void TextTest(std::shared_ptr<Ideal::IText> Text);
void RendererSizeTest();
void ParticleSystemTransform(std::shared_ptr<Ideal::IParticleSystem> ParticleSystem);
float lightColor[3] = { 1.f, 1.f, 1.f };
float lightAngleX = 0.f;
float lightAngleY = 0.f;
float lightIntensity = 1.f;

float g_cameraSpeed = 0.04f;
bool g_CameraMove = true;
void CameraWindow(std::shared_ptr<Ideal::ICamera> Camera);

static const char* items[] =
{
	"800x600",
	"1200x900",
	"1280x720",
	"1920x1080",
	"1920x1200",
	"2560x1440",
	"3440x1440",
	"3840x2160"
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(3822);
#endif

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_IDEALGRAPHICSDEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 애플리케이션 초기화를 수행합니다:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IDEALGRAPHICSDEMO));

	MSG msg = {};
	WCHAR programpath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, programpath);
	{
#pragma region EditorInitialize
		//EGraphicsInterfaceType type = EGraphicsInterfaceType::D3D12;
		//EGraphicsInterfaceType type = EGraphicsInterfaceType::D3D12_EDITOR;
		//EGraphicsInterfaceType type = EGraphicsInterfaceType::D3D12_RAYTRACING;
		EGraphicsInterfaceType type = EGraphicsInterfaceType::D3D12_RAYTRACING_EDITOR;
		gRenderer = CreateRenderer(
			type,
			&g_hWnd,
			WIDTH,
			HEIGHT,
			L"../Resources/Assets/",
			L"../Resources/Models/",
			L"../Resources/Textures/"
		);

		gRenderer->Init();

		auto skybox = gRenderer->CreateSkyBox(L"../Resources/Textures/SkyBox/flower_road_8khdri_1kcubemap.BC7.DDS");
		gRenderer->SetSkyBox(skybox);
		//gRenderer->SetSkyBox(L"../Resources/Textures/SkyBox/custom1.dds");

		Vector3 pointLightPosition = Vector3(3.f);

		//-------------------Create Camera-------------------//
		std::shared_ptr<Ideal::ICamera> camera = gRenderer->CreateCamera();
		InitCamera(camera);
		gRenderer->SetMainCamera(camera);

		//gRenderer->SetDisplayResolutionOption(Ideal::Resolution::EDisplayResolutionOption::R_3440_1440);
		//gRenderer->SetDisplayResolutionOption(Ideal::Resolution::EDisplayResolutionOption::R_800_600);
		gRenderer->SetDisplayResolutionOption(Ideal::Resolution::EDisplayResolutionOption::R_1920_1080);
		//gRenderer->ToggleFullScreenWindow();
#pragma endregion

#pragma region Sample
		//// Create Static Mesh
		//std::shared_ptr<Ideal::IMeshObject> SampleCart = gRenderer->CreateStaticMeshObject(L"cart/SM_cart");
		//SampleCart->SetTransformMatrix(Matrix::CreateTranslation(Vector3(5, 2, 0)));
		//
		//// Create Material
		//std::shared_ptr<Ideal::IMaterial> SampleCartMaterial0 = gRenderer->CreateMaterial();
		//std::shared_ptr<Ideal::IMaterial> SampleCartMaterial1 = gRenderer->CreateMaterial();
		//
		//// Create Texture
		//std::shared_ptr<Ideal::ITexture> SampleCartBaseTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_cartdeco_BaseMap.png", true, false, false);
		//SampleCartMaterial0->SetBaseMap(SampleCartBaseTexture0);
		//std::shared_ptr<Ideal::ITexture> SampleCartNormalTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_cartdeco_Normal.png", true, true, true);
		//SampleCartMaterial0->SetNormalMap(SampleCartNormalTexture0);
		//std::shared_ptr<Ideal::ITexture> SampleCartMaskTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_cartdeco_MaskMap.png", true, true, true);
		//SampleCartMaterial0->SetMaskMap(SampleCartMaskTexture0);
		//// Bind Material
		//SampleCart->GetMeshByIndex(0).lock()->SetMaterialObject(SampleCartMaterial0);
		//
		//// Create Texture
		//std::shared_ptr<Ideal::ITexture> SampleCartBaseTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_fruitcart_BaseMap.png", true, false, false);
		//SampleCartMaterial1->SetBaseMap(SampleCartBaseTexture1);
		//std::shared_ptr<Ideal::ITexture> SampleCartNormalTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_fruitcart_Normal.png", true, true, true);
		//SampleCartMaterial1->SetNormalMap(SampleCartNormalTexture1);
		//std::shared_ptr<Ideal::ITexture> SampleCartMaskTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/cart/T_fruitcart_MaskMap.png", true, true, true);
		//SampleCartMaterial1->SetMaskMap(SampleCartMaskTexture1);
		//// Bind Material
		//SampleCart->GetMeshByIndex(1).lock()->SetMaterialObject(SampleCartMaterial1);
		//SampleCart->GetMeshByIndex(2).lock()->SetMaterialObject(SampleCartMaterial1);
#pragma endregion

#pragma region FBXConvert
		//-------------------Convert FBX(Model, Animation)-------------------//
		//gRenderer->ConvertAssetToMyFormat(L"DebugPlayer/asciiFbxAni.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"DebugPlayer/asciiFbxAni.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"MapData/1_HN_Scene2/brick_base.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"EnemyTest/idelTest.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"EnemyTest/idelTest.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"DebugPlayer/animation_ka_walk.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"DebugPlayer/animation_ka_walk.fbx");

		//gRenderer->ConvertAssetToMyFormat(L"PlayerRe/SM_chronos.Main_tPose.fbx", true);
		//gRenderer->ConvertAssetToMyFormat(L"PlayerRe/untitled.fbx", true);

		//gRenderer->ConvertAssetToMyFormat(L"0_Particle/Slash.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"cart/SM_cart.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"building/building_dummy3_hanna.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"UVSphere/UVSphere.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"DebugObject/debugCube.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"DebugObject/debugSphere.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"DebugPlane/Plane.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"player/SK_Fencer_Lady_Nude@T-Pose.fbx", true);
		//gRenderer->ConvertAssetToMyFormat(L"DebugObject/debugCube.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"Kachujin/Mesh.fbx", true);
		//gRenderer->ConvertAssetToMyFormat(L"PlayerAnimations/ChargedAttack/M_Big_Sword@Attack_3Combo_1.fbx", false);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"PlayerAnimations/Idle/idle.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"statue_chronos/SMown_chronos_statue.fbx", false);
		//gRenderer->ConvertAssetToMyFormat(L"formula1/Formula 1 mesh.fbx", false);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"player/Hip Hop Dancing.fbx");
		//gRenderer->ConvertAnimationAssetToMyFormat(L"Kachujin/HipHop.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"player/SK_Fencer_Lady_Nude.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"player/Dancing Twerk.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"player2/myPlayer2.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"player2/Capoeira.fbx");
		//ERROR : 
		//gRenderer->ConvertAnimationAssetToMyFormat(L"CatwalkWalkForward3/CatwalkWalkForward3.fbx");// -> Assimp Converter에서 FLAG 해제
		//gRenderer->ConvertAssetToMyFormat(L"CatwalkWalkForward3/CatwalkWalkForward3.fbx", true);
		//gRenderer->ConvertAssetToMyFormat(L"test2/run_.fbx", true);
		//gRenderer->ConvertAnimationAssetToMyFormat(L"test2/run_.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"Boss/bosshall.fbx", false, false);
		//ReadVertexPosition(L"../Resources/Models/Tower/Tower.pos");
		//gRenderer->ConvertAnimationAssetToMyFormat(L"Kachujin/Run.fbx");
		//gRenderer->ConvertAnimationAssetToMyFormat(L"Kachujin/Idle.fbx");
		//gRenderer->ConvertAnimationAssetToMyFormat(L"Kachujin/Slash.fbx");
		//gRenderer->ConvertAnimationAssetToMyFormat(L"PlayerRe/Sword And Shield Slash.fbx");
		//gRenderer->ConvertAssetToMyFormat(L"boss/bosshall.fbx", false);
#pragma endregion

#pragma region CreateMeshObjectAndAnimation
		//-------------------Create Mesh Object-------------------//

		//std::shared_ptr<Ideal::ISkinnedMeshObject> DebugPlayer = gRenderer->CreateSkinnedMeshObject(L"DebugPlayer/animation_ka_walk_ori");
		//std::shared_ptr<Ideal::IAnimation> DebugPlayerAnim = gRenderer->CreateAnimation(L"DebugPlayer/animation_ka_walk_ori");
		//DebugPlayer->AddAnimation("Debug", DebugPlayerAnim);
		//
		//std::shared_ptr<Ideal::ISkinnedMeshObject> DebugEnemy = gRenderer->CreateSkinnedMeshObject(L"EnemyTest/idelTest");
		//std::shared_ptr<Ideal::IAnimation> DebugEnemyAnim = gRenderer->CreateAnimation(L"EnemyTest/idelTest");
		//DebugEnemy->AddAnimation("Debug", DebugEnemyAnim);

		//std::shared_ptr<Ideal::ISkinnedMeshObject> DebugPlayer0 = gRenderer->CreateSkinnedMeshObject(L"PlayerAnimations/ChargedAttack/M_Big_Sword@Attack_3Combo_1");
		//std::shared_ptr<Ideal::IAnimation> DebugPlayerAnim0 = gRenderer->CreateAnimation(L"PlayerAnimations/Idle/idle");
		//DebugPlayer0->AddAnimation("Debug", DebugPlayerAnim0);

		std::vector < std::shared_ptr<Ideal::IMeshObject>> staticMeshes;
		for (int i = 0; i < 30; i++)
		{
			std::shared_ptr<Ideal::IMeshObject> testObj = gRenderer->CreateStaticMeshObject(L"PlayerAnimations/ChargedAttack/M_Big_Sword@Attack_3Combo_1");
			testObj->SetTransformMatrix(Matrix::CreateTranslation(Vector3(i, 0, 0)));
			staticMeshes.push_back(testObj);
		}

		//std::shared_ptr<Ideal::IMeshObject> DebugStaticEnemy = gRenderer->CreateStaticMeshObject(L"PlayerAnimations/ChargedAttack/M_Big_Sword@Attack_3Combo_1");
		//DebugStaticEnemy->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(3, 0, 0));

		//std::shared_ptr<Ideal::ISkinnedMeshObject> DebugEnemy = gRenderer->CreateSkinnedMeshObject(L"BossAnimations/Idle/Idle");
		//std::shared_ptr<Ideal::IAnimation> DebugPlayerAnim = gRenderer->CreateAnimation(L"BossAnimations/Idle/BossEntranceRoot");
		//DebugEnemy->AddAnimation("Debug", DebugPlayerAnim);
		// 

		Matrix floorMat = Matrix::CreateRotationY(3.14);


#pragma endregion
#pragma region TestPlane
		auto planeMaterial = gRenderer->CreateMaterial();
		auto planeAlbedoTexture = gRenderer->CreateTexture(L"../Resources/Textures/archtile/T_archtile_BaseMap.png", true);
		auto planeMaskTexture = gRenderer->CreateTexture(L"../Resources/Textures/archtile/T_archtile_MaskMap.png", true);
		//auto planeMaskTexture = gRenderer->CreateTexture(L"../Resources/DefaultData/DefaultBlack.png");
		auto planeNormalTexture = gRenderer->CreateTexture(L"../Resources/Textures/archtile/T_archtile_Normal.png", true, true);
		//auto planeNormalTexture = gRenderer->CreateTexture(L"../Resources/DefaultData/DefaultNormalMap.png", true, true);
		planeMaterial->SetBaseMap(planeAlbedoTexture);
		planeMaterial->SetMaskMap(planeMaskTexture);
		planeMaterial->SetNormalMap(planeNormalTexture);
		planeMaterial->SetSurfaceTypeTransparent(false);
		planeMaterial->ChangeLayerBitMask(0x0011);

		//std::shared_ptr<Ideal::IMeshObject> plane = gRenderer->CreateStaticMeshObject(L"DebugPlane/Plane");
		//plane->GetMeshByIndex(0).lock()->SetMaterialObject(planeMaterial);
		//plane->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(2, 8, 2)));
		//meshes.push_back(plane);

		//auto garlandMaterial = gRenderer->CreateMaterial();
		//auto garlandBaseTex = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_Garland_BaseMap.png");
		//auto garlandNormalTex = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_Garland_Normal.png");
		//auto garlandMaskTex = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_Garland_MaskMap.png");
		//garlandMaterial->SetBaseMap(garlandBaseTex);
		//garlandMaterial->SetNormalMap(garlandNormalTex);
		//garlandMaterial->SetMaskMap(garlandMaskTex);
		//garlandMaterial->SetAlphaClipping(true);
		//garlandMaterial->SetSurfaceTypeTransparent(true);
		//DebugPlayer->GetMeshByIndex(4).lock()->SetMaterialObject(garlandMaterial);
		//DebugPlayer->AlphaClippingCheck();
		//std::shared_ptr<Ideal::IMeshObject> garland = gRenderer->CreateStaticMeshObject(L"DebugPlane/Plane");
		//garland->GetMeshByIndex(0).lock()->SetMaterialObject(garlandMaterial);
		//garland->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(0, 5, 0)));

		//auto windowMaterial = gRenderer->CreateMaterial();
		//auto windowBase = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_town_glass_BaseMap.png");
		//auto windowNormal = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_town_glass_Normal.png");
		//auto windowMask = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/T_town_glass_MaskMap.png");
		//windowMaterial->SetBaseMap(windowBase);
		//windowMaterial->SetNormalMap(windowNormal);
		//windowMaterial->SetMaskMap(windowMask);
		//windowMaterial->SetSurfaceTypeTransparent(true);
		//windowMaterial->ChangeLayer(PlayerLayer);
		std::vector<std::shared_ptr<Ideal::IMeshObject>> meshes;
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				std::shared_ptr<Ideal::IMeshObject> plane = gRenderer->CreateStaticMeshObject(L"DebugPlane/Plane");
				plane->GetMeshByIndex(0).lock()->SetMaterialObject(planeMaterial);
				//plane->GetMeshByIndex(0).lock()->SetMaterialObject(windowMaterial);
				plane->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(y * 2, 0, x * 2)));
				meshes.push_back(plane);
				//plane->AlphaClippingCheck();
			}
		}

		//std::shared_ptr<Ideal::IMeshObject> plane = gRenderer->CreateStaticMeshObject(L"DebugPlane/Plane");
		//plane->GetMeshByIndex(0).lock()->SetMaterialObject(garlandMaterial);
		//plane->AlphaClippingCheck();
		//plane->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(2, 0, 2)));

		//plane->GetMeshByIndex(0).lock()->SetMaterialObject(garlandMaterial);
		//plane->AlphaClippingCheck();

#pragma endregion
#pragma region CreateDebugMesh
		//std::shared_ptr<Ideal::IMeshObject> debugCart = gRenderer->CreateDebugMeshObject(L"cart/SM_cart");
		//debugCart->SetTransformMatrix(Matrix::CreateTranslation(Vector3(0, 10, 0)));
		//cart->SetTransformMatrix(Matrix::CreateTranslation(Vector3(0, 2, 0)));
		//cart->GetMeshByIndex(0).lock()->SetMaterialObject(windowMaterial);

		//cart->SetStaticWhenRunTime(true);
		//cart2->SetTransformMatrix(Matrix::CreateTranslation(Vector3(0, 0, 21)));
#pragma endregion

#pragma region CreateUI
		//std::shared_ptr<Ideal::ISprite> sprite = gRenderer->CreateSprite();
		////sprite->SetTexture(eyeTexture);
		////sprite->SetTextureSamplePosition(Vector2(0, 0));
		//sprite->SetScale(Vector2(0.1, 0.1));
		//sprite->SetPosition(Vector2(0, 0));
		//sprite->SetAlpha(0.8f);
		//sprite->SetZ(0.2);
		//// 아래의 값은 기본으로 적용되어 있음. (Set Texture 할 때 Texture의 사이즈로 아래의 작업을 함)
		////sprite->SetSampleRect({ 0,0,faceTexture->GetWidth(), faceTexture->GetHeight() });
		//
		////sprite->SetTextureSize(Vector2(512, 512));
		////sprite->SetTextureSamplePosition(Vector2(0, 0));
		////sprite->SetTextureSampleSize(Vector2(2048, 2048));
		//
		//std::shared_ptr<Ideal::ISprite> sprite2 = gRenderer->CreateSprite();
		////sprite2->SetTexture(eyeTexture);
		////sprite2->SetSampleRect({ 0, 0, 4096*2, 4096*2 });
		//sprite2->SetSampleRect({ 1024, 0, 2048, 2048 });
		//sprite2->SetSampleRect({ 1024, 0, 4096, 2048 });
		////sprite2->SetSampleRect({ 0, 0, 4096, 2048 });
		////sprite2->SetSampleRect({ 1024, 0, 4096, 2048});
		////sprite2->SetSampleRect({ 0, 0, 1024, 2048 });
		//sprite2->SetScale(Vector2(0.1, 0.1));
		//sprite2->SetPosition(Vector2(400, 50));
		//sprite2->SetZ(0.1);
		//
		//std::shared_ptr<Ideal::ISprite> sprite3 = gRenderer->CreateSprite();
		//sprite3->SetScale(Vector2(0.3, 0.3));
		//sprite3->SetPosition(Vector2(1180, 860));
		//sprite3->SetZ(0);
		//
		////---Text---//
		////std::shared_ptr<Ideal::IText> text = gRenderer->CreateText(200, 100, 18);
		//std::shared_ptr<Ideal::IText> text = gRenderer->CreateText(100, 90, 30);	// 기본 tahoma 글꼴임
		////std::shared_ptr<Ideal::IText> text = gRenderer->CreateText(55, 65, 30, L"times new roman");
		////text->ChangeText(L"UI\n Test");
		//text->ChangeText(L"Test");
		//text->SetPosition(Vector2(500, 500));
		//text->SetZ(0.2);
		//
		//std::vector<std::shared_ptr<Ideal::IText>>texts;


		//std::shared_ptr<Ideal::ITexture> uiTex0 = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/ingame_CP_base.png");
		//std::shared_ptr<Ideal::ITexture> uiTex1 = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/ingame_CP_gph.png", false, false, true);
		//std::shared_ptr<Ideal::ITexture> uiTex2 = gRenderer->CreateTexture(L"../Resources/Textures/Test_10_15/ingame_CP_deco.png");
		//
		//std::shared_ptr<Ideal::ISprite> sp0 = gRenderer->CreateSprite();
		//sp0->SetTexture(uiTex0);
		//sp0->SetPosition(Vector2(500, 400));
		//sp0->SetZ(0.7);
		//sp0->SetScale(Vector2(2, 2));
		//std::shared_ptr<Ideal::ISprite> sp1 = gRenderer->CreateSprite();
		//sp1->SetTexture(uiTex2);
		//sp1->SetPosition(Vector2(500, 400));
		//sp1->SetZ(0.6);
		//sp1->SetScale(Vector2(2, 2));
		//std::shared_ptr<Ideal::ISprite> sp2 = gRenderer->CreateSprite();
		//sp2->SetTexture(uiTex1);
		//sp2->SetPosition(Vector2(500, 400));
		//sp2->SetZ(0.5);
		//sp2->SetScale(Vector2(2, 2));

#pragma endregion

#pragma region CreateLight
		//--------------------Create Light----------------------//
		std::shared_ptr<Ideal::IDirectionalLight> dirLight = gRenderer->CreateDirectionalLight();
		dirLight->SetDirection(Vector3(0.f, 1.f, 0.f));
		dirLight->SetIntensity(1.f);

		std::shared_ptr<Ideal::ISpotLight> spotLight = gRenderer->CreateSpotLight();
		spotLight->SetPosition(Vector3(0.f, 3.f, 3.f));
		spotLight->SetRange(6.f);
		spotLight->SetLightColor(Color(1.f, 0.f, 1.f, 1.f));
		spotLight->SetIntensity(0.f);


		std::shared_ptr<Ideal::IPointLight> pointLight = gRenderer->CreatePointLight();
		pointLight->SetPosition(Vector3(0.f, 3.f, 3.f));
		pointLight->SetRange(6.f);
		pointLight->SetLightColor(Color(1.f, 0.f, 1.f, 1.f));
		pointLight->SetIntensity(0.f);
		//pointLight->ChangeLayer(0);
		pointLight->ChangeLayerBitMask(0x0010);

		std::shared_ptr<Ideal::IPointLight> pointLight2 = gRenderer->CreatePointLight();
		pointLight2->SetPosition(Vector3(0.f, 3.f, 3.f));
		pointLight2->SetRange(6.f);
		pointLight2->SetLightColor(Color(0.f, 1.f, 0.f, 1.f));
		pointLight2->SetIntensity(0.f);


		gRenderer->DeleteLight(pointLight2);

		std::vector<std::shared_ptr<Ideal::IPointLight>> pointLights;
#pragma endregion

#pragma region CompileShader
		//------------------------Compile Shader---------------------------//
		// 아래는 셰이더를 컴파일하여 저장한다.
		gRenderer->CompileShader(
			L"../Shaders/Particle/TestCustomParticle.hlsl",
			L"../Shaders/Particle/",
			L"TestCustomParticle",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		// 아래는 컴파일 된 셰이더를 가져온다.
		std::shared_ptr<Ideal::IShader> particleCustomShader = gRenderer->CreateAndLoadParticleShader(L"TestCustomParticle");

		gRenderer->CompileShader(
			L"../Shaders/Particle/SwordSlash.hlsl",
			L"../Shaders/Particle/",
			L"SwordSlash",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> swordParticleShader = gRenderer->CreateAndLoadParticleShader(L"SwordSlash");

		// boss
		gRenderer->CompileShader(
			L"../Shaders/Particle/BossFX_1Beam.hlsl",
			L"../Shaders/Particle/",
			L"BossFX_1Beam",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossParticleShader0 = gRenderer->CreateAndLoadParticleShader(L"BossFX_1Beam");

		gRenderer->CompileShader(
			L"../Shaders/Particle/DefaultParticlePS.hlsl",
			L"../Shaders/Particle/",
			L"DefaultParticlePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> DefaultParticlePSShader = gRenderer->CreateAndLoadParticleShader(L"DefaultParticlePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossFX_1BeamRing.hlsl",
			L"../Shaders/Particle/",
			L"BossFX_1BeamRingPS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossBeamRingShader = gRenderer->CreateAndLoadParticleShader(L"BossFX_1BeamRingPS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossFX_FileProjectile.hlsl",
			L"../Shaders/Particle/",
			L"BossFX_FileProjectilePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossFireProjectileShader = gRenderer->CreateAndLoadParticleShader(L"BossFX_FileProjectilePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossFX_FileProjectile1.hlsl",
			L"../Shaders/Particle/",
			L"BossFX_FileProjectilePS_1",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossFireProjectileShader1 = gRenderer->CreateAndLoadParticleShader(L"BossFX_FileProjectilePS_1");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossSphereImpact.hlsl",
			L"../Shaders/Particle/",
			L"BossSphereImpactPS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossSphereImpactShader = gRenderer->CreateAndLoadParticleShader(L"BossSphereImpactPS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossBlackHole.hlsl",
			L"../Shaders/Particle/",
			L"BossBlackHolePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossBlackHoleShader = gRenderer->CreateAndLoadParticleShader(L"BossBlackHolePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/BossBlackHoleSphere.hlsl",
			L"../Shaders/Particle/",
			L"BossBlackHoleSpherePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossBlackHoleSphereShader = gRenderer->CreateAndLoadParticleShader(L"BossBlackHoleSpherePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/DefaultTextureParticlePS.hlsl",
			L"../Shaders/Particle/",
			L"DefaultTextureParticlePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> defaultTextureParticleShader = gRenderer->CreateAndLoadParticleShader(L"DefaultTextureParticlePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/DefaultTextureParticleClipPS.hlsl",
			L"../Shaders/Particle/",
			L"DefaultTextureParticleClipPS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> defaultTextureParticleClipShader = gRenderer->CreateAndLoadParticleShader(L"DefaultTextureParticleClipPS");


		gRenderer->CompileShader(
			L"../Shaders/Particle/BossFireFloor.hlsl",
			L"../Shaders/Particle/",
			L"BossFireFloorPS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> bossFireFloorShader = gRenderer->CreateAndLoadParticleShader(L"BossFireFloorPS");


		// Test
		gRenderer->CompileShader(
			L"../Shaders/Particle/DefaultParticleBillboardShader.hlsl",
			L"../Shaders/Particle/",
			L"DefaultParticleBillboardShaderPS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> billboardTestPS = gRenderer->CreateAndLoadParticleShader(L"DefaultParticleBillboardShaderPS");

		// Default Particle Clip
		gRenderer->CompileShader(
			L"../Shaders/Particle/DefaultParticlePS_Clip.hlsl",
			L"../Shaders/Particle/",
			L"DefaultParticlePS_Clip",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> defaultParticlePS_Clip = gRenderer->CreateAndLoadParticleShader(L"DefaultParticlePS_Clip");

		gRenderer->CompileShader(
			L"../Shaders/Particle/GroundSpike.hlsl",
			L"../Shaders/Particle/",
			L"GroundSpikePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> groundSpikePS = gRenderer->CreateAndLoadParticleShader(L"GroundSpikePS");

		gRenderer->CompileShader(
			L"../Shaders/Particle/EnemyCharge.hlsl",
			L"../Shaders/Particle/",
			L"EnemyChargePS",
			L"ps_6_3",
			L"PSMain",
			L"../Shaders/Particle/"
		);
		std::shared_ptr<Ideal::IShader> enemyChargePS = gRenderer->CreateAndLoadParticleShader(L"EnemyChargePS");

#pragma endregion

		std::vector<std::shared_ptr<Ideal::ITexture>> particleTexturesToDelete;

#ifdef MAKE_PARTICLE
#pragma region Beam1
		//------------------------Create Particle---------------------------//

		std::shared_ptr<Ideal::IParticleMaterial> bossParticleMaterial0 = gRenderer->CreateParticleMaterial();
		bossParticleMaterial0->SetShader(bossParticleShader0);

		std::shared_ptr<Ideal::ITexture> bossParticleTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/boss/PerlinMap_4.png");
		std::shared_ptr<Ideal::ITexture> bossParticleTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/boss/GausianNoise_3.png");
		bossParticleMaterial0->SetTexture0(bossParticleTexture0);
		bossParticleMaterial0->SetTexture1(bossParticleTexture1);
		particleTexturesToDelete.push_back(bossParticleTexture0);
		particleTexturesToDelete.push_back(bossParticleTexture1);

		bossParticleMaterial0->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);

		std::shared_ptr<Ideal::IParticleSystem> bossParticleSystem0 = gRenderer->CreateParticleSystem(bossParticleMaterial0);

		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Cylinder_01.fbx");
		std::shared_ptr<Ideal::IMesh> bossParticleMesh = gRenderer->CreateParticleMesh(L"0_Particle/Cylinder_01");

		bossParticleSystem0->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bossParticleSystem0->SetRenderMesh(bossParticleMesh);

		bossParticleSystem0->SetLoop(false);
		// 첫 사이즈
		bossParticleSystem0->SetStartSize(0.35f);
		bossParticleSystem0->SetSimulationSpeed(2.f);
		// 첫 색상
		bossParticleSystem0->SetStartColor(DirectX::SimpleMath::Color(0.1080087, 0.5713133, 2.544206, 1));

		bossParticleSystem0->SetTransformMatrix(
			DirectX::SimpleMath::Matrix::CreateScale(Vector3(0.4, 0.4, 1)) *
			DirectX::SimpleMath::Matrix::CreateRotationX(3.14f * 0.5f)
		);

		///////////// 같이 쓰는거
		std::shared_ptr<Ideal::IParticleMaterial> bossParticleMaterial1 = gRenderer->CreateParticleMaterial();
		bossParticleMaterial1->SetShader(DefaultParticlePSShader);

		bossParticleMaterial1->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);

		std::shared_ptr<Ideal::IParticleSystem> bossParticleSystem1 = gRenderer->CreateParticleSystem(bossParticleMaterial1);

		//gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Cylinder_01.fbx", true, Vector3(0.35f, 0.35f, 0.35f));
		//std::shared_ptr<Ideal::IMesh> bossParticleMesh = gRenderer->CreateParticleMesh(L"0_Particle/Cylinder_01");

		bossParticleSystem1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bossParticleSystem1->SetRenderMesh(bossParticleMesh);

		bossParticleSystem1->SetStartSize(0.25f);
		bossParticleSystem1->SetLoop(false);
		// 첫 색상
		bossParticleSystem1->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		bossParticleSystem1->SetTransformMatrix(
			DirectX::SimpleMath::Matrix::CreateScale(Vector3(0.4, 0.4, 1)) *
			DirectX::SimpleMath::Matrix::CreateRotationX(3.14f * 0.5f));
		bossParticleSystem1->SetSimulationSpeed(2.f);
		// 크기 
		bossParticleSystem1->SetSizeOverLifetime(true);
		{
			auto& graph = bossParticleSystem1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,1 });
			graph.AddControlPoint({ 0.5,1.5f });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = bossParticleSystem1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1 });
			graph.AddControlPoint({ 0.5,1.5f });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = bossParticleSystem1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,1 });
		}
		// 또 같이 쓰는거

		std::shared_ptr<Ideal::IParticleMaterial> bossBeamRingMaterial = gRenderer->CreateParticleMaterial();
		bossBeamRingMaterial->SetShader(bossBeamRingShader);
		std::shared_ptr<Ideal::ITexture> bossBeamTex0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/boss/Ring_2.png");
		bossBeamRingMaterial->SetTexture0(bossBeamTex0);

		particleTexturesToDelete.push_back(bossBeamTex0);

		bossBeamRingMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		std::shared_ptr<Ideal::IParticleSystem> bossBeamRingParticleSystem = gRenderer->CreateParticleSystem(bossBeamRingMaterial);
		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Plane.fbx");
		std::shared_ptr<Ideal::IMesh> particleMeshPlane = gRenderer->CreateParticleMesh(L"0_Particle/Plane");
		bossBeamRingParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bossBeamRingParticleSystem->SetRenderMesh(particleMeshPlane);
		bossBeamRingParticleSystem->SetStartSize(0.5f);
		bossBeamRingParticleSystem->SetStartLifetime(1.f);
		bossBeamRingParticleSystem->SetDuration(1.f);
		bossBeamRingParticleSystem->SetLoop(false);
		bossBeamRingParticleSystem->SetStartColor(Color(0.515574, 2.434655, 5.470814, 1));

		bossBeamRingParticleSystem->SetSizeOverLifetime(true);
		{
			auto& graph = bossBeamRingParticleSystem->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.5 });
			graph.AddControlPoint({ 1,3 });
		}
		{
			auto& graph = bossBeamRingParticleSystem->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1 });
		}
		{
			auto& graph = bossBeamRingParticleSystem->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.5 });
			graph.AddControlPoint({ 1,3 });
		}

#pragma endregion
#pragma region FireProjectile
		///----------------------FileProjectile---------------------///
		std::shared_ptr<Ideal::IParticleMaterial> bossFireProjectileMaterial0 = gRenderer->CreateParticleMaterial();
		bossFireProjectileMaterial0->SetShader(bossFireProjectileShader);

		std::shared_ptr<Ideal::ITexture> bossFileProjectileTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossProjectile/mask_4.png");
		std::shared_ptr<Ideal::ITexture> bossFileProjectileTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossProjectile/Fire_Single_2.png");

		particleTexturesToDelete.push_back(bossFileProjectileTexture0);
		particleTexturesToDelete.push_back(bossFileProjectileTexture1);

		bossFireProjectileMaterial0->SetTexture0(bossFileProjectileTexture0);
		bossFireProjectileMaterial0->SetTexture1(bossFileProjectileTexture1);

		bossFireProjectileMaterial0->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);
		bossFireProjectileMaterial0->SetBackFaceCulling(false);
		std::shared_ptr<Ideal::IParticleSystem> bossFireProjectileParticleSystem0 = gRenderer->CreateParticleSystem(bossFireProjectileMaterial0);

		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/UVSphere.fbx");
		std::shared_ptr<Ideal::IMesh> bossParticleMeshSphere = gRenderer->CreateParticleMesh(L"0_Particle/UVSphere");

		bossFireProjectileParticleSystem0->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bossFireProjectileParticleSystem0->SetRenderMesh(bossParticleMeshSphere);

		bossFireProjectileParticleSystem0->SetStartSize(35.f);
		bossFireProjectileParticleSystem0->SetStartColor(DirectX::SimpleMath::Color(3.5, 0.03, 0, 1));
		bossFireProjectileParticleSystem0->SetLoop(false);
		bossFireProjectileParticleSystem0->SetDuration(3.f);
		bossFireProjectileParticleSystem0->SetStartLifetime(3.f);

		bossFireProjectileParticleSystem0->SetTransformMatrix(
			DirectX::SimpleMath::Matrix::CreateScale(Vector3(2, 2, 4))
			// * DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(0, 5, 0))
		);


		std::shared_ptr<Ideal::IParticleMaterial> bossFireProjectileMaterial1 = gRenderer->CreateParticleMaterial();
		bossFireProjectileMaterial1->SetShader(bossFireProjectileShader);

		//std::shared_ptr<Ideal::ITexture> bossFileProjectileTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossProjectile/mask_4.png");
		//std::shared_ptr<Ideal::ITexture> bossFileProjectileTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossProjectile/Fire_Single_2.png");
		std::shared_ptr<Ideal::ITexture> bossFileProjectileTexture2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossProjectile/Normal_16.png");

		bossFireProjectileMaterial1->SetTexture0(bossFileProjectileTexture0);
		bossFireProjectileMaterial1->SetTexture1(bossFileProjectileTexture1);

		//bossFireProjectileMaterial1->SetTexture2(bossFileProjectileTexture2);
		bossFireProjectileMaterial1->SetBackFaceCulling(false);
		bossFireProjectileMaterial1->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);
		std::shared_ptr<Ideal::IParticleSystem> bossFireProjectileParticleSystem1 = gRenderer->CreateParticleSystem(bossFireProjectileMaterial1);

		//gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/UVSphere.fbx");
		//std::shared_ptr<Ideal::IMesh> bossParticleMeshSphere1 = gRenderer->CreateParticleMesh(L"0_Particle/UVSphere");

		bossFireProjectileParticleSystem1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bossFireProjectileParticleSystem1->SetRenderMesh(bossParticleMeshSphere);

		bossFireProjectileParticleSystem1->SetStartSize(35.f);
		//bossFireProjectileParticleSystem1->SetStartColor(DirectX::SimpleMath::Color(0.749, 0.106, 0.020));
		bossFireProjectileParticleSystem1->SetStartColor(DirectX::SimpleMath::Color(1, 1, 0.7, 1));


		bossFireProjectileParticleSystem1->SetLoop(false);
		bossFireProjectileParticleSystem1->SetDuration(3.f);
		bossFireProjectileParticleSystem0->SetStartLifetime(3.f);

		bossFireProjectileParticleSystem1->SetTransformMatrix(
			DirectX::SimpleMath::Matrix::CreateScale(Vector3(1, 1, 5))
			//* DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(0, 5, -1))
			* DirectX::SimpleMath::Matrix::CreateTranslation(Vector3(0, 0, -1))
		);

#pragma endregion
#pragma region SphereImpact
		//bossFireProjectileParticleSystem->SetColorOverLifetime(true);
		//{
		//	auto& graph = bossFireProjectileParticleSystem->GetColorOverLifetimeGradientGraph();
		//	graph.AddPoint(Color(1, 1, 1, 0), 0.f);
		//	graph.AddPoint(Color(1, 1, 1, 0.31), 0.015f);
		//	graph.AddPoint(Color(1, 1, 1, 1), 0.11f);
		//	graph.AddPoint(Color(1, 1, 1, 0), 1.f);
		//}

		//------------------------Sphere Impact---------------------------//
		std::shared_ptr<Ideal::IParticleMaterial> sphereImpactMaterial = gRenderer->CreateParticleMaterial();
		sphereImpactMaterial->SetShader(bossSphereImpactShader);

		std::shared_ptr<Ideal::ITexture> sITex0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossSpearImpact/PerlinNoise_9.png");
		std::shared_ptr<Ideal::ITexture> sITex1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossSpearImpact/Normal_13.png");
		std::shared_ptr<Ideal::ITexture> sITex2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossSpearImpact/mask_4.png");
		particleTexturesToDelete.push_back(sITex0);
		particleTexturesToDelete.push_back(sITex1);
		particleTexturesToDelete.push_back(sITex2);
		sphereImpactMaterial->SetTexture0(sITex0);
		sphereImpactMaterial->SetTexture1(sITex1);
		sphereImpactMaterial->SetTexture2(sITex2);
		sphereImpactMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		sphereImpactMaterial->SetBackFaceCulling(false);

		std::shared_ptr<Ideal::IParticleSystem> sphereImpactParticleSystem = gRenderer->CreateParticleSystem(sphereImpactMaterial);
		sphereImpactParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		sphereImpactParticleSystem->SetRenderMesh(bossParticleMeshSphere);
		sphereImpactParticleSystem->SetTransformMatrix(
			Matrix::CreateScale(35.f)
			* Matrix::CreateScale(Vector3(6, 6, 2))
			* Matrix::CreateRotationX(3.1415 * 0.5)
			* Matrix::CreateTranslation(-3, 0, 0)
		);
		sphereImpactParticleSystem->SetStartColor(Color(0, 0.6506, 5.8796, 1));
		sphereImpactParticleSystem->SetLoop(false);
		sphereImpactParticleSystem->SetDuration(3.f);
		sphereImpactParticleSystem->SetStartLifetime(3.f);

		{
			auto& graph = sphereImpactParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0, 0.1 });
		}

		sphereImpactParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = sphereImpactParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 0);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.059);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.309);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 1);
		}

		{
			auto& graph = sphereImpactParticleSystem->GetCustomData1Y();
			graph.AddControlPoint({ 0,5 });
		}

		{
			auto& graph = sphereImpactParticleSystem->GetCustomData1Z();
			graph.AddControlPoint({ 0,7.5 });
		}
		// Sphere Impact1
		std::shared_ptr<Ideal::IParticleSystem> sphereImpactParticleSystem1 = gRenderer->CreateParticleSystem(sphereImpactMaterial);
		sphereImpactParticleSystem1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		sphereImpactParticleSystem1->SetRenderMesh(bossParticleMeshSphere);
		sphereImpactParticleSystem1->SetTransformMatrix(
			Matrix::CreateScale(35.f)
			* Matrix::CreateScale(Vector3(7, 7, 2))
			* Matrix::CreateRotationX(3.1415 * 0.5)
			* Matrix::CreateTranslation(-3, 0, 0)
		);
		sphereImpactParticleSystem1->SetStartColor(Color(0, 0.1608106, 5.930247, 1));
		sphereImpactParticleSystem1->SetLoop(false);
		sphereImpactParticleSystem1->SetDuration(3.f);
		sphereImpactParticleSystem1->SetStartLifetime(3.f);

		{
			auto& graph = sphereImpactParticleSystem1->GetCustomData1X();
			graph.AddControlPoint({ 0, 0.1 });
		}

		sphereImpactParticleSystem1->SetColorOverLifetime(true);
		{
			auto& graph = sphereImpactParticleSystem1->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 0);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.059);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.309);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 1);
		}

		{
			auto& graph = sphereImpactParticleSystem1->GetCustomData1Y();
			graph.AddControlPoint({ 0,0 });
		}

		{
			auto& graph = sphereImpactParticleSystem1->GetCustomData1Z();
			graph.AddControlPoint({ 0,1 });
		}

		// Sphere Impact2
		std::shared_ptr<Ideal::IParticleSystem> sphereImpactParticleSystem2 = gRenderer->CreateParticleSystem(sphereImpactMaterial);
		sphereImpactParticleSystem2->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		sphereImpactParticleSystem2->SetRenderMesh(bossParticleMeshSphere);
		sphereImpactParticleSystem2->SetTransformMatrix(
			Matrix::CreateScale(35.f)
			* Matrix::CreateScale(Vector3(1.5, 1.5, 3))
			* Matrix::CreateRotationX(3.1415 * 0.5)
			* Matrix::CreateTranslation(-3, 0.5, 0)
		);
		sphereImpactParticleSystem2->SetStartColor(Color(0, 0.1608106, 5.930247, 1));
		sphereImpactParticleSystem2->SetLoop(false);
		sphereImpactParticleSystem2->SetDuration(3.f);
		sphereImpactParticleSystem2->SetStartLifetime(3.f);

		{
			auto& graph = sphereImpactParticleSystem2->GetCustomData1X();
			graph.AddControlPoint({ 0, 0.1 });
		}

		sphereImpactParticleSystem2->SetColorOverLifetime(true);
		{
			auto& graph = sphereImpactParticleSystem2->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 0);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.059);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 1), 0.309);
			graph.AddPoint(Color(0, 0.6506, 5.8796, 0), 1);
		}

		{
			auto& graph = sphereImpactParticleSystem2->GetCustomData1Y();
			graph.AddControlPoint({ 0,5 });
		}

		{
			auto& graph = sphereImpactParticleSystem2->GetCustomData1Z();
			//graph.AddControlPoint({ 0,3.5 });
			graph.AddControlPoint({ 0,7.5 });
		}
#pragma endregion
#pragma region BlackHole
		//------------------------Boss Blackhole---------------------------//
		std::shared_ptr<Ideal::IParticleMaterial> blackholeMaterial = gRenderer->CreateParticleMaterial();
		blackholeMaterial->SetShader(bossBlackHoleShader);
		//blackholeMaterial->SetWriteDepthBuffer(true);


		std::shared_ptr<Ideal::ITexture> bhTex0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/BossBlackHole/PerlinMap_1.png");
		std::shared_ptr<Ideal::ITexture> bhTex1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/BossBlackHole/Normal_4.png");
		std::shared_ptr<Ideal::ITexture> bhTex2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/BossBlackHole/Normal_5.png");
		particleTexturesToDelete.push_back(bhTex0);
		particleTexturesToDelete.push_back(bhTex1);
		particleTexturesToDelete.push_back(bhTex2);
		blackholeMaterial->SetTexture0(bhTex0);
		blackholeMaterial->SetTexture1(bhTex1);
		blackholeMaterial->SetTexture2(bhTex2);
		//blackholeMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Blend);
		blackholeMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		blackholeMaterial->SetBackFaceCulling(false);


		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Circle_2.fbx");
		std::shared_ptr<Ideal::IMesh> blackHoleMaterialMesh = gRenderer->CreateParticleMesh(L"0_Particle/Circle_2");


		std::shared_ptr<Ideal::IParticleSystem> blackHoleParticleSystem = gRenderer->CreateParticleSystem(blackholeMaterial);
		blackHoleParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		blackHoleParticleSystem->SetRenderMesh(blackHoleMaterialMesh);
		blackHoleParticleSystem->SetStartSize(0.1);
		blackHoleParticleSystem->SetStartColor(Color(1, 1, 1, 1));
		blackHoleParticleSystem->SetLoop(false);
		blackHoleParticleSystem->SetDuration(8.f);
		blackHoleParticleSystem->SetStartLifetime(8.f);
		blackHoleParticleSystem->SetTransformMatrix(
			Matrix::CreateRotationX(3.14 * 0.5)
			* Matrix::CreateTranslation(Vector3(-4, 0, 0))
		);
		blackHoleParticleSystem->SetColorOverLifetime(true);
		//{
		//	auto& graph = blackHoleParticleSystem->GetColorOverLifetimeGradientGraph();
		//	graph.AddPoint(Color(0, 0, 0, 0), 0);
		//	graph.AddPoint(Color(0, 0.4, 1, 1), 0.082);
		//	graph.AddPoint(Color(0, 0.4, 1, 1), 0.92);
		//	graph.AddPoint(Color(0, 0.4, 1, 0), 1);
		//}
		{
			auto& graph = blackHoleParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0, 0, 0, 0), 0);
			graph.AddPoint(Color(0, 0.4, 1, 1), 0.082 / 8);
			graph.AddPoint(Color(0, 0.4, 1, 1), 0.92 / 8);
			graph.AddPoint(Color(0.8, 0.6, 0, 1), (float)3 / 8);	// float으로...
			graph.AddPoint(Color(0.8, 0.6, 0, 1), (float)5 / 8);
			graph.AddPoint(Color(1, 0.1, 0, 1), (float)6 / 8);
			graph.AddPoint(Color(1, 0.1, 0, 1), (float)8 / 8);
		}

		// blackhole sphere
		std::shared_ptr<Ideal::IParticleMaterial> blackholeSphereMaterial = gRenderer->CreateParticleMaterial();
		blackholeSphereMaterial->SetShader(bossBlackHoleSphereShader);
		blackholeSphereMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);
		//blackholeSphereMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		blackholeSphereMaterial->SetBackFaceCulling(true);
		//blackholeSphereMaterial->SetWriteDepthBuffer(false);
		//blackholeSphereMaterial->SetTransparency(false);
		blackholeSphereMaterial->SetWriteDepthBuffer(true);
		std::shared_ptr<Ideal::IParticleSystem> blackHoleSphereParticleSystem = gRenderer->CreateParticleSystem(blackholeSphereMaterial);
		blackHoleSphereParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		blackHoleSphereParticleSystem->SetRenderMesh(bossParticleMeshSphere);
		blackHoleSphereParticleSystem->SetStartSize(15.f);
		blackHoleSphereParticleSystem->SetDuration(8.f);
		blackHoleSphereParticleSystem->SetStartLifetime(8.f);
		blackHoleSphereParticleSystem->SetStartColor(Color(0, 0, 0, 1));
		blackHoleSphereParticleSystem->SetStartColor(Color(1, 0, 0, 1));
		blackHoleSphereParticleSystem->SetLoop(false);
		blackHoleSphereParticleSystem->SetTransformMatrix(Matrix::CreateTranslation(Vector3(-4, 0, 0)));
#pragma endregion

#pragma region CreateParticle
		//------------------------Create Particle---------------------------//
		std::shared_ptr<Ideal::IParticleMaterial> particleMaterial = gRenderer->CreateParticleMaterial();
		particleMaterial->SetShader(particleCustomShader);

		std::shared_ptr<Ideal::ITexture> particleTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Slash_1_Slashfx.png");
		particleTexturesToDelete.push_back(particleTexture);
		particleMaterial->SetTexture0(particleTexture);

		// --- 반드시 뺴먹지 말 것 --- Blending Mode---//
		particleMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);
		//particleMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);

		std::shared_ptr<Ideal::IParticleSystem> particleSystem = gRenderer->CreateParticleSystem(particleMaterial);

		// 컨버팅
		// 아래의 파티클 매쉬는 크기가 너무 작아 처음에 크기를 100배 키우고 있음. true-> 크기를 적용한다. Vector3(100.f) -> 크기
		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Slash.fbx", true, Vector3(100.f));

		// 미리 컨버팅한 매쉬를 불러온다.
		std::shared_ptr<Ideal::IMesh> particleMesh = gRenderer->CreateParticleMesh(L"0_Particle/Slash");

		// 첫 색상
		particleSystem->SetStartColor(DirectX::SimpleMath::Color(1.5, 0.7, 1.2, 1));
		// 루프 설정
		particleSystem->SetLoop(false);
		// 총 지속 시간
		particleSystem->SetDuration(2.f);
		// 한 번 재생하고 끝낼지
		particleSystem->SetStopWhenFinished(false);
		// 일단 더 추가될 예정인 렌더 모드. 
		// Mesh일 경우 추가한 매쉬의 모양으로 나온다.
		// Billboard일 경우 항상 매쉬가 날 바라보게 만들 예정
		particleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		// 렌더모드가 매쉬일 경우 아래의 SetRenderMesh를 통해 매쉬를 추가해준다.
		particleSystem->SetRenderMesh(particleMesh);
		// 지속시간동안 Rotation을 할지 여부
		particleSystem->SetRotationOverLifetime(true);
		// 아래는 X Y Z축을 기준으로 어떻게 돌지 만약 변경점이 없을 경우 AddControlPoint를 안해줘도 된다.
		//auto& graphX = particleSystem->GetRotationOverLifetimeAxisX();
		//graphX.AddControlPoint({ 0,0 });
		// 아래는 커브 그래프. 순서대로 넣어주어야 한다. 2차원 좌표계. x, y 순
		auto& graphY = particleSystem->GetRotationOverLifetimeAxisY();
		graphY.AddControlPoint({ 0,1 });
		graphY.AddControlPoint({ 0.5, 0 });
		graphY.AddControlPoint({ 1,0 });

		particleSystem->SetTransformMatrix(DirectX::SimpleMath::Matrix::CreateTranslation(0, 10, 0));

		//auto& graphZ = particleSystem->GetRotationOverLifetimeAxisZ();
		//graphZ.AddControlPoint({ 0,0 });

		// 아래는 커스텀 데이터이다. 셰이더에서 커스텀 데이터를 추가하고 싶을 경우 GetCustomData[N][?] (N은 1,2 ?는 XYZW) 로 그래프를 불러와 수정 할 것
		auto& custom1X = particleSystem->GetCustomData1X();
		custom1X.AddControlPoint({ 0,1 });
		custom1X.AddControlPoint({ 1,0 });
#pragma endregion
#pragma region EnemyAimRing
		std::shared_ptr<Ideal::IParticleMaterial> magicCircleMaterial = gRenderer->CreateParticleMaterial();
		magicCircleMaterial->SetShader(defaultTextureParticleShader);
		std::shared_ptr<Ideal::ITexture> magicCircleTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Enemy/MagicCircle51.png");
		particleTexturesToDelete.push_back(magicCircleTexture);

		magicCircleMaterial->SetTexture0(magicCircleTexture);
		magicCircleMaterial->SetBackFaceCulling(false);
		magicCircleMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);

		std::shared_ptr<Ideal::IParticleSystem> magicCircleParticleSystem = gRenderer->CreateParticleSystem(magicCircleMaterial);
		magicCircleParticleSystem->SetLoop(true);
		magicCircleParticleSystem->SetDuration(2.5f);
		magicCircleParticleSystem->SetStartLifetime(2.5f);
		magicCircleParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		magicCircleParticleSystem->SetRenderMesh(particleMeshPlane);
		magicCircleParticleSystem->SetTransformMatrix(Matrix::CreateScale(0.5f) * Matrix::CreateRotationX(1.57f) * Matrix::CreateTranslation(Vector3(0, 3, 0)));
		magicCircleParticleSystem->SetStartColor(Color(1, 1, 1, 1));

		// 아래 색 조정, 사이즈 조절은 유니티 별로여서 걍 내가 넣은거임. 알아서 넣을지 말지 결정하셈
		magicCircleParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = magicCircleParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 0), 0.f);
			graph.AddPoint(Color(1, 1, 1, 1), 0.5f / 2.5f);
			graph.AddPoint(Color(1, 1, 1, 0), 2.5f / 2.5f);
		}
		magicCircleParticleSystem->SetSizeOverLifetime(true);
		{
			auto& graph = magicCircleParticleSystem->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.7f });
			graph.AddControlPoint({ 2.5f, 0.9f });
		}
		{
			auto& graph = magicCircleParticleSystem->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1.f });
			graph.AddControlPoint({ 2.5f,1.f });
		}
		{
			auto& graph = magicCircleParticleSystem->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.7f });
			graph.AddControlPoint({ 2.5f  ,0.9f });
		}
#pragma endregion
#pragma region EnemyAimAttack
		std::shared_ptr<Ideal::IParticleMaterial> bowAttackMaterial = gRenderer->CreateParticleMaterial();
		bowAttackMaterial->SetShader(defaultTextureParticleShader);
		std::shared_ptr<Ideal::ITexture> bowAttackTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Enemy/FX_117.png");
		particleTexturesToDelete.push_back(bowAttackTexture);

		bowAttackMaterial->SetTexture0(bowAttackTexture);
		bowAttackMaterial->SetBackFaceCulling(false);
		bowAttackMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);

		std::shared_ptr<Ideal::IParticleSystem> bowAttackParticleSystem = gRenderer->CreateParticleSystem(bowAttackMaterial);
		bowAttackParticleSystem->SetLoop(true);
		bowAttackParticleSystem->SetDuration(2.5f);
		bowAttackParticleSystem->SetStartLifetime(2.5f);
		bowAttackParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		bowAttackParticleSystem->SetRenderMesh(particleMeshPlane);
		bowAttackParticleSystem->SetTransformMatrix(Matrix::CreateScale(0.8f) * Matrix::CreateRotationX(1.57f) * Matrix::CreateRotationY(-1.57f) * Matrix::CreateTranslation(Vector3(0, 3, -0.2f)));
		bowAttackParticleSystem->SetStartColor(Color(1, 1, 1, 1));
		// 아래 색 조정, 사이즈 조절은 유니티 별로여서 걍 내가 넣은거임. 알아서 넣을지 말지 결정하셈
		bowAttackParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = bowAttackParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 0), 0.f);
			graph.AddPoint(Color(1, 1, 1, 1), 0.5f / 2.5f);
			graph.AddPoint(Color(1, 1, 1, 0), 2.5f / 2.5f);
		}

#pragma endregion
#pragma region Nor_Attack_Effect
		//---------------Particle Sword Slash-------------------//
		// Shader
		std::shared_ptr<Ideal::IParticleMaterial> slashParticleMaterial = gRenderer->CreateParticleMaterial();
		slashParticleMaterial->SetShader(swordParticleShader);
		//
		std::shared_ptr<Ideal::ITexture> slashParticleTexture0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke12.png");
		slashParticleMaterial->SetTexture0(slashParticleTexture0);
		std::shared_ptr<Ideal::ITexture> slashParticleTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Crater62.png");
		slashParticleMaterial->SetTexture1(slashParticleTexture1);
		std::shared_ptr<Ideal::ITexture> slashParticleTexture2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Noise43b.png");
		slashParticleMaterial->SetTexture2(slashParticleTexture2);

		particleTexturesToDelete.push_back(slashParticleTexture0);
		particleTexturesToDelete.push_back(slashParticleTexture1);
		particleTexturesToDelete.push_back(slashParticleTexture2);

		slashParticleMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);
		//slashParticleMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Additive);
		//
		std::shared_ptr<Ideal::IParticleSystem> slashParticleSystem = gRenderer->CreateParticleSystem(slashParticleMaterial);
		//
		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Slash3.fbx");
		// 미리 컨버팅한 매쉬를 불러온다.
		std::shared_ptr<Ideal::IMesh> slashParticleMesh = gRenderer->CreateParticleMesh(L"0_Particle/Slash3");
		//
		slashParticleSystem->SetLoop(false);
		slashParticleSystem->SetDuration(2.f);
		// 첫 색상
		slashParticleSystem->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));

		// 라이프타임
		slashParticleSystem->SetStartLifetime(1.f);

		// Renderer 
		slashParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		slashParticleSystem->SetRenderMesh(slashParticleMesh);
		//
		slashParticleSystem->SetRotationOverLifetime(true);
		{
			auto& graphY = slashParticleSystem->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = slashParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = slashParticleSystem->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = slashParticleSystem->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = slashParticleSystem->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		// use color over lifetime
		slashParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = slashParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}
#pragma endregion
#pragma region Nor_S_Attack_Effect
		// Nor_S_Attack_Effect
		std::shared_ptr<Ideal::IParticleSystem> norAttackEffect0 = gRenderer->CreateParticleSystem(slashParticleMaterial);
		norAttackEffect0->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norAttackEffect0->SetRenderMesh(slashParticleMesh);
		norAttackEffect0->SetLoop(false);
		norAttackEffect0->SetDuration(2.f);
		norAttackEffect0->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		norAttackEffect0->SetRotationOverLifetime(true);
		norAttackEffect0->SetTransformMatrix(
			Matrix::CreateScale(Vector3(1.5, 3, 1.5))
			* Matrix::CreateRotationZ(0.13f)
			* Matrix::CreateTranslation(Vector3(3, 0, 0))
		);

		{
			auto& graphY = norAttackEffect0->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect0->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect0->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = norAttackEffect0->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = norAttackEffect0->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		norAttackEffect0->SetSizeOverLifetime(true);

		{
			auto& graph = norAttackEffect0->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}

		{
			auto& graph = norAttackEffect0->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}

		{
			auto& graph = norAttackEffect0->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}


		// use color over lifetime
		norAttackEffect0->SetColorOverLifetime(true);
		{
			auto& graph = norAttackEffect0->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}

		// Nor_S_Attack_Effect2
		std::shared_ptr<Ideal::IParticleSystem> norAttackEffect1 = gRenderer->CreateParticleSystem(slashParticleMaterial);
		norAttackEffect1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norAttackEffect1->SetRenderMesh(slashParticleMesh);
		norAttackEffect1->SetLoop(false);
		norAttackEffect1->SetDuration(2.f);
		norAttackEffect1->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		norAttackEffect1->SetRotationOverLifetime(true);
		norAttackEffect1->SetTransformMatrix(
			Matrix::CreateScale(Vector3(1.5, 3, 1.5))
			* Matrix::CreateRotationY(2.44f)
			* Matrix::CreateRotationZ(-0.13f)
			* Matrix::CreateTranslation(Vector3(3, 0, 0))
		);

		{
			auto& graphY = norAttackEffect1->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect1->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect1->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = norAttackEffect1->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = norAttackEffect1->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		norAttackEffect1->SetSizeOverLifetime(true);

		{
			auto& graph = norAttackEffect1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}

		{
			auto& graph = norAttackEffect1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}

		{
			auto& graph = norAttackEffect1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0.f, 0.7f });
			graph.AddControlPoint({ 1.f, 1.f });
		}


		// use color over lifetime
		norAttackEffect1->SetColorOverLifetime(true);
		{
			auto& graph = norAttackEffect1->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}

		// norAttack2
		std::shared_ptr<Ideal::IParticleSystem> norAttackEffect2 = gRenderer->CreateParticleSystem(slashParticleMaterial);
		norAttackEffect2->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norAttackEffect2->SetRenderMesh(slashParticleMesh);
		norAttackEffect2->SetLoop(false);
		norAttackEffect2->SetDuration(2.f);
		norAttackEffect2->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		norAttackEffect2->SetTransformMatrix(
			Matrix::CreateRotationY(2.44f)
			* Matrix::CreateTranslation(Vector3(3, 0, 0))
		);

		norAttackEffect2->SetRotationOverLifetime(true);
		{
			auto& graphY = norAttackEffect2->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect2->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = norAttackEffect2->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = norAttackEffect2->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = norAttackEffect2->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		// use color over lifetime
		norAttackEffect2->SetColorOverLifetime(true);
		{
			auto& graph = norAttackEffect2->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}
#pragma endregion
#pragma region ComAttack
		//------Com Attack------//
		// com attack 0
		std::shared_ptr<Ideal::IParticleSystem> comAttack0 = gRenderer->CreateParticleSystem(slashParticleMaterial);
		comAttack0->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		comAttack0->SetRenderMesh(slashParticleMesh);
		comAttack0->SetLoop(false);
		comAttack0->SetDuration(2.f);
		comAttack0->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		comAttack0->SetTransformMatrix(
			Matrix::CreateRotationY(-2.7925f)
			* Matrix::CreateTranslation(Vector3(6, 0, 0))
		);

		comAttack0->SetRotationOverLifetime(true);
		{
			auto& graphY = comAttack0->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = comAttack0->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = comAttack0->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = comAttack0->GetCustomData2Z();
			graph.AddControlPoint({ 0,0.3 });
		}
		{
			auto& graph = comAttack0->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		// use color over lifetime
		comAttack0->SetColorOverLifetime(true);
		{
			auto& graph = comAttack0->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}

		// com attack 1
		std::shared_ptr<Ideal::IParticleSystem> comAttack1 = gRenderer->CreateParticleSystem(slashParticleMaterial);
		comAttack1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);

		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Slash4.fbx");
		// 미리 컨버팅한 매쉬를 불러온다.
		std::shared_ptr<Ideal::IMesh> slashParticleMesh2 = gRenderer->CreateParticleMesh(L"0_Particle/Slash4");
		comAttack1->SetRenderMesh(slashParticleMesh2);
		comAttack1->SetLoop(false);
		comAttack1->SetDuration(2.f);
		comAttack1->SetStartColor(DirectX::SimpleMath::Color(1, 1, 1, 1));
		comAttack1->SetTransformMatrix(
			Matrix::CreateScale(Vector3(1.5f, 1.2f, 1.5f))
			* Matrix::CreateRotationY(-3.14f)
			* Matrix::CreateTranslation(Vector3(6, 0, 0))
		);

		comAttack1->SetRotationOverLifetime(true);
		{
			auto& graphY = comAttack1->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = comAttack1->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = comAttack1->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = comAttack1->GetCustomData2Z();
			graph.AddControlPoint({ 0,0.3 });
		}
		{
			auto& graph = comAttack1->GetCustomData2W();
			graph.AddControlPoint({ 0,0.2 });
		}

		comAttack1->SetSizeOverLifetime(true);
		{
			auto& graph = comAttack1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.6 });
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = comAttack1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,0.6 });
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = comAttack1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.6 });
			graph.AddControlPoint({ 1,1 });
		}
		// use color over lifetime
		comAttack1->SetColorOverLifetime(true);
		{
			auto& graph = comAttack1->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 1, 1, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(1, 1, 1, 0), 1.f);	// 끝 색상
		}

#pragma endregion
#pragma region DodgeEffect
		//----------Dodge effect----------//
		std::shared_ptr<Ideal::IParticleMaterial> slashParticleMaterial1 = gRenderer->CreateParticleMaterial();
		slashParticleMaterial1->SetShader(swordParticleShader);
		//
		std::shared_ptr<Ideal::ITexture> slashParticleTexture1_0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke12.png");
		slashParticleMaterial1->SetTexture0(slashParticleTexture1_0);
		std::shared_ptr<Ideal::ITexture> slashParticleTexture1_1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Fire13.png");
		slashParticleMaterial1->SetTexture1(slashParticleTexture1_1);
		//particleMaterial->SetTexture0(slashParticleTexture1);
		std::shared_ptr<Ideal::ITexture> slashParticleTexture1_2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Noise43b.png");
		slashParticleMaterial1->SetTexture2(slashParticleTexture1_2);


		particleTexturesToDelete.push_back(slashParticleTexture1_0);
		particleTexturesToDelete.push_back(slashParticleTexture1_1);
		particleTexturesToDelete.push_back(slashParticleTexture1_2);

		slashParticleMaterial1->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);


		std::shared_ptr<Ideal::IParticleSystem> dodgeEffect = gRenderer->CreateParticleSystem(slashParticleMaterial1);
		dodgeEffect->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		dodgeEffect->SetRenderMesh(slashParticleMesh);
		dodgeEffect->SetLoop(false);
		dodgeEffect->SetDuration(2.f);
		dodgeEffect->SetStartColor(DirectX::SimpleMath::Color(0.2509f, 0, 0.7529f, 1));
		dodgeEffect->SetTransformMatrix(
			Matrix::CreateRotationY(-2.44f)
			* Matrix::CreateTranslation(Vector3(9, 0, 0))
		);

		dodgeEffect->SetRotationOverLifetime(true);
		{
			auto& graphY = dodgeEffect->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = dodgeEffect->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = dodgeEffect->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = dodgeEffect->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = dodgeEffect->GetCustomData2W();
			graph.AddControlPoint({ 0,1 });
		}

		// use color over lifetime
		dodgeEffect->SetColorOverLifetime(true);
		{
			auto& graph = dodgeEffect->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0.2509f, 0, 0.7529f, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(0.2509f, 0, 0.7529f, 0), 1.f);	// 끝 색상
		}
#pragma endregion
#pragma region FireExplosionBillboardParticle
		std::shared_ptr<Ideal::IParticleMaterial> fireExplosionMaterial = gRenderer->CreateParticleMaterial();
		//billboardMaterialTest->SetShader(billboardTestPS);
		fireExplosionMaterial->SetShader(bossFireFloorShader);
		fireExplosionMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);
		std::shared_ptr<Ideal::ITexture> particleBillboardAnimationSheet = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/bossFireFloor/Explosion_1.png");
		std::shared_ptr<Ideal::ITexture> particleBillboardNormal = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/BossBlackHole/Normal_4.png");
		fireExplosionMaterial->SetTexture0(particleBillboardAnimationSheet);
		fireExplosionMaterial->SetTexture1(particleBillboardNormal);
		fireExplosionMaterial->SetWriteDepthBuffer(true);

		std::shared_ptr<Ideal::IParticleSystem> fireExplosionParticle = gRenderer->CreateParticleSystem(fireExplosionMaterial);
		fireExplosionParticle->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Billboard);
		//fireExplosionParticle->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);	-> 테스트용
		//fireExplosionParticle->SetRenderMesh(particleMeshPlane);							-> 테스트용

		fireExplosionParticle->SetActive(true);
		fireExplosionParticle->SetLoop(false);
		fireExplosionParticle->SetDuration(1.f);
		fireExplosionParticle->SetStartLifetime(1.f);
		fireExplosionParticle->SetSimulationSpeed(2.f);
		fireExplosionParticle->SetMaxParticles(50);
		fireExplosionParticle->SetShapeMode(true);
		fireExplosionParticle->SetShape(Ideal::ParticleMenu::EShape::Circle);
		fireExplosionParticle->SetRadius(1.f);
		fireExplosionParticle->SetRadiusThickness(0.5f);

		fireExplosionParticle->SetVelocityOverLifetime(true);
		fireExplosionParticle->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Random);
		fireExplosionParticle->SetVelocityDirectionRandom(-10.f, 10.f);
		//fireExplosionParticle->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Random);
		//fireExplosionParticle->SetVelocitySpeedModifierRandom(0.f, 0.9f);
		fireExplosionParticle->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Const);
		fireExplosionParticle->SetVelocitySpeedModifierConst(0.f);
		//billboardTest->SetTransformMatrix(Matrix::CreateRotationX(1.57f));

		// Animation
		fireExplosionParticle->SetTextureSheetAnimation(true);
		fireExplosionParticle->SetTextureSheetAnimationTiles({ 8,8 });
		fireExplosionParticle->SetTransformMatrix(Matrix::CreateRotationX(1.57f) * Matrix::CreateTranslation(Vector3(0, 3, 0)));


		fireExplosionParticle->SetStartColor(Color(2.2f, 0.f, 0.f, 1.f));
		fireExplosionParticle->SetColorOverLifetime(true);
		{
			auto& graph = fireExplosionParticle->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 0.f), 0.f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 1.f), 0.12f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 1.f), 0.388f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 0.f), 1.f);
		}

		{
			auto& graph = fireExplosionParticle->GetCustomData1Z();
			graph.AddControlPoint({ 0,6 });
		}

#pragma endregion

#pragma region FireExplosionBillboardParticle2
		std::shared_ptr<Ideal::IParticleSystem> fireExplosionParticle2 = gRenderer->CreateParticleSystem(fireExplosionMaterial);
		fireExplosionParticle2->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Billboard);
		//fireExplosionParticle->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);	-> 테스트용
		//fireExplosionParticle->SetRenderMesh(particleMeshPlane);							-> 테스트용

		fireExplosionParticle2->SetActive(true);
		fireExplosionParticle2->SetLoop(false);
		fireExplosionParticle2->SetDuration(5.f);
		fireExplosionParticle2->SetStartLifetime(5.f);

		fireExplosionParticle2->SetMaxParticles(100);
		fireExplosionParticle2->SetShapeMode(true);
		fireExplosionParticle2->SetShape(Ideal::ParticleMenu::EShape::Circle);
		fireExplosionParticle2->SetRadius(1.f);
		fireExplosionParticle2->SetRadiusThickness(0.5f);

		fireExplosionParticle2->SetVelocityOverLifetime(true);
		//fireExplosionParticle2->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Random);
		//fireExplosionParticle2->SetVelocityDirectionRandom(-10.f, 10.f);
		fireExplosionParticle2->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Const);
		fireExplosionParticle2->SetVelocityDirectionConst(Vector3(0, 1, 0));
		fireExplosionParticle2->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Random);
		fireExplosionParticle2->SetVelocitySpeedModifierRandom(0.f, 0.9f);

		fireExplosionParticle2->SetRateOverTime(true);
		fireExplosionParticle2->SetEmissionRateOverTime(25.f);

		// Animation
		fireExplosionParticle2->SetTextureSheetAnimation(true);
		fireExplosionParticle2->SetTextureSheetAnimationTiles({ 8,8 });
		fireExplosionParticle2->SetTransformMatrix(Matrix::CreateRotationX(1.57f) * Matrix::CreateTranslation(Vector3(0, 3, 0)));


		fireExplosionParticle2->SetStartColor(Color(2.2f, 0.f, 0.f, 1.f));
		fireExplosionParticle2->SetColorOverLifetime(true);
		{
			auto& graph = fireExplosionParticle2->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 0.f), 0.f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 1.f), 0.12f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 1.f), 0.388f);
			graph.AddPoint(Color(2.2f, 0.f, 0.f, 0.f), 0.f);
		}

		{
			auto& graph = fireExplosionParticle2->GetCustomData1Z();
			graph.AddControlPoint({ 0,6 });
		}

#pragma endregion

#pragma region Nor_Damage

		std::shared_ptr<Ideal::IParticleMaterial> norDamageMaterial0 = gRenderer->CreateParticleMaterial();
		norDamageMaterial0->SetShader(bossBeamRingShader);
		norDamageMaterial0->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		norDamageMaterial0->SetBackFaceCulling(false);
		std::shared_ptr<Ideal::ITexture> norDamageTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Flash21.png");
		norDamageMaterial0->SetTexture0(norDamageTexture);

		std::shared_ptr<Ideal::IParticleSystem> norDamageParticleSystem0 = gRenderer->CreateParticleSystem(norDamageMaterial0);

		norDamageParticleSystem0->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norDamageParticleSystem0->SetRenderMesh(particleMeshPlane);
		//norDamageParticleSystem0->SetRenderMeshBillboard(false);
		norDamageParticleSystem0->SetStartSize(0.3f);
		norDamageParticleSystem0->SetStartLifetime(1.f);
		norDamageParticleSystem0->SetDuration(1.f);
		norDamageParticleSystem0->SetSimulationSpeed(8.f);
		norDamageParticleSystem0->SetLoop(false);
		norDamageParticleSystem0->SetStartColor(Color(1.f, 1.f, 1.f, 1.f));

		norDamageParticleSystem0->SetSizeOverLifetime(true);
		norDamageParticleSystem0->SetTransformMatrix(Matrix::CreateRotationX(1.07f) * Matrix::CreateTranslation(0, 5, 0)); // 로테이션은 적용, 위치는 데모에서 위치 확인용
		{
			auto& graph = norDamageParticleSystem0->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}
		{
			auto& graph = norDamageParticleSystem0->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1.f });
		}
		{
			auto& graph = norDamageParticleSystem0->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}

		std::shared_ptr<Ideal::IParticleSystem> norDamageParticleSystem0_1 = gRenderer->CreateParticleSystem(norDamageMaterial0);

		norDamageParticleSystem0_1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norDamageParticleSystem0_1->SetRenderMesh(particleMeshPlane);
		norDamageParticleSystem0_1->SetStartSize(0.3f);
		norDamageParticleSystem0_1->SetStartLifetime(1.f);
		norDamageParticleSystem0_1->SetDuration(1.f);
		norDamageParticleSystem0_1->SetSimulationSpeed(8.f);
		norDamageParticleSystem0_1->SetLoop(false);
		norDamageParticleSystem0_1->SetStartColor(Color(1.f, 1.f, 1.f, 1.f));
		norDamageParticleSystem0_1->SetSizeOverLifetime(true);
		norDamageParticleSystem0_1->SetTransformMatrix(Matrix::CreateRotationX(1.57f) * Matrix::CreateRotationY(1.57f) * Matrix::CreateTranslation(0, 5, 0)); // 로테이션은 적용, 위치는 데모에서 위치 확인용
		{
			auto& graph = norDamageParticleSystem0_1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}
		{
			auto& graph = norDamageParticleSystem0_1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1.f });
		}
		{
			auto& graph = norDamageParticleSystem0_1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}

		//1 

		std::shared_ptr<Ideal::IParticleMaterial> norDamageMaterial1 = gRenderer->CreateParticleMaterial();
		norDamageMaterial1->SetShader(bossBeamRingShader);
		norDamageMaterial1->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		norDamageMaterial1->SetBackFaceCulling(false);
		std::shared_ptr<Ideal::ITexture> norDamageTexture1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Flare23.png");
		norDamageMaterial1->SetTexture0(norDamageTexture1);

		std::shared_ptr<Ideal::IParticleSystem> norDamageParticleSystem1 = gRenderer->CreateParticleSystem(norDamageMaterial1);

		norDamageParticleSystem1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norDamageParticleSystem1->SetRenderMesh(particleMeshPlane);
		norDamageParticleSystem1->SetStartSize(0.3f);
		norDamageParticleSystem1->SetStartLifetime(1.f);
		norDamageParticleSystem1->SetDuration(1.f);
		norDamageParticleSystem1->SetSimulationSpeed(8.f);
		norDamageParticleSystem1->SetLoop(false);
		norDamageParticleSystem1->SetStartColor(Color(1.f, 1.f, 1.f, 1.f));
		norDamageParticleSystem1->SetSizeOverLifetime(true);
		norDamageParticleSystem1->SetTransformMatrix(Matrix::CreateRotationX(1.57f) * Matrix::CreateTranslation(0, 5, 0)); // 로테이션은 적용, 위치는 데모에서 위치 확인용
		{
			auto& graph = norDamageParticleSystem1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}
		{
			auto& graph = norDamageParticleSystem1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1.f });
		}
		{
			auto& graph = norDamageParticleSystem1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}

		std::shared_ptr<Ideal::IParticleSystem> norDamageParticleSystem1_1 = gRenderer->CreateParticleSystem(norDamageMaterial1);

		norDamageParticleSystem1_1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		norDamageParticleSystem1_1->SetRenderMesh(particleMeshPlane);
		norDamageParticleSystem1_1->SetStartSize(0.3f);
		norDamageParticleSystem1_1->SetStartLifetime(1.f);
		norDamageParticleSystem1_1->SetDuration(1.f);
		norDamageParticleSystem1_1->SetSimulationSpeed(8.f);
		norDamageParticleSystem1_1->SetLoop(false);
		norDamageParticleSystem1_1->SetStartColor(Color(1.f, 1.f, 1.f, 1.f));
		norDamageParticleSystem1_1->SetSizeOverLifetime(true);
		norDamageParticleSystem1_1->SetTransformMatrix(Matrix::CreateRotationX(1.57f) * Matrix::CreateRotationY(1.57f) * Matrix::CreateTranslation(0, 5, 0)); // 로테이션은 적용, 위치는 데모에서 위치 확인용
		{
			auto& graph = norDamageParticleSystem1_1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}
		{
			auto& graph = norDamageParticleSystem1_1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1.f });
		}
		{
			auto& graph = norDamageParticleSystem1_1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.5f });
			graph.AddControlPoint({ 1,3 });
		}

		// 2 Particle
		std::shared_ptr<Ideal::IParticleMaterial> norDamageMaterial2 = gRenderer->CreateParticleMaterial();
		norDamageMaterial2->SetShader(bossBeamRingShader);
		norDamageMaterial2->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		std::shared_ptr<Ideal::ITexture> glowTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Glow1.png");
		norDamageMaterial2->SetTexture0(glowTexture);
		norDamageMaterial2->SetWriteDepthBuffer(false);

		std::shared_ptr<Ideal::IParticleSystem> norDamageParticleSystem2 = gRenderer->CreateParticleSystem(norDamageMaterial2);
		norDamageParticleSystem2->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Billboard);
		//norDamageParticleSystem2->SetRenderMeshBillboard(true);
		norDamageParticleSystem2->SetMaxParticles(20);
		norDamageParticleSystem2->SetDuration(0.4f);
		norDamageParticleSystem2->SetStartLifetime(0.4f);
		norDamageParticleSystem2->SetLoop(false);
		norDamageParticleSystem2->SetShapeMode(true);
		norDamageParticleSystem2->SetShape(Ideal::ParticleMenu::EShape::Circle);
		norDamageParticleSystem2->SetRadius(0.001f);
		norDamageParticleSystem2->SetRadiusThickness(1.f);
		norDamageParticleSystem2->SetVelocityOverLifetime(true);
		norDamageParticleSystem2->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Random);
		norDamageParticleSystem2->SetVelocityDirectionRandom(-10.f, 10.f);
		norDamageParticleSystem2->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Random);
		norDamageParticleSystem2->SetVelocitySpeedModifierRandom(1.8f, 2.f);
		norDamageParticleSystem2->SetTransformMatrix(Matrix::CreateTranslation(0, 5, 0)); // 데모에서 위치 확인용
		norDamageParticleSystem2->SetSizeOverLifetime(true);
		{
			auto& graph = norDamageParticleSystem2->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0, 0.03f });
			graph.AddControlPoint({ 0.2f, 0.01f });
		}
		{
			auto& graph = norDamageParticleSystem2->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0, 0.03f });
			graph.AddControlPoint({ 0.2f, 0.01f });
		}
#pragma endregion

#pragma region Enforce_Com_S_Attack_Ground_Effect
		std::shared_ptr<Ideal::IParticleMaterial> groundEffectMaterial = gRenderer->CreateParticleMaterial();
		groundEffectMaterial->SetShader(groundSpikePS);
		std::shared_ptr<Ideal::ITexture> noiseTex62 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Noise62.png");
		std::shared_ptr<Ideal::ITexture> noiseTex1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Noise1.png");
		groundEffectMaterial->SetTexture0(noiseTex62);
		groundEffectMaterial->SetTexture1(noiseTex1);
		groundEffectMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);
		groundEffectMaterial->SetBackFaceCulling(false);

		std::shared_ptr<Ideal::IParticleSystem> groundEffectParticleSystem = gRenderer->CreateParticleSystem(groundEffectMaterial);
		groundEffectParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);

		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/IceSpikes2.fbx");
		std::shared_ptr<Ideal::IMesh> iceSpikeMesh = gRenderer->CreateParticleMesh(L"0_Particle/IceSpikes2");
		groundEffectParticleSystem->SetRenderMesh(iceSpikeMesh);
		groundEffectParticleSystem->SetStartSize(1.f);
		groundEffectParticleSystem->SetStartColor(Color(1.f, 0.5295228f, 0.259434f, 1.f));
		//groundEffectParticleSystem->SetStartColor(Color(0.f, 0.f, 1.3f, 1.f));	// 테스트용
		groundEffectParticleSystem->SetLoop(false);
		groundEffectParticleSystem->SetStartLifetime(5.f);
		groundEffectParticleSystem->SetDuration(5.f);
		groundEffectParticleSystem->SetTransformMatrix(Matrix::CreateTranslation(Vector3(3, 5, 0))); // 데모에서 위치 확인용
		groundEffectParticleSystem->SetSimulationSpeed(2.f);
		groundEffectParticleSystem->SetSizeOverLifetime(true);
		{
			auto& graph = groundEffectParticleSystem->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0, 0 });
			graph.AddControlPoint({ 0.07, 1.5 });
			graph.AddControlPoint({ 0.1, 1.3 });
			graph.AddControlPoint({ 5, 1.3 });
		}
		{
			auto& graph = groundEffectParticleSystem->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0, 0 });
			graph.AddControlPoint({ 0.07, 1.5 });
			graph.AddControlPoint({ 0.1, 1.3 });
			graph.AddControlPoint({ 5, 1.3 });
		}
		{
			auto& graph = groundEffectParticleSystem->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0, 0 });
			graph.AddControlPoint({ 0.07, 1.5 });
			graph.AddControlPoint({ 0.1, 1.3 });
			graph.AddControlPoint({ 5, 1.3 });
		}

		{
			auto& graph = groundEffectParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0, 0 });
			graph.AddControlPoint({ 5, 1 });
		}
		// Ground Effect Smoke
		std::shared_ptr<Ideal::IParticleMaterial> groundSmokeEffectMaterial = gRenderer->CreateParticleMaterial();
		groundSmokeEffectMaterial->SetShader(defaultParticlePS_Clip);
		groundSmokeEffectMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		std::shared_ptr<Ideal::ITexture> smokeTex = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke24.png");
		groundSmokeEffectMaterial->SetTexture0(smokeTex);
		groundSmokeEffectMaterial->SetWriteDepthBuffer(true);
		std::shared_ptr<Ideal::IParticleSystem> groundSmokeParticleSystem = gRenderer->CreateParticleSystem(groundSmokeEffectMaterial);
		groundSmokeParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Billboard);
		groundSmokeParticleSystem->SetLoop(false);
		groundSmokeParticleSystem->SetDuration(0.2f);
		groundSmokeParticleSystem->SetStartLifetime(0.2f);

		groundSmokeParticleSystem->SetMaxParticles(40);
		groundSmokeParticleSystem->SetShapeMode(true);
		groundSmokeParticleSystem->SetShape(Ideal::ParticleMenu::EShape::Circle);
		groundSmokeParticleSystem->SetRadius(0.1f);
		groundSmokeParticleSystem->SetRadiusThickness(0.5f);

		groundSmokeParticleSystem->SetVelocityOverLifetime(true);
		groundSmokeParticleSystem->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Random);
		groundSmokeParticleSystem->SetVelocityDirectionRandom(-10.f, 10.f);
		groundSmokeParticleSystem->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Random);
		groundSmokeParticleSystem->SetVelocitySpeedModifierRandom(10.f, 13.f);

		groundSmokeParticleSystem->SetTransformMatrix(Matrix::CreateTranslation(Vector3(3, 5, 0))); // 데모에서 위치 확인용
		groundSmokeParticleSystem->SetStartColor(Color(0.8f, 0.6f, 0.4f, 1.f));

		// Ground Fire Effect
		std::shared_ptr<Ideal::IParticleMaterial> groundFireEffectMaterial = gRenderer->CreateParticleMaterial();
		groundFireEffectMaterial->SetShader(bossFireFloorShader);
		groundFireEffectMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		std::shared_ptr<Ideal::ITexture> smokeTex21 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke21.png");
		groundFireEffectMaterial->SetTexture0(smokeTex21);
		groundFireEffectMaterial->SetWriteDepthBuffer(true);

		std::shared_ptr<Ideal::IParticleSystem> groundFireParticleSystem = gRenderer->CreateParticleSystem(groundFireEffectMaterial);

		groundFireParticleSystem->SetTransformMatrix(Matrix::CreateTranslation(Vector3(3, 5, 0))); // 데모에서 위치 확인용


		groundFireParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Billboard);
		groundFireParticleSystem->SetDuration(0.2f);
		groundFireParticleSystem->SetStartLifetime(0.2f);

		groundFireParticleSystem->SetMaxParticles(40);
		groundFireParticleSystem->SetShapeMode(true);
		groundFireParticleSystem->SetShape(Ideal::ParticleMenu::EShape::Circle);
		groundFireParticleSystem->SetRadius(0.1f);
		groundFireParticleSystem->SetRadiusThickness(1.f);
		groundFireParticleSystem->SetLoop(false);

		groundFireParticleSystem->SetVelocityOverLifetime(true);
		groundFireParticleSystem->SetVelocityDirectionMode(Ideal::ParticleMenu::EMode::Random);
		groundFireParticleSystem->SetVelocityDirectionRandom(-10.f, 10.f);
		groundFireParticleSystem->SetVelocitySpeedModifierMode(Ideal::ParticleMenu::EMode::Random);
		groundFireParticleSystem->SetVelocitySpeedModifierRandom(5.f, 7.f);

		groundFireParticleSystem->SetTextureSheetAnimation(true);
		groundFireParticleSystem->SetTextureSheetAnimationTiles({ 8,8 });

		groundFireParticleSystem->SetStartColor(Color(1, 0.72, 0.3, 1));


#pragma endregion

#pragma region AbilitySlash

		std::shared_ptr<Ideal::IParticleMaterial> abilitySlashMaterial = gRenderer->CreateParticleMaterial();
		abilitySlashMaterial->SetShader(swordParticleShader);
		//
		//std::shared_ptr<Ideal::ITexture> slashParticleTexture1_0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke12.png");
		abilitySlashMaterial->SetTexture0(slashParticleTexture1_0);
		//std::shared_ptr<Ideal::ITexture> slashParticleTexture1_1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Fire13.png");
		abilitySlashMaterial->SetTexture1(slashParticleTexture1_1);
		//particleMaterial->SetTexture0(slashParticleTexture1);
		//std::shared_ptr<Ideal::ITexture> slashParticleTexture1_2 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Noise43b.png");
		abilitySlashMaterial->SetTexture2(slashParticleTexture1_2);

		abilitySlashMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		//abilitySlashMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);

		std::shared_ptr<Ideal::IParticleSystem> abilitySlashParticleSystem = gRenderer->CreateParticleSystem(abilitySlashMaterial);
		abilitySlashParticleSystem->SetTransformMatrix(
			Matrix::CreateRotationY(-2.44f)
			* Matrix::CreateTranslation(Vector3(13, 0, 0))
		);


		abilitySlashParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		abilitySlashParticleSystem->SetRenderMesh(slashParticleMesh);
		abilitySlashParticleSystem->SetLoop(false);
		abilitySlashParticleSystem->SetDuration(2.f);
		//abilitySlashParticleSystem->SetStartColor(DirectX::SimpleMath::Color(0.2509f, 0, 0.7529f, 1));
		abilitySlashParticleSystem->SetStartColor(Color(0.5647f, 0.0705f, 1.f, 0.5));
		abilitySlashParticleSystem->SetRotationOverLifetime(true);
		{
			auto& graphY = abilitySlashParticleSystem->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = abilitySlashParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = abilitySlashParticleSystem->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = abilitySlashParticleSystem->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = abilitySlashParticleSystem->GetCustomData2W();
			graph.AddControlPoint({ 0,1 });
		}

		// use color over lifetime
		abilitySlashParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = abilitySlashParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0.3f, 0.07f, 1.f, 1), 0.f);	// 시작 생상
			graph.AddPoint(Color(0.3f, 0.07f, 1.f, 1), 1.f);	// 끝 색상
		}
#pragma endregion

#pragma region EnemyAttack
		std::shared_ptr<Ideal::IParticleMaterial> enemySlashMaterial = gRenderer->CreateParticleMaterial();
		enemySlashMaterial->SetShader(swordParticleShader);
		//
		std::shared_ptr<Ideal::ITexture> smoke27 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke27.png");
		//std::shared_ptr<Ideal::ITexture> mytex0 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Flare23.png");
		//std::shared_ptr<Ideal::ITexture> mytex1 = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Smoke27.png");
		enemySlashMaterial->SetTexture0(smoke27);
		enemySlashMaterial->SetTexture1(norDamageTexture1);
		enemySlashMaterial->SetTexture2(glowTexture);

		enemySlashMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);

		std::shared_ptr<Ideal::IParticleSystem> enemySlashParticleSystem = gRenderer->CreateParticleSystem(enemySlashMaterial);
		enemySlashParticleSystem->SetTransformMatrix(
			Matrix::CreateRotationY(-2.44f)
			* Matrix::CreateTranslation(Vector3(16, 0, 0))
		);

		enemySlashParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		enemySlashParticleSystem->SetRenderMesh(slashParticleMesh);
		enemySlashParticleSystem->SetLoop(false);
		enemySlashParticleSystem->SetDuration(2.f);
		//abilitySlashParticleSystem->SetStartColor(DirectX::SimpleMath::Color(0.2509f, 0, 0.7529f, 1));
		enemySlashParticleSystem->SetStartColor(Color(1, 1, 1.f, 0.5));
		enemySlashParticleSystem->SetRotationOverLifetime(true);
		{
			auto& graphY = enemySlashParticleSystem->GetRotationOverLifetimeAxisY();
			graphY.AddControlPoint({ 0,1 });
			graphY.AddControlPoint({ 0.5, 0 });
			graphY.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = enemySlashParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 0.1,2 });
			graph.AddControlPoint({ 0.3,0.6 });
			graph.AddControlPoint({ 1,0 });
		}
		{
			auto& graph = enemySlashParticleSystem->GetCustomData1Y();
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = enemySlashParticleSystem->GetCustomData2Z();
			graph.AddControlPoint({ 0,0 });
		}
		{
			auto& graph = enemySlashParticleSystem->GetCustomData2W();
			graph.AddControlPoint({ 0,1 });
		}

		// use color over lifetime
		enemySlashParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = enemySlashParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0.f, 0.f, 0.f, 0), 0.f);	// 시작 생상
			graph.AddPoint(Color(0.7311f, 0.9524f, 1.f, 1), 0.3f);	// 시작 생상
			graph.AddPoint(Color(0.f, 0.f, 0.f, 0), 1.f);	// 끝 색상
		}
#pragma endregion

#pragma region EnemyCharge
		gRenderer->ConvertParticleMeshAssetToMyFormat(L"0_Particle/Cone2.fbx");
		std::shared_ptr<Ideal::IMesh> Cone2Mesh = gRenderer->CreateParticleMesh(L"0_Particle/Cone2");

		std::shared_ptr<Ideal::IParticleMaterial> enemyChargeMaterial = gRenderer->CreateParticleMaterial();
		enemyChargeMaterial->SetShader(enemyChargePS);
		//enemyChargeMaterial->SetTexture0(norDamageTexture1);
		enemyChargeMaterial->SetTexture0(glowTexture);
		enemyChargeMaterial->SetTexture1(norDamageTexture1);
		//enemyChargeMaterial->SetTexture1(slashParticleTexture1_1);
		enemyChargeMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		enemyChargeMaterial->SetBackFaceCulling(true);

		std::shared_ptr<Ideal::IParticleSystem> enemyChargeParticleSystem = gRenderer->CreateParticleSystem(enemyChargeMaterial);
		enemyChargeParticleSystem->SetLoop(false);
		enemyChargeParticleSystem->SetStartLifetime(1.f);
		enemyChargeParticleSystem->SetDuration(2.f);
		enemyChargeParticleSystem->SetTransformMatrix(
			Matrix::CreateScale(Vector3(0.5, 0.5, 1))				// 여기는 적용
			* Matrix::CreateTranslation(Vector3(19, 0, 0))	// 여기는 데모 위치 확인용
		);
		enemyChargeParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		enemyChargeParticleSystem->SetRenderMesh(Cone2Mesh);

		{
			auto& graph = enemyChargeParticleSystem->GetCustomData1X();
			graph.AddControlPoint({ 0,5 });
			graph.AddControlPoint({ 2,0 });
		}

		//2
		std::shared_ptr<Ideal::IParticleSystem> enemyChargeParticleSystem2 = gRenderer->CreateParticleSystem(enemyChargeMaterial);
		enemyChargeParticleSystem2->SetLoop(false);
		enemyChargeParticleSystem2->SetStartLifetime(1.f);
		enemyChargeParticleSystem2->SetDuration(2.f);
		enemyChargeParticleSystem2->SetTransformMatrix(
			Matrix::CreateScale(Vector3(0.5, 0.5, 1))				// 여기는 적용
			* Matrix::CreateTranslation(Vector3(19, 0, 0))	// 여기는 데모 위치 확인용
		);
		enemyChargeParticleSystem2->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		enemyChargeParticleSystem2->SetRenderMesh(Cone2Mesh);

		{
			auto& graph = enemyChargeParticleSystem2->GetCustomData1X();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 1,1 });
		}
		{
			auto& graph = enemyChargeParticleSystem2->GetCustomData1Y();
			graph.AddControlPoint({ 0,0 });
			graph.AddControlPoint({ 1,1 });
		}
#pragma endregion

#pragma region Scanner
		std::shared_ptr<Ideal::IParticleMaterial> ScannerMaterial = gRenderer->CreateParticleMaterial();
		ScannerMaterial->SetShader(bossBlackHoleSphereShader);
		ScannerMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		//ScannerMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::Alpha);
		ScannerMaterial->SetBackFaceCulling(false);
		//std::shared_ptr<Ideal::ITexture>

		std::shared_ptr<Ideal::IParticleSystem> ScannerParticleSystem = gRenderer->CreateParticleSystem(ScannerMaterial);
		ScannerParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		ScannerParticleSystem->SetRenderMesh(bossParticleMeshSphere);
		ScannerParticleSystem->SetStartSize(100.f);
		ScannerParticleSystem->SetStartLifetime(2.f);
		ScannerParticleSystem->SetDuration(2.f);
		ScannerParticleSystem->SetSimulationSpeed(2.f);
		ScannerParticleSystem->SetLoop(false);
		ScannerParticleSystem->SetStartColor(Color(0.4f, 0.4f, 0.4f, 1.f));
		ScannerParticleSystem->SetSizeOverLifetime(true);
		{
			auto& graph = ScannerParticleSystem->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0, 0.f });
			graph.AddControlPoint({ 2, 15.f });
		}
		{
			auto& graph = ScannerParticleSystem->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0, 0.f });
			graph.AddControlPoint({ 2, 15.f });
		}
		{
			auto& graph = ScannerParticleSystem->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0, 0.f });
			graph.AddControlPoint({ 2, 15.f });
		}

		ScannerParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = ScannerParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(0.4f, 0.4f, 0.4f, 1), 0.f);
			graph.AddPoint(Color(0, 0, 0, 0), 2.f / 2.f);
		}

#pragma endregion

#pragma region Portal
		std::shared_ptr<Ideal::IParticleMaterial> portalMaterial = gRenderer->CreateParticleMaterial();
		portalMaterial->SetShader(defaultTextureParticleClipShader);
		std::shared_ptr<Ideal::ITexture> portalTexture = gRenderer->CreateTexture(L"../Resources/Textures/0_Particle/Circle.png");
		portalMaterial->SetTexture0(portalTexture);
		portalMaterial->SetBlendingMode(Ideal::ParticleMaterialMenu::EBlendingMode::AlphaAdditive);
		portalMaterial->SetBackFaceCulling(false);
		std::shared_ptr<Ideal::IParticleSystem> portalParticleSystem = gRenderer->CreateParticleSystem(portalMaterial);
		portalParticleSystem->SetTransformMatrix(
			Matrix::CreateRotationX(1.57f)
			* Matrix::CreateTranslation(0, 8, 0)
		);

		portalParticleSystem->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		portalParticleSystem->SetRenderMesh(particleMeshPlane);
		portalParticleSystem->SetStartSize(0.5f);
		portalParticleSystem->SetDuration(1.f);
		portalParticleSystem->SetStartLifetime(1.f);
		portalParticleSystem->SetLoop(true);
		portalParticleSystem->SetStartColor(Color(1, 0, 0, 1));
		portalParticleSystem->SetSizeOverLifetime(true);
		{
			auto& graph = portalParticleSystem->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.7 });
			graph.AddControlPoint({ 1,2 });
		}
		{
			auto& graph = portalParticleSystem->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1 });
		}
		{
			auto& graph = portalParticleSystem->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.7 });
			graph.AddControlPoint({ 1,3 });
		}

		portalParticleSystem->SetColorOverLifetime(true);
		{
			auto& graph = portalParticleSystem->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 0, 0, 1), 0);
			graph.AddPoint(Color(1, 0, 0, 0), 1);
		}

		// 2

		std::shared_ptr<Ideal::IParticleSystem> portalParticleSystem1 = gRenderer->CreateParticleSystem(portalMaterial);
		portalParticleSystem1->SetTransformMatrix(
			Matrix::CreateRotationX(1.57f)
			* Matrix::CreateTranslation(0, 8, 0)
		);

		portalParticleSystem1->SetRenderMode(Ideal::ParticleMenu::ERendererMode::Mesh);
		portalParticleSystem1->SetRenderMesh(particleMeshPlane);
		portalParticleSystem1->SetStartSize(0.5f);
		portalParticleSystem1->SetDuration(1.f);
		portalParticleSystem1->SetStartLifetime(1.f);
		portalParticleSystem1->SetLoop(true);
		portalParticleSystem1->SetStartColor(Color(1, 0, 0, 1));
		portalParticleSystem1->SetSizeOverLifetime(true);
		{
			auto& graph = portalParticleSystem1->GetSizeOverLifetimeAxisX();
			graph.AddControlPoint({ 0,0.3 });
			graph.AddControlPoint({ 1,1.7 });
		}
		{
			auto& graph = portalParticleSystem1->GetSizeOverLifetimeAxisY();
			graph.AddControlPoint({ 0,1 });
		}
		{
			auto& graph = portalParticleSystem1->GetSizeOverLifetimeAxisZ();
			graph.AddControlPoint({ 0,0.3 });
			graph.AddControlPoint({ 1,2.7 });
		}

		portalParticleSystem1->SetColorOverLifetime(true);
		{
			auto& graph = portalParticleSystem1->GetColorOverLifetimeGradientGraph();
			graph.AddPoint(Color(1, 0, 0, 1), 0);
			graph.AddPoint(Color(1, 0, 0, 0), 1);
		}
#pragma endregion
#endif
		DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix world2 = DirectX::SimpleMath::Matrix::Identity;
		float angle = 0.f;
		float angleX = 0.f;
		float angleY = 0.f;
		float angleZ = 0.f;

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				g_FrameCount++;
				ULONGLONG curTick = GetTickCount64();
				if (curTick - g_PrvUpdateTick > 16)
				{
					g_PrvUpdateTick = curTick;
				}
				if (curTick - g_PrvFrameCheckTick > 1000)
				{
					g_PrvFrameCheckTick = curTick;
					WCHAR wchTxt[64];
					swprintf_s(wchTxt, L"FPS:%u", g_FrameCount);
					SetWindowText(g_hWnd, wchTxt);
					g_FrameCount = 0;
				}

				CameraTick(camera, spotLight);

				if (angle > 360.f)
				{
					angle = 0.f;
				}
				world = Matrix::CreateRotationY(DirectX::XMConvertToRadians(angle)) * Matrix::CreateTranslation(Vector3(0.f, 0.f, 0.f));

				world2 = Matrix::CreateRotationX(DirectX::XMConvertToRadians(angleX))
					* Matrix::CreateRotationY(DirectX::XMConvertToRadians(angleY))
					* Matrix::CreateRotationZ(DirectX::XMConvertToRadians(angleZ))
					* Matrix::CreateTranslation(Vector3(0.f, 0.f, 0.f));

#ifdef MAKE_PARTICLE
				if (GetAsyncKeyState('F') & 0x8000)
				{
					slashParticleSystem->Pause();
				}
				//if (GetAsyncKeyState('B') & 0x8000)
				//{
				//	slashParticleSystem->SetSimulationSpeed(2.f);
				//}
				if (GetAsyncKeyState('G') & 0x8000)
				{
					//particleSystem->Play();
					Matrix temp = Matrix::Identity;
					temp *= Matrix::CreateTranslation(Vector3(0.2, 0.2, 0.2));
					slashParticleSystem->SetTransformMatrix(temp);
					slashParticleSystem->Play();
				}
				if (GetAsyncKeyState('H') & 0x8000)
				{
					particleSystem->Play();
					//slashParticleSystem->Play();
				}

				if (GetAsyncKeyState('Z') & 0x8000)
				{
					bossParticleSystem0->Play();
					bossParticleSystem1->Play();
					bossBeamRingParticleSystem->Play();
					//slashParticleSystem->Play();
				}
				if (GetAsyncKeyState('X') & 0x8000)
				{
					bossFireProjectileParticleSystem0->Play();
					bossFireProjectileParticleSystem1->Play();
				}
				if (GetAsyncKeyState('C') & 0x8000)
				{
					bossFireProjectileParticleSystem0->Pause();
					bossFireProjectileParticleSystem1->Pause();

				}
				if (GetAsyncKeyState('V') & 0x8000)
				{
					sphereImpactParticleSystem->Play();
					sphereImpactParticleSystem1->Play();
					sphereImpactParticleSystem2->Play();
				}
				if (GetAsyncKeyState('B') & 0x8000)
				{
					sphereImpactParticleSystem->Pause();
					sphereImpactParticleSystem1->Pause();
					sphereImpactParticleSystem2->Pause();
				}
				if (GetAsyncKeyState('J') & 0x8000)
				{
					blackHoleParticleSystem->Play();
					blackHoleSphereParticleSystem->Play();
				}
				if (GetAsyncKeyState('K') & 0x8000)
				{
					blackHoleParticleSystem->Pause();
					blackHoleSphereParticleSystem->Pause();
				}
				if (GetAsyncKeyState('L') & 0x8000)
				{
					blackHoleParticleSystem->Resume();
					blackHoleSphereParticleSystem->Resume();
				}

				if (GetAsyncKeyState('R') & 0x8000)
				{
					norAttackEffect0->Play();
					norAttackEffect1->Play();
					norAttackEffect2->Play();
				}

				if (GetAsyncKeyState('T') & 0x8000)
				{
					comAttack0->Play();
					comAttack1->Play();
				}

				if (GetAsyncKeyState('Y') & 0x8000)
				{
					dodgeEffect->Play();

					// 위랑 아래랑 다른거임.

					abilitySlashParticleSystem->Play();

					// 위랑 아래랑 다른거임.

					enemySlashParticleSystem->Play();

					// 위랑 아래랑 다른거임.

					enemyChargeParticleSystem->Play();
					enemyChargeParticleSystem2->Play();
				}

				if (GetAsyncKeyState('U') & 0x8000)
				{
					magicCircleParticleSystem->Play();
					bowAttackParticleSystem->Play();
				}
				if (GetAsyncKeyState('I') & 0x8000)
				{
					fireExplosionParticle->Play();
				}
				if (GetAsyncKeyState('O') & 0x8000)
				{
					fireExplosionParticle->Pause();
				}
				if (GetAsyncKeyState('P') & 0x8000)
				{
					fireExplosionParticle2->Play();
				}

				static int textPos = 0;
				static int lightPos = 0;

				if (GetAsyncKeyState(VK_DELETE) & 0x8000)
				{
					norDamageParticleSystem0->Play();
					norDamageParticleSystem0_1->Play();
					norDamageParticleSystem1->Play();
					norDamageParticleSystem1_1->Play();
					norDamageParticleSystem2->Play();


					if (texts.size())
					{
						std::shared_ptr<Ideal::IText> text = texts.back();
						texts.pop_back();
						gRenderer->DeleteText(text);
						textPos--;
					}

					if (pointLights.size())
					{
						std::shared_ptr<Ideal::IPointLight> light = pointLights.back();
						pointLights.pop_back();
						gRenderer->DeleteLight(light);
						lightPos--;
					}
				}
				if (GetAsyncKeyState(VK_END) & 0x8000)
				{
					groundEffectParticleSystem->Play();
					groundSmokeParticleSystem->Play();
					groundFireParticleSystem->Play();

					gRenderer->GetTopLeftEditorPos();
					gRenderer->GetRightBottomEditorPos();

					int a = 3;

					ScannerParticleSystem->Play();

					//

// 					if (texts.size())
// 					{
// 						std::shared_ptr<Ideal::IText> text = texts.back();
// 						texts.pop_back();
// 						gRenderer->DeleteText(text);
// 					}
//					if (texts.size() < 3)
					{
						std::shared_ptr<Ideal::IText> text = gRenderer->CreateText(100, 90, 30);	// 기본 tahoma 글꼴임
						text->ChangeText(L"Test");
						text->SetPosition(Vector2(textPos * 100, 500));
						text->SetZ(0.2);
						texts.push_back(text);
						textPos++;
					}

					{
						std::shared_ptr<Ideal::IPointLight> pointLight = gRenderer->CreatePointLight();
						pointLight->SetPosition(Vector3(lightPos, 3.f, 3.f));
						pointLight->SetRange(6.f);
						pointLight->SetLightColor(Color(1.f, 0.f, 1.f, 1.f));
						pointLight->SetIntensity(6.f);
						pointLights.push_back(pointLight);
						lightPos++;
					}
				}
#endif
				static float delayHomeKey = 0.3f;
				if (delayHomeKey <= 0.3f)
				{
					delayHomeKey += 0.003f;
				}
				if (delayHomeKey >= 0.3f)
				{
					if (GetAsyncKeyState(VK_HOME) & 0x8001)
					{
						delayHomeKey = 0.f;
						gRenderer->ToggleFullScreenWindow();
					}
				}

				// Animation // 역재생 안됨
				//ka->AnimationDeltaTime(0.002f);
				//cat->AnimationDeltaTime(0.002f);
				//player->AnimationDeltaTime(0.002f);
				//player3->AnimationDeltaTime(0.002f);
				//playerRe->AnimationDeltaTime(0.002f);
				//DebugEnemy->AnimationDeltaTime(0.003f);
				//DebugEnemy->AnimationDeltaTime(0.003f);
				//DebugPlayer->AnimationDeltaTime(1.003f);

				if (GetAsyncKeyState(VK_END) * 0x8000)
				{
					//DebugPlayer->AnimationDeltaTime(0.106f);
				}

				//DebugPlayer0->AnimationDeltaTime(0.003f);
#ifdef MAKE_PARTICLE
				particleSystem->SetDeltaTime(0.003f);
				slashParticleSystem->SetDeltaTime(0.0015f);
				bossParticleSystem0->SetDeltaTime(0.003f);
				bossParticleSystem1->SetDeltaTime(0.003f);
				bossFireProjectileParticleSystem0->SetDeltaTime(0.003f);
				bossFireProjectileParticleSystem1->SetDeltaTime(0.003f);
				sphereImpactParticleSystem->SetDeltaTime(0.003f);
				sphereImpactParticleSystem1->SetDeltaTime(0.003f);
				sphereImpactParticleSystem2->SetDeltaTime(0.003f);
				blackHoleParticleSystem->SetDeltaTime(0.003f);
				blackHoleSphereParticleSystem->SetDeltaTime(0.003f);
				bossBeamRingParticleSystem->SetDeltaTime(0.003f);
				norAttackEffect0->SetDeltaTime(0.003f);
				norAttackEffect1->SetDeltaTime(0.003f);
				norAttackEffect2->SetDeltaTime(0.003f);
				comAttack0->SetDeltaTime(0.003f);
				comAttack1->SetDeltaTime(0.003f);
				dodgeEffect->SetDeltaTime(0.003f);
				magicCircleParticleSystem->SetDeltaTime(0.003f);
				bowAttackParticleSystem->SetDeltaTime(0.003f);
				fireExplosionParticle->SetDeltaTime(0.003f);
				fireExplosionParticle2->SetDeltaTime(0.003f);
				norDamageParticleSystem0->SetDeltaTime(0.003f);
				norDamageParticleSystem0_1->SetDeltaTime(0.003f);
				norDamageParticleSystem1->SetDeltaTime(0.003f);
				norDamageParticleSystem1_1->SetDeltaTime(0.003f);
				norDamageParticleSystem2->SetDeltaTime(0.003f);
				groundEffectParticleSystem->SetDeltaTime(0.003f);
				groundSmokeParticleSystem->SetDeltaTime(0.003f);
				groundFireParticleSystem->SetDeltaTime(0.003f);
				abilitySlashParticleSystem->SetDeltaTime(0.003f);
				enemySlashParticleSystem->SetDeltaTime(0.003f);
				enemyChargeParticleSystem->SetDeltaTime(0.003f);
				enemyChargeParticleSystem2->SetDeltaTime(0.003f);
				ScannerParticleSystem->SetDeltaTime(0.003f);
				portalParticleSystem->SetDeltaTime(0.003f);
				portalParticleSystem1->SetDeltaTime(0.003f);
#endif

				//-----ImGui Test-----//
				gRenderer->ClearImGui();
				if (type == EGraphicsInterfaceType::D3D12_EDITOR || type == EGraphicsInterfaceType::D3D12_RAYTRACING_EDITOR)
				{
					{
						RendererSizeTest();
						CameraWindow(camera);
						//AnimationTest(slashAnim);
						//SkinnedMeshObjectAnimationTest(ka);
						if (dirLight)
						{
							LightTest(dirLight);
						}
						if (pointLight)
						{
							PointLightInspecter(pointLight);
						}
						if (spotLight)
						{
							SpotLightInspector(spotLight);
						}
					}
				}
				// MAIN LOOP
				gRenderer->Render();
			}


			//gRenderer->DeleteMeshObject(mesh);
			//mesh.reset();
			//
			//gRenderer->DeleteMeshObject(cat);
			//cat.reset();

			//gRenderer->DeleteMeshObject(car);
			//car.reset();

			//gRenderer->DeleteMeshObject(boss);
			//boss.reset();

#ifdef MAKE_PARTICLE
			gRenderer->DeleteTexture(slashParticleTexture0);
			slashParticleTexture0.reset();
			gRenderer->DeleteTexture(slashParticleTexture1);
			slashParticleTexture1.reset();
			gRenderer->DeleteTexture(slashParticleTexture2);
			slashParticleTexture2.reset();
			gRenderer->DeleteParticleSystem(slashParticleSystem);


			gRenderer->DeleteTexture(particleTexture);
			particleTexture.reset();
			gRenderer->DeleteParticleSystem(particleSystem);

			for (int i = 0; i < particleTexturesToDelete.size(); i++)
			{
				gRenderer->DeleteTexture(particleTexturesToDelete[i]);
				particleTexturesToDelete[i].reset();
			}

#endif
		}
		//meshes.clear();

		//gRenderer->DeleteMeshObject(cart);
		//cart.reset();
		//gRenderer->DeleteMeshObject(cart2);
		//cart2.reset();


		//gRenderer->DeleteMeshObject(DebugStaticEnemy);
		//DebugStaticEnemy.reset();
		//gRenderer->DeleteMeshObject(DebugEnemy);
		//DebugEnemy.reset();
		//gRenderer->DeleteTexture(faceTexture);
		//faceTexture.reset();
		////gRenderer->DeleteTexture(normalTexture);
		////normalTexture.reset();
		//gRenderer->DeleteTexture(faceNormalTexture);
		//faceNormalTexture.reset();
		//gRenderer->DeleteTexture(eyeTexture);
		//eyeTexture.reset();
		//gRenderer->DeleteTexture(skirtBottomTexture);
		//skirtBottomTexture.reset();
		//gRenderer->DeleteTexture(skirtBottomNormalTexture);
		//skirtBottomNormalTexture.reset();

		gRenderer->DeleteMaterial(planeMaterial);
		planeMaterial.reset();
		gRenderer->DeleteTexture(planeAlbedoTexture);
		planeAlbedoTexture.reset();
		gRenderer->DeleteTexture(planeMaskTexture);
		planeMaskTexture.reset();
		gRenderer->DeleteTexture(planeNormalTexture);
		planeNormalTexture.reset();
		gRenderer->DeleteTexture(skybox);
		skybox.reset();
		gRenderer.reset();
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IDEALGRAPHICSDEMO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	RECT windowRect = { 0,0, WIDTH, HEIGHT };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);
	g_hWnd = hWnd;

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (gRenderer)
	{
		gRenderer->SetImGuiWin32WndProcHandler(hWnd, message, wParam, lParam);
	}
	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 메뉴 선택을 구문 분석합니다:
			switch (wmId)
			{
				break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_SIZE:
		{
			if (gRenderer)
			{
				RECT rect;
				GetClientRect(g_hWnd, &rect);
				//AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
				DWORD width = rect.right - rect.left;
				DWORD height = rect.bottom - rect.top;
				gRenderer->Resize(width, height);
			}
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void InitCamera(std::shared_ptr<Ideal::ICamera> Camera)
{
	float aspectRatio = float(WIDTH) / HEIGHT;
	//float aspectRatio = float(1296) / 999.f;
	//Camera->SetLens(0.25f * 3.141592f, aspectRatio, 1.f, 3000.f);
	Camera->SetLens(0.25f * 3.141592f, aspectRatio, 0.1f, 3000.f);
	//Camera->SetLens(0.25f * 3.141592f, aspectRatio, 0.01f, 3000.f);
	//Camera->SetLens(0.25f * 3.141592f, aspectRatio, 0.001f, 3000.f);
	//Camera->SetLens(0.25f * 3.141592f, aspectRatio, 1.f, 3000.f);
	//Camera->SetLensWithoutAspect(0.7f * 3.141592f, 1.f, 3000.f);
	Camera->SetPosition(Vector3(3.f, 3.f, -10.f));
}

void CameraTick(std::shared_ptr<Ideal::ICamera> Camera, std::shared_ptr<Ideal::ISpotLight> SpotLight /*= nullptr*/, std::shared_ptr<Ideal::IPointLight> PointLight /*= nullptr*/)
{
	if (!g_CameraMove)
		return;
	float speed = g_cameraSpeed;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		speed *= 0.1f;
	}
	if (GetAsyncKeyState('W') & 0x8000)
	{
		Camera->Walk(speed);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		Camera->Walk(-speed);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		Camera->Strafe(-speed);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		Camera->Strafe(speed);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		auto p = Camera->GetPosition();
		p.y += speed;
		Camera->SetPosition(p);
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		auto p = Camera->GetPosition();
		p.y -= speed;
		Camera->SetPosition(p);
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		Camera->RotateY(-speed * 0.2);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		Camera->RotateY(speed * 0.2);
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		Camera->Pitch(-speed * 0.2);
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		Camera->Pitch(speed * 0.2);
	}

	if (SpotLight)
	{
		SpotLight->SetPosition(Camera->GetPosition());
		SpotLight->SetDirection(Camera->GetLook());
	}

	if (PointLight)
	{
		PointLight->SetPosition(Camera->GetPosition());
	}
}

void CameraWindow(std::shared_ptr<Ideal::ICamera> Camera)
{
	if (show_angle_window)
	{
		ImGui::Begin("Camera Window");
		ImGui::DragFloat("Camera Speed", &g_cameraSpeed, 0.01f, 0.0f, 1.f);
		ImGui::Checkbox("Move", &g_CameraMove);
		ImGui::End();
	}
}

void DirLightAngle(float* x, float* y, float* z)
{
	if (show_angle_window)
	{
		ImGui::Begin("Directional Light Angle Window");
		ImGui::SliderFloat("X", x, 0.0f, 360.0f);
		ImGui::SliderFloat("Y", y, 0.0f, 360.0f);
		ImGui::SliderFloat("Z", z, 0.0f, 360.0f);
		ImGui::End();
	}
}

void PointLightInspecter(std::shared_ptr<Ideal::IPointLight> light)
{
	ImVec4 lightColor;
	{
		lightColor.x = light->GetLightColor().R();
		lightColor.y = light->GetLightColor().G();
		lightColor.z = light->GetLightColor().B();
		lightColor.w = light->GetLightColor().A();
	}

	Vector3 lightPosition = light->GetPosition();
	float range = light->GetRange();
	float intensity = light->GetIntensity();
	bool noShadowCast = light->GetIsShadowCasting();
	if (show_point_light_window)
	{
		ImGui::Begin("Point Light Inspector");
		ImGui::ColorEdit3("Light Color", (float*)&lightColor);
		light->SetLightColor(Color(lightColor.x, lightColor.y, lightColor.z, lightColor.w));
		//ImGui::InputFloat3("Position", &lightPosition.x);
		ImGui::DragFloat3("Position", &lightPosition.x, 1.f, 0.f, 10.f);
		light->SetPosition(lightPosition);

		ImGui::DragFloat("Range", &range, 1.f, 0.f, 1000.f);
		light->SetRange(range);

		ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.f, 100.f);
		light->SetIntensity(intensity);

		ImGui::Checkbox("ShadowCast", &noShadowCast);
		light->SetShadowCasting(noShadowCast);

		ImGui::End();
	}
}


void SpotLightInspector(std::shared_ptr<Ideal::ISpotLight> SpotLight)
{

	ImVec4 lightColor;
	{
		lightColor.x = SpotLight->GetLightColor().R();
		lightColor.y = SpotLight->GetLightColor().G();
		lightColor.z = SpotLight->GetLightColor().B();
		lightColor.w = SpotLight->GetLightColor().A();
	}

	ImVec4 lightPos;
	{
		lightPos.x = SpotLight->GetPosition().x;
		lightPos.y = SpotLight->GetPosition().y;
		lightPos.z = SpotLight->GetPosition().z;
	}

	float dir[3] = {
		SpotLight->GetDirection().x,
		SpotLight->GetDirection().y,
		SpotLight->GetDirection().z
	};

	float range = SpotLight->GetRange();
	float intensity = SpotLight->GetIntensity();
	float angle = SpotLight->GetSpotAngle();

	float softness = SpotLight->GetSoftness();

	ImGui::Begin("Spot Light Inspector");

	ImGui::InputFloat3("Light Position", &lightPos.x);
	ImGui::InputFloat3("Light Direction", &dir[0]);

	ImGui::ColorEdit3("Light Color", (float*)&lightColor);
	SpotLight->SetLightColor(Color(lightColor.x, lightColor.y, lightColor.z, lightColor.w));
	//ImGui::InputFloat3("Position", &lightPosition.x);
	ImGui::DragFloat("Angle", &angle, 1.f, 0.f, 10.f);
	SpotLight->SetSpotAngle(angle);

	ImGui::DragFloat("Range", &range, 1.f, 0.f, 1000.f);
	SpotLight->SetRange(range);

	ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.f, 100.f);
	SpotLight->SetIntensity(intensity);

	ImGui::DragFloat("Softness", &softness, 0.1f, 0.f, 30.f);
	SpotLight->SetSoftness(softness);

	ImGui::End();
}

void ImGuiTest()
{
	{
		ImGuiIO& io = ImGui::GetIO();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			//ImGui::Image()

			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}
	}
}

void SkinnedMeshObjectAnimationTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject)
{
	ImGui::Begin("Skinned Mesh Object Animation Window");

	static float animationSpeed = 1.f; // 임시 static // 실제 사용 조심
	ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.0f, 3.0f);
	SkinnedMeshObject->SetAnimationSpeed(animationSpeed);

	ImGui::Text("Current Animation Index : %d", SkinnedMeshObject->GetCurrentAnimationIndex());

	ImGui::Text("Change Next Animation");
	if (ImGui::Button("Run"))
	{
		SkinnedMeshObject->SetAnimation("Run", false);
	}
	if (ImGui::Button("Slash"))
	{
		SkinnedMeshObject->SetAnimation("Slash", false);
	}

	ImGui::Text("Reserve Next Animation");
	if (ImGui::Button("Run2"))
	{
		SkinnedMeshObject->SetAnimation("Run", true);
	}
	if (ImGui::Button("Slash2"))
	{
		SkinnedMeshObject->SetAnimation("Slash", true);
	}

	ImGui::End();
}

void AnimationTest(std::shared_ptr<Ideal::IAnimation> Animation)
{
	ImGui::Begin("Animation Window");
	ImGui::Text("Animation Max Frame Count : %d", Animation->GetFrameCount());
	ImGui::End();
}

void LightTest(std::shared_ptr<Ideal::IDirectionalLight> DirLight)
{
	ImGui::Begin("Directional Light");
	ImGui::Text("Rotation Axis X");
	ImGui::SliderFloat("X", &lightAngleX, 0.f, 6.28f);
	Matrix mat = Matrix::Identity;
	mat *= Matrix::CreateRotationX(lightAngleX);
	ImGui::ColorEdit3("Diffuse Color", lightColor);
	ImGui::SliderFloat("Y", &lightAngleY, 0.f, 6.28f);
	mat *= Matrix::CreateRotationY(lightAngleY);

	ImGui::SliderFloat("Intensity", &lightIntensity, 0.f, 10.f);

	Vector3 rot = mat.Forward();
	if (DirLight)
	{
		DirLight->SetDirection(rot);
		DirLight->SetDiffuseColor(Color(lightColor[0], lightColor[1], lightColor[2], 1.f));
		DirLight->SetIntensity(lightIntensity);
	}

	ImGui::End();
}

// ImGui에 이미지를 띄워보는 용도의 함수
void ImageTest(std::shared_ptr<Ideal::ITexture> Texture)
{
	ImGui::Begin("Image Test");
	const ImVec2 size(100, 100);
	ImGui::Image((ImTextureID)Texture->GetImageID(), size);
	ImGui::End();
}

// 기능 본 정보 추출
void SkinnedMeshObjectBoneInfoTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject)
{
	ImGui::Begin("SkinnedMesh Bone Test");
	auto boneSize = SkinnedMeshObject->GetBonesSize();
	ImGui::Separator();
	// 본 정보
	ImGui::Text("Bone Info");
	for (int i = 0; i < boneSize; ++i)
	{
		auto bone = SkinnedMeshObject->GetBoneByIndex(i);
		ImGui::Text(bone.lock()->GetName().c_str());
		auto matrix = bone.lock()->GetTransform();
	}
	ImGui::End();
}
// 기능 정리 :
// 모델에서 매쉬 오브젝트 추출 -> 매쉬 이거 가지고 있지 말 것. 지금 shared_ptr로 되어 있음
// 머테리얼 적용 및 가지고 있는 머테리얼 가져오기
void SkinnedMeshObjectGetMeshTest(std::shared_ptr<Ideal::ISkinnedMeshObject> SkinnedMeshObject, std::shared_ptr<Ideal::IMaterial> Material, std::shared_ptr<Ideal::IMaterial> Material2 /*= nullptr*/, std::shared_ptr<Ideal::ITexture> Texture /*= nullptr*/, std::shared_ptr<Ideal::ITexture> Texture2 /*= nullptr*/)
{
	ImGui::Begin("SkinnedMesh Get Mesh Test");
	auto meshSize = SkinnedMeshObject->GetMeshesSize();
	ImGui::Separator();

	//ImVec2 s(100, 100);
	//ImGui::Image(ImTextureID(SkinnedMeshObject->GetMeshByIndex(5).lock()->GetMaterialObject().lock()->GetBaseMap().lock()->GetImageID()), s);


	// 매쉬 정보
	ImGui::Text("Mesh Info");
	for (int i = 0; i < meshSize; ++i)
	{
		auto mesh = SkinnedMeshObject->GetMeshByIndex(i);
		ImGui::Text(mesh.lock()->GetName().c_str());
	}
	if (Texture)
	{
		static int once = 0;
		once++;
		if (once >= 3000)
		{
			once = 1;
			// 예시로 GetMaterialObject를 사용함. -> 이는 기본 머테리얼을 가져와서 사용하므로 가급적 사용X
			// 차라리 Material을 새로 만들어서 SetMaterialObject를 해주셈
			// 이건 그냥 SetBaseMap의 예시
			//SkinnedMeshObject->GetMeshByIndex(5).lock()->GetMaterialObject().lock()->SetBaseMap(Texture);
			//SkinnedMeshObject->GetMeshByIndex(5).lock()->GetMaterialObject().lock()->SetNormalMap(Texture2);
		}
	}

	// 아래는 머테리얼을 바꾼다
	if (Material)
	{
		static int once = 0;
		once++;
		if (once >= 1000)
		{
			once = 1;
			//SkinnedMeshObject->GetMeshByIndex(0).lock()->SetMaterialObject(Material);
		}
	}
	if (Material2)
	{
		static int once = 0;
		once++;
		if (once >= 1000)
		{
			once = 1;
			//SkinnedMeshObject->GetMeshByIndex(4).lock()->SetMaterialObject(Material2);
		}
	}
	ImGui::End();
}

void SpriteTest(std::shared_ptr<Ideal::ISprite> Sprite)
{
	ImGui::Begin("Sprite Test");
	float a = Sprite->GetAlpha();
	ImGui::SliderFloat("Alpha", &a, 0.f, 1.f);
	Sprite->SetAlpha(a);

	//float c[4];
	//c[0] = Sprite->GetColor().R();
	//c[1] = Sprite->GetColor().G();
	//c[2] = Sprite->GetColor().B();
	//c[3] = 1;
	//ImGui::ColorEdit3("Sprite Color", c);
	//Sprite->SetColor(Color(c[0], c[1], c[2], c[3]));

	bool b = Sprite->GetActive();
	ImGui::Checkbox("Active", &b);
	Sprite->SetActive(b);

	ImGui::End();
}


bool InputTextW(const char* label, std::wstring& wstr) {
	// wstring을 utf-8 string으로 변환
	std::string utf8Str = wstring_to_utf8Func(wstr);

	// 버퍼 크기를 확인
	char buffer[256]; // 임시로 256바이트 버퍼 사용
	strncpy_s(buffer, utf8Str.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = 0; // null-terminate

	// ImGui InputText 호출
	bool result = ImGui::InputText(label, buffer, sizeof(buffer));

	// 입력이 변경되었으면 utf-8 -> wstring 변환
	if (result) {
		wstr = utf8_to_wstringFunc(std::string(buffer));
	}

	return result;
}


bool InputTextMultilineW(const char* label, std::wstring& wstr, const ImVec2& size = ImVec2(0, 0)) {
	// wstring을 utf-8 string으로 변환
	std::string utf8Str = wstring_to_utf8Func(wstr);

	// 버퍼 크기를 확인하고 적절한 메모리를 할당
	std::vector<char> buffer(utf8Str.begin(), utf8Str.end());
	buffer.resize(1024); // 임의로 1024바이트로 버퍼 크기 설정

	// ImGui InputTextMultiline 호출
	bool result = ImGui::InputTextMultiline(label, buffer.data(), buffer.size(), size);

	// 입력이 변경되었으면 utf-8 -> wstring 변환
	if (result) {
		wstr = utf8_to_wstringFunc(std::string(buffer.data()));
	}

	return result;
}



std::wstring finalStr;
void TextTest(std::shared_ptr<Ideal::IText> Text)
{
	ImGui::Begin("Text test");
	float a = Text->GetAlpha();
	ImGui::SliderFloat("Alpha", &a, 0.f, 1.f);
	Text->SetAlpha(a);
	bool b = Text->GetActive();
	ImGui::Checkbox("Active", &b);
	Text->SetActive(b);
	//InputTextW("Label", finalStr);
	InputTextMultilineW("Label", finalStr);	// <- 이거 기능 진짜 개쩌는듯 ㅋㅋ 엔터치면 다음줄로감 
	Text->ChangeText(finalStr);	// 변경이 안됐는데 매 프레임 호출하지 말 것. 이전 string과 비교해서 다를경우에만 넣어줄 것
	ImGui::End();
}

void RendererSizeTest()
{
	ImGui::Begin("Window Size Test", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);
	bool b = gRenderer->IsFullScreen();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("FullScreenMode").x - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
	ImGui::Checkbox("FullScreenMode", &b);
	if (b != gRenderer->IsFullScreen())
	{
		gRenderer->ToggleFullScreenWindow();
	}

	static int val = 0;
	int beforeVal = val;
	ImGui::Combo("Display Resolution", &val, items, IM_ARRAYSIZE(items));
	if (val != beforeVal)
	{
		gRenderer->SetDisplayResolutionOption((Ideal::Resolution::EDisplayResolutionOption)val);
	}
	ImGui::End();
}

void ParticleSystemTransform(std::shared_ptr<Ideal::IParticleSystem> ParticleSystem)
{
	ImGui::Begin("ParticleSystem");
	auto pos = ParticleSystem->GetTransformMatrix().Translation();
	float x, y, z;
	x = pos.x;
	y = pos.y;
	z = pos.z;
	ImGui::SliderFloat("X", &x, -10, 10);
	ImGui::SliderFloat("Y", &y, -10, 10);
	ImGui::SliderFloat("Z", &z, -10, 10);

	ParticleSystem->SetTransformMatrix(Matrix::CreateTranslation(Vector3(x, y, z)));

	ImGui::End();
}
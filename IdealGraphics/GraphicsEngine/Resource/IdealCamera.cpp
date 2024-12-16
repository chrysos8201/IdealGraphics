#include "GraphicsEngine/Resource/IdealCamera.h"
//#include "GraphicsEngine/D3D12/D3D12ThirdParty.h"

using namespace Ideal;

using namespace DirectX;
using namespace DirectX::SimpleMath;

IdealCamera::IdealCamera()
	: m_position(),
	m_right(1.f, 0.f, 0.f),
	m_up(0.f, 1.f, 0.f),
	m_look(0.f, 0.f, 1.f),
	m_nearZ(0.f),
	m_farZ(0.f),
	m_aspect(0.f),
	m_fovY(0.f),
	m_nearWindowHeight(0.f),
	m_farWindowHeight(0.f),
	m_viewDirty(true),
	m_view(Matrix::Identity),
	m_proj(Matrix::Identity)
{

}

void IdealCamera::SetLensWithoutAspect(float FovY, float NearZ, float FarZ)
{
	m_fovY = FovY;
	m_nearZ = NearZ;
	m_farZ = FarZ;

	m_nearWindowHeight = 2.f * m_nearZ * std::tanf(0.5f * m_fovY);
	m_farWindowHeight = 2.f * m_farZ * std::tanf(0.5f * m_fovY);

	m_proj = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}

void Ideal::IdealCamera::Walk(float d)
{
	Vector3 s = Vector3(d);
	Vector3 l = m_look;
	Vector3 p = m_position;
	m_position = s * l + p;
}

void Ideal::IdealCamera::Strafe(float d)
{
	Vector3 s = Vector3(d);
	Vector3 r = m_right;
	Vector3 p = m_position;
	m_position = s * r + p;
}

void Ideal::IdealCamera::Pitch(float Angle)
{
	// Right º¤ÅÍ´Â ¹Ù²îÁö ¾ÊÀ½.

	//Matrix R = Matrix::CreateRotationX(Angle);
	Matrix R = Matrix::CreateFromAxisAngle(m_right, Angle);
	m_look = m_look.TransformNormal(m_look, R);
	m_up = m_up.TransformNormal(m_up, R);
	//UpdateViewMatrix();
}

void Ideal::IdealCamera::RotateY(float Angle)
{
	Matrix R = Matrix::CreateRotationY(Angle);
	m_right = m_right.TransformNormal(m_right, R);
	m_up = m_up.TransformNormal(m_up, R);
	m_look = m_look.TransformNormal(m_look, R);
	//SetLook(m_look);
	//UpdateViewMatrix();
}

void IdealCamera::SetPosition(const Vector3& Position)
{
	m_position = Position;
}

void Ideal::IdealCamera::SetLook(Vector3 Look)
{
	Look.Normalize();
	m_look = Look;
	return;


	Look.Normalize();

	Vector3 worldUp(0.f, 1.f, 0.f);
	Vector3 right = -Look.Cross(worldUp);
	//Vector3 right = worldUp.Cross(Look);
	right.Normalize();

	Vector3 up = Look.Cross(right);

	m_look = Look;
	m_up = up;
	m_right = right;
	//UpdateViewMatrix();
}

void IdealCamera::SetAspectRatio(float AspectRatio)
{
	m_aspect = AspectRatio;
	SetLens(m_fovY, AspectRatio, m_nearZ, m_farZ);
}

void IdealCamera::UpdateMatrix2()
{
	Vector3 eyePosition = m_position;
	Vector3 focusPosition = eyePosition + m_look;
	Vector3 right = Vector3::Up.Cross(m_look);
	Vector3 upDir = m_look.Cross(right);

	m_right = right;
	m_up = upDir;

	m_view = XMMatrixLookAtLH(eyePosition, focusPosition, upDir);
	m_proj = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
	//Vector3 upDir = 
}

IdealCamera::~IdealCamera()
{

}

void IdealCamera::SetLens(float FovY, float Aspect, float NearZ, float FarZ)
{
	m_fovY = FovY;
	m_aspect = Aspect;
	m_nearZ = NearZ;
	m_farZ = FarZ;

	m_nearWindowHeight = 2.f * m_nearZ * std::tanf(0.5f * m_fovY);
	m_farWindowHeight = 2.f * m_farZ * std::tanf(0.5f * m_fovY);

	m_proj = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
	//m_proj = Matrix::CreatePerspectiveFieldOfView(m_fovY, m_aspect, m_nearZ, m_farZ);
}

void IdealCamera::UpdateViewMatrix()
{
	/*m_view = CreateViewMatrix(m_position, m_look, Vector3::Up);
	return;*/
	Vector3 Right = m_right;
	Vector3 Up = m_up;
	Vector3 Look = m_look;
	Vector3 Position = m_position;

	Look.Normalize();
	Up = Look.Cross(Right);
	Up.Normalize();

	Right = Up.Cross(Look);

	float x = -XMVectorGetX(XMVector3Dot(Position, Right));
	float y = -XMVectorGetX(XMVector3Dot(Position, Up));
	float z = -XMVectorGetX(XMVector3Dot(Position, Look));

	m_right = Right;
	m_up = Up;
	m_look = Look;

	Matrix view;
	view(0, 0) = m_right.x;
	view(1, 0) = m_right.y;
	view(2, 0) = m_right.z;
	view(3, 0) = x;

	view(0, 1) = m_up.x;
	view(1, 1) = m_up.y;
	view(2, 1) = m_up.z;
	view(3, 1) = y;

	view(0, 2) = m_look.x;
	view(1, 2) = m_look.y;
	view(2, 2) = m_look.z;
	view(3, 2) = z;

	view(0, 3) = 0.f;
	view(1, 3) = 0.f;
	view(2, 3) = 0.f;
	view(3, 3) = 1.f;

	m_view = view;
}

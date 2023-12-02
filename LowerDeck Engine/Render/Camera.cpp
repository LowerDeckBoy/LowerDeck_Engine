#include "Camera.hpp"
#include"../../LowerDeck Editor/Dependencies/ImGui/imgui.h"


void Camera::Initialize(float AspectRatio)
{
	// Defaulting positions on startup
	m_Position	= m_DefaultPosition;
	m_Target	= m_DefaultTarget;
	m_Up		= m_DefaultUp;

	m_AspectRatio	= AspectRatio;
	XMStoreFloat4x4(&m_View, XMMatrixLookAtLH(GetPosition(), GetTarget(), GetUp()));
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(m_FieldOfView, m_AspectRatio, m_zNear, m_zFar));

	CameraSlider = { m_Position.x, m_Position.y, m_Position.z };
}

void Camera::Update()
{
	// Load vectors and matrices
	XMVECTOR forward{ XMLoadFloat3(&m_Forward) };
	XMVECTOR right{ XMLoadFloat3(&m_Right) };
	XMVECTOR up{ XMLoadFloat3(&m_Up) };
	XMMATRIX rotationMatrix{ XMLoadFloat4x4(&m_RotationMatrix) };
	XMVECTOR target{ XMLoadFloat3(&m_Target) };
	XMVECTOR position{ XMLoadFloat3(&m_Position) };

	rotationMatrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
	target = XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotationMatrix));

	const XMMATRIX rotation{ XMMatrixRotationY(m_Yaw) };

	forward = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotation);
	right = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultRight), rotation);
	up = XMVector3TransformCoord(XMLoadFloat3(&m_Up), rotation);

	position += (MoveForwardBack * forward);
	position += (MoveRightLeft * right);
	position += (MoveUpDown * up);

	MoveForwardBack = 0.0f;
	MoveRightLeft = 0.0f;
	MoveUpDown = 0.0f;

	target += position;

	XMStoreFloat4x4(&m_View, XMMatrixLookAtLH(position, target, up));
	CameraSlider = { XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position) };
	
	// Store vector and matrices
	XMStoreFloat3(&m_Forward, forward);
	XMStoreFloat3(&m_Right, right);
	XMStoreFloat3(&m_Up, up);
	XMStoreFloat4x4(&m_RotationMatrix, rotationMatrix);
	XMStoreFloat3(&m_Target, target);
	XMStoreFloat3(&m_Position, position);
}

void Camera::SetPosition(const std::array<float, 3> NewPosition) noexcept
{
	m_Position = XMFLOAT3(NewPosition.at(0), NewPosition.at(1), NewPosition.at(2));
}

inline void Camera::ResetFieldOfView() noexcept
{
	m_FieldOfView = 45.0f;
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_AspectRatio, m_zNear, m_zFar));
}

void Camera::ResetCamera() noexcept
{
	ResetPitch();
	ResetYaw();
	ResetFieldOfView();
	ResetPosition();
}

void Camera::DrawGUI()
{
	if (ImGui::DragFloat3("Position", CameraSlider.data()))
	{
		SetPosition(CameraSlider);
		Update();
	}

	ImGui::SliderFloat("Speed", &CameraSpeed, 1.0f, 500.0f, "%.2f");
	if (ImGui::SliderFloat("FoV", &m_FieldOfView, 1.0f, 160.0f, "%.2f"))
		XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_AspectRatio, m_zNear, m_zFar));

	if (ImGui::Button("Reset"))
	{
		ResetCamera();
		Update();
	}
}

void Camera::OnAspectRatioChange(float NewAspectRatio) noexcept
{
	m_AspectRatio = NewAspectRatio;
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), NewAspectRatio, m_zNear, m_zFar));
}

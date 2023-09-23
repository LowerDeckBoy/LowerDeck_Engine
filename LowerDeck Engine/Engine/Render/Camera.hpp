#pragma once
#include <array>
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	void Initialize(float AspectRatio);

	void Update();

	//void SetPosition(const DirectX::XMVECTOR NewPosition) noexcept { }
	void SetPosition(const std::array<float, 3> NewPosition) noexcept;

	void ResetPosition() { m_Position = m_DefaultPosition; }

	void ResetPitch() noexcept		 { m_Pitch = 0.0f; }
	void ResetYaw() noexcept		 { m_Yaw = 0.0f; }
	void ResetFieldOfView() noexcept { m_FieldOfView = XMConvertToRadians(45.0f); }

	void ResetCamera() noexcept;

	void DrawGUI();

	const XMMATRIX GetView() const noexcept			{ return XMLoadFloat4x4(&m_View); }
	const XMMATRIX GetProjection() const noexcept	{ return XMLoadFloat4x4(&m_Projection);}
	const XMMATRIX GetViewProjection() noexcept		{ return XMLoadFloat4x4(&m_ViewProjection);}

	const XMVECTOR GetPosition() const noexcept { return XMLoadFloat3(&m_Position); }
	const XMVECTOR GetTarget() const noexcept	{ return XMLoadFloat3(&m_Target); }
	const XMVECTOR GetUp() const noexcept		{ return XMLoadFloat3(&m_Up); }

	/// Required to call when window is resizing as Render Targets change their aspect ratio
	void OnAspectRatioChange(float NewAspectRatio) noexcept;

	//float GetCameraSpeed() const noexcept { return m_Camera;
	//void SetCameraSpeed(float NewSpeed) noexcept;

	void  SetZNear(float NewZ) noexcept { m_zNear = NewZ; }
	void  SetZFar(float NewZ) noexcept	{ m_zFar = NewZ; }
	float GetZNear() const noexcept		{ return m_zNear; }
	float GetZFar() const noexcept		{ return m_zFar; }

private:
	XMFLOAT4X4 m_View				{ XMFLOAT4X4() };
	XMFLOAT4X4 m_Projection			{ XMFLOAT4X4() };
	XMFLOAT4X4 m_ViewProjection		{ XMFLOAT4X4() };

	XMFLOAT3 m_Position				{ XMFLOAT3(0.0f, 1.0f, -10.0f) };
	XMFLOAT3 m_Target				{ XMFLOAT3(0.0f, 5.0f, 0.0f) };
	XMFLOAT3 m_Up					{ XMFLOAT3(0.0f, 1.0f, 0.0f) };

	XMFLOAT4X4 m_RotationX			{ XMFLOAT4X4() };
	XMFLOAT4X4 m_RotationY			{ XMFLOAT4X4() };
	XMFLOAT4X4 m_RotationMatrix		{ XMFLOAT4X4() };

	XMFLOAT3 m_Forward				{ XMFLOAT3(0.0f, 0.0f, 1.0f) };
	XMFLOAT3 m_Right				{ XMFLOAT3(1.0f, 0.0f, 0.0f) };
	XMFLOAT3 m_Upward				{ XMFLOAT3(0.0f, 1.0f, 0.0f) };

	XMFLOAT3 const m_DefaultPosition{ XMFLOAT3(0.0f, 1.0f, -10.0f) };
	XMFLOAT3 const m_DefaultTarget	{ XMFLOAT3(0.0f, 5.0f, 0.0f) };
	XMFLOAT3 const m_DefaultUp		{ XMFLOAT3(0.0f, 1.0f, 0.0f) };

	XMFLOAT3 const m_DefaultForward	{ XMFLOAT3(0.0f, 0.0f, 1.0f) };
	XMFLOAT3 const m_DefaultRight	{ XMFLOAT3(1.0f, 0.0f, 0.0f) };
	XMFLOAT3 const m_DefaultUpward	{ XMFLOAT3(0.0f, 1.0f, 0.0f) };

	float m_zNear{ 0.1f };
	float m_zFar { 500'000.0f };

	float m_AspectRatio{ 1.0f };
	float m_FieldOfView{ 45.0f };

public:
	// For calling camera movement from keyboard inputs
	float MoveForwardBack{ 0.0f };
	float MoveRightLeft	 { 0.0f };
	float MoveUpDown	 { 0.0f };

	float m_Pitch{ 0.0f };
	float m_Yaw	 { 0.0f };

	// For GUI usage
	std::array<float, 3> CameraSlider{ };

	inline static float CameraSpeed{ 25.0f };

};

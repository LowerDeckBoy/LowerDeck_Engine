#pragma once
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <memory>

class Camera;

/// <summary>
/// dinput.h based controls for Camera
/// </summary>
class CameraInput
{
public:
	void Initialize();
	//void ProcessInputs(std::shared_ptr<Camera> pCamera, float DeltaTime);
	void ProcessInputs(Camera* pCamera, float DeltaTime);
	void Release() noexcept;

private:
	inline static IDirectInputDevice8* DxKeyboard{};
	inline static IDirectInputDevice8* DxMouse{};
	inline static LPDIRECTINPUT8 DxInput{};
	inline static DIMOUSESTATE DxLastMouseState{};

};

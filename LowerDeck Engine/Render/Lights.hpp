#pragma once
#include <DirectXMath.h>
using namespace DirectX;


struct DirectionalLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT4 DiffuseColor;
	XMFLOAT3 Direction;
	float Intensity;
};

struct PointLight
{
	XMFLOAT4 AmbientColor;
	XMFLOAT3 Position;
	float Radius;
};

struct SpotLight
{

};

struct AreaLight
{

};

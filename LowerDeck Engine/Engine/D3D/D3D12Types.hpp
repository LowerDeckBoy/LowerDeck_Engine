#pragma once

// TODO:

/// <summary>
/// Used to determine desired shader version
/// </summary>
enum class ShaderModel : uint8_t
{
	e6_0 = 0x00,
	e6_1,
	e6_2,
	e6_3,
	e6_4,
	e6_5,
	e6_6,
	e5_1 = 0xA,
};

/// <summary>
/// Used to determine desired shader type
/// </summary>
enum class ShaderType : uint8_t
{
	eVertex = 0x00,
	ePixel,
	eCompute,
	eLibrary,
	eRayTracing,
	eGeometry,
	eHull,
	eDomain
};

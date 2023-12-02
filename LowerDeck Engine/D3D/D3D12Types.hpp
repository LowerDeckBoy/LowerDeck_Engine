#pragma once
#include <cstdint>
// Mostly helper enum types to use across D3D and Shader content.

// TODO:
// Add namespace - RHI perhaps

/// <summary>
/// Used to determine desired target shader version.
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
	e5_0
};

/// <summary>
/// Used to determine desired shader type.
/// </summary>
enum class ShaderType : uint8_t
{
	eVertex = 0x00,
	ePixel,
	eCompute,
	eLibrary,
	eRayTracing,
	eAmplification,
	eMesh,
	eGeometry,
	eHull,
	eDomain
};

enum class BufferType : uint8_t
{
	eVertex		= 0x00,
	eIndex		= 0x01,
	eConstant	= 0x02,
	eStructured = 0x04
};

/// <summary>
/// Applies to both PipelineState and RootSignature
/// </summary>
enum class PipelineType : uint8_t
{
	eGraphics = 0x00,
	eCompute
};

/// <summary>
/// Geometry topology.
/// </summary>
enum class TopologyType : uint8_t
{
	eTriangle = 0x00,
	ePoint,
	eLine,
	eUndefined
};

/// <summary>
/// For Textures and Samplers.
/// </summary>
enum class AddressMode : uint8_t
{
	eWrap = 0x00,
	eClamp,
	eBorder,
	eMirror,
	eMirrorOnce
};

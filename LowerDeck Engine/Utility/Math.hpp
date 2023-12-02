#pragma once
#include <cmath>

namespace Math
{
	
	inline constexpr uint32_t Align(uint32_t Value, uint32_t Alignment)
	{
		return static_cast<uint32_t>((Value + (Alignment - 1)) & ~(Alignment - 1));
	}

	inline constexpr uint64_t Align(uint64_t Value, uint64_t Alignment)
	{
		return static_cast<uint64_t>((Value + (Alignment - 1)) & ~(Alignment - 1));
	}

	/// <summary>
	/// Check whether given value is a power of two.
	/// </summary>
	/// <param name="Value"> Value to check. </param>
	/// <returns> <c>True</c> if value is a power of two. </returns>
	inline constexpr bool IsPowerOfTwo(uint32_t Value)
	{
		return (Value & (Value - 1)) == 0;
	}

	/// <summary>
	/// Fetch next power of 2 for given value.
	/// </summary>
	/// <param name="Value"> Base value. </param>
	/// <returns> Next power of 2. </returns>
	uint32_t PowerOfTwo(uint32_t Value);
}


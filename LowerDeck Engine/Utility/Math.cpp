#include "Math.hpp"

namespace Math
{
	uint32_t PowerOfTwo(uint32_t Value)
	{
		--Value;
		Value |= Value >> 1;
		Value |= Value >> 2;
		Value |= Value >> 4;
		Value |= Value >> 8;
		Value |= Value >> 16;
		++Value;

		return Value;
	}

}

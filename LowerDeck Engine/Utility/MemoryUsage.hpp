#pragma once
#include <Psapi.h>

namespace utility
{
	/// <summary>
	/// Note: RAM usage output is NOT actually accurate.<br/>
	/// Given output is higher, than actual usage.<br/>
	/// Problem lays in VS itself.<br/>
	/// TODO: to investigate...
	/// </summary>
	class MemoryUsage
	{
	public:
		static float ReadRAM()
		{
			MEMORYSTATUSEX mem{};
			mem.dwLength = sizeof(MEMORYSTATUSEX);
			// for convertion to MB and GB
			//constexpr DWORD dwMBFactor = 0x00100000;

			PROCESS_MEMORY_COUNTERS_EX pcmex{};

			if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pcmex, sizeof(PROCESS_MEMORY_COUNTERS_EX)))
				return -1.0f;

			if (!::GlobalMemoryStatusEx(&mem))
				return -1.0f;

			return static_cast<float>(pcmex.WorkingSetSize / (1024.0f * 1024.0f));
		}
	};
}

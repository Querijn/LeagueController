#pragma once

#include <spek/util/types.hpp>

namespace MemoryHelper
{

#if _WIN32 || _WIN64
	#if _WIN64
		#define ENV_64 1
		using Address = u64;
	#else
		#define ENV_32 1
		using Address = u32;
	#endif
#endif

#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define ENV_64 1
		using Address = u64;
	#else
		#define ENV_32 1
		using Address = u32;
	#endif
#endif

}

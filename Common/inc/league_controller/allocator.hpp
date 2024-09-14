#pragma once

#if LEACON_SUBMISSION
#define LEACON_PROFILE_ALLOC(p, size)	do { } while(0)
#define LEACON_PROFILE_FREE(p)			do { } while(0)
#else
#include "tracy/Tracy.hpp"

#define LEACON_PROFILE_ALLOC(p, size)	do { TracyCAllocS(p, size, 12) } while(0)
#define LEACON_PROFILE_FREE(p)			do { TracyCFreeS(p, 12) } while(0)
#endif

namespace LeagueController
{
	template<typename T>
	class ProfilerAllocator : public std::allocator<T>
	{
		T* allocate(std::size_t n)
		{
			T* ptr = std::allocator<T>::allocate(n);
			LEACON_PROFILE_ALLOC(ptr, n);
			return ptr;
		}
		
		void deallocate(T* p, std::size_t n)
		{
			LEACON_PROFILE_FREE(p);
			std::allocator<T>::deallocate(p, n);
		}
	};
}
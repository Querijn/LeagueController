#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <vector>
#include <string>
#include <memory>

#include <skerm/math_base_token.hpp>

namespace Skerm
{
	namespace ASDF
	{
		// TODO: Correct aligning
	#pragma pack(push, 1)
		template<typename PointerType, typename SizeType = uint16_t, int OffsetAmount = sizeof(SizeType)>
		class RelPtr
		{
		public:
			operator PointerType*()
			{
				if (m_offset == 0) return nullptr;
				return (PointerType*)((uint8_t*)&m_offset + m_offset);
			}

			operator const PointerType*() const
			{
				if (m_offset == 0) return nullptr;
				return (const PointerType*)((uint8_t*)&m_offset + m_offset);
			}

			PointerType* operator->()
			{
				assert(m_offset != 0); // nullptr
				return (PointerType*)((uint8_t*)&m_offset + m_offset);
			}

			const PointerType* operator->() const
			{
				assert(m_offset != 0); // nullptr
				return (const PointerType*)((uint8_t*)&m_offset + m_offset);
			}

			PointerType& operator[](size_t inIndex)
			{
				assert(m_offset != 0); // nullptr
				uint8_t* ptr = (uint8_t*)&m_offset + m_offset;
				PointerType* result = (PointerType*)(ptr + inIndex * OffsetAmount);

				return *result;
			}

			const PointerType& operator[](size_t inIndex) const
			{
				assert(m_offset != 0); // nullptr
				uint8_t* baseOffset = (uint8_t*)&m_offset;
				uint8_t* ptr = baseOffset + m_offset;
				const PointerType* result = (const PointerType*)(ptr + inIndex * OffsetAmount);

				return *result;
			}

			const PointerType* get() const 
			{
				return (const PointerType*)((uint8_t*)&m_offset + m_offset);
			}

			void operator=(PointerType* inPointer) 
			{
				m_offset = inPointer ? ((uint8_t*)inPointer) - (uint8_t*)this : 0;
			}

		private:
			SizeType m_offset;
			RelPtr(RelPtr&) = delete; // Can't copy these! They'll break!
		};

		template<typename T, typename SizeType = uint16_t, int OffsetAmount = sizeof(T)>
		using Array = RelPtr<T, SizeType, OffsetAmount>;

		struct StringPacked
		{
			uint32_t Size;
			ASDF::RelPtr<char> Content;
		};
	#pragma pack(pop)
	}
}

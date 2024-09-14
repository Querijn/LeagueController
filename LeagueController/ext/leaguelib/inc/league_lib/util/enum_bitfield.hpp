#pragma once

#include <spek/util/types.hpp>

namespace LeagueLib
{
	template<typename EnumType, int DefaultValue = 0, typename FieldType = u32>
	struct EnumBitField
	{
	public:
		template<typename... Args>
		EnumBitField(Args&&... a_Args)
		{
			Add(std::forward<Args>(a_Args)...);
		}

		void Set(FieldType a_Type)
		{
			m_Data = a_Type;
		}

		operator FieldType() const { return m_Data; }

		template<typename... Args>
		inline void Add(EnumType a_Enum, Args&&... a_Args)
		{
			Add(a_Enum);
			Add(std::forward<Args>(a_Args)...);
		}

		inline void Add(EnumType a_Enum)
		{
			m_Data |= a_Enum;
		}

		inline void Add() {}

		template<typename... Args>
		inline void Remove(EnumType a_Enum, Args&&... a_Args)
		{
			Remove(a_Enum);
			Remove(std::forward<Args>(a_Args)...);
		}

		inline void Remove(EnumType a_Enum)
		{
			m_Data &= ~a_Enum;
		}

		inline void Remove() {}

		template<typename... Args>
		inline bool HasAll(EnumType a_Enum, Args&&... a_Args)
		{
			return Has(a_Enum) && Has(std::forward<Args>(a_Args)...);
		}

		template<typename... Args>
		inline bool HasAny(EnumType a_Enum, Args&&... a_Args)
		{
			return Has(a_Enum) || Has(std::forward<Args>(a_Args)...);
		}

		inline bool Has(EnumType a_Enum)
		{
			return (m_Data >> a_Enum) & 1U;
		}

	private:
		FieldType m_Data = DefaultValue;
	};
}
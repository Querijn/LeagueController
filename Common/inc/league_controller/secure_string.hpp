#pragma once

#include <league_controller/obfuscation.hpp>

#include <vector>
#include <string>

namespace LeagueController
{
	template <int... Pack> struct IndexList {};
	template <typename IndexList, int Right> struct Append;
	template <int... Left, int Right> struct Append<IndexList<Left...>, Right>
	{
		typedef IndexList<Left..., Right> Result;
	};
	template <int N> struct ConstructIndexList
	{
		typedef typename Append<typename ConstructIndexList<N - 1>::Result, N - 1>::Result Result;
	};
	template <> struct ConstructIndexList<0>
	{
		typedef IndexList<> Result;
	};
	template <typename IndexList> class StaticSecureStringX;
	template <int... Index> class StaticSecureStringX<IndexList<Index...>>
	{
	public:
		constexpr StaticSecureStringX(const char* const inString) : m_contents{ /* REDACTED */ } {}

		operator const char* () const
		{
			return m_contents;
		}

		size_t length() const
		{
			return sizeof...(Index);
		}

	private:
		char m_contents[sizeof...(Index) + 1];
	};
#define StaticSecureString(String) (LeagueController::StaticSecureStringX<LeagueController::ConstructIndexList<sizeof(String)-1>::Result>(String))

	class SecureString
	{
	public:
		SecureString() = default;
		SecureString(const SecureString& other) = default;
		
		template<typename L>
		constexpr SecureString(const StaticSecureStringX<L>& other) :
			m_data((const char*)other, (const char*)other + other.length())
		{
		}

		SecureString(const char* other) :
			m_data(other, other + strlen(other))
		{
		}

		bool operator==(const SecureString& str) const;
		bool operator!=(const SecureString& str) const;
		bool operator<(const SecureString& str) const;
		bool operator>(const SecureString& str) const;
		bool operator<=(const SecureString& str) const;
		bool operator>=(const SecureString& str) const;
		SecureString operator+(const SecureString& str);
		SecureString operator+(const char* s);
		SecureString operator+(char c);
		SecureString& operator=(const SecureString& s);
		SecureString& operator=(const char* s);
		SecureString& operator=(char ch);
		char& operator[](int index);

		std::vector<char>& GetData();
		const std::vector<char>& GetData() const;
		std::string ToEncryptedString() const;

	private:
		std::vector<char> m_data;
	};

	SecureString operator+(const char* s, const SecureString& s2);
	SecureString operator+(char c, const SecureString& s2);
}

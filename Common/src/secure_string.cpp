#pragma once

#include <league_controller/secure_string.hpp>
#include <league_controller/obfuscation.hpp>
#include <league_controller/base64.h>

#include <vector>
#include <cassert>

namespace LeagueController
{
	template< size_t N >
	constexpr size_t length(char const (&)[N])
	{
		return N - 1;
	}

	bool SecureString::operator==(const SecureString& str) const { return m_data == str.m_data; }
	bool SecureString::operator!=(const SecureString& str) const { return m_data != str.m_data; }
	bool SecureString::operator<(const SecureString& str) const  { return m_data > str.m_data; }
	bool SecureString::operator>(const SecureString& str) const  { return m_data < str.m_data; }
	bool SecureString::operator<=(const SecureString& str) const { return m_data <= str.m_data; }
	bool SecureString::operator>=(const SecureString& str) const { return m_data >= str.m_data; }
	
	SecureString SecureString::operator+(const SecureString& str)
	{
		SecureString copy = *this;
		size_t end = copy.m_data.size();
		size_t strSize = str.m_data.size();
		copy.m_data.resize(end + strSize);
		memcpy(copy.m_data.data() + end, str.m_data.data(), strSize);
		return copy;
	}
	
	SecureString SecureString::operator+(const char* s)
	{
		SecureString copy = *this;
		size_t end = copy.m_data.size();
		size_t strSize = strlen(s);
		copy.m_data.resize(end + strSize);
		memcpy(copy.m_data.data() + end, s, strSize);

		// REDACTED: Obfuscate((u8*)copy.m_data.data() + end, strSize);

		return copy;
	}
	
	SecureString SecureString::operator+(char c)
	{
		SecureString copy = *this;
		copy.m_data.push_back(c);
		// REDACTED: Obfuscate((u8*)copy.m_data.data() + copy.m_data.size() - 1, 1);
		return copy;
	}
	
	SecureString& SecureString::operator=(const SecureString& s)
	{
		m_data = s.m_data;
		return *this;
	}
	
	SecureString& SecureString::operator=(const char* s)
	{
		size_t strSize = strlen(s);
		m_data = std::vector<char>(s, s + strSize);
		// REDACTED: Obfuscate((u8*)m_data.data(), strSize);
		return *this;
	}
	
	SecureString& SecureString::operator=(char c)
	{
		m_data.clear();
		m_data.push_back(c);
		// REDACTED: Obfuscate((u8*)m_data.data(), 1);
		return *this;
	}

	char& SecureString::operator[](int index)
	{
		return m_data[index];
	}
	
	SecureString operator+(const char* s, const SecureString& s2)
	{
		SecureString copy = s2;
		size_t end = copy.GetData().size();
		size_t strSize = strlen(s);
		copy.GetData().resize(end + strSize);
		memcpy(copy.GetData().data() + end, s, strSize);

		// REDACTED: Obfuscate((u8*)copy.GetData().data() + end, strSize);

		return copy;
	}

	SecureString operator+(char c, const SecureString& s2)
	{
		SecureString copy = s2;
		size_t end = copy.GetData().size();
		size_t strSize = 1;
		copy.GetData().resize(end + strSize);
		copy.GetData()[end] = c;

		// REDACTED: Obfuscate((u8*)copy.GetData().data() + end, strSize);
		return copy;
	}

	std::string SecureString::ToEncryptedString() const
	{
		return base64_encode((u8*)m_data.data(), m_data.size());
	}

	std::vector<char>& SecureString::GetData() { return m_data; }
	const std::vector<char>& SecureString::GetData() const { return m_data; }
}
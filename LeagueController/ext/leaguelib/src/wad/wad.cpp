#include "league_lib/wad/wad.hpp"

#include <spek/util/assert.hpp>

#include <xxhash64.h>

extern "C"
{
#define ZSTD_STATIC_LINKING_ONLY
#include <zlib.h>
#include <zstd.h>
}

#include <fstream>

namespace LeagueLib
{
	using namespace Spek;

	namespace fs = std::filesystem;

#pragma pack(push, 1)
	struct BaseWAD
	{
		char Magic[2]; // RW
		char Major;
		char Minor;

		bool IsValid() const { return Magic[0] == 'R' && Magic[1] == 'W'; }
	};

	namespace WADv3
	{
		struct Header
		{
			BaseWAD Base;
			char ECDSA[256];
			uint64_t Checksum;
			uint32_t FileCount;
		};
	}
#pragma pack(pop)

	WAD::WAD(const char* a_FileName) :
		m_FileName(a_FileName)
	{
	}

	bool WAD::IsParsed() const
	{
		return m_IsParsed;
	}

	void WAD::Parse()
	{
		if (m_IsParsed)
			return;

		std::ifstream t_FileStream;
		t_FileStream.open(m_FileName, std::ifstream::binary);
		if (!t_FileStream)
		{
			m_LoadState = File::LoadState::FailedToLoad;
			m_IsParsed = true;
			return;
		}

		BaseWAD t_WAD;
		t_FileStream.read(reinterpret_cast<char*>(&t_WAD), sizeof(BaseWAD));
		assert(t_WAD.IsValid(), "The WAD header is not valid!");

		m_Version.Major = t_WAD.Major;
		m_Version.Minor = t_WAD.Minor;

		if (m_Version.Major != 3)
		{
			SPEK_ASSERT(m_Version.Major == 3, "Unexpected major version!");
			m_LoadState = File::LoadState::FailedToLoad;
			m_IsParsed = true;
			return;
		}

		t_FileStream.seekg(0, std::ios::beg);

		WADv3::Header t_Header;
		t_FileStream.read(reinterpret_cast<char*>(&t_Header), sizeof(WADv3::Header));

		for (int i = 0; i < t_Header.FileCount; i++)
		{
			WAD::FileData t_Source;
			t_FileStream.read(reinterpret_cast<char*>(&t_Source), sizeof(WAD::FileData));
			m_FileData[t_Source.PathHash] = t_Source;
		}

		m_LoadState = File::LoadState::Loaded;
		printf("Loaded %s\n", m_FileName.c_str());
		m_IsParsed = true;
	}

	bool WAD::HasFile(uint64_t a_FileHash) const
	{
		return m_FileData.find(a_FileHash) != m_FileData.end();
	}

	uint64_t HashFileName(const char* a_FileName)
	{
		char t_FileName[FILENAME_MAX];
		strncpy(t_FileName, a_FileName, FILENAME_MAX);
		for (char* t_Char = t_FileName; *t_Char; t_Char++)
			*t_Char = tolower(*t_Char);

		return XXHash64::hash(a_FileName, strlen(a_FileName), 0);
	}

	bool WAD::HasFile(const char* a_FileName) const
	{
		uint64_t t_Hash = HashFileName(a_FileName);
		return m_FileData.find(t_Hash) != m_FileData.end();
	}

	bool WAD::ExtractFile(std::string_view a_FileName, std::vector<u8>& a_Result)
	{
		return ExtractFile(HashFileName(a_FileName.data()), a_Result);
	}

	bool WAD::ExtractFile(uint64_t a_Hash, std::vector<u8>& a_Result)
	{
		const auto& t_FileDataIterator = m_FileData.find(a_Hash);
		if (t_FileDataIterator == m_FileData.end())
			return false;

		size_t t_Offset = 0;

		std::ifstream t_FileStream;
		t_FileStream.open(m_FileName.c_str(), std::ifstream::binary);

		const auto& t_FileData = t_FileDataIterator->second;

		t_FileStream.seekg(t_FileData.Offset, t_FileStream.beg);

		switch (t_FileData.Type)
		{
		case WAD::StorageType::UNCOMPRESSED:
		{
			a_Result.resize(t_FileData.FileSize);
			t_FileStream.read((char*)a_Result.data(), t_FileData.FileSize);
			break;
		}

		case WAD::StorageType::UNKNOWN:
			printf("Unknown WAD storage type found\n");
			__debugbreak();
			break;

		case WAD::StorageType::ZSTD_COMPRESSED:
		{
			std::vector<char> t_CompressedData(t_FileData.CompressedSize);
			t_FileStream.read(t_CompressedData.data(), t_FileData.CompressedSize);

			size_t t_UncompressedSize = t_FileData.FileSize;

			std::vector<uint8_t> t_Uncompressed;
			t_Uncompressed.resize(t_UncompressedSize);
			size_t t_Size = ZSTD_decompress(t_Uncompressed.data(), t_UncompressedSize, t_CompressedData.data(), t_FileData.CompressedSize);
			if (ZSTD_isError(t_Size))
			{
				printf("ZSTD Error trying to unpack '%u': %s", a_Hash, ZSTD_getErrorName(t_Size));
				return false;
			}

			a_Result = t_Uncompressed;
			break;
		}

		default:
			printf("Unidentified storage type detected: %d\n", t_FileData.Type);
			break;
		}

		return a_Result.empty() == false;
	}

	Spek::File::LoadState WAD::GetLoadState() const
	{
		return  m_LoadState;
	}

	WAD::MinFileData::MinFileData() { }

	WAD::MinFileData::MinFileData(const FileData& a_FileData)
	{
		Offset = a_FileData.Offset;
		CompressedSize = a_FileData.CompressedSize;
		FileSize = a_FileData.FileSize;
		Type = a_FileData.Type;
	}
}
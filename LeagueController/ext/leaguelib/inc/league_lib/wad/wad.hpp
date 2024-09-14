#pragma once

#include <spek/file/file.hpp>

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>

namespace LeagueLib
{
	class WAD
	{
	public:
		using FileNameHash = uint64_t;
		enum StorageType : uint8_t
		{
			UNCOMPRESSED = 0,
			ZLIB_COMPRESSED = 1,
			UNKNOWN = 2,
			ZSTD_COMPRESSED = 3
		};
	#pragma pack(push, 1)
		struct FileData
		{
			uint64_t PathHash;
			uint32_t Offset;
			uint32_t CompressedSize;
			uint32_t FileSize;
			WAD::StorageType Type;
			uint8_t Duplicate;
			uint8_t Unknown[2];
			uint64_t SHA256;
		};
	#pragma pack(pop)

		struct MinFileData
		{
			MinFileData();
			MinFileData(const FileData& a_FileData);

			uint32_t Offset;
			uint32_t CompressedSize;
			uint32_t FileSize;
			WAD::StorageType Type;
		};

		using FileDataMap = std::unordered_map<FileNameHash, MinFileData>;

		WAD(const char* a_FileName);

		bool IsParsed() const;
		void Parse();

		bool HasFile(uint64_t a_FileHash) const;
		bool HasFile(const char* a_FileName) const;
		bool ExtractFile(std::string_view a_FileName, std::vector<u8>& a_Output);
		bool ExtractFile(uint64_t a_FileName, std::vector<u8>& a_Result);

		Spek::File::LoadState GetLoadState() const;

		FileDataMap::const_iterator begin() const { return m_FileData.begin(); }
		FileDataMap::const_iterator end() const { return m_FileData.end(); }

		std::string GetFileName() const { return m_FileName; }

	private:
		FileDataMap m_FileData;
		std::string m_FileName;
		Spek::File::LoadState m_LoadState = Spek::File::LoadState::NotLoaded;

		struct { char Major = 0, Minor = 0; } m_Version;
		bool m_IsParsed = false;
	};
}
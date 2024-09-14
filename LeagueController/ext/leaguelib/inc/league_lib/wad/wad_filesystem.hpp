#pragma once

#include <league_lib/wad/wad.hpp>

#include <spek/file/base_filesystem.hpp>
#include <spek/file/default_filesystem.hpp>
#include <spek/file/file.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

namespace LeagueLib
{
	class WADFileSystem : public Spek::BaseFileSystem
	{
	public:
		WADFileSystem(const char* a_Root);
		~WADFileSystem();

		void Update() override;

	protected:
		Spek::File::Handle Get(const char* a_Location, u32 a_LoadFlags) override;
		Spek::File::Handle GetInternal(const char* a_Location, bool a_Invalidate);
		bool Has(Spek::File::Handle a_Pointer) const override;
		void IndexFiles(OnIndexFunction a_OnIndex) override;
		bool Exists(const char* a_Location) override;
		std::string GetRelativePath(std::string_view a_AbsolutePath) const;
		bool IsReadyToExit() const override;

		struct LoadRequest
		{
			WAD* Archive;
			Spek::File::Handle File;
			std::string Name;
		};

		std::vector<std::string> m_Entries;
		std::vector<std::unique_ptr<WAD>> m_Archives;
		std::unordered_map<uint64_t, Spek::File::Handle> m_Files;
		std::vector<LoadRequest> m_LoadRequests;

		static std::unordered_map<WADFileSystem*, WADFileSystem::OnIndexFunction> m_IndexFunctions; // TODO: Clean up
	};
}

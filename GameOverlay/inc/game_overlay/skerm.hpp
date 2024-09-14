#pragma once

#include <spek/file/file.hpp>
#include <skerm/source_file.hpp>
#include <skerm/compiler.hpp>
#include <skerm/asdf.hpp>

#include <memory>
#include <vector>
#include <filesystem>

#include <glm/vec2.hpp>

namespace GameOverlay
{
	class SourceHandler
	{
	public:
		struct Source
		{
			using Ptr = std::shared_ptr<Source>;

			Spek::File::Handle FileHandle;
			Skerm::SourceFile Source;
			Skerm::SourceFileErrors Errors;
			Skerm::ASDF::UINamespace Output;
		};

		SourceHandler(std::filesystem::path sourceFolder)
		{
			std::filesystem::path f = sourceFolder / "GameOverlay/ext/skerm/data/asdf";
			m_compiler.AddFolder(f.generic_string().c_str());
		}

		using OnLoadFunction = std::function<void(Source::Ptr, Spek::File::Handle, Spek::File::LoadState)>;
		Spek::File::Handle CompileFile(const char* inSourceFile, OnLoadFunction inOnLoad = nullptr);
		Skerm::Compiler::CompileOutput CompileSourceFile(Source& inFile);
		Source::Ptr GetSourceByName(const char* inSourceFile) const;
		void Clear();

	private:
		std::vector<Source::Ptr> m_sources;
		Skerm::Compiler m_compiler;
	};

	void RenderUI(Skerm::ASDF::UIContainer* container, int screenWidth, int screenHeight);
	void RenderUI(Skerm::ASDF::UIContainer* container, const glm::vec2& min, const glm::vec2& max, int screenWidth, int screenHeight);

	void DestroySkerm();
	
	int GetWidth();
	int GetHeight();
}
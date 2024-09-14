#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <map>
#include <string>

namespace Skerm
{
	struct SourceFileErrors
	{
		std::map<std::string, std::vector<std::string>> Errors;
		std::map<std::string, std::vector<std::string>> Warnings;
	};

	enum class HorizontalAnchor
	{
		Unknown = -2,
		Left = -1,
		Center = 0,
		Right = 1,
	};

	enum class VerticalAnchor
	{
		Unknown = -2,
		Top = -1,
		Middle = 0,
		Bottom = 1,
	};

	const char* GetName(VerticalAnchor inAnchor);
	const char* GetName(HorizontalAnchor inAnchor);

	void ParseAnchor(const char* inAnchor, const char* inElementName, HorizontalAnchor& inTypeX, VerticalAnchor& inTypeY, SourceFileErrors* inErrorContainer);

	struct SourceFileContainer
	{
		virtual void Verify(SourceFileErrors& inErrorContainer) const;
		virtual bool IsDerived() const { return false; }
		virtual bool IsImage() const { return false; }
		virtual bool IsText() const { return false; }

		std::string Name;
		std::string Left, Right, Top, Bottom;
		std::string X, Y;
		std::string Width, Height;
		std::string Color;

		HorizontalAnchor AnchorX;
		VerticalAnchor AnchorY;

		std::vector<std::shared_ptr<SourceFileContainer>> Children;
	};

	struct SourceFileImage : public SourceFileContainer
	{
		bool IsDerived() const override { return true; }
		bool IsImage() const override { return true; }

		std::string Source;
		std::string Sprite;
	};

	struct SourceFileText : public SourceFileContainer
	{
		bool IsDerived() const override { return true; }
		bool IsText() const override { return true; }

		std::string Content;
		std::string FontFile;
		std::string FontSize;
	};

	struct SourceFile
	{
		void Verify(SourceFileErrors& inErrorContainer) const;

		static bool Parse(const char* inFileName, SourceFile& inOutput, SourceFileErrors* inErrorContainer = nullptr);

		std::vector<std::shared_ptr<SourceFileContainer>> Elements;
		std::string Name;
	};
}

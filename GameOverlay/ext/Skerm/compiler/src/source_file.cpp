#include <skerm/source_file.hpp>
#include <skerm/tokeniser.hpp>

#include <pugixml.hpp>

#include <asdf/error_handling.hpp>

namespace Skerm
{
	void ParseContainer(pugi::xml_node& inSource, SourceFileContainer& inOutput, SourceFileErrors* inErrorContainer);

	const char* GetName(VerticalAnchor inAnchor)
	{
		switch (inAnchor)
		{
		case VerticalAnchor::Bottom:
			return "Bottom";
		case VerticalAnchor::Middle:
			return "Middle";
		case VerticalAnchor::Top:
			return "Top";
		}

		return "unknown";
	}

	const char* GetName(HorizontalAnchor inAnchor)
	{
		switch (inAnchor)
		{
		case HorizontalAnchor::Left:
			return "Left";
		case HorizontalAnchor::Right:
			return "Right";
		case HorizontalAnchor::Center:
			return "Center";
		}

		return "unknown";
	}

	template <typename... Args>
	void ParseAssertWarning(bool inCondition, SourceFileErrors* inErrorContainer, const std::string& inElement, char const* const inMessage, Args const& ...inArgs) noexcept
	{
		if (inCondition || inErrorContainer == nullptr)
			return;

		char errorBuffer[1024];
		snprintf(errorBuffer, 1024, inMessage, inArgs...);

		inErrorContainer->Warnings[inElement].push_back(errorBuffer);
	}

	template <typename... Args>
	void ParseAssertError(bool inCondition, SourceFileErrors* inErrorContainer, const std::string& inElement, char const* const inMessage, Args const& ...inArgs) noexcept
	{
		if (inCondition || inErrorContainer == nullptr)
			return;

		char errorBuffer[1024];
		snprintf(errorBuffer, 1024, inMessage, inArgs...);

		inErrorContainer->Errors[inElement].push_back(errorBuffer);
	}

	void ParseAnchor(const char* inAnchor, const char* inElementName, SourceFileContainer& inContainer, HorizontalAnchor& inTypeX, VerticalAnchor& inTypeY, SourceFileErrors* inErrorContainer)
	{
		inTypeX = HorizontalAnchor::Unknown;
		inTypeY = VerticalAnchor::Unknown;

		// Parse the source
		const char* text = inAnchor;
		while (*text != 0)
		{
			std::string currentToken;
			bool shouldBreak = false;
			for (char character = *text; character != 0; text++, character = *text)
			{
				if (character >= 'A' && character <= 'Z')
					character += 32;
				else if (character < 'a' || character > 'z')
				{
					shouldBreak = true;
					continue;
				}

				if (shouldBreak)
					break;

				currentToken += character;
			}

			if (currentToken == "bottom")
			{
				ParseAssertWarning(inTypeY == VerticalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeY), currentToken.c_str());
				inTypeY = VerticalAnchor::Bottom;
			}
			else if (currentToken == "middle")
			{
				ParseAssertWarning(inTypeY == VerticalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeY), currentToken.c_str());
				inTypeY = VerticalAnchor::Middle;
			}
			else if (currentToken == "top")
			{
				ParseAssertWarning(inTypeY == VerticalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeY), currentToken.c_str());
				inTypeY = VerticalAnchor::Top;
			}
			else if (currentToken == "left")
			{
				ParseAssertWarning(inTypeX == HorizontalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeX), currentToken.c_str());
				inTypeX = HorizontalAnchor::Left;
			}
			else if (currentToken == "center")
			{
				ParseAssertWarning(inTypeX == HorizontalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeX), currentToken.c_str());
				inTypeX = HorizontalAnchor::Center;
			}
			else if (currentToken == "right")
			{
				ParseAssertWarning(inTypeX == HorizontalAnchor::Unknown, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s', while it already had parsed the value '%s'. It's now set to '%s'.",
					currentToken.c_str(), inElementName, GetName(inTypeX), currentToken.c_str());
				inTypeX = HorizontalAnchor::Right;
			}
			else
			{
				ParseAssertError(false, inErrorContainer, inElementName,
					"Anchor parser found '%s' token in '%s'. This is not a valid value, it needs to be 'Left', 'Right', 'Top' or 'Bottom'.",
					currentToken.c_str(), inElementName, GetName(inTypeY), currentToken.c_str());
				inTypeX = HorizontalAnchor::Unknown;
				inTypeY = VerticalAnchor::Unknown;
				break;
			}
		}

		if (inTypeX == HorizontalAnchor::Unknown && inTypeY == VerticalAnchor::Unknown)
		{
			if (inContainer.Left.empty() == false)
				inTypeX = HorizontalAnchor::Left;
			else if (inContainer.Right.empty() == false)
				inTypeX = HorizontalAnchor::Right;
			else
				inTypeX = HorizontalAnchor::Center;

			if (inContainer.Top.empty() == false)
				inTypeY = VerticalAnchor::Top;
			else if (inContainer.Bottom.empty() == false)
				inTypeY = VerticalAnchor::Bottom;
			else	  
				inTypeY = VerticalAnchor::Middle;
		}

		// Default to center if necessary
		if (inTypeX == HorizontalAnchor::Unknown)
			inTypeX = HorizontalAnchor::Center;
		if (inTypeY == VerticalAnchor::Unknown)
			inTypeY = VerticalAnchor::Middle;
	}

	void ParseImage(pugi::xml_node& inSource, SourceFileImage& inOutput, SourceFileErrors* inErrorContainer)
	{
		ParseContainer(inSource, inOutput, inErrorContainer);
		inOutput.Source = inSource.attribute("Source").value();
		inOutput.Sprite = inSource.attribute("Sprite").value();

		if (inOutput.Sprite.empty())
			inOutput.Sprite = "[0,0,1,1]"; // Default to the full image.
	}

	void ParseText(pugi::xml_node& inSource, SourceFileText& inOutput, SourceFileErrors* inErrorContainer)
	{
		ParseContainer(inSource, inOutput, inErrorContainer);
		inOutput.Content = inSource.attribute("Content").value();
		inOutput.FontFile = inSource.attribute("FontFile").value();
		inOutput.FontSize = inSource.attribute("FontSize").value();

		if (inOutput.Width.empty())
			inOutput.Width = "100w";
		if (inOutput.Height.empty())
			inOutput.Height = "100h";
	}

	void ParseElementContainer(std::vector<std::shared_ptr<SourceFileContainer>>& inElements, pugi::xml_node inParent, SourceFileErrors* inErrorContainer)
	{
		for (auto& child : inParent.children())
		{
			auto name = child.name();
			if (stricmp(name, "Container") == 0)
			{
				std::shared_ptr<SourceFileContainer> container = std::make_shared<SourceFileContainer>();
				ParseContainer(child, *container, inErrorContainer);

				inElements.push_back(std::move(container));
				continue;
			}
			else if (stricmp(name, "Image") == 0)
			{
				std::shared_ptr<SourceFileImage> image = std::make_shared<SourceFileImage>();
				ParseImage(child, *image, inErrorContainer);

				inElements.push_back(std::move(image));
				continue;
			}
			else if (stricmp(name, "Text") == 0)
			{
				std::shared_ptr<SourceFileText> text = std::make_shared<SourceFileText>();
				ParseText(child, *text, inErrorContainer);

				inElements.push_back(std::move(text));
				continue;
			}
			else
			{
				__debugbreak();
			}
		}
	}

	void ParseContainer(pugi::xml_node& inSource, SourceFileContainer& inOutput, SourceFileErrors* inErrorContainer)
	{
		inOutput.Name = inSource.attribute("Name").value();

		inOutput.Left = inSource.attribute("Left").value();
		inOutput.Right = inSource.attribute("Right").value();
		inOutput.Top = inSource.attribute("Top").value();
		inOutput.Bottom = inSource.attribute("Bottom").value();

		inOutput.X = inSource.attribute("X").value();
		inOutput.Y = inSource.attribute("Y").value();
		inOutput.Width = inSource.attribute("Width").value();
		inOutput.Height = inSource.attribute("Height").value();

		inOutput.Color = inSource.attribute("Color").value();

		std::string anchor = inSource.attribute("Anchor").value();
		ParseAnchor(anchor.c_str(), inOutput.Name.c_str(), inOutput, inOutput.AnchorX, inOutput.AnchorY, inErrorContainer);

		ParseElementContainer(inOutput.Children, inSource, inErrorContainer);
	}

	bool SourceFile::Parse(const char* inFileName, SourceFile& inOutput, SourceFileErrors* inErrorContainer)
	{
		inOutput = SourceFile();

		pugi::xml_document file;
		auto result = file.load_file(inFileName);
		if (!result)
			return false;

		pugi::xml_node skerm = file.child("Skerm");
		if (!skerm)
			return false;

		ParseElementContainer(inOutput.Elements, skerm, inErrorContainer);

		inOutput.Name = skerm.attribute("Namespace").value();

		return true;
	}

	void VerifyEquation(const std::string& inName, const std::string& inEquation, SourceFileErrors& inErrorContainer)
	{
		if (inEquation.empty())
			return;

		auto result = Skerm::Tokenize(inEquation.c_str());
		if (result.Errors.empty() == false)
		{
			auto& errors = inErrorContainer.Errors[inName];
			errors.insert(errors.end(), result.Errors.begin(), result.Errors.end());
		}
	}

	void SourceFileContainer::Verify(SourceFileErrors& inErrorContainer) const
	{
		VerifyEquation(Name, Left, inErrorContainer);
		VerifyEquation(Name, Right, inErrorContainer);
		VerifyEquation(Name, Top, inErrorContainer);
		VerifyEquation(Name, Bottom, inErrorContainer);

		VerifyEquation(Name, X, inErrorContainer);
		VerifyEquation(Name, Y, inErrorContainer);
		VerifyEquation(Name, Width, inErrorContainer);
		VerifyEquation(Name, Height, inErrorContainer);

		VerifyEquation(Name, Color, inErrorContainer);
	}

	void SourceFile::Verify(SourceFileErrors& inErrorContainer) const
	{
		for (auto& element : Elements)
		{
			element->Verify(inErrorContainer);
		}
	}
}

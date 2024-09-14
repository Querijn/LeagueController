#include <skerm/compiler.hpp>
#include <skerm/math_parser.hpp>

#include <asdf/compiler.hpp>

#include <fstream>
#include <string>

namespace Skerm
{
	static ASDF::DataParser::ElementData g_divTemplate;
	static ASDF::DataParser::ElementData g_mulTemplate;
	static ASDF::DataParser::ElementData g_addTemplate;
	static ASDF::DataParser::ElementData g_subTemplate;
	static ASDF::DataParser::ElementData g_varTemplate;
	static ASDF::DataParser::ElementData g_equationTemplate;
	static ASDF::DataParser::ElementData g_structTemplate;
	static bool g_areTemplatesSetup = false;

	template<typename TokenType>
	void CompileEquation(ASDF::DataParser::ElementData* inElement, const TokenType& inEquation, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser, const char* inTypeHint = "EquationTokenType")
	{
		if (inTypeHint)
			inElement->GetSubElement("Type")->Value = inTypeHint;

		ASDF::DataParser::ElementData* tokenArray = inElement->GetSubElement("Tokens").get();
		if (!g_areTemplatesSetup)
		{
			g_areTemplatesSetup = true;

			g_divTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathDivisionToken"));
			g_mulTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathMultiplicationToken"));
			g_addTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathAdditionToken"));
			g_subTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathSubtractionToken"));
			g_varTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathVariableToken"));
			g_equationTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathEquationToken"));
			g_structTemplate = inDataParser.ParseStruct(*(ASDF::TypeInfo::StructElement*)inInfo.GetElement("MathStructToken"));

			g_divTemplate.GetSubElement("Type")->Value = "DivisionTokenType";
			g_mulTemplate.GetSubElement("Type")->Value = "MultiplicationTokenType";
			g_addTemplate.GetSubElement("Type")->Value = "AdditionTokenType";
			g_subTemplate.GetSubElement("Type")->Value = "SubtractionTokenType";
			g_varTemplate.GetSubElement("Type")->Value = "ValueVariableTokenType";
			g_equationTemplate.GetSubElement("Type")->Value = "EquationTokenType";
			g_structTemplate.GetSubElement("Type")->Value = "StructTokenType";
		}

		// Write out these equations to the document
		for (const Token::Ptr token : inEquation)
		{
			// Ordered by most likely to appear
			if (token->IsValue())
			{
				auto variable = g_varTemplate.Copy();
				variable.GetSubElement("Value")->Value = token->GetValue();
				variable.GetSubElement("Variable")->Value = token->GetVar();
				tokenArray->AddToArray(variable);
			}
			else if (token->IsMultiply())
			{
				tokenArray->AddToArray(g_mulTemplate);
			}
			else if (token->IsAdd())
			{
				tokenArray->AddToArray(g_addTemplate);
			}
			else if (token->IsEquation())
			{
				auto equation = g_equationTemplate.Copy();
				CompileEquation(&equation, *(const EquationToken*)token.get(), inInfo, inDataParser);
				tokenArray->AddToArray(equation);
			}
			else if (token->IsStruct())
			{
				auto asStruct = g_structTemplate.Copy();
				CompileEquation(&asStruct, *(const StructToken*)token.get(), inInfo, inDataParser, nullptr);
				tokenArray->AddToArray(asStruct);
			}
			else if (token->IsSubtract())
			{
				tokenArray->AddToArray(g_subTemplate);
			}
			else if (token->IsDivide())
			{
				tokenArray->AddToArray(g_divTemplate);
			}
			else
			{
				__debugbreak();
			}
		}
	}

	struct HorizontalWarnings
	{
		const char* AnchorLeftIgnoresX = "Left' and 'X' are both defined on '%s', while anchored to the left. 'Left' is always used before 'X', so 'X' is ignored here.";
		const char* AnchorLeftIgnoresWidth = "'Right' and 'Width' are both defined on '%s', while anchored to the left. 'Right' is always used before 'Width', so 'Width' is ignored here.";

		const char* AnchorCenterIgnoresLeft = "'Left', 'Right' and 'X' are invalid arguments on '%s' due to the anchor being 'center'. They are being ignored, please use 'Width'.";

		const char* AnchorRightIgnoresX = "'Right' and 'X' are both defined on '%s', while anchored to the right. 'Right' is always used before 'X', so 'X' is ignored here.";
		const char* AnchorRightIgnoresWidth = "'Left' and 'Width' are both defined on '%s', while anchored to the right. 'Left' is always used before 'Width', so 'Width' is ignored here.";
	} g_horizontalWarnings;

	struct VerticalWarnings
	{
		const char* AnchorLeftIgnoresX = "Top' and 'Y' are both defined on '%s', while anchored to the top. 'Top' is always used before 'Y', so 'Y' is ignored here.";
		const char* AnchorLeftIgnoresWidth = "'Bottom' and 'Height' are both defined on '%s', while anchored to the top. 'Bottom' is always used before 'Height', so 'Height' is ignored here.";

		const char* AnchorCenterIgnoresLeft = "'Top', 'Bottom' and 'Y' are invalid arguments on '%s' due to the anchor being 'middle'. They are being ignored, please use 'Height'.";

		const char* AnchorRightIgnoresX = "'Bottom' and 'Y' are both defined on '%s', while anchored to the bottom. 'Bottom' is always used before 'Y', so 'Y' is ignored here.";
		const char* AnchorRightIgnoresWidth = "'Top' and 'Height' are both defined on '%s', while anchored to the bottom. 'Top' is always used before 'Height', so 'Height' is ignored here.";
	} g_verticalWarnings;

	// This function is both used for vertical and horizontal offsets. It mentions left and right, which will translate to top and bottom respectively. X becomes Y, Width becomes Height.
	template<typename EnumType, typename WarningsType>
	void CompileOffsets(SourceFileContainer& inSource, EnumType inType, std::string& inLeftOut, std::string& inRightOut, const std::string& inLeft, const std::string& inRight, const std::string& inX, const std::string& inWidth, const WarningsType& inWarnings, ASDF::ErrorHandler& inErrorHandler)
	{
		const std::string& name = inSource.Name;
		switch ((int)inType)
		{
		case -1: // Left or Top
			if (!inLeft.empty())
				inLeftOut = inLeft;
			else if (!inX.empty())
				inLeftOut = inX;
			else
				inLeftOut = "0";
			inErrorHandler.AssertWarning(inX.empty() || inLeft.empty(), inWarnings.AnchorLeftIgnoresX, name.c_str()); // Output a warning that X is ignored

			if (!inRight.empty())
				inRightOut = inRight;
			else if (!inWidth.empty())
				inRightOut = std::string("100% - ((") + inWidth + ") + (" + inLeftOut + "))"; // Calculate right from width + left
			else
				inRightOut = "0";
			inErrorHandler.AssertWarning(inRight.empty() || inWidth.empty(), inWarnings.AnchorLeftIgnoresWidth, name.c_str());
			break;

			// If the anchor is centered, you cannot use left, right or x. Use width instead.
		default:
		case 0: // Middle or center
			if (inWidth.empty())
			{
				inRightOut = "0";
				inLeftOut = inRightOut;
			}
			else
			{
				inRightOut = std::string("50% - 0.5 * (") + inWidth + ")";
				inLeftOut = std::string("50% - 0.5 * (") + inWidth + ")";
			}

			inErrorHandler.AssertWarning(inX.empty() && inLeft.empty() && inRight.empty(), inWarnings.AnchorCenterIgnoresLeft, name.c_str());
			break;

		case 1: // Right or Bottom
			if (!inRight.empty())
				inRightOut = inRight;
			else if (!inX.empty())
				inRightOut = inX;
			else
				inRightOut = "0";
			inErrorHandler.AssertWarning(inX.empty() || inRight.empty(), inWarnings.AnchorRightIgnoresX, name.c_str());

			if (!inLeft.empty())
				inLeftOut = inLeft;
			else if (!inWidth.empty())
				inLeftOut = std::string("100% - ((") + inWidth + ") + (" + inRightOut + "))"; // Calculate left from width + right
			else
				inLeftOut = "0";
			inErrorHandler.AssertWarning(inLeft.empty() || inWidth.empty(), inWarnings.AnchorRightIgnoresWidth, name.c_str());
			break;
		}
	}

	namespace Internal
	{
		struct CompilerData : ASDF::Compiler
		{
			ASDF::DataParser::ElementData NamespaceTemplate;
			ASDF::DataParser::ElementData ContainerTemplate;
			ASDF::DataParser::ElementData ImageTemplate;
			ASDF::DataParser::ElementData TextTemplate;

			bool CompileFileInternal(const SourceFile& inUIFile, ASDF::DataParser::ElementData& ns);
			void CompileElement(SourceFileContainer& child, ASDF::DataParser::ElementData& inArray);
			bool CompileContainer(SourceFileContainer& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser);
			bool CompileImage(SourceFileImage& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser);
			bool CompileText(SourceFileText& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser);
		};
	}
	
	Compiler::Compiler() :
		m_data(std::make_unique<Internal::CompilerData>())
	{
	}

	Compiler::~Compiler(){}

	void Compiler::AddFolder(const char* inSource)
	{
		m_data->Schema.AddFolder(inSource);

		// Init some required templates
		auto ns = m_data->Info.GetElement("UINamespace");
		if (ns && m_data->NamespaceTemplate.Element == nullptr)
			m_data->NamespaceTemplate = m_data->Data.ParseStruct(*(ASDF::TypeInfo::StructElement*)ns);

		auto uicontainer = m_data->Info.GetElement("UIContainer");
		if (uicontainer && m_data->ContainerTemplate.Element == nullptr)
		{
			m_data->ContainerTemplate = m_data->Data.ParseStruct(*(ASDF::TypeInfo::StructElement*)uicontainer);
			m_data->ContainerTemplate.GetSubElement("Type")->Value = "ContainerType";
		}

		auto uiimage = m_data->Info.GetElement("UIImage");
		if (uiimage && m_data->ImageTemplate.Element == nullptr)
		{
			m_data->ImageTemplate = m_data->Data.ParseStruct(*(ASDF::TypeInfo::StructElement*)uiimage);
			m_data->ImageTemplate.GetSubElement("Type")->Value = "ImageType";
		}

		auto uitext = m_data->Info.GetElement("UIText");
		if (uitext && m_data->TextTemplate.Element == nullptr)
		{
			m_data->TextTemplate = m_data->Data.ParseStruct(*(ASDF::TypeInfo::StructElement*)uitext);
			m_data->TextTemplate.GetSubElement("Type")->Value = "TextType";
		}
	}

	bool Internal::CompilerData::CompileContainer(SourceFileContainer& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser)
	{
		inTarget.GetSubElement("Name")->Value = inSource.Name;

		// Parse anchor
		auto targetAnchor = inTarget.GetSubElement("Anchor");
		targetAnchor->Value = std::string(GetName(inSource.AnchorY)) + GetName(inSource.AnchorX);

		std::string Left;
		std::string Right;
		std::string Top;
		std::string Bottom;

		// Calculate horizontal offsets
		CompileOffsets(inSource, inSource.AnchorX, Left, Right, inSource.Left, inSource.Right, inSource.X, inSource.Width, g_horizontalWarnings, inErrorHandler);

		// Calculate vertical offsets
		CompileOffsets(inSource, inSource.AnchorY, Top, Bottom, inSource.Top, inSource.Bottom, inSource.Y, inSource.Height, g_verticalWarnings, inErrorHandler);

		auto colorTokens = Skerm::Tokenize(inSource.Color.c_str());
		auto leftTokens = Skerm::Tokenize(Left.c_str());
		auto rightTokens = Skerm::Tokenize(Right.c_str());
		auto topTokens = Skerm::Tokenize(Top.c_str());
		auto bottomTokens = Skerm::Tokenize(Bottom.c_str());

		colorTokens.Equation.ReduceSelf();
		leftTokens.Equation.ReduceSelf();
		rightTokens.Equation.ReduceSelf();
		topTokens.Equation.ReduceSelf();
		bottomTokens.Equation.ReduceSelf();

		CompileEquation(inTarget.GetSubElement("Color").get(), colorTokens.Equation, inInfo, inDataParser);

		CompileEquation(inTarget.GetSubElement("Left").get(), leftTokens.Equation, inInfo, inDataParser);
		CompileEquation(inTarget.GetSubElement("Right").get(), rightTokens.Equation, inInfo, inDataParser);
		CompileEquation(inTarget.GetSubElement("Top").get(), topTokens.Equation, inInfo, inDataParser);
		CompileEquation(inTarget.GetSubElement("Bottom").get(), bottomTokens.Equation, inInfo, inDataParser);

		auto array = inTarget.GetSubElement("Children");
		for (auto& child : inSource.Children)
			CompileElement(*child, *array);

		return true;
	}

	bool Internal::CompilerData::CompileImage(SourceFileImage& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser)
	{
		if (CompileContainer(inSource, inTarget, inErrorHandler, inInfo, inDataParser) == false)
			return false;

		std::string spriteExtents = inSource.Sprite;
		auto spriteExtentTokens = Skerm::Tokenize(spriteExtents.c_str());
		spriteExtentTokens.Equation.ReduceSelf();

		inTarget.GetSubElement("Source")->Value = inSource.Source;

		CompileEquation(inTarget.GetSubElement("Sprite").get(), spriteExtentTokens.Equation, inInfo, inDataParser);

		return true;
	}

	bool Internal::CompilerData::CompileText(SourceFileText& inSource, ASDF::DataParser::ElementData& inTarget, ASDF::ErrorHandler& inErrorHandler, ASDF::TypeInfo& inInfo, ASDF::DataParser& inDataParser)
	{
		if (CompileContainer(inSource, inTarget, inErrorHandler, inInfo, inDataParser) == false)
			return false;

		auto fontSize = inSource.FontSize.empty() == false ? inSource.FontSize : "100%";
		auto fontSizeTokens = Skerm::Tokenize(fontSize.c_str());
		fontSizeTokens.Equation.ReduceSelf();

		inTarget.GetSubElement("Content")->Value = inSource.Content;
		inTarget.GetSubElement("FontFile")->Value = inSource.FontFile;
		CompileEquation(inTarget.GetSubElement("FontSize").get(), fontSizeTokens.Equation, inInfo, inDataParser);
		return true;
	}

	void Internal::CompilerData::CompileElement(SourceFileContainer& inChild, ASDF::DataParser::ElementData& inArray)
	{
		if (inChild.IsDerived() == false)
		{
			auto container = ContainerTemplate.Copy();
			CompileContainer(inChild, container, Errors, Info, Data);

			inArray.AddToArray(container);
		}
		else if (inChild.IsImage())
		{
			auto container = ImageTemplate.Copy();
			CompileImage(static_cast<SourceFileImage&>(inChild), container, Errors, Info, Data);

			inArray.AddToArray(container);
		}
		else if (inChild.IsText())
		{
			auto container = TextTemplate.Copy();
			CompileText(static_cast<SourceFileText&>(inChild), container, Errors, Info, Data);

			inArray.AddToArray(container);
		}
		else
		{
			__debugbreak();
		}
	}

	bool Internal::CompilerData::CompileFileInternal(const SourceFile& inUIFile, ASDF::DataParser::ElementData& ns)
	{
		Errors.Clear();

		ns.GetSubElement("Name")->Value = inUIFile.Name;

		auto array = ns.GetSubElement("Elements");
		for (auto& child : inUIFile.Elements)
			CompileElement(*child, *array);

		return true;
	}

	bool Compiler::CompileFile(const SourceFile& inUIFile, CompileOutput& inOutput)
	{
		ASDF::DataParser::ElementData ns = m_data->NamespaceTemplate.Copy();
		if (m_data->CompileFileInternal(inUIFile, ns) == false)
			return false;

		ASDF::DataParser::OutputBlobOutput state;
		m_data->Data.OutputBlobToMemory(state, ns, m_data->Serialisation);
		inOutput.Data = state.Data;
		inOutput.Errors = m_data->Errors.GetErrors();
		inOutput.Warnings = m_data->Errors.GetWarnings();

	#if ASDF_DEBUG_DATA_PARSING
		inOutput.DebugData = state.DebugData;

		{
			std::ofstream debugFile("debug.json", std::ios::binary);
			debugFile.write(state.DebugData.data(), state.DebugData.size());
		}
		{
			std::ofstream outputFile("debug.bin", std::ios::binary);
			outputFile.write(reinterpret_cast<const char*>(state.Data.data()), state.Data.size());
			outputFile.close();

		}
	#endif

		m_data->Errors.OutputErrors();
		m_data->Errors.OutputWarnings();
		return true;
	}
	
	bool Compiler::CompileFile(const SourceFile& inUIFile, const char* inOutputFile)
	{
		ASDF::DataParser::ElementData ns = m_data->NamespaceTemplate.Copy();
		if (m_data->CompileFileInternal(inUIFile, ns) == false)
			return false;

		m_data->Data.OutputBlob(inOutputFile, ns, m_data->Serialisation);
		return true;
	}

	bool Compiler::CompileFile(const char* inUIFile, CompileOutput& inOutput)
	{
		SourceFile file;
		if (SourceFile::Parse(inUIFile, file) == false)
			return false;

		return CompileFile(file, inOutput);
	}

	bool Compiler::CompileFile(const char* inUIFile, const char* inOutputFile)
	{
		SourceFile file;
		if (SourceFile::Parse(inUIFile, file) == false)
			return false;

		return CompileFile(file, inOutputFile);
	}
}

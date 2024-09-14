#pragma once

#include <skerm/asdf_interpreter.hpp>
#include <skerm/tokens.hpp>

#include <skerm/ui_container.hpp>
#include <skerm/ui_image.hpp>
#include <skerm/ui_text.hpp>
#include <skerm/math_addition_token.hpp>
#include <skerm/math_subtraction_token.hpp>
#include <skerm/math_multiplication_token.hpp>
#include <skerm/math_division_token.hpp>
#include <skerm/math_variable_token.hpp>
#include <skerm/math_equation_token.hpp>
#include <skerm/math_struct_token.hpp>
#include <skerm/math_token_type.hpp>

#include <string>

namespace Skerm
{
	namespace ASDF
	{
		void Convert(std::string& inTarget, const StringPacked& inSource)
		{
			inTarget = inSource.Content ? inSource.Content : "";
		}

		void Convert(std::string& inTarget, const std::string& inSource)
		{
			inTarget = inSource;
		}

		void Convert(std::shared_ptr<Skerm::Token>& inTarget, const MathEquationTokenPacked::TokensPtr& inSource)
		{
			switch (inSource->Type)
			{
			case MathTokenType::MultiplicationTokenType:
				inTarget = std::make_shared<Skerm::MultiplicationToken>();
				break;

			case MathTokenType::DivisionTokenType:
				inTarget = std::make_shared<Skerm::DivisionToken>();
				break;

			case MathTokenType::AdditionTokenType:
				inTarget = std::make_shared<Skerm::AdditionToken>();
				break;

			case MathTokenType::SubtractionTokenType:
				inTarget = std::make_shared<Skerm::SubtractionToken>();
				break;

			case MathTokenType::ValueVariableTokenType:
			{
				auto pointer = std::make_shared<Skerm::ValueVariableToken>();
				Convert(*pointer, (const MathVariableTokenPacked&)*inSource.get());
				inTarget = std::move(pointer);
				break;
			}

			case MathTokenType::EquationTokenType:
			{
				auto pointer = std::make_shared<Skerm::EquationToken>();
				Convert(*pointer, (const MathEquationTokenPacked&)*inSource.get());
				inTarget = std::move(pointer);
				break;
			}

			case MathTokenType::StructTokenType:
			{
				auto pointer = std::make_shared<Skerm::StructToken>();
				Convert(*pointer, (const MathStructTokenPacked&)*inSource.get());
				inTarget = std::move(pointer);
				break;
			}

			default:
				// TODO
				__debugbreak();
				break;
			}
		}

		void Convert(Skerm::AdditionToken& inTarget, const MathAddTokenPacked& inSource) {}
		void Convert(Skerm::SubtractionToken& inTarget, const MathSubtractionTokenPacked& inSource) {}
		void Convert(Skerm::MultiplicationToken& inTarget, const MathMultiplicationTokenPacked& inSource) {}
		void Convert(Skerm::DivisionToken& inTarget, const MathDivisionTokenPacked& inSource) {}

		void Convert(Skerm::ValueVariableToken& inTarget, const MathVariableTokenPacked& inSource)
		{
			inTarget.SetValue(inSource.Value);
			inTarget.SetVar(inSource.Variable.Content ? inSource.Variable.Content : "");
		}

		void Convert(Skerm::EquationToken& inTarget, const MathEquationTokenPacked& inSource)
		{
			Skerm::Token::List Tokens;
			for (uint32_t i = 0; i < inSource.TokenCount; i++)
			{
				Skerm::Token::Ptr token;
				Convert(token, inSource.Tokens[i]);
				Tokens.push_back(token);
			}

			inTarget.SetTokens(std::move(Tokens));
		}

		void Convert(Skerm::StructToken& inTarget, const MathStructTokenPacked& inSource)
		{
			Skerm::Token::List Tokens;
			for (uint32_t i = 0; i < inSource.TokenCount; i++)
			{
				Skerm::Token::Ptr token;
				Convert(token, inSource.Tokens[i]);
				Tokens.push_back(token);
			}

			inTarget.SetTokens(std::move(Tokens));
		}

		void Convert(std::shared_ptr<UIContainer>& inTarget, const UIContainerPacked::ChildrenPtr& inSource)
		{
			switch (inSource->Type)
			{
			case UITypePacked::ContainerType:
				inTarget = std::make_shared<UIContainer>();
				Convert(*inTarget, *inSource);
				break;

			case UITypePacked::ImageType:
			{
				auto pointer = std::make_shared<UIImage>();
				const UIImagePacked* source = reinterpret_cast<const UIImagePacked*>(inSource.get());
				Convert(*pointer, *source);
				inTarget = std::move(pointer);
				break;
			}

			case UITypePacked::TextType:
			{
				auto pointer = std::make_shared<UIText>();
				const UITextPacked* source = reinterpret_cast<const UITextPacked*>(inSource.get());
				Convert(*pointer, *source);
				inTarget = std::move(pointer);
				break;
			}

			default:
				// TODO
				__debugbreak();
				break;
			}
		}
	}
}
#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <vector>
#include <string>
#include <memory>

#include <skerm/math_base_token.hpp>
#include <skerm/math_equation_token.hpp>
#include <skerm/math_struct_token.hpp>
#include <skerm/ui_container.hpp>
#include <skerm/base_types.hpp>

namespace Skerm
{
	class Token;
	class AdditionToken;
	class SubtractionToken;
	class MultiplicationToken;
	class DivisionToken;
	class ValueVariableToken;
	class EquationToken;
	class StructToken;

	namespace ASDF
	{
		struct MathAddTokenPacked;
		struct MathSubtractionTokenPacked;
		struct MathMultiplicationTokenPacked;
		struct MathDivisionTokenPacked;
		struct MathVariableTokenPacked;

		void Convert(std::string& inTarget, const StringPacked& inSource);
		void Convert(std::string& inTarget, const std::string& inSource);

		void Convert(Skerm::AdditionToken& inTarget, const MathAddTokenPacked& inSource);
		void Convert(Skerm::SubtractionToken& inTarget, const MathSubtractionTokenPacked& inSource);
		void Convert(Skerm::MultiplicationToken& inTarget, const MathMultiplicationTokenPacked& inSource);
		void Convert(Skerm::DivisionToken& inTarget, const MathDivisionTokenPacked& inSource);
		void Convert(Skerm::ValueVariableToken& inTarget, const MathVariableTokenPacked& inSource);
		void Convert(Skerm::EquationToken& inTarget, const MathEquationTokenPacked& inSource);
		void Convert(Skerm::StructToken& inTarget, const MathStructTokenPacked& inSource);
		void Convert(std::shared_ptr<Skerm::Token>& inTarget, const MathEquationTokenPacked::TokensPtr& inSource);
		void Convert(std::shared_ptr<UIContainer>& inTarget, const UIContainerPacked::ChildrenPtr& inSource);

		template<typename Element>
		void Convert(Element& a, const Element& b) { a = b; }
	}
}

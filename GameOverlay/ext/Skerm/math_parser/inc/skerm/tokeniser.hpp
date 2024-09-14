#pragma once

#include <skerm/tokens.hpp>

#include <string>
#include <vector>

namespace Skerm
{
	struct TokenizeResult
	{
		TokenizeResult(const std::vector<std::string>& inErrorList, Token::List&& inList) : Errors(inErrorList), Equation(std::move(inList)) {}
		std::vector<std::string> Errors;
		EquationToken Equation;
	};
	TokenizeResult Tokenize(const char* inData);
}

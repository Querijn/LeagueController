#include <skerm/tokeniser.hpp>

#include <string>

namespace Skerm
{
	void PushVariable(Token::List& inResult, const char* inVariable)
	{
		inResult.push_back(std::make_unique<ValueVariableToken>("1", inVariable));
	}

	void PushValue(Token::List& inResult, const char* inValue)
	{
		inResult.push_back(std::make_unique<ValueVariableToken>(inValue, ""));
	}

	enum class StateType
	{
		None,
		WritingNumber,
		WritingVariable,
		WritingStruct
	};

	TokenizeResult Skerm::Tokenize(const char* inString)
	{
		Token::List inEquation;
		std::vector<std::string> errors;
		Token::List structValues;

		StateType state = StateType::None;
		std::string value;
		std::string structElement;
		std::string textBuffer;
		bool negate = false;
		bool isMakingImplicitStruct = false;

		auto resetState = [&inEquation, &state, &value, &structElement, &textBuffer, &negate]()
		{
			if (state == StateType::WritingNumber)
				inEquation.push_back(std::make_unique<ValueVariableToken>(textBuffer.c_str(), ""));
			else if (state == StateType::WritingVariable)
				inEquation.push_back(std::make_unique<ValueVariableToken>(value.empty() == false ? value.c_str() : "1", textBuffer.c_str(), structElement.c_str()));

			state = StateType::None;
			value = "";
			textBuffer = "";
			structElement = "";
			negate = false;
		};

		for (const char* c = inString; *c != 0; c++)
		{
			if (state == StateType::WritingStruct)
			{
				const char* start = c;
				int depth = 0;
				for (; *c != 0 && *c != ',' && *c != ']'; c++)
				{
					if (*c == '(')
						depth++;
					else if (*c == ')')
						depth--;
				}

				if (c == start || depth != 0 || *c == 0)
				{
					errors.push_back("Was unable to find argument end for struct!");
					break;
				}

				std::string equationString(start, c);
				auto result = Tokenize(equationString.c_str());
				for (auto& error : result.Errors)
					errors.push_back(error);

				if (result.Errors.empty())
				{
					result.Equation.ReduceSelf();
					size_t size = result.Equation.size();
					if (size == 1)
					{
						structValues.push_back(*result.Equation.begin());
					}
					else if (size > 1)
					{
						structValues.push_back(std::make_shared<EquationToken>(std::move(result.Equation)));
					}
				}

				if (*c == ']')
				{
					inEquation.push_back(std::make_shared<StructToken>(structValues));
					structValues.clear();
					resetState();
				}
				continue;
			}

			switch (tolower(*c))
			{
			case ',': // This is only hit if we're making an implicit struct ("0,0,1,1" for instance)
			{
				// TODO: Some verification
				//			- Check for ] at the end.

				std::string newString = "[ ";
				newString += inString;
				newString += " ]";
				return Tokenize(newString.c_str());
			}

			case '.':
				// If we were writing a variable, it was a struct, 
				if (state == StateType::WritingVariable)
				{
					structElement = textBuffer;
					textBuffer = "";
					break;
				}

				// Explicit fallthrough

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9': 
				// If we were writing a variable before this number, push that first
				if (state == StateType::WritingVariable)
				{
					PushVariable(inEquation, value.c_str()); // Push the variable we were working on
					inEquation.push_back(std::make_unique<MultiplicationToken>()); // Multiply against the number we're about to write
					textBuffer = "";
				}

				if (state == StateType::None)
				{
					// If our last Token was a different number, multiply
					if (state == StateType::None && inEquation.empty() == false && inEquation.back()->IsValue())
						inEquation.push_back(std::make_unique<MultiplicationToken>());

					// Add negation state if requested
					if (negate)
					{
						negate = false;
						textBuffer += "-";
					}

					// stod doesn't allow starting with a period.
					if (*c == '.')
						textBuffer += "0";
				}

				state = StateType::WritingNumber; // Reset state.

				// Check for multiple periods in a number
				if (*c == '.')
				{
					for (auto character : textBuffer)
					{
						if (character == '.')
						{
							errors.push_back("A number cannot contain 2 periods!");
							break;
						}
					}
				}

				textBuffer += *c;
				break;

			case '%':
			{
				if (state != StateType::WritingNumber && (inEquation.empty() || inEquation.back()->IsValue() == false))
				{
					errors.push_back("Cannot use '%' on its own!");
					break;
				}

				resetState();
				auto lastVar = inEquation.back();

				// If we had a variable before the percentage sign, multiply it by 100%
				if (lastVar->GetVar().empty() == false)
				{
					inEquation.push_back(std::make_unique<MultiplicationToken>());
					inEquation.push_back(std::make_unique<ValueVariableToken>("100", "%"));
					break;
				}

				lastVar->SetVar("%");
				break;
			}

			case '/':
				resetState();
				inEquation.push_back(std::make_unique<DivisionToken>());
				break;

			case '*':
				resetState();
				inEquation.push_back(std::make_unique<MultiplicationToken>());
				break;

			case '+':
				resetState();

				// If we're prepended before a number or var, don't do anything.
				if (inEquation.empty() || // If we're the first element
					inEquation.back()->IsValue() == false)
				{
					break;
				}

				inEquation.push_back(std::make_unique<AdditionToken>());
				break;

			case '-':
				resetState();

				// Setup negation
				if (inEquation.empty() || // If we're the first element
					(inEquation.back()->IsValue() == false &&
					inEquation.back()->IsEquation() == false))
				{
					negate = !negate;
					break;
				}

				// Subtract the upcoming from our current.
				// We use this hack instead of a sub_token to trick the reducer to multiply, 
				// which allows us to negate parenthesis.
				inEquation.push_back(std::make_unique<AdditionToken>());
				inEquation.push_back(std::make_unique<ValueVariableToken>("-1", ""));
				inEquation.push_back(std::make_unique<MultiplicationToken>());
				break;

			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
			case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
			case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
			case 'v': case 'w': case 'x': case 'y': case 'z':
				// If we were writing a number before this variable, push that first
				if (state == StateType::WritingNumber)
				{
					value = textBuffer;
					textBuffer = "";
				}

				// If we were have a value of some kind before this number.
				else if (inEquation.empty() == false)
				{
					auto lastValue = inEquation.back();
					if (lastValue->IsValue())
					{
						if (lastValue->GetVar().empty())
						{
							value = lastValue->GetValueString();
							inEquation.erase(std::next(inEquation.rbegin()).base()); // Erase last
						}
						else
						{
							inEquation.push_back(std::make_unique<MultiplicationToken>());
						}
					}
				}

				state = StateType::WritingVariable; // Reset state.
				if (state == StateType::WritingVariable)
					textBuffer += *c;
				break;

			case ' ':
			case '\r':
			case '\n':
			case '\t':
				resetState();
				break;

			case ')':
				errors.push_back("Unexpected ')' character found!");
				break;

			case '(':
			{
				resetState();

				if (inEquation.empty() == false && inEquation.back()->IsValue())
				{
					inEquation.push_back(std::make_unique<MultiplicationToken>()); // Multiply against the number we're about to write
				}

				const char* start = ++c;
				int depth = 1;
				for (; depth != 0 && *c != 0; c++)
				{
					if (*c == '(')
						depth++;
					else if (*c == ')')
						depth--;
				}

				c--;
				if (c == start || depth != 0)
				{
					errors.push_back("Was unable to match opening parenthesis!");
					break;
				}

				std::string equationString(start, c);

				auto result = Tokenize(equationString.c_str());
				for (auto& error : result.Errors)
					errors.push_back(error);

				if (result.Errors.empty())
				{
					result.Equation.ReduceSelf();
					size_t size = result.Equation.size();
					if (size == 1)
					{
						inEquation.push_back(*result.Equation.begin());
					}
					else if (size > 1)
					{
						inEquation.push_back(std::make_shared<EquationToken>(std::move(result.Equation)));
					}
				}
				break;
			}

			case '[':
				resetState();
				state = StateType::WritingStruct;
				break;

			default:
				errors.push_back(std::string("Unexpected '") + (*c) + "' character found!");
				break;
			}
		}

		resetState();
		if (inEquation.empty())
		{
			PushValue(inEquation, "0");
		}

		return TokenizeResult(errors, errors.empty() ? inEquation : Token::List());
	}
}

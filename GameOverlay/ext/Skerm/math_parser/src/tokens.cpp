#include <skerm/tokens.hpp>

#include <stdexcept>
#include <cassert>

namespace Skerm
{
	double ValueVariableToken::GetValue() const
	{
		if (std::holds_alternative<double>(m_value))
			return std::get<double>(m_value);

		try
		{
			return std::stod(std::get<std::string>(m_value));
		}
		catch (...)
		{
			return 0;
		}
	}

	std::string ValueVariableToken::GetValueString() const
	{
		if (std::holds_alternative<std::string>(m_value))
			return std::get<std::string>(m_value);

		try
		{
			return std::to_string(std::get<double>(m_value));
		}
		catch (...)
		{
			return "0.0";
		}
	}

	EquationToken::EquationToken(const EquationToken& other) noexcept
	{
		m_tokens.clear();

		for (auto token : other.m_tokens)
		{
			if (token->IsAdd())
				m_tokens.push_back(std::make_shared<AdditionToken>());
			else if (token->IsSubtract())
				m_tokens.push_back(std::make_shared<SubtractionToken>());
			else if (token->IsMultiply())
				m_tokens.push_back(std::make_shared<MultiplicationToken>());
			else if (token->IsDivide())
				m_tokens.push_back(std::make_shared<DivisionToken>());
			else if (token->IsValue())
				m_tokens.push_back(std::make_shared<ValueVariableToken>(token->GetValueString().c_str(), token->GetVar().c_str()));
			else if (token->IsEquation())
				m_tokens.push_back(std::make_shared<EquationToken>(*(EquationToken*)token.get()));
			else if (token->IsStruct())
				m_tokens.push_back(std::make_shared<StructToken>(*(StructToken*)token.get()));

			else
				__debugbreak();
		}
	}

	EquationToken& EquationToken::operator=(const EquationToken& other) noexcept
	{
		m_tokens.clear();

		for (auto token : other.m_tokens)
		{
			if (token->IsAdd())
				m_tokens.push_back(std::make_shared<AdditionToken>());
			else if (token->IsSubtract())
				m_tokens.push_back(std::make_shared<SubtractionToken>());
			else if (token->IsMultiply())
				m_tokens.push_back(std::make_shared<MultiplicationToken>());
			else if (token->IsDivide())
				m_tokens.push_back(std::make_shared<DivisionToken>());
			else if (token->IsValue())
				m_tokens.push_back(std::make_shared<ValueVariableToken>(token->GetValueString().c_str(), token->GetVar().c_str()));
			else if (token->IsEquation())
				m_tokens.push_back(std::make_shared<EquationToken>(*(EquationToken*)token.get()));

			else
				__debugbreak();
		}

		return *this;
	}

	std::string EquationToken::ToString() const
	{
		std::string result = "(";
		bool first = true;
		for (auto& token : m_tokens)
		{
			if (!first)
				result += " ";
			else
				first = false;

			result += token->ToString();
		}

		return result + ")";
	}

	bool SimplifyVariable(Token::Ptr& inElement, const Token::VarMap& inVarMap)
	{
		if (inElement->IsStruct())
		{
			// If we're a struct, simplify all members
			for (auto& equation : (StructToken&)*inElement)
			{
				if (SimplifyVariable(equation, inVarMap) == false)
					return false;
			}

			return true;
		}

		// We're expecting ValueVariableTokens from here on out
		if (inElement->IsValue() == false)
			return false;

		// Is a simple value
		if (inElement->GetVar().empty())
			return true;
		
		// We're a variable, check if we have a value
		auto index = inVarMap.find(inElement->GetVar());
		if (index == inVarMap.end())
			return false;

		double first = inElement->GetValue();
		double second = 0.0;
		if (std::holds_alternative<double>(index->second))
		{
			second = std::get<double>(index->second);
		}
		else try
		{
			second = std::stod(std::get<std::string>(index->second));
		}
		catch (...) { }

		// Replace with value
		inElement = std::make_shared<ValueVariableToken>(first * second, "");
		return true;
	}
	
	enum class MathType
	{
		Exponentials,
		MultDiv,
		AddSub,
	};

	bool ApplyMathDouble(Token::Ptr inLeft, Token::Ptr inOperation, Token::Ptr inRight, MathType inType, double& inResult)
	{
		if (inOperation->IsOperation() == false)
			return false;

		double leftValue = 0;
		double rightValue = 0;
		try
		{
			leftValue = inLeft->GetValue();
		}
		catch (std::invalid_argument e)
		{
			printf("Error while parsing left value (%s), could not evaluate to double\n", inLeft->GetValueString().c_str());
			return false;
		}
		catch (std::out_of_range e)
		{
			printf("Left value ('%s') is out of range, unable to evaluate to double.\n", inLeft->GetValueString().c_str());
			return false;
		}

		try
		{
			rightValue = inRight->GetValue();
		}
		catch (std::invalid_argument e)
		{
			printf("Error while parsing right value (%s), could not evaluate to double\n", inRight->GetValueString().c_str());
			return false;
		}
		catch (std::out_of_range e)
		{
			printf("Right value ('%s') is out of range, unable to evaluate to double.\n", inRight->GetValueString().c_str());
			return false;
		}

		switch (inType)
		{
		case MathType::AddSub:
			if (inOperation->IsAdd())
			{
				inResult = leftValue + rightValue;
				return true;
			}
			else if (inOperation->IsSubtract())
			{
				inResult = leftValue - rightValue;
				return true;
			}
			break;

		case MathType::Exponentials:
			break; // TODO

		case MathType::MultDiv:
			if (inOperation->IsMultiply())
			{
				inResult = leftValue * rightValue;
				return true;
			}
			else if (inOperation->IsDivide())
			{
				inResult = leftValue / rightValue;
				return true;
			}
			break;
		}

		printf("Unable to apply operation '%s'\n", inOperation->ToString().c_str());
		return false;
	}

	bool ApplyMathStruct(Token::Ptr& inLeft, Token::Ptr inOperation, Token::Ptr inRight, MathType inType, Token::List& inResults)
	{
		auto applyOneStruct = [&inResults, inOperation, inRight, inType](StructToken* inStruct, Token::Ptr inOther, bool inStructIsFirst)
		{
			for (auto& token : *inStruct)
			{
				if (token->IsValue())
				{
					double asDouble;
					if (ApplyMathDouble(token, inOperation, inRight, inType, asDouble) == false)
						return false;

					inResults.push_back(std::make_shared<ValueVariableToken>(asDouble));
					continue;
				}
				
				__debugbreak();
				return false;
			}

			return true;
		};

		if (inLeft->IsStruct() && !inRight->IsStruct())
		{
			if (applyOneStruct((StructToken*)inLeft.get(), inRight, true) == false)
				return false;
		}
		else if (!inLeft->IsStruct() && inRight->IsStruct())
		{
			if (applyOneStruct((StructToken*)inRight.get(), inLeft, false) == false)
				return false;
		}
		else if (inLeft->IsStruct() && inRight->IsStruct())
		{
			__debugbreak(); // TODO
		}

		return true;
	}

	bool ApplyMath(Token::Ptr& inLeft, Token::Ptr inOperation, Token::Ptr inRight, MathType inType)
	{
		if (inLeft->IsStruct() || inRight->IsStruct())
		{
			Token::List results;
			if (ApplyMathStruct(inLeft, inOperation, inRight, inType, results) == false)
				return false;

			inLeft = std::make_shared<StructToken>(results);
			return true;
		}

		double result = 0;
		if (!ApplyMathDouble(inLeft, inOperation, inRight, inType, result))
			return false;

		// Make sure the variables align
		if (inRight->GetVar().empty() == false)
		{
			if (inLeft->GetVar().empty())
				inLeft->SetVar(inRight->GetVar().c_str());
			else
				assert(inRight->GetVar() == inLeft->GetVar());
		}

		inLeft->SetValue(result);
		return true;
	}

	bool TryReduce(MathType inType, Token::Ptr& inLeft, Token::Ptr inOperation, Token::Ptr inRight, Token::List inTokens, const Token::VarMap& inVarMap)
	{
		bool leftIsSimple = SimplifyVariable(inLeft, inVarMap); // This tries to exchange vars for values.
		bool rightIsSimple = SimplifyVariable(inRight, inVarMap);
		bool isSimple = leftIsSimple && rightIsSimple;
		bool bothUseSameVars = inLeft->GetVar() == inRight->GetVar(); // TODO: Make this work for structs

		bool canReduce = false;

		if (inType == MathType::MultDiv) // Either side of the operation has to be simple (4x * 3, or both use the same variable.
			canReduce = (leftIsSimple || rightIsSimple) || bothUseSameVars;

		else if (inType == MathType::AddSub) // Both sides are simple (4 + 4), or neither, but the vars are the same in that case. (4x + 3x)
			canReduce = isSimple || (!isSimple && bothUseSameVars);

		// TODO: Divide, exponentials
		if (canReduce == false || ApplyMath(inLeft, inOperation, inRight, inType) == false)
			return false;

		return true;
	}

	bool MultiplyEquations(Token::Ptr& inLeft, Token::Ptr& inOperation, Token::Ptr& inRight, Token::List inTokens, const Token::VarMap& inVarMap)
	{
		if (inLeft->IsEquation() && inRight->IsEquation())
		{
			__debugbreak(); // TODO
			EquationToken* leftEq = (EquationToken*)inLeft.get();
			EquationToken* rightEq = (EquationToken*)inRight.get();

			Token::List result;
			for (auto leftElement : *leftEq)
			{
				if (leftElement->IsOperation())
					continue;
				for (auto rightElem : *rightEq)
				{
				}
			}
			return true;
		}

		auto doMerge = [inOperation, &inTokens, inVarMap](Token::Ptr& inOutput, Token::Ptr& inLeft, EquationToken* inEquation)
		{
			Token::List result;

			auto add = [inOperation, &inLeft, &result, &inTokens, inVarMap](auto& inRightIterator)
			{
				Token::Ptr tempLeft = std::make_shared<ValueVariableToken>(*(ValueVariableToken*)inLeft.get()); // Copy
				Token::Ptr right = *inRightIterator;
				if (TryReduce(MathType::MultDiv, tempLeft, inOperation, right, inTokens, inVarMap))
				{
					result.push_back(tempLeft);
				}
				else
				{
					result.push_back(tempLeft);
					if (inOperation->IsMultiply())
						result.push_back(std::make_shared<MultiplicationToken>());
					else if (inOperation->IsDivide())
						result.push_back(std::make_shared<DivisionToken>());
					result.push_back(right);
				}
			};

			for (auto it = inEquation->begin(); it != inEquation->end(); it++)
			{
				if ((*it)->IsAdd() == false) // todo: subtract
					continue;

				add(std::prev(it));
				result.push_back(std::make_shared<AdditionToken>());
				add(std::next(it));
			}

			inOutput = std::make_shared<EquationToken>(std::move(result));
		};

		if (inLeft->IsEquation())
		{
			doMerge(inLeft, inRight, (EquationToken*)inLeft.get());
			return true;
		}
		else if (inOperation->IsDivide() == false && inRight->IsEquation())
		{
			doMerge(inLeft, inLeft, (EquationToken*)inRight.get());
			return true;
		}

		return false;
	}

	size_t CommonReduce(const Token::List& inTokens, double* inOutput, size_t inArrayCount, const Token::VarMap& inVarMap, bool inIsStruct) 
	{
		for (int i = 0; i < inArrayCount; i++)
			inOutput[i] = 0;

		enum class OperationType
		{
			Unset,
			Add,
			Subtract,
			Multiply,
			Divide
		};
		OperationType operation = OperationType::Add;

		// The first token must be the only token, as a value, without a variable appended.
		size_t size = 1;
		size_t i = 0;
		for (const auto& token : inTokens)
		{
			if (token->IsValue())
			{
				double number = token->GetValue();
				auto variable = token->GetVar();
				if (variable.empty() == false)
				{
					auto varIndex = inVarMap.find(variable);
					if (varIndex == inVarMap.end())
						return 0;

					number *= std::get<double>(varIndex->second); // TODO: I think I can safely do this right now right?
				}

				switch (operation)
				{
				default:
					__debugbreak();
				case OperationType::Unset: // Invalid
					return 0;

				case OperationType::Add:
					inOutput[i] += number;
					break;

				case OperationType::Subtract:
					inOutput[i] -= number;
					break;

				case OperationType::Multiply:
					inOutput[i] *= number;
					break;

				case OperationType::Divide:
					inOutput[i] /= number;
					break;
				}
			}
			else if (token->IsStruct())
			{
				auto* asStruct = static_cast<StructToken*>(token.get());

				size = asStruct->Calculate(inOutput, inArrayCount, inVarMap);
				if (size == 0)
					return 0;
			}
			else if (token->IsAdd())
			{
				operation = OperationType::Add;
			}
			else if (token->IsSubtract())
			{
				operation = OperationType::Subtract;
			}
			else if (token->IsMultiply())
			{
				operation = OperationType::Multiply;
			}
			else if (token->IsDivide())
			{
				operation = OperationType::Divide;
			}
			else
			{
				__debugbreak();
			}

			if (inIsStruct)
				i++;
		}

		return inIsStruct ? i : size;
	}

	size_t EquationToken::Calculate(double* inOutput, size_t inArrayCount, const Token::VarMap& inVarMap) const
	{
		return CommonReduce(m_tokens, inOutput, inArrayCount, inVarMap, false);
	}

	void EquationToken::ReduceSelf(const Token::VarMap& inVarMap)
	{
		// Reduce parenthesis, reduce variables
		for (auto& token : m_tokens)
		{
			if (token->IsEquation())
				((EquationToken*)token.get())->ReduceSelf();
			else if (token->IsValue())
				SimplifyVariable(token, inVarMap);
		}

		// The inner reduce loop for Exponentials, Mult/Div, Add/Sub.
		auto reduceInner = [this, inVarMap](MathType inType) -> bool
		{
			if (m_tokens.empty())
				return true;

			bool fullyReduced = true;
			for (auto it = std::next(m_tokens.begin()); it != m_tokens.end(); it++)
			{
				Token::Ptr token = *it;

				// If the current token isn't the math operation we're looking for, skip
				switch (inType)
				{
				case MathType::MultDiv:
					if (!token->IsMultiply() && !token->IsDivide())
						continue;
					break;
				case MathType::AddSub:
					if (!token->IsAdd() && !token->IsSubtract())
						continue;
					break;

				default:
					assert(false);
					break;
				}

				auto leftIterator = std::prev(it);
				auto rightIterator = std::next(it);
				if (rightIterator == m_tokens.end())
				{
					m_tokens.erase(it);
					break; // TODO: Error
				}

				Token::Ptr& left = *leftIterator;
				Token::Ptr& right = *rightIterator;

				// This would mean it's broken
				assert(!left->IsOperation() && !right->IsOperation());

				// printf("Reducing '%s %s %s'\n", left->ToString().c_str(), Token->ToString().c_str(), right->ToString().c_str());

				// If either one is a multiplication, we need to multiply / divide with all members of the equation
				if (inType != MathType::MultDiv && (left->IsEquation() || right->IsEquation()))
				{
					fullyReduced = false;
					continue;
				}

				if (left->IsEquation() || right->IsEquation())
				{
					if (!MultiplyEquations(left, token, right, m_tokens, inVarMap))
					{
						fullyReduced = false;
						continue;
					}
				}
				else if (!TryReduce(inType, left, token, right, m_tokens, inVarMap))
				{
					fullyReduced = false;
					continue;
				}

				m_tokens.erase(it);
				m_tokens.erase(rightIterator);
				it = m_tokens.begin();
			}

			return fullyReduced;
		};

		// reduce_inner(math_type::exponentials);
		if (reduceInner(MathType::MultDiv))
		{
			// Remove all equations, put them into the main one.
			for (auto it = m_tokens.begin(); it != m_tokens.end(); it++)
			{
				if ((*it)->IsEquation() == false)
					continue;

				EquationToken& equation = *(EquationToken*)(it->get());
				for (auto token : equation)
					m_tokens.insert(it, token);
				m_tokens.erase(it);
				it = m_tokens.begin();
			}
		}

		reduceInner(MathType::AddSub);

		bool canReduceFurther = true;
		std::map<std::string, double> sum;
		{
			bool subToken = false;
			for (auto& token : m_tokens)
			{
				if (token->IsMultiply())
				{
					canReduceFurther = false;
					break;
				}

				if (token->IsSubtract())
				{
					subToken = true;
					continue;
				}

				// Expected non-value
				if (token->IsAdd())
					continue;

				// Get every value
				if (token->IsValue() == false)
				{
					// printf("Cannot reduce further: Found unexpected Token '%s'\n", tok->ToString().c_str());
					canReduceFurther = false;
					break;
				}

				double number = token->GetValue();
				if (subToken)
					sum[token->GetVar()] -= number;
				else
					sum[token->GetVar()] += number;
				subToken = false;
			}
		}

		if (canReduceFurther == false)
			return;

		/*std::string old;
		for (auto& tok : m_tokens)
			old += tok->ToString() + " ";
		printf("Before full reduction => %s\n", old.c_str());*/

		Token::List finalTokens;
		bool first = true;
		for (auto& value : sum)
		{
			double number = value.second;
			if (first)
			{
				first = false;
			}
			else if (value.second < 0.0) // We have a negative number, meaning we have to subtract
			{
				finalTokens.push_back(std::make_shared<SubtractionToken>());
				number = abs(number);
			}
			else
			{
				finalTokens.push_back(std::make_shared<AdditionToken>());
			}

			finalTokens.push_back(std::make_shared<ValueVariableToken>(std::to_string(number).c_str(), value.first.c_str()));
		}

		SetTokens(std::move(finalTokens));
	}

	void EquationToken::SetTokens(Token::List&& inTokens)
	{
		m_tokens = std::move(inTokens);
	}

	void StructToken::SetTokens(Token::List&& inTokens)
	{
		m_values = std::move(inTokens);
	}
	
	std::string StructToken::ToString() const
	{
		std::string result = "[ ";

		bool first = true;
		for (auto& entry : m_values)
		{
			if (first == false)
				result += ", ";
			else
				first = false;

			result += entry->ToString();
		}

		return result + " ]";
	}
	
	size_t StructToken::Calculate(double* inOutput, size_t inArrayCount, const Token::VarMap& inVarMap) const
	{
		return CommonReduce(m_values, inOutput, inArrayCount, inVarMap, true);
	}
}

#pragma once

#include <list>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <variant>

namespace Skerm
{
	using Structlike = std::map<std::string, std::variant<std::string, double>>;
	using ValueVariant = std::variant<std::string, double, Structlike>;

	class Token
	{
	public:
		using Ptr = std::shared_ptr<Token>;
		using List = std::list<Token::Ptr>;
		using VarMap = std::unordered_map<std::string, ValueVariant>;

		virtual bool IsOperation() const { return false; }
		virtual bool IsMultiply() const { return false; }
		virtual bool IsDivide() const { return false; }
		virtual bool IsAdd() const { return false; }
		virtual bool IsSubtract() const { return false; }
		virtual bool IsValue() const { return false; }
		virtual bool IsEquation() const { return false; }
		virtual bool IsStruct() const { return false; }

		virtual void SetValue(double value) {}
		virtual void SetValue(const char* value) {}
		virtual void SetVar(const char* var) {}
		virtual bool HasParent() const { return false; }

		virtual std::string ToString() const = 0;
		virtual double		GetValue() const { return 0; }
		virtual std::string GetValueString() const { return "####"; }
		virtual std::string GetVar() const { return "####"; }
		virtual std::string GetParent() const { return "####"; }
	};

	class MultiplicationToken : public Token
	{
	public:
		bool IsOperation() const override { return true; }
		bool IsMultiply() const override { return true; }
		std::string ToString() const override { return "*"; }
	};

	class DivisionToken : public Token
	{
	public:
		bool IsOperation() const override { return true; }
		bool IsDivide() const override { return true; }
		std::string ToString() const override { return "/"; }
	};

	class AdditionToken : public Token
	{
	public:
		bool IsOperation() const override { return true; }
		bool IsAdd() const override { return true; }
		std::string ToString() const override { return "+"; }
	};

	class SubtractionToken : public Token
	{
	public:
		bool IsOperation() const override { return true; }
		bool IsSubtract() const override { return true; }
		std::string ToString() const override { return "-"; }
	};

	class ValueVariableToken : public Token
	{
	public:
		ValueVariableToken(const char* inValue = "0.0", const char* inVariable = "", const char* inParentStruct = "") : m_value(inValue), m_variable(inVariable), m_parent(inParentStruct) { }
		ValueVariableToken(double inValue, const char* inVariable = "", const char* inParentStruct = "") : m_value(inValue), m_variable(inVariable) { }
		bool IsValue() const override { return true; }

		const char* GetName() const { return m_variable.c_str(); }
		std::string ToString() const override 
		{ 
			if (m_parent.empty())
				return GetValueString() + m_variable;
			else
				return m_parent + "." + m_variable;
		}

		double		GetValue() const override;
		std::string GetValueString() const override;
		std::string GetVar() const override { return m_variable; }

		bool		HasParent() const override { return m_parent.empty(); }
		std::string GetParent() const override { return m_parent; }

		void SetValue(double value) override { m_value = value; }
		void SetValue(const char* value) override { m_value = value; }
		void SetVar(const char* var) override { m_variable = var; }
	private:
		ValueVariant m_value;
		std::string m_variable;
		std::string m_parent;
	};

	class EquationToken : public Token
	{
	public:
		EquationToken() {}
		EquationToken(Token::List&& inTokens) : m_tokens(std::move(inTokens)) {}
		EquationToken(const EquationToken&& inOther) noexcept : m_tokens(std::move(inOther.m_tokens)) {}
		EquationToken(const EquationToken& inOther) noexcept;
		EquationToken& operator=(const EquationToken& inOther) noexcept;

		bool IsEquation() const override { return true; }
		std::string ToString() const override;

		Token::List::const_iterator begin() const { return m_tokens.begin(); }
		Token::List::const_iterator end() const { return m_tokens.end(); }
		Token::List::const_reference back() const { return m_tokens.back(); }
		size_t size() const { return m_tokens.size(); }

		size_t Calculate(double* inOutput, size_t inArrayCount, const Token::VarMap& inVarMap = Token::VarMap()) const;
		void ReduceSelf(const Token::VarMap& inVarMap = Token::VarMap());
		void SetTokens(Token::List&& inTokens);

	private:
		Token::List m_tokens;
	};

	class StructToken : public Token
	{
	public:
		StructToken() { }
		StructToken(Token::List& inList) : m_values(inList) { }
		bool IsStruct() const override { return true; }

		std::string ToString() const override;
		size_t Calculate(double* inOutput, size_t inArrayCount, const Token::VarMap& inVarMap = Token::VarMap()) const;

		Token::List::const_iterator begin() const { return m_values.begin(); }
		Token::List::const_iterator end() const { return m_values.end(); }
		Token::List::const_reference back() const { return m_values.back(); }

		Token::List::iterator begin() { return m_values.begin(); }
		Token::List::iterator end() { return m_values.end(); }
		Token::List::reference back() { return m_values.back(); }

		void SetTokens(Token::List&& inTokens);

	private:
		Token::List m_values;
	};
}

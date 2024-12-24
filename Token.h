#pragma once

#include <memory>
#include <string>
#include <istream>

struct TokenBody;

class Token
{
public:
	enum class Type {
		UNDEFINED,

		// type
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_I8,
		TYPE_I16,
		TYPE_I32,
		TYPE_I64,
		TYPE_U8,
		TYPE_U16,
		TYPE_U32,
		TYPE_U64,
		TYPE_F32,
		TYPE_F64,
		
		// constant
		CONSTANT_BOOL,
		CONSTANT_INTEGER,
		CONSTANT_FLOAT,
		CONSTANT_STRING,

		// operator
		PARENTHESIS_LEFT,
		PARENTHESIS_RIGHT,
		SQUARE_BRACKET_LEFT,
		SQUARE_BRACKET_RIGHT,
		DOT,
		ASTERISK,
		SLASH,
		PERCENT,
		PLUS,
		MINUS,
		// shift left,right
		COMPARE_LESSER_THAN,
		COMPARE_LESSER_EQUAL,
		COMPARE_GREATER_THAN,
		COMPARE_GREATER_EQUAL,
		COMPARE_EQUAL,
		COMPARE_NOT_EQUAL,
		// bit and,xor,or
		LOGICAL_OR,
		LOGICAL_AND,
		ASSIGN_EQUAL,
		START_OPERATOR,

		// key word
		STRUCT,
		EXTERN,
		FUNCTION,
		RETURN,
		LET,
		NEW,
		IF,
		ELSE,
		WHILE,
		BREAK,

		// other
		CURLY_BRACKET_LEFT,
		CURLY_BRACKET_RIGHT,
		COMMA,
		SEMICOLON,
		TRIPLE_DOT,
		AMPERSAND,
		SYMBOL,

		END_OF_FILE,
	};

	explicit Token(Type type = Type::UNDEFINED, const std::string& str = "", const std::string& filepath = "", size_t line = 0, size_t column = 0);
	Token(const Token& other);
	Token(Token&& other) noexcept;
	virtual ~Token();
	Token& operator=(const Token& other);
	Token& operator=(Token&& other) noexcept;

	Type getType() const;
	const std::string& getString() const;
	const std::string& getFilepath() const;
	size_t getLine() const;
	size_t getColumn() const;
	bool isType() const;
	bool isEqualOrNotEqualOperator() const;
	bool isGreaterOrLesserOperator() const;
	bool isPlusOrMinusOperator() const;
	bool isMulDivModOperator() const;
	bool isConstant() const;
	bool isIntegerType() const;
	int getPriority() const;
	bool isOperator(Token::Type);

	static bool isType(Token::Type);
	static bool isEqualOrNotEqualOperator(Token::Type);
	static bool isGreaterOrLesserOperator(Token::Type);
	static bool isPlusOrMinusOperator(Token::Type);
	static bool isMulDivModOperator(Token::Type);
	static bool isConstant(Token::Type);
	static bool isBool(Token::Type);
	static bool isIntegerType(Token::Type);
	static bool isSignedIntegerType(Token::Type);
	static bool isUnsignedIntegerType(Token::Type);
	static bool isFloatingPointType(Token::Type);
	
private:

	std::shared_ptr<TokenBody> body_;
};

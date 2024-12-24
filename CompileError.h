#pragma once

#include "Token.h"
#include "Generator.h"

class CompileError
{
public:
	CompileError(const Token& token) : token_(token) {}
	virtual ~CompileError() = default;

	virtual const char* getErrorName() const = 0;

	virtual const Token& getToken() const {
		return token_;
	}

	void printErrorMessage(std::ostream& out) const {
		out << getToken().getFilepath() << "\t" << getToken().getLine() << "\t" << getToken().getColumn() << "\t" << getErrorName();
	}

private:
	Token token_;
};

class UnexpectedTokenError : public CompileError {
private:
	class Expected {
	public:
		virtual ~Expected() = default;
	};

public:
	class ExpectedToken : public Expected {
	public:
		ExpectedToken(Token::Type type) : type_(type) {}

	private:
		Token::Type type_;
	};

	class ExpectedTypeToken : public Expected {
	};

	UnexpectedTokenError(const Token& errorToken) : CompileError(errorToken), expected_(nullptr) {}
	~UnexpectedTokenError() = default;

	const char* getErrorName() const {
		return "UnexpectedToken";
	}

	void setExpected(const std::shared_ptr<Expected>& expected) {
		expected_ = expected;
	}

private:
	std::shared_ptr<Expected> expected_;
};

class IllegalFileFormatError : public CompileError {
public:
	IllegalFileFormatError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "IllegalFileFormat";
	}
};

class UnexpectedEofError : public CompileError {
public:
	UnexpectedEofError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "UnexpectedEof";
	}
};

class UnexpectedCharactorError : public CompileError {
public:
	UnexpectedCharactorError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "UnexpectedCharactor";
	}
};

class OperandTypesMismatchError : public CompileError {
public:
	OperandTypesMismatchError(const Token& operatorToken, const ValueType& lhsType, const ValueType& rhsType)
		: CompileError(operatorToken), operatorToken_(operatorToken), rhsType_(rhsType), lhsType_(lhsType) {}

	const char* getErrorName() const {
		return "OperandTypeMismatch";
	}

private:
	Token operatorToken_;
	ValueType lhsType_;
	ValueType rhsType_;
};

class TypeMismatchError : public CompileError {
public:
	TypeMismatchError(const Token& operatorToken, const ValueType& expected, const ValueType& actual)
		: CompileError(operatorToken), operatorToken_(operatorToken), expected_(expected), actual_(actual) {}
	TypeMismatchError(const Token& operatorToken, Token::Type expected, const ValueType& actual)
		: CompileError(operatorToken), operatorToken_(operatorToken), expected_(expected), actual_(actual) {}

	const char* getErrorName() const {
		return "TypeMismatch";
	}

private:
	Token operatorToken_;
	ValueType expected_;
	ValueType actual_;
};

class NotArithmeticTypeError : public CompileError {
public:
	NotArithmeticTypeError(const Token& operatorToken, const ValueType& actual)
		: CompileError(operatorToken), operatorToken_(operatorToken), actual_(actual) {}

	const char* getErrorName() const {
		return "NotArithmeticType";
	}

private:
	Token operatorToken_;
	ValueType actual_;
};

class NotComparableTypeError : public CompileError {
public:
	NotComparableTypeError(const Token& operatorToken, const ValueType& actual)
		: CompileError(operatorToken), operatorToken_(operatorToken), actual_(actual) {}

	const char* getErrorName() const {
		return "NotComparableType";
	}

private:
	Token operatorToken_;
	ValueType actual_;
};

class NotBeAbleToEqualTypeError : public CompileError {
public:
	NotBeAbleToEqualTypeError(const Token& operatorToken, const ValueType& actual)
		: CompileError(operatorToken), operatorToken_(operatorToken), actual_(actual) {}

	const char* getErrorName() const {
		return "NotBeAbleToEqualType";
	}

private:
	Token operatorToken_;
	ValueType actual_;
};

class UndefinedSymbolError : public CompileError {
public:
	UndefinedSymbolError(const Token& token) : CompileError(token), token_(token) {}

	const char* getErrorName() const {
		return "UndefinedSymbol";
	}

private:
	Token token_;
};

class TypeOrInitializerMustBeSpecifiedError : public CompileError {
public:
	TypeOrInitializerMustBeSpecifiedError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "TypeOrInitilizerMustBeSpecified";
	}

};

class ConstantTooLarge : public CompileError {
public:
	ConstantTooLarge(const Token& operatorToken) : CompileError(operatorToken) {}
	const char* getErrorName() const {
		return "ConstantTooLarge";
	}
};

class InvalidCallArgumentLength : public CompileError {
public:
	InvalidCallArgumentLength(const Token& callToken, const Token& functionNameToken) : CompileError(callToken), functionName_(functionNameToken) {}

	const char* getErrorName() const {
		return "InvalidCallArgumentLength";
	}

private:
	Token functionName_;
};

class CanNotOverwriteArgumentError : public CompileError {
public:
	CanNotOverwriteArgumentError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "CanNotOverwriteArgument";
	}
};

class InvalidBreakError : public CompileError {
public:
	InvalidBreakError(const Token& breakToken) : CompileError(breakToken) {}

	const char* getErrorName() const {
		return "InvalidBreak";
	}
};

class CanNotGiveInstructionAfterBreakOrReturn : public CompileError {
public:
	CanNotGiveInstructionAfterBreakOrReturn(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "CanNodeGiveInstructionAfterBreakOrReturn";
	}
};

class MissingReturnError : public CompileError {
public:
	MissingReturnError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "MissingReturn";
	}
};

class InvalidExternTypeError : public CompileError {
public:
	InvalidExternTypeError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "InvalidExternType";
	}
};

class InvalidReferenceTypeError : public CompileError {
public:
	InvalidReferenceTypeError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "InvalidReferenceTypeError";
	}
};

class ArgumentCanNotBeVoidTypeError : public CompileError {
public:
	ArgumentCanNotBeVoidTypeError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "ArgumentCanNotBeVoidType";
	}
};

class ArraySizeMustBeConstantIntegerError : public CompileError {
public:
	ArraySizeMustBeConstantIntegerError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "ArraySizeMustBeConstantInteger";
	}
};

class EachElementMustHaveIdenticallyTypeError : public CompileError {
public:
	EachElementMustHaveIdenticallyTypeError(const Token& token) : CompileError(token) {}

	const char* getErrorName() const {
		return "EachElementMustHaveIdenticallyType";
	}
};

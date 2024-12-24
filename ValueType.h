#pragma once
#include <vector>
#include "Token.h"

struct ValueType {
	Token::Type basicType;
	size_t pointerCount;
	bool isReference;
	bool isArgument;
	std::vector<size_t> arraySizes;

	ValueType();
	explicit ValueType(Token::Type type);
	ValueType(Token::Type type, size_t pointerCount, bool isReference);
	bool operator==(const ValueType& other) const;
	bool operator!=(const ValueType& other) const;
	bool isArithmetic() const;
	bool isComparable() const;
	bool isAbleToEqual() const;
	bool isBool() const;
	bool isString() const;
	bool isCompatible(const ValueType& other) const;
};

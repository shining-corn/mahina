#include "ValueType.h"

ValueType::ValueType() : basicType(Token::Type::UNDEFINED), pointerCount(0), isReference(false), isArgument(false) {}
ValueType::ValueType(Token::Type type) : basicType(type), pointerCount(0), isReference(false), isArgument(false) {}
ValueType::ValueType(Token::Type type, size_t pointerCount, bool isReference) : basicType(type), pointerCount(pointerCount), isReference(isReference), isArgument(false) {}

bool ValueType::operator==(const ValueType& other) const {
	if (basicType != other.basicType) {
		return false;
	}
	if (pointerCount != other.pointerCount) {
		return false;
	}
	if (isReference != other.isReference) {
		return false;
	}
	if (arraySizes != other.arraySizes) {
		return false;
	}
	// ignore isArgument
	return  true;
}

bool ValueType::operator!=(const ValueType& other) const {
	return !operator==(other);
}

bool ValueType::isArithmetic() const {
	if (pointerCount != 0) {
		return false;
	}
	return Token::isIntegerType(basicType) ||
		Token::isFloatingPointType(basicType) ||
		basicType == Token::Type::CONSTANT_INTEGER ||
		basicType == Token::Type::CONSTANT_FLOAT;
}

bool ValueType::isComparable() const {
	return isArithmetic();
}

bool ValueType::isAbleToEqual() const {
	return isArithmetic() || isBool();
}

bool ValueType::isBool() const {
	if (pointerCount != 0) {
		return false;
	}
	return Token::isBool(basicType);
}

bool ValueType::isString() const {
	return (basicType == Token::Type::TYPE_I8) && (pointerCount == 1);
}

bool ValueType::isCompatible(const ValueType& other) const {
	if (*this == other) {
		return true;
	}

	if (pointerCount != 0) {
		return false;
	}
	if (arraySizes != other.arraySizes) {
		return false;
	}

	// TODO: support reference type

	if ((basicType == Token::Type::CONSTANT_INTEGER) && (Token::isIntegerType(other.basicType))) {
		return true;
	}
	if ((other.basicType == Token::Type::CONSTANT_INTEGER) && (Token::isIntegerType(basicType))) {
		return true;
	}
	if ((basicType == Token::Type::CONSTANT_FLOAT) && (Token::isFloatingPointType(other.basicType))) {
		return true;
	}
	if ((other.basicType == Token::Type::CONSTANT_FLOAT) && (Token::isFloatingPointType(basicType))) {
		return true;
	}
	if ((basicType == Token::Type::CONSTANT_BOOL) && (Token::isBool(other.basicType))) {
		return true;
	}
	if ((other.basicType == Token::Type::CONSTANT_BOOL) && (Token::isBool(basicType))) {
		return true;
	}
	if ((basicType == Token::Type::CONSTANT_STRING) && other.isString()) {
		return true;
	}
	if ((other.basicType == Token::Type::CONSTANT_STRING) && this->isString()) {
		return true;
	}
	return false;
}

#include "Token.h"

static struct TokenBody {
	Token::Type type;
	std::string str;
	std::string filepath;
	size_t line;
	size_t column;

	TokenBody() : type(Token::Type::UNDEFINED), line(0), column(0) {}
	TokenBody(const TokenBody& other) : type(other.type), str(other.str), filepath(other.filepath), line(other.line), column(other.column) {}
	TokenBody(Token::Type type, const std::string& str, const std::string& filepath, size_t line, size_t column) : type(type), str(str), filepath(filepath), line(line), column(column) {}
};

Token::Token(Token::Type type, const std::string& str, const std::string& filepath, size_t line, size_t column) {
	body_ = std::make_shared<TokenBody>(type, str, filepath, line, column);
}

Token::Token(const Token& other) {
	if (other.body_ != nullptr) {
		body_ = std::make_shared<TokenBody>(*other.body_);
	}
	else {
		body_ = std::make_shared<TokenBody>();
	}
}

Token::Token(Token&& other) noexcept {
	body_.swap(other.body_);
}

Token::~Token() {}

Token& Token::operator=(const Token& other) {
	*body_ = *other.body_;
	return *this;
}

Token& Token::operator=(Token&& other) noexcept {
	body_.swap(other.body_);
	return *this;
}

Token::Type Token::getType() const {
	return body_->type;
}

const std::string& Token::getString() const {
	return body_->str;
}

const std::string& Token::getFilepath() const {
	return body_->filepath;
}

size_t Token::getLine() const {
	return body_->line;
}

size_t Token::getColumn() const {
	return body_->column;
}

bool Token::isType() const {
	return isType(body_->type);
}

bool Token::isEqualOrNotEqualOperator() const {
	return isEqualOrNotEqualOperator(body_->type);
}

bool Token::isGreaterOrLesserOperator() const {
	return isGreaterOrLesserOperator(body_->type);
}

bool Token::isPlusOrMinusOperator() const {
	return isPlusOrMinusOperator(body_->type);
}

bool Token::isMulDivModOperator() const {
	return isMulDivModOperator(body_->type);
}

bool Token::isConstant() const {
	return isConstant(body_->type);
}

bool Token::isIntegerType() const {
	return isIntegerType(body_->type);
}

int Token::getPriority() const {
	switch (body_->type) {
	case Type::ASTERISK:
	case Type::SLASH:
	case Type::PERCENT:
		return 9;
	case Type::PLUS:
	case Type::MINUS:
		return 8;
	case Type::COMPARE_GREATER_EQUAL:
	case Type::COMPARE_GREATER_THAN:
	case Type::COMPARE_LESSER_EQUAL:
	case Type::COMPARE_LESSER_THAN:
		return 6;
	case Type::COMPARE_EQUAL:
	case Type::COMPARE_NOT_EQUAL:
		return 5;
	case Type::LOGICAL_AND:
		return 2;
	case Type::LOGICAL_OR:
		return 1;
	default:
		return 0;
	}
}

bool Token::isOperator(Token::Type type) {
	return getPriority() != 0;
}

bool Token::isType(Token::Type type) {
	return (Token::Type::TYPE_VOID <= type) && (type <= Token::Type::TYPE_F64);
}

bool Token::isEqualOrNotEqualOperator(Token::Type type) {
	return (type == Token::Type::COMPARE_EQUAL) || (type == Token::Type::COMPARE_NOT_EQUAL);
}

bool Token::isGreaterOrLesserOperator(Token::Type type) {
	return (type == Token::Type::COMPARE_GREATER_EQUAL) ||
		(type == Token::Type::COMPARE_GREATER_THAN) ||
		(type == Token::Type::COMPARE_LESSER_EQUAL) ||
		(type == Token::Type::COMPARE_LESSER_THAN);
}

bool Token::isPlusOrMinusOperator(Token::Type type) {
	return (type == Token::Type::PLUS) ||
		(type == Token::Type::MINUS);
}

bool Token::isMulDivModOperator(Token::Type type) {
	return (type == Token::Type::ASTERISK) ||
		(type == Token::Type::SLASH) ||
		(type == Token::Type::PERCENT);
}

bool Token::isConstant(Token::Type type) {
	return (Type::CONSTANT_BOOL <= type) && (type <= Type::CONSTANT_STRING);
}

bool Token::isBool(Token::Type type) {
	return (type == Token::Type::TYPE_BOOL) || (type == Token::Type::CONSTANT_BOOL);
}

bool Token::isIntegerType(Token::Type type) {
	return isSignedIntegerType(type) || isUnsignedIntegerType(type);
}

bool Token::isSignedIntegerType(Token::Type type) {
	return (Type::TYPE_I8 <= type) && (type <= Type::TYPE_I64);
}

bool Token::isUnsignedIntegerType(Token::Type type) {
	return (Type::TYPE_U8 <= type) && (type <= Type::TYPE_U64);
}

bool Token::isFloatingPointType(Token::Type type) {
	return (type == Type::TYPE_F32) || (type == Type::TYPE_F64);
}

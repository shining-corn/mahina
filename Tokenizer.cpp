#include "Tokenizer.h"

//
// keyword
//

struct Keyword {
	std::string keyword;
	Token::Type type;
};

static const Keyword kTypeVoid = { "void", Token::Type::TYPE_VOID };
static const Keyword kTypeBool = { "bool", Token::Type::TYPE_BOOL };
static const Keyword kTypeI8 = { "i8", Token::Type::TYPE_I8 };
static const Keyword kTypeI16 = { "i16", Token::Type::TYPE_I16 };
static const Keyword kTypeI32 = { "i32", Token::Type::TYPE_I32 };
static const Keyword kTypeI64 = { "i64", Token::Type::TYPE_I64 };
static const Keyword kTypeU8 = { "u8", Token::Type::TYPE_U8 };
static const Keyword kTypeU16 = { "u16", Token::Type::TYPE_U16 };
static const Keyword kTypeU32 = { "u32", Token::Type::TYPE_U32 };
static const Keyword kTypeU64 = { "u64", Token::Type::TYPE_U64 };
static const Keyword kTypeF32 = { "f32", Token::Type::TYPE_F32 };
static const Keyword kTypeF64 = { "f64", Token::Type::TYPE_F64 };
static const Keyword kStruct = { "struct", Token::Type::STRUCT };
static const Keyword kExtern = { "extern", Token::Type::EXTERN };
static const Keyword kFunction = { "fn", Token::Type::FUNCTION };
static const Keyword kReturn = { "return", Token::Type::RETURN };
static const Keyword kLet = { "let", Token::Type::LET };
static const Keyword kNew = { "new", Token::Type::NEW };
static const Keyword kIf = { "if", Token::Type::IF };
static const Keyword kElse = { "else", Token::Type::ELSE };
static const Keyword kWhile = { "while", Token::Type::WHILE };
static const Keyword kBreak = { "break", Token::Type::BREAK };
static const Keyword kLiteralTrue = { "true", Token::Type::CONSTANT_BOOL };
static const Keyword kLiteralFalse = { "false", Token::Type::CONSTANT_BOOL };

static const std::vector<Keyword> kKeywords = {
	kTypeVoid, kTypeBool, kTypeI8, kTypeI16, kTypeI32, kTypeI64, kTypeU8, kTypeU16, kTypeU32, kTypeU64, kTypeF32, kTypeF64,
	kStruct, kExtern, kFunction, kReturn, kLet, kNew, kIf, kElse, kWhile, kBreak, kLiteralTrue, kLiteralFalse,
};

bool Tokenizer::initialize(std::shared_ptr<CompileError>& error) {
	c_ = src_.get();

	// read UTF-8 BOM
	if (c_ == 0xEF) {
		c_ = src_.get();
		if (c_ != 0xBB) {
			error = std::make_shared<IllegalFileFormatError>(makeToken(Token::Type::UNDEFINED, ""));
			return true;	// invalid BOM
		}

		c_ = src_.get();
		if (c_ != 0xBF) {
			error = std::make_shared<IllegalFileFormatError>(makeToken(Token::Type::UNDEFINED, ""));
			return true;	// invalid BOM
		}

		c_ = src_.get();
	}

	return false;
}

bool Tokenizer::getToken(Token& result, std::shared_ptr<CompileError>& error) {
	switch (c_) {
	case EOF:
		result = makeToken(Token::Type::END_OF_FILE, "");
		break;
	case '\r':
	case '\n':
		readNewLine();
		return getToken(result, error);
	case '(':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::PARENTHESIS_LEFT, "(");
		break;
	case ')':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::PARENTHESIS_RIGHT, ")");
		break;
	case '{':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::CURLY_BRACKET_LEFT, "{");
		break;
	case '}':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::CURLY_BRACKET_RIGHT, "}");
		break;
	case '[':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::SQUARE_BRACKET_LEFT, "[");
		break;
	case ']':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::SQUARE_BRACKET_RIGHT, "]");
		break;
	case ',':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::COMMA, ",");
		break;
	case ';':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::SEMICOLON, ";");
		break;
	case '+':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::PLUS, "+");
		break;
	case '-':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::MINUS, "-");
		break;
	case '*':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::ASTERISK, "*");
		break;
	case '/':
		c_ = src_.get();
		column_++;
		if (c_ == '/') {
			c_ = src_.get();
			column_++;
			while ((c_ != '\r') && (c_ != '\n') && (c_ != EOF)) {
				c_ = src_.get();
				column_++;
			}
			previousLine_ = line_;
			previousColumn_ = column_;
			return getToken(result, error);
		}
		else if (c_ == '*') {
			c_ = src_.get();
			column_++;
			for (;;) {
				switch (c_) {
				case EOF:
					error = std::make_shared<UnexpectedEofError>(makeToken(Token::Type::END_OF_FILE, ""));
					return true;
				case '\r':
				case '\n':
					readNewLine();
					break;
				case '*':
					c_ = src_.get();
					column_++;
					if (c_ == '/') {
						c_ = src_.get();
						column_++;
						previousLine_ = line_;
						previousColumn_ = column_;
						return getToken(result, error);
					}
					break;
				default:
					c_ = src_.get();
					column_++;
					break;
				}
			}
		}
		else {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::SLASH, "/");
		}
		break;
	case '%':
		c_ = src_.get();
		column_++;
		result = makeToken(Token::Type::PERCENT, "%");
		break;
	case '=':
		c_ = src_.get();
		column_++;
		if (c_ == '=') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::COMPARE_EQUAL, "==");
		}
		else {
			result = makeToken(Token::Type::ASSIGN_EQUAL, "=");
		}
		break;
	case '!':
		c_ = src_.get();
		column_++;
		if (c_ == '=') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::COMPARE_NOT_EQUAL, "!=");
		}
		else {
			std::string buffer;
			buffer += c_;
			error = std::make_shared<UnexpectedCharactorError>(makeToken(Token::Type::UNDEFINED, buffer));
			return true;	// TODO: not
		}
		break;
	case '|':
		c_ = src_.get();
		column_++;
		if (c_ == '|') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::LOGICAL_OR, "||");
		}
		else {
			std::string buffer;
			buffer += c_;
			error = std::make_shared<UnexpectedCharactorError>(makeToken(Token::Type::UNDEFINED, buffer));
			return true;	// TODO: bit or
		}
		break;
	case '&':
		c_ = src_.get();
		column_++;
		if (c_ == '&') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::LOGICAL_AND, "&&");
		}
		else {
			result = makeToken(Token::Type::AMPERSAND, "&");
		}
		break;
	case '<':
		c_ = src_.get();
		column_++;
		if (c_ == '=') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::COMPARE_LESSER_EQUAL, "<=");
		}
		else {
			result = makeToken(Token::Type::COMPARE_LESSER_THAN, "<");
		}
		break;
	case '>':
		c_ = src_.get();
		column_++;
		if (c_ == '=') {
			c_ = src_.get();
			column_++;
			result = makeToken(Token::Type::COMPARE_GREATER_EQUAL, ">=");
		}
		else {
			result = makeToken(Token::Type::COMPARE_GREATER_THAN, ">");
		}
		break;
	case '.':
		c_ = src_.get();
		column_++;
		if (c_ == '.') {
			c_ = src_.get();
			column_++;
			if (c_ == '.') {
				c_ = src_.get();
				column_++;
				result = makeToken(Token::Type::TRIPLE_DOT, "...");
			}
			else {
				std::string buffer;
				buffer += c_;
				error = std::make_shared<UnexpectedCharactorError>(makeToken(Token::Type::UNDEFINED, buffer));
				return true;
			}
		}
		else {
			result = makeToken(Token::Type::DOT, ".");
		}
		break;
	case '"':
		return getStringLiteralToken(result, error);
	default:
		return getOtherToken(result, error);
	}

	return false;
}

bool Tokenizer::getStringLiteralToken(Token& result, std::shared_ptr<CompileError>& error) {
	c_ = src_.get();
	column_++;

	std::string buffer;
	while (c_ != '"') {
		if (c_ == '\\') {
			c_ = src_.get();
			column_++;
			switch (c_) {
			case 'r':
				buffer += "\x0D";
				break;
			case 'n':
				buffer += "\x0A";
				break;
			case 't':
				buffer += "\x09";
				break;
			case '"':
				// FALL THROUGH
			case '\\':
				buffer += c_;
				break;
			default:
				std::string buffer;
				buffer += c_;
				error = std::make_shared<UnexpectedCharactorError>(makeToken(Token::Type::UNDEFINED, buffer));
				return true;
			}
		}
		else {
			buffer += c_;
		}

		c_ = src_.get();
		column_++;
	}
	c_ = src_.get();
	column_++;

	result = makeToken(Token::Type::CONSTANT_STRING, buffer);
	return false;
}

bool Tokenizer::getOtherToken(Token& result, std::shared_ptr<CompileError>& error) {
	std::string buffer;

	if ((c_ == ' ') || (c_ == '\t')) {
		c_ = src_.get();
		column_++;
		while ((c_ == ' ') || (c_ == '\t')) {
			c_ = src_.get();
			column_++;
		}
		previousLine_ = line_;
		previousColumn_ = column_;
		return getToken(result, error);
	}
	else if (c_ == '_' || isalpha(c_)) {
		buffer += c_;
		c_ = src_.get();
		column_++;
		while (c_ == '_' || isalnum(c_)) {
			buffer += c_;
			c_ = src_.get();
			column_++;
		}

		bool found = false;
		for (const auto& k : kKeywords) {
			if (buffer == k.keyword) {
				result = makeToken(k.type, k.keyword);
				found = true;
				break;
			}
		}
		if (!found) {
			result = makeToken(Token::Type::SYMBOL, buffer);
		}
	}
	else if (isdigit(c_)) {
		buffer += c_;
		c_ = src_.get();
		column_++;
		while (c_ == '_' || isdigit(c_)) {
			if (c_ != '_') {
				buffer += c_;
			}
			c_ = src_.get();
			column_++;
		}

		if (c_ == '.') {
			buffer += c_;
			c_ = src_.get();
			column_++;
			while (c_ == '_' || isdigit(c_)) {
				if (c_ != '_') {
					buffer += c_;
				}
				c_ = src_.get();
				column_++;
			}
			result = makeToken(Token::Type::CONSTANT_FLOAT, buffer);
		}
		else {
			result = makeToken(Token::Type::CONSTANT_INTEGER, buffer);
		}
	}
	else {
		std::string buffer;
		buffer += c_;
		error = std::make_shared<UnexpectedCharactorError>(makeToken(Token::Type::UNDEFINED, buffer));
		return true;
	}

	return false;
}

Token Tokenizer::makeToken(Token::Type type, const std::string& str) {
	Token token(type, str, filepath_, previousLine_, previousColumn_);
	previousLine_ = line_;
	previousColumn_ = column_;
	return token;
}

void Tokenizer::readNewLine() {
	line_++;
	column_ = 1;
	if (c_ == '\r') {
		c_ = src_.get();
		if (c_ == '\n') {
			c_ = src_.get();
		}
	}
	else {
		c_ = src_.get();
	}

	previousLine_ = line_;
	previousColumn_ = column_;
}

const std::string& Tokenizer::getKeywordString(Token::Type type) {
	static const std::string empty;
	for (auto& keyword : kKeywords) {
		if (type == keyword.type) {
			return keyword.keyword;
		}
	}
	return empty;
}

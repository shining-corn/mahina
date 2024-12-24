#pragma once

#include <istream>
#include "Token.h"
#include "CompileError.h"

class Tokenizer
{
public:
	Tokenizer(std::istream& src, const std::string& filepath) : c_(-1), src_(src), filepath_(filepath), line_(1), column_(1), previousLine_(1), previousColumn_(1) {}
	~Tokenizer() = default;

	bool initialize(std::shared_ptr<CompileError>& error);
	bool getToken(Token& result, std::shared_ptr<CompileError>& error);

	const std::string& getFilepath() const {
		return filepath_;
	}

	static const std::string& getKeywordString(Token::Type type);

private:
	int c_;
	std::istream& src_;
	const std::string& filepath_;
	size_t line_;
	size_t column_;
	size_t previousLine_;
	size_t previousColumn_;

	bool getStringLiteralToken(Token&, std::shared_ptr<CompileError>&);
	bool getOtherToken(Token&, std::shared_ptr<CompileError>&);
	Token makeToken(Token::Type, const std::string&);
	void readNewLine();
};

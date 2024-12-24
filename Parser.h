#pragma once

#include <fstream>
#include <vector>
#include "Tokenizer.h"
#include "Node.h"
#include "CompileError.h"

class Parser
{
public:
	Parser(const std::string& sourcePath) : src_(sourcePath, std::ios::binary), tokenizer_(src_, sourcePath) {}
	virtual ~Parser() = default;

	bool fail() const;
	bool parse();

	const std::vector<std::shared_ptr<CompileError>>& getErrors() const {
		return errors_;
	}

	Context& getRootNode() {
		return context_;
	}

private:
	std::ifstream src_;
	Tokenizer tokenizer_;
	Context context_;
	std::vector<std::shared_ptr<CompileError>> errors_;
	Token currentToken_;

	bool next();
	bool expect(Token::Type);
	bool expectTypeOrSymbol();

	bool parseStruct(StructNode&);
	bool parseDeclare(FunctionNode&);
	bool parseFunction(FunctionNode&);

	bool parseBlock(BlockNode&);
	bool parseLet(std::shared_ptr<StatementNode>&);
	bool parseIf(std::shared_ptr<StatementNode>&);
	bool parseWhile(std::shared_ptr<StatementNode>&);
	bool parseReturn(std::shared_ptr<StatementNode>&);
	bool parseBreak(std::shared_ptr<StatementNode>&);

	bool parseType(TypeNode&);
	bool parsePrimitiveType(TypeNode&);
	bool parseAssignOrCall(std::shared_ptr<StatementNode>&);
	bool parseVariableValue(VariableValueNode&);
	bool parseValueList(ValueListNode&);

	bool parseExpression(std::shared_ptr<ExpressionNode>&);
	bool parseLogicalOr(std::shared_ptr<ExpressionNode>&);
	bool parseLogicalAnd(std::shared_ptr<ExpressionNode>&);
	bool parseCompareEqualOrNotEqual(std::shared_ptr<ExpressionNode>&);
	bool parseCompareGreaterOrLesser(std::shared_ptr<ExpressionNode>&);
	bool parsePlusMinus(std::shared_ptr<ExpressionNode>&);
	bool parseMulDivMod(std::shared_ptr<ExpressionNode>&);
	bool parseValue(std::shared_ptr<ExpressionNode>&);
	bool parseCast(std::shared_ptr<ExpressionNode>&);
};
